#include "HTTPError.hpp"

http_error::http_error(const char *_Message, HTTP::t_status status) : runtime_error(_Message), status_(status) {}

HTTP::t_status http_error::get_status() const {
    return status_;
}
