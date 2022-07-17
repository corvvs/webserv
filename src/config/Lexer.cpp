#include "Lexer.hpp"
#include "Parser.hpp"
#include "Validator.hpp"
#include <iostream>
#include <string>
#include <vector>

namespace config {

/// public functions

Lexer::Lexer(void) : idx_(0), line_count_(1) {}
Lexer::~Lexer(void) {}

// トークンを1つ返す(パーサーで使用する)
wsToken *Lexer::read(void) {
    if (tokens_.size() <= idx_) {
        return NULL;
    } else {
        idx_ += 1;
        return &tokens_[idx_ - 1];
    }
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
            data = skip_spaces(data);
            continue;
        }

        // Line comment
        if (data[0] == '#') {
            data = skip_line(data);
            line_count_ += 1;
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

void Lexer::reset_read_idx(void) {
    idx_ = 0;
}

/// private functions

bool Lexer::is_space(const char &c) const {
    const std::string spaces = " \t";

    return spaces.find(c) != std::string::npos;
}

bool Lexer::is_special(const char &c) const {
    return c == '{' || c == '}' || c == ';';
}

bool Lexer::is_quote(const char &c) const {
    return c == '\'' || c == '\"';
}

size_t Lexer::count_lines(const std::string &s, const size_t &len) const {
    size_t lines = 0;
    for (size_t i = 0; i < len; i++) {
        if (s[i] == '\n') {
            lines += 1;
        }
    }
    return lines;
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

std::string Lexer::skip_spaces(std::string &s) const {
    size_t pos = s.find_first_not_of(" \t", 1);
    if (pos == std::string::npos) {
        s = s.substr(s.size());
    } else {
        s = s.substr(pos);
    }
    return s;
}

std::string Lexer::tokenize_error(const std::string &s) const {
    size_t lines = count_lines(s, s.size());
    return validation_error("unexpected end of file, expecting \";\" or \"}\"", lines);
}

// クォートなどの前後に挟まれた文字列をトークンの配列に追加する
std::string Lexer::tokenize_string(std::string &s, const char &end) {
    std::string::size_type pos = s.find(end, 1);
    if (pos == std::string::npos) {
        throw SyntaxError(tokenize_error(s));
    }

    wsToken tok = {s.substr(1, pos - 1), line_count_, is_quote(end)};
    tokens_.push_back(tok);
    s = s.substr(pos + 1);
    return s;
}

// 意味を持たない文字列をトークンの配列に追加する
std::string Lexer::tokenize_bare_string(std::string &s) {
    size_t pos = s.find_first_of(" \t\n{};");
    if (pos == std::string::npos) {
        throw SyntaxError(tokenize_error(s));
    }

    wsToken tok = {s.substr(0, pos), line_count_, false};
    tokens_.push_back(tok);
    s = s.substr(pos);
    return s;
}

void Lexer::print_tokens(void) const {
    for (std::vector<wsToken>::const_iterator it = tokens_.begin(); it != tokens_.end(); ++it) {
        std::cout << "Line: ";
        if (it->line < 10) {
            std::cout << "0";
        }
        std::cout << it->line << " ";
        std::cout << "Quote: " << it->is_quoted << " ";
        std::cout << "Value: " << it->value << "\n";
    }
}

} // namespace config
