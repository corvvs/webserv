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
    std::string comment;
    //    std::vector<int> includes;
    std::vector<Directive> block; // 入れ子になっている場合に子要素を格納する
};

class Parser
{
public:
    Parser();
    ~Parser();

    static void Parse(std::string filename);
    static std::vector<Directive> parse(std::vector<ngxToken> &tokens, std::vector<std::string> ctx);

private:
};

#endif
