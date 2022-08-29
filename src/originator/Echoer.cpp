#include "Echoer.hpp"

Echoer::Echoer(const RequestMatchingResult &match_result) : originated_(false) {
    (void)match_result;
}

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

HTTP::byte_string Echoer::reroute_path() const {
    assert(false);
    return HTTP::byte_string();
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

ResponseHTTP *Echoer::respond(const RequestHTTP *request, bool should_close) {
    HTTP::byte_string message = request->get_plain_message();
    response_data.inject(message, true);
    ResponseHTTP::header_list_type headers;
    IResponseDataConsumer::t_sending_mode sm = response_data.determine_sending_mode();
    switch (sm) {
        case ResponseDataList::SM_CHUNKED:
            headers.push_back(std::make_pair(HeaderHTTP::transfer_encoding, HTTP::strfy("chunked")));
            break;
        case ResponseDataList::SM_NOT_CHUNKED:
            headers.push_back(
                std::make_pair(HeaderHTTP::content_length, ParserHelper::utos(response_data.current_total_size(), 10)));
            break;
        default:
            break;
    }
    ResponseHTTP *res
        = new ResponseHTTP(request->get_http_version(), HTTP::STATUS_OK, &headers, &response_data, should_close);
    return res;
}
