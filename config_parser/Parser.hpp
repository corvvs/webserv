#ifndef PARSER_HPP
#define PARSER_HPP
#include <string>
#include <iostream>
#include "Lexer.hpp"
#include <utility>

struct Directive
{
    std::string directive;
    int line;
    std::vector<std::string> args;
    std::vector<Directive> block; // 入れ子になっている場合に子要素を格納する

    Directive(std::string d, int l, std::vector<std::string> a = std::vector<std::string>(), std::vector<Directive> b = std::vector<Directive>())
        : directive(d), line(l), args(a), block(b)
    {
    }
};

class Parser
{
public:
    Parser();
    ~Parser();

    void Parse(std::string filename);
    std::vector<Directive> parse(std::vector<std::string> ctx);

private:
    Lexer lexer_;
};

#endif
