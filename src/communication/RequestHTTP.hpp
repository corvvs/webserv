#ifndef REQUESTHTTP_HPP
#define REQUESTHTTP_HPP
#include "../utils/HTTPError.hpp"
#include "../utils/LightString.hpp"
#include "../utils/UtilsString.hpp"
#include "../utils/http.hpp"
#include "../utils/test_common.hpp"
#include "ChunkedBody.hpp"
#include "ControlHeaderHTTP.hpp"
#include "HeaderHTTP.hpp"
#include "ParserHelper.hpp"
#include "RoutingParameters.hpp"
#include "ValidatorHTTP.hpp"
#include <iostream>
#include <list>
#include <map>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

struct RequestTarget {
    typedef HTTP::light_string light_string;

    // リクエストの"form"
    // - origin-form: "/"から始まるパスだけのやつ
    // - authority-form: ドメイン部だけのやつ
    // - absolute-form: URLまるごと
    // - asterisk-form: "*"のみ
    enum t_form { FORM_UNKNOWN, FORM_ORIGIN, FORM_AUTHORITY, FORM_ABSOLUTE, FORM_ASTERISK };

    // 与えられたリクエストターゲット
    light_string given;
    // エラーかどうか
    bool is_error;
    // 何formか
    t_form form;

    // cf. https://datatracker.ietf.org/doc/html/rfc3986#section-3
    //
    //     foo://example.com:8042/over/there?name=ferret#nose
    //     \_/   \______________/\_________/ \_________/ \__/
    //     |           |            |            |        |
    // scheme     authority       path        query   fragment
    //     |   _____________________|__
    //     / \ /                        \
    //     urn:example:animal:ferret:nose

    light_string scheme;
    light_string authority;
    light_string path;
    light_string query;

    RequestTarget();
    RequestTarget(const light_string &target);
};

std::ostream &operator<<(std::ostream &ost, const RequestTarget &f);

class IRequestMatchingParam {
public:
    virtual ~IRequestMatchingParam() {}

    virtual const RequestTarget &get_request_target() const = 0;
    virtual HTTP::t_method get_http_method() const          = 0;
    virtual HTTP::t_version get_http_version() const        = 0;
    virtual const HTTP::CH::Host &get_host() const          = 0;
};

// [HTTPリクエストクラス]
// [責務]
// - 順次供給されるバイト列をHTTPリクエストとして解釈すること
class RequestHTTP {
public:
    enum t_parse_progress {
        PP_UNREACHED,
        // 開始行の開始位置 を探している
        PP_REQLINE_START,
        // 開始行の終了位置 を探している
        PP_REQLINE_END,
        // ヘッダの終了位置 を探している
        PP_HEADER_SECTION_END,
        // ボディ を探している
        PP_BODY,

        // チャンクのサイズ行の終了位置 を探している
        PP_CHUNK_SIZE_LINE_END,
        // チャンクの終了位置 を探している
        PP_CHUNK_DATA_END,
        // チャンクの終了直後の改行 を探している
        PP_CHUNK_DATA_CRLF,
        // チャンクのトレイラーフィールドの終了位置 を探している
        PP_TRAILER_FIELD_END,

        PP_OVER,
        PP_ERROR
    };

    typedef HTTP::byte_string byte_string;
    // 他の byte_string の一部分を参照する軽量 string
    typedef HTTP::light_string light_string;
    typedef std::map<byte_string, light_string> header_dict_type;
    typedef HeaderItem::header_val_type header_val_type;
    typedef HeaderHolderHTTP header_holder_type;

    struct ParserStatus {
        // ヘッダ終端探索時において, 最後に遭遇したCRLFのレンジ
        IndexRange crlf_in_header;
        // ヘッダ行解析において obs-fold に遭遇したかどうか
        bool found_obs_fold;

        t_parse_progress parse_progress;
        size_t start_of_reqline;
        size_t end_of_reqline;
        size_t start_of_header;
        size_t end_of_header;
        size_t start_of_body;
        size_t end_of_body;
        size_t start_of_current_chunk;
        size_t start_of_current_chunk_data;
        size_t start_of_trailer_field;
        size_t end_of_trailer_field;
        ChunkedBody::Chunk current_chunk;
        // 凍結されたかどうか
        bool is_freezed;

        ParserStatus();
    };

