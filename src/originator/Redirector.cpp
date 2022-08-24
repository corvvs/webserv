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

void Redirector::generate_html() {
    std::stringstream ss;
    ss << status_code;
    std::string status = ss.str();
    std::string message;
    switch (status_code) {
        case HTTP::STATUS_MOVED_PERMANENTLY:
            message = "Moved Permanently";
            break;
        case HTTP::STATUS_FOUND:
            message = "Found";
            break;
        case HTTP::STATUS_NOT_MODIFIED:
            message = "Not Modified";
            break;
        case HTTP::STATUS_TEMPORARY_REDIRECT:
            message = "Temporary Redirect";
            break;
        case HTTP::STATUS_PERMANENT_REDIRECT:
            message = "Permanent Redirect";
            break;
        default:;
    }
    response_data.inject(HTTP::strfy("<html>\n"
                                     "<head><title>"
                                     + status + " " + message
                                     + "</title></head>\n"
                                       "<body>\n"),
                         false);

    // page header

    response_data.inject(HTTP::strfy("<center><h1>" + status + " " + message
                                     + "</h1></center>\n"
                                       "<hr><center>webserv</center>\n"),
                         false);

    // page footer

    response_data.inject(HTTP::strfy("</body>\n"
                                     "</html>\n"),
                         false);
    response_data.inject("", 0, true);
}

ResponseHTTP *Redirector::respond(const RequestHTTP *request) {
    ResponseHTTP::header_list_type headers;

    // redirect_to を Location: に設定
    switch (status_code) {
        case HTTP::STATUS_MOVED_PERMANENTLY:
        case HTTP::STATUS_FOUND:
        case HTTP::STATUS_NOT_MODIFIED:
        case HTTP::STATUS_TEMPORARY_REDIRECT:
        case HTTP::STATUS_PERMANENT_REDIRECT:
            headers.push_back(std::make_pair(HeaderHTTP::location, redirect_to));
            generate_html();
            break;
        default:;
            response_data.inject(redirect_to, true);
    }

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

    ResponseHTTP *res = new ResponseHTTP(request->get_http_version(), status_code, &headers, &response_data, false);

    res->start();
    return res;
}
