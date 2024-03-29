#include "../../src/Interfaces.hpp"
#include "../../src/config/Context.hpp"
#include "../../src/config/Lexer.hpp"
#include "../../src/config/Parser.hpp"
#include "../../src/router/RequestMatcher.hpp"
#include "../../src/utils/File.hpp"
#include "TestParam.hpp"
#include "gtest/gtest.h"
#include <iostream>
#include <vector>

namespace {
class original_directive_test : public testing::Test {
protected:
    original_directive_test() {}
    virtual ~original_directive_test() {}
    void setup_based_on_str(const std::string &data) {
        configs = parser.parse(data);
    }

    config::Parser parser;
    std::map<config::host_port_pair, std::vector<config::Config> > configs;
};

TEST_F(original_directive_test, get_exec_cgi) {
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
    setup_based_on_str(config_data);
    const config::host_port_pair hp = std::make_pair("0.0.0.0", 80);
    const config::Config conf       = configs[hp].front();

    EXPECT_EQ(false, conf.get_exec_cgi("/"));
    EXPECT_EQ(true, conf.get_exec_cgi("/on/"));
    EXPECT_EQ(false, conf.get_exec_cgi("/off/"));
}

TEST_F(original_directive_test, get_exec_delete) {
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
    setup_based_on_str(config_data);
    const config::host_port_pair hp = std::make_pair("0.0.0.0", 80);
    const config::Config conf       = configs[hp].front();
    EXPECT_EQ(false, conf.get_exec_delete("/"));
    EXPECT_EQ(true, conf.get_exec_delete("/on/"));
    EXPECT_EQ(false, conf.get_exec_delete("/off/"));
}

TEST_F(original_directive_test, get_cgi_path) {
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
    setup_based_on_str(config_data);
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

TEST_F(original_directive_test, request_match_cgi_found) {
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
    setup_based_on_str(config_data);
    const config::host_port_pair hp = std::make_pair("0.0.0.0", 80);
    const config::Config conf       = configs[hp].front();
    RequestMatcher rm;

    {
        TestParam tp(HTTP::METHOD_GET, "/ruby/blank.rb", HTTP::V_1_1, "localhost", "80");
        EXPECT_NO_THROW({
            RequestMatchingResult res = rm.request_match(configs[hp], tp);
            EXPECT_EQ(HTTP::strfy("./cgi"), res.cgi_resource.root);
            EXPECT_EQ(HTTP::strfy("/ruby/blank.rb"), res.cgi_resource.script_name);
            EXPECT_EQ(HTTP::strfy(""), res.cgi_resource.path_info);
            EXPECT_EQ(HTTP::strfy("/usr/bin/ruby"), res.path_cgi_executor);
        });
    }

    {
        TestParam tp(HTTP::METHOD_GET, "/ruby/blank.rb/path/after", HTTP::V_1_1, "localhost", "80");
        EXPECT_NO_THROW({
            RequestMatchingResult res = rm.request_match(configs[hp], tp);
            EXPECT_EQ(HTTP::strfy("./cgi"), res.cgi_resource.root);
            EXPECT_EQ(HTTP::strfy("/ruby/blank.rb"), res.cgi_resource.script_name);
            EXPECT_EQ(HTTP::strfy("/path/after"), res.cgi_resource.path_info);
            EXPECT_EQ(HTTP::strfy("/usr/bin/ruby"), res.path_cgi_executor);
        });
    }
}

TEST_F(original_directive_test, request_match_cgi_not_found) {
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
    setup_based_on_str(config_data);
    const config::host_port_pair hp = std::make_pair("0.0.0.0", 80);
    const config::Config conf       = configs[hp].front();
    RequestMatcher rm;

    {
        TestParam tp(HTTP::METHOD_GET, "/ruby/not_exist.rb/path/after", HTTP::V_1_1, "localhost", "80");
        RequestMatchingResult res = rm.request_match(configs[hp], tp);
        EXPECT_TRUE(res.error.is_error());
    }
}

} // namespace
