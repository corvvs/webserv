#include "CGI.hpp"
#include "../communication/RoundTrip.hpp"
#include <cstring>
#include <signal.h>
#include <unistd.h>
#define MAX_SEND_SIZE 1024
#define MAX_RECEIVE_SIZE 1024

extern char **environ;

// fd`to`をfd`from`と同一視する
int redirect_fd(t_fd from, t_fd to) {
    if (from == to) {
        return 0;
    }
    // TODO: closeのエラーチェック
    close(to);
    return dup2(from, to);
}

CGI::ParserStatus::ParserStatus() : parse_progress(PP_HEADER_SECTION_END), start_of_header(0), is_freezed(false) {}

CGI::CGI(const byte_string &script_path,
         const byte_string &query_string,
         metavar_dict_type &metavar,
         size_type content_length)
    : attr((Attribute){script_path, query_string, NULL, NULL, 0, NULL})
    , metavar_(metavar)
    , content_length_(content_length)
    , mid(0) {
    memset(&status, 0, sizeof(Status));
    ps.start_of_header = 0;
}

CGI::~CGI() {
    // TODO: 関数化; 異常終了したことを検知できるように
    if (attr.cgi_pid != 0) {
        // TODO: 条件付き?
        ::kill(attr.cgi_pid, SIGKILL);
        waitpid(attr.cgi_pid, NULL, 0);
        attr.cgi_pid = 0;
    }
    delete attr.sock;
    DXOUT("DESTROYED: " << this);
}

void CGI::inject_socketlike(ISocketLike *socket_like) {
    VOUT(socket_like);
    attr.master = socket_like;
}

void CGI::start_origination(IObserver &observer) {
    // TODO: CGIが起動できるかどうかチェックする

    std::pair<SocketUNIX *, t_fd> socks = SocketUNIX::socket_pair();

    pid_t pid = fork();
    VOUT(pid);
    if (pid < 0) {
        throw std::runtime_error("failed to fork");
    }
    if (pid == 0) {
        // child: CGI process
        delete socks.first;
        // 引数の準備
        char **argv = flatten_argv(attr.script_path_);
        if (argv == NULL) {
            exit(1);
        }
        char **mvs = flatten_metavar(metavar_);
        if (mvs == NULL) {
            exit(1);
        }
        // 子プロセス側ソケットに標準入出力をマップ
        if (redirect_fd(socks.second, STDIN_FILENO) < 0) {
            exit(1);
        }
        if (redirect_fd(socks.second, STDOUT_FILENO) < 0) {
            exit(1);
        }
        // if (redirect_fd(socks.second, STDERR_FILENO) < 0) {
        //     exit(1);
        // }
        // 起動
        errno  = 0;
        int rv = execve(HTTP::restrfy(attr.script_path_).c_str(), argv, mvs);
        VOUT(rv);
        VOUT(errno);
        QVOUT(strerror(errno));
        exit(0);
    }
    // parent: server process
    attr.cgi_pid = pid;
    attr.sock    = socks.first;
    close(socks.second);
    attr.observer = &observer;
    observer.reserve_hold(this);
    observer.reserve_set(this, IObserver::OT_READ);
    observer.reserve_set(this, IObserver::OT_WRITE);
    status.is_started = true;
}

CGI::byte_string CGI::draw_data() const {
    return content_response_;
}

char **CGI::flatten_argv(const byte_string &script_path) {
    size_t n     = 2;
    char **frame = (char **)malloc(sizeof(char *) * n);
    if (frame == NULL) {
        return frame;
    }
    frame[0] = strdup(HTTP::restrfy(script_path).c_str());
    if (frame[0] == NULL) {
        free(frame);
        return NULL;
    }
    frame[1] = NULL;
    return frame;
}

