#include "RequestHTTP.hpp"

HTTP::t_method discriminate_request_method(const HTTP::light_string &str) {
    if (str == "GET") {
        return HTTP::METHOD_GET;
    }
    if (str == "POST") {
        return HTTP::METHOD_POST;
    }
    if (str == "DELETE") {
        return HTTP::METHOD_DELETE;
    }
    throw http_error("unsupported method", HTTP::STATUS_NOT_IMPLEMENTED);
}

HTTP::t_version discriminate_request_version(const HTTP::light_string &str) {
    if (str == HTTP::version_str(HTTP::V_1_0)) {
        return HTTP::V_1_0;
    }
    if (str == HTTP::version_str(HTTP::V_1_1)) {
        return HTTP::V_1_1;
    }
    throw http_error("unsupported version", HTTP::STATUS_VERSION_NOT_SUPPORTED);
}

RequestHTTP::Attribute::Attribute() : client_max_body_size(0) {}

RequestHTTP::ParserStatus::ParserStatus()
    : mid(0)
    , found_obs_fold(false)
    , start_of_reqline(0)
    , end_of_reqline(0)
    , start_of_header(0)
    , end_of_header(0)
    , start_of_body(0)
    , end_of_body(0)
    , is_freezed(false) {}

RequestHTTP::RequestHTTP()
    : lifetime(Lifetime::make_request()), lifetime_header(Lifetime::make_request_header()), rp() {
    DXOUT("[create_request]");
    this->ps.parse_progress = PP_REQLINE_START;
    bytebuffer.reserve(MAX_REQLINE_END);
    lifetime.activate();
    lifetime_header.activate();
}

RequestHTTP::~RequestHTTP() {}

void RequestHTTP::after_injection(bool is_disconnected) {
    size_t len;
    t_parse_progress flow;
    do {
        len = bytebuffer.size() - this->ps.mid;
        switch (this->ps.parse_progress) {
            case PP_REQLINE_START: {
                flow = reach_reqline_start(len, is_disconnected);
                if (flow == PP_UNREACHED) {
                    return;
                }
                this->ps.parse_progress = flow;
                continue;
            }

            case PP_REQLINE_END: {
                flow = reach_reqline_end(len, is_disconnected);
                if (flow == PP_UNREACHED) {
                    return;
                }
                this->ps.parse_progress = flow;
                continue;
            }

            case PP_HEADER_SECTION_END: {
                flow = reach_headers_end(len, is_disconnected);
                if (flow == PP_UNREACHED) {
                    return;
                }
                this->ps.parse_progress = flow;
                continue;
            }

            case PP_BODY: {
                flow = reach_fixed_body_end(len, is_disconnected);
                if (flow == PP_UNREACHED) {
                    return;
                }
                this->ps.parse_progress = flow;
                continue;
            }

            case PP_CHUNK_SIZE_LINE_END: {
                flow = reach_chunked_size_line(len, is_disconnected);
                if (flow == PP_UNREACHED) {
                    return;
                }
                this->ps.parse_progress = flow;
                continue;
            }

            case PP_CHUNK_DATA_END: {
                flow = reach_chunked_data_end(len, is_disconnected);
                if (flow == PP_UNREACHED) {
                    return;
                }
                this->ps.parse_progress = flow;
                continue;
            }

            case PP_CHUNK_DATA_CRLF: {
                flow = reach_chunked_data_termination(len, is_disconnected);
                if (flow == PP_UNREACHED) {
                    return;
                }
                this->ps.parse_progress = flow;
                continue;
            }

            case PP_TRAILER_FIELD_END: {
                flow = reach_chunked_trailer_end(len, is_disconnected);
                if (flow == PP_UNREACHED) {
                    return;
                }
                this->ps.parse_progress = flow;
                continue;
            }

            case PP_OVER: {
                // 仮: ここでルーティングを開始

                return;
            }

            default:
                throw http_error("not implemented yet", HTTP::STATUS_NOT_IMPLEMENTED);
        }
    } while (len > 0 || this->ps.parse_progress == PP_OVER);
}

