#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <exception>
#include "test_common.hpp"

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

std::ostream &operator<<(std::ostream &os, const ngxToken &nt)
{
    os << "line: " << nt.line << " "
       << std::boolalpha << "quote: " << nt.isQuoted << " "
       << "value: " << nt.value
       << nt.error;
    return os;
}

std::ostream &operator<<(std::ostream &os, const charLine &cl)
{
    os << cl.line << ": " << cl.chars;
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

ngxToken create_ngx_token(
    std::string value,
    int line,
    bool isQuoted,
    std::string error = "")
{
    ngxToken new_token;
    new_token.value = value;
    new_token.line = line;
    new_token.isQuoted = isQuoted;
    new_token.error = error;
    return new_token;
}

strLine create_str_line(std::string line, int lineno)
{
    strLine sl;

    sl.str = line;
    sl.line = lineno;
    return sl;
}

void error_exit(int line, const std::string &msg)
{
    std::cout << "webserv: [emerg] " << msg << " :" << line << std::endl;
    exit(1);
}

std::vector<strLine> file_read(std::string filename)
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
        v.push_back(create_str_line(line, lineno));
        lineno += 1;
    }
    return v;
}

std::string trim_space(std::string s)
{
    const std::string spaces = " \f\n\r\t\v";
    std::string res;

    std::string::size_type left = s.find_first_not_of(spaces);
    if (left != std::string::npos)
    {
        std::string::size_type right = s.find_last_not_of(spaces);
        res = s.substr(left, right - left + 1);
    }
    return res;
}

// bool is_space(std::string s)
// {
//     return trim_space(s).size() == 0;
// }

bool is_space(char c)
{
    const std::string spaces = " \f\n\r\t\v";

    return spaces.find(c) != std::string::npos;
}

std::vector<ngxToken> tokenize(std::string filename)
{
    std::vector<strLine> lines(file_read(filename));

    std::vector<ngxToken> tokens;

    bool ok;
    (void)ok;
    std::string token;
    int token_line;

    DSOUT() << "===config data===" << std::endl;
    print(lines);
    // 一文字ずつ読んでいく
    for (std::vector<strLine>::iterator it = lines.begin(); it != lines.end(); it++)
    {
        int line = it->line;
        std::string::iterator cur = it->str.begin();
        while (cur != it->str.end())
        {
            //            std::cout << "line: " << line << "| char: " << *cur << std::endl;
            // 空文字が来たらこれまでのtokenをtokenの配列に追加する
            if (is_space(*cur))
            {
                if (!token.empty())
                {
                    tokens.push_back(create_ngx_token(token, token_line, false));
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
            // コメントだった場合
            if (token.empty() && *cur == '#')
            {
                int line_at_start = line;
                while (*cur != '\n')
                {
                    //                    debug(*cur);
                    token += *cur;
                    cur++;
                    if (cur == it->str.end())
                    {
                        break;
                    }
                }
                // コメントを追加する
                tokens.push_back(create_ngx_token(token, line_at_start, false));
                token = "";

                // また頭からまた見ていく
                continue;
            }

            if (token.empty())
            {
                token_line = line;
            }

            // 引用符
            if (*cur == '"' || *cur == '\'')
            {
                if (!token.empty())
                {
                    // 引用符がトークンの中にある場合は他と同様に扱う
                    token_line = *cur;
                    continue;
                }

                char quote = *cur;
                cur++;

                while (*cur != quote)
                {
                    if (*cur == '\\' + quote)
                    {
                        token += quote;
                    }
                    else
                    {
                        token += *cur;
                    }
                    cur++;
                    if (cur == it->str.end())
                    {
                        break;
                    }
                }

                // quoteに囲まれているのでtrue
                tokens.push_back(create_ngx_token(token, token_line, true));
                token = "";
                continue;
            }

            // 完全なトークンのように扱われる特殊文字を処理する
            if (*cur == '{' || *cur == '}' || *cur == ';')
            {
                if (!token.empty())
                {
                    tokens.push_back(create_ngx_token(token, token_line, false));
                    token = "";
                }

                // 単体でトークンとみなされる
                std::string c = "";
                c += *cur;
                tokens.push_back(create_ngx_token(c, token_line, false));
                cur++;
                continue;
            }
            token += *cur;
            cur++;
        }

        if (!token.empty())
        {
            tokens.push_back(create_ngx_token(token, token_line, false));
        }
    }
    DSOUT() << "===tokenize===" << std::endl;
    print(tokens);
    return tokens;
}

ngxToken lex()
{
    //    return balance_braces(tokenize(reader));
    return ngxToken();
}

bool balance_braces(std::vector<ngxToken> tokens)
{
    int depth = 0;
    int line = 0;
    for (std::vector<ngxToken>::iterator it = tokens.begin(); it != tokens.end(); it++)
    {
        line = it->line;
        if (it->value == "}" && !it->isQuoted)
        {
            depth -= 1;
        }
        else if (it->value == "{" && !it->isQuoted)
        {
            depth += 1;
        }

        if (depth < 0)
        {
            const std::string msg = "unexpected \"}\"";
            error_exit(line, msg);
            //            throw std::runtime_error(msg);
        }
    }

    if (depth > 0)
    {
        const std::string msg = "unexpected end of file, expecting \"}\"";
        error_exit(line, msg);
        //        throw std::runtime_error(msg);
    }
    return true;
}

int main()
{
    std::string filename = "./conf/01_default.conf";
    //    std::string filename = "./conf/02_unexpected_brace.conf";
    //    std::string filename = "./conf/03_unexpected_eof.conf";

    std::vector<ngxToken> t = tokenize(filename);
    if (balance_braces(t))
    {
        std::cout << "webserv: the configuration file " << filename << " syntax is ok" << std::endl;
    }
}