char **CGI::flatten_metavar(const metavar_dict_type &metavar) {
    metavar_dict_type prodvar;
    for (char **e = environ; *e; ++e) {
        const char *item = *e;
        HTTP::char_string str(item);
        HTTP::char_string::size_type eq = str.find_first_of('=');
        if (eq == HTTP::char_string::npos) {
            // something wrong
            HTTP::byte_string key = HTTP::strfy(item);
            prodvar[key]          = HTTP::strfy("");
            continue;
        }
        HTTP::byte_string key = HTTP::strfy(str.substr(0, eq));
        HTTP::byte_string val = HTTP::strfy(str.substr(eq + 1));
        prodvar[key]          = val;
    }
    for (metavar_dict_type::const_iterator it = metavar.begin(); it != metavar.end(); ++it) {
        prodvar[it->first] = it->second;
    }
    size_t n     = prodvar.size();
    char **frame = (char **)malloc(sizeof(char *) * (n + 1));
    if (frame == NULL) {
        return frame;
    }
    size_t i = 0;
    for (metavar_dict_type::const_iterator it = prodvar.begin(); it != prodvar.end(); ++it, ++i) {
        size_t j   = it->first.size() + 1 + it->second.size();
        char *item = (char *)malloc(sizeof(char) * (j + 1));
        if (item == NULL) {
            for (size_t k = 0; k < i; ++k) {
                free(frame[k]);
            }
            free(frame);
            return NULL;
        }
        memcpy(item, &(it->first.front()), it->first.size());
        item[it->first.size()] = '=';
        memcpy(item + it->first.size() + 1, &(it->second.front()), it->second.size());
        frame[i]    = item;
        frame[i][j] = '\0';
        VOUT(frame[i]);
    }
    frame[n] = NULL;
    return frame;
}

void CGI::set_content(const byte_string &content) {
    content_request_    = content;
    content_length_     = content.size();
    status.content_sent = 0;
}

CGI::metavar_dict_type CGI::make_metavars_from_envp(char **envp) {
    metavar_dict_type metavars;
    if (envp) {
        while (*envp) {
            const HTTP::byte_string var(*envp, *envp + strlen(*envp));
            HTTP::light_string lvar(var);
            HTTP::light_string key_part = lvar.substr_before("=");
            HTTP::light_string val_part;
            if (key_part.length() < lvar.length()) {
                val_part = lvar.substr(key_part.length() + 1);
            }

            metavars.insert(metavar_dict_type::value_type(key_part.str(), val_part.str()));
            ++envp;
        }
    }
    return metavars;
}

t_fd CGI::get_fd() const {
    return attr.sock ? attr.sock->get_fd() : -1;
}

void CGI::notify(IObserver &observer, IObserver::observation_category cat, t_time_epoch_ms epoch) {
    DXOUT("CGI received: " << cat);
    if (attr.master) {
        switch (cat) {
            case IObserver::OT_WRITE:
            case IObserver::OT_READ:
            case IObserver::OT_EXCEPTION:
            case IObserver::OT_TIMEOUT:
                retransmit(observer, cat, epoch);
                return;
            default:
                break;
        }
    }
    switch (cat) {
        case IObserver::OT_WRITE:
        case IObserver::OT_ORIGINATOR_WRITE:
            // CGIスクリプトへのデータ送信
            perform_sending(observer);
            return;
        case IObserver::OT_READ:
        case IObserver::OT_ORIGINATOR_READ:
            perform_receiving(observer);
            return;
        case IObserver::OT_TIMEOUT:
        case IObserver::OT_ORIGINATOR_TIMEOUT:
            VOUT(epoch);
            return;
        default:
            DXOUT("unexpected cat: " << cat);
            assert(false);
    }
}

void CGI::retransmit(IObserver &observer, IObserver::observation_category cat, t_time_epoch_ms epoch) {
    assert(attr.master != NULL);
    switch (cat) {
        case IObserver::OT_READ:
            cat = IObserver::OT_ORIGINATOR_READ;
            break;
        case IObserver::OT_WRITE:
            cat = IObserver::OT_ORIGINATOR_WRITE;
            break;
        case IObserver::OT_EXCEPTION:
            cat = IObserver::OT_ORIGINATOR_EXCEPTION;
            break;
        case IObserver::OT_TIMEOUT:
            cat = IObserver::OT_ORIGINATOR_TIMEOUT;
            break;
        default:
            assert(false);
    }
    attr.master->notify(observer, cat, epoch);
}

