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
    Parser(void);
    ~Parser(void);
    std::vector<Config> Parse(const std::string &file_data);

private:
    enum ContextType {
        GLOBAL,
        MAIN,
        SERVER,
        LOCATION,
        LIMIT_EXCEPT,
    };

    typedef void (Parser::*add_directive_functions)(const std::vector<std::string> &args);
    typedef std::map<std::string, add_directive_functions> DirectiveFunctionsMap;

    /// Member variables
    Lexer lexer_;
    std::vector<ContextServer> ctx_servers_;
    ContextMain ctx_main_;
    ContextType ctx_;
    std::stack<ContextType> ctx_stack_;

    std::map<std::string, Parser::add_directive_functions> add_directives_func_map;
    DirectiveFunctionsMap setting_directive_functions(void);

    /// Member functions
    std::string brace_balanced(void);

    /// Analyze
    std::vector<ContextServer> parse(std::vector<Directive> vdir);
    std::vector<Directive> analyze(std::vector<std::string> ctx = std::vector<std::string>());
    std::vector<std::string> enter_block_ctx(Directive dire, std::vector<std::string> ctx);
    bool is_special(const std::string &s) const;

    bool is_conflicted_server_name(const std::vector<ContextServer> &servers);
    size_t count_nested_locations(void) const;
    ContextLocation *get_current_location(void);

    void inherit_data(std::vector<ContextServer> &servers);
    void inherit_locations(const ContextLocation &parent, std::vector<ContextLocation> &locs);
    void inherit_main_to_srv(const ContextMain &main, ContextServer &srv);
    void inherit_loc_to_loc(const ContextLocation &parent, ContextLocation &child);

    /// Block
    void add_http(const std::vector<std::string> &args);
    void add_server(const std::vector<std::string> &args);
    void add_location(const std::vector<std::string> &args);
    void add_limit_except(const std::vector<std::string> &args);

    /// Simple
    void add_autoindex(const std::vector<std::string> &args);
    void add_error_page(const std::vector<std::string> &args);
    void add_index(const std::vector<std::string> &args);
    void add_listen(const std::vector<std::string> &args);
    void add_return(const std::vector<std::string> &args);
    void add_root(const std::vector<std::string> &args);
    void add_server_name(const std::vector<std::string> &args);
    void add_client_max_body_size(const std::vector<std::string> &args);

    /// Original
    void add_upload_store(const std::vector<std::string> &args);

    /// Debug
    void
    print_directives(const std::vector<Directive> &d, const bool &is_block = false, const std::string &before = "");
    void print_location(const std::vector<ContextLocation> &loc);
    void print_server(const ContextServer &serv);
    void print_limit_except(const ContextLimitExcept &lmt);
};
} // namespace config

#endif
