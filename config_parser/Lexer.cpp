#include "Lexer.hpp"
#include "test_common.hpp"
#include <exception>
#include <fstream>
#include <iostream>
#include <string>
#include <sys/stat.h>
#include <vector>

/// public functions

Lexer::Lexer(void) : idx_(0) {}
Lexer::~Lexer(void) {}

/*＊
 * パーサーが使う用の関数
 * トークンを1つ返す
 */
Lexer::wsToken *Lexer::read(void) {
    if (tokens_.size() <= idx_) {
        return NULL;
    } else {
        idx_ += 1;
        return &tokens_[idx_ - 1];
    }
}

void Lexer::lex(const std::string &filename) {
    std::vector<strLine> lines(read_file(filename));
    tokenize(lines);
    balance_braces();
}

/// private functions

void Lexer::check_file(const std::string &path) const {
    struct stat st;

    if (stat(path.c_str(), &st) != 0) {
        throw std::runtime_error("webserv: [emerg] open() \"" + path + "\" failed (2: No such file or directory)");
    }

    switch (st.st_mode & S_IFMT) {
        case S_IFDIR:
            throw std::runtime_error("webserv: [crit] stat() \"" + path + "\" failed (21: Is a directory)");
        case S_IFREG:
            if ((st.st_mode & S_IRUSR) == 0) {
                throw std::runtime_error("webserv: [emerg] stat() \"" + path + "\"" + "failed (13: Permission denied)");
            }
            break;
        default:
            throw std::runtime_error("webserv: [crit] stat() \"" + path + "\" failed (Invalid file type)");
    }
}

std::vector<Lexer::strLine> Lexer::read_file(const std::string &path) const {
    check_file(path); // Throw an exception if it is incorrect

    std::ifstream input_file(path);
    if (!input_file.is_open()) {
        throw std::runtime_error("webserv: [crit] \"" + path + "\" failed (Not opened)");
    }

    std::string line;
    std::vector<strLine> v;
    int lineno = 1;
    while (std::getline(input_file, line)) {
        v.push_back(strLine(line, lineno));
        lineno += 1;
    }
    return v;
}

void Lexer::add(tokenMgr &tmgr) {
    tokens_.push_back(Lexer::wsToken(tmgr.token, tmgr.line, tmgr.is_quoted));
    tmgr.token = "";
}

bool Lexer::is_space(char c) const {
    const std::string spaces = " \f\n\r\t\v";

    return spaces.find(c) != std::string::npos;
}

bool Lexer::is_comment(char c) const {
    return c == '#';
}

bool Lexer::is_quote(char c) const {
    return c == '"' || c == '\'';
}

bool Lexer::is_special(char c) const {
    return c == '{' || c == '}' || c == ';';
}

const char *Lexer::quote(const char *p, tokenMgr &tmgr) {
    const char quote_type = *p;

    p++;
    while (*p && *p != quote_type) {
        tmgr.token += *p;
        p++;
    }
    if (*p == quote_type) {
        p++;
    }
    tmgr.is_quoted = true;
    add(tmgr);
    tmgr.is_quoted = false;
    return p;
}

// 特殊文字は単体で区切り文字として扱う
const char *Lexer::special_char(const char *p, tokenMgr &tmgr) {
    if (!tmgr.token.empty()) {
        add(tmgr);
    }
    tmgr.token += *p;
    add(tmgr);
    p++;
    return p;
}

/**
 * strLineループ(vector<string>)
 * 1文字ずつ読む(stringを分解する)
 * 1. スペースのチェック
 * 2. コメントのチェック
 * 3. 引用符のチェック
 * 4. 特殊文字のチェック
 */
void Lexer::tokenize(std::vector<strLine> lines) {
    tokenMgr tmgr;

    for (std::vector<strLine>::iterator it = lines.begin(); it != lines.end(); it++) {
        tmgr.line     = it->line;
        const char *p = it->str.c_str();

        while (*p) {
            // Whitespace
            if (is_space(*p)) {
                p++;
                if (!tmgr.token.empty()) {
                    add(tmgr);
                }
                continue;
            }

            // Line comment
            if (is_comment(*p) && tmgr.token.empty()) {
                while (*p && *p != '\n') {
                    p++;
                }
                continue;
            }

            // Literal
            if (is_quote(*p) && tmgr.token.empty()) {
                p = quote(p, tmgr);
                continue;
            }

            // Special characters
            if (is_special(*p)) {
                p = special_char(p, tmgr);
                continue;
            }

            tmgr.token += *p;
            p++;
        }

        if (!tmgr.token.empty()) {
            add(tmgr);
        }
    }
}

bool Lexer::balance_braces(void) const {
    int depth = 0;
    int line  = 0;

    for (std::vector<Lexer::wsToken>::const_iterator it = tokens_.begin(); it != tokens_.end(); it++) {
        line = it->line;
        if (it->value == "}" && !it->is_quoted) {
            depth -= 1;
        } else if (it->value == "{" && !it->is_quoted) {
            depth += 1;
        }
        if (depth < 0) {
            throw std::runtime_error("webserv: [emerg] unexpected \"}\" :" + std::to_string(line));
        }
    }
    if (depth > 0) {
        throw std::runtime_error("webserv: [emerg] unexpected end of file, expecting \"}\" :" + std::to_string(line));
    }
    return true;
}

std::ostream &operator<<(std::ostream &os, const Lexer::wsToken &token) {
    std::string line = std::to_string(token.line);
    if (token.line < 10) {
        line = "0" + line;
    }

    os << "[L] : " << line << " "
       << "[Quote]: " << token.is_quoted << " "
       << "[Val]: " << token.value;
    return os;
}