void CGI::perform_sending(IObserver &observer) {
    DXOUT("CGI on Write");
    if (content_length_ > 0) {
        ssize_t sent_size = attr.sock->send(&content_request_.front(), content_length_, 0);
        VOUT(sent_size);
        if (sent_size < 0) {
            throw http_error("failed to send data to CGI script", HTTP::STATUS_INTERNAL_SERVER_ERROR);
        }
    }
    observer.reserve_unset(this, IObserver::OT_WRITE);
    attr.sock->shutdown_write();
}

void CGI::perform_receiving(IObserver &observer) {
    DXOUT("CGI on Read");
    u8t buf[MAX_RECEIVE_SIZE];
    ssize_t received_size = attr.sock->receive(buf, MAX_RECEIVE_SIZE, 0);
    VOUT(received_size);
    if (received_size == 0) {
        DXOUT("sock closed?");
    }
    if (received_size < 0) {
        throw http_error("failed to receive data from CGI script", HTTP::STATUS_INTERNAL_SERVER_ERROR);
    }

    inject_bytestring(buf, buf + received_size);
    bool is_disconnected = (received_size == 0);
    if (is_disconnected) {
        // Read側の切断を検知
        observer.reserve_unset(this, IObserver::OT_WRITE);
        observer.reserve_unset(this, IObserver::OT_READ);
    }
    after_injection(is_disconnected);
    // content_response_.insert(content_response_.end(), buf, buf + received_size);
}

bool CGI::is_originatable() const {
    return !status.is_started;
}

bool CGI::is_origination_started() const {
    return status.is_started;
}

bool CGI::is_reroutable() const {
    // TODO: ちゃんと実装
    return false;
}

bool CGI::is_responsive() const {
    return status.is_complete;
}

void CGI::leave() {
    attr.observer->reserve_unhold(this);
}

CGI::t_parse_progress CGI::reach_headers_end(size_t len, bool is_disconnected) {
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
    IndexRange res = ParserHelper::find_crlf(bytebuffer, this->mid, len);
    this->mid      = res.second;
    if (res.is_invalid()) {
        return PP_UNREACHED;
    }
    if (this->ps.crlf_in_header.second != res.first) {
        // はずれ: CRLFとCRLFの間が空いていた
        this->ps.crlf_in_header = res;
        return PP_HEADER_SECTION_END;
    }
    // あたり: ヘッダの終わりが見つかった
    analyze_headers(res);
    return PP_BODY;
}

void CGI::analyze_headers(IndexRange res) {
    this->ps.end_of_header = res.first;
    this->ps.start_of_body = res.second;
    DXOUT("DETECTED END of HEADER: " << this->ps.end_of_header);
    // -> [start_of_header, end_of_header) を解析する
    VOUT(this->ps.start_of_header);
    const light_string header_lines(bytebuffer, this->ps.start_of_header, this->ps.end_of_header);
    DXOUT("```" << std::endl << header_lines << std::endl << "```");
    parse_header_lines(header_lines, &this->header_holder);
    extract_control_headers();
    VOUT(this->ps.end_of_header);
    VOUT(this->ps.start_of_body);
}

void CGI::after_injection(bool is_disconnected) {
    size_t len;
    t_parse_progress flow;
    do {
        len = bytebuffer.size() - this->mid;
        switch (this->ps.parse_progress) {
            case PP_HEADER_SECTION_END: {
                DXOUT("CGI PP_HEADER_SECTION_END");
                flow = reach_headers_end(len, is_disconnected);
                if (flow == PP_UNREACHED) {
                    return;
                }
                this->ps.parse_progress = flow;
                continue;
            }

            case PP_BODY: {
                DXOUT("CGI PP_BODY");
                flow = reach_fixed_body_end(len, is_disconnected);
                if (flow == PP_UNREACHED) {
                    return;
                }
                this->ps.parse_progress = flow;
                continue;
            }

            case PP_OVER: {
                // 仮: ここでルーティングを開始
                DXOUT("CGI PP_OVER");
                status.is_complete = true;
                DXOUT("bytebuffer:" << std::endl << "```" << std::endl << bytebuffer << std::endl << "```");
                return;
            }

            default:
                throw http_error("not implemented yet", HTTP::STATUS_NOT_IMPLEMENTED);
        }
    } while (len > 0);
}

