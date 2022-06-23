#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <exception>
#include "test_common.hpp"
#include "Lexer.hpp"

/*************************************************************/
// debug
std::ostream &operator<<(std::ostream &os, const wsToken &nt)
{
    os << "line: " << nt.line << " "
       << std::boolalpha << "quote: " << nt.is_quoted << " "
       << "value: " << nt.value;
    return os;
}

std::ostream &operator<<(std::ostream &os, const strLine &cl)
{
    os << cl.line << ": " << cl.str;
    return os;
}

template <class T>
void print(T t)
{
    for (typename T::iterator it = t.begin(); it != t.end(); it++)
    {
        std::cout << *it << std::endl;
    }
}
/*************************************************************/

Lexer::Lexer(void)
{
}

Lexer::~Lexer(void)
{
}

std::vector<wsToken> Lexer::lex(const std::string &filename)
{
    std::vector<strLine> lines(file_read(filename));
    std::vector<wsToken> t = tokenize(lines);
    if (balance_braces(t))
    {
        std::cout << "webserv: the configuration file " << filename << " syntax is ok" << std::endl;
    }
    //    DSOUT() << "===tokenize===" << std::endl;
    //    print(t);

    return t;
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

    //    DSOUT() << "===config data===" << std::endl;
    //    print(lines);
    for (std::vector<strLine>::iterator it = lines.begin(); it != lines.end(); it++)
    {
        int line = it->line;
        int token_line;
        std::string token;
        std::string::iterator cur = it->str.begin();
        while (cur != it->str.end())
        {
            //            debug(*cur);
            //            std::cout << "line: " << line << "| char: " << *cur << std::endl;
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
                int line_at_start = line;
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

            if (token.empty())
            {
                token_line = line;
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
