#include "../../src/communication/RequestHTTP.hpp"
#include "../../src/config/Context.hpp"
#include "../../src/config/Lexer.hpp"
#include "../../src/config/Parser.hpp"
#include "../../src/router/RequestMatcher.hpp"
#include "../../src/utils/File.hpp"
#include "../../src/utils/HTTPError.hpp"
#include "./TestParam.hpp"
#include "gtest/gtest.h"
#include <iostream>
#include <vector>

namespace {

class request_matcher_test : public testing::Test {
protected:
    request_matcher_test() {}
    virtual ~request_matcher_test() {}
    void setup_based_on_str(const std::string &data) {
        configs = parser.parse(data);
    }

    config::Parser parser;
    config::config_dict configs;

    RequestMatcher rm;
};

TEST_F(request_matcher_test, join_root_and_index) {
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
    const config::host_port_pair &hp = std::make_pair("0.0.0.0", 80);

    {
        TestParam tp(HTTP::METHOD_GET, "/", HTTP::V_1_1, "localhost", "80");
        EXPECT_NO_THROW({
            const RequestMatchingResult res = rm.request_match(configs[hp], tp);
            EXPECT_EQ(HTTP::strfy("./error_page/404.html"), res.path_local);
            EXPECT_EQ(HTTP::strfy(""), res.path_after);
        });
    }

    {
        TestParam tp(HTTP::METHOD_GET, "/405.html", HTTP::V_1_1, "localhost", "80");
        EXPECT_NO_THROW({
            const RequestMatchingResult res = rm.request_match(configs[hp], tp);
            EXPECT_EQ(HTTP::strfy("./error_page/405.html"), res.path_local);
            EXPECT_EQ(HTTP::strfy(""), res.path_after);
        });
    }

    {
        TestParam tp(HTTP::METHOD_GET, "///500.html", HTTP::V_1_1, "localhost", "80");
        EXPECT_NO_THROW({
            const RequestMatchingResult res = rm.request_match(configs[hp], tp);
            EXPECT_EQ(HTTP::strfy("./error_page/500.html"), res.path_local);
            EXPECT_EQ(HTTP::strfy(""), res.path_after);
        });
    }
}

TEST_F(request_matcher_test, url_validation) {
    const std::string config_data = "\
http { \
    server { \
        listen 80; \
    } \
} \
";

    setup_based_on_str(config_data);
    const config::host_port_pair &hp = std::make_pair("0.0.0.0", 80);

    {
        TestParam tp(HTTP::METHOD_GET, "../", HTTP::V_1_1, "localhost", "80");
        EXPECT_THROW(rm.request_match(configs[hp], tp), http_error);
    }

    {
        TestParam tp(HTTP::METHOD_GET, "/xxx/../../", HTTP::V_1_1, "localhost", "80");
        EXPECT_THROW(rm.request_match(configs[hp], tp), http_error);
    }

    {
        TestParam tp(HTTP::METHOD_GET, "/xxx/../xxx/../../", HTTP::V_1_1, "localhost", "80");
        EXPECT_THROW(rm.request_match(configs[hp], tp), http_error);
    }

    {
        TestParam tp(HTTP::METHOD_GET, "/xxx/yyy/../../../", HTTP::V_1_1, "localhost", "80");
        EXPECT_THROW(rm.request_match(configs[hp], tp), http_error);
    }
}

TEST_F(request_matcher_test, method_validation) {
    const std::string config_data = "\
http { \
    server { \
        listen 80; \
        root ./tests; \
        location /get/ { \
            limit_except GET {} \
        } \
        location /post/ { \
            limit_except POST {} \
        } \
        location /delete/ { \
            limit_except DELETE {} \
        } \
    } \
} \
";

    setup_based_on_str(config_data);
    const config::host_port_pair &hp = std::make_pair("0.0.0.0", 80);

    {
        TestParam tp(HTTP::METHOD_UNKNOWN, "/", HTTP::V_1_1, "localhost", "80");
        EXPECT_THROW(rm.request_match(configs[hp], tp), http_error);
    }

    {
        TestParam tp_get(HTTP::METHOD_GET, "/get/", HTTP::V_1_1, "localhost", "80");
        TestParam tp_post(HTTP::METHOD_POST, "/get/", HTTP::V_1_1, "localhost", "80");
        TestParam tp_delete(HTTP::METHOD_DELETE, "/get/", HTTP::V_1_1, "localhost", "80");
        EXPECT_NO_THROW(rm.request_match(configs[hp], tp_get));
        EXPECT_THROW(rm.request_match(configs[hp], tp_post), http_error);
        EXPECT_THROW(rm.request_match(configs[hp], tp_delete), http_error);
    }

    {
        TestParam tp_get(HTTP::METHOD_GET, "/post/", HTTP::V_1_1, "localhost", "80");
        TestParam tp_post(HTTP::METHOD_POST, "/post/", HTTP::V_1_1, "localhost", "80");
        TestParam tp_delete(HTTP::METHOD_DELETE, "/post/", HTTP::V_1_1, "localhost", "80");
        EXPECT_THROW(rm.request_match(configs[hp], tp_get), http_error);
        EXPECT_NO_THROW(rm.request_match(configs[hp], tp_post));
        EXPECT_THROW(rm.request_match(configs[hp], tp_delete), http_error);
    }

    {
        TestParam tp_get(HTTP::METHOD_GET, "/delete/", HTTP::V_1_1, "localhost", "80");
        TestParam tp_post(HTTP::METHOD_POST, "/delete/", HTTP::V_1_1, "localhost", "80");
        TestParam tp_delete(HTTP::METHOD_DELETE, "/delete/", HTTP::V_1_1, "localhost", "80");
        EXPECT_THROW(rm.request_match(configs[hp], tp_get), http_error);
        EXPECT_THROW(rm.request_match(configs[hp], tp_post), http_error);
        EXPECT_NO_THROW(rm.request_match(configs[hp], tp_delete));
    }
}

TEST_F(request_matcher_test, redirection) {
    const std::string config_data = "\
http { \
    server { \
        listen 80; \
        location /42tokyo/ { \
            return 301 https://42tokyo.jp/; \
\
            location /42tokyo/not/reach/ { \
                return 420 \"can not reach\"; \
            } \
        } \
    } \
} \
";

    setup_based_on_str(config_data);
    const config::host_port_pair &hp = std::make_pair("0.0.0.0", 80);

    {
        TestParam tp(HTTP::METHOD_GET, "/42tokyo/", HTTP::V_1_1, "localhost", "80");
        EXPECT_NO_THROW({
            const RequestMatchingResult res = rm.request_match(configs[hp], tp);
            EXPECT_EQ(HTTP::strfy("https://42tokyo.jp/"), res.redirect_location);
            EXPECT_EQ(HTTP::t_status(301), res.status_code);
        });
    }

    {
        TestParam tp(HTTP::METHOD_GET, "/42tokyo/not/reach/", HTTP::V_1_1, "localhost", "80");
        EXPECT_NO_THROW({
            const RequestMatchingResult res = rm.request_match(configs[hp], tp);
            EXPECT_EQ(HTTP::strfy("https://42tokyo.jp/"), res.redirect_location);
            EXPECT_EQ(HTTP::t_status(301), res.status_code);
        });
    }
}

} // namespace
