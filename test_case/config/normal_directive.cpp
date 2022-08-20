#include "../../src/config/Context.hpp"
#include "../../src/config/Lexer.hpp"
#include "../../src/config/Parser.hpp"
#include "../../src/utils/File.hpp"
#include "gtest/gtest.h"
#include <iostream>
#include <vector>

namespace {
class normal_directive_test : public testing::Test {
protected:
    normal_directive_test() {}
    virtual ~normal_directive_test() {}
    void setup_based_on_file(const std::string &path) {
        assert(file::check(path) == file::NONE);
        configs = parser.parse(file::read(path));
    }
    virtual void TearDown() {}

    config::Parser parser;
    std::map<config::host_port_pair, std::vector<config::Config> > configs;
};

/// 01_default.conf
TEST_F(normal_directive_test, get_auto_index) {
    setup_based_on_file("./conf/valid/01_default.conf");
    const config::host_port_pair hp = std::make_pair("0.0.0.0", 80);
    const config::Config conf       = configs[hp].front();

    const std::string target = "/";
    const bool autoindex     = false;
    EXPECT_EQ(autoindex, conf.get_autoindex(target));
}

TEST_F(normal_directive_test, get_error_page) {
    setup_based_on_file("./conf/valid/01_default.conf");
    const config::host_port_pair hp = std::make_pair("0.0.0.0", 80);
    const config::Config conf       = configs[hp].front();

    const std::string target = "/";
    std::map<HTTP::t_status, std::string> error_page;
    //    error_page[404] = "";
    EXPECT_EQ(error_page, conf.get_error_page(target));
}

TEST_F(normal_directive_test, get_index) {
    setup_based_on_file("./conf/valid/01_default.conf");
    const config::host_port_pair hp = std::make_pair("0.0.0.0", 80);
    const config::Config conf       = configs[hp].front();

    const std::string target = "/";
    std::vector<std::string> index;
    index.push_back("index.html");
    index.push_back("index.htm");
    EXPECT_EQ(index, conf.get_index(target));
}

TEST_F(normal_directive_test, get_root) {
    setup_based_on_file("./conf/valid/01_default.conf");
    const config::host_port_pair hp = std::make_pair("0.0.0.0", 80);
    const config::Config conf       = configs[hp].front();

    const std::string target = "/";
    const std::string root   = "/data/server1/";
    EXPECT_EQ(root, conf.get_root(target));
}

TEST_F(normal_directive_test, get_client_max_body_size) {
    setup_based_on_file("./conf/valid/01_default.conf");
    const config::host_port_pair hp = std::make_pair("0.0.0.0", 80);
    const config::Config conf       = configs[hp].front();

    const std::string target        = "/";
    const long client_max_body_size = 1024;
    EXPECT_EQ(client_max_body_size, conf.get_client_max_body_size(target));
}

TEST_F(normal_directive_test, get_host) {
    setup_based_on_file("./conf/valid/01_default.conf");
    const config::host_port_pair hp = std::make_pair("0.0.0.0", 80);
    const config::Config conf       = configs[hp].front();

    const std::string host = "0.0.0.0";
    EXPECT_EQ(host, conf.get_host());
}

TEST_F(normal_directive_test, get_port) {
    setup_based_on_file("./conf/valid/01_default.conf");
    const config::host_port_pair hp = std::make_pair("0.0.0.0", 80);
    const config::Config conf       = configs[hp].front();

    const int port = 80;
    EXPECT_EQ(port, conf.get_port());
}

TEST_F(normal_directive_test, get_redirect) {
    setup_based_on_file("./conf/valid/01_default.conf");
    const config::host_port_pair hp = std::make_pair("0.0.0.0", 80);
    const config::Config conf       = configs[hp].front();

    const std::string target                        = "/";
    std::pair<HTTP::t_status, std::string> redirect = std::make_pair(HTTP::STATUS_REDIRECT_INIT, "");
    EXPECT_EQ(redirect, conf.get_redirect(target));
}

TEST_F(normal_directive_test, get_server_name) {
    setup_based_on_file("./conf/valid/01_default.conf");
    const config::host_port_pair hp = std::make_pair("0.0.0.0", 80);
    const config::Config conf       = configs[hp].front();

    std::vector<std::string> server_name;
    server_name.push_back("server1");
    EXPECT_EQ(server_name, conf.get_server_name());
}

TEST_F(normal_directive_test, get_upload_store) {
    setup_based_on_file("./conf/valid/01_default.conf");
    const config::host_port_pair hp = std::make_pair("0.0.0.0", 80);
    const config::Config conf       = configs[hp].front();

    const std::string target = "/";
    const std::string upload_store;
    EXPECT_EQ(upload_store, conf.get_upload_store(target));
}

TEST_F(normal_directive_test, get_default_server) {
    setup_based_on_file("./conf/valid/01_default.conf");
    const config::host_port_pair hp = std::make_pair("0.0.0.0", 80);
    const config::Config conf       = configs[hp].front();

    const std::string target  = "/";
    const bool default_server = false;
    EXPECT_EQ(default_server, conf.get_default_server());
}

TEST_F(normal_directive_test, get_limit_except) {
    setup_based_on_file("./conf/valid/01_default.conf");
    const config::host_port_pair hp = std::make_pair("0.0.0.0", 80);
    const config::Config conf       = configs[hp].front();

    const std::string target = "/";
    std::set<enum config::Methods> limit_except;
    //    limit_except.insert(config::Methods::GET);
    EXPECT_EQ(limit_except, conf.get_limit_except(target));
}

/// 11_mix.conf
TEST_F(normal_directive_test, get_auto_index_from_mix_context) {
    setup_based_on_file("./conf/valid/11_mix.conf");
    const config::host_port_pair hp = std::make_pair("127.0.0.1", 80);
    const config::Config conf       = configs[hp].front();

    EXPECT_EQ(true, conf.get_autoindex("/"));
    EXPECT_EQ(true, conf.get_autoindex("/dir1"));
    EXPECT_EQ(false, conf.get_autoindex("/dir2"));
    EXPECT_EQ(false, conf.get_autoindex("/dir2/dir3"));
}

TEST_F(normal_directive_test, get_error_page_from_mix_context) {
    setup_based_on_file("./conf/valid/11_mix.conf");
    const config::host_port_pair hp = std::make_pair("127.0.0.1", 80);
    const config::Config conf       = configs[hp].front();

    std::map<HTTP::t_status, std::string> error_page;
    error_page[HTTP::STATUS_BAD_REQUEST] = "error.html";
    EXPECT_EQ(error_page, conf.get_error_page("/"));
    error_page.clear();
    error_page[HTTP::STATUS_UNAUTHORIZED] = "error1.html";
    EXPECT_EQ(error_page, conf.get_error_page("/dir1"));
    error_page.clear();
    error_page[(HTTP::t_status)402] = "error2.html";
    EXPECT_EQ(error_page, conf.get_error_page("/dir2"));
    error_page.clear();
    error_page[HTTP::STATUS_FORBIDDEN] = "error3.html";
    EXPECT_EQ(error_page, conf.get_error_page("/dir2/dir3"));
}

TEST_F(normal_directive_test, get_index_from_mix_context) {
    setup_based_on_file("./conf/valid/11_mix.conf");
    const config::host_port_pair hp = std::make_pair("127.0.0.1", 80);
    const config::Config conf       = configs[hp].front();

    std::vector<std::string> index;
    index.push_back("index.html");
    EXPECT_EQ(index, conf.get_index("/"));
    index.clear();
    index.push_back("index1.html");
    EXPECT_EQ(index, conf.get_index("/dir1"));
    index.clear();
    index.push_back("index2.html");
    EXPECT_EQ(index, conf.get_index("/dir2"));
    index.clear();
    index.push_back("index3.html");
    EXPECT_EQ(index, conf.get_index("/dir2/dir3"));
}

TEST_F(normal_directive_test, get_root_from_mix_context) {
    setup_based_on_file("./conf/valid/11_mix.conf");
    const config::host_port_pair hp = std::make_pair("127.0.0.1", 80);
    const config::Config conf       = configs[hp].front();

    EXPECT_EQ("/root1", conf.get_root("/dir1"));
    EXPECT_EQ("/root2", conf.get_root("/dir2/"));
    EXPECT_EQ("/root3", conf.get_root("/dir2/dir3"));
}

TEST_F(normal_directive_test, get_client_max_body_size_from_mix_context) {
    setup_based_on_file("./conf/valid/11_mix.conf");
    const config::host_port_pair hp = std::make_pair("127.0.0.1", 80);
    const config::Config conf       = configs[hp].front();

    EXPECT_EQ(4242, conf.get_client_max_body_size("/"));
    EXPECT_EQ(1, conf.get_client_max_body_size("/dir1"));
    EXPECT_EQ(2, conf.get_client_max_body_size("/dir2"));
    EXPECT_EQ(3, conf.get_client_max_body_size("/dir2/dir3"));
}

TEST_F(normal_directive_test, get_host_from_mix_context) {
    setup_based_on_file("./conf/valid/11_mix.conf");
    {
        const config::host_port_pair hp = std::make_pair("127.0.0.1", 80);
        const config::Config conf       = configs[hp].front();
        EXPECT_EQ("127.0.0.1", conf.get_host());
    }
    {
        const config::host_port_pair hp = std::make_pair("1.1.1.1", 81);
        const config::Config conf       = configs[hp].back();
        EXPECT_EQ("1.1.1.1", conf.get_host());
    }
}

TEST_F(normal_directive_test, get_port_from_mix_context) {
    setup_based_on_file("./conf/valid/11_mix.conf");
    {
        const config::host_port_pair hp = std::make_pair("127.0.0.1", 80);
        const config::Config conf       = configs[hp].front();
        EXPECT_EQ(80, conf.get_port());
    }
    {
        const config::host_port_pair hp = std::make_pair("1.1.1.1", 81);
        const config::Config conf       = configs[hp].front();
        EXPECT_EQ(81, conf.get_port());
    }
}

TEST_F(normal_directive_test, get_redirect_from_mix_context) {
    setup_based_on_file("./conf/valid/11_mix.conf");
    const config::host_port_pair hp = std::make_pair("127.0.0.1", 80);
    const config::Config conf       = configs[hp].front();

    EXPECT_EQ(std::make_pair((HTTP::t_status)300, std::string("/")), conf.get_redirect("/"));
    EXPECT_EQ(std::make_pair((HTTP::t_status)300, std::string("/")), conf.get_redirect("/dir1"));
    EXPECT_EQ(std::make_pair((HTTP::t_status)300, std::string("/")), conf.get_redirect("/dir2"));
    EXPECT_EQ(std::make_pair((HTTP::t_status)300, std::string("/")), conf.get_redirect("/dir2/dir3"));
}

TEST_F(normal_directive_test, get_server_name_from_mix_context) {
    setup_based_on_file("./conf/valid/11_mix.conf");
    const config::host_port_pair hp = std::make_pair("127.0.0.1", 80);
    const config::Config conf       = configs[hp].front();

    std::vector<std::string> server_name;
    server_name.push_back("server");
    EXPECT_EQ(server_name, conf.get_server_name());
}

TEST_F(normal_directive_test, get_upload_store_from_mix_context) {
    setup_based_on_file("./conf/valid/11_mix.conf");
    const config::host_port_pair hp = std::make_pair("127.0.0.1", 80);
    const config::Config conf       = configs[hp].front();

    EXPECT_EQ("/upload", conf.get_upload_store("/"));
    EXPECT_EQ("/upload1", conf.get_upload_store("/dir1"));
    EXPECT_EQ("/upload2", conf.get_upload_store("/dir2"));
    EXPECT_EQ("/upload3", conf.get_upload_store("/dir2/dir3"));
}

TEST_F(normal_directive_test, get_default_server_from_mix_context) {
    setup_based_on_file("./conf/valid/11_mix.conf");
    {
        const config::host_port_pair hp = std::make_pair("127.0.0.1", 80);
        const config::Config conf       = configs[hp].front();
        EXPECT_EQ(80, conf.get_port());
        EXPECT_EQ(false, conf.get_default_server());
    }
    {
        const config::host_port_pair hp = std::make_pair("1.1.1.1", 81);
        const config::Config conf       = configs[hp].front();
        EXPECT_EQ(81, conf.get_port());
        EXPECT_EQ(true, conf.get_default_server());
    }
}

TEST_F(normal_directive_test, get_limit_except_from_mix_context) {
    setup_based_on_file("./conf/valid/11_mix.conf");
    const config::host_port_pair hp = std::make_pair("127.0.0.1", 80);
    const config::Config conf       = configs[hp].front();
    {
        std::set<enum config::Methods> limit_except;
        EXPECT_EQ(limit_except, conf.get_limit_except("/"));
    }
    {
        std::set<enum config::Methods> limit_except;
        limit_except.insert(config::GET);
        EXPECT_EQ(limit_except, conf.get_limit_except("/dir1"));
    }
    {
        std::set<enum config::Methods> limit_except;
        limit_except.insert(config::GET);
        limit_except.insert(config::POST);
        EXPECT_EQ(limit_except, conf.get_limit_except("/dir2"));
    }
    {
        std::set<enum config::Methods> limit_except;
        limit_except.insert(config::GET);
        limit_except.insert(config::POST);
        limit_except.insert(config::DELETE);
        EXPECT_EQ(limit_except, conf.get_limit_except("/dir2/dir3"));
    }
}
} // namespace
