#include "Echoer.hpp"

Echoer::Echoer() : originated_(false) {}

void Echoer::notify(IObserver &observer, IObserver::observation_category cat, t_time_epoch_ms epoch) {
    (void)observer;
    (void)cat;
    (void)epoch;
    assert(false);
}

void Echoer::inject_socketlike(ISocketLike *socket_like) {
    (void)socket_like;
}

bool Echoer::is_originatable() const {
    return !originated_;
}

bool Echoer::is_origination_started() const {
    return originated_;
}

bool Echoer::is_reroutable() const {
    return false;
}

bool Echoer::is_responsive() const {
    return originated_;
}

void Echoer::start_origination(IObserver &observer) {
    (void)observer;
    originated_ = true;
}

void Echoer::leave() {
    delete this;
}

ResponseHTTP *Echoer::respond(const RequestHTTP &request) {
    HTTP::byte_string message = request.get_plain_message();
    response_data.inject(&message.front(), message.size(), true);
    ResponseHTTP *res = new ResponseHTTP(request.get_http_version(), HTTP::STATUS_OK, &response_data);
    res->start();
    return res;
}
