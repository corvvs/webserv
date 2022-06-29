#ifndef PARSER_HPP
#define PARSER_HPP
#include "Lexer.hpp"
#include <iostream>
#include <string>
#include <utility>

struct Directive {
    std::string directive;
    int line;
    std::vector<std::string> args;
    std::vector<Directive> block; // 入れ子になっている場合に子要素を格納する

    Directive(std::string d,
              int l,
              std::vector<std::string> a = std::vector<std::string>(),
              std::vector<Directive> b   = std::vector<Directive>())
        : directive(d), line(l), args(a), block(b) {}
};

// DEBUG用
void print(std::vector<Directive> d, bool is_block = false, std::string before = "");

class ConfigValidationException : public std::runtime_error {
public:
    ConfigValidationException(void) : std::runtime_error("config: validation error") {}
    ConfigValidationException(const std::string &what) : std::runtime_error(what) {}
};

class Parser {
public:
    Parser();
    ~Parser();

    std::vector<Directive> Parse(std::string filename);
    std::vector<Directive> parse(std::vector<std::string> ctx = std::vector<std::string>());

private:
    // Member variables
    Lexer lexer_;

    static const std::string None;
    std::vector<std::string> enter_block_ctx(Directive dire, std::vector<std::string> ctx);
    std::string brace_balanced(void);
};

#endif
