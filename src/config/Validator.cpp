#include "Validator.hpp"
#include "Lexer.hpp"
#include "Parser.hpp"
#include "test_common.hpp"
#include <iostream>
#include <locale>
#include <map>
#include <sstream>
#include <string>
#include <vector>

namespace config {

static std::map<std::vector<std::string>, int> setting_contexts(void);
static std::map<std::string, int> setting_directives(void);
static int get_directive_mask(std::string dire);
static int get_context_mask(std::vector<std::string> ctx);
static bool is_valid_flag(std::string s);
static bool is_must_be_on_off(Directive dire, int mask);

bool is_integer(const std::string &s) {
    if (s.empty()) {
        return false;
    }
    for (std::string::const_iterator it = s.begin(); it != s.end();) {
        if (!std::isdigit(*it)) {
            return false;
        }
    }
    return true;
}

std::vector<std::string> split_str(const std::string &s, const std::string &sep) {
    size_t len = sep.length();

    std::cout << "Split :" << s << std::endl;
    std::cout << "Sep   :" << sep << std::endl;

    std::vector<std::string> vec;
    if (len == 0) {
        vec.push_back(s);
        return vec;
    }

    size_t offset = 0;
    while (1) {
        size_t pos = s.find(sep, offset);
        if (pos == std::string::npos) {
            vec.push_back(s.substr(offset));
            break;
        }
        vec.push_back(s.substr(offset, pos - offset));
        offset = pos + len;
    }

    std::cout << "DONE" << std::endl;
    return vec;
}

bool is_ipaddr(const std::string &s) {
    std::vector<std::string> v(split_str(s, "."));
    // 要素が4つじゃなかったらout
    if (v.size() != 4) {
        return false;
    }
    // 範囲が0~255じゃなかったらOUT
    for (std::vector<std::string>::iterator it = v.begin(); it != v.end(); ++it) {
        if (!is_integer(*it)) {
            return false;
        }
        size_t n = std::stol(*it);
        if (!(0 <= n && n <= 255)) {
            return false;
        }
    }
    return true;
}

bool is_status_code(const std::string &arg) {
    if (!is_integer(arg)) {
        return false;
    }
    const int n = std::stoi(arg);

    return 300 <= n && n <= 599;
}

bool is_port(const std::string &arg) {
    if (!is_integer(arg)) {
        return false;
    }
    const int n = std::stoi(arg);
    return 0 <= n && n <= 65535;
}

bool is_valid_allow_deny(const std::string &arg) {
    if (arg == "all") {
        return true;
    }
    return is_ipaddr(arg);
}

// returnも
bool is_valid_error_page(const std::string &arg) {
    return is_status_code(arg);
}

bool is_valid_listen(const std::string &arg) {
    std::cout << "is_valid_listen: " << arg << std::endl;

    std::vector<std::string> v(split_str(arg, ":"));

    if (v.size() != 1 && v.size() != 2) {
        return false;
    }

    if (v.size() == 1) {
        if (v.front() == "localhost" || v.front() == "*") {
            return true;
        }
        if (is_port(v.front())) {
            return true;
        }
    }

    if (v.size() == 2) {
        if (v.front() == "localhost" || v.front() == "*") {
            return true;
        }
        if (is_ipaddr(v.front())) {
            return true;
        }

        if (is_port(v.back())) {
            return true;
        }
    }
    return false;
}

std::string validation_error(const std::string &message, const size_t &line, const std::string directive) {
    std::ostringstream oss;

    oss << "config: ";
    if (!directive.empty()) {
        oss << "\"" << directive << "\" directive ";
    }
    oss << message << " in line:" << line;
    return oss.str();
}

// ディレクティブとコンテキストのルールを設定する
static const std::map<std::string, int> directives            = setting_directives();
static const std::map<std::vector<std::string>, int> contexts = setting_contexts();

static std::map<std::string, int> setting_directives(void) {
    std::map<std::string, int> directives;

    /// Block
    directives["http"]         = (MAIN | BLOCK | NOARGS);
    directives["server"]       = (HTTP_MAIN | BLOCK | NOARGS);
    directives["location"]     = (HTTP_SRV | HTTP_LOC | BLOCK | TAKE12);
    directives["limit_except"] = (HTTP_LOC | BLOCK | MORE1);

    /// Normal
    directives["allow"] = (HTTP_MAIN | HTTP_SRV | HTTP_LOC | HTTP_LMT | TAKE1);
    directives["deny"]  = (HTTP_MAIN | HTTP_SRV | HTTP_LOC | HTTP_LMT | TAKE1);

    directives["autoindex"]   = (HTTP_MAIN | HTTP_SRV | HTTP_LOC | FLAG);
    directives["error_page"]  = (HTTP_MAIN | HTTP_SRV | HTTP_LOC | MORE2);
    directives["index"]       = (HTTP_MAIN | HTTP_SRV | HTTP_LOC | MORE1);
    directives["listen"]      = (HTTP_SRV | MORE1);
    directives["return"]      = (HTTP_SRV | HTTP_LOC | TAKE12);
    directives["root"]        = (HTTP_MAIN | HTTP_SRV | HTTP_LOC | TAKE1);
    directives["server_name"] = (HTTP_SRV | MORE1);

    directives["client_max_body_size"] = (HTTP_MAIN | HTTP_SRV | HTTP_LOC | TAKE1);

    /// Original
    directives["upload_store"] = (HTTP_SRV | HTTP_LOC | TAKE1);

    return directives;
}

static std::map<std::vector<std::string>, int> setting_contexts(void) {
    std::map<std::vector<std::string>, int> contexts;

    contexts[std::vector<std::string>()] = MAIN;

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

static int get_directive_mask(std::string dire) {
    std::map<std::string, int>::const_iterator it;
    int mask = 0;

    it = directives.find(dire);
    if (it != directives.end()) {
        mask = it->second;
    }
    return mask;
}

static int get_context_mask(std::vector<std::string> ctx) {
    std::map<std::vector<std::string>, int>::const_iterator it;
    int mask = 0;

    it = contexts.find(ctx);
    if (it != contexts.end()) {
        mask = it->second;
    }
    return mask;
}

// on または offが必須のもの
static bool is_valid_flag(std::string s) {
    std::locale loc;
    for (size_t i = 0; i < s.size(); i++) {
        s[i] = std::tolower(s[i], loc);
    }
    return s == "on" || s == "off";
}

bool is_correct_detail(Directive dire) {
    if (dire.directive == "allow" || dire.directive == "deny") {
        return is_valid_allow_deny(dire.args[0]);
    }
    if (dire.directive == "error_page" || dire.directive == "return") {
        return is_valid_error_page(dire.args[0]);
    }
    if (dire.directive == "listen") {
        return is_valid_listen(dire.args[0]);
    }
    return true;
}

/**
 * 引数の数チェック
 * (mask >> argsの数) & 1 = 1 (1であれば正しい)
 * (0x01>>0)&1 = 1 // 0 args (0001 -> 0001)
 * (0x02>>1)&1 = 1 // 1 args (0010 -> 0001)
 * (0x04>>2)&1 = 1 // 2 args (0100 -> 0001)
 * (0x08>>3)&1 = 1 // 3 args (1000 -> 0001)
 */
static bool is_correct_number_of_args(Directive dire, int mask) {
    // 引数の数が指定通りか
    if ((mask >> dire.args.size() & 1) != 0 && dire.args.size() <= 7) {
        return true;
    }

    // 引数が on, off か
    if ((mask & FLAG) != 0 && dire.args.size() == 1 && is_valid_flag(dire.args[0])) {
        return true;
    }

    // 引数の数が0以上か
    if ((mask & ANY) != 0 && dire.args.size() >= 0) {
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

    // どれも条件を満たしていない場合は引数が正しくない
    return false;
}

static bool is_must_be_on_off(Directive dire, int mask) {
    return ((mask & FLAG) != 0 && dire.args.size() == 1 && !is_valid_flag(dire.args[0]));
}

error_type validate(Directive dire, std::string term, std::vector<std::string> ctx) {
    const int dire_mask = get_directive_mask(dire.directive);
    const int ctx_mask  = get_context_mask(ctx);

    if (dire_mask == 0) {
        return validation_error("unknown directive", dire.line, dire.directive);
    }

    // ディレクティブがこのコンテキストで使用できない場合
    if ((dire_mask & ctx_mask) == 0) {
        return validation_error("is not allowed here", dire.line, dire.directive);
    }

    // ブロックディレクティブで波括弧が続いていない場合
    if ((dire_mask & BLOCK) != 0 && term != "{") {
        return validation_error(" has no opening \"{\"", dire.line, dire.directive);
    }

    // シンプルディレクティブで ";"が続いていない場合
    if ((dire_mask & BLOCK) == 0 && term != ";") {
        return validation_error("is not terminated by \";\"", dire.line, dire.directive);
    }

    // 引数の数が正しくない場合
    if (!is_correct_number_of_args(dire, dire_mask)) {
        if (is_must_be_on_off(dire, dire_mask)) {
            return validation_error("it must be \"on\" or \"off\"", dire.line, dire.directive);

        } else {
            return validation_error("invalid number of arguments", dire.line, dire.directive);
        }
    }

    if (!is_correct_detail(dire)) {
        return validation_error("invalid arguments", dire.line, dire.directive);
    }
    return "";
}
} // namespace config