void CGI::extract_control_headers() {
    // 取得したヘッダから制御用の情報を抽出する.
    this->rp.determine_content_type(header_holder);
    this->rp.determine_body_size(header_holder);
    this->rp.determine_status(header_holder);
    this->rp.determine_location(header_holder);
}

CGI::t_parse_progress CGI::reach_fixed_body_end(size_t len, bool is_disconnected) {
    // - 接続が切れていたら
    //   -> ここまでをbodyとする
    // - content-lengthが不定なら
    //   -> 続行
    // - 受信済みサイズが content-length を下回っていたら
    //   -> 続行
    // - (else) 受信済みサイズが content-length 以上なら
    //   -> 受信済みサイズ = this->mid - start_of_body が content-length になるよう調整
    this->mid += len;
    if (!is_disconnected && parsed_body_size() < this->rp.body_size) {
        return PP_UNREACHED;
    }
    this->ps.end_of_body = this->rp.body_size + this->ps.start_of_body;
    this->mid            = this->ps.end_of_body;
    DXOUT("[" << this->ps.start_of_body << "," << this->ps.end_of_body << ")");
    return PP_OVER;
}

void CGI::RoutingParameters::determine_body_size(const header_holder_type &holder) {
    // https://datatracker.ietf.org/doc/html/rfc7230#section-3.3.3

    const byte_string *cl = holder.get_val(HeaderHTTP::content_length);
    VOUT(cl);
    // Transfer-Encodingがなく, Content-Lengthが正当である場合
    // -> Content-Length の値がボディの長さとなる.
    if (cl) {
        VOUT(*cl);
        body_size = ParserHelper::stou(*cl);
        // content-length の値が妥当でない場合, ここで例外が飛ぶ
    } else {
        // ボディの長さは0.
        body_size = 0;
    }
    VOUT(body_size);
}

void CGI::RoutingParameters::determine_content_type(const header_holder_type &holder) {
    // https://datatracker.ietf.org/doc/html/rfc3875#section-4.1.3
    // If the request includes a message-body, the CONTENT_TYPE variable is
    // set to the Internet Media Type [6] of the message-body.

    //     CONTENT_TYPE = "" | media-type
    //     media-type   = type "/" subtype *( ";" parameter )
    //     type         = token
    //     subtype      = token
    //     parameter    = attribute "=" value
    //     attribute    = token
    //     value        = token | quoted-string

    // The type, subtype and parameter attribute names are not
    // case-sensitive.  Parameter values may be case sensitive.  Media types
    // and their use in HTTP are described section 3.7 of the HTTP/1.1
    // specification [4].

    // HTTPにはOWSがあるが, CGIにはない.
    // -> OWSがあるのはparameter部分だけなので, そこに差異を押し込められそう.

    const byte_string *ct = holder.get_val(HeaderHTTP::content_type);
    if (!ct || ct->size() == 0) {
        content_type.value = HTTP::CH::ContentType::default_value;
        return;
    }
    const light_string lct(*ct);
    // [`type`の捕捉]
    // type = token = 1*tchar
    light_string::size_type type_end = lct.find_first_not_of(HTTP::CharFilter::tchar);
    if (type_end == light_string::npos) {
        DXOUT("[KO] no /: \"" << lct << "\"");
        return;
    }
    // [`/`の捕捉]
    if (lct[type_end] != '/') {
        DXOUT("[KO] not separated by /: \"" << lct << "\"");
        return;
    }
    light_string type_str(lct, 0, type_end);
    // [`subtype`の捕捉]
    // subtype = token = 1*char
    light_string::size_type subtype_end = lct.find_first_not_of(HTTP::CharFilter::tchar, type_end + 1);
    light_string subtype_str(lct, type_end + 1, subtype_end);
    if (subtype_str.size() == 0) {
        DXOUT("[KO] no subtype after /: \"" << lct << "\"");
        return;
    }

    if (subtype_end == light_string::npos) {
        return;
    }
    content_type.value = lct.substr(0, subtype_end).str();
    DXOUT("content_type.value: " << content_type.value);
    // [`parameter`の捕捉]
    // parameter  = 1*tchar "=" ( 1*tchar / quoted-string )
    light_string parameters_str(lct, subtype_end);
    QVOUT(parameters_str);
    light_string continuation = decompose_semicoron_separated_kvlist(parameters_str, content_type);
    DXOUT("parameter: " << content_type.parameters.size());
    QVOUT(continuation);
}

