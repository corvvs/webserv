#ifndef LEXER_HPP
#define LEXER_HPP
#include <string>
#include <vector>

class Lexer {
public:
    struct wsToken {
        std::string value;
        size_t line;
        bool is_quoted;
    };

    Lexer(void);
    ~Lexer(void);

    void lex(const std::string &filename);
    wsToken *read(void);
    void reset_read_idx();

private:
    /// member variables
    std::vector<wsToken> tokens_;
    size_t idx_;
    size_t line_count_;

    /// member functions
    void check_file_exception_ifneed(const std::string &path) const;
    std::string read_file(const std::string &path) const;

    void tokenize(std::string data);
    bool is_quote(char c) const;
    bool is_space(char c) const;
    bool is_special(char c) const;

    std::string skip_line(std::string &s) const;
    std::string skip_space(std::string &s) const;

    std::string tokenize_string(std::string &s, char end);
    std::string tokenize_bare_string(std::string &s);
    std::string tokenize_special(std::string &s);

    void line_count_up(const std::string &s, const size_t &len);
    void bad_token_exception(const std::string &s);
};

std::ostream &operator<<(std::ostream &os, const Lexer::wsToken &token);

#endif
