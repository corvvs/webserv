#ifndef CGI_HPP
#define CGI_HPP
#include "../Interfaces.hpp"
#include "../communication/HeaderHTTP.hpp"
#include "../communication/Lifetime.hpp"
#include "../communication/RequestHTTP.hpp"
#include "../communication/ResponseDataList.hpp"
#include "../communication/RoutingParameters.hpp"
#include "../socket/SocketUNIX.hpp"
#include "../utils/http.hpp"
#include <map>

class RoundTrip;

class CGI : public ISocketLike, public IOriginator {
public:
    enum t_parse_progress {
        PP_UNREACHED,
        // ヘッダの終了位置 を探している
        PP_HEADER_SECTION_END,
        // ボディ を探している
        PP_BODY,

        PP_OVER,
        PP_ERROR
    };

    enum t_cgi_response_type {
        CGIRES_UNKNOWN,
        // ドキュメント応答
        CGIRES_DOCUMENT,
        // ローカルリダイレクト応答
        CGIRES_REDIRECT_LOCAL,
        // クライアントリダイレクト応答
        CGIRES_REDIRECT_CLIENT,
        // ドキュメントつきクライアントリダイレクト応答
        CGIRES_REDIRECT_CLIENT_DOCUMENT
    };

    typedef HTTP::byte_string byte_string;
    typedef HTTP::light_string light_string;
    typedef byte_string::size_type size_type;
    typedef std::map<byte_string, byte_string> metavar_dict_type;
    typedef HeaderHolderCGI header_holder_type;

    struct Attribute {
        const ICGIConfigurationProvider &configuration_provider_;
        const byte_string executor_path_;
        const byte_string script_path_;
        const byte_string query_string_;
        IObserver *observer;
        ISocketLike *master;
        // CGIスクリプトプロセスのPID
        pid_t cgi_pid;
        // CGIスクリプトプロセスとの通信に使うUNIXドメインソケットオブジェクト
        SocketUNIX *sock;

        Attribute(const RequestMatchingResult &matching_result,
                  const ICGIConfigurationProvider &configuration_provider);
    };

    struct Status {
        bool is_started;
        // 送信済み content_request のバイト数
        size_type to_script_content_sent_;
        // レスポンス送信可能フラグ
        bool is_responsive;
        // オリジネーション完了フラグ
        bool is_complete;
        ResponseDataList response_data;

        Status();
    };

    struct ParserStatus {
        // ヘッダ終端探索時において, 最後に遭遇したCRLFのレンジ
        IndexRange crlf_in_header;

        t_parse_progress parse_progress;
        size_t start_of_header;
        size_t end_of_header;
        size_t start_of_body;
        size_t end_of_body;
        // 凍結されたかどうか
        bool is_freezed;

        ParserStatus();
    };

    struct RoutingParameters : public ARoutingParameters {

        CGIP::CH::ContentType content_type;
        CGIP::CH::Status status;
        CGIP::CH::Location location;
        CGIP::CH::SetCookie set_cookie;

        // いろいろ抽出関数群

        void determine_body_size(const header_holder_type &holder);

        // CGIレスポンスタイプの判定
        CGI::t_cgi_response_type get_response_type() const;
    };

    static const byte_string META_GATEWAY_INTERFACE;
    static const byte_string META_REQUEST_METHOD;
    static const byte_string META_SERVER_PROTOCOL;
    static const byte_string META_CONTENT_TYPE;
    static const byte_string META_SERVER_NAME;
    static const byte_string META_SERVER_PORT;
    static const byte_string META_CONTENT_LENGTH;
    static const byte_string META_PATH_INFO;
    static const byte_string META_SCRIPT_NAME;
    static const byte_string META_QUERY_STRING;
    static const byte_string META_REQUEST_URI;

private:
    bool leaving;
    Attribute attr;
    Lifetime lifetime;

    metavar_dict_type metavar_;
    size_type to_script_content_length_;
    byte_string to_script_content_;

    // bool is_disconnected;
    byte_string bytebuffer;
    ssize_t mid;
    ParserStatus ps;
    RoutingParameters rp;

    Status status;

    // [CGIヘッダ]
    header_holder_type from_script_header_holder;

    static char **flatten_argv(const byte_string &executor_path, const byte_string &script_path);
    static char **flatten_metavar(const metavar_dict_type &metavar);

    IResponseDataProducer &response_data_producer();

    // 「CGIスクリプトが実行可能であること」を確認する
    // (オリジネーション可能性とは関係ないことに注意)
    // - スクリプトのパスにファイルが存在すること
    // - 当該ファイルが実行可能ファイルであること(ディレクトリ等でないこと)
    // - 実行権限があること
    void check_executable() const;
    // もしCGIスクリプトが終了していたら, 子プロセスを waitpid で回収する。
    // (異常終了している場合は500を投げる)
    void capture_script_termination();

    void perform_receiving(IObserver &observer);
    void perform_sending(IObserver &observer);
    // (ISocketLikeのメソッドではない)
    // 自分に来たイベント通知を, 他のISocketLikeに飛ばす.
    // その際イベント通知に変換を施すかもしれない.
    void retransmit(IObserver &observer, IObserver::observation_category cat, t_time_epoch_ms epoch);

    t_parse_progress reach_headers_end(size_t len, bool is_disconnected);
    t_parse_progress reach_fixed_body_end(size_t len, bool is_disconnected);
    void analyze_headers(const IndexRange res);
    void extract_control_headers();

    // CGI応答の整合性をチェック
    // 不整合があるなら http_error 例外を飛ばす
    void check_cgi_response_consistensy();

    HTTP::t_status determine_response_status() const;
    // ※ `from_script_header_holder`に対して破壊的
    ResponseHTTP::header_list_type determine_response_headers(const IResponseDataConsumer::t_sending_mode sm) const;

public:
    CGI(const RequestMatchingResult &match_result, const ICGIConfigurationProvider &request);
    ~CGI();

    // CGIスクリプトに送信するためのデータを投入する
    void set_content(const byte_string &content);

    t_fd get_fd() const;
    t_port get_port() const;
    void notify(IObserver &observer, IObserver::observation_category cat, t_time_epoch_ms epoch);

    void inject_socketlike(ISocketLike *socket_like);
    bool is_originatable() const;
    bool is_reroutable() const;
    HTTP::byte_string reroute_path() const;
    bool is_responsive() const;
    bool is_origination_started() const;
    void start_origination(IObserver &observer);
    void leave();
    ResponseHTTP *respond(const RequestHTTP *request);
    bool is_timeout(t_time_epoch_ms now) const;

    // 内部バッファにバイト列を追加する
    template <class InputItr>
    void inject_bytestring(const InputItr begin, const InputItr end) {
        bytebuffer.insert(bytebuffer.end(), begin, end);
        // VOUT(bytebuffer.size());
        // BVOUT(bytebuffer);
    }

    void after_injection(bool is_disconnected);

    size_t parsed_body_size() const;

    // 環境変数`envp`からmetavar_dict_typeを作る
    static metavar_dict_type make_metavars_from_envp(char **envp);
};

#endif
