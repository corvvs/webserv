#include "Parser.hpp"
#include "Config.hpp"
#include "ConfigUtility.hpp"
#include "Context.hpp"
#include "Lexer.hpp"
#include "Validator.hpp"
#include "test_common.hpp"
#include <iomanip>
#include <iostream>
#include <map>
#include <queue>
#include <utility>

namespace config {

Parser::Parser(void) {}
Parser::~Parser(void) {}

// ディレクティブとコンテキストを追加する関数を設定する
Parser::DirectiveFunctionsMap Parser::setting_directive_functions(void) {
    DirectiveFunctionsMap directives;

    /// Block
    directives["http"]         = &Parser::add_http;
    directives["server"]       = &Parser::add_server;
    directives["location"]     = &Parser::add_location;
    directives["limit_except"] = &Parser::add_limit_except;

    /// Normal
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

// vector<ContextServer>を返す
std::vector<Config> Parser::Parse(const std::string &file_data) {
    // トークンごとに分割する
    lexer_.tokenize(file_data);

    ErrorType err;
    if ((err = brace_balanced()) != "") {
        throw SyntaxError(err);
    }
    std::vector<Directive> pre_parsed = pre_parse();

    ctx_                                      = GLOBAL;
    std::vector<ContextServer> server_configs = parse(pre_parsed);

    std::vector<Config> configs;

    std::vector<ContextServer>::iterator it = server_configs.begin();
    for (; it != server_configs.end(); ++it) {
        //        print_server(*it);

        // TODO: listenの数だけConfigを作成する(圧縮する必要あり)
        std::vector<host_port_pair>::const_iterator hp_it = it->host_ports.begin();
        for (; hp_it != it->host_ports.end(); ++hp_it) {
            // サーバーコンテキストからconfigを作成
            Config conf(*it);

            // configに対応するhostとportのペアを与える
            conf.set_host_port(*hp_it);
            configs.push_back(conf);
        }
    }
    return configs;
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
            dire.line = cur->line;
            cur       = lexer_.read();
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

void Parser::add_http(const std::vector<std::string> &args) {
    (void)args;
    ctx_ = MAIN;
}

void Parser::add_server(const std::vector<std::string> &args) {
    (void)args;
    ctx_ = SERVER;
    ContextServer s(ctx_main_);
    ctx_servers_.push_back(s);
}

void Parser::add_location(const std::vector<std::string> &args) {
    ContextLocation l(ctx_servers_.back());
    l.path = args.front();
    if (ctx_ == SERVER) {
        ctx_servers_.back().locations.push_back(l);
    }
    if (ctx_ == LOCATION) {
        ctx_servers_.back().locations.back().locations.push_back(l);
    }
    ctx_ = LOCATION;
}

void Parser::add_limit_except(const std::vector<std::string> &args) {
    for (size_t i = 0; i < args.size(); ++i) {
        DXOUT(args[i]);
    }
    ctx_ = LIMIT_EXCEPT;

    ContextLimitExcept *lmt = new ContextLimitExcept();
    for (std::vector<std::string>::const_iterator it = args.begin(); it != args.end(); ++it) {
        if (str_tolower(*it) == "get") {
            lmt->allowed_methods.insert(GET);
        }
        if (str_tolower(*it) == "post") {
            lmt->allowed_methods.insert(POST);
        }
        if (str_tolower(*it) == "delete") {
            lmt->allowed_methods.insert(DELETE);
        }
    }
    ctx_servers_.back().locations.back().limit_except = lmt;
}

/// Normal
void Parser::add_autoindex(const std::vector<std::string> &args) {
    const bool &flag = (str_tolower(args.front()) == "on");

    if (ctx_ == SERVER) {
        ctx_servers_.back().autoindex = flag;
    }
    if (ctx_ == LOCATION) {
        ctx_servers_.back().locations.back().autoindex = flag;
    }
}

void Parser::add_error_page(const std::vector<std::string> &args) {
    const int &error_code   = std::atoi(args.front().c_str());
    const std::string &path = args.back();

    switch (ctx_) {
        case MAIN:
            ctx_main_.error_pages[error_code] = path;
            break;
        case SERVER:
            ctx_servers_.back().error_pages[error_code] = path;
            break;
        case LOCATION:
            ctx_servers_.back().locations.back().error_pages[error_code] = path;
            break;
        default:;
    }
}

void Parser::add_index(const std::vector<std::string> &args) {
    if (ctx_ == SERVER) {
        ContextServer &srv = ctx_servers_.back();
        for (std::vector<std::string>::const_iterator it = args.begin(); it != args.end(); ++it) {
            if (std::find(srv.indexes.begin(), srv.indexes.end(), *it) == srv.indexes.end()) {
                srv.indexes.push_back(*it);
            }
        }
    }
    if (ctx_ == LOCATION) {
        ContextLocation &loc = ctx_servers_.back().locations.back();
        for (std::vector<std::string>::const_iterator it = args.begin(); it != args.end(); ++it) {
            if (std::find(loc.indexes.begin(), loc.indexes.end(), *it) == loc.indexes.end()) {
                loc.indexes.push_back(*it);
            }
        }
    }
}

void Parser::add_listen(const std::vector<std::string> &args) {
    const std::vector<std::string> &splitted = split_str(args.front(), ":");

    std::string host;
    int port;
    if (splitted.size() == 1) {
        if (is_host(splitted.front())) {
            host = splitted.front();
            if (host == "localhost") {
                host = "127.0.0.1";
            } else if (host == "*") {
                host = "0.0.0.0";
            }
            port = 80;
        } else {
            port = std::atoi(splitted.front().c_str());
            host = "0.0.0.0";
        }
    }

    if (splitted.size() == 2) {
        host = splitted.front();
        if (host == "localhost") {
            host = "127.0.0.1";
        } else if (host == "*") {
            host = "0.0.0.0";
        }
        port = std::atoi(splitted.back().c_str());
    }

    ctx_servers_.back().host_ports.push_back(std::make_pair(host, port));
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

// ブロックを内包するかどうかで判断しても良さそう
bool is_block(Directive dir) {
    return dir.block.size() != 0 && dir.name != "limit_except";
}

// blockディレクティブの場合はstackに積んでおいてあとから処理する
std::vector<ContextServer> Parser::parse(std::vector<Directive> vdir) {
    add_directives_func_map = setting_directive_functions();
    std::queue<Directive> que;

    for (std::vector<Directive>::iterator it = vdir.begin(); it != vdir.end(); ++it) {
        DXOUT(it->name);

        if (is_block(*it)) {
            que.push(*it);
        } else {
            // ディレクティブの処理
            add_directive_functions f = add_directives_func_map[it->name];
            (this->*f)(it->args);
        }
    }
    // ブロックディレクティブの処理
    while (!que.empty()) {
        ContextType before = ctx_;
        Directive d        = que.front();
        DXOUT(d.name);
        add_directive_functions f = add_directives_func_map[d.name];
        (this->*f)(d.args);
        parse(d.block);

        ctx_ = before;
        que.pop();
    }
    return ctx_servers_;
}

/// Debug functions
void Parser::print_directives(const std::vector<Directive> &d, const bool &is_block, const std::string &before) {
    std::string dir = is_block ? "Dire" : "Block";

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
            print_directives(d[i].block, true, b);
        }
    }
}

void Parser::print_limit_except(const ContextLimitExcept *lmt) {
    if (lmt == NULL) {
        std::cout << std::setw(INDENT_SIZE) << std::left << "  LimitExcept"
                  << ": { }" << std::endl;
        return;
    }
    std::cout << std::setw(INDENT_SIZE) << std::left << "  LimitExcept {" << std::endl;
    std::cout << std::setw(INDENT_SIZE) << std::left << "  allowed_methods"
              << ": { ";
    for (std::set<enum Methods>::iterator it = lmt->allowed_methods.begin(); it != lmt->allowed_methods.end(); ++it) {
        if (it != lmt->allowed_methods.begin()) {
            std::cout << ", ";
        }
        switch (*it) {
            case GET:
                std::cout << "GET";
                break;
            case POST:
                std::cout << "POST";
                break;
            case DELETE:
                std::cout << "DELETE";
                break;
            default:;
        }
    }
    std::cout << "  }" << std::endl;
}

void Parser::print_location(const std::vector<ContextLocation> &loc) {
    if (loc.size() == 0) {
        std::cout << std::setw(INDENT_SIZE) << std::left << "Locations"
                  << ": { }" << std::endl;
        return;
    }

    size_t i = 0;
    for (std::vector<ContextLocation>::const_iterator it = loc.begin(); it != loc.end(); ++it) {
        std::cout << "Locations[" << i++ << "]  {" << std::endl;
        print_key_value("location_path", it->path, true);
        print_key_value("client_max_body_size", it->client_max_body_size, true);
        print_key_value("autoindex", it->autoindex, true);
        print_key_value("root", it->root, true);
        print_key_value("indexes", vector_to_string(it->indexes), true);
        print_key_value("error_pages", map_to_string(it->error_pages), true);
        print_key_value("redirect", pair_to_string(it->redirect), true);
        print_limit_except(it->limit_except);
        print_location(it->locations);
        std::cout << "}" << std::endl;
    }
}

void Parser::print_server(const ContextServer &serv) {
    print_key_value("client_max_body_size", serv.client_max_body_size);
    print_key_value("autoindex", serv.autoindex);
    print_key_value("root", serv.root);
    print_key_value("indexes", vector_to_string(serv.indexes));
    print_key_value("error_pages", map_to_string(serv.error_pages));
    print_key_value("host, port", vector_pair_to_string(serv.host_ports));
    print_key_value("upload_store", serv.upload_store);
    print_key_value("server_names", vector_to_string(serv.server_names));
    print_key_value("redirect", pair_to_string(serv.redirect));
    print_location(serv.locations);
}
} // namespace config
