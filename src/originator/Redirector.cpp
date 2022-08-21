#include "Redirector.hpp"
#include <unistd.h>

Redirector::Redirector(const RequestMatchingResult &match_result)
    : originated_(true), redirect_to(match_result.redirect_location), status_code(match_result.status_code) {}

void Redirector::notify(IObserver &observer, IObserver::observation_category cat, t_time_epoch_ms epoch) {
    (void)observer;
    (void)cat;
    (void)epoch;
    assert(false);
}

void Redirector::inject_socketlike(ISocketLike *socket_like) {
    (void)socket_like;
}

bool Redirector::is_originatable() const {
    return !originated_;
}

bool Redirector::is_origination_started() const {
    return originated_;
}

bool Redirector::is_reroutable() const {
    return false;
}

HTTP::byte_string Redirector::reroute_path() const {
    assert(false);
    return HTTP::byte_string();
}

bool Redirector::is_responsive() const {
    return originated_;
}

void Redirector::start_origination(IObserver &observer) {
    (void)observer;
    // オリジネーションとしての特定の動作はないので何もない
}

void Redirector::leave() {
    delete this;
}

ResponseHTTP *Redirector::respond(const RequestHTTP *request, bool should_close) {
    response_data.inject("", 0, true);
    response_data.determine_sending_mode();
    ResponseHTTP::header_list_type headers;
    // redirect_to を Location: に設定
    headers.push_back(std::make_pair(HeaderHTTP::location, redirect_to));
    ResponseHTTP *res
        = new ResponseHTTP(request->get_http_version(), status_code, &headers, &response_data, should_close);
    res->start();
    return res;
}
