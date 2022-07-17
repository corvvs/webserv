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

IOriginator *HTTPServer::route_origin(const RequestHTTP *request) {
    (void)request;
    return NULL;
}

ResponseHTTP *HTTPServer::route(const RequestHTTP *request) {
    ResponseHTTP *res = new ResponseHTTP(request->get_http_version(), HTTP::STATUS_OK);
    return res;
}

ResponseHTTP *HTTPServer::respond_error(const RequestHTTP *request, http_error error) {
    (void)request;
    ResponseHTTP *res = new ResponseHTTP(HTTP::DEFAULT_HTTP_VERSION, error);
    res->render();
    DXOUT(res->get_message_text());
    return res;
}
