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

bool Redirector::is_responsive() const {
    return originated_;
}

void Redirector::start_origination(IObserver &observer) {
    (void)observer;
}

void Redirector::leave() {
    delete this;
}

ResponseHTTP *Redirector::respond(const RequestHTTP &request) {
    response_data.inject("", 0, true);
    response_data.determine_sending_mode();
    ResponseHTTP::header_list_type headers;
    headers.push_back(std::make_pair(HeaderHTTP::location, redirect_to));
    ResponseHTTP *res = new ResponseHTTP(request.get_http_version(), status_code, &headers, &response_data);
    res->start();
    return res;
}
