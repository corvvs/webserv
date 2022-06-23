#include "Lexer.hpp"
#include "Parser.hpp"
#include "test_common.hpp"
#include <iostream>
#include <vector>

void test_lexer(const std::string &filename)
{
    Lexer lexer;
    lexer.lex(filename);

    wsToken *token;
    std::cout << "===============LEX===============" << std::endl;
    while ((token = lexer.read()) != NULL)
    {
        std::cout << *token << std::endl;
    }
}

void test_parser(const std::string &filename)
{
    std::vector<Directive> parsed;
    Parser parser;
    parsed = parser.Parse(filename);

    std::cout << "==============PARSE==============" << std::endl;
    print_parsed_data(parsed);
}

int main()
{
    //     std::string filename = "./conf/valid/01_default.conf";
    //     std::string filename = "./conf/invalid/02_unexpected_brace.conf";
    //     std::string filename = "./conf/invalid/03_unexpected_eof.conf";
    //     std::string filename = "./conf/invalid/04_wrong_comment.conf";
    //     std::string filename = "./conf/invalid/05_wrong_quote.conf";

    std::string filename = "./conf/test.conf";

    test_lexer(filename);
    test_parser(filename);
}
