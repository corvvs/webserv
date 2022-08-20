#include "../../src/Interfaces.hpp"
#include "../../src/config/Context.hpp"
#include "../../src/config/Lexer.hpp"
#include "../../src/config/Parser.hpp"
#include "../../src/router/RequestMatcher.hpp"
#include "../../src/utils/File.hpp"
#include "gtest/gtest.h"
#include <iostream>
#include <vector>

class TestParam : public IRequestMatchingParam {
private:
    HTTP::t_method method_;
    HTTP::t_version version_;
    HTTP::CH::Host host_;
    RequestTarget target_;

    HTTP::byte_string path_;

public:
    TestParam(HTTP::t_method method,
              const HTTP::char_string &path,
              HTTP::t_version version,
              const HTTP::char_string &host,
              const HTTP::char_string &port)
        : method_(method), version_(version), path_(HTTP::strfy(path)) {
        target_    = RequestTarget(path_);
        host_.host = HTTP::strfy(host);
        host_.port = HTTP::strfy(port);
    }

    const RequestTarget &get_request_target() const {
        return target_;
    };
    HTTP::t_method get_http_method() const {
        return method_;
    }
    HTTP::t_version get_http_version() const {
        return version_;
    }
    const HTTP::CH::Host &get_host() const {
        return host_;
    }
};
