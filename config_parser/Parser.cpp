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

void Parser::Parse(std::string filename)
{
    Lexer lexer;
    std::vector<wsToken> tokens = lexer.lex(filename);

    std::vector<std::string> ctx;

    // filename, status, ctx
    std::vector<Directive> parsed = parse(tokens, ctx);

    print_parsed_data(parsed, false, "");
}

// TODO: 再帰的に処理をする
static int advance = 0;
std::vector<Directive> Parser::parse(std::vector<wsToken> &tokens, std::vector<std::string> ctx)
{
    std::vector<Directive> parsed;

    // 再帰的にパースする
    // advanceの数に進める
    std::vector<wsToken>::iterator it = tokens.begin() + advance;
    for (; it != tokens.end(); it++, advance++)
    {
        debug(it->value);
        // blockの解析中
        // 閉じていたら閉じていたら終了
        if (it->value == "}" && !it->is_quoted)
        {
            break;
        }

        Directive stmt;
        stmt.directive = it->value;
        stmt.line = it->line;
        stmt.args = std::vector<std::string>();

        std::vector<std::string> comments_in_args;

        // コメントのチェック
        if (it->value[0] == '#' && !it->is_quoted)
        {
            comments_in_args.push_back(it->value);
            stmt.directive = "#";
            stmt.comment = it->value.substr(1, it->value.size() - 1);
            parsed.push_back(stmt);
            continue;
        }

        // ここで一個すすめる？
        it++;
        advance++;

        // parse arguments by reading tokens
        while (it->is_quoted || (it->value != "{" && it->value != ";" && it->value != "}"))
        {
            if (it->value[0] == '#' && !it->is_quoted)
            {
                comments_in_args.push_back(it->value.substr(0, it->value.size() - 1));
            }
            else
            {
                stmt.args.push_back(it->value);
            }
            it++;
            advance++;
            // 終端のチェック必要
            if (it == tokens.end())
            {
                // TODO: 後ほど変更する
                return parsed;
            }
        }

        // DOUT() << "analyze" << std::endl;
        // debug(stmt.directive);
        // debug(stmt.line);
        // debug(stmt.args);
        // debug(stmt.comment);
        // debug(it->value);
        // debug(ctx);
        if (!analyze(stmt, it->value, ctx))
        {
            error_exit("analyze error");
        }

        debug(it->value);
        debug(it->is_quoted);
        // "{" で終わってた場合はcontextを調べる
        std::vector<std::string> inner;
        if (it->value == "{" && !it->is_quoted)
        {
            inner = enterBlockCtx(stmt, ctx); // get context for block
            std::vector<Directive> block;
            advance += 1;
            block = parse(tokens, inner);
            if (block.size() == 0)
            {
                error_exit("block error");
            }
            stmt.block = block;
            it = tokens.begin() + advance;
            if (it == tokens.end())
            {
                parsed.push_back(stmt);
                return parsed;
            }
        }

        parsed.push_back(stmt);

        for (std::vector<std::string>::iterator com_it = comments_in_args.begin(); com_it != comments_in_args.end(); com_it++)
        {
            std::string comment = *com_it;
            Directive d;
            d.directive = "#";
            d.line = stmt.line;
            d.args = std::vector<std::string>();
            d.comment = comment;

            parsed.push_back(d);
        }
    }

    if (it == tokens.end())
    {
        return parsed;
    }
    advance += 1;
    return parsed;
}
