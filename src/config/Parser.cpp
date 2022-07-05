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

std::vector<Directive> Parser::Parse(const std::string &file_data) {
    // トークンごとに分割する
    lexer_.tokenize(file_data);

    ErrorType err;
    if ((err = brace_balanced()) != "") {
        throw SyntaxError(err);
    }
    std::vector<Directive> pre_parsed = pre_parse();

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
void Parser::add_http(const enum ContextType &ctx, const std::vector<std::string> &args) {
    (void)ctx;
    (void)args;
}
void Parser::add_server(const enum ContextType &ctx, const std::vector<std::string> &args) {
    (void)ctx;
    (void)args;
    ContextServer s(ctx_main_);
    ctx_servers_.push_back(s);
}
void Parser::add_location(const enum ContextType &ctx, const std::vector<std::string> &args) {
    (void)ctx;
    (void)args;
    ContextLocation l(ctx_servers_.back());
    ctx_servers_.back().locations.push_back(l);
}
void Parser::add_limit_except(const enum ContextType &ctx, const std::vector<std::string> &args) {
    (void)ctx;
    (void)args;
}

/// Normal
void Parser::add_allow(const enum ContextType &ctx, const std::vector<std::string> &args) {
    (void)ctx;
    (void)args;
}
void Parser::add_deny(const enum ContextType &ctx, const std::vector<std::string> &args) {
    (void)ctx;
    (void)args;
}

void Parser::add_autoindex(const enum ContextType &ctx, const std::vector<std::string> &args) {
    bool flag = (args[0] == "on");

    if (ctx == SERVER) {
        ctx_servers_.back().autoindex = flag;
    }
    if (ctx == LOCATION) {
        ctx_servers_.back().locations.back().autoindex = flag;
    }
}
void Parser::add_error_page(const enum ContextType &ctx, const std::vector<std::string> &args) {
    (void)ctx;
    (void)args;
}
void Parser::add_index(const enum ContextType &ctx, const std::vector<std::string> &args) {
    if (ctx == SERVER) {
        ctx_servers_.back().indexes = args;
    }
    if (ctx == LOCATION) {
        ctx_servers_.back().locations.back().indexes = args;
    }
}
// TODO: デフォルトサーバーの処理
void Parser::add_listen(const enum ContextType &ctx, const std::vector<std::string> &args) {
    (void)ctx;
    (void)args;
    //    ctx_servers_.
    // 分割の処理を行う
    // ipとportのペアとして処理する
}

void Parser::add_return(const enum ContextType &ctx, const std::vector<std::string> &args) {
    (void)ctx;
    (void)args;
}
void Parser::add_root(const enum ContextType &ctx, const std::vector<std::string> &args) {
    const std::string path = args[0];

    if (ctx == SERVER) {
        ctx_servers_.back().root = path;
    }
    if (ctx == LOCATION) {
        ctx_servers_.back().locations.back().root = path;
    }
}
void Parser::add_server_name(const enum ContextType &ctx, const std::vector<std::string> &args) {
    (void)ctx;
    (void)args;
    ctx_servers_.back().server_names = args;
}

void Parser::add_client_max_body_size(const enum ContextType &ctx, const std::vector<std::string> &args) {
    (void)ctx;
    (void)args;
}

/// Original
void Parser::add_upload_store(const enum ContextType &ctx, const std::vector<std::string> &args) {
    (void)ctx;
    (void)args;
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

// ContextLocation do_location(ContextLocation &ctx_loc, std::vector<Directive> location_directive) {
//     std::queue<Directive> que;

//     for (std::vector<Directive>::iterator it = server_directive.begin(); it != server_directive.end(); ++it) {
//         if (is_block(*it)) {
//             que.push(*it);
//         } else {
//             // シンプルディレクティブの処理を行う
//             // ctx_serverに情報を追加する(IContextで受け取ってaddする)
//             do_directive(ctx_loc, *it);
//         }
//     }
//     while (!que.empty()) {
//         // locationから引き継ぐ可能性もあるのでコンストラクタを2つ作成する
//         ContextLocation loc(ctx_loc);
//         do_location(loc, que.front());
//         ctx_loc.locations.push_back(loc);
//         que.pop_front();
//     }
//     return ctx_loc;
// }

// // 継承済みのサーバーにディレクティブの情報を追加する
// ContextServer do_server(ContextServer &ctx_server, std::vector<Directive> server_directive) {
//     std::queue<Directive> que;

//     for (std::vector<Directive>::iterator it = server_directive.begin(); it != server_directive.end(); ++it) {
//         if (is_block(*it)) {
//             que.push(*it);
//         } else {
//             // シンプルディレクティブの処理を行う
//             // ctx_serverに情報を追加する(IContextで受け取ってaddする)
//             do_directive(ctx_server, *it);
//         }
//     }
//     while (!que.empty()) {
//         ContextLocation loc(ctx_server);
//         do_location(loc, que.front());
//         ctx_server.locations.push_back(loc);
//         que.pop_front();
//     }
//     return ctx_server;
// }

// // blockディレクティブの場合はstackに積んでおいてあとから処理する
std::vector<ContextServer> Parser::parse(std::vector<Directive> vdir) {
    std::queue<Directive> que;
    //    ContextType ctx = GLOBAL;

    (void)vdir;

    // for (std::vector<Directive>::iterator it = vdir.begin(); it != vdir.end(); ++it) {
    //     if (it->name == "http") {
    //         // serverの処理
    //         for (std::vector<Directive>::iterator it2 = it->block.begin(); it2 != it->block.end(); ++it2) {
    //             ContextServer s(*it);
    //             s = do_server(s, *it2) servers.push_back(s);
    //         }
    //     }
    // }
    // ブロックの場合積んでおいてあとから処理する
    // if (is_block(*it)) {
    //     que.push(*it);
    // } else {
    //     // do_simple_block();
    // }

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

/**
    int client_max_body_size;
    bool autoindex;
    std::string allow;
    std::string deny;
    std::string root;
    std::vector<std::string> indexes;
    std::map<int, std::string> error_pages;

    // 新たに追加する
    int port;
    std::string host;
    std::vector<std::string> server_names;
    std::string upload_store;
    std::pair<int, std::string> redirect;

    // 内部にlocationを持つ可能性がある
    std::vector<class ContextLocation> locations;
 */

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
