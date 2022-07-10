#ifndef CGI_HPP
#define CGI_HPP
#include "../communication/RequestHTTP.hpp"
#include "../interface/IObserver.hpp"
#include "../interface/IOriginator.hpp"
#include "../interface/IRouter.hpp"
#include "../interface/ISocketlike.hpp"
#include "../socket/SocketUNIX.hpp"
#include "../utils/http.hpp"
#include <map>

class CGI : public ISocketLike /*, public IOriginator*/ {
public:
    typedef HTTP::byte_string byte_string;
    typedef HTTP::light_string light_string;
    typedef byte_string::size_type size_type;
    typedef std::map<byte_string, byte_string> metavar_dict_type;

    struct Status {
        // 送信済みcontent_requestのバイト数
        size_type content_sent;
    };

private:
    pid_t cgi_pid;
    SocketUNIX *sock;
    byte_string script_path_;
    byte_string query_string_;
    metavar_dict_type metavar_;
    size_type content_length_;
    byte_string content_request_;

    Status status;

    CGI();
    CGI(pid_t cgi_pid,
        SocketUNIX *sock,
        byte_string &script_path,
        byte_string &query_string,
        metavar_dict_type &metavar,
        size_type content_length);

    static char **flatten_argv(const byte_string &script_path);
    static char **flatten_metavar(const metavar_dict_type &metavar);

public:
    ~CGI();

    // 「CGIスクリプト」を起動し, 起動状態を管理するCGIオブジェクトを返す
    static CGI *start_cgi(byte_string &script_path,
                          byte_string &query_string,
                          metavar_dict_type &metavar,
                          size_type content_length);

    // CGIスクリプトに送信するためのデータを投入する
    void set_content(byte_string &content);

    t_fd get_fd() const;
    virtual void notify(IObserver &observer);
    virtual void timeout(IObserver &observer, t_time_epoch_ms epoch);

    // 環境変数`envp`からmetavar_dict_typeを作る
    static metavar_dict_type make_metavars_from_envp(char **envp);

    void send_content();
    void receive();
};

#endif
