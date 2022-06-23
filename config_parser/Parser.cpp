#include <iostream>
#include <map>
#include <utility>
#include "Parser.hpp"
#include "Lexer.hpp"
#include "Analyzer.hpp"
#include "test_common.hpp"

void error_exit(std::string msg)
{
    perror(msg.c_str());
    exit(1);
}

Parser::Parser()
{
}

Parser::~Parser()
{
}

std::vector<Directive> Parser::Parse(std::string filename)
{
    // トークンごとに分割する
    lexer_.lex(filename);

    std::vector<std::string> ctx;
    std::vector<Directive> parsed = parse(ctx);

    return parsed;
}

/**
 * @brief lexerによって分割されたtokenを解析する
 * 1. "}"のチェック
 * 2. コメントのチェック
 * 3. 引数のチェック
 * 4. 構文解析
 *  - ディレクティブが正しいか
 *  - 引数の数が正しいか
 * 5. ブロックディレクティブの場合は再帰的にパースする
 */

std::vector<Directive> Parser::parse(std::vector<std::string> ctx)
{
    std::vector<Directive> parsed;

    wsToken *cur = NULL;
    while (1)
    {
        cur = lexer_.read();
        if (cur == NULL)
        {
            return parsed;
        }

        if (cur->value == "}" && !cur->is_quoted)
        {
            break;
        }

        Directive dire(cur->value, cur->line);

        // コメントのチェック
        // TODO: 後ほど内部処理の削除
        if (cur->value[0] == '#' && !cur->is_quoted)
        {
            dire.directive = "#";
            dire.args.push_back(cur->value.substr(1, cur->value.size() - 1));
            parsed.push_back(dire);
            continue;
        }

        cur = lexer_.read();
        // TODO: ここで終わった場合のチェック
        if (cur == NULL)
        {
            return parsed;
        }

        // 引数のパース(特殊文字が来るまで足し続ける)
        while (cur->is_quoted || (cur->value != "{" && cur->value != ";" && cur->value != "}"))
        {
            if (cur->value[0] == '#' && !cur->is_quoted)
            {
            }
            else
            {
                dire.args.push_back(cur->value);
            }

            cur = lexer_.read();
            if (cur == NULL)
            {
                return parsed;
            }
        }

        if (!analyze(dire, cur->value, ctx))
        {
            error_exit("analyze error");
        }

        // "{" で終わってた場合はcontextを調べる
        std::vector<std::string> inner;
        if (cur->value == "{" && !cur->is_quoted)
        {
            inner = enter_block_ctx(dire, ctx); // get context for block
            std::vector<Directive> block;

            block = parse(inner);
            if (block.size() == 0)
            {
                error_exit("block error");
            }

            dire.block = block;

            cur = lexer_.read();
        }
        parsed.push_back(dire);
    }

    return parsed;
}

/*************************************************************/
// debug
void print_parsed_data(std::vector<Directive> d, bool is_block, std::string before)
{
    std::string dir;
    if (is_block)
    {
        dir = "block";
    }
    else
    {
        dir = "Directive";
    }
    for (size_t i = 0; i < d.size(); i++)
    {
        std::cout << before << dir << "[" << i << "].directive : " << d[i].directive << std::endl;

        if (d[i].args.size() == 0)
        {
            std::cout << before << dir << "[" << i << "].args : {}" << std::endl;
        }
        else
        {
            for (size_t j = 0; j < d[i].args.size(); j++)
            {
                std::cout << before << dir << "[" << i << "].args[" << j << "]: " << d[i].args[j] << std::endl;
            }
        }
        if (d[i].block.size() == 0)
        {
            std::cout << before << dir << "[" << i << "].block : {}" << std::endl;
        }
        else
        {
            std::string b = before + dir + "[" + std::to_string(i) + "]" + ".";
            print_parsed_data(d[i].block, true, b);
        }
    }
}
/*************************************************************/
