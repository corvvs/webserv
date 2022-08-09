#include "File.hpp"
#include <fstream>
#include <iostream>
#include <string>
#include <sys/stat.h>

namespace file {

ErrorType check(const std::string &path) {
    struct stat st;

    if (stat(path.c_str(), &st) != 0) {
        return NO_SUCH_FILE_OR_DIR;
    }
    switch (st.st_mode & S_IFMT) {
        case S_IFDIR:
            return IS_A_DIR;
        case S_IFREG:
            if ((st.st_mode & S_IRUSR) == 0) {
                return PERMISSION;
            }
            return NONE;
        default:
            return UNKNOWN;
    }
}

std::string error_message(const ErrorType &type) {
    switch (type) {
        case NONE:
            return "";
        case NO_SUCH_FILE_OR_DIR:
            return "No such file or directory";
        case IS_A_DIR:
            return "Is a directory";
        case PERMISSION:
            return "Permission denied";
        case UNKNOWN:
            return "Invalid file type";
    }
    //    コンパイルを通すため
    return "no reach";
}

std::string get_directory_name(const std::string &file_path) {
    size_t slash_pos = file_path.rfind('/');
    if (slash_pos == std::string::npos) {
        return std::string(".");
    }
    return  file_path.substr(0, slash_pos);
}

std::string read(const std::string &path) {
    std::ifstream input_file(path);
    std::string data((std::istreambuf_iterator<char>(input_file)), std::istreambuf_iterator<char>());
    return data;
}

} // namespace file
