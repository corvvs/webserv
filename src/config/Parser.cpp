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

// ディレクティブとコンテキストを追加する関数を設定する
// Parser::DirectiveFunctionsMap Parser::setting_directive_functions(void) {
Parser::DirectiveFunctionsMap Parser::setting_directive_functions(void) {
    DirectiveFunctionsMap directives;

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

    // debug
    std::vector<ContextServer>::iterator it = server_configs.begin();
    for (; it != server_configs.end(); ++it) {
        print_server(*it);
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

// static std::map<std::string, void *f()()> add_directive_funcs;
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
    ctx_ = LIMIT_EXCEPT;

    ContextLimitExcept *lmt = new ContextLimitExcept;
    for (std::vector<std::string>::const_iterator it = args.begin(); it != args.end(); ++it) {
        if (utility::str_tolower(*it) == "get") {
            lmt->allowed_methods.insert(GET);
        }
        if (utility::str_tolower(*it) == "post") {
            lmt->allowed_methods.insert(POST);
        }
        if (utility::str_tolower(*it) == "delete") {
            lmt->allowed_methods.insert(DELETE);
        }
    }
    ctx_servers_.back().locations.back().limit_except = lmt;
}

/// Normal
void Parser::add_allow(const std::vector<std::string> &args) {
    std::string ip_addr = args.front();
    if (args.front() == "all") {
        ip_addr = "0.0.0.0";
    }
    switch (ctx_) {
        case MAIN:
            ctx_main_.allow = ip_addr;
            break;
        case SERVER:
            ctx_servers_.back().allow = ip_addr;
            break;
        case LOCATION:
            ctx_servers_.back().locations.back().allow = ip_addr;
            break;
        case LIMIT_EXCEPT:
            ctx_servers_.back().locations.back().limit_except->allow = ip_addr;
            break;
        default:;
    }
}

void Parser::add_deny(const std::vector<std::string> &args) {
    std::string ip_addr = args.front();
    if (args.front() == "all") {
        ip_addr = "0.0.0.0";
    }
    switch (ctx_) {
        case MAIN:
            ctx_main_.deny = ip_addr;
            break;
        case SERVER:
            ctx_servers_.back().deny = ip_addr;
            break;
        case LOCATION:
            ctx_servers_.back().locations.back().deny = ip_addr;
            break;
        case LIMIT_EXCEPT:
            ctx_servers_.back().locations.back().limit_except->deny = ip_addr;
            break;
        default:;
    }
}

void Parser::add_autoindex(const std::vector<std::string> &args) {
    const bool &flag = (utility::str_tolower(args.front()) == "on");

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
    const std::vector<std::string> &splitted = utility::split_str(args.front(), ":");

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
    return dir.block.size() != 0;
}

// blockディレクティブの場合はstackに積んでおいてあとから処理する
std::vector<ContextServer> Parser::parse(std::vector<Directive> vdir) {
    add_directives_func_map = setting_directive_functions();
    std::queue<Directive> que;

    for (std::vector<Directive>::iterator it = vdir.begin(); it != vdir.end(); ++it) {
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
        ContextType before        = ctx_;
        Directive d               = que.front();
        add_directive_functions f = add_directives_func_map[d.name];
        (this->*f)(d.args);
        parse(d.block);

        ctx_ = before;
        que.pop();
    }
    return ctx_servers_;
}

/*************************************************************/
// debug
void print_directives(std::vector<Directive> d, bool is_block, std::string before) {
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
            print_directives(d[i].block, true, b);
        }
    }
}

template <class First, class Second>
std::string pair_to_string(std::pair<First, Second> p) {
    std::ostringstream oss;
    oss << "< ";
    oss << p.first << ", " << p.second << " >";
    return oss.str();
}

template <class T>
std::string vector_to_string(std::vector<T> v) {
    std::ostringstream oss;
    oss << "{ ";
    for (typename std::vector<T>::iterator it = v.begin(); it != v.end(); ++it) {
        if (it != v.begin()) {
            oss << ", ";
        }
        oss << *it;
    }
    oss << " }";
    return oss.str();
}

template <class First, class Second>
std::string vector_pair_to_string(std::vector<std::pair<First, Second> > vp) {
    std::ostringstream oss;
    oss << "{ ";
    for (typename std::vector<std::pair<First, Second> >::iterator it = vp.begin(); it != vp.end(); ++it) {
        if (it != vp.begin()) {
            oss << ", ";
        }
        oss << pair_to_string(*it);
    }
    oss << " }";
    return oss.str();
}

template <class Key, class Value>
std::string map_to_string(std::map<Key, Value> mp) {
    std::ostringstream oss;
    oss << "{ ";
    for (typename std::map<Key, Value>::iterator it = mp.begin(); it != mp.end(); ++it) {
        oss << "< " << it->first << ", " << it->second << " >";
    }
    oss << " }";
    return oss.str();
}

template <class T>
std::string set_to_string(std::set<T> st) {
    std::ostringstream oss;
    oss << "{ ";
    for (typename std::set<T>::iterator it = st.begin(); it != st.end(); ++it) {
        if (it != st.begin()) {
            oss << ", ";
        }
        oss << *it;
    }
    oss << " }";
    return oss.str();
}

void Parser::indent(size_t size) {
    for (size_t i = 0; i < size; i++) {
        std::cout << " ";
    }
}

template <class Key, class Value>
void Parser::print_key_value(const Key &key, const Value &value, bool has_indent) {
    int size = 24;
    if (has_indent) {
        indent(2);
        size = 22;
    }
    std::cout << std::setw(size) << std::left << key << ": " << value << std::endl;
}

void Parser::print_limit_except(const ContextLimitExcept *lmt) {
    if (lmt == NULL) {
        std::cout << std::setw(24) << std::left << "  LimitExcept"
                  << ": { }" << std::endl;
        return;
    }
    std::cout << std::setw(24) << std::left << "  LimitExcept {" << std::endl;
    std::cout << std::setw(24) << std::left << "  allowed_methods"
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
    std::cout << " }" << std::endl;
    print_key_value("allow", lmt->allow, true);
    print_key_value("deny", lmt->deny, true);
    std::cout << "  }" << std::endl;
}

void Parser::print_location(const std::vector<ContextLocation> &loc) {
    if (loc.size() == 0) {
        std::cout << std::setw(24) << std::left << "Locations"
                  << ": { }" << std::endl;
        return;
    }

    size_t i = 0;
    for (std::vector<ContextLocation>::const_iterator it = loc.begin(); it != loc.end(); ++it) {
        std::cout << "Locations[" << i++ << "]  {" << std::endl;
        print_key_value("location_path", it->path, true);
        print_key_value("client_max_body_size", it->client_max_body_size, true);
        print_key_value("autoindex", it->autoindex, true);
        print_key_value("allow", it->allow, true);
        print_key_value("deny", it->deny, true);
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
    print_key_value("allow", serv.allow);
    print_key_value("deny", serv.deny);
    print_key_value("root", serv.root);
    print_key_value("indexes", vector_to_string(serv.indexes));
    print_key_value("error_pages", map_to_string(serv.error_pages));
    print_key_value("host, port", vector_pair_to_string(serv.host_ports));
    print_key_value("upload_store", serv.upload_store);
    print_key_value("server_names", vector_to_string(serv.server_names));
    print_key_value("redirect", pair_to_string(serv.redirect));
    print_location(serv.locations);
}
/*************************************************************/
} // namespace config
