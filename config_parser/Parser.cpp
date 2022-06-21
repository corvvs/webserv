#include <iostream>
#include <map>
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

// bool analyze(std::string fname, Directive stmt, std::string term, std::vector<std::string> ctx)
//{
// }

void Parser::Parse(std::string filename)
{
    Lexer lexer;
    std::vector<ngxToken> tokens = lexer.lex(filename);

    std::vector<std::string> ctx;

    // filename, status, ctx
    //    std::vector<Directive> parsed = parse(tokens, false);
    std::vector<Directive> parsed = parse(tokens, ctx, false, 0);

    /// debug
    std::vector<Directive>::iterator it = parsed.begin();
    for (; it != parsed.end(); it++)
    {
        std::cout << "directive: " << it->directive << std::endl;
        std::cout << "line     : " << it->line << std::endl;

        std::cout << "args     : ";
        for (std::vector<std::string>::iterator s_it = it->args.begin(); s_it != it->args.end(); s_it++)
        {
            std::cout << *s_it << " ";
        }
        std::cout << std::endl;
        std::cout << "comment   : " << it->comment << std::endl;
    }
}

// TODO:
// 再帰的処理になるので、どこまで処理したのかを覚えておく必要がある
// コルーチン的な処理がしたい

// FIXME: ひとまずiteratorを引数で受けることでそこから処理を行うようにする
// std::vector<Directive> Parser::parse(std::vector<ngxToken> tokens, bool consume)
std::vector<Directive> Parser::parse(std::vector<ngxToken> tokens, std::vector<std::string> ctx, bool consume, int advance)
{
    std::vector<Directive> parsed;

    // 再帰的にパースする
    // advanceの数に進める
    std::vector<ngxToken>::iterator it = tokens.begin() + advance;
    for (; it != tokens.end(); it++, advance++)
    {
        debug(it->value);
        // エラーがあった場合
        if (!it->error.empty())
        {
            error_exit(it->error);
            return parsed;
        }

        // blockの解析中
        // 閉じていたら閉じていたら終了
        if (it->value == "}" && !it->isQuoted)
        {
            break;
        }

        if (consume)
        {
            if (it->value == "{" && !it->isQuoted)
            {
                // TODO: 途中から処理したい
                //                parse(tokens, true);
                parse(tokens, std::vector<std::string>(), true, advance);
                //                error_exit("consume");
            }
            continue;
        }

        Directive stmt;
        stmt.directive = it->value;
        stmt.line = it->line;
        stmt.args = std::vector<std::string>();

        std::vector<std::string> comments_in_args;

        // コメントのチェック
        if (it->value[0] == '#' && !it->isQuoted)
        {
            comments_in_args.push_back(it->value);
            stmt.directive = "#";
            stmt.comment = it->value.substr(1, it->value.size() - 1);
            parsed.push_back(stmt);
            continue;
        }

        // ここで一個すすめる？
        // it++;

        // parse arguments by reading tokens
        while (it->isQuoted || (it->value != "{" && it->value != ";" && it->value != "}"))
        {
            if (it->value[0] == '#' && !it->isQuoted)
            {
                comments_in_args.push_back(it->value.substr(0, it->value.size() - 1));
            }
            else
            {
                stmt.args.push_back(it->value);
            }
            it++;
            // 終端のチェック必要
            if (it == tokens.end())
            {
                exit(42);
                // TODO:どうするか
            }
        }

        //        if (analyze())
        //        {
        //        }

        // "{" で終わってた場合はcontextを調べる
        if (it->value == "{" && !it->isQuoted)
        {
            // TODO:
            std::vector<std::string> inner = enterBlockCtx(stmt, ctx); // get context for block
            std::vector<Directive> block;
            block = parse(tokens, inner, false, advance + 1);
            if (block.size() == 0)
            {
                error_exit("block error");
            }
            stmt.block = block;
        }

        parsed.push_back(stmt);

        // コメントを追加する

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

    return parsed;
}
