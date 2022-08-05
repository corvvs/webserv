#include "HTTPServer.hpp"
#include "../config/File.hpp"
#include "../config/Parser.hpp"

HTTPServer::HTTPServer(IObserver *observer) : socket_observer_(observer) {}

HTTPServer::~HTTPServer() {
    delete socket_observer_;
}

void HTTPServer::init(const std::string &config_path) {
    file::ErrorType err;
    if ((err = file::check(config_path)) != file::NONE) {
        throw std::runtime_error(file::error_message(err));
    }

    config::Parser parser;
    const config::config_dict &configs = parser.parse(file::read(config_path));

    for (config::config_dict::const_iterator it = configs.begin(); it != configs.end(); ++it) {
        const config::host_port_pair &hp     = it->first;
        const config::config_vector &configs = it->second;
        this->listen(SD_IP4, ST_TCP, hp.second, configs);
    }
}

void HTTPServer::listen(t_socket_domain sdomain,
                        t_socket_type stype,
                        t_port port,
                        const config::config_vector &configs) {
    Channel *ch            = new Channel(this, sdomain, stype, port, configs);
    channels[ch->get_id()] = ch;
    socket_observer_->reserve_hold(ch);
    socket_observer_->reserve_set(ch, IObserver::OT_READ);
}

void HTTPServer::run() {
    socket_observer_->loop();
}

IOriginator *HTTPServer::route(const RequestHTTP &request, const config::config_vector &configs) {
    // 1. リクエストを見て, 要求されているリソースがなんなのかを特定する
    // 2. ↑の特定結果をもとにオリジネータを作る

    // TODO:
    //    RequestMatchingResult result = mock_matcher.request_match(configs, request.get_request_matching_param());
    (void)request;
    (void)configs;
    {
        CGI::byte_string script_path  = HTTP::strfy("./cgi.rb");
        CGI::byte_string query_string = HTTP::strfy("");
        CGI *o                        = new CGI(script_path, query_string, request);
        return o;
    }
    // return new FileWriter(HTTP::strfy("./write_test"), request->get_plain_message());
    // return new FileReader("./hat.png");
    // return new Echoer(*request);
}
