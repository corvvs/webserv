#ifndef WEBSERV_TOKENLIST_H
#define WEBSERV_TOKENLIST_H

#include "Token.h"

class TokenList {
    std::vector<Token> token_list;
    size_t index;
    string_class const *base;

public:
    TokenList(string_class const *base, const std::vector<Token> &token_list)
        : index(), base(base), token_list(token_list) {}
    ~TokenList() {
        delete base;
    }
    const Token get() {
        return token_list.at(index++);
    }
    bool is_end() {
        return token_list.at(index).get_kind() == Token::END;
    }
    size_t count_line(const Token &token) {
        size_t count = 0;
        for (int i = 0; i < token.get_first(); ++i) {
            if ((*base)[i] == '\n') {
                ++count;
            }
        }
        return count;
    }
};

#endif // WEBSERV_TOKENLIST_H
