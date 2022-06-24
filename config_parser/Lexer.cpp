#include "Lexer.hpp"
#include "test_common.hpp"
#include <exception>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

/// public functions

Lexer::Lexer(void) : idx_(0) {}
Lexer::~Lexer(void) {}

// パーサーが呼び出す関数
// トークンを1つ渡す
wsToken *Lexer::read(void) {
    if (tokens_.size() <= (size_t)idx_) {
        return NULL;
    } else {
        idx_ += 1;
        return &tokens_[idx_ - 1];
    }
}

void Lexer::lex(const std::string &filename) {
    std::vector<strLine> lines(file_read(filename));
    tokenize(lines);
    balance_braces();
    //        std::cout << "webserv: the configuration file " << filename << " syntax is ok" << std::endl;
}

/// private functions

void Lexer::error_exit(int line, const std::string &msg) const {
    std::cout << "webserv: [emerg] " << msg << " :" << line << std::endl;
    exit(1);
}

// TODO: ファイルがディレクトリだった場合に弾くようにする
std::vector<strLine> Lexer::file_read(std::string filename) const {
    std::vector<strLine> v;

    std::ifstream input_file(filename);
    if (!input_file.is_open()) {
        throw std::runtime_error("file not opened");
    }

    std::string line;
    int lineno = 1;
    while (std::getline(input_file, line)) {
        v.push_back(strLine(line, lineno));
        lineno += 1;
    }
    return v;
}

void Lexer::add(tokenMgr &tmgr) {
    tokens_.push_back(wsToken(tmgr.token, tmgr.line, tmgr.is_quoted));
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
    tmgr.is_quoted = true;
    add(tmgr);
    tmgr.is_quoted = false;
    return p;
}

// 特殊文字は単体で区切り文字として扱われるのでこれまでのトークンとは別でpushする
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
 * 1. strLineループ(vector<string>)
 * 2. 1文字ずつ読む(stringを分解する)
 * 3. スペースのチェック
 * 4. コメントのチェック
 * 5. 引用符のチェック
 * 6. 特殊文字のチェック
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

    for (std::vector<wsToken>::const_iterator it = tokens_.begin(); it != tokens_.end(); it++) {
        line = it->line;
        if (it->value == "}" && !it->is_quoted) {
            depth -= 1;
        } else if (it->value == "{" && !it->is_quoted) {
            depth += 1;
        }

        if (depth < 0) {
            const std::string msg = "unexpected \"}\"";
            error_exit(line, msg);
        }
    }

    if (depth > 0) {
        const std::string msg = "unexpected end of file, expecting \"}\"";
        error_exit(line, msg);
    }
    return true;
}

std::ostream &operator<<(std::ostream &os, const wsToken &token) {
    std::string line = std::to_string(token.line);
    if (token.line < 10) {
        line = "0" + line;
    }

    os << "[L] : " << line << " "
       << "[Quote]: " << token.is_quoted << " "
       << "[Val]: " << token.value;
    return os;
}

std::ostream &operator<<(std::ostream &os, const strLine &sl) {
    os << sl.line << ": " << sl.str;
    return os;
}
