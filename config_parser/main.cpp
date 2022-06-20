#include "Lexer.hpp"
#include <iostream>
#include <vector>

int main()
{
    std::string filename = "./conf/01_default.conf";
    //    std::string filename = "./conf/02_unexpected_brace.conf";
    //    std::string filename = "./conf/03_unexpected_eof.conf";

    Lexer lexer;
    lexer.lex(filename);
}
