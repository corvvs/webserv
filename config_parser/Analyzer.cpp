#include "Lexer.hpp"
#include "Parser.hpp"
#include "Analyzer.hpp"
#include <iostream>
#include "test_common.hpp"

void error_exit(std::string msg);

bool validFlag(std::string s)
{
    // TODO: tolowerかける
    std::string l = s;
    return l == "on" || l == "off";
}

std::vector<std::string> enterBlockCtx(Directive stmt, std::vector<std::string> ctx)
{
    if (ctx.size() > 0 && ctx[0] == "http" && stmt.directive == "location")
    {
        ctx.clear();
        ctx.push_back("http");
        ctx.push_back("location");
        return ctx;
    }

    // 他のブロックコンテキストは、locationのようにネストすることができないので、追加するだけ
    ctx.push_back(stmt.directive);
    return ctx;
}

bool analyze(Directive stmt, std::string term, std::vector<std::string> ctx)
{
    // debug(stmt.directive);
    // debug(ctx);
    std::vector<int> masks = get_directives(stmt.directive);
    int currCtx = get_contexts(ctx);

    // directiveが該当しない場合はエラーを投げる
    if (masks.size() == 0)
    {
        const std::string msg = "unknown directive \"" + stmt.directive + "\"";
        error_exit(msg);
    }

    // debug(masks.size());
    // ディレクティブがこのコンテキストで使用できない場合はエラーを投げる
    std::vector<int> ctxMasks;
    for (std::vector<int>::iterator it = masks.begin(); it != masks.end(); it++)
    {
        int mask = *it;
        // debug(mask);
        if ((mask & currCtx) != 0)
        {
            ctxMasks.push_back(mask);
        }
    }
    if (ctxMasks.size() == 0)
    {
        const std::string msg = "\"" + stmt.directive + "\" is not allowed here";
        error_exit(msg);
    }

    for (std::vector<int>::iterator it = ctxMasks.begin(); it != ctxMasks.end(); it++)
    {
        int mask = *it;

        // ブロックディレクティブで "{" が続いていなかったらエラーを投げる
        if ((mask & ngxConfBlock) != 0 && term != "{")
        {
            const std::string msg = "directive \"" + stmt.directive + "\" has no opening \"{\"";
            error_exit(msg);
        }

        // シンプルディレクティブで ";"が続いていなかったらエラーを投げる
        if ((mask & ngxConfBlock) == 0 && term != ";")
        {
            const std::string msg = "directive \"" + stmt.directive + "\" is not terminated by \";\"";
            error_exit(msg);
        }

        // 引数の数が合わない場合はエラーを投げる
        if (((mask >> stmt.args.size() & 1) != 0 && stmt.args.size() <= 7) || // NOARGS to TAKE7
            ((mask & ngxConfFlag) != 0 && stmt.args.size() == 1 && validFlag(stmt.args[0])) ||
            ((mask & ngxConfAny) != 0 && stmt.args.size() >= 0) ||
            ((mask & ngxConf1More) != 0 && stmt.args.size() >= 1) ||
            ((mask & ngxConf2More) != 0 && stmt.args.size() >= 2))
        {
            return true;
        }
        else if ((mask & ngxConfFlag) != 0 && stmt.args.size() == 1 && !validFlag(stmt.args[0]))
        {
            const std::string msg = "invalid value \"" + stmt.args[0] + "\" in \"" + stmt.directive + "\"" + " directive, it must be \"on\" or \"off\"";
            error_exit(msg);
        }
        else
        {
            const std::string msg = "invalid number of arguments in \"" + stmt.directive + "\" directive";
            error_exit(msg);
        }
    }
    return false;
}

int get_contexts(std::vector<std::string> blockCtx)
{
    // map for getting bitmasks from certain context tuples
    std::map<std::vector<std::string>, int> contexts;
    contexts[std::vector<std::string>()] = ngxMainConf;

    std::vector<std::string> v;
    v.push_back("events");
    contexts[v] = ngxEventConf;
    // contexts[{"mail"}]                              = ngxMailMainConf;
    // contexts[{"mail", "server"}]                    = ngxMailSrvConf;
    // contexts[{"stream"}]                            = ngxStreamMainConf;
    // contexts[{"stream", "server"}]                  = ngxStreamSrvConf;
    // contexts[{"stream", "upstream"}]                = ngxStreamUpsConf;

    v.clear();
    v.push_back("http");
    contexts[v] = ngxHttpMainConf;
    v.push_back("server");
    contexts[v] = ngxHttpSrvConf;

    v.clear();
    v.push_back("http");
    v.push_back("location");
    contexts[v] = ngxHttpLocConf;
    // contexts[{"http", "upstream"}]                  = ngxHttpUpsConf;
    // contexts[{"http", "server", "if"}]              = ngxHttpSifConf;
    // contexts[{"http", "location", "if"}]            = ngxHttpLifConf;
    v.push_back("limit_except");
    contexts[v] = ngxHttpLmtConf;

    return contexts[blockCtx];
}

std::vector<int> get_directives(std::string directive)
{
    std::map<std::string, std::vector<int>> directives;
    // http

    std::vector<int> v;
    v.push_back(ngxMainConf | ngxConfBlock | ngxConfNoArgs);
    directives["http"] = v;

    // server
    v.clear();
    v.push_back(ngxHttpMainConf | ngxConfBlock | ngxConfNoArgs);
    v.push_back(ngxHttpUpsConf | ngxConf1More);
    v.push_back(ngxMailMainConf | ngxConfBlock | ngxConfNoArgs);
    v.push_back(ngxStreamMainConf | ngxConfBlock | ngxConfNoArgs);
    v.push_back(ngxStreamUpsConf | ngxConf1More);
    directives["server"] = v;

    // listen
    v.clear();
    v.push_back(ngxHttpSrvConf | ngxConf1More);
    v.push_back(ngxMailSrvConf | ngxConf1More);
    v.push_back(ngxStreamSrvConf | ngxConf1More);
    directives["listen"] = v;

    // server_name
    v.clear();
    v.push_back(ngxHttpSrvConf | ngxConf1More);
    v.push_back(ngxMailMainConf | ngxMailSrvConf | ngxConfTake1);
    directives["server_name"] = v;

    // location
    v.clear();
    v.push_back(ngxHttpSrvConf | ngxHttpLocConf | ngxConfBlock | ngxConfTake12);
    directives["location"] = v;

    // root
    v.clear();
    v.push_back(ngxHttpMainConf | ngxHttpSrvConf | ngxHttpLocConf | ngxHttpLifConf | ngxConfTake1);
    directives["root"] = v;

    // index
    v.clear();
    v.push_back(ngxHttpMainConf | ngxHttpSrvConf | ngxHttpLocConf | ngxConf1More);
    directives["index"] = v;

    return directives[directive];
}