RequestHTTP::t_parse_progress RequestHTTP::reach_reqline_start(size_t len, bool is_disconnected) {
    (void)is_disconnected;
    DXOUT("* determining start_of_reqline... *");
    size_t non_crlf_heading = ParserHelper::ignore_crlf(bytebuffer, this->ps.mid, len);
    this->ps.mid += non_crlf_heading;
    if (non_crlf_heading == len) {
        return PP_UNREACHED;
    }
    // 開始行の開始位置が定まった
    this->ps.start_of_reqline = this->ps.mid;
    return PP_REQLINE_END;
}

RequestHTTP::t_parse_progress RequestHTTP::reach_reqline_end(size_t len, bool is_disconnected) {
    (void)is_disconnected;
    if (!seek_reqline_end(len)) {
        return PP_UNREACHED;
    }
    light_string raw_req_line(bytebuffer, this->ps.start_of_reqline, this->ps.end_of_reqline);
    // -> [start_of_reqline, end_of_reqline) が 開始行かどうか調べる.
    parse_reqline(raw_req_line);
    return PP_HEADER_SECTION_END;
}

RequestHTTP::t_parse_progress RequestHTTP::reach_headers_end(size_t len, bool is_disconnected) {
    // ヘッダ部の終わりを探索する
    // `crlf_in_header` には「最後に見つけたCRLF」のレンジが入っている.
    // mid以降についてCRLFを検索する:
    // - 見つかった
    //   - first == crlf_in_header.second である
    //     -> あたり. このCRLFがヘッダの終わりを示す.
    //   - そうでない
    //     -> はずれ. このCRLFを crlf_in_header として続行.
    // - 見つからなかった
    //   -> もう一度受信する
    (void)is_disconnected;
    IndexRange res = ParserHelper::find_crlf(bytebuffer, this->ps.mid, len);
    this->ps.mid   = res.second;
    if (res.is_invalid()) {
        return PP_UNREACHED;
    }
    if (this->ps.crlf_in_header.second != res.first) {
        // はずれ: CRLFとCRLFの間が空いていた
        this->ps.crlf_in_header = res;
        return PP_HEADER_SECTION_END;
    }
    // あたり: ヘッダの終わりが見つかった
    minor_error me  = analyze_headers(res);
    this->ps.merror = me;
    VOUT(me);
    lifetime_header.deactivate();
    if (this->rp.is_body_chunked) {
        return PP_CHUNK_SIZE_LINE_END;
    } else {
        return PP_BODY;
    }
}

RequestHTTP::t_parse_progress RequestHTTP::reach_fixed_body_end(size_t len, bool is_disconnected) {
    // 前提: 本文長は chunked では決まらない
    //
    // 1. 受信済み本文長が想定される本文長以上になった
    // 2. 接続が切れた
    // のいずれかが成り立つ場合は, 本文の最後に到達したものとする.
    //
    // [1.が成り立っている場合]
    // 想定される本文長から`end_of_body`を逆算する.
    // [1.が成り立っていない場合]
    // (-> 2.は成り立っているはず)
    // `end_of_body` = `start_of_body` + 受信済み本文長 とする.
    // さらにリクエストを不完全マークする.
    this->ps.mid += len;
    if (parsed_body_size() >= this->rp.body_size) {
        this->ps.end_of_body = this->ps.start_of_body + this->rp.body_size;
        return PP_OVER;
    }
    if (is_disconnected) {
        this->ps.end_of_body = this->ps.start_of_body + parsed_body_size();
        return PP_OVER;
    }
    return PP_UNREACHED;
}

RequestHTTP::t_parse_progress RequestHTTP::reach_chunked_size_line(size_t len, bool is_disconnected) {
    (void)is_disconnected;
    IndexRange res = ParserHelper::find_crlf(bytebuffer, this->ps.mid, len);
    this->ps.mid   = res.second;
    if (res.is_invalid()) {
        return PP_UNREACHED;
    }
    // [start_of_current_chunk, res.first) がサイズ行のはず.
    light_string chunk_size_line(bytebuffer, this->ps.start_of_current_chunk, res.first);
    QVOUT(chunk_size_line);
    parse_chunk_size_line(chunk_size_line);
    // TODO: サイズ行の解析
    VOUT(this->ps.current_chunk.chunk_size);
    if (this->ps.current_chunk.chunk_size == 0) {
        // 最終チャンクなら PP_TRAILER_FIELD_END に飛ばす
        this->ps.crlf_in_header         = res;
        this->ps.start_of_trailer_field = res.second;
        VOUT(chunked_body.size());
        VOUT(chunked_body.body());
        VOUT(this->ps.crlf_in_header.first);
        VOUT(this->ps.crlf_in_header.second);
        VOUT(this->ps.mid);
        return PP_TRAILER_FIELD_END;
    } else {
        this->ps.start_of_current_chunk_data = this->ps.mid;
        return PP_CHUNK_DATA_END;
    }
}

