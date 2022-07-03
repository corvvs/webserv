#ifndef LEXER_HPP
#define LEXER_HPP
#include <string>
#include <vector>

namespace config {
struct wsToken {
    std::string value;
    size_t line;
    bool is_quoted;
};

class Lexer {
public:
    Lexer(void);
    ~Lexer(void);

    wsToken *read(void);
    void reset_read_idx();
    void tokenize(std::string data);

private:
    /// member variables
    std::vector<wsToken> tokens_;
    size_t idx_;
    size_t line_count_;

    /// member functions

    bool is_quote(char c) const;
    bool is_space(char c) const;
    bool is_special(char c) const;

    std::string skip_line(std::string &s) const;
    std::string skip_space(std::string &s) const;

    std::string tokenize_string(std::string &s, char end);
    std::string tokenize_bare_string(std::string &s);
    std::string tokenize_special(std::string &s);

    void line_count_up(const std::string &s, const size_t &len);
    void tokenize_error_exception(const std::string &s);
};

std::ostream &operator<<(std::ostream &os, const wsToken &token);

} // namespace config

#endif
