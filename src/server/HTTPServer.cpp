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
    {
        CGI::metavar_dict_type metavar = request->get_cgi_http_vars();
        CGI::byte_string script_path   = HTTP::strfy("./cgi.rb");
        CGI::byte_string query_string  = HTTP::strfy("");
        CGI *o                         = new CGI(script_path, query_string, metavar, 0);
        o->set_content(request->get_plain_message());
        return o;
    }
    // return new FileWriter(HTTP::strfy("./write_test"), request->get_plain_message());
    // return new FileReader("./hat.png");
    // return new Echoer(*request);
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
