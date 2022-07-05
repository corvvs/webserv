#ifndef PARSER_HPP
#define PARSER_HPP
#include "Config.hpp"
#include "Lexer.hpp"
#include <iostream>
#include <string>
#include <utility>

namespace config {

struct Directive {
    std::string name;
    int line;
    std::vector<std::string> args;
    std::vector<Directive> block; // 入れ子になっている場合に子要素を格納する
};

// DEBUG用
void print(std::vector<Directive> d, bool is_block = false, std::string before = "");

class Parser {
public:
    Parser(void);
    ~Parser(void);
    std::vector<Directive> Parse(const std::string &file_data);

private:
    enum ContextType {
        GLOBAL,
        MAIN,
        SERVER,
        LOCATION,
        LIMIT_EXCEPT,
    };
    // Member variables
    Lexer lexer_;
    std::vector<ContextServer> ctx_servers_;
    ContextMain ctx_main_;

    std::vector<ContextServer> parse(std::vector<Directive> vdir);
    std::vector<Directive> pre_parse(std::vector<std::string> ctx = std::vector<std::string>());
    std::vector<std::string> enter_block_ctx(Directive dire, std::vector<std::string> ctx);
    std::string brace_balanced(void);

    /// Add functions
    void add_http(const enum ContextType &ctx, const std::vector<std::string> &args);
    void add_server(const enum ContextType &ctx, const std::vector<std::string> &args);
    void add_location(const enum ContextType &ctx, const std::vector<std::string> &args);
    void add_limit_except(const enum ContextType &ctx, const std::vector<std::string> &args);

    /// Normal
    void add_allow(const enum ContextType &ctx, const std::vector<std::string> &args);
    void add_deny(const enum ContextType &ctx, const std::vector<std::string> &args);

    void add_autoindex(const enum ContextType &ctx, const std::vector<std::string> &args);
    void add_error_page(const enum ContextType &ctx, const std::vector<std::string> &args);
    void add_index(const enum ContextType &ctx, const std::vector<std::string> &args);
    void add_listen(const enum ContextType &ctx, const std::vector<std::string> &args);
    void add_return(const enum ContextType &ctx, const std::vector<std::string> &args);
    void add_root(const enum ContextType &ctx, const std::vector<std::string> &args);
    void add_server_name(const enum ContextType &ctx, const std::vector<std::string> &args);

    void add_client_max_body_size(const enum ContextType &ctx, const std::vector<std::string> &args);

    /// Original
    void add_upload_store(const enum ContextType &ctx, const std::vector<std::string> &args);

    void print(const std::vector<ContextLocation> &loc);
    void print(const ContextServer &serv);
};
} // namespace config
#endif
