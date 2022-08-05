#ifndef HTTPSERVER_HPP
#define HTTPSERVER_HPP
#include "../Interfaces.hpp"
#include "../Originators.hpp"
#include "../communication/Channel.hpp"
#include "../config/Config.hpp"
#include <map>

// [サーバクラス]
// [責務]
// - TODO: confファイルを読み取り解釈すること
// - (TODO: confファイルに基づいて)リスニングソケットを生成すること
// - ソケット監視処理を起動すること
// - TODO: リクエストを適切にルーティングしてレスポンスを生成すること
class HTTPServer : public IRouter {
public:
    typedef std::map<Channel::t_channel_id, Channel *> channel_map;

private:
    IObserver *socket_observer_;
    channel_map channels;
    config::config_dict configs_;

public:
    HTTPServer(IObserver *observer);

    ~HTTPServer();

    void init(const std::string &config_path);

    // ソケットlisten開始
    // ほんとはconfに基づいてやる

    // イベントループ開始
    void run();

private:
    void listen(t_socket_domain sdomain, t_socket_type stype, t_port port, const config::config_vector &configs);

    IOriginator *route(const RequestHTTP &request, const config::config_vector &configs);
};

#endif
