#ifndef CONFIGUTILITY_HPP
#define CONFIGUTILITY_HPP

#include "Config.hpp"
#include "Parser.hpp"
#include <iomanip>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

namespace config {

const static size_t INDENT_SIZE = 24;

std::vector<std::string> split_str(const std::string &s, const std::string &sep);
std::string str_tolower(const std::string &s);
void indent(const size_t &size);

template <class First, class Second>
std::string pair_to_string(const std::pair<First, Second> &p) {
    std::ostringstream oss;
    oss << "< ";
    oss << p.first << ", " << p.second << " >";
    return oss.str();
}

template <class T>
std::string vector_to_string(const std::vector<T> &v) {
    std::ostringstream oss;
    oss << "{ ";
    for (typename std::vector<T>::const_iterator it = v.begin(); it != v.end(); ++it) {
        if (it != v.begin()) {
            oss << ", ";
        }
        oss << *it;
    }
    oss << " }";
    return oss.str();
}

template <class First, class Second>
std::string vector_pair_to_string(const std::vector<std::pair<First, Second> > &vp) {
    std::ostringstream oss;
    oss << "{ ";
    for (typename std::vector<std::pair<First, Second> >::const_iterator it = vp.begin(); it != vp.end(); ++it) {
        if (it != vp.begin()) {
            oss << ", ";
        }
        oss << pair_to_string(*it);
    }
    oss << " }";
    return oss.str();
}

template <class Key, class Value>
std::string map_to_string(const std::map<Key, Value> &mp) {
    std::ostringstream oss;
    oss << "{ ";
    for (typename std::map<Key, Value>::const_iterator it = mp.begin(); it != mp.end(); ++it) {
        oss << "< " << it->first << ", " << it->second << " >";
    }
    oss << " }";
    return oss.str();
}

template <class T>
std::string set_to_string(const std::set<T> &st) {
    std::ostringstream oss;
    oss << "{ ";
    for (typename std::set<T>::const_iterator it = st.begin(); it != st.end(); ++it) {
        if (it != st.begin()) {
            oss << ", ";
        }
        oss << *it;
    }
    oss << " }";
    return oss.str();
}

template <class Key, class Value>
void print_key_value(const Key &key, const Value &value, const bool &has_indent = false) {
    int size = INDENT_SIZE;
    if (has_indent) {
        indent(2);
        size = INDENT_SIZE - 2;
    }
    std::cout << std::setw(size) << std::left << key << ": " << value << std::endl;
}

} // namespace config
#endif