RequestHTTP::t_parse_progress RequestHTTP::reach_chunked_data_end(size_t len, bool is_disconnected) {
    (void)is_disconnected;
    const byte_string::size_type received_data_size = this->ps.mid - this->ps.start_of_current_chunk_data;
    const byte_string::size_type data_end           = this->ps.current_chunk.chunk_size - received_data_size;
    if (data_end > len) {
        this->ps.mid += len;
        return PP_UNREACHED;
    }
    this->ps.mid += data_end;
    this->ps.current_chunk.chunk_str = light_string(bytebuffer, this->ps.start_of_current_chunk, this->ps.mid);
    this->ps.current_chunk.data_str  = light_string(bytebuffer, this->ps.start_of_current_chunk_data, this->ps.mid);
    QVOUT(this->ps.current_chunk.chunk_str);
    QVOUT(this->ps.current_chunk.data_str);
    return PP_CHUNK_DATA_CRLF;
}

RequestHTTP::t_parse_progress RequestHTTP::reach_chunked_data_termination(size_t len, bool is_disconnected) {
    VOUT(this->ps.mid);
    IndexRange nl = ParserHelper::find_leading_crlf(bytebuffer, this->ps.mid, len, is_disconnected);
    if (nl.is_invalid()) {
        throw http_error("invalid chunk-data end?", HTTP::STATUS_BAD_REQUEST);
    }
    if (nl.length() == 0) {
        return PP_UNREACHED;
    }
    this->ps.current_chunk.data_str = light_string(bytebuffer, this->ps.start_of_current_chunk_data, this->ps.mid);
    this->ps.mid                    = nl.second;
    this->ps.start_of_current_chunk = this->ps.mid;
    chunked_body.add_chunk(this->ps.current_chunk);
    VOUT(this->ps.mid);
    return PP_CHUNK_SIZE_LINE_END;
}

RequestHTTP::t_parse_progress RequestHTTP::reach_chunked_trailer_end(size_t len, bool is_disconnected) {
    // トレイラーフィールドの終わりを探索する
    // `crlf_in_header` には「最後に見つけたCRLF」のレンジが入っている.
    // mid以降についてCRLFを検索する:
    // - 見つかった
    //   - first == crlf_in_header.second である
    //     -> あたり. このCRLFがヘッダの終わりを示す.
    //   - そうでない
    //     -> はずれ. このCRLFを crlf_in_header として続行.
    // - 見つからなかった
    //   -> もう一度受信する
    (void)is_disconnected;
    IndexRange res = ParserHelper::find_crlf(bytebuffer, this->ps.mid, len);
    this->ps.mid   = res.second;
    if (res.is_invalid()) {
        return PP_UNREACHED;
    }
    if (this->ps.crlf_in_header.second != res.first) {
        // はずれ
        this->ps.crlf_in_header = res;
        return PP_TRAILER_FIELD_END;
    }
    // あたり
    this->ps.end_of_trailer_field = res.first;
    const light_string trailer_field_lines(bytebuffer, this->ps.start_of_trailer_field, this->ps.end_of_trailer_field);
    this->header_holder.parse_header_lines(trailer_field_lines, NULL);
    return PP_OVER;
}

minor_error RequestHTTP::analyze_headers(IndexRange res) {
    this->ps.end_of_header = res.first;
    this->ps.start_of_body = res.second;
    // -> [start_of_header, end_of_header) を解析する
    const light_string header_lines(bytebuffer, this->ps.start_of_header, this->ps.end_of_header);
    minor_error me;

    minor_error header_error  = this->header_holder.parse_header_lines(header_lines, &this->header_holder);
    me                        = me.is_error() ? me : header_error;
    minor_error control_error = extract_control_headers();
    me                        = me.is_error() ? me : control_error;

    if (this->rp.is_body_chunked) {
        this->ps.start_of_current_chunk = this->ps.start_of_body;
    }
    VOUT(me);
    return me;
}