    // リクエストの制御, ルーティングにかかわるパラメータ
    struct RoutingParameters : public ARoutingParameters, public IRequestMatchingParam {
        // TODO: リクエストマッチングに必要なものを外部に公開する
        RequestTarget given_request_target; // リクエストマッチングに必要

        HTTP::t_method http_method; // リクエストマッチングに必要
        HTTP::t_version http_version;

        bool is_body_chunked;

        HTTP::CH::Host header_host; // リクエストマッチングに必要
        HTTP::CH::ContentType content_type;
        HTTP::CH::TransferEncoding transfer_encoding;
        HTTP::CH::Connection connection;
        HTTP::CH::TE te;
        HTTP::CH::Upgrade upgrade;
        HTTP::CH::Via via;

        // いろいろ抽出関数群

        // TODO: struct に結びつくやつは struct に移したほうがいいかも
        void determine_host(const header_holder_type &holder);
        // リクエストのボディサイズ(にかかわるパラメータ)を決定する
        void determine_body_size(const header_holder_type &holder);

        const RequestTarget &get_request_target() const;
        HTTP::t_method get_http_method() const;
        HTTP::t_version get_http_version() const;
        const HTTP::CH::Host &get_host() const;
    };

private:
    byte_string bytebuffer;
    ssize_t mid;

    // 解析中の情報
    ParserStatus ps;
    // ルーティングパラメータ
    RoutingParameters rp;

    // chunked本文
    ChunkedBody chunked_body;

    // [HTTPヘッダ]
    header_holder_type header_holder;

    // [フロー関数群]
    t_parse_progress reach_reqline_start(size_t len, bool is_disconnected);
    t_parse_progress reach_reqline_end(size_t len, bool is_disconnected);
    t_parse_progress reach_headers_end(size_t len, bool is_disconnected);
    t_parse_progress reach_fixed_body_end(size_t len, bool is_disconnected);
    t_parse_progress reach_chunked_size_line(size_t len, bool is_disconnected);
    t_parse_progress reach_chunked_data_end(size_t len, bool is_disconnected);
    t_parse_progress reach_chunked_data_termination(size_t len, bool is_disconnected);
    t_parse_progress reach_chunked_trailer_end(size_t len, bool is_disconnected);
    void analyze_headers(IndexRange res);

    // [パース関数群]

    // 開始行の終了位置を探す
    bool seek_reqline_end(size_t len);
    // [begin, end) を要求行としてパースする
    void parse_reqline(const light_string &line);
    // ヘッダ行全体をパースする
    void parse_header_lines(const light_string &lines, header_holder_type *holder) const;
    // ヘッダ行をパースする
    void parse_header_line(const light_string &line, header_holder_type *holder) const;

    void parse_chunk_size_line(const light_string &line);
    void parse_chunk_data(const light_string &data);

    // 要求行の整合性をチェック
    void check_reqline_consistensy();

    // ヘッダから必要な情報を取る
    void extract_control_headers();

public:
    RequestHTTP();
    ~RequestHTTP();

    // 内部バッファにバイト列を追加する
    template <class InputItr>
    void inject_bytestring(const InputItr begin, const InputItr end) {
        bytebuffer.insert(bytebuffer.end(), begin, end);
    }
    void after_injection(bool is_disconnected);

    // リクエストを凍結し, 余ったデータを返す
    light_string freeze();

    // 受信済み(未解釈含む)データサイズ
    size_t receipt_size() const;
    // 解釈済みボディサイズ
    size_t parsed_body_size() const;
    // 解釈済みデータサイズ
    size_t parsed_size() const;

    // リクエストのHTTPバージョン
    HTTP::t_version get_http_version() const;
    // リクエストのHTTPメソッド
    HTTP::t_method get_method() const;

    HTTP::byte_string get_content_type() const;

    // 受信したデータから本文を抽出して返す
    byte_string get_body() const;
    // HTTPメッセージ全文を返す
    byte_string get_plain_message() const;

    // predicate: ナビゲーション(ルーティング)できる状態になったかどうか
    bool is_routable() const;
    // predicate: レスポンスの受信が完了したかどうか
    bool is_complete() const;
    // predicate: レスポンスを凍結したかどうか
    bool is_freezed() const;
    // predicate: このリクエストに対するレスポンスを送り終わった後, 接続を維持すべきかどうか
    bool should_keep_in_touch() const;

    header_holder_type::joined_dict_type get_cgi_http_vars() const;
    const IRequestMatchingParam &get_request_mathing_param() const;
};

#endif
