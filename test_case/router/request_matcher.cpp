#include "../../src/communication/RequestHTTP.hpp"
#include "../../src/config/Context.hpp"
#include "../../src/config/File.hpp"
#include "../../src/config/Lexer.hpp"
#include "../../src/config/Parser.hpp"
#include "../../src/router/RequestMatcher.hpp"
#include "gtest/gtest.h"
#include <iostream>
#include <vector>

namespace {

class test_request_matcher : public testing::Test {
protected:
    test_request_matcher() {}
    virtual ~test_request_matcher() {}
    void setup_based_on_str(const std::string &data) {
        conf_dict = parser.parse(data);
    }

    RequestTarget create_target(const HTTP::byte_string &path) {
        RequestTarget res;
        res.path = HTTP::light_string(path);
        return res;
    }

    config::Parser parser;
    config::config_dict conf_dict;
    RequestMatcher matcher;
};

TEST_F(test_request_matcher, join_root_and_index) {
    const std::string config_data = "\
http { \
    server { \
        listen 80; \
        location / { \
            index 404.html; \
            root ./error_page/; \
        } \
    } \
} \
";

    setup_based_on_str(config_data);
    const config::host_port_pair &hp   = std::make_pair("0.0.0.0", 80);
    const config::config_vector &confs = conf_dict[hp];

    {
        RequestHTTP::RoutingParameters rp;
        const HTTP::byte_string &path(HTTP::strfy("/"));
        rp.given_request_target = create_target(path);
        rp.http_method          = HTTP::METHOD_GET;

        RequestMatchingResult expected;
        expected.path_local  = HTTP::strfy("./error_page/404.html");
        expected.result_type = RequestMatchingResult::RT_FILE;

        RequestMatchingResult actual = matcher.request_match(confs, rp);
        EXPECT_EQ(HTTP::restrfy(expected.path_local), HTTP::restrfy(actual.path_local));
        EXPECT_EQ(expected.result_type, actual.result_type);
    }
    {
        RequestHTTP::RoutingParameters rp;
        const HTTP::byte_string &path(HTTP::strfy("/405.html"));
        rp.given_request_target = create_target(path);
        rp.http_method          = HTTP::METHOD_GET;

        RequestMatchingResult expected;
        expected.path_local  = HTTP::strfy("./error_page/405.html");
        expected.result_type = RequestMatchingResult::RT_FILE;

        RequestMatchingResult actual = matcher.request_match(confs, rp);
        EXPECT_EQ(HTTP::restrfy(expected.path_local), HTTP::restrfy(actual.path_local));
        EXPECT_EQ(expected.result_type, actual.result_type);
    }
    {
        RequestHTTP::RoutingParameters rp;
        const HTTP::byte_string &path(HTTP::strfy("///500.html"));
        rp.given_request_target = create_target(path);
        rp.http_method          = HTTP::METHOD_GET;

        RequestMatchingResult expected;
        expected.path_local  = HTTP::strfy("./error_page/500.html");
        expected.result_type = RequestMatchingResult::RT_FILE;

        RequestMatchingResult actual = matcher.request_match(confs, rp);
        EXPECT_EQ(HTTP::restrfy(expected.path_local), HTTP::restrfy(actual.path_local));
        EXPECT_EQ(expected.result_type, actual.result_type);
    }
}

} // namespace
