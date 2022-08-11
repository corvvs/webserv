#include "File.hpp"
#include <cassert>
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
        default:
            return "Unknown error";
    }
}

std::string get_directory_name(const std::string &file_path) {
    assert(!file_path.empty());

    size_t slash_pos = file_path.rfind('/');
    if (slash_pos == std::string::npos) {
        return std::string("./");
    }
    return file_path.substr(0, slash_pos + 1);
}

std::string read(const std::string &path) {
    std::ifstream input_file(path);
    std::string data((std::istreambuf_iterator<char>(input_file)), std::istreambuf_iterator<char>());
    return data;
}

bool is_dir(const std::string &path) {
    struct stat st;
    if (stat(path.c_str(), &st) != 0) {
        return false;
    }
    return S_ISDIR(st.st_mode);
}

bool is_file(const std::string &path) {
    struct stat st;
    if (stat(path.c_str(), &st) != 0) {
        return false;
    }
    return S_ISREG(st.st_mode);
}

bool is_readable(const std::string &path) {
    struct stat st;
    if (stat(path.c_str(), &st) != 0) {
        return false;
    }
    return S_ISREG(st.st_mode) && (st.st_mode & S_IRUSR);
}

bool is_writable(const std::string &path) {
    struct stat st;
    if (stat(path.c_str(), &st) != 0) {
        return false;
    }
    return S_ISREG(st.st_mode) && (st.st_mode & S_IWUSR);
}

bool is_executable(const std::string &path) {
    struct stat st;
    if (stat(path.c_str(), &st) != 0) {
        return false;
    }
    return S_ISREG(st.st_mode) && (st.st_mode & S_IXUSR);
}

} // namespace file
