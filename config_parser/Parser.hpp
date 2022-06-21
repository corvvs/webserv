#ifndef PARSER_HPP
#define PARSER_HPP
#include <string>
#include <iostream>
#include "Lexer.hpp"

struct Directive
{
    std::string directive;
    int line;
    std::vector<std::string> args;
    std::vector<int> includes;
    std::vector<Directive> block;
    std::string comment;
};

class Parser
{
public:
    Parser();
    ~Parser();

    static void Parse(std::string filename);
    ///    static std::vector<Directive> parse(std::vector<ngxToken> tokens, bool consume);
    static std::vector<Directive> parse(std::vector<ngxToken> tokens, std::vector<std::string> ctx, bool consume, int advance);

private:
};

#endif
