#include "Lexer.hpp"
#include "Parser.hpp"
#include <iostream>
#include <vector>

int main()
{
    std::string filename = "./conf/01_default.conf";
    //     std::string filename = "./conf/02_unexpected_brace.conf";
    //     std::string filename = "./conf/03_unexpected_eof.conf";
    //     std::string filename = "./conf/03_unexpected_eof.conf";
    //     std::string filename = "./conf/04_wrong_comment.conf";
    //     std::string filename = "./conf/05_wrong_quote.conf";

    // Lexer lexer;
    // lexer.lex(filename);
    Parser parser;
    parser.Parse(filename);
}
