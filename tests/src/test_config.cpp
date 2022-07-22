#include "../../src/config/Context.hpp"
#include "../../src/config/File.hpp"
#include "../../src/config/Lexer.hpp"
#include "../../src/config/Parser.hpp"
#include "gtest/gtest.h"
#include <iostream>
#include <vector>

namespace {
class ConfigTest : public testing::Test {
protected:
    ConfigTest() {}
    virtual ~ConfigTest() {}
    virtual void SetUp(const std::string &path) {
        assert(file::check(path) == file::NONE);
        configs = parser.parse(file::read(path));
    }
    virtual void TearDown() {}

    config::Parser parser;
    std::vector<config::Config> configs;
};

/// 01_default.conf
TEST_F(ConfigTest, GetAutoIndex) {
    SetUp("../conf/valid/01_default.conf");
    config::Config conf      = configs.front();
    const std::string target = "/";
    const bool autoindex     = false;
    EXPECT_EQ(autoindex, conf.get_autoindex(target));
}

TEST_F(ConfigTest, GetErrorPage) {
    SetUp("../conf/valid/01_default.conf");
    config::Config conf      = configs.front();
    const std::string target = "/";
    std::map<int, std::string> error_page;
    //    error_page[404] = "";
    EXPECT_EQ(error_page, conf.get_error_page(target));
}

TEST_F(ConfigTest, GetIndex) {
    SetUp("../conf/valid/01_default.conf");
    config::Config conf      = configs.front();
    const std::string target = "/";
    std::vector<std::string> index;
    index.push_back("index.html");
    index.push_back("index.htm");
    EXPECT_EQ(index, conf.get_index(target));
}

TEST_F(ConfigTest, GetRoot) {
    SetUp("../conf/valid/01_default.conf");
    config::Config conf      = configs.front();
    const std::string target = "/";
    const std::string root   = "/data/server1/";
    EXPECT_EQ(root, conf.get_root(target));
}

TEST_F(ConfigTest, GetClientMaxBodySize) {
    SetUp("../conf/valid/01_default.conf");
    config::Config conf             = configs.front();
    const std::string target        = "/";
    const long client_max_body_size = 1024;
    EXPECT_EQ(client_max_body_size, conf.get_client_max_body_size(target));
}

TEST_F(ConfigTest, GetHost) {
    SetUp("../conf/valid/01_default.conf");
    config::Config conf      = configs.front();
    const std::string target = "/";
    const std::string host   = "0.0.0.0";
    EXPECT_EQ(host, conf.get_host(target));
}

TEST_F(ConfigTest, GetPort) {
    SetUp("../conf/valid/01_default.conf");
    config::Config conf      = configs.front();
    const std::string target = "/";
    const int port           = 80;
    EXPECT_EQ(port, conf.get_port(target));
}

TEST_F(ConfigTest, GetRedirect) {
    SetUp("../conf/valid/01_default.conf");
    config::Config conf                  = configs.front();
    const std::string target             = "/";
    std::pair<int, std::string> redirect = std::make_pair(-1, "");
    EXPECT_EQ(redirect, conf.get_redirect(target));
}

TEST_F(ConfigTest, GetServerName) {
    SetUp("../conf/valid/01_default.conf");
    config::Config conf      = configs.front();
    const std::string target = "/";
    std::vector<std::string> server_name;
    server_name.push_back("server1");
    EXPECT_EQ(server_name, conf.get_server_name(target));
}

TEST_F(ConfigTest, GetUploadStore) {
    SetUp("../conf/valid/01_default.conf");
    config::Config conf      = configs.front();
    const std::string target = "/";
    const std::string upload_store;
    EXPECT_EQ(upload_store, conf.get_upload_store(target));
}

TEST_F(ConfigTest, GetDefaultServer) {
    SetUp("../conf/valid/01_default.conf");
    config::Config conf       = configs.front();
    const std::string target  = "/";
    const bool default_server = false;
    EXPECT_EQ(default_server, conf.get_default_server(target));
}

TEST_F(ConfigTest, GetLimitExcept) {
    SetUp("../conf/valid/01_default.conf");
    config::Config conf      = configs.front();
    const std::string target = "/";
    std::set<enum config::Methods> limit_except;
    //    limit_except.insert(config::Methods::GET);
    EXPECT_EQ(limit_except, conf.get_limit_except(target));
}

/// 11_mix.conf
TEST_F(ConfigTest, GetAutoIndexFromMixContext) {
    SetUp("../conf/valid/11_mix.conf");
    config::Config conf = configs.front();
    EXPECT_EQ(true, conf.get_autoindex("/"));
    EXPECT_EQ(true, conf.get_autoindex("/dir1"));
    EXPECT_EQ(false, conf.get_autoindex("/dir2"));
    EXPECT_EQ(false, conf.get_autoindex("/dir2/dir3"));
}

TEST_F(ConfigTest, GetErrorPageFromMixContext) {
    SetUp("../conf/valid/11_mix.conf");
    config::Config conf = configs.front();
    std::map<int, std::string> error_page;
    error_page[400] = "error.html";
    EXPECT_EQ(error_page, conf.get_error_page("/"));
    error_page.clear();
    error_page[401] = "error1.html";
    EXPECT_EQ(error_page, conf.get_error_page("/dir1"));
    error_page.clear();
    error_page[402] = "error2.html";
    EXPECT_EQ(error_page, conf.get_error_page("/dir2"));
    error_page.clear();
    error_page[403] = "error3.html";
    EXPECT_EQ(error_page, conf.get_error_page("/dir2/dir3"));
}

TEST_F(ConfigTest, GetIndexFromMixContext) {
    SetUp("../conf/valid/11_mix.conf");
    config::Config conf = configs.front();
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

TEST_F(ConfigTest, GetRootFromMixContext) {
    SetUp("../conf/valid/11_mix.conf");
    config::Config conf = configs.front();
    EXPECT_EQ("/root1", conf.get_root("/dir1"));
    EXPECT_EQ("/root2", conf.get_root("/dir2/"));
    EXPECT_EQ("/root3", conf.get_root("/dir2/dir3"));
}

TEST_F(ConfigTest, GetClientMaxBodySizeFromMixContext) {
    SetUp("../conf/valid/11_mix.conf");
    config::Config conf = configs.front();
    EXPECT_EQ(4242, conf.get_client_max_body_size("/"));
    EXPECT_EQ(1, conf.get_client_max_body_size("/dir1"));
    EXPECT_EQ(2, conf.get_client_max_body_size("/dir2"));
    EXPECT_EQ(3, conf.get_client_max_body_size("/dir2/dir3"));
}

TEST_F(ConfigTest, GetHostFromMixContext) {
    SetUp("../conf/valid/11_mix.conf");
    config::Config conf1     = configs.front();
    config::Config conf2     = configs.back();
    const std::string target = "/";
    EXPECT_EQ("127.0.0.1", conf1.get_host(target));
    EXPECT_EQ("1.1.1.1", conf2.get_host(target));
}

TEST_F(ConfigTest, GetPortFromMixContext) {
    SetUp("../conf/valid/11_mix.conf");
    const std::string target = "/";
    config::Config conf1     = configs.front();
    config::Config conf2     = configs.back();
    EXPECT_EQ(80, conf1.get_port(target));
    EXPECT_EQ(81, conf2.get_port(target));
}

TEST_F(ConfigTest, GetRedirectFromMixContext) {
    SetUp("../conf/valid/11_mix.conf");
    config::Config conf = configs.front();
    EXPECT_EQ(std::make_pair(300, std::string("/")), conf.get_redirect("/"));
    EXPECT_EQ(std::make_pair(300, std::string("/")), conf.get_redirect("/dir1"));
    EXPECT_EQ(std::make_pair(300, std::string("/")), conf.get_redirect("/dir2"));
    EXPECT_EQ(std::make_pair(300, std::string("/")), conf.get_redirect("/dir2/dir3"));
}

TEST_F(ConfigTest, GetServerNameFromMixContext) {
    SetUp("../conf/valid/11_mix.conf");
    config::Config conf = configs.front();
    std::vector<std::string> server_name;
    server_name.push_back("server");
    EXPECT_EQ(server_name, conf.get_server_name("/"));
    EXPECT_EQ(server_name, conf.get_server_name("/dir1"));
}

TEST_F(ConfigTest, GetUploadStoreFromMixContext) {
    SetUp("../conf/valid/11_mix.conf");
    config::Config conf = configs.front();
    EXPECT_EQ("/upload", conf.get_upload_store("/"));
    EXPECT_EQ("/upload1", conf.get_upload_store("/dir1"));
    EXPECT_EQ("/upload2", conf.get_upload_store("/dir2"));
    EXPECT_EQ("/upload3", conf.get_upload_store("/dir2/dir3"));
}

TEST_F(ConfigTest, GetDefaultServerFromMixContext) {
    SetUp("../conf/valid/11_mix.conf");
    config::Config conf1     = configs.front();
    config::Config conf2     = configs.back();
    const std::string target = "/";
    EXPECT_EQ(false, conf1.get_default_server(target));
    EXPECT_EQ(true, conf2.get_default_server(target));
}

TEST_F(ConfigTest, GetLimitExceptFromMixContext) {
    SetUp("../conf/valid/11_mix.conf");
    config::Config conf = configs.front();
    std::set<enum config::Methods> limit_except;
    EXPECT_EQ(limit_except, conf.get_limit_except("/"));
    limit_except.insert(config::Methods::GET);
    EXPECT_EQ(limit_except, conf.get_limit_except("/dir1"));
    limit_except.insert(config::Methods::POST);
    EXPECT_EQ(limit_except, conf.get_limit_except("/dir2"));
    limit_except.insert(config::Methods::DELETE);
    EXPECT_EQ(limit_except, conf.get_limit_except("/dir2/dir3"));
}
} // namespace
