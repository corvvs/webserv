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
        file::ErrorType err;
        if ((err = file::check(path)) != file::NONE) {
            std::cerr << file::error_message(err) << std::endl;
        }
        configs = parser.parse(file::read(path));
    }
    virtual void TearDown() {}

    config::Parser parser;
    std::vector<config::Config> configs;
};

TEST_F(ConfigTest, GetAutoIndex) {
    SetUp("../conf/valid/01_default.conf");
    config::Config conf      = configs.front();
    const std::string target = "/";
    const bool autoindex     = false;
    EXPECT_EQ(conf.get_autoindex(target), autoindex);
}

TEST_F(ConfigTest, GetErrorPage) {
    SetUp("../conf/valid/01_default.conf");
    config::Config conf      = configs.front();
    const std::string target = "/";
    std::map<int, std::string> error_page;
    //    error_page[404] = "";
    EXPECT_EQ(conf.get_error_page(target), error_page);
}

TEST_F(ConfigTest, GetIndex) {
    SetUp("../conf/valid/01_default.conf");
    config::Config conf      = configs.front();
    const std::string target = "/";
    std::vector<std::string> index;
    index.push_back("index.html");
    index.push_back("index.htm");
    EXPECT_EQ(conf.get_index(target), index);
}

TEST_F(ConfigTest, GetRoot) {
    SetUp("../conf/valid/01_default.conf");
    config::Config conf      = configs.front();
    const std::string target = "/";
    const std::string root   = "/data/server1/";
    EXPECT_EQ(conf.get_root(target), root);
}

TEST_F(ConfigTest, GetClientMaxBodySize) {
    SetUp("../conf/valid/01_default.conf");
    config::Config conf             = configs.front();
    const std::string target        = "/";
    const long client_max_body_size = 1024;
    EXPECT_EQ(conf.get_client_max_body_size(target), client_max_body_size);
}

TEST_F(ConfigTest, GetHost) {
    SetUp("../conf/valid/01_default.conf");
    config::Config conf      = configs.front();
    const std::string target = "/";
    const std::string host   = "0.0.0.0";
    EXPECT_EQ(conf.get_host(target), host);
}

TEST_F(ConfigTest, GetPort) {
    SetUp("../conf/valid/01_default.conf");
    config::Config conf      = configs.front();
    const std::string target = "/";
    const int port           = 80;
    EXPECT_EQ(conf.get_port(target), port);
}

TEST_F(ConfigTest, GetRedirect) {
    SetUp("../conf/valid/01_default.conf");
    config::Config conf                  = configs.front();
    const std::string target             = "/";
    std::pair<int, std::string> redirect = std::make_pair(-1, "");
    EXPECT_EQ(conf.get_redirect(target), redirect);
}

TEST_F(ConfigTest, GetServerName) {
    SetUp("../conf/valid/01_default.conf");
    config::Config conf      = configs.front();
    const std::string target = "/";
    std::vector<std::string> server_name;
    server_name.push_back("server1");
    EXPECT_EQ(conf.get_server_name(target), server_name);
}

TEST_F(ConfigTest, GetUploadStore) {
    SetUp("../conf/valid/01_default.conf");
    config::Config conf      = configs.front();
    const std::string target = "/";
    const std::string upload_store;
    EXPECT_EQ(conf.get_upload_store(target), upload_store);
}

TEST_F(ConfigTest, GetDefaultServer) {
    SetUp("../conf/valid/01_default.conf");
    config::Config conf       = configs.front();
    const std::string target  = "/";
    const bool default_server = false;
    EXPECT_EQ(conf.get_default_server(target), default_server);
}

TEST_F(ConfigTest, GetLimitExcept) {
    SetUp("../conf/valid/01_default.conf");
    config::Config conf      = configs.front();
    const std::string target = "/";
    std::set<enum config::Methods> limit_except;
    //    limit_except.insert(config::Methods::GET);
    EXPECT_EQ(conf.get_limit_except(target), limit_except);
}
} // namespace
