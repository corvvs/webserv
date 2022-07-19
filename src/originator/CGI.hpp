#ifndef CGI_HPP
#define CGI_HPP
#include "../Interfaces.hpp"
#include "../communication/HeaderHTTP.hpp"
#include "../communication/RequestHTTP.hpp"
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

    typedef HTTP::byte_string byte_string;
    typedef HTTP::light_string light_string;
    typedef byte_string::size_type size_type;
    typedef std::map<byte_string, byte_string> metavar_dict_type;
    typedef HeaderHolderCGI header_holder_type;

    struct Attribute {
        const byte_string script_path_;
        const byte_string query_string_;
        IObserver *observer;
        ISocketLike *master;
        // CGIスクリプトプロセスのPID
        pid_t cgi_pid;
        // CGIスクリプトプロセスとの通信に使うUNIXドメインソケットオブジェクト
        SocketUNIX *sock;
    };

    struct Status {
        bool is_started;
        // 送信済みcontent_requestのバイト数
        size_type content_sent;
        bool is_complete;
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

        // いろいろ抽出関数群

        void determine_body_size(const header_holder_type &holder);
        void determine_content_type(const header_holder_type &holder);
        void determine_status(const header_holder_type &holder);
        void determine_location(const header_holder_type &holder);
    };

private:
    Attribute attr;
    metavar_dict_type metavar_;
    size_type content_length_;
    byte_string content_request_;
    byte_string content_response_;

    // bool is_disconnected;
    byte_string bytebuffer;
    ssize_t mid;
    ParserStatus ps;
    RoutingParameters rp;

    Status status;

    // [CGIヘッダ]
    header_holder_type header_holder;

    static char **flatten_argv(const byte_string &script_path);
    static char **flatten_metavar(const metavar_dict_type &metavar);

    void perform_receiving(IObserver &observer);
    void perform_sending(IObserver &observer);
    // (ISocketLikeのメソッドではない)
    // 自分に来たイベント通知を, 他のISocketLikeに飛ばす.
    // その際イベント通知に変換を施すかもしれない.
    void retransmit(IObserver &observer, IObserver::observation_category cat, t_time_epoch_ms epoch);

    t_parse_progress reach_headers_end(size_t len, bool is_disconnected);
    t_parse_progress reach_fixed_body_end(size_t len, bool is_disconnected);
    void analyze_headers(IndexRange res);

    void parse_header_lines(const light_string &lines, header_holder_type *holder) const;
    void parse_header_line(const light_string &line, header_holder_type *holder) const;
    void extract_control_headers();

public:
    CGI(const byte_string &script_path,
        const byte_string &query_string,
        metavar_dict_type &metavar,
        size_type content_length);
    ~CGI();

    // CGIスクリプトに送信するためのデータを投入する
    void set_content(const byte_string &content);

    t_fd get_fd() const;
    void notify(IObserver &observer, IObserver::observation_category cat, t_time_epoch_ms epoch);

    void inject_socketlike(ISocketLike *socket_like);
    bool is_originatable() const;
    bool is_reroutable() const;
    bool is_responsive() const;
    bool is_origination_started() const;
    void start_origination(IObserver &observer);
    byte_string draw_data() const;
    void leave();

    // 内部バッファにバイト列を追加する
    template <class InputItr>
    void inject_bytestring(const InputItr begin, const InputItr end) {
        bytebuffer.insert(bytebuffer.end(), begin, end);
    }
    void after_injection(bool is_disconnected);

    size_t parsed_body_size() const;

    // 環境変数`envp`からmetavar_dict_typeを作る
    static metavar_dict_type make_metavars_from_envp(char **envp);
};

#endif
