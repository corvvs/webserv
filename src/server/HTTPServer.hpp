#ifndef HTTPSERVER_HPP
#define HTTPSERVER_HPP
#include "../Interfaces.hpp"
#include "../Originators.hpp"
#include "../communication/Channel.hpp"
#include "../config/Config.hpp"
#include "../utils/FileCacher.hpp"
#include <map>

class MockMatcher : public IRequestMatcher {
public:
    RequestMatchingResult request_match(const std::vector<config::Config> &configs,
                                        const IRequestMatchingParam &request);
};

// [サーバクラス]
// [責務]
// - TODO: confファイルを読み取り解釈すること
// - (TODO: confファイルに基づいて)リスニングソケットを生成すること
// - ソケット監視処理を起動すること
// - TODO: リクエストを適切にルーティングしてレスポンスを生成すること
class HTTPServer : public IRouter {
public:
    typedef std::map<Channel::t_channel_id, Channel *> channel_map;
    FileCacher cacher_;

private:
    IObserver *socket_observer_;
    channel_map channels;
    config::config_dict configs_;

    MockMatcher mock_matcher;

public:
    HTTPServer(IObserver *observer);

    ~HTTPServer();

    void init(const std::string &config_path);

    // ソケットlisten開始
    // ほんとはconfに基づいてやる

    // イベントループ開始
    void run();

    RequestMatchingResult route(const IRequestMatchingParam &request, const config::config_vector &configs);

private:
    void listen(t_socket_domain sdomain,
                t_socket_type stype,
                t_port port,
                const config::config_vector &configs,
                FileCacher &cacher);
};

#endif
