#ifndef LEXER_HPP
#define LEXER_HPP
#include <string>
#include <vector>

// TODO: structのコンストラクタもcppに書く必要あるか確認する
struct wsToken {
    std::string value;
    int line;
    bool is_quoted;

    wsToken(std::string v, int l, bool is_q) : value(v), line(l), is_quoted(is_q) {}
};

// TODO: privateにする(debug用で外に出してる)
struct strLine {
    std::string str;
    int line;

    strLine(std::string s, int l) : str(s), line(l) {}
};

// トークナイズの処理を管理するもの
struct tokenMgr {
    std::string token;
    int line;
    bool is_quoted;
    tokenMgr() : token(""), line(0), is_quoted(false) {}
};

std::ostream &operator<<(std::ostream &os, const wsToken &token);
std::ostream &operator<<(std::ostream &os, const strLine &sl);

class Lexer {
public:
    Lexer(void);
    ~Lexer(void);

    void lex(const std::string &filename);

    wsToken *read(void);

private:
    // member variables
    std::vector<wsToken> tokens_;
    int idx_;

    std::vector<strLine> file_read(std::string filename) const;
    void tokenize(std::vector<strLine> lines);
    bool balance_braces(void) const;

    void add(tokenMgr &tmgr);
    bool is_space(char c) const;
    bool is_comment(char c) const;
    bool is_special(char c) const;
    bool is_quote(char c) const;

    const char *special_char(const char *p, tokenMgr &tmgr);
    const char *quote(const char *p, tokenMgr &tmgr);

    void error_exit(int line, const std::string &msg) const;
};

#endif