bool RequestHTTP::seek_reqline_end(size_t len) {
    // DSOUT() << "* determining end_of_reqline... *" << std::endl;
    IndexRange res = ParserHelper::find_crlf(bytebuffer, this->ps.mid, len);
    this->ps.mid   = res.second;
    if (res.is_invalid()) {
        return false;
    }
    // CRLFが見つかった
    this->ps.end_of_reqline  = res.first;
    this->ps.start_of_header = this->ps.mid;
    // -> end_of_reqline が8192バイト以内かどうか調べる。
    if (MAX_REQLINE_END <= this->ps.end_of_reqline) {
        throw http_error("Invalid Response: request line is too long", HTTP::STATUS_URI_TOO_LONG);
    }
    this->ps.crlf_in_header = IndexRange(this->ps.start_of_header, this->ps.start_of_header);
    return true;
}

void RequestHTTP::parse_reqline(const light_string &raw_req_line) {
    std::vector<light_string> splitted = ParserHelper::split_by_sp(raw_req_line);

    switch (splitted.size()) {
        case 2:
        case 3: {
            // HTTP/0.9?
            // HTTP/1.*?
            this->rp.http_method = discriminate_request_method(splitted[0]);
            DXOUT(splitted[0] << " -> http_method: " << this->rp.http_method);
            this->rp.given_request_target = RequestTarget(splitted[1]);
            if (this->rp.given_request_target.decoded_target_has_unacceptable()) {
                throw http_error("request target has unacceptable data", HTTP::STATUS_BAD_REQUEST);
            }
            DXOUT("given_request_target:");
            DXOUT(this->rp.given_request_target);
            if (splitted.size() == 3) {
                this->rp.http_version = discriminate_request_version(splitted[2]);
            } else {
                this->rp.http_version = HTTP::V_0_9;
            }
            check_reqline_consistensy();
            break;
        }
        default:
            throw http_error("invalid request-line?", HTTP::STATUS_BAD_REQUEST);
    }
    DXOUT("* parsed reqline *");
    if (this->rp.http_version.is_null()) {
        throw http_error("unspecified http-version", HTTP::STATUS_VERSION_NOT_SUPPORTED);
    }
    if (this->rp.http_version.value() != HTTP::V_1_1) {
        throw http_error("unsupported http-version", HTTP::STATUS_VERSION_NOT_SUPPORTED);
    }
}

void RequestHTTP::check_reqline_consistensy() {
    switch (this->rp.given_request_target.form) {
        case RequestTarget::FORM_ORIGIN:
            break;
        case RequestTarget::FORM_ABSOLUTE:
            break;
        case RequestTarget::FORM_ASTERISK:
            // server-wide OPTIONSでないとエラー -> OPTIONS実装してないので即エラー
            throw http_error("asterisk-form is available for only server-wide OPTIONS", HTTP::STATUS_BAD_REQUEST);
        case RequestTarget::FORM_AUTHORITY:
            // CONNECTでないとエラー -> CONNECT実装してないので即エラー
            throw http_error("authority-form is available for only CONNECT", HTTP::STATUS_BAD_REQUEST);
        default:
            throw http_error("request target's form is unknown", HTTP::STATUS_BAD_REQUEST);
    }
    if (this->rp.given_request_target.is_error) {
        this->ps.merror
            = erroneous(this->ps.merror, minor_error("request target is invalid", HTTP::STATUS_BAD_REQUEST));
    }
    DXOUT("OK.");
}

