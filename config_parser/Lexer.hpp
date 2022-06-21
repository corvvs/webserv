#ifndef LEXER_HPP
#define LEXER_HPP
#include <string>
#include <vector>

struct ngxToken
{
    std::string value;
    int line;
    bool isQuoted;
    std::string error;
};

struct charLine
{
    std::string chars;
    int line;
};

struct strLine
{
    std::string str;
    int line;
};

class Lexer
{
public:
    Lexer(void);
    ~Lexer(void);

    ngxToken create_ngx_token(
        std::string value,
        int line,
        bool isQuoted,
        std::string error);

    strLine create_str_line(std::string line, int lineno);

    void error_exit(int line, const std::string &msg);
    std::vector<strLine> file_read(std::string filename);

    std::string trim_space(std::string s);
    bool is_space(char c);
    std::vector<ngxToken> tokenize(std::string filename);
    bool balance_braces(std::vector<ngxToken> tokens);
    std::vector<ngxToken> lex(const std::string &filename);
};

#endif
