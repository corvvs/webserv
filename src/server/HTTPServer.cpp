#include "HTTPServer.hpp"

HTTPServer::HTTPServer(IObserver *observer) : socket_observer_(observer) {}

HTTPServer::~HTTPServer() {
    delete socket_observer_;
}

void HTTPServer::listen(t_socket_domain sdomain, t_socket_type stype, t_port port) {
    Channel *ch            = new Channel(this, sdomain, stype, port);
    channels[ch->get_id()] = ch;
    socket_observer_->reserve_set(ch, IObserver::OT_READ);
}

void HTTPServer::run() {
    socket_observer_->loop();
}

IOriginator *HTTPServer::route_origin(RequestHTTP *request) {
    (void)request;
    return NULL;
}

ResponseHTTP *HTTPServer::route(RequestHTTP *request) {
    ResponseHTTP *res = new ResponseHTTP(request->get_http_version(), HTTP::STATUS_OK);

    res->feed_body(request->get_body());
    res->render();
    DXOUT(res->get_message_text());
    return res;
}

ResponseHTTP *HTTPServer::respond_error(RequestHTTP *request, http_error error) {
    (void)request;
    ResponseHTTP *res = new ResponseHTTP(HTTP::DEFAULT_HTTP_VERSION, error);
    res->render();
    DXOUT(res->get_message_text());
    return res;
}
