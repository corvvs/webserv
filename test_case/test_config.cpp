#include "../../src/config/Context.hpp"
#include "../../src/config/File.hpp"
#include "../../src/config/Lexer.hpp"
#include "../../src/config/Parser.hpp"
#include "gtest/gtest.h"
#include <iostream>
#include <vector>

namespace {
class ConfigBasedOnFile : public testing::Test {
protected:
    ConfigBasedOnFile() {}
    virtual ~ConfigBasedOnFile() {}
    void SetUpBasedOnFile(const std::string &path) {
        assert(file::check(path) == file::NONE);
        configs = parser.parse(file::read(path));
    }
    virtual void TearDown() {}

    config::Parser parser;
    std::map<config::host_port_pair, std::vector<config::Config> > configs;
};

/// 01_default.conf
TEST_F(ConfigBasedOnFile, GetAutoIndex) {
    SetUpBasedOnFile("./conf/valid/01_default.conf");
    const config::host_port_pair hp = std::make_pair("0.0.0.0", 80);
    const config::Config conf       = configs[hp].front();

    const std::string target = "/";
    const bool autoindex     = false;
    EXPECT_EQ(autoindex, conf.get_autoindex(target));
}

TEST_F(ConfigBasedOnFile, GetErrorPage) {
    SetUpBasedOnFile("./conf/valid/01_default.conf");
    const config::host_port_pair hp = std::make_pair("0.0.0.0", 80);
    const config::Config conf       = configs[hp].front();

    const std::string target = "/";
    std::map<int, std::string> error_page;
    //    error_page[404] = "";
    EXPECT_EQ(error_page, conf.get_error_page(target));
}

TEST_F(ConfigBasedOnFile, GetIndex) {
    SetUpBasedOnFile("./conf/valid/01_default.conf");
    const config::host_port_pair hp = std::make_pair("0.0.0.0", 80);
    const config::Config conf       = configs[hp].front();

    const std::string target = "/";
    std::vector<std::string> index;
    index.push_back("index.html");
    index.push_back("index.htm");
    EXPECT_EQ(index, conf.get_index(target));
}

TEST_F(ConfigBasedOnFile, GetRoot) {
    SetUpBasedOnFile("./conf/valid/01_default.conf");
    const config::host_port_pair hp = std::make_pair("0.0.0.0", 80);
    const config::Config conf       = configs[hp].front();

    const std::string target = "/";
    const std::string root   = "/data/server1/";
    EXPECT_EQ(root, conf.get_root(target));
}

TEST_F(ConfigBasedOnFile, GetClientMaxBodySize) {
    SetUpBasedOnFile("./conf/valid/01_default.conf");
    const config::host_port_pair hp = std::make_pair("0.0.0.0", 80);
    const config::Config conf       = configs[hp].front();

    const std::string target        = "/";
    const long client_max_body_size = 1024;
    EXPECT_EQ(client_max_body_size, conf.get_client_max_body_size(target));
}

TEST_F(ConfigBasedOnFile, GetHost) {
    SetUpBasedOnFile("./conf/valid/01_default.conf");
    const config::host_port_pair hp = std::make_pair("0.0.0.0", 80);
    const config::Config conf       = configs[hp].front();

    const std::string target = "/";
    const std::string host   = "0.0.0.0";
    EXPECT_EQ(host, conf.get_host(target));
}

TEST_F(ConfigBasedOnFile, GetPort) {
    SetUpBasedOnFile("./conf/valid/01_default.conf");
    const config::host_port_pair hp = std::make_pair("0.0.0.0", 80);
    const config::Config conf       = configs[hp].front();

    const std::string target = "/";
    const int port           = 80;
    EXPECT_EQ(port, conf.get_port(target));
}

TEST_F(ConfigBasedOnFile, GetRedirect) {
    SetUpBasedOnFile("./conf/valid/01_default.conf");
    const config::host_port_pair hp = std::make_pair("0.0.0.0", 80);
    const config::Config conf       = configs[hp].front();

    const std::string target             = "/";
    std::pair<int, std::string> redirect = std::make_pair(-1, "");
    EXPECT_EQ(redirect, conf.get_redirect(target));
}

TEST_F(ConfigBasedOnFile, GetServerName) {
    SetUpBasedOnFile("./conf/valid/01_default.conf");
    const config::host_port_pair hp = std::make_pair("0.0.0.0", 80);
    const config::Config conf       = configs[hp].front();

    const std::string target = "/";
    std::vector<std::string> server_name;
    server_name.push_back("server1");
    EXPECT_EQ(server_name, conf.get_server_name(target));
}

TEST_F(ConfigBasedOnFile, GetUploadStore) {
    SetUpBasedOnFile("./conf/valid/01_default.conf");
    const config::host_port_pair hp = std::make_pair("0.0.0.0", 80);
    const config::Config conf       = configs[hp].front();

    const std::string target = "/";
    const std::string upload_store;
    EXPECT_EQ(upload_store, conf.get_upload_store(target));
}

TEST_F(ConfigBasedOnFile, GetDefaultServer) {
    SetUpBasedOnFile("./conf/valid/01_default.conf");
    const config::host_port_pair hp = std::make_pair("0.0.0.0", 80);
    const config::Config conf       = configs[hp].front();

    const std::string target  = "/";
    const bool default_server = false;
    EXPECT_EQ(default_server, conf.get_default_server(target));
}

TEST_F(ConfigBasedOnFile, GetLimitExcept) {
    SetUpBasedOnFile("./conf/valid/01_default.conf");
    const config::host_port_pair hp = std::make_pair("0.0.0.0", 80);
    const config::Config conf       = configs[hp].front();

    const std::string target = "/";
    std::set<enum config::Methods> limit_except;
    //    limit_except.insert(config::Methods::GET);
    EXPECT_EQ(limit_except, conf.get_limit_except(target));
}

/// 11_mix.conf
TEST_F(ConfigBasedOnFile, GetAutoIndexFromMixContext) {
    SetUpBasedOnFile("./conf/valid/11_mix.conf");
    const config::host_port_pair hp = std::make_pair("127.0.0.1", 80);
    const config::Config conf       = configs[hp].front();

    EXPECT_EQ(true, conf.get_autoindex("/"));
    EXPECT_EQ(true, conf.get_autoindex("/dir1"));
    EXPECT_EQ(false, conf.get_autoindex("/dir2"));
    EXPECT_EQ(false, conf.get_autoindex("/dir2/dir3"));
}

TEST_F(ConfigBasedOnFile, GetErrorPageFromMixContext) {
    SetUpBasedOnFile("./conf/valid/11_mix.conf");
    const config::host_port_pair hp = std::make_pair("127.0.0.1", 80);
    const config::Config conf       = configs[hp].front();

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

TEST_F(ConfigBasedOnFile, GetIndexFromMixContext) {
    SetUpBasedOnFile("./conf/valid/11_mix.conf");
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

TEST_F(ConfigBasedOnFile, GetRootFromMixContext) {
    SetUpBasedOnFile("./conf/valid/11_mix.conf");
    const config::host_port_pair hp = std::make_pair("127.0.0.1", 80);
    const config::Config conf       = configs[hp].front();

    EXPECT_EQ("/root1", conf.get_root("/dir1"));
    EXPECT_EQ("/root2", conf.get_root("/dir2/"));
    EXPECT_EQ("/root3", conf.get_root("/dir2/dir3"));
}

TEST_F(ConfigBasedOnFile, GetClientMaxBodySizeFromMixContext) {
    SetUpBasedOnFile("./conf/valid/11_mix.conf");
    const config::host_port_pair hp = std::make_pair("127.0.0.1", 80);
    const config::Config conf       = configs[hp].front();

    EXPECT_EQ(4242, conf.get_client_max_body_size("/"));
    EXPECT_EQ(1, conf.get_client_max_body_size("/dir1"));
    EXPECT_EQ(2, conf.get_client_max_body_size("/dir2"));
    EXPECT_EQ(3, conf.get_client_max_body_size("/dir2/dir3"));
}

TEST_F(ConfigBasedOnFile, GetHostFromMixContext) {
    SetUpBasedOnFile("./conf/valid/11_mix.conf");
    const std::string target = "/";
    {
        const config::host_port_pair hp = std::make_pair("127.0.0.1", 80);
        const config::Config conf       = configs[hp].front();
        EXPECT_EQ("127.0.0.1", conf.get_host(target));
    }
    {
        const config::host_port_pair hp = std::make_pair("1.1.1.1", 81);
        const config::Config conf       = configs[hp].back();
        EXPECT_EQ("1.1.1.1", conf.get_host(target));
    }
}

TEST_F(ConfigBasedOnFile, GetPortFromMixContext) {
    SetUpBasedOnFile("./conf/valid/11_mix.conf");
    const std::string target = "/";
    {
        const config::host_port_pair hp = std::make_pair("127.0.0.1", 80);
        const config::Config conf       = configs[hp].front();
        EXPECT_EQ(80, conf.get_port(target));
    }
    {
        const config::host_port_pair hp = std::make_pair("1.1.1.1", 81);
        const config::Config conf       = configs[hp].front();
        EXPECT_EQ(81, conf.get_port(target));
    }
}

TEST_F(ConfigBasedOnFile, GetRedirectFromMixContext) {
    SetUpBasedOnFile("./conf/valid/11_mix.conf");
    const config::host_port_pair hp = std::make_pair("127.0.0.1", 80);
    const config::Config conf       = configs[hp].front();

    EXPECT_EQ(std::make_pair(300, std::string("/")), conf.get_redirect("/"));
    EXPECT_EQ(std::make_pair(300, std::string("/")), conf.get_redirect("/dir1"));
    EXPECT_EQ(std::make_pair(300, std::string("/")), conf.get_redirect("/dir2"));
    EXPECT_EQ(std::make_pair(300, std::string("/")), conf.get_redirect("/dir2/dir3"));
}

TEST_F(ConfigBasedOnFile, GetServerNameFromMixContext) {
    SetUpBasedOnFile("./conf/valid/11_mix.conf");
    const config::host_port_pair hp = std::make_pair("127.0.0.1", 80);
    const config::Config conf       = configs[hp].front();

    std::vector<std::string> server_name;
    server_name.push_back("server");
    EXPECT_EQ(server_name, conf.get_server_name("/"));
    EXPECT_EQ(server_name, conf.get_server_name("/dir1"));
}

TEST_F(ConfigBasedOnFile, GetUploadStoreFromMixContext) {
    SetUpBasedOnFile("./conf/valid/11_mix.conf");
    const config::host_port_pair hp = std::make_pair("127.0.0.1", 80);
    const config::Config conf       = configs[hp].front();

    EXPECT_EQ("/upload", conf.get_upload_store("/"));
    EXPECT_EQ("/upload1", conf.get_upload_store("/dir1"));
    EXPECT_EQ("/upload2", conf.get_upload_store("/dir2"));
    EXPECT_EQ("/upload3", conf.get_upload_store("/dir2/dir3"));
}

TEST_F(ConfigBasedOnFile, GetDefaultServerFromMixContext) {
    SetUpBasedOnFile("./conf/valid/11_mix.conf");
    const std::string target = "/";
    {
        const config::host_port_pair hp = std::make_pair("127.0.0.1", 80);
        const config::Config conf       = configs[hp].front();
        EXPECT_EQ(false, conf.get_default_server(target));
    }
    {
        const config::host_port_pair hp = std::make_pair("1.1.1.1", 81);
        const config::Config conf       = configs[hp].front();
        EXPECT_EQ(81, conf.get_port(target));
        EXPECT_EQ(true, conf.get_default_server(target));
    }
}

TEST_F(ConfigBasedOnFile, GetLimitExceptFromMixContext) {
    SetUpBasedOnFile("./conf/valid/11_mix.conf");
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
