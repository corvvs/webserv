#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "Context.hpp"
#include <map>
#include <set>
#include <string>
#include <vector>
/**
 * [directives]
 * autoindex            bool
 * error_page           vector<map<int, string> > -> (status, path)
 * index                vector<string>
 * root                 string
 * client_max_body_size long
 * host_port            pair<string, int>
 * redirect             pair<int, string> -> (status, path)
 * server_name          vector<string>
 * upload_store         string
 * default_server       bool
 * limit_except         set<enum> method
 */

namespace config {

class Config {
public:
    Config(const ContextServer &srv);
    ~Config(void);

    /// Setter
    void set_host_port(const host_port_pair &hp);

    /// Getter
    bool get_autoindex(const std::string &target) const;
    std::map<int, std::string> get_error_page(const std::string &target) const;
    std::vector<std::string> get_index(const std::string &target) const;
    std::string get_root(const std::string &target) const;
    long get_client_max_body_size(const std::string &target) const;
    std::string get_host(const std::string &target) const;
    int get_port(const std::string &target) const;
    std::pair<int, std::string> get_redirect(const std::string &target) const;
    std::vector<std::string> get_server_name(const std::string &target) const;
    std::string get_upload_store(const std::string &target) const;
    bool get_default_server(const std::string &target) const;
    std::set<enum Methods> get_limit_except(const std::string &target) const;

private:
    ContextServer ctx_server_;

    // configに対応するhostとportのペア
    host_port_pair host_port_;

    ContextLocation longest_prefix_match_location(const ContextServer &srv, const std::string &path) const;
};

} // namespace config
#endif
