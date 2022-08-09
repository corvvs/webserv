#include "../../src/config/Context.hpp"
#include "../../src/config/File.hpp"
#include "../../src/config/Lexer.hpp"
#include "../../src/config/Parser.hpp"
#include "gtest/gtest.h"
#include <iostream>
#include <vector>

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

} // namespace
