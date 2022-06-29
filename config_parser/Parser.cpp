#include "Parser.hpp"
#include "Analyzer.hpp"
#include "Lexer.hpp"
#include "test_common.hpp"
#include <iostream>
#include <map>
#include <utility>

void error_exit(std::string msg) {
    perror(msg.c_str());
    exit(1);
}

Parser::Parser() {}

Parser::~Parser() {}

std::vector<Directive> Parser::Parse(std::string filename) {
    // トークンごとに分割する
    lexer_.lex(filename);
    if (!is_brace_balanced()) {
        // 後でタイプを受け取って例外を投げるように修正する
        std::cout << "Error " << std::endl;
    }

    // TODO: はじめはmainコンテキストを渡す?
    //    std::vector<std::string> ctx;
    std::vector<Directive> parsed = parse();

    return parsed;
}

bool Parser::is_brace_balanced(void) {
    int depth = 0;
    int line  = 0;

    Lexer::wsToken *tok;
    while ((tok = lexer_.read()) != NULL) {
        line = tok->line;
        if (tok->value == "}" && !tok->is_quoted) {
            depth -= 1;
        } else if (tok->value == "{" && !tok->is_quoted) {
            depth += 1;
        }
        if (depth < 0) {
            throw std::runtime_error("webserv: [emerg] unexpected \"}\" :" + std::to_string(line));
        }
    }
    if (depth > 0) {
        throw std::runtime_error("webserv: [emerg] unexpected end of file, expecting \"}\" :" + std::to_string(line));
    }

    lexer_.reset_read_idx();
    return true;
}

/**
 * @brief lexerによって分割されたtokenを解析する
 * 1. "}"のチェック
 * 2. 引数のチェック
 * 3. 構文解析
 *  - ディレクティブが正しいか
 *  - 引数の数が正しいか
 * 4. ブロックディレクティブの場合は再帰的にパースする
 */
std::vector<Directive> Parser::parse(std::vector<std::string> ctx) {
    std::vector<Directive> parsed;

    Lexer::wsToken *cur;
    while (1) {
        cur = lexer_.read();
        if (cur == NULL) {
            return parsed;
        }

        if (cur->value == "}" && !cur->is_quoted) {
            break;
        }

        Directive dire(cur->value, cur->line);

        cur = lexer_.read();
        if (cur == NULL) {
            return parsed;
        }

        // 引数のパース(特殊文字が来るまで足し続ける)
        // TODO: is_specialに置き換える
        while (cur->is_quoted || (cur->value != "{" && cur->value != ";" && cur->value != "}")) {
            dire.args.push_back(cur->value);
            cur = lexer_.read();
            if (cur == NULL) {
                return parsed;
            }
        }

        try {
            analyze(dire, cur->value, ctx);
        } catch (std::exception &e) {
            throw std::runtime_error(e.what());
        }

        // "{" で終わってた場合はcontextを調べる
        std::vector<std::string> inner;
        if (cur->value == "{" && !cur->is_quoted) {
            inner = enter_block_ctx(dire, ctx); // get context for block
            std::vector<Directive> block;

            block = parse(inner);
            if (block.size() == 0) {
                error_exit("block error");
            }

            dire.block = block;
        }
        parsed.push_back(dire);
    }

    return parsed;
}

/*************************************************************/
// debug
void print(std::vector<Directive> d, bool is_block, std::string before) {
    std::string dir;
    if (is_block) {
        dir = "block";
    } else {
        dir = "Dire";
    }
    for (size_t i = 0; i < d.size(); i++) {
        std::cout << before << dir << "[" << i << "].dire   : " << d[i].directive << std::endl;

        if (d[i].args.size() == 0) {
            std::cout << before << dir << "[" << i << "].args   : {}" << std::endl;
        } else {
            for (size_t j = 0; j < d[i].args.size(); j++) {
                std::cout << before << dir << "[" << i << "].args[" << j << "]: " << d[i].args[j] << std::endl;
            }
        }
        if (d[i].block.size() == 0) {
            std::cout << before << dir << "[" << i << "].block  : {}" << std::endl;
        } else {
            std::string b = before + dir + "[" + std::to_string(i) + "]" + ".";
            print(d[i].block, true, b);
        }
    }
}
/*************************************************************/
