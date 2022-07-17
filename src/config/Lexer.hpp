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
    void reset_read_idx(void);
    void tokenize(std::string data);

private:
    /// member variables
    std::vector<wsToken> tokens_;
    size_t idx_;
    size_t line_count_;

    /// member functions
    bool is_quote(const char &c) const;
    bool is_space(const char &c) const;
    bool is_special(const char &c) const;

    std::string skip_line(std::string &s) const;
    std::string skip_spaces(std::string &s) const;

    std::string tokenize_string(std::string &s, const char &end);
    std::string tokenize_bare_string(std::string &s);
    std::string tokenize_special(std::string &s);

    size_t count_lines(const std::string &s, const size_t &len) const;
    std::string tokenize_error(const std::string &s) const;

#ifdef NDEBUG
public:
#endif
    // debug functions
    void print_tokens(void) const;
};

} // namespace config

#endif
