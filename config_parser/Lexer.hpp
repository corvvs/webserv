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

class Lexer
{
    // TODO: read(): 1つのトークンを返す関数を作成する
    // read()だけpublicにする
public:
    Lexer(void);
    ~Lexer(void);

    std::vector<wsToken> lex(const std::string &filename);

private:
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
