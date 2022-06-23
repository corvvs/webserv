#ifndef LEXER_HPP
#define LEXER_HPP
#include <string>
#include <vector>

// TODO: structのコンストラクタもcppに書く必要あるか確認する
struct wsToken
{
    std::string value;
    int line;
    bool is_quoted;

    wsToken(std::string v, int l, bool is_q)
        : value(v), line(l), is_quoted(is_q)
    {
    }
};

// TODO: privateにする(debug用で外に出してる)
struct strLine
{
    std::string str;
    int line;

    strLine(std::string s, int l)
        : str(s), line(l)
    {
    }
};

std::ostream &operator<<(std::ostream &os, const wsToken &token);
std::ostream &operator<<(std::ostream &os, const strLine &sl);

class Lexer
{
public:
    Lexer(void);
    ~Lexer(void);

    void lex(const std::string &filename);

    wsToken *read(void);

private:
    // member variables
    std::vector<wsToken> tokens_;
    int idx_;

    std::vector<strLine> file_read(std::string filename);
    bool balance_braces(std::vector<wsToken> tokens);
    std::vector<wsToken> tokenize(std::vector<strLine> lines);

    void error_exit(int line, const std::string &msg);

    bool is_space(char c);
    bool is_comment(char c);
    bool is_special(char c);
    bool is_quote(char c);
};

#endif