void CGI::RoutingParameters::determine_status(const header_holder_type &holder) {
    // https://datatracker.ietf.org/doc/html/rfc3875#section-6.3.3
    // The Status header field contains a 3-digit integer result code that
    // indicates the level of success of the script's attempt to handle the
    // request.

    //     Status         = "Status:" status-code SP reason-phrase NL
    //     status-code    = "200" | "302" | "400" | "501" | extension-code
    //     extension-code = 3digit
    //     reason-phrase  = *TEXT
    //     TEXT           = <any printable character>

    const byte_string *ct = holder.get_val(HeaderHTTP::status);
    if (!ct || ct->size() == 0) {
        status.code = HTTP::STATUS_UNSPECIFIED;
        return;
    }
    light_string lct(*ct);
    // `status-code`の捕捉
    const light_string code = lct.substr_while(HTTP::CharFilter::digit);
    if (code.size() != 3) {
        QVOUT(code);
        throw http_error("invalid status code", HTTP::STATUS_INTERNAL_SERVER_ERROR);
    }
    std::stringstream ss;
    ss << code.str();
    int scode;
    ss >> scode;
    status.code = scode;
    lct         = lct.substr(code.size());
    // `sp`の捕捉
    // 1つ以上を許容しておく
    const light_string sps = lct.substr_while(HTTP::CharFilter::cgi_sp);
    if (sps.size() == 0) {
        throw http_error("not enough spaces", HTTP::STATUS_INTERNAL_SERVER_ERROR);
    }
    lct = lct.substr(sps.size());
    // reason-phraseの捕捉
    lct = lct.rtrim(HTTP::CharFilter::sp);
    if (lct.substr_while(HTTP::CharFilter::printables).size() != lct.size()) {
        QVOUT(lct);
        throw http_error("invalid reason-phrase", HTTP::STATUS_INTERNAL_SERVER_ERROR);
    }
    status.reason = lct.str();
}

