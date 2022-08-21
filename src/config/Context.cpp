#include "Context.hpp"
#include "../utils/File.hpp"
#include <sys/stat.h>
namespace config {

static cgi_executer_map setup_executer_map();
static const cgi_executer_map default_executers = setup_executer_map();

static std::map<HTTP::t_status, std::string> setup_default_error_pages() {
    std::map<HTTP::t_status, std::string> error_pages;
    error_pages[HTTP::STATUS_NOT_FOUND]             = "./error_page/404.html";
    error_pages[HTTP::STATUS_METHOD_NOT_ALLOWED]    = "./error_page/405.html";
    error_pages[HTTP::STATUS_INTERNAL_SERVER_ERROR] = "./error_page/500.html";
    return error_pages;
}

ContextMain::ContextMain() {
    client_max_body_size = 1024;
    autoindex            = false;
    error_pages          = setup_default_error_pages();
}
ContextMain::~ContextMain() {}

ContextServer::ContextServer() : redirect(std::make_pair(REDIRECT_INITIAL_VALUE, "")) {
    error_pages = setup_default_error_pages();
}
ContextServer::~ContextServer() {}

ContextLocation::ContextLocation()
    : redirect(std::make_pair(REDIRECT_INITIAL_VALUE, "")), cgi_executers(default_executers) {
    error_pages = setup_default_error_pages();
}

ContextLocation::ContextLocation(const ContextServer &server) {
    client_max_body_size = server.client_max_body_size;
    autoindex            = server.autoindex;
    root                 = server.root;
    indexes              = server.indexes;
    error_pages          = server.error_pages;
    redirect             = server.redirect;
    upload_store         = server.upload_store;
    exec_cgi             = false;
    exec_delete          = false;
    cgi_executers        = default_executers;
}
ContextLocation::~ContextLocation() {}

ContextLimitExcept::ContextLimitExcept() {}
ContextLimitExcept::~ContextLimitExcept() {}

// 事前に探しておくCGIのエグゼキュータのリスト
static executer_vector setup_search_executers() {
    executer_vector v;
    v.push_back(std::make_pair(".rb", "ruby"));
    v.push_back(std::make_pair(".py", "python"));
    v.push_back(std::make_pair(".pl", "perl"));
    v.push_back(std::make_pair(".php", "php"));
    v.push_back(std::make_pair(".go", "go"));
    return v;
}

static cgi_executer_map setup_executer_map() {
    executer_vector search_executers = setup_search_executers();

    cgi_executer_map executers;
    std::string s  = std::getenv("PATH");
    byte_string ss = HTTP::strfy(s);
    light_string path_env(ss);
    std::vector<light_string> paths = path_env.split(":");

    for (size_t i = 0; i < paths.size(); ++i) {
        for (size_t j = 0; j < search_executers.size(); ++j) {
            const std::string executer = search_executers[j].second;
            const std::string path     = HTTP::restrfy(paths[i].str()) + "/" + executer;

            // 実行可能な場合、cgiのpathを追加する
            if (file::is_executable(path)) {
                extension_type extension = search_executers[j].first;
                executers[extension]     = path;
            }
        }
    }
    return executers;
}

} // namespace config
