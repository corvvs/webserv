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
        });
    }

    {
        TestParam tp(HTTP::METHOD_GET, "/405.html", HTTP::V_1_1, "localhost", "80");
        EXPECT_NO_THROW({
            const RequestMatchingResult res = rm.request_match(configs[hp], tp);
            EXPECT_EQ(HTTP::strfy("./error_page/405.html"), res.path_local);
        });
    }

    {
        TestParam tp(HTTP::METHOD_GET, "///500.html", HTTP::V_1_1, "localhost", "80");
        EXPECT_NO_THROW({
            const RequestMatchingResult res = rm.request_match(configs[hp], tp);
            EXPECT_EQ(HTTP::strfy("./error_page/500.html"), res.path_local);
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
        TestParam tp(HTTP::METHOD_GET, "/../", HTTP::V_1_1, "localhost", "80");
        RequestMatchingResult res = rm.request_match(configs[hp], tp);
        EXPECT_EQ(minor_error::make("invalid url target", HTTP::STATUS_BAD_REQUEST), res.error);
    }

    {
        TestParam tp(HTTP::METHOD_GET, "/xxx/../../", HTTP::V_1_1, "localhost", "80");
        RequestMatchingResult res = rm.request_match(configs[hp], tp);
        EXPECT_EQ(minor_error::make("invalid url target", HTTP::STATUS_BAD_REQUEST), res.error);
    }

    {
        TestParam tp(HTTP::METHOD_GET, "/xxx/../xxx/../../", HTTP::V_1_1, "localhost", "80");
        RequestMatchingResult res = rm.request_match(configs[hp], tp);
        EXPECT_EQ(minor_error::make("invalid url target", HTTP::STATUS_BAD_REQUEST), res.error);
    }

    {
        TestParam tp(HTTP::METHOD_GET, "/xxx/yyy/../../../", HTTP::V_1_1, "localhost", "80");
        RequestMatchingResult res = rm.request_match(configs[hp], tp);
        EXPECT_EQ(minor_error::make("invalid url target", HTTP::STATUS_BAD_REQUEST), res.error);
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
        TestParam tp_unknown(HTTP::METHOD_UNKNOWN, "/", HTTP::V_1_1, "localhost", "80");
        RequestMatchingResult res = rm.request_match(configs[hp], tp_unknown);
        EXPECT_EQ(minor_error::make("method not allowed", HTTP::STATUS_METHOD_NOT_ALLOWED), res.error);
    }

    {
        TestParam tp_get(HTTP::METHOD_GET, "/get/", HTTP::V_1_1, "localhost", "80");
        RequestMatchingResult res = rm.request_match(configs[hp], tp_get);
        EXPECT_EQ(minor_error::ok(), res.error);
    }

    {
        TestParam tp_post(HTTP::METHOD_POST, "/get/", HTTP::V_1_1, "localhost", "80");
        RequestMatchingResult res = rm.request_match(configs[hp], tp_post);
        EXPECT_EQ(minor_error::make("method not allowed", HTTP::STATUS_METHOD_NOT_ALLOWED), res.error);
    }

    {
        TestParam tp_delete(HTTP::METHOD_DELETE, "/get/", HTTP::V_1_1, "localhost", "80");
        RequestMatchingResult res = rm.request_match(configs[hp], tp_delete);
        EXPECT_EQ(minor_error::make("method not allowed", HTTP::STATUS_METHOD_NOT_ALLOWED), res.error);
    }

    {
        TestParam tp_get(HTTP::METHOD_GET, "/post/", HTTP::V_1_1, "localhost", "80");
        RequestMatchingResult res = rm.request_match(configs[hp], tp_get);
        EXPECT_EQ(minor_error::make("method not allowed", HTTP::STATUS_METHOD_NOT_ALLOWED), res.error);
    }

    {
        TestParam tp_post(HTTP::METHOD_POST, "/post/", HTTP::V_1_1, "localhost", "80");
        RequestMatchingResult res = rm.request_match(configs[hp], tp_post);
        EXPECT_EQ(minor_error::ok(), res.error);
    }

    {
        TestParam tp_delete(HTTP::METHOD_DELETE, "/post/", HTTP::V_1_1, "localhost", "80");
        RequestMatchingResult res = rm.request_match(configs[hp], tp_delete);
        EXPECT_EQ(minor_error::make("method not allowed", HTTP::STATUS_METHOD_NOT_ALLOWED), res.error);
    }

    {
        TestParam tp_get(HTTP::METHOD_GET, "/delete/", HTTP::V_1_1, "localhost", "80");
        RequestMatchingResult res = rm.request_match(configs[hp], tp_get);
        EXPECT_EQ(minor_error::make("method not allowed", HTTP::STATUS_METHOD_NOT_ALLOWED), res.error);
    }

    {
        TestParam tp_post(HTTP::METHOD_POST, "/delete/", HTTP::V_1_1, "localhost", "80");
        RequestMatchingResult res = rm.request_match(configs[hp], tp_post);
        EXPECT_EQ(minor_error::make("method not allowed", HTTP::STATUS_METHOD_NOT_ALLOWED), res.error);
    }

    {
        TestParam tp_delete(HTTP::METHOD_DELETE, "/delete/", HTTP::V_1_1, "localhost", "80");
        RequestMatchingResult res = rm.request_match(configs[hp], tp_delete);
        EXPECT_EQ(minor_error::ok(), res.error);
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
        const RequestMatchingResult res = rm.request_match(configs[hp], tp);
        EXPECT_EQ(HTTP::strfy("https://42tokyo.jp/"), res.redirect_location);
        EXPECT_EQ(HTTP::t_status(301), res.status_code);
    }

    {
        TestParam tp(HTTP::METHOD_GET, "/42tokyo/not/reach/", HTTP::V_1_1, "localhost", "80");
        const RequestMatchingResult res = rm.request_match(configs[hp], tp);
        EXPECT_EQ(HTTP::strfy("https://42tokyo.jp/"), res.redirect_location);
        EXPECT_EQ(HTTP::t_status(301), res.status_code);
    }
}

TEST_F(request_matcher_test, auto_index) {
    const std::string config_data = "\
http { \
    server { \
        listen 80; \
        autoindex on; \
        root ./ ; \
    } \
} \
";
    setup_based_on_str(config_data);
    const config::host_port_pair &hp = std::make_pair("0.0.0.0", 80);
    {
        TestParam tp(HTTP::METHOD_GET, "/error_page", HTTP::V_1_1, "localhost", "80");
        const RequestMatchingResult res = rm.request_match(configs[hp], tp);
        EXPECT_EQ(RequestMatchingResult::RT_AUTO_INDEX, res.result_type);
    }
    {
        TestParam tp(HTTP::METHOD_GET, "/error_page/", HTTP::V_1_1, "localhost", "80");
        const RequestMatchingResult res = rm.request_match(configs[hp], tp);
        EXPECT_EQ(RequestMatchingResult::RT_AUTO_INDEX, res.result_type);
    }
    {
        TestParam tp(HTTP::METHOD_GET, "/error_page/404.html", HTTP::V_1_1, "localhost", "80");
        const RequestMatchingResult res = rm.request_match(configs[hp], tp);
        EXPECT_EQ(RequestMatchingResult::RT_FILE, res.result_type);
    }
}

TEST_F(request_matcher_test, pct_encoded) {
    const std::string config_data = "\
http { \
    server { \
        listen 80; \
        location /tests/ { \
            root ./; \
        } \
\
        location /tests/ふぉーてぃーつー/ { \
            root ./; \
        } \
    } \
} \
";

    setup_based_on_str(config_data);
    const config::host_port_pair &hp = std::make_pair("0.0.0.0", 80);

    {
        TestParam tp(HTTP::METHOD_GET, "/tests/%E3%81%B0%E3%81%AA%E3%81%AA.html", HTTP::V_1_1, "localhost", "80");
        EXPECT_NO_THROW({
            const RequestMatchingResult res = rm.request_match(configs[hp], tp);
            EXPECT_EQ(HTTP::strfy("./tests/ばなな.html"), res.path_local);
        });
    }

    {
        // 先頭以外のスラッシュを"%2F"に置換
        TestParam tp(HTTP::METHOD_GET, "/tests%2F%E3%81%B0%E3%81%AA%E3%81%AA.html", HTTP::V_1_1, "localhost", "80");
        EXPECT_NO_THROW({
            const RequestMatchingResult res = rm.request_match(configs[hp], tp);
            EXPECT_EQ(HTTP::strfy("./tests/ばなな.html"), res.path_local);
        });
    }

    {
        // 先頭のスラッシュを"%2F"に置換 → ヒットしなくなる
        TestParam tp(HTTP::METHOD_GET, "%2Ftests%2F%E3%81%B0%E3%81%AA%E3%81%AA.html", HTTP::V_1_1, "localhost", "80");
        EXPECT_THROW(rm.request_match(configs[hp], tp);, http_error);
    }

    {
        TestParam tp(HTTP::METHOD_GET,
                     "/tests/%E3%81%B5%E3%81%89%E3%83%BC%E3%81%A6%E3%81%83%E3%83%BC%E3%81%A4%E3%83%BC/banana.txt",
                     HTTP::V_1_1,
                     "localhost",
                     "80");
        EXPECT_NO_THROW({
            const RequestMatchingResult res = rm.request_match(configs[hp], tp);
            EXPECT_EQ(HTTP::strfy("./tests/ふぉーてぃーつー/banana.txt"), res.path_local);
        });
    }
}

} // namespace
