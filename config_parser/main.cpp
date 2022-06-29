#include "Lexer.hpp"
#include "Parser.hpp"
#include "Validator.hpp"
#include "test_common.hpp"
#include <iostream>
#include <vector>

void test_lexer(const std::string &filename) {
    config::Lexer lexer;

    try {
        lexer.lex(filename);
    } catch (const std::runtime_error &e) {
        std::cout << e.what() << std::endl;
        return;
    }

#ifdef DEBUG
    std::cout << "===============LEX===============" << std::endl;
    Lexer::wsToken *token;
    while ((token = lexer.read()) != NULL) {
        std::cout << *token << std::endl;
    }
#else
    std::cout << "lexer : the configuration file " + filename + " syntax is ok" << std::endl;
#endif
}

void test_parser(const std::string &filename) {
    std::vector<config::Directive> parsed;
    config::Parser parser;
    try {
        parsed = parser.Parse(filename);
    } catch (const config::SyntaxError &e) {
        std::cout << e.what() << std::endl;
        return;
    }

#ifdef DEBUG
    std::cout << "==============PARSE==============" << std::endl;
    print(parsed);
#else
    std::cout << "parser: the configuration file " + filename + " syntax is ok" << std::endl;
#endif
}

int main(int argc, char **argv) {
    if (argc < 2) {
        return 0;
    }
    const char *filename = argv[1];

    test_lexer(filename);
    test_parser(filename);
    return 0;
}
