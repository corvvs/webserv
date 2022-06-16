#include "ConfigParser.hpp"

#include <fstream>
#include <iostream>

ConfigParser::ConfigParser(const std::string& filepath)
{
    std::ifstream fs(filepath);
    if (fs.fail())
    {
        throw std::runtime_error("file not opened");
    }
    std::string line;
    while (getline(fs, line))
    {
        data_ += (line + '\n');
    }
    std::cout << data_ << std::endl;
}

ConfigParser::~ConfigParser()
{
}

ConfigParser::ConfigParser(const ConfigParser& other)
{
    *this = other;
}

ConfigParser& ConfigParser::operator=(const ConfigParser& other)
{
    if (this == &other) {
        return *this;
    }
    return *this;
}

