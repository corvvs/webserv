#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <exception>

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

std::ostream &operator<<(std::ostream &os, const ngxToken &nt)
{
    os << nt.line << ": "
       << std::boolalpha << nt.isQuoted << ": "
       << nt.value;
    return os;
}

std::ostream &operator<<(std::ostream &os, const charLine &cl)
{
    os << cl.line << ": " << cl.chars;
    return os;
}

template <class T>
void debug(T t)
{
    for (typename T::iterator it = t.begin(); it != t.end(); it++)
    {
        std::cout << *it << std::endl;
    }
}

std::vector<charLine> file_read(std::string filename)
{
    std::vector<charLine> v;

    std::ifstream input_file(filename);
    if (!input_file.is_open())
    {
        throw std::runtime_error("file not opened");
    }
    std::string line;
    int lineno = 0;
    while (std::getline(input_file, line))
    {
        v.push_back({line, lineno});
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

bool is_space(std::string s)
{
    return trim_space(s).size() == 0;
}

ngxToken tokenize(std::string filename)
{
    std::vector<charLine> lines(file_read(filename));

    std::vector<ngxToken> tokens;

    bool ok;
    std::string token;
    int token_line;

    debug(lines);
    for (std::vector<charLine>::iterator it = lines.begin(); it != lines.end(); it++)
    {
        // 空行が来たらこれまでのtokenをtokenの配列に追加する
        if (is_space(it->chars))
        {
            if (!token.empty())
            {
                tokens.push_back({token, token_line, false});
                token = "";
            }
            // TODO: 空行の間進める
            while (is_space(it->chars))
            {
                if (it == lines.end())
                {
                    break;
                }
                it++;
            }
        }
    }

    debug(tokens);
    return ngxToken();
}

ngxToken lex()
{
    //    return balance_braces(tokenize(reader));
    return ngxToken();
}

int main()
{
    std::string filename = "./default.conf";

    tokenize(filename);
}
