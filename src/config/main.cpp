#include "File.hpp"
#include "Lexer.hpp"
#include "Parser.hpp"
#include "Validator.hpp"
#include "test_common.hpp"
#include <iostream>
#include <vector>

std::string read_file(const std::string &path) {
    file::ErrorType err;
    if ((err = file::check(path)) != file::NONE) {
        std::cerr << file::error_message(err) << std::endl;
        exit(EXIT_FAILURE);
    }
    return file::read(path);
}

void test_lexer(const std::string &path) {
    const std::string &file_data = read_file(path);
    config::Lexer lexer;
    try {
        lexer.tokenize(file_data);
    } catch (const std::runtime_error &e) {
        std::cout << e.what() << std::endl;
        return;
    }

#ifdef NDEBUG
    std::cout << "===============LEX===============" << std::endl;
    config::wsToken *token;
    while ((token = lexer.read()) != NULL) {
        std::cout << *token << std::endl;
    }
#else
    std::cout << "lexer : the configuration file " + path + " syntax is ok" << std::endl;
#endif
}

void test_validation(const std::string &path) {
    const std::string &file_data = read_file(path);
    config::Parser parser;

    try {
        parser.Parse(file_data);
    } catch (const config::SyntaxError &e) {
        std::cout << e.what() << std::endl;
        return;
    }

#ifdef NDEBUG
    std::cout << "==============PARSE==============" << std::endl;
#else
    std::cout << "parser: the configuration file " + path + " syntax is ok" << std::endl;
#endif
}

void test_parser(const std::string &path) {
    const std::string &file_data = read_file(path);

    config::Parser parser;
    std::vector<config::Config> configs;
    try {
        configs = parser.Parse(file_data);
    } catch (const config::SyntaxError &e) {
        std::cout << e.what() << std::endl;
        return;
    }
    // TODO: サーバーディレクティブの数 + listenの数だけソケットを作成する
}

int main(int argc, char **argv) {
    if (argc < 2) {
        return 0;
    }
    const char *path = argv[1];

    // test_lexer(path);
    test_validation(path);

    return 0;
}
