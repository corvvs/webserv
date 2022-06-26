#ifndef LEXER_HPP
#define LEXER_HPP
#include <string>
#include <vector>

class Lexer {
public:
    struct wsToken {
        std::string value;
        int line;
        bool is_quoted;
        wsToken(std::string v, int l, bool is_q) : value(v), line(l), is_quoted(is_q) {}
    };

    Lexer(void);
    ~Lexer(void);

    void lex(const std::string &filename);
    wsToken *read(void);

private:
    struct strLine {
        std::string str;
        int line;

        strLine(std::string s, int l) : str(s), line(l) {}
    };

    struct tokenMgr {
        std::string token;
        int line;
        bool is_quoted;
        tokenMgr(void) : token(""), line(0), is_quoted(false) {}
    };

    /// member variables
    std::vector<wsToken> tokens_;
    size_t idx_;

    /// member functions
    void check_file(const std::string &path) const;
    std::vector<strLine> read_file(const std::string &path) const;

    void tokenize(std::vector<strLine> lines);
    void add(tokenMgr &tmgr);
    bool is_quote(char c) const;
    bool is_space(char c) const;
    bool is_special(char c) const;
    bool is_comment(char c) const;

    const char *special_char(const char *p, tokenMgr &tmgr);
    const char *quote(const char *p, tokenMgr &tmgr);
};

std::ostream &operator<<(std::ostream &os, const Lexer::wsToken &token);

#endif
