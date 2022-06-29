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

static std::map<std::vector<std::string>, int> setting_contexts(void);
static std::map<std::string, int> setting_directives(void);
static int get_directive_mask(std::string dire);
static int get_context_mask(std::vector<std::string> ctx);
static bool is_valid_flag(std::string s);
static bool is_must_be_on_off(Directive dire, int mask);

// ディレクティブとコンテキストのルールを設定する
static const std::map<std::string, int> directives            = setting_directives();
static const std::map<std::vector<std::string>, int> contexts = setting_contexts();

// nginx: [emerg] "root" directive is not allowed here in /usr/local/etc/nginx/nginx.conf:4
std::string validation_error(const std::string &message, const std::string &directive, const size_t &line = 0) {
    std::ostringstream oss;

    oss << "config: ";
    oss << "\"" << directive << "\" directive ";
    oss << message << " in line: " << line;
    return oss.str();
}

static std::map<std::string, int> setting_directives(void) {
    std::map<std::string, int> directives;

    /// Block
    directives["http"]         = (WS_CONF::MAIN | WS_CONF::BLOCK | WS_CONF::NOARGS);
    directives["server"]       = (WS_CONF::HTTP_MAIN | WS_CONF::BLOCK | WS_CONF::NOARGS);
    directives["location"]     = (WS_CONF::HTTP_SRV | WS_CONF::HTTP_LOC | WS_CONF::BLOCK | WS_CONF::TAKE12);
    directives["limit_except"] = (WS_CONF::HTTP_LOC | WS_CONF::BLOCK | WS_CONF::MORE1);

    /// Normal
    directives["allow"]
        = (WS_CONF::HTTP_MAIN | WS_CONF::HTTP_SRV | WS_CONF::HTTP_LOC | WS_CONF::HTTP_LMT | WS_CONF::TAKE1);
    directives["deny"]
        = (WS_CONF::HTTP_MAIN | WS_CONF::HTTP_SRV | WS_CONF::HTTP_LOC | WS_CONF::HTTP_LMT | WS_CONF::TAKE1);

    directives["autoindex"]   = (WS_CONF::HTTP_MAIN | WS_CONF::HTTP_SRV | WS_CONF::HTTP_LOC | WS_CONF::FLAG);
    directives["error_page"]  = (WS_CONF::HTTP_MAIN | WS_CONF::HTTP_SRV | WS_CONF::HTTP_LOC | WS_CONF::MORE2);
    directives["index"]       = (WS_CONF::HTTP_MAIN | WS_CONF::HTTP_SRV | WS_CONF::HTTP_LOC | WS_CONF::MORE1);
    directives["listen"]      = (WS_CONF::HTTP_SRV | WS_CONF::MORE1);
    directives["return"]      = (WS_CONF::HTTP_SRV | WS_CONF::HTTP_LOC | WS_CONF::TAKE12);
    directives["root"]        = (WS_CONF::HTTP_MAIN | WS_CONF::HTTP_SRV | WS_CONF::HTTP_LOC | WS_CONF::TAKE1);
    directives["server_name"] = (WS_CONF::HTTP_SRV | WS_CONF::MORE1);

    /// Original
    directives["upload_store"] = (WS_CONF::HTTP_SRV | WS_CONF::HTTP_LOC | WS_CONF::TAKE1);

    return directives;
}

static std::map<std::vector<std::string>, int> setting_contexts(void) {
    std::map<std::vector<std::string>, int> contexts;

    contexts[std::vector<std::string>()] = WS_CONF::MAIN;

    std::vector<std::string> v;
    v.push_back("http");
    contexts[v] = WS_CONF::HTTP_MAIN;
    v.push_back("server");
    contexts[v] = WS_CONF::HTTP_SRV;

    v.clear();
    v.push_back("http");
    v.push_back("location");
    contexts[v] = WS_CONF::HTTP_LOC;
    v.push_back("limit_except");
    contexts[v] = WS_CONF::HTTP_LMT;

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
    if ((mask & WS_CONF::FLAG) != 0 && dire.args.size() == 1 && is_valid_flag(dire.args[0])) {
        return true;
    }

    // 引数の数が0以上か
    if ((mask & WS_CONF::ANY) != 0 && dire.args.size() >= 0) {
        return true;
    }

    // 引数の数が1以上か
    if ((mask & WS_CONF::MORE1) != 0 && dire.args.size() >= 1) {
        return true;
    }

    // 引数の数が2以上か
    if ((mask & WS_CONF::MORE2) != 0 && dire.args.size() >= 2) {
        return true;
    }

    // どれも条件を満たしていない場合は引数が正しくない
    return false;
}

static bool is_must_be_on_off(Directive dire, int mask) {
    return ((mask & WS_CONF::FLAG) != 0 && dire.args.size() == 1 && !is_valid_flag(dire.args[0]));
}

std::string validate(Directive dire, std::string term, std::vector<std::string> ctx) {
    const int dire_mask = get_directive_mask(dire.directive);
    const int ctx_mask  = get_context_mask(ctx);

    if (dire_mask == 0) {
        return validation_error("unknown directive", dire.directive, dire.line);
    }

    // ディレクティブがこのコンテキストで使用できない場合
    if ((dire_mask & ctx_mask) == 0) {
        return validation_error("is not allowed here", dire.directive, dire.line);
    }

    // ブロックディレクティブで波括弧が続いていない場合
    if ((dire_mask & WS_CONF::BLOCK) != 0 && term != "{") {
        return validation_error(" has no opening \"{\"", dire.directive, dire.line);
    }

    // シンプルディレクティブで ";"が続いていない場合
    if ((dire_mask & WS_CONF::BLOCK) == 0 && term != ";") {
        return validation_error("is not terminated by \";\"", dire.directive, dire.line);
    }

    // 引数の数が正しくない場合
    if (!is_correct_number_of_args(dire, dire_mask)) {
        if (is_must_be_on_off(dire, dire_mask)) {
            return validation_error("it must be \"on\" or \"off\"", dire.directive, dire.line);

        } else {
            return validation_error("invalid number of arguments", dire.directive, dire.line);
        }
    }
    return "";
}
