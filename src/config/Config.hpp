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
 * default_server       bool
 * redirect             pair<int, string> -> (status, path)
 * server_name          vector<string>
 * upload_store         string
 * limit_except         set<enum> method
 * exec_cgi             bool
 * exec_delete          bool
 * cgi_executers        map<string, string> -> extension, executer_path
 */

namespace config {

class Config;

typedef std::vector<Config> config_vector;
typedef std::map<host_port_pair, config_vector> config_dict;

class Config {
public:
    Config(const ContextServer &srv);
    ~Config(void);

    /// Setter
    void set_host_port(const host_port_pair &hp);
    void set_is_default_server(const bool &flag);

    /// Getter
    std::string get_host() const;
    int get_port() const;
    bool get_default_server() const;
    std::vector<std::string> get_server_name() const;

    bool get_autoindex(const std::string &target) const;
    std::map<HTTP::t_status, std::string> get_error_page(const std::string &target) const;
    std::vector<std::string> get_index(const std::string &target) const;
    std::string get_root(const std::string &target) const;
    long get_client_max_body_size(const std::string &target) const;
    std::pair<HTTP::t_status, std::string> get_redirect(const std::string &target) const;
    std::string get_upload_store(const std::string &target) const;
    std::set<enum Methods> get_limit_except(const std::string &target) const;
    bool get_exec_cgi(const std::string &target) const;
    bool get_exec_delete(const std::string &target) const;
    cgi_executer_map get_cgi_path(const std::string &target) const;

private:
    ContextServer ctx_server_;
    host_port_pair host_port_;
    bool is_default_server_;

    ContextLocation longest_prefix_match_location(const ContextServer &srv, const std::string &path) const;
};

} // namespace config
#endif
