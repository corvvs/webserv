#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <exception>
#include "test_common.hpp"
#include "Lexer.hpp"

Lexer::Lexer(void) : idx_(0)
{
}

Lexer::~Lexer(void)
{
}

/**
 * @brief トークンを1つ返す
 */
wsToken *Lexer::read(void)
{
    if (tokens_.size() <= (size_t)idx_)
    {
        return NULL;
    }
    else
    {
        idx_ += 1;
        return &tokens_[idx_ - 1];
    }
}

void Lexer::lex(const std::string &filename)
{
    std::vector<strLine> lines(file_read(filename));
    std::vector<wsToken> t = tokenize(lines);
    if (balance_braces(t))
    {
        //        std::cout << "webserv: the configuration file " << filename << " syntax is ok" << std::endl;
    }

    // メンバ変数に代入
    tokens_ = t;
}

/**
 * 1. strLineループ(vector<string>)
 * 2. 1文字ずつ読む(stringを分解する)
 * 3. スペースのチェック
 * 4. コメントのチェック
 * 5. 引用符のチェック
 * 6. 特殊文字のチェック
 */
std::vector<wsToken> Lexer::tokenize(std::vector<strLine> lines)
{
    std::vector<wsToken> tokens;

    for (std::vector<strLine>::iterator it = lines.begin(); it != lines.end(); it++)
    {
        std::string token;
        int token_line = it->line;
        std::string::iterator cur = it->str.begin();
        while (cur != it->str.end())
        {
            if (is_space(*cur))
            {
                // スペースが区切り文字扱いなのでこれまでのトークンをpbする
                if (!token.empty())
                {
                    tokens.push_back(wsToken(token, token_line, false));
                    token = "";
                }
                while (is_space(*cur))
                {
                    cur++;
                    if (cur == it->str.end())
                    {
                        break;
                    }
                }
                continue;
            }

            // 繋がっている場合は普通の文字として扱う
            if (is_comment(*cur) && token.empty())
            {
                int line_at_start = token_line;
                while (*cur != '\n')
                {
                    token += *cur;
                    cur++;
                    if (cur == it->str.end())
                    {
                        break;
                    }
                }
                tokens.push_back(wsToken(token, line_at_start, false));
                token = "";
                continue;
            }

            // 繋がっている場合は普通の文字として扱う
            if (is_quote(*cur) && token.empty())
            {
                char quote = *cur;
                cur++;

                while (*cur != quote)
                {
                    token += *cur;
                    cur++;
                    if (cur == it->str.end())
                    {
                        break;
                    }
                }

                // quoteに囲まれているのでtrue
                tokens.push_back(wsToken(token, token_line, true));
                token = "";
                cur++;
                continue;
            }

            if (is_special(*cur))
            {
                // 特殊文字は区切り文字として扱われるのでこれまでtokenをpbする
                if (!token.empty())
                {
                    tokens.push_back(wsToken(token, token_line, false));
                    token = "";
                }

                // 特殊文字は単体でトークンとして扱う
                std::string c(1, *cur);
                tokens.push_back(wsToken(c, token_line, false));
                cur++;
                continue;
            }
            token += *cur;
            cur++;
        }

        if (!token.empty())
        {
            tokens.push_back(wsToken(token, token_line, false));
        }
    }
    return tokens;
}

bool Lexer::balance_braces(std::vector<wsToken> tokens)
{
    int depth = 0;
    int line = 0;
    for (std::vector<wsToken>::iterator it = tokens.begin(); it != tokens.end(); it++)
    {
        line = it->line;
        if (it->value == "}" && !it->is_quoted)
        {
            depth -= 1;
        }
        else if (it->value == "{" && !it->is_quoted)
        {
            depth += 1;
        }

        if (depth < 0)
        {
            const std::string msg = "unexpected \"}\"";
            error_exit(line, msg);
        }
    }

    if (depth > 0)
    {
        const std::string msg = "unexpected end of file, expecting \"}\"";
        error_exit(line, msg);
    }
    return true;
}

void Lexer::error_exit(int line, const std::string &msg)
{
    std::cout << "webserv: [emerg] " << msg << " :" << line << std::endl;
    exit(1);
}

// TODO: ファイルがディレクトリだった場合に弾くようにする
// ファイルを読んで行数と文字列のペアを持つようにする
std::vector<strLine> Lexer::file_read(std::string filename)
{
    std::vector<strLine> v;

    std::ifstream input_file(filename);
    if (!input_file.is_open())
    {
        throw std::runtime_error("file not opened");
    }

    std::string line;
    int lineno = 1;
    while (std::getline(input_file, line))
    {
        v.push_back(strLine(line, lineno));
        lineno += 1;
    }
    return v;
}

bool Lexer::is_space(char c)
{
    const std::string spaces = " \f\n\r\t\v";

    return spaces.find(c) != std::string::npos;
}

bool Lexer::is_comment(char c)
{
    return c == '#';
}

bool Lexer::is_quote(char c)
{
    return c == '"' || c == '\'';
}

bool Lexer::is_special(char c)
{
    return c == '{' || c == '}' || c == ';';
}

/*************************************************************/
// TODO: 後で消す(debug)
std::ostream &operator<<(std::ostream &os, const wsToken &token)
{
    std::string line = std::to_string(token.line);
    if (token.line < 10)
    {
        line = "0" + line;
    }

    os << "[L] : " << line << " "
       << "[Quote]: " << token.is_quoted << " "
       << "[Val]: " << token.value;
    return os;
}

std::ostream &operator<<(std::ostream &os, const strLine &sl)
{
    os << sl.line << ": " << sl.str;
    return os;
}
/*************************************************************/
