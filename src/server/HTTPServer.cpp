#include "HTTPServer.hpp"

RequestMatchingResult MockMatcher::request_match(const std::vector<config::Config> &configs,
                                                 const IRequestMatchingParam &param) {
    (void)configs;
    RequestMatchingResult result;
    const HTTP::light_string &path  = param.get_request_target().path;
    HTTP::light_string::size_type i = path.rfind(".rb");
    if (i != HTTP::light_string::npos && i + strlen(".rb") == path.size()) {
        result.result_type = RequestMatchingResult::RT_CGI;
    } else if (path.size() > 0 && path[path.size() - 1] == '/') {
        result.result_type = RequestMatchingResult::RT_AUTO_INDEX;
    } else {
        result.result_type = RequestMatchingResult::RT_FILE;
    }
    result.target            = &param.get_request_target();
    result.path_local        = HTTP::strfy(".") + param.get_request_target().path.str();
    result.path_after        = HTTP::strfy("");
    result.path_cgi_executor = HTTP::strfy("/usr/bin/ruby");
    result.status_code       = HTTP::STATUS_MOVED_PERMANENTLY;
    result.redirect_location = HTTP::strfy("/mmmmm");
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

    // Connectionに紐づくserver群に対し, リクエストを渡してマッチングを要請.
    // その結果を使ってここでオリジネータを生成する.
    std::vector<config::Config> confs;
    RequestMatchingResult result = mock_matcher.request_match(confs, request.get_request_matching_param());
    switch (result.result_type) {
        case RequestMatchingResult::RT_CGI:
            return new CGI(result, request);
        case RequestMatchingResult::RT_FILE_DELETE:
            return new FileDeleter(result);
        case RequestMatchingResult::RT_FILE_PUT:
            return new FileWriter(result, request.get_plain_message());
        case RequestMatchingResult::RT_AUTO_INDEX:
            return new AutoIndexer(result);
        case RequestMatchingResult::RT_EXTERNAL_REDIRECTION:
            return new Redirector(result);
        case RequestMatchingResult::RT_ECHO:
            return new Echoer(result);
        case RequestMatchingResult::RT_FILE:
            return new FileReader(result);
        default:
            break;
    }
    return NULL;
}
