#include "UtilsString.hpp"

HTTP::byte_string HTTP::Utils::downcase(const byte_string &str) {
    HTTP::byte_string rv(str.size(), 0);
    for (byte_string::size_type i = 0; i < str.size(); ++i) {
        rv[i] = tolower(str[i]);
    }
    return rv;
}

void HTTP::Utils::downcase(byte_string &str) {
    for (byte_string::size_type i = 0; i < str.size(); ++i) {
        str[i] = tolower(str[i]);
    }
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
    HTTP::byte_string rv = rstrip_path(directory_path).str();
    rv += HTTP::strfy("/");
    rv += lstrip_path(basename).str();
    return rv;
}

HTTP::light_string HTTP::Utils::rstrip_path(const light_string &path) {
    light_string::size_type cd_tail = path.find_last_not_of("/");
    const light_string stripped     = path.substr(0, cd_tail == light_string::npos ? 0 : cd_tail + 1);
    return stripped;
}

HTTP::light_string HTTP::Utils::lstrip_path(const light_string &path) {
    const light_string stripped = path.substr_after("/");
    return stripped;
}

bool HTTP::Utils::is_relative_path(const light_string &path) {
    // 先頭が'/'なら絶対, そうでなければ相対
    return !path.starts_with("/");
}

HTTP::light_string HTTP::Utils::basename(const light_string &path) {
    // path末尾の /+ は取り除いて考える
    light_string::size_type i_last_not_slash = path.find_last_not_of("/");
    const bool should_rstrip                 = i_last_not_slash != light_string::npos;
    const light_string p = should_rstrip ? path.substr(0, i_last_not_slash + 1) : path.substr(path.size());
    light_string::size_type i_last_slash = p.find_last_of("/");
    if (i_last_slash == light_string::npos) {
        return p;
    } else {
        return p.substr(i_last_slash + 1);
    }
}
