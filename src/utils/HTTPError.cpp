#include "HTTPError.hpp"

// [http_error]

http_error::http_error(const char *_Message, HTTP::t_status status) : runtime_error(_Message), status_(status) {}

HTTP::t_status http_error::get_status() const {
    return status_;
}

// [minor_error]

minor_error::minor_error() : std::pair<first_type, second_type>("", HTTP::STATUS_OK) {}

minor_error::minor_error(first_type message, second_type status_code)
    : std::pair<first_type, second_type>(message, status_code) {}

minor_error::minor_error(const minor_error &other) {
    *this = other;
}

minor_error &minor_error::operator=(const minor_error &rhs) {
    if (this != &rhs) {
        first  = rhs.first;
        second = rhs.second;
    }
    return *this;
}

bool minor_error::is_ok() const {
    return !is_error();
}

bool minor_error::is_error() const {
    return first.size() > 0;
}

minor_error::first_type minor_error::message() const {
    return first;
}

minor_error::second_type minor_error::status_code() const {
    return second;
}

minor_error minor_error::ok() {
    return make("", HTTP::STATUS_OK);
}

minor_error minor_error::make(const std::string &message, HTTP::t_status status_code) {
    return minor_error(message, status_code);
}

std::ostream &operator<<(std::ostream &ost, const minor_error &f) {
    if (f.is_ok()) {
        return ost << "[" << f.status_code() << "]";
    } else {
        return ost << "[" << f.status_code() << "] " << f.message();
    }
}

const minor_error &erroneous(const minor_error &e1, const minor_error &e2) {
    return e1.is_error() ? e1 : e2;
}
