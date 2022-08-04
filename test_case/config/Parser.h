#ifndef WEBSERV_PARSER_H
#define WEBSERV_PARSER_H

#include "string"
#include "vector"

class Directive {
    std::string name;
    std::vector<std::string> args;
    std::vector<Directive> block;

    Directive(const std::string &n,
              std::vector<std::string> a = std::vector<std::string>(),
              std::vector<Directive> b   = std::vector<Directive>())
        : name(n), args(a), block(b) {}
};

#endif // WEBSERV_PARSER_H
