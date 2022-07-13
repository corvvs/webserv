#ifndef PARSER_HPP
#define PARSER_HPP
#include "Config.hpp"
#include "Lexer.hpp"
#include <iostream>
#include <map>
#include <string>
#include <utility>

namespace config {

struct Directive {
    std::string name;
    int line;
    std::vector<std::string> args;
    std::vector<Directive> block; // 入れ子になっている場合に子要素を格納する
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

    std::map<std::string, Parser::add_directive_functions> add_directives_func_map;
    DirectiveFunctionsMap setting_directive_functions(void);

    /// PreParser
    std::vector<ContextServer> parse(std::vector<Directive> vdir);
    std::vector<Directive> pre_parse(std::vector<std::string> ctx = std::vector<std::string>());
    std::vector<std::string> enter_block_ctx(Directive dire, std::vector<std::string> ctx);
    std::string brace_balanced(void);

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
    void print_limit_except(const ContextLimitExcept *lmt);
};
} // namespace config

#endif
