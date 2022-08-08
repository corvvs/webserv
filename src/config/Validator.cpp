#include "Validator.hpp"
#include "../utils/test_common.hpp"
#include "ConfigUtility.hpp"
#include "Lexer.hpp"
#include "Parser.hpp"
#include <iostream>
#include <locale>
#include <map>
#include <string>
#include <vector>

namespace config {
namespace Validator {

std::string validation_error(const std::string &message, const size_t &line, const std::string name) {
    std::ostringstream oss;

    oss << "config: ";
    if (!name.empty()) {
        oss << "\"" << name << "\" directive ";
    }
    oss << message << " in line:" << line;
    return oss.str();
}

bool is_valid_integer(const std::string &s) {
    char *err;
    std::strtol(s.c_str(), &err, 10);
    if (*err != '\0' || errno == ERANGE || errno == EINVAL) {
        return false;
    }
    return true;
}

bool is_ipaddr(const std::string &s) {
    const std::vector<std::string> splitted(split_str(s, "."));

    if (splitted.size() != 4) {
        return false;
    }
    for (std::vector<std::string>::const_iterator it = splitted.begin(); it != splitted.end(); ++it) {
        if (!is_valid_integer(*it)) {
            return false;
        }

        std::istringstream iss(*it);
        long n;
        iss >> n;
        if (!(0 <= n && n <= 255)) {
            return false;
        }
    }
    return true;
}

bool is_host(const std::string &s) {
    if (s == "localhost" || s == "*") {
        return true;
    }
    return is_ipaddr(s);
}

bool is_port(const std::string &arg) {
    if (!is_valid_integer(arg)) {
        return false;
    }
    std::istringstream iss(arg);
    long n;
    iss >> n;
    return 0 <= n && n <= 65535;
}

bool is_valid_error_page(const std::vector<std::string> &args) {
    for (size_t i = 0; i < args.size() - 1; ++i) {
        if (!is_valid_integer(args[i])) {
            return false;
        }
        std::istringstream iss(args[i]);
        long n;
        iss >> n;
        if (!(300 <= n && n <= 599)) {
            return false;
        }
    }
    return true;
}

bool is_valid_return(const std::vector<std::string> &args) {
    if (!is_valid_integer(args.front())) {
        return false;
    }
    std::istringstream iss(args.front());
    long n;
    iss >> n;
    return 0 <= n && n <= 999;
}

bool is_valid_client_max_body_size(const std::vector<std::string> &args) {
    if (!is_valid_integer(args.front())) {
        return false;
    }
    std::istringstream iss(args.front());
    long n;
    iss >> n;
    return n >= 0;
}

bool is_valid_listen(const std::vector<std::string> &args) {
    if (args.size() == 2 && args.back() != "default_server") {
        return false;
    }

    const std::vector<std::string> splitted(split_str(args.front(), ":"));
    if (splitted.size() != 1 && splitted.size() != 2) {
        return false;
    }

    if (splitted.size() == 1) {
        if (is_host(splitted.front()) || is_port(splitted.front())) {
            return true;
        }
    }

    if (splitted.size() == 2) {
        if (is_host(splitted.front()) && is_port(splitted.back())) {
            return true;
        }
    }
    return false;
}

// ディレクティブとコンテキストのルールを設定する
const std::map<std::string, int> directives            = setting_directives();
const std::map<std::vector<std::string>, int> contexts = setting_contexts();

std::map<std::string, int> setting_directives(void) {
    std::map<std::string, int> directives;

    /// Block
    directives["http"]         = (GLOBAL | BLOCK | NOARGS);
    directives["server"]       = (HTTP_MAIN | BLOCK | NOARGS);
    directives["location"]     = (HTTP_SRV | HTTP_LOC | BLOCK | TAKE12);
    directives["limit_except"] = (HTTP_LOC | BLOCK | MORE1);

    /// Simple
    directives["autoindex"]            = (HTTP_MAIN | HTTP_SRV | HTTP_LOC | FLAG);
    directives["error_page"]           = (HTTP_MAIN | HTTP_SRV | HTTP_LOC | MORE2);
    directives["index"]                = (HTTP_MAIN | HTTP_SRV | HTTP_LOC | MORE1);
    directives["root"]                 = (HTTP_MAIN | HTTP_SRV | HTTP_LOC | TAKE1);
    directives["client_max_body_size"] = (HTTP_MAIN | HTTP_SRV | HTTP_LOC | TAKE1);
    directives["server_name"]          = (HTTP_SRV | MORE1);
    directives["listen"]               = (HTTP_SRV | MORE1);
    directives["return"]               = (HTTP_SRV | HTTP_LOC | TAKE12);
    directives["upload_store"]         = (HTTP_SRV | HTTP_LOC | TAKE1);

    /// Original
    directives["exec_cgi"]    = (HTTP_LOC | FLAG);
    directives["exec_delete"] = (HTTP_LOC | FLAG);
    directives["cgi_path"]    = (HTTP_LOC | TAKE2);

    return directives;
}

std::map<std::vector<std::string>, int> setting_contexts(void) {
    std::map<std::vector<std::string>, int> contexts;

    contexts[std::vector<std::string>()] = GLOBAL;

    std::vector<std::string> v;
    v.push_back("http");
    contexts[v] = HTTP_MAIN;
    v.push_back("server");
    contexts[v] = HTTP_SRV;

    v.clear();
    v.push_back("http");
    v.push_back("location");
    contexts[v] = HTTP_LOC;
    v.push_back("limit_except");
    contexts[v] = HTTP_LMT;

    return contexts;
}

int get_directive_mask(std::string dire) {
    int mask = 0;
    std::map<std::string, int>::const_iterator it;
    it = directives.find(dire);
    if (it != directives.end()) {
        mask = it->second;
    }
    return mask;
}

int get_context_mask(std::vector<std::string> ctx) {
    int mask = 0;
    std::map<std::vector<std::string>, int>::const_iterator it;
    it = contexts.find(ctx);
    if (it != contexts.end()) {
        mask = it->second;
    }
    return mask;
}

bool is_valid_flag(std::string s) {
    std::locale loc;
    for (size_t i = 0; i < s.size(); i++) {
        s[i] = std::tolower(s[i], loc);
    }
    return s == "on" || s == "off";
}

bool is_correct_details(Directive dire) {
    if (dire.name == "error_page") {
        return is_valid_error_page(dire.args);
    }
    if (dire.name == "return") {
        return is_valid_return(dire.args);
    }
    if (dire.name == "listen") {
        return is_valid_listen(dire.args);
    }
    if (dire.name == "client_max_body_size") {
        return is_valid_client_max_body_size(dire.args);
    }
    return true;
}

bool is_must_be_on_off(Directive dire, int mask) {
    return ((mask & FLAG) != 0 && dire.args.size() == 1 && !is_valid_flag(dire.args[0]));
}

/**
 * @brief 引数の数が正しいか判定する
 * (mask >> argsの数) & 1 = 1 (1であれば正しい)
 * (0x01>>0)&1 = 1 // 0 args (0001 -> 0001)
 * (0x02>>1)&1 = 1 // 1 args (0010 -> 0001)
 * (0x04>>2)&1 = 1 // 2 args (0100 -> 0001)
 * (0x08>>3)&1 = 1 // 3 args (1000 -> 0001)
 */
bool is_correct_number_of_args(Directive dire, int mask) {
    // 引数の数が指定通りか
    if ((mask >> dire.args.size() & 1) != 0 && dire.args.size() <= 7) {
        return true;
    }

    // 引数が on, off か
    if ((mask & FLAG) != 0 && dire.args.size() == 1 && is_valid_flag(dire.args[0])) {
        return true;
    }

    // 引数の数が0以上か
    if ((mask & ANY) != 0) {
        return true;
    }

    // 引数の数が1以上か
    if ((mask & MORE1) != 0 && dire.args.size() >= 1) {
        return true;
    }

    // 引数の数が2以上か
    if ((mask & MORE2) != 0 && dire.args.size() >= 2) {
        return true;
    }

    return false;
}

ErrorMsg validate(Directive dire, std::string term, std::vector<std::string> ctx) {
    const int dire_mask = get_directive_mask(dire.name);
    const int ctx_mask  = get_context_mask(ctx);

    if (dire_mask == 0) {
        return validation_error("unknown name", dire.line, dire.name);
    }

    // ディレクティブが現在のコンテキストで使用できない場合
    if ((dire_mask & ctx_mask) == 0) {
        return validation_error("is not allowed here", dire.line, dire.name);
    }

    // ブロックディレクティブで `{` が続いていない場合
    if ((dire_mask & BLOCK) != 0 && term != "{") {
        return validation_error("has no opening \"{\"", dire.line, dire.name);
    }

    // シンプルディレクティブで `;` が続いていない場合
    if ((dire_mask & BLOCK) == 0 && term != ";") {
        return validation_error("is not terminated by \";\"", dire.line, dire.name);
    }

    // 引数の数が正しくない場合
    if (!is_correct_number_of_args(dire, dire_mask)) {
        if (is_must_be_on_off(dire, dire_mask)) {
            return validation_error("it must be \"on\" or \"off\"", dire.line, dire.name);
        } else {
            return validation_error("invalid number of arguments", dire.line, dire.name);
        }
    }
    // 数値の範囲など詳細なチェック(あるものだけ)
    if (!is_correct_details(dire)) {
        return validation_error("invalid arguments", dire.line, dire.name);
    }
    return "";
}
} // namespace Validator
} // namespace config
