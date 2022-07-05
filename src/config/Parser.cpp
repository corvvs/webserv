#include "Parser.hpp"
#include "Config.hpp"
#include "Lexer.hpp"
#include "Validator.hpp"
#include "test_common.hpp"
#include <iomanip>
#include <iostream>
#include <map>
#include <queue>
#include <sstream>
#include <utility>

namespace config {

Parser::Parser(void) {}
Parser::~Parser(void) {}

// Parser::DirectiveFunctionsMap directives = setting_directive_functions();

// ディレクティブとコンテキストを追加する関数を設定する
// Parser::DirectiveFunctionsMap Parser::setting_directive_functions(void) {
Parser::DirectiveFunctionsMap Parser::setting_directive_functions(void) {
    // std::map<std::string, void (Parser::*add_directive_functions)(const std::vector<std::string> &args)> directives;
    DirectiveFunctionsMap directives;

    // map<string, void(*)(const std::vector<string>)>

    /// Block

    directives["http"]         = &Parser::add_http;
    directives["server"]       = &Parser::add_server;
    directives["location"]     = &Parser::add_location;
    directives["limit_except"] = &Parser::add_limit_except;

    /// Normal
    directives["allow"] = &Parser::add_allow;
    directives["deny"]  = &Parser::add_deny;

    directives["autoindex"]   = &Parser::add_autoindex;
    directives["error_page"]  = &Parser::add_error_page;
    directives["index"]       = &Parser::add_index;
    directives["listen"]      = &Parser::add_listen;
    directives["return"]      = &Parser::add_return;
    directives["root"]        = &Parser::add_root;
    directives["server_name"] = &Parser::add_server_name;

    directives["client_max_body_size"] = &Parser::add_client_max_body_size;

    /// Original
    directives["upload_store"] = &Parser::add_upload_store;

    return directives;
}

std::vector<Directive> Parser::Parse(const std::string &file_data) {
    // トークンごとに分割する
    lexer_.tokenize(file_data);

    ErrorType err;
    if ((err = brace_balanced()) != "") {
        throw SyntaxError(err);
    }
    std::vector<Directive> pre_parsed = pre_parse();

    ctx_                                      = GLOBAL;
    std::vector<ContextServer> server_configs = parse(pre_parsed);

    std::vector<ContextServer>::iterator it = server_configs.begin();
    for (; it != server_configs.end(); ++it) {
        print(*it);
    }

    return pre_parsed;
}

std::vector<std::string> Parser::enter_block_ctx(Directive dire, std::vector<std::string> ctx) {
    if (ctx.size() > 0 && ctx[0] == "http" && dire.name == "location") {
        ctx.clear();
        ctx.push_back("http");
        ctx.push_back("location");
        return ctx;
    }

    // location以外はネストすることができないので追加する
    ctx.push_back(dire.name);
    return ctx;
}

ErrorType Parser::brace_balanced(void) {
    int depth = 0;
    int line  = 0;

    wsToken *tok;
    while ((tok = lexer_.read()) != NULL) {
        line = tok->line;
        if (tok->value == "}" && !tok->is_quoted) {
            depth -= 1;
        } else if (tok->value == "{" && !tok->is_quoted) {
            depth += 1;
        }
        if (depth < 0) {
            return validation_error("unexpected \"}\"", line);
        }
    }
    if (depth > 0) {
        return validation_error("unexpected end of file, expecting \"}\"", line);
    }
    lexer_.reset_read_idx();
    return "";
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
std::vector<Directive> Parser::pre_parse(std::vector<std::string> ctx) {
    std::vector<Directive> parsed;

    wsToken *cur;
    while (1) {
        cur = lexer_.read();
        if (cur == NULL) {
            return parsed;
        }

        if (cur->value == "}" && !cur->is_quoted) {
            break;
        }

        Directive dire = {cur->value, cur->line, std::vector<std::string>(), std::vector<Directive>()};

        cur = lexer_.read();
        if (cur == NULL) {
            return parsed;
        }

        // 引数のパース(特殊文字が来るまで足し続ける)
        while (cur->is_quoted || (cur->value != "{" && cur->value != ";" && cur->value != "}")) {
            dire.args.push_back(cur->value);
            cur = lexer_.read();
            if (cur == NULL) {
                return parsed;
            }
        }
        std::string err;
        if ((err = validate(dire, cur->value, ctx)) != "") {
            throw SyntaxError(err);
        }

        // "{" で終わってた場合はcontextを調べる
        std::vector<std::string> inner;
        if (cur->value == "{" && !cur->is_quoted) {
            inner      = enter_block_ctx(dire, ctx); // get context for block
            dire.block = pre_parse(inner);
        }
        parsed.push_back(dire);
    }

    return parsed;
}

// static std::map<std::string, void *f()()> add_directive_funcs;
void Parser::add_http(const std::vector<std::string> &args) {
    (void)args;
}

void Parser::add_server(const std::vector<std::string> &args) {
    (void)args;
    ContextServer s(ctx_main_);
    ctx_servers_.push_back(s);
}

void Parser::add_location(const std::vector<std::string> &args) {
    (void)args;
    ContextLocation l(ctx_servers_.back());
    ctx_servers_.back().locations.push_back(l);
}

void Parser::add_limit_except(const std::vector<std::string> &args) {
    (void)args;
    ctx_servers_.back().locations.back().limit_expect = new ContextLimitExpect;
}

/// Normal
void Parser::add_allow(const std::vector<std::string> &args) {
    (void)args;
}

void Parser::add_deny(const std::vector<std::string> &args) {
    (void)args;
}

void Parser::add_autoindex(const std::vector<std::string> &args) {
    bool flag = (args.front() == "on");

    if (ctx_ == SERVER) {
        ctx_servers_.back().autoindex = flag;
    }
    if (ctx_ == LOCATION) {
        ctx_servers_.back().locations.back().autoindex = flag;
    }
}

void Parser::add_error_page(const std::vector<std::string> &args) {
    (void)args;
}

void Parser::add_index(const std::vector<std::string> &args) {
    const std::vector<std::string> &indexes = args;

    if (ctx_ == SERVER) {
        ctx_servers_.back().indexes = indexes;
    }
    if (ctx_ == LOCATION) {
        ctx_servers_.back().locations.back().indexes = indexes;
    }
}

void Parser::add_listen(const std::vector<std::string> &args) {
    const std::vector<std::string> &splitted = utility::split_str(args.front(), ":");
    if (splitted.size() == 1) {
        if (is_host(splitted.front())) {
            std::string host = splitted.front();
            if (host == "localhost") {
                ctx_servers_.back().host = "127.0.0.1";
            } else if (host == "*") {
                ctx_servers_.back().host = "0.0.0.0";
            } else {
                ctx_servers_.back().host = host;
            }
        } else {
            ctx_servers_.back().port = std::atoi(splitted.front().c_str());
        }
    }

    if (splitted.size() == 2) {
        const std::string &host = splitted.front();
        if (host == "localhost") {
            ctx_servers_.back().host = "127.0.0.1";
        } else if (host == "*") {
            ctx_servers_.back().host = "0.0.0.0";
        } else {
            ctx_servers_.back().host = host;
        }
        ctx_servers_.back().port = std::atoi(splitted.back().c_str());
    }

    if (args.size() == 2) {
        ctx_servers_.back().default_server = (args.back() == "default_server");
    }
}

void Parser::add_return(const std::vector<std::string> &args) {
    const int &status_code  = std::atoi(args.front().c_str());
    const std::string &path = args.back();

    if (ctx_ == SERVER) {
        ctx_servers_.back().redirect = std::make_pair(status_code, path);
    }
    if (ctx_ == LOCATION) {
        ctx_servers_.back().locations.back().redirect = std::make_pair(status_code, path);
    }
}
void Parser::add_root(const std::vector<std::string> &args) {
    const std::string &path = args.front();

    if (ctx_ == SERVER) {
        ctx_servers_.back().root = path;
    }
    if (ctx_ == LOCATION) {
        ctx_servers_.back().locations.back().root = path;
    }
}
void Parser::add_server_name(const std::vector<std::string> &args) {
    const std::vector<std::string> &server_names = args;
    ctx_servers_.back().server_names             = server_names;
}

void Parser::add_client_max_body_size(const std::vector<std::string> &args) {
    const int &client_max_body_size = std::atoi(args.front().c_str());

    switch (ctx_) {
        case MAIN:
            ctx_main_.client_max_body_size = client_max_body_size;
            break;
        case SERVER:
            ctx_servers_.back().client_max_body_size = client_max_body_size;
            break;
        case LOCATION:
            ctx_servers_.back().locations.back().client_max_body_size = client_max_body_size;
            break;
        default:;
    }
}

/// Original
void Parser::add_upload_store(const std::vector<std::string> &args) {
    const std::string &path          = args.front();
    ctx_servers_.back().upload_store = path;
}

// static std::map<std::string, int> setting_directives(void) {
//     std::map<std::string, int> directives;

//     /// Block
//     directives["http"]         = (MAIN | BLOCK | NOARGS);
//     directives["server"]       = (HTTP_MAIN | BLOCK | NOARGS);
//     directives["location"]     = (HTTP_SRV | HTTP_LOC | BLOCK | TAKE12);
//     directives["limit_except"] = (HTTP_LOC | BLOCK | MORE1);

//     /// Normal
//     directives["allow"] = (HTTP_MAIN | HTTP_SRV | HTTP_LOC | HTTP_LMT | TAKE1);
//     directives["deny"]  = (HTTP_MAIN | HTTP_SRV | HTTP_LOC | HTTP_LMT | TAKE1);

//     directives["autoindex"]   = (HTTP_MAIN | HTTP_SRV | HTTP_LOC | FLAG);
//     directives["error_page"]  = (HTTP_MAIN | HTTP_SRV | HTTP_LOC | MORE2);
//     directives["index"]       = (HTTP_MAIN | HTTP_SRV | HTTP_LOC | MORE1);
//     directives["listen"]      = (HTTP_SRV | MORE1);
//     directives["return"]      = (HTTP_SRV | HTTP_LOC | TAKE12);
//     directives["root"]        = (HTTP_MAIN | HTTP_SRV | HTTP_LOC | TAKE1);
//     directives["server_name"] = (HTTP_SRV | MORE1);

//     directives["client_max_body_size"] = (HTTP_MAIN | HTTP_SRV | HTTP_LOC | TAKE1);

//     /// Original
//     directives["upload_store"] = (HTTP_SRV | HTTP_LOC | TAKE1);

//     return directives;
// }

// ブロックを内包するかどうかで判断しても良さそう
bool is_block(Directive dir) {
    return dir.block.size() != 0;
}

// blockディレクティブの場合はstackに積んでおいてあとから処理する
std::vector<ContextServer> Parser::parse(std::vector<Directive> vdir) {
    std::queue<Directive> que;

    // (void)vdir;
    for (std::vector<Directive>::iterator it = vdir.begin(); it != vdir.end(); ++it) {
        if (!is_block(*it)) {
            que.push(*it);
        } else {
            // ディレクティブの処理
        }
    }
    // ブロックディレクティブの処理
    while (!que.empty()) {
        // do_directive(que.front());
        que.pop();
    }
    return std::vector<ContextServer>();
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
        std::cout << before << dir << "[" << i << "].dire   : " << d[i].name << std::endl;

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

// 後でtemplate関数に切り替える
std::string to_string(std::vector<std::string> v) {
    std::ostringstream oss;
    oss << "{ ";
    for (std::vector<std::string>::iterator it = v.begin(); it != v.end(); ++it) {
        if (it != v.begin()) {
            oss << ", ";
        }
        oss << *it;
    }
    oss << " }";
    return oss.str();
}

std::string to_string(std::pair<int, std::string> p) {
    std::ostringstream oss;
    oss << "< ";
    oss << p.first << ", " << p.second << " >";
    return oss.str();
}

std::string to_string(std::map<int, std::string> mp) {
    std::ostringstream oss;
    oss << "{ ";
    for (std::map<int, std::string>::iterator it = mp.begin(); it != mp.end(); ++it) {
        oss << "< " << it->first << " " << it->second << " >";
    }
    oss << " }";
    return oss.str();
}

template <typename T>
void print_key_value(const std::string &key, const T &value) {
    std::cout << std::setw(22) << std::left << key << ": " << value << std::endl;
}

void Parser::print(const std::vector<ContextLocation> &loc) {
    for (std::vector<ContextLocation>::const_iterator it = loc.begin(); it != loc.end(); ++it) {
        print_key_value("client_max_body_size", it->client_max_body_size);
        print_key_value("autoindex", it->autoindex);
        print_key_value("allow", it->allow);
        print_key_value("deny", it->deny);
        print_key_value("root", it->root);
        print_key_value("indexes", to_string(it->indexes));
        print_key_value("error_pages", to_string(it->error_pages));
        print_key_value("redirect", to_string(it->redirect));
        print_key_value("path", it->path);
    }

    // print_key_value("locations" << ":" << std::vector<class ContextLocation> locations);,
    // print_key_value("limit_except" << ":" << loc.limit_except);,
}

void Parser::print(const ContextServer &serv) {
    print_key_value("client_max_body_size", serv.client_max_body_size);
    print_key_value("autoindex", serv.autoindex);
    print_key_value("allow", serv.allow);
    print_key_value("deny", serv.deny);
    print_key_value("root", serv.root);
    print_key_value("indexes", to_string(serv.indexes));
    print_key_value("error_pages", to_string(serv.error_pages));

    print_key_value("port", serv.port);
    print_key_value("host", serv.host);
    print_key_value("upload_store", serv.upload_store);
    print_key_value("server_names", to_string(serv.server_names));
    print_key_value("redirect", to_string(serv.redirect));

    std::cout << "locations :";
    if (serv.locations.size() == 0) {
        std::cout << "{}";
    } else {
        print(serv.locations);
    }
}
/*************************************************************/
} // namespace config

/*
//
前人未到という会社にいた
ニューン

*/
