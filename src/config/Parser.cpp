#include "Parser.hpp"
#include "../utils/test_common.hpp"
#include "Config.hpp"
#include "ConfigUtility.hpp"
#include "Context.hpp"
#include "Lexer.hpp"
#include "Validator.hpp"
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <map>
#include <stack>
#include <utility>

namespace config {

Parser::Parser(void) {
    adder_maps = setting_directive_functions();
}
Parser::~Parser(void) {}

Parser::DirectiveFunctionsMap Parser::setting_directive_functions(void) {
    DirectiveFunctionsMap directives;

    directives["http"]                 = &Parser::add_http;
    directives["server"]               = &Parser::add_server;
    directives["location"]             = &Parser::add_location;
    directives["limit_except"]         = &Parser::add_limit_except;
    directives["autoindex"]            = &Parser::add_autoindex;
    directives["error_page"]           = &Parser::add_error_page;
    directives["index"]                = &Parser::add_index;
    directives["listen"]               = &Parser::add_listen;
    directives["return"]               = &Parser::add_return;
    directives["root"]                 = &Parser::add_root;
    directives["server_name"]          = &Parser::add_server_name;
    directives["client_max_body_size"] = &Parser::add_client_max_body_size;
    directives["upload_store"]         = &Parser::add_upload_store;
    return directives;
}

std::vector<Config> Parser::parse(const std::string &file_data) {
    lexer_.tokenize(file_data);

    ErrorMsg err;
    if ((err = brace_balanced()) != "") {
        throw SyntaxError(err);
    }
    std::vector<Directive> analyzed = clusterize();

    std::stack<ContextType> ctx;
    ctx.push(GLOBAL);
    std::vector<ContextServer> server_configs = forestize(analyzed, ctx);
    if (is_conflicted_server_name(server_configs)) {
        throw SyntaxError("config: conflicting server name");
    }
    inherit_data(server_configs);
    return create_configs(server_configs);
}

std::vector<Config> Parser::create_configs(const std::vector<ContextServer> &ctx_servers) {
    std::vector<Config> configs;
    std::vector<ContextServer>::const_iterator it = ctx_servers.begin();
    for (; it != ctx_servers.end(); ++it) {
#ifdef NDEBUG
        print_server(*it);
#endif
        for (size_t i = 0; i < it->host_ports.size(); ++i) {
            Config conf(*it);
            conf.set_host_port(it->host_ports[i]);
            conf.set_is_default_server(it->is_default_servers[i]);
            configs.push_back(conf);
        }
    }
    return configs;
}

ErrorMsg Parser::brace_balanced(void) {
    int depth = 0;
    int line  = 0;

    Token *tok;
    while ((tok = lexer_.read()) != NULL) {
        line = tok->line;
        if (tok->value == "}" && !tok->is_quoted) {
            depth -= 1;
        } else if (tok->value == "{" && !tok->is_quoted) {
            depth += 1;
        }
        if (depth < 0) {
            return Validator::validation_error("unexpected \"}\"", line);
        }
    }
    if (depth > 0) {
        return Validator::validation_error("unexpected end of file, expecting \"}\"", line);
    }
    lexer_.reset_read_idx();
    return "";
}

/// Analyze

bool Parser::is_special(const std::string &s) const {
    return s == "{" || s == "}" || s == ";";
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

/**
 * @brief lexerによって分割されたtokenを解析する
 * 1. `}`のチェック
 * 2. 引数を追加する
 * 3. 構文解析
 *   - ディレクティブが正しいか
 *   - 引数の数が正しいか
 *   - 現在のコンテキストに含まれていても良いか
 * 4. ブロックディレクティブの場合は再帰的に処理する
 */
std::vector<Directive> Parser::clusterize(std::vector<std::string> ctx) {
    std::vector<Directive> parsed;

    Token *cur;
    while (1) {
        if ((cur = lexer_.read()) == NULL) {
            break;
        }
        if (cur->value == "}" && !cur->is_quoted) {
            break;
        }

        // Directives
        Directive dire(cur->value, cur->line);
        if ((cur = lexer_.read()) == NULL) {
            break;
        }

        // Arguments
        while (cur->is_quoted || !is_special(cur->value)) {
            dire.args.push_back(cur->value);
            dire.line = cur->line;
            if ((cur = lexer_.read()) == NULL) {
                return parsed;
            }
        }

        // Validation
        ErrorMsg err;
        if ((err = Validator::validate(dire, cur->value, ctx)) != "") {
            throw SyntaxError(err);
        }

        // For block directives, forestize recursively
        if (cur->value == "{" && !cur->is_quoted) {
            std::vector<std::string> inner;
            inner      = enter_block_ctx(dire, ctx);
            dire.block = clusterize(inner);
        }
        parsed.push_back(dire);
    }
    return parsed;
}

/// Adder

void Parser::add_http(const std::vector<std::string> &args, std::stack<ContextType> &ctx) {
    (void)args;
    ctx.push(MAIN);
}

void Parser::add_server(const std::vector<std::string> &args, std::stack<ContextType> &ctx) {
    (void)args;
    ContextServer srv;
    ctx_servers_.push_back(srv);
    ctx.push(SERVER);
}

void Parser::add_location(const std::vector<std::string> &args, std::stack<ContextType> &ctx) {
    ContextLocation loc;
    loc.path = args.front();

    ContextServer &srv = ctx_servers_.back();
    if (ctx.top() == SERVER) {
        srv.locations.push_back(loc);
    }
    if (ctx.top() == LOCATION) {
        ContextLocation *p = get_current_location(ctx);
        if (loc.path.find(p->path) != 0) {
            throw SyntaxError("config: " + dquote(loc.path) + " is outside location " + dquote(p->path));
        }
        p->locations.push_back(loc);
    }
    ctx.push(LOCATION);
}

void Parser::add_limit_except(const std::vector<std::string> &args, std::stack<ContextType> &ctx) {
    ContextLimitExcept lmt;
    for (std::vector<std::string>::const_iterator it = args.begin(); it != args.end(); ++it) {
        if (str_tolower(*it) == "get") {
            lmt.allowed_methods.insert(GET);
        }
        if (str_tolower(*it) == "post") {
            lmt.allowed_methods.insert(POST);
        }
        if (str_tolower(*it) == "delete") {
            lmt.allowed_methods.insert(DELETE);
        }
    }
    ContextLocation *p = get_current_location(ctx);
    p->limit_except    = lmt;
    ctx.push(LIMIT_EXCEPT);
}

void Parser::add_autoindex(const std::vector<std::string> &args, std::stack<ContextType> &ctx) {
    const bool flag = (str_tolower(args.front()) == "on");

    switch (ctx.top()) {
        case MAIN:
            ctx_main_.autoindex = flag;
            break;
        case SERVER:
            ctx_servers_.back().autoindex             = flag;
            ctx_servers_.back().defined_["autoindex"] = true;
            break;
        case LOCATION: {
            ContextLocation *p       = get_current_location(ctx);
            p->autoindex             = flag;
            p->defined_["autoindex"] = true;
            break;
        }
        default:;
    }
}

void Parser::add_error_page(const std::vector<std::string> &args, std::stack<ContextType> &ctx) {
    std::vector<int> error_codes;
    for (size_t i = 0; i < args.size() - 1; ++i) {
        error_codes.push_back(std::atoi(args[i].c_str()));
    }
    const std::string path = args.back();
    switch (ctx.top()) {
        case MAIN:
            for (std::vector<int>::iterator it = error_codes.begin(); it != error_codes.end(); ++it) {
                ctx_main_.error_pages[*it] = path;
            }
            break;
        case SERVER:
            for (std::vector<int>::iterator it = error_codes.begin(); it != error_codes.end(); ++it) {
                ctx_servers_.back().error_pages[*it] = path;
            }
            ctx_servers_.back().defined_["error_page"] = true;
            break;
        case LOCATION: {
            ContextLocation *p = get_current_location(ctx);
            for (std::vector<int>::iterator it = error_codes.begin(); it != error_codes.end(); ++it) {
                p->error_pages[*it] = path;
            }
            p->defined_["error_page"] = true;
            break;
        }
        default:;
    }
}

void Parser::add_index(const std::vector<std::string> &args, std::stack<ContextType> &ctx) {
    if (ctx.top() == SERVER) {
        ContextServer &srv = ctx_servers_.back();
        for (std::vector<std::string>::const_iterator it = args.begin(); it != args.end(); ++it) {
            if (std::find(srv.indexes.begin(), srv.indexes.end(), *it) == srv.indexes.end()) {
                srv.indexes.push_back(*it);
            }
        }
        srv.defined_["index"] = true;
    }
    if (ctx.top() == LOCATION) {
        ContextLocation *p = get_current_location(ctx);
        for (std::vector<std::string>::const_iterator it = args.begin(); it != args.end(); ++it) {
            if (std::find(p->indexes.begin(), p->indexes.end(), *it) == p->indexes.end()) {
                p->indexes.push_back(*it);
            }
        }
        p->defined_["index"] = true;
    }
}

void Parser::add_listen(const std::vector<std::string> &args, std::stack<ContextType> &ctx) {
    (void)ctx;
    const std::vector<std::string> splitted = split_str(args.front(), ":");

    std::string host = "0.0.0.0";
    int port         = 80;
    if (splitted.size() == 1) {
        if (Validator::is_host(splitted.front())) {
            host = splitted.front();
            if (host == "localhost") {
                host = "127.0.0.1";
            } else if (host == "*") {
                host = "0.0.0.0";
            }
        } else {
            port = std::atoi(splitted.front().c_str());
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

    ContextServer &srv = ctx_servers_.back();
    host_port_pair p(host, port);

    // 既にlistenで同じ引数が定義されていたらエラー
    if (std::find(srv.host_ports.begin(), srv.host_ports.end(), p) != srv.host_ports.end()) {
        throw SyntaxError("config: duplicate listen");
    }
    ctx_servers_.back().host_ports.push_back(std::make_pair(host, port));

    const bool flag = (args.back() == "default_server");
    ctx_servers_.back().is_default_servers.push_back(flag);
    ctx_servers_.back().defined_["listen"] = true;
}

void Parser::add_return(const std::vector<std::string> &args, std::stack<ContextType> &ctx) {
    const int status_code  = std::atoi(args.front().c_str());
    const std::string path = args.back();

    if (ctx.top() == SERVER) {
        ctx_servers_.back().redirect             = std::make_pair(status_code, path);
        ctx_servers_.back().defined_["redirect"] = true;
    }
    if (ctx.top() == LOCATION) {
        ContextLocation *p      = get_current_location(ctx);
        p->redirect             = std::make_pair(status_code, path);
        p->defined_["redirect"] = true;
    }
}

void Parser::add_root(const std::vector<std::string> &args, std::stack<ContextType> &ctx) {
    const std::string &path = args.front();

    ContextServer &srv = ctx_servers_.back();
    switch (ctx.top()) {
        case MAIN:
            if (ctx_main_.defined_["root"]) {
                throw SyntaxError("config: \"root\" directive is duplicate");
            }
            ctx_main_.root             = path;
            ctx_main_.defined_["root"] = true;

            break;
        case SERVER:
            if (srv.defined_["root"]) {
                throw SyntaxError("config: \"root\" directive is duplicate");
            }
            srv.root             = path;
            srv.defined_["root"] = true;
            break;
        case LOCATION: {
            ContextLocation *p = get_current_location(ctx);
            if (p->defined_["root"]) {
                throw SyntaxError("config: \"root\" directive is duplicate");
            }
            p->root             = path;
            p->defined_["root"] = true;
            break;
        }
        default:;
    }
}

void Parser::add_server_name(const std::vector<std::string> &args, std::stack<ContextType> &ctx) {
    (void)ctx;
    const std::vector<std::string> &server_names = args;
    ctx_servers_.back().server_names             = server_names;
    ctx_servers_.back().defined_["server_name"]  = true;
}

void Parser::add_client_max_body_size(const std::vector<std::string> &args, std::stack<ContextType> &ctx) {
    const long size = std::strtol(args.front().c_str(), NULL, 10);

    switch (ctx.top()) {
        case MAIN:
            ctx_main_.client_max_body_size = size;
            break;
        case SERVER:
            ctx_servers_.back().client_max_body_size             = size;
            ctx_servers_.back().defined_["client_max_body_size"] = true;
            break;
        case LOCATION: {
            ContextLocation *p                  = get_current_location(ctx);
            p->client_max_body_size             = size;
            p->defined_["client_max_body_size"] = true;
            break;
        }
        default:;
    }
}

void Parser::add_upload_store(const std::vector<std::string> &args, std::stack<ContextType> &ctx) {
    const std::string &path = args.front();
    ContextServer &srv      = ctx_servers_.back();

    if (ctx.top() == SERVER) {
        srv.upload_store             = path;
        srv.defined_["upload_store"] = true;
    }
    if (ctx.top() == LOCATION) {
        ContextLocation *p          = get_current_location(ctx);
        p->upload_store             = path;
        p->defined_["upload_store"] = true;
    }
}

/// Inheritance

void Parser::inherit_main_to_srv(const ContextMain &main, ContextServer &srv) {
    srv.client_max_body_size
        = srv.defined_["client_max_body_size"] ? srv.client_max_body_size : main.client_max_body_size;
    srv.autoindex   = srv.defined_["autoindex"] ? srv.autoindex : main.autoindex;
    srv.root        = srv.defined_["root"] ? srv.root : main.root;
    srv.indexes     = srv.defined_["index"] ? srv.indexes : main.indexes;
    srv.error_pages = srv.defined_["error_page"] ? srv.error_pages : main.error_pages;
}

void Parser::inherit_loc_to_loc(const ContextLocation &parent, ContextLocation &child) {
    child.client_max_body_size
        = child.defined_["client_max_body_size"] ? child.client_max_body_size : parent.client_max_body_size;
    child.autoindex    = child.defined_["autoindex"] ? child.autoindex : parent.autoindex;
    child.root         = child.defined_["root"] ? child.root : parent.root;
    child.indexes      = child.defined_["index"] ? child.indexes : parent.indexes;
    child.error_pages  = child.defined_["error_page"] ? child.error_pages : parent.error_pages;
    child.upload_store = child.defined_["upload_store"] ? child.upload_store : parent.upload_store;
    child.redirect     = (parent.redirect.first == REDIRECT_INITIAL_VALUE) ? child.redirect : parent.redirect;
}

void Parser::inherit_locations(const ContextLocation &parent, std::vector<ContextLocation> &locs) {
    for (std::vector<ContextLocation>::iterator it = locs.begin(); it != locs.end(); ++it) {
        inherit_loc_to_loc(parent, *it);
        inherit_locations(*it, it->locations);
    }
}

// main -> server -> locationの順に継承していく
void Parser::inherit_data(std::vector<ContextServer> &servers) {
    for (std::vector<ContextServer>::iterator it = servers.begin(); it != servers.end(); ++it) {
        inherit_main_to_srv(ctx_main_, *it);
        ContextLocation loc(*it);
        inherit_locations(loc, it->locations);
    }
}

bool is_block(std::string directive) {
    return (directive == "http" || directive == "server" || directive == "location" || directive == "limit_except");
}

bool Parser::is_conflicted_server_name(const std::vector<ContextServer> &servers) {
    for (size_t i = 0; i < servers.size(); ++i) {
        for (size_t j = i + 1; j < servers.size(); ++j) {
            const ContextServer &srv1              = servers[i];
            const ContextServer &srv2              = servers[j];
            const std::vector<host_port_pair> &hp1 = srv1.host_ports;
            const std::vector<host_port_pair> &hp2 = srv2.host_ports;

            for (size_t k = 0; k < hp2.size(); ++k) {
                // 同一のhostとportを持っているか
                if (std::find(hp1.begin(), hp1.end(), hp2[k]) != hp1.end()) {
                    const std::vector<std::string> &names1 = srv1.server_names;
                    const std::vector<std::string> &names2 = srv2.server_names;
                    for (size_t l = 0; l < names2.size(); ++l) {
                        // 同一のサーバーネームを持っているか
                        if (std::find(names1.begin(), names1.end(), names2[l]) != names1.end()) {
                            return true;
                        }
                    }
                }
            }
        }
    }
    return false;
}

size_t Parser::count_nested_locations(const std::stack<ContextType> &ctx) const {
    std::stack<ContextType> sta(ctx);
    size_t cnt = 0;
    while (!sta.empty() && sta.top() == LOCATION) {
        cnt += 1;
        sta.pop();
    }
    return cnt;
}

ContextLocation *Parser::get_current_location(const std::stack<ContextType> &ctx) {
    ContextLocation *p = &ctx_servers_.back().locations.back();

    const size_t &cnt = count_nested_locations(ctx);
    for (size_t i = 1; i < cnt; i++) {
        p = &p->locations.back();
    }
    return p;
}

std::vector<ContextServer> Parser::forestize(std::vector<Directive> vdir, std::stack<ContextType> &ctx) {
    for (std::vector<Directive>::iterator it = vdir.begin(); it != vdir.end(); ++it) {
        add_directive_functions f = adder_maps[it->name];
        (this->*f)(it->args, ctx);
        if (is_block(it->name)) {
            forestize(it->block, ctx);
            ctx.pop();
        }
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

void Parser::print_limit_except(const ContextLimitExcept &lmt) {
    std::cout << std::setw(INDENT_SIZE) << std::left << "  LimitExcept"
              << ": { ";
    for (std::set<enum Methods>::iterator it = lmt.allowed_methods.begin(); it != lmt.allowed_methods.end(); ++it) {
        if (it != lmt.allowed_methods.begin()) {
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
    std::cout << " }" << std::endl;
}

void Parser::print_location(const std::vector<ContextLocation> &loc) {
    if (loc.size() == 0) {
        std::cout << std::setw(INDENT_SIZE) << std::left << "Locations"
                  << ": {  }" << std::endl;
        return;
    }

    size_t i = 0;
    for (std::vector<ContextLocation>::const_iterator it = loc.begin(); it != loc.end(); ++it) {
        std::cout << "Locations[" << i++ << "]  {" << std::endl;
        print_key_value("location_path", it->path, true);
        print_key_value("client_max_body_size", it->client_max_body_size, true);
        print_key_value("autoindex", it->autoindex, true);
        print_key_value("root", it->root, true);
        print_key_value("index", vector_to_string(it->indexes), true);
        print_key_value("error_page", map_to_string(it->error_pages), true);
        print_key_value("upload_store", it->upload_store, true);
        print_key_value("redirect", pair_to_string(it->redirect), true);
        print_limit_except(it->limit_except);
        print_location(it->locations);
        std::cout << "}" << std::endl;
    }
}

void Parser::print_server(const ContextServer &srv) {
    print_key_value("client_max_body_size", srv.client_max_body_size);
    print_key_value("autoindex", srv.autoindex);
    print_key_value("root", srv.root);
    print_key_value("indexes", vector_to_string(srv.indexes));
    print_key_value("error_page", map_to_string(srv.error_pages));
    print_key_value("host, port", vector_pair_to_string(srv.host_ports));
    print_key_value("upload_store", srv.upload_store);
    print_key_value("server_name", vector_to_string(srv.server_names));
    print_key_value("redirect", pair_to_string(srv.redirect));
    print_location(srv.locations);
}
} // namespace config
