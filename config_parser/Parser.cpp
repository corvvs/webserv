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

/*************************************************************/
// debug
void print_parsed_data(std::vector<Directive> d, bool is_block, std::string before = "")
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

void Parser::Parse(std::string filename)
{
    // トークンごとに分割する
    lexer_.lex(filename);

    std::vector<std::string> ctx;
    std::vector<Directive> parsed = parse(ctx);

    // debug
    print_parsed_data(parsed, false);
}

std::vector<Directive> Parser::parse(std::vector<std::string> ctx)
{
    std::vector<Directive> parsed;

    wsToken *cur;
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

        Directive stmt(cur->value, cur->line);

        // コメントのチェック
        // TODO: 後ほど内部処理の削除
        if (cur->value[0] == '#' && !cur->is_quoted)
        {
            stmt.directive = "#";
            stmt.args.push_back(cur->value.substr(1, cur->value.size() - 1));
            parsed.push_back(stmt);
            continue;
        }

        cur = lexer_.read();

        // parse arguments by reading tokens
        while (cur->is_quoted || (cur->value != "{" && cur->value != ";" && cur->value != "}"))
        {
            if (cur->value[0] == '#' && !cur->is_quoted)
            {
            }
            else
            {
                stmt.args.push_back(cur->value);
            }

            cur = lexer_.read();
            if (cur == NULL)
            {
                return parsed;
            }
        }

        if (!analyze(stmt, cur->value, ctx))
        {
            error_exit("analyze error");
        }

        // "{" で終わってた場合はcontextを調べる
        std::vector<std::string> inner;
        if (cur->value == "{" && !cur->is_quoted)
        {
            inner = enterBlockCtx(stmt, ctx); // get context for block
            std::vector<Directive> block;

            block = parse(inner);
            if (block.size() == 0)
            {
                error_exit("block error");
            }

            stmt.block = block;

            cur = lexer_.read();
            if (cur == NULL)
            {
                parsed.push_back(stmt);
                return parsed;
            }
        }
        parsed.push_back(stmt);
    }

    return parsed;
}
