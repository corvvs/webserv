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
        QVOUT(target_.given);
        QVOUT(target_.path);
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

namespace {
class config_original : public testing::Test {
protected:
    config_original() {}
    virtual ~config_original() {}
    void SetUpBasedOnStr(const std::string &data) {
        configs = parser.parse(data);
    }

    config::Parser parser;
    std::map<config::host_port_pair, std::vector<config::Config> > configs;
};

TEST_F(config_original, get_exec_cgi) {
    const std::string config_data = "\
http { \
    server { \
        listen 80; \
        location /on/ { \
            exec_cgi on; \
        } \
        location /off/ { \
            exec_cgi off; \
        } \
    } \
} \
";
    SetUpBasedOnStr(config_data);
    const config::host_port_pair hp = std::make_pair("0.0.0.0", 80);
    const config::Config conf       = configs[hp].front();

    EXPECT_EQ(false, conf.get_exec_cgi("/"));
    EXPECT_EQ(true, conf.get_exec_cgi("/on/"));
    EXPECT_EQ(false, conf.get_exec_cgi("/off/"));
}

TEST_F(config_original, get_exec_delete) {
    const std::string config_data = "\
http { \
    server { \
        listen 80; \
        location /on/ { \
            exec_delete on; \
        } \
        location /off/ { \
            exec_delete off; \
        } \
    } \
} \
";
    SetUpBasedOnStr(config_data);
    const config::host_port_pair hp = std::make_pair("0.0.0.0", 80);
    const config::Config conf       = configs[hp].front();
    EXPECT_EQ(false, conf.get_exec_delete("/"));
    EXPECT_EQ(true, conf.get_exec_delete("/on/"));
    EXPECT_EQ(false, conf.get_exec_delete("/off/"));
}

TEST_F(config_original, get_cgi_path) {
    const std::string config_data = "\
http { \
    server { \
        listen 80; \
        location /ruby/ { \
            exec_cgi on; \
            cgi_path .rb /usr/bin/ruby; \
        } \
        location /python/ { \
            exec_cgi on; \
            cgi_path .py /usr/bin/python; \
        } \
    } \
} \
";
    SetUpBasedOnStr(config_data);
    const config::host_port_pair hp = std::make_pair("0.0.0.0", 80);
    const config::Config conf       = configs[hp].front();

    {
        const config::cgi_executer_map expected;
        //        const config::cgi_executer_map actual = conf.get_cgi_path("/");
        //        EXPECT_EQ(expected.empty(), actual.empty());
    }
    {
        config::cgi_executer_map expected;
        const std::string extension     = ".rb";
        expected[".rb"]                 = "/usr/bin/ruby";
        config::cgi_executer_map actual = conf.get_cgi_path("/ruby/");
        //        EXPECT_EQ(expected.size(), actual.size());
        EXPECT_EQ(expected[extension], actual[extension]);
    }
    {
        config::cgi_executer_map expected;
        const std::string extension     = ".py";
        expected[extension]             = "/usr/bin/python";
        config::cgi_executer_map actual = conf.get_cgi_path("/python/");
        //        EXPECT_EQ(expected.size(), actual.size());
        EXPECT_EQ(expected[extension], actual[extension]);
    }
}

TEST_F(config_original, request_match_cgi_found) {
    const std::string config_data = "\
http { \
    server { \
        listen 80; \
        location /ruby/ { \
            exec_cgi on; \
            root ./cgi/;\
            cgi_path .rb /usr/bin/ruby; \
        } \
        location /python/ { \
            exec_cgi on; \
            cgi_path .py /usr/bin/python; \
        } \
    } \
} \
";
    SetUpBasedOnStr(config_data);
    const config::host_port_pair hp = std::make_pair("0.0.0.0", 80);
    const config::Config conf       = configs[hp].front();
    RequestMatcher rm;

    {
        TestParam tp(HTTP::METHOD_GET, "/ruby/blank.rb", HTTP::V_1_1, "localhost", "80");
        RequestMatchingResult res;
        EXPECT_NO_THROW({
            res = rm.request_match(configs[hp], tp);
            EXPECT_GT(res.path_local.size(), 0);
            EXPECT_EQ(res.path_after, HTTP::strfy(""));
        });
    }

    {
        TestParam tp(HTTP::METHOD_GET, "/ruby/blank.rb/path/after", HTTP::V_1_1, "localhost", "80");
        RequestMatchingResult res;
        EXPECT_NO_THROW({
            res = rm.request_match(configs[hp], tp);
            EXPECT_GT(res.path_local.size(), 0);
            EXPECT_EQ(res.path_after, HTTP::strfy("/path/after"));
        });
    }
}

TEST_F(config_original, request_match_cgi_not_found) {
    const std::string config_data = "\
http { \
    server { \
        listen 80; \
        location /ruby/ { \
            exec_cgi on; \
            root ./cgi/;\
            cgi_path .rb /usr/bin/ruby; \
        } \
        location /python/ { \
            exec_cgi on; \
            cgi_path .py /usr/bin/python; \
        } \
    } \
} \
";
    SetUpBasedOnStr(config_data);
    const config::host_port_pair hp = std::make_pair("0.0.0.0", 80);
    const config::Config conf       = configs[hp].front();
    RequestMatcher rm;

    {
        TestParam tp(HTTP::METHOD_GET, "/ruby/not_exist.rb/path/after", HTTP::V_1_1, "localhost", "80");
        RequestMatchingResult res;
        EXPECT_THROW(rm.request_match(configs[hp], tp), http_error);
    }
}

} // namespace
