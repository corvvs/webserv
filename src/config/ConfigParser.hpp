#ifndef CONFIGPARSER_HPP
#define CONFIGPARSER_HPP

#include <map>
#include <string>
#include <vector>

class ConfigParser {
public:
    // member variables
    std::string data_;

    // member functions
    ConfigParser(const std::string &filepath = "./default.conf");
    ConfigParser(const ConfigParser &other);
    ConfigParser &operator=(const ConfigParser &other);
    ~ConfigParser(void);

    void parse(void);
    bool test(void);
private:
};

#endif
