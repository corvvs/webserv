#include "Lexer.hpp"
#include <exception>
#include <fstream>
#include <iostream>
#include <string>
#include <sys/stat.h>
#include <vector>

/// public functions

Lexer::Lexer(void) : idx_(0), line_count_(1) {}
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

void Lexer::reset_read_idx(void) {
    idx_ = 0;
}

void Lexer::lex(const std::string &filename) {
    std::string filedata(read_file(filename));
    tokenize(filedata);
}

/// private functions

// TODO: パーサー追加時に例外クラスを変更する
// ファイルの形式が不正な場合は例外を投げる
void Lexer::check_file_exception_ifneed(const std::string &path) const {
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

std::string Lexer::read_file(const std::string &path) const {
    check_file_exception_ifneed(path);

    std::ifstream input_file(path);
    if (input_file.fail()) {
        throw std::runtime_error("webserv: [crit] \"" + path + "\" failed (Not opened)");
    }

    std::string data((std::istreambuf_iterator<char>(input_file)), std::istreambuf_iterator<char>());
    return data;
}

bool Lexer::is_space(char c) const {
    const std::string spaces = " \t";

    return spaces.find(c) != std::string::npos;
}

bool Lexer::is_special(char c) const {
    return c == '{' || c == '}' || c == ';';
}

bool Lexer::is_quote(char c) const {
    return c == '\'' || c == '\"';
}

void Lexer::line_count_up(const std::string &s, const size_t &len) {
    for (size_t i = 0; i < len; i++) {
        if (s[i] == '\n') {
            line_count_ += 1;
        }
    }
}

// 特殊文字は単体で区切り文字として扱う
std::string Lexer::tokenize_special(std::string &s) {
    wsToken tok = {s.substr(0, 1), line_count_, false};
    tokens_.push_back(tok);
    s = s.substr(1);
    return s;
}

std::string Lexer::skip_line(std::string &s) const {
    size_t pos = s.find('\n');

    if (pos == std::string::npos) {
        s = s.substr(s.size());
    } else {
        s = s.substr(pos + 1);
    }
    return s;
}

std::string Lexer::skip_space(std::string &s) const {
    size_t pos = s.find_first_not_of(" \t", 1);

    if (pos == std::string::npos) {
        s = s.substr(s.size());
    } else {
        s = s.substr(pos);
    }
    return s;
}

void Lexer::bad_token_exception(const std::string &s) {
    line_count_up(s, s.size());
    throw std::runtime_error("webserv: [emerg] unexpected end of file, expecting \";\" or \"}\" line:"
                             + std::to_string(line_count_));
}

// クォートなどの前後に挟まれた文字列をトークンの配列に追加する
std::string Lexer::tokenize_string(std::string &s, char end) {
    std::string::size_type pos = s.find(end, 1);

    if (pos == std::string::npos) {
        bad_token_exception(s);
    }

    Lexer::wsToken tok = {s.substr(1, pos - 1), line_count_, is_quote(end)};
    tokens_.push_back(tok);
    s = s.substr(pos + 1);
    return s;
}

// 意味を持たない文字列をトークンの配列に追加する
std::string Lexer::tokenize_bare_string(std::string &s) {
    size_t pos = s.find_first_of(" \t\n{};");

    if (pos == std::string::npos) {
        bad_token_exception(s);
    }

    Lexer::wsToken tok = {s.substr(0, pos), line_count_, false};
    tokens_.push_back(tok);
    s = s.substr(pos);
    return s;
}

void Lexer::tokenize(std::string data) {
    while (!data.empty()) {
        if (data[0] == '\n') {
            data = data.substr(1);
            line_count_ += 1;
            continue;
        }

        // Whitespace
        if (is_space(data[0])) {
            data = skip_space(data);
            continue;
        }

        // Line comment
        if (data[0] == '#') {
            data = skip_line(data);
            continue;
        }

        // Literal
        if (is_quote(data[0])) {
            data = tokenize_string(data, data[0]);
            continue;
        }

        // Special characters
        if (is_special(data[0])) {
            data = tokenize_special(data);
            continue;
        }

        // Normal characters
        data = tokenize_bare_string(data);
    }
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

/*
int main(int argc, char **argv) {
    if (argc < 2) {
        return 0;
    }
    const char *filename = argv[1];

    Lexer lexer;
    try {
        lexer.lex(filename);
    } catch (const std::runtime_error &e) {
        std::cout << e.what() << std::endl;
        return;
    }
    return 0;
}
*/