void RequestHTTP::parse_chunk_size_line(const light_string &line) {
    light_string size_str = line.substr_while(HTTP::CharFilter::hexdig);
    if (size_str.size() == 0) {
        throw http_error("no size for chunk", HTTP::STATUS_BAD_REQUEST);
    }
    // chunk-size の解析
    // chunk-size   = 1*HEXDIG
    this->ps.current_chunk.size_str = size_str;
    QVOUT(this->ps.current_chunk.size_str);
    std::pair<bool, unsigned int> x = ParserHelper::xtou(size_str);
    if (!x.first) {
        throw http_error("invalid size for chunk", HTTP::STATUS_BAD_REQUEST);
    }
    this->ps.current_chunk.chunk_size = x.second;
    ;
    VOUT(this->ps.current_chunk.chunk_size);
    // chunk-ext の解析(スルーするだけ)
    // chunk-ext    = *( BWS ";" BWS chunk-ext-name [ BWS "=" BWS chunk-ext-val ] )
    // chunk-ext-name = token
    // chunk-ext-val  = token / quoted-string
    light_string rest = line.substr(size_str.size());
    for (;;) {
        QVOUT(rest);
        rest = rest.substr_after(HTTP::CharFilter::bad_sp);
        if (rest.size() == 0) {
            DXOUT("away");
            break;
        }
        if (rest[0] != ';') {
            DXOUT("[KO] bad separator");
            break;
        }
        rest = rest.substr_after(HTTP::CharFilter::bad_sp, 1);
        QVOUT(rest);
        const light_string chunk_ext_name = rest.substr_while(HTTP::CharFilter::tchar);
        QVOUT(chunk_ext_name);
        if (chunk_ext_name.size() == 0) {
            DXOUT("[KO] no chunk_ext_name");
            break;
        }
        rest = rest.substr(chunk_ext_name.size());
        rest = rest.substr_after(HTTP::CharFilter::bad_sp);
        QVOUT(rest);
        if (rest.size() > 0 && rest[0] == '=') {
            // chunk-ext-val がある
            rest                             = rest.substr(1);
            const light_string chunk_ext_val = ParserHelper::extract_quoted_or_token(rest);
            QVOUT(chunk_ext_val);
            if (chunk_ext_val.size() == 0) {
                DXOUT("[KO] zero-width chunk_ext_val");
                break;
            }
            rest = rest.substr(chunk_ext_val.size());
        }
    }
}

minor_error RequestHTTP::extract_control_headers() {
    // 取得したヘッダから制御用の情報を抽出する.
    // TODO: ここで何を抽出すべきか洗い出す
    minor_error me;
    me = erroneous(me, this->rp.determine_host(header_holder));
    me = erroneous(me, this->rp.content_type.determine(header_holder));
    me = erroneous(me, this->rp.content_disposition.determine(header_holder));
    me = erroneous(me, this->rp.transfer_encoding.determine(header_holder));
    me = erroneous(me, this->rp.content_length.determine(header_holder));
    this->rp.determine_body_size();
    me = erroneous(me, this->rp.connection.determine(header_holder));
    me = erroneous(me, this->rp.te.determine(header_holder));
    me = erroneous(me, this->rp.upgrade.determine(header_holder));
    me = erroneous(me, this->rp.via.determine(header_holder));
    me = erroneous(me, this->rp.date.determine(header_holder));
    me = erroneous(me, this->rp.if_modified_since.determine(header_holder));
    me = erroneous(me, this->rp.cookie.determine(header_holder));
    VOUT(me);
    return me;
}

void RequestHTTP::inject_reroute_path(const HTTP::byte_string &path) {
    rp.reroute_path           = path;
    rp.reroute_request_target = RequestTarget(rp.reroute_path);
    rp.use_reroute            = true;
    VOUT(rp.reroute_request_target);
}

void RequestHTTP::check_size_limitation() {
    // ボディ
    if (attr.client_max_body_size > 0) {
        if (effective_parsed_body_size() > (size_t)attr.client_max_body_size) {
            // ダメ
            throw http_error("request body size exceeded the limit", HTTP::STATUS_PAYLOAD_TOO_LARGE);
        }
    }
}

// [RequestHTTP::RoutingParameters]

RequestHTTP::RoutingParameters::RoutingParameters()
    : use_reroute(false), http_method(HTTP::METHOD_UNKNOWN), is_body_chunked(false) {}

