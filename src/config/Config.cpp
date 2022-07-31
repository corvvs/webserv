#include "Config.hpp"
#include <stack>

namespace config {

Config::Config(const ContextServer &srv) : ctx_server_(srv){};
Config::~Config(void){};

void Config::set_host_port(const host_port_pair &hp) {
    host_port_ = hp;
}

void Config::set_is_default_server(const bool &flag) {
    is_default_server_ = flag;
}

bool Config::get_autoindex(const std::string &target) const {
    const ContextLocation &loc = longest_prefix_match_location(ctx_server_, target);
    return loc.autoindex;
}

std::map<int, std::string> Config::get_error_page(const std::string &target) const {
    const ContextLocation &loc = longest_prefix_match_location(ctx_server_, target);
    return loc.error_pages;
}

std::vector<std::string> Config::get_index(const std::string &target) const {
    const ContextLocation &loc = longest_prefix_match_location(ctx_server_, target);

    if (loc.indexes.empty()) {
        std::vector<std::string> res;
        res.push_back("index.html");
        return res;
    }
    return loc.indexes;
}

std::string Config::get_root(const std::string &target) const {
    const ContextLocation &loc = longest_prefix_match_location(ctx_server_, target);
    return loc.root;
}

long Config::get_client_max_body_size(const std::string &target) const {
    const ContextLocation &loc = longest_prefix_match_location(ctx_server_, target);
    return loc.client_max_body_size;
}

std::string Config::get_host(const std::string &target) const {
    (void)target;
    return host_port_.first;
}

int Config::get_port(const std::string &target) const {
    (void)target;
    return host_port_.second;
}

std::pair<int, std::string> Config::get_redirect(const std::string &target) const {
    const ContextLocation &loc = longest_prefix_match_location(ctx_server_, target);
    return loc.redirect;
}

std::set<enum Methods> Config::get_limit_except(const std::string &target) const {
    const ContextLocation &loc = longest_prefix_match_location(ctx_server_, target);
    return loc.limit_except.allowed_methods;
}

std::vector<std::string> Config::get_server_name(const std::string &target) const {
    (void)target;
    return ctx_server_.server_names;
}

std::string Config::get_upload_store(const std::string &target) const {
    const ContextLocation &loc = longest_prefix_match_location(ctx_server_, target);
    return loc.upload_store;
}

bool Config::get_default_server(const std::string &target) const {
    (void)target;
    return is_default_server_;
}

bool Config::get_exec_cgi(const std::string &target) const {
    const ContextLocation &loc = longest_prefix_match_location(ctx_server_, target);
    return loc.exec_cgi;
}

bool Config::get_exec_delete(const std::string &target) const {
    const ContextLocation &loc = longest_prefix_match_location(ctx_server_, target);
    return loc.exec_delete;
}

std::map<extension, executer_path> Config::get_cgi_path(const std::string &target) const {
    const ContextLocation &loc = longest_prefix_match_location(ctx_server_, target);
    return loc.cgi_paths;
}

/**
 * サーバーコンテキストの中で最長前方一致するロケーションを返す
 * 一致しない場合はサーバーコンテキストの情報をそのまま継承したロケーションを返す
 */
ContextLocation Config::longest_prefix_match_location(const ContextServer &srv, const std::string &path) const {
    ContextLocation longest(srv);

    std::stack<ContextLocation> sta;
    for (std::vector<ContextLocation>::const_iterator it = srv.locations.begin(); it != srv.locations.end(); ++it) {
        sta.push(*it);
    }

    while (!sta.empty()) {
        ContextLocation cur = sta.top();
        sta.pop();
        // 一致していたら子要素をstackに積む
        if (path.find(cur.path) == 0) {
            std::vector<ContextLocation>::const_iterator it = cur.locations.begin();
            for (; it != cur.locations.end(); ++it) {
                sta.push(*it);
            }
            // マッチしてる部分が長い場合は更新する
            if (longest.path.size() < cur.path.size()) {
                longest = cur;
            }
        }
    }
    return longest;
}
} // namespace config
