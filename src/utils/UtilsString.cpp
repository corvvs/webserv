#include "UtilsString.hpp"

HTTP::byte_string HTTP::Utils::downcase(const byte_string &str) {
    HTTP::byte_string rv(str.size(), 0);
    for (byte_string::size_type i = 0; i < str.size(); ++i) {
        rv[i] = tolower(str[i]);
    }
    return rv;
}

HTTP::byte_string &HTTP::Utils::downcase(byte_string &str) {
    for (byte_string::size_type i = 0; i < str.size(); ++i) {
        str[i] = tolower(str[i]);
    }
    return str;
}

HTTP::byte_string HTTP::Utils::upcase(const byte_string &str) {
    HTTP::byte_string rv(str.size(), 0);
    for (byte_string::size_type i = 0; i < str.size(); ++i) {
        rv[i] = toupper(str[i]);
    }
    return rv;
}

void HTTP::Utils::upcase(byte_string &str) {
    for (byte_string::size_type i = 0; i < str.size(); ++i) {
        str[i] = toupper(str[i]);
    }
}

void HTTP::Utils::normalize_cgi_metavar_key(byte_string &key) {
    for (byte_string::size_type i = 0; i < key.size(); ++i) {
        if (key[i] == '-') {
            key[i] = '_';
        } else {
            key[i] = toupper(key[i]);
        }
    }
}

HTTP::byte_string HTTP::Utils::join_path(const light_string &directory_path, const light_string &basename) {
    const light_string chopped_directory_path
        = (directory_path.size() > 0 && directory_path[directory_path.size() - 1] == '/')
              ? directory_path.substr(0, directory_path.size() - 1)
              : directory_path;
    const light_string chopped_basename = (basename.size() > 0 && basename[0] == '/') ? basename.substr(1) : basename;
    return chopped_directory_path.str() + "/" + chopped_basename.str();
}
