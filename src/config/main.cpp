#include "ConfigParser.hpp"
#include "File.hpp"
#include "Lexer.hpp"
#include "Parser.hpp"
#include "Validator.hpp"
#include "test_common.hpp"
#include <iostream>
#include <vector>

void test_lexer(const std::string &path) {
    file::error_type err;
    if ((err = file::check(path)) != file::NONE) {
        std::cerr << file::error_message(err) << std::endl;
        return;
    }

    std::string file_data = file::read(path);
    config::Lexer lexer;
    try {
        lexer.tokenize(file_data);
    } catch (const std::runtime_error &e) {
        std::cout << e.what() << std::endl;
        return;
    }

#ifdef DEBUG
    std::cout << "===============LEX===============" << std::endl;
    config::wsToken *token;
    while ((token = lexer.read()) != NULL) {
        std::cout << *token << std::endl;
    }
#else
    std::cout << "lexer : the configuration file " + path + " syntax is ok" << std::endl;
#endif
}

void test_parser(const std::string &path) {
    file::error_type err;
    if ((err = file::check(path)) != file::NONE) {
        std::cerr << file::error_message(err) << std::endl;
        return;
    }

    std::string file_data = file::read(path);
    std::vector<config::Directive> parsed;
    config::Parser parser;
    try {
        parsed = parser.Parse(file_data);
    } catch (const config::SyntaxError &e) {
        std::cout << e.what() << std::endl;
        return;
    }

#ifdef DEBUG
    std::cout << "==============PARSE==============" << std::endl;
    config::print(parsed);
#else
    std::cout << "parser: the configuration file " + path + " syntax is ok" << std::endl;
#endif
}

int main(int argc, char **argv) {
    if (argc < 2) {
        return 0;
    }
    const char *path = argv[1];

    test_lexer(path);
    //    test_parser(path);k
    return 0;
}