void RequestHTTP::RoutingParameters::determine_body_size() {
    // https://www.rfc-editor.org/rfc/rfc9112.html#name-message-body-length

    // メッセージが Transfer-Encoding: と Content-Length: をどちらも持っている場合,
    // Transfer-Encoding: のみがある扱いとする.
    // --
    // If a message is received with both a Transfer-Encoding and a Content-Length header field,
    // the Transfer-Encoding overrides the Content-Length.

    if (transfer_encoding.currently_chunked) {
        // メッセージが Transfer-Encoding: を持ち, かつ最後の encoding が chunked である場合,
        // 本文長は chunked で決定される.
        // --
        // If a Transfer-Encoding header field is present
        // and the chunked transfer coding (Section 7.1) is the final encoding,
        // the message body length is determined by reading
        // and decoding the chunked data until the transfer coding indicates the data is complete.

        // HTTPリクエストに Transfer-Encoding: があり, かつ chunked が最後の encoding でない場合,
        // ボディの長さを確実に決定することができないので 400 とすべき.
        // (ボディの長さを決定できないので持続的接続もできなくなる -> 接続を閉じるべき)
        // --
        // If a Transfer-Encoding header field is present in a request
        // and the chunked transfer coding is not the final encoding,
        // the message body length cannot be determined reliably;
        // the server MUST respond with the 400 (Bad Request) status code and then close the connection.
        DXOUT("by chunk");
        is_body_chunked = true;
        return;
    }

    if (transfer_encoding.empty()) {
        // HTTPメッセージが Transfer-Encoding: を持たず Content-Length: もおかしい場合,
        // これを受信者は回復不能なエラーとしなければならない
        // (Content-Length: がリストであり, その値が全て同じかつ有効だった場合は「おかしい」とは呼ばない.)
        // リクエストで回復不能なエラーが起きた場合, 400を返して接続を切る.
        // --
        // If a message is received without Transfer-Encoding and with an invalid Content-Length header field,
        // then the message framing is invalid and the recipient MUST treat it as an unrecoverable error,
        // unless the field value can be successfully parsed as a comma-separated list (Section 5.6.1 of [HTTP]),
        // all values in the list are valid, and all values in the list are the same
        // (in which case, the message is processed with that single value used as the Content-Length field value).
        // If the unrecoverable error is in a request message, the server MUST respond with a 400 (Bad Request) status
        // code and then close the connection. If it is in a response message received by a proxy, the proxy MUST close
        // the connection to the server, discard the received response, and send a 502 (Bad Gateway) response to the
        // client. If it is in a response message received by a user agent, the user agent MUST close the connection to
        // the server and discard the received response.

        // Transfer-Encoding: がなく, valid な Content-Length: がある場合, その(10進の)値が本文長となる.
        // 本文長に達する前に接続が閉じられた場合, メッセージは「不完全」となる.
        // --
        // If a valid Content-Length header field is present without Transfer-Encoding, its decimal value defines the
        // expected message body length in octets. If the sender closes the connection or the recipient times out before
        // the indicated number of octets are received, the recipient MUST consider the message to be incomplete and
        // close the connection.
        if (content_length.merror.is_error()) {
            throw http_error("multiple content-length", HTTP::STATUS_BAD_REQUEST);
        }
        if (!content_length.empty()) {
            body_size       = content_length.value;
            is_body_chunked = false;
            // content-length の値が妥当でない場合, ここで例外が飛ぶ
            DXOUT("body_size = " << body_size);
            return;
        }
    }
    // これまでの全てが当てはまらないHTTPリクエストは, 本文長は0となる.
    // --
    // If this is a request message and none of the above are true,
    // then the message body length is zero (no message body is present).
    body_size       = 0;
    is_body_chunked = false;
    DXOUT("body_size is zero.");

    // Note: Request messages are never close-delimited because
    // they are always explicitly framed by length or transfer coding,
    // with the absence of both implying the request ends
    // immediately after the header section.
}

minor_error RequestHTTP::RoutingParameters::determine_host(const header_holder_type &holder) {
    // https://triple-underscore.github.io/RFC7230-ja.html#header.host
    if (http_version.is_null()) {
        // この時点で null なのはおかしい
        throw http_error("version is undefined", HTTP::STATUS_BAD_REQUEST);
    }

    const header_holder_type::value_list_type *hosts = holder.get_vals(HeaderHTTP::host);
    if (!hosts || hosts->size() == 0) {
        // HTTP/1.1 なのに Host: がない場合, BadRequest を出す
        if (http_version.value() == HTTP::V_1_1) {
            return minor_error::make("no host for HTTP/1.1", HTTP::STATUS_BAD_REQUEST);
        }
        // Host: がないけどHTTP/1.1でない
        // TODO: okでいいの?
        return minor_error::ok();
    }
    if (hosts->size() > 1) {
        // Host: が複数ある場合 -> BadRequest
        return minor_error::make("multiple hosts", HTTP::STATUS_BAD_REQUEST);
    }
    // Host: の値のバリデーション (と, 必要ならBadRequest)
    const HTTP::light_string lhost(hosts->front());
    if (!HTTP::Validator::is_valid_header_host(lhost)) {
        return minor_error::make("host is not valid", HTTP::STATUS_BAD_REQUEST);
    }
    pack_host(header_host, lhost);
    return minor_error::ok();
}

