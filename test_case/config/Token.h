#ifndef WEBSERV_TOKEN_H
#define WEBSERV_TOKEN_H

#include "Config.h"

class Token {
public:
    enum TokenKind { LEFT_BRACE, RIGHT_BRACE, SEMI_COLON, STRING, NORMAL, END };
private:
    light_string str;
    TokenKind kind;

public:
    Token(const light_string &str, TokenKind kind) : str(str), kind(kind) {}

    const light_string &get_str() const {
        return str;
    }

    TokenKind get_kind() const {
        return kind;
    }
    size_t get_first() const {
        return str.get_first();
    }
};

#endif // WEBSERV_TOKEN_H
