#ifndef WEBSERV_LEXER_H
#define WEBSERV_LEXER_H

#include "Token.h"
#include "TokenList.h"
#include "vector"

class Lexer {
    std::vector<Token> token_list;
    string_class const *base;

    static const CharFilter space;
    static const CharFilter left_brace;
    static const CharFilter right_brace;
    static const CharFilter semi_colon;
    static const CharFilter single_quote;
    static const CharFilter double_quote;
    static const CharFilter delimiter;

public:
    Lexer(const string_class &s) : base(new string_class(s)) {}
    const TokenList tokenize() {
        light_string rest(*base);
        while (rest.size() != 0) {
            if (start_with_space(rest)) {
                rest = skip_space(rest);
                continue;
            }
            if (start_with_comment(rest)) {
                rest = skip_comment(rest);
                continue;
            }
            if (start_with_single_quote(rest)) {
                rest = tokenize_string_single(rest);
                continue;
            }
            if (start_with_double_quote(rest)) {
                rest = tokenize_string_double(rest);
                continue;
            }
            if (start_with_left_brace(rest)) {
                rest = tokenize_left_brace(rest);
                continue;
            }
            if (start_with_right_brace(rest)) {
                rest = tokenize_right_brace(rest);
                continue;
            }
            if (start_semi_colon(rest)) {
                rest = tokenize_semi_colon(rest);
                continue;
            }
            rest = tokenize_normal(rest);
        }
        token_list.push_back(Token(rest.substr(), Token::END));
        return TokenList(base, token_list);
    }
    light_string &tokenize_normal(light_string &rest) {
        token_list.push_back(Token(rest.substr_before(delimiter), Token::NORMAL));
        rest = rest.substr(rest.find_first_of(delimiter));
        return rest;
    }
    light_string tokenize_string_double(const light_string &rest) {
        token_list.push_back(Token(rest.substr_before(double_quote, 1), Token::STRING));
        return rest.substr(rest.find_first_of(double_quote, 1) + 1);
    }
    light_string tokenize_string_single(const light_string &rest) {
        token_list.push_back(Token(rest.substr_before(single_quote, 1), Token::STRING));
        return rest.substr(rest.find_first_of(single_quote, 1) + 1);
    }
    light_string tokenize_left_brace(const light_string &rest) {
        token_list.push_back(Token(rest.substr(0, 1), Token::LEFT_BRACE));
        return rest.substr(1);
    }
    light_string tokenize_right_brace(const light_string &rest) {
        token_list.push_back(Token(rest.substr(0, 1), Token::RIGHT_BRACE));
        return rest.substr(1);
    }
    light_string tokenize_semi_colon(const light_string &rest) {
        token_list.push_back(Token(rest.substr(0, 1), Token::SEMI_COLON));
        return rest.substr(1);
    }
    bool start_with_single_quote(const light_string &rest) const {
        return rest.find_first_of(single_quote) == 0;
    }
    bool start_with_double_quote(const light_string &rest) const {
        return rest.find_first_of(double_quote) == 0;
    }
    bool start_with_left_brace(const light_string &rest) const {
        return rest.find_first_of(left_brace) == 0;
    }
    bool start_with_right_brace(const light_string &rest) const {
        return rest.find_first_of(right_brace) == 0;
    }
    light_string skip_comment(const light_string &rest) const {
        return rest.substr(rest.find_first_of("\n") + 1);
    }
    bool start_with_comment(const light_string &rest) const {
        return rest.find_first_of("#") == 0;
    }
    bool start_semi_colon(const light_string &rest) const {
        return rest.find_first_of(semi_colon) == 0;
    }
    light_string skip_space(const light_string &rest) const {
        return rest.substr(rest.find_first_of(space) + 1);
    }
    bool start_with_space(const light_string &rest) const {
        return rest.find_first_of(space) == 0;
    }
};

#endif // WEBSERV_LEXER_H