const RequestTarget &RequestHTTP::RoutingParameters::get_request_target() const {
    if (use_reroute) {
        return reroute_request_target;
    } else {
        return given_request_target;
    }
}

HTTP::t_method RequestHTTP::RoutingParameters::get_http_method() const {
    return http_method;
}

HTTP::t_version RequestHTTP::RoutingParameters::get_http_version() const {
    if (http_version.is_null()) {
        return HTTP::DEFAULT_HTTP_VERSION;
    }
    return http_version.value();
}

const HTTP::CH::Host &RequestHTTP::RoutingParameters::get_host() const {
    return header_host;
}

bool RequestHTTP::is_routable() const {
    return this->ps.parse_progress >= PP_BODY;
}

bool RequestHTTP::is_complete() const {
    return this->ps.parse_progress >= PP_OVER;
}

bool RequestHTTP::is_freezed() const {
    return this->ps.is_freezed;
}

bool RequestHTTP::is_timeout(t_time_epoch_ms now) const {
    return lifetime.is_timeout(now) || lifetime_header.is_timeout(now);
}

size_t RequestHTTP::receipt_size() const {
    return bytebuffer.size();
}

size_t RequestHTTP::parsed_body_size() const {
    return this->ps.mid - this->ps.start_of_body;
}

size_t RequestHTTP::effective_parsed_body_size() const {
    if (!rp.is_body_chunked && ps.parse_progress >= PP_OVER) {
        return ps.end_of_body - ps.start_of_body;
    }
    return parsed_body_size();
}

size_t RequestHTTP::parsed_size() const {
    return this->ps.mid;
}

HTTP::t_version RequestHTTP::get_http_version() const {
    return this->rp.get_http_version();
}

HTTP::t_method RequestHTTP::get_method() const {
    return this->rp.http_method;
}

const RequestHTTP::byte_string &RequestHTTP::get_content_type() const {
    return this->rp.content_type.value;
}

const HTTP::CH::ContentType &RequestHTTP::get_content_type_item() const {
    return this->rp.content_type;
}

const HTTP::CH::ContentDisposition &RequestHTTP::get_content_disposition_item() const {
    return this->rp.content_disposition;
}

const HTTP::CH::IfModifiedSince &RequestHTTP::get_if_modified_since() const {
    return this->rp.if_modified_since;
}

RequestHTTP::byte_string RequestHTTP::generate_body_data() const {
    if (rp.is_body_chunked) {
        return chunked_body.body();
    } else {
        return byte_string(bytebuffer.begin() + this->ps.start_of_body, bytebuffer.begin() + this->ps.end_of_body);
    }
}

RequestHTTP::byte_string RequestHTTP::get_plain_message() const {
    return RequestHTTP::byte_string(bytebuffer.begin(), bytebuffer.begin() + this->ps.mid);
}

void RequestHTTP::set_max_body_size(ssize_t size) {
    attr.client_max_body_size = size;
    check_size_limitation();
}

RequestHTTP::light_string RequestHTTP::freeze() {
    if (this->ps.is_freezed) {
        return light_string();
    }
    this->ps.is_freezed = true;
    lifetime.deactivate();
    return light_string(bytebuffer, this->ps.end_of_body);
}

bool RequestHTTP::should_keep_in_touch() const {
    // TODO: 仮実装
    if (this->rp.connection.close_) {
        return false;
    }
    return true;
}

RequestHTTP::header_holder_type::joined_dict_type RequestHTTP::get_cgi_meta_vars() const {
    return header_holder.get_cgi_meta_vars();
}

const IRequestMatchingParam &RequestHTTP::get_request_matching_param() const {
    return rp;
}

minor_error RequestHTTP::purge_error() {
    if (ps.merror.is_ok()) {
        return ps.merror;
    }
    minor_error to_purge = ps.merror;
    ps.merror            = minor_error::ok();
    return to_purge;
}

const minor_error &RequestHTTP::current_error() const {
    return ps.merror;
}
