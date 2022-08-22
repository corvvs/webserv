#include "HTTPServer.hpp"
#include "../config/Parser.hpp"
#include "../router/RequestMatcher.hpp"
#include "../utils/File.hpp"
#include "../utils/ObjectHolder.hpp"

HTTPServer::HTTPServer(IObserver *observer) : socket_observer_(observer) {}

HTTPServer::~HTTPServer() {
    delete socket_observer_;
}

int HTTPServer::test_configuration(const std::string &path) {
    file::ErrorType err;
    if ((err = file::check(path)) != file::NONE) {
        std::cerr << file::error_message(err) << std::endl;
        return EXIT_FAILURE;
    }

    config::Parser parser;
    try {
        parser.parse(file::read(path));
    } catch (const config::SyntaxError &err) {
        std::cout << err.what() << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "webserv: the configuration file " + path + " syntax is ok" << std::endl;
    return EXIT_SUCCESS;
}

void HTTPServer::init(const std::string &config_path) {
    file::ErrorType err;
    if ((err = file::check(config_path)) != file::NONE) {
        throw std::runtime_error(file::error_message(err));
    }

    config::Parser parser;
    configs_ = parser.parse(file::read(config_path));

    for (config::config_dict::const_iterator it = configs_.begin(); it != configs_.end(); ++it) {
        const config::host_port_pair &hp     = it->first;
        const config::config_vector &configs = it->second;
        this->listen(SD_IP4, ST_TCP, hp.second, configs, cacher_);
    }
}

void HTTPServer::listen(t_socket_domain sdomain,
                        t_socket_type stype,
                        t_port port,
                        const config::config_vector &configs,
                        FileCacher &cacher) {
    ObjectHolder<Channel> ch(new Channel(this, sdomain, stype, port, configs, cacher));
    socket_observer_->reserve_hold(ch.value());
    ch.waive();
    socket_observer_->reserve_set(ch.value(), IObserver::OT_READ);
}

void HTTPServer::run() {
    socket_observer_->loop();
}

RequestMatchingResult HTTPServer::route(const IRequestMatchingParam &matching_param,
                                        const config::config_vector &configs) {

    // Connectionに紐づくserver群に対し, リクエストを渡してマッチングを要請.
    // その結果を使ってここでオリジネータを生成する.
    RequestMatcher rm;
    return rm.request_match(configs, matching_param);
}