void CGI::RoutingParameters::determine_location(const header_holder_type &holder) {
    // https://datatracker.ietf.org/doc/html/rfc3875#section-6.3.2
    // The Location header field is used to specify to the server that the
    // script is returning a reference to a document rather than an actual
    // document (see sections 6.2.3 and 6.2.4).  It is either an absolute
    // URI (optionally with a fragment identifier), indicating that the
    // client is to fetch the referenced document, or a local URI path
    // (optionally with a query string), indicating that the server is to
    // fetch the referenced document and return it to the client as the
    // response.

    //     Location        = local-Location | client-Location
    //     client-Location = "Location:" fragment-URI NL
    //     fragment-URI    = absoluteURI [ "#" fragment ]
    //     fragment        = *uric
    //     local-Location  = "Location:" local-pathquery NL
    //     local-pathquery = abs-path [ "?" query-string ]
    //     abs-path        = "/" path-segments
    //     path-segments   = segment *( "/" segment )
    //     segment         = *pchar
    //     pchar           = unreserved | escaped | extra
    //     extra           = ":" | "@" | "&" | "=" | "+" | "$" | ","
    //     escaped         = "%" hex hex

    const byte_string *ct = holder.get_val(HeaderHTTP::location);
    if (!ct || ct->size() == 0) {
        location.value.clear();
        location.is_local = false;
        return;
    }
    location.value = *ct;
    light_string lct(*ct);
    if (lct[0] == '/') {
        // local-Location?
        // `local-pathquery`
        // = abs-path [ "?" query-string ]
        // = "/" path-segments [ "?" *uric ]
        // = "/" segment *( "/" segment ) [ "?" *(reserved | unreserved | escaped) ]
        // = "/" *(unreserved | escaped | extra) *( "/" *(unreserved | escaped | extra) ) [ "?" *(reserved | unreserved
        // | escaped) ]

        // `abs-path`と`query_string`(あれば)に分ける
        const light_string abs_path = lct.substr_before("?");
        const light_string query_string
            = abs_path.size() < lct.size() ? lct.substr(abs_path.size() + 1) : HTTP::strfy("");
        QVOUT(abs_path);
        // `abs_path`のvalidityを検証する
        {
            const HTTP::CharFilter unreserved_extra = HTTP::CharFilter::cgi_unreserved | HTTP::CharFilter::cgi_extra;
            std::vector<light_string> segments      = abs_path.split("/");
            if (!HTTP::Validator::is_uri_path(abs_path, unreserved_extra)) {
                throw http_error("invalid segment", HTTP::STATUS_INTERNAL_SERVER_ERROR);
            }
            location.abs_path = abs_path;
        }
        QVOUT(query_string);
        // `query_string`のvalidityを検証する
        {
            const HTTP::CharFilter unreserved_reserved
                = HTTP::CharFilter::cgi_unreserved | HTTP::CharFilter::cgi_reserved;
            if (!HTTP::Validator::is_segment(query_string, unreserved_reserved)) {
                throw http_error("invalid query_string", HTTP::STATUS_INTERNAL_SERVER_ERROR);
            }
            DXOUT("query_string is valid");
            location.query_string = query_string;
        }

    } else {
        // client-Location?
        // `fragment-URI`
        // = absoluteURI [ "#" fragment ]
        // absolute-URI  = scheme ":" hier-part [ "?" query ]

        // `scheme`の捕捉
        // scheme        = ALPHA *( ALPHA / DIGIT / "+" / "-" / "." )
        light_string tmp;
        HTTP::CharFilter ftr_scheme = HTTP::CharFilter::alpha | HTTP::CharFilter::digit | "+-.";
        const light_string scheme   = lct.substr_while(ftr_scheme);
        if (scheme.size() < 1 || !HTTP::CharFilter::alpha.includes(scheme[0])) {
            QVOUT(scheme);
            throw http_error("invalid scheme", HTTP::STATUS_INTERNAL_SERVER_ERROR);
        }
        // `scheme`の直後が`:`であることの確認
        if (scheme.size() >= lct.size() || lct[scheme.size()] != ':') {
            throw http_error("invalid scheme separator", HTTP::STATUS_INTERNAL_SERVER_ERROR);
        }
        // `hier-part`の捕捉
        // hier-part   = "//" authority path-abempty
        //     / path-absolute
        //     / path-rootless
        //     / path-empty
        tmp                                 = lct.substr(scheme.size() + 1);
        const light_string bang_fragment    = tmp.substr_from("#");
        const light_string fragment         = bang_fragment.substr(1);
        const light_string without_fragment = tmp.substr_before("#");
        const light_string question_query   = without_fragment.substr_from("?");
        const light_string query            = question_query.substr(1);
        const light_string hier_part        = without_fragment.substr_before("?");
        QVOUT(hier_part);
        QVOUT(query);
        QVOUT(fragment);
        const HTTP::CharFilter unreserved_extra = HTTP::CharFilter::cgi_unreserved | HTTP::CharFilter::cgi_extra;
        if (hier_part.size() >= 2 && hier_part.substr(0, 2) == "//") {
            // hier-part    = "//" authority path-abempty
            // authority    = [ userinfo "@" ] host [ ":" port ]
            // host         = IP-literal / IPv4address / reg-name
            // port         = *DIGIT
            light_string rest = hier_part.substr(2);
            QVOUT(rest);
            const light_string authority = rest.substr_before("/");
            QVOUT(authority);
            if (!HTTP::Validator::is_uri_authority(authority)) {
                throw http_error("invalid authority", HTTP::STATUS_INTERNAL_SERVER_ERROR);
            }
            location.authority = authority;
            // validate `path_abempty`
            // path-abempty = *( "/" segment )
            // segment      = *(unreserved | escaped | extra)
            const light_string path_abempty = rest.substr(authority.size());
            QVOUT(path_abempty);
            {
                if (!HTTP::Validator::is_uri_path(path_abempty, unreserved_extra)) {
                    throw http_error("invalid path-abempty", HTTP::STATUS_INTERNAL_SERVER_ERROR);
                }
                DXOUT("hier_part is path-abempty");
            }
        } else if (hier_part.size() > 0) {
            // hier-part     = path-absolute | path-rootless
            // path-absolute = "/" [ segment-nz *( "/" segment ) ]
            // segment-nz    = 1*pchar
            // (segment-nz は長さ1以上のsegment)
            // hier-part   = path-rootless
            // path-rootless = segment-nz *( "/" segment )
            light_string tmp = hier_part[0] == '/' ? hier_part.substr(1) : hier_part;
            if (tmp.substr_before("/").size() == 0) { // 実はチェックしなくていい ("//"で始まってないはずなので)
                throw http_error("first segment is empty", HTTP::STATUS_INTERNAL_SERVER_ERROR);
            }
            if (!HTTP::Validator::is_uri_path(tmp, unreserved_extra)) {
                throw http_error("invalid path-absolute or path-rootless", HTTP::STATUS_INTERNAL_SERVER_ERROR);
            }
            DXOUT("hier_part is path-absolute or path-rootless");
        } else {
            // hier-part   = path-empty
            // path-empty = "" (空文字列)
            DXOUT("hier_part is path-empty");
        }

        const HTTP::CharFilter uric = HTTP::CharFilter::cgi_reserved | HTTP::CharFilter::cgi_unreserved;
        // `query`の捕捉
        {
            //     QUERY_STRING    = query-string
            //     query-string    = *uric
            //     uric            = reserved | unreserved | escaped
            if (!HTTP::Validator::is_segment(query, uric)) {
                throw http_error("invalid query", HTTP::STATUS_INTERNAL_SERVER_ERROR);
            }
            DXOUT("query is valid");
            location.query_string = query;
        }

        // `fragment`の捕捉
        {
            // fragment = *(reserved | unreserved | escaped)
            if (!HTTP::Validator::is_segment(fragment, uric)) {
                throw http_error("invalid fragment", HTTP::STATUS_INTERNAL_SERVER_ERROR);
            }
            DXOUT("fragment is valid");
            location.fragment = fragment;
        }
    }
    QVOUT(location.value);
    VOUT(location.is_local);
    QVOUT(location.abs_path);
    QVOUT(location.fragment);
    QVOUT(location.query_string);
}

