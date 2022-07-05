#ifndef HTTPSERVER_HPP
#define HTTPSERVER_HPP
#include "../interface/IOriginator.hpp"
#include "../interface/IRouter.hpp"
#include "../communication/Channel.hpp"
#include "../interface/IObserver.hpp"
#include "../interface/ISocketlike.hpp"
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

public:
    HTTPServer(IObserver *observer);

    ~HTTPServer();

    // ソケットlisten開始
    // ほんとはconfに基づいてやる
    void listen(t_socket_domain sdomain, t_socket_type stype, t_port port);

    // イベントループ開始
    void run();

    IOriginator *route_origin(RequestHTTP *request);

    ResponseHTTP *route(RequestHTTP *request);

    ResponseHTTP *respond_error(RequestHTTP *request, http_error error);
};

#endif
