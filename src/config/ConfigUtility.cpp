#include "ConfigUtility.hpp"
#include <iostream>

namespace config {

std::vector<std::string> split_str(const std::string &s, const std::string &sep) {
    size_t len = sep.length();
    std::vector<std::string> vec;
    if (len == 0) {
        vec.push_back(s);
        return vec;
    }

    size_t offset = 0;
    while (1) {
        size_t pos = s.find(sep, offset);
        if (pos == std::string::npos) {
            vec.push_back(s.substr(offset));
            break;
        }
        vec.push_back(s.substr(offset, pos - offset));
        offset = pos + len;
    }
    return vec;
}

std::string str_tolower(const std::string &s) {
    std::string low;
    for (std::string::const_iterator it = s.begin(); it != s.end(); ++it) {
        low += std::tolower(*it);
    }
    return low;
}

void indent(const size_t &size) {
    for (size_t i = 0; i < size; i++) {
        std::cout << " ";
    }
}

} // namespace config
