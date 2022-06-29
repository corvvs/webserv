#ifndef PARSER_HPP
#define PARSER_HPP
#include "Lexer.hpp"
#include <iostream>
#include <string>
#include <utility>

namespace config {



struct Directive {
    std::string directive;
    int line;
    std::vector<std::string> args;
    std::vector<Directive> block; // 入れ子になっている場合に子要素を格納する
};

// DEBUG用
void print(std::vector<Directive> d, bool is_block = false, std::string before = "");

class Parser {
public:
    Parser();
    ~Parser();
    std::vector<Directive> Parse(std::string filename);

private:
    // Member variables
    Lexer lexer_;

    std::vector<Directive> parse(std::vector<std::string> ctx = std::vector<std::string>());
    std::vector<std::string> enter_block_ctx(Directive dire, std::vector<std::string> ctx);
    std::string brace_balanced(void);
};
} // namespace config
#endif
