#ifndef PARSER_HPP
#define PARSER_HPP
#include "Config.hpp"
#include "Lexer.hpp"
#include <iostream>
#include <map>
#include <stack>
#include <string>
#include <utility>

namespace config {

struct Directive {
    std::string name;
    int line;
    std::vector<std::string> args;
    std::vector<Directive> block;

    Directive(const std::string &n,
              const int &l,
              std::vector<std::string> a = std::vector<std::string>(),
              std::vector<Directive> b   = std::vector<Directive>())
        : name(n), line(l), args(a), block(b) {}
};

class Parser {
public:
    typedef std::vector<Config> config_vector;
    typedef std::map<host_port_pair, config_vector> config_dict;

public:
    Parser(void);
    ~Parser(void);
    config_dict parse(const std::string &file_data);

private:
    enum ContextType { GLOBAL, MAIN, SERVER, LOCATION, LIMIT_EXCEPT };

    typedef void (Parser::*add_directive_functions)(const std::vector<std::string> &args, std::stack<ContextType> &ctx);
    typedef std::map<std::string, add_directive_functions> DirectiveFunctionsMap;

    /// Member variables
    Lexer lexer_;
    std::vector<ContextServer> ctx_servers_;
    ContextMain ctx_main_;
    DirectiveFunctionsMap adder_maps;

    /// Member functions
    config_dict create_configs(const std::vector<ContextServer> &ctx_servers);
    DirectiveFunctionsMap setting_directive_functions(void);
    std::string brace_balanced(void);

    std::vector<ContextServer> forestize(std::vector<Directive> vdir, std::stack<ContextType> &ctx);
    std::vector<Directive> clusterize(std::vector<std::string> ctx = std::vector<std::string>());
    std::vector<std::string> enter_block_ctx(Directive dire, std::vector<std::string> ctx);
    bool is_special(const std::string &s) const;

    /// Adder
    void add_http(const std::vector<std::string> &args, std::stack<ContextType> &ctx);
    void add_server(const std::vector<std::string> &args, std::stack<ContextType> &ctx);
    void add_location(const std::vector<std::string> &args, std::stack<ContextType> &ctx);
    void add_limit_except(const std::vector<std::string> &args, std::stack<ContextType> &ctx);
    void add_autoindex(const std::vector<std::string> &args, std::stack<ContextType> &ctx);
    void add_error_page(const std::vector<std::string> &args, std::stack<ContextType> &ctx);
    void add_index(const std::vector<std::string> &args, std::stack<ContextType> &ctx);
    void add_listen(const std::vector<std::string> &args, std::stack<ContextType> &ctx);
    void add_return(const std::vector<std::string> &args, std::stack<ContextType> &ctx);
    void add_root(const std::vector<std::string> &args, std::stack<ContextType> &ctx);
    void add_server_name(const std::vector<std::string> &args, std::stack<ContextType> &ctx);
    void add_client_max_body_size(const std::vector<std::string> &args, std::stack<ContextType> &ctx);
    void add_upload_store(const std::vector<std::string> &args, std::stack<ContextType> &ctx);
    void add_exec_cgi(const std::vector<std::string> &args, std::stack<ContextType> &ctx);
    void add_exec_delete(const std::vector<std::string> &args, std::stack<ContextType> &ctx);
    void add_cgi_path(const std::vector<std::string> &args, std::stack<ContextType> &ctx);

    bool is_conflicted_server_name(const std::vector<ContextServer> &servers);
    size_t count_nested_locations(const std::stack<ContextType> &ctx) const;
    ContextLocation *get_current_location(const std::stack<ContextType> &ctx);

    void inherit_data(std::vector<ContextServer> &servers);
    void inherit_locations(const ContextLocation &parent, std::vector<ContextLocation> &locs);
    void inherit_main_to_srv(const ContextMain &main, ContextServer &srv);
    void inherit_loc_to_loc(const ContextLocation &parent, ContextLocation &child);

    /// Debug

    void print_directives(const std::vector<Directive> &d,
                          const bool &is_block      = false,
                          const std::string &before = "") const;
    void print_location(const std::vector<ContextLocation> &loc) const;
    void print_server(const ContextServer &serv) const;
    void print_limit_except(const ContextLimitExcept &lmt) const;
    void debug_print(const std::vector<ContextServer> &servers) const;
};
} // namespace config

#endif
