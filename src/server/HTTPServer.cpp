#include "HTTPServer.hpp"

HTTPServer::HTTPServer(IObserver *observer) : socket_observer_(observer) {}

HTTPServer::~HTTPServer() {
    delete socket_observer_;
}

void HTTPServer::listen(t_socket_domain sdomain, t_socket_type stype, t_port port) {
    Channel *ch            = new Channel(this, sdomain, stype, port);
    channels[ch->get_id()] = ch;
    socket_observer_->reserve_hold(ch);
    socket_observer_->reserve_set(ch, IObserver::OT_READ);
}

void HTTPServer::run() {
    socket_observer_->loop();
}

IOriginator *HTTPServer::route(const RequestHTTP &request) {
    // 1. リクエストを見て, 要求されているリソースがなんなのかを特定する
    // 2. ↑の特定結果をもとにオリジネータを作る

    (void)request;
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
