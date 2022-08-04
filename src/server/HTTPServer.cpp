#include "HTTPServer.hpp"

RequestMatchingResult MockMatcher::request_match(const RequestHTTP &request) {
    (void)request;
    RequestMatchingResult result;
    result.result_type       = RequestMatchingResult::RT_CGI;
    result.path_local        = HTTP::strfy("./cgi.rb");
    result.path_after        = HTTP::strfy("");
    result.path_cgi_executor = HTTP::strfy("/usr/bin/ruby");
    return result;
}

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

    (void)request;
    // Connectionに紐づくserver群に対し, リクエストを渡してマッチングを要請.
    // その結果を使ってここでオリジネータを生成する.
    RequestMatchingResult result = mock_matcher.request_match(request);
    // switch (result.result_type) {
    //     case RequestMatchingResult::RT_CGI: {
    //         CGI::byte_string query_string = HTTP::strfy("");
    //         CGI *o                        = new CGI(result, request);
    //         return o;
    //     }
    //     default:
    //         break;
    // }
    // return new FileWriter(HTTP::strfy("./write_test"), request.get_plain_message());
    // return new FileReader("./hat.png");
    // return new AutoIndexer(HTTP::strfy("./"));
    // return new Echoer();
    return NULL;
}