size_t CGI::parsed_body_size() const {
    return this->mid - this->ps.start_of_body;
}

void CGI::parse_header_lines(const light_string &lines, header_holder_type *holder) const {
    light_string rest(lines);
    while (true) {
        QVOUT(rest);
        const IndexRange res = ParserHelper::find_crlf_header_value(rest);
        if (res.is_invalid()) {
            break;
        }
        const light_string header_line = rest.substr(0, res.first);
        QVOUT(header_line);
        if (header_line.length() > 0) {
            // header_line が空文字列でない
            // -> ヘッダ行としてパースを試みる
            parse_header_line(header_line, holder);
        }
        rest = rest.substr(res.second);
    }
}

void CGI::parse_header_line(const light_string &line, header_holder_type *holder) const {

    const light_string key = line.substr_before(ParserHelper::HEADER_KV_SPLITTER);
    QVOUT(line);
    QVOUT(key);
    if (key.length() == line.length()) {
        // [!] Apache は : が含まれず空白から始まらない行がヘッダー部にあると、 400 応答を返します。 nginx
        // は無視して処理を続けます。
        throw http_error("no coron in a header line", HTTP::STATUS_BAD_REQUEST);
    }
    // ":"があった -> ":"の前後をキーとバリューにする
    if (key.length() == 0) {
        throw http_error("header key is empty", HTTP::STATUS_BAD_REQUEST);
    }
    light_string val = line.substr(key.length() + 1);
    // [!] 欄名と : の間には空白は認められていません。 鯖は、空白がある場合 400 応答を返して拒絶しなければなりません。
    // 串は、下流に転送する前に空白を削除しなければなりません。
    light_string::size_type key_tail = key.find_last_not_of(ParserHelper::OWS);
    if (key_tail + 1 != key.length()) {
        throw http_error("trailing space on header key", HTTP::STATUS_BAD_REQUEST);
    }
    // [!] 欄値の前後の OWS は、欄値の一部ではなく、 構文解析の際に削除します
    val = val.trim(ParserHelper::OWS);
    // holder があるなら holder に渡す. あとの処理は holder に任せる.
    if (holder != NULL) {
        holder->add_item(key, val);
    } else {
        DXOUT("no holder");
        QVOUT(key);
        QVOUT(val);
    }
}
