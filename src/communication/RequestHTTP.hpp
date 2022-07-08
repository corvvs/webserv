#ifndef REQUESTHTTP_HPP
#define REQUESTHTTP_HPP
#include "ChunkedBody.hpp"
#include "ControlHeaderHTTP.hpp"
#include "HeaderHTTP.hpp"
#include "../utils/LightString.hpp"
#include "ValidatorHTTP.hpp"
#include "../utils/http.hpp"
#include "../utils/HTTPError.hpp"
#include "ParserHelper.hpp"
#include "../utils/test_common.hpp"
#include "../utils/UtilsString.hpp"
#include <iostream>
#include <list>
#include <map>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

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

    static const size_t MAX_REQLINE_END = 8192;
    typedef HTTP::byte_string byte_string;
    // 他の byte_string の一部分を参照する軽量 string
    typedef LightString<HTTP::char_type> light_string;
    typedef std::map<byte_string, light_string> header_dict_type;
    typedef HeaderHTTPItem::header_val_type header_val_type;

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

        ParserStatus();
    };

    // リクエストの制御, ルーティングにかかわるパラメータ
    struct ControlParams {
        light_string request_path;
        HTTP::t_method http_method;
        HTTP::t_version http_version;

        size_t body_size;
        bool is_body_chunked;

        HTTP::CH::Host header_host;
        HTTP::CH::ContentType content_type;
        HTTP::CH::TransferEncoding transfer_encoding;
        HTTP::CH::Connection connection;
        HTTP::CH::TE te;
        HTTP::CH::Upgrade upgrade;
        HTTP::CH::Via via;

        // いろいろ抽出関数群

        // TODO: struct に結びつくやつは struct に移したほうがいいかも
        void determine_host(const HeaderHTTPHolder &holder);
        void determine_transfer_encoding(const HeaderHTTPHolder &holder);
        // リクエストのボディサイズ(にかかわるパラメータ)を決定する
        void determine_body_size(const HeaderHTTPHolder &holder);
        void determine_content_type(const HeaderHTTPHolder &holder);
        void determine_connection(const HeaderHTTPHolder &holder);
        void determine_te(const HeaderHTTPHolder &holder);
        void determine_upgrade(const HeaderHTTPHolder &holder);
        void determine_via(const HeaderHTTPHolder &holder);

        // `lhost`の中身を`Host`構造体に詰める.
        // `lhost`はHost:ヘッダとしてvalidでなければならない.
        void pack_host(HTTP::Term::Host &host_item, const light_string &lhost);

        // "セミコロン分割key-valueリスト" をパースして辞書に詰める
        // その後, パースできた部分以降を返す
        light_string decompose_semicoron_separated_kvlist(const light_string &list_str, HTTP::IDictHolder &holder);

        // val_lstr のコメント部分を抽出して返す.
        // val_lstr はコメント部分を通過した状態になる.
        light_string extract_comment(light_string &val_lstr);

        // HTTP における`comment`を取り出す.
        light_string extract_comment(const light_string &str);
    };

private:
    byte_string bytebuffer;
    ssize_t mid;

    // 解析中の情報
    ParserStatus ps;

    // 確定した情報
    // 制御パラメータ
    ControlParams cp;

    // chunked本文
    ChunkedBody chunked_body;

    // [HTTPヘッダ]
    HeaderHTTPHolder header_holder;

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
    void parse_header_lines(const light_string &lines, HeaderHTTPHolder *holder) const;
    // ヘッダ行をパースする
    void parse_header_line(const light_string &line, HeaderHTTPHolder *holder) const;

    void parse_chunk_size_line(const light_string &line);
    void parse_chunk_data(const light_string &data);

    // ヘッダから必要な情報を取る
    void extract_control_headers();

public:
    RequestHTTP();
    ~RequestHTTP();

    // 内部バッファにバイト列を追加し, ラフパースを試みる
    void feed_bytestring(char *bytes, size_t len);

    // 受信済み(未解釈含む)データサイズ
    size_t receipt_size() const;
    // 解釈済みボディサイズ
    size_t parsed_body_size() const;
    // 解釈済みデータサイズ
    size_t parsed_size() const;

    // リクエストのHTTPバージョン
    HTTP::t_version get_http_version() const;

    // 受信したデータから本文を抽出して返す
    byte_string get_body() const;

    // predicate: ナビゲーション(ルーティング)できる状態になったかどうか
    bool is_ready_to_navigate() const;
    // predicate: レスポンスを作成できる状態になったかどうか
    bool is_ready_to_respond() const;
    // predicate: このリクエストに対するレスポンスを送り終わった後, 接続を維持すべきかどうか
    bool should_keep_in_touch() const;
};

#endif
