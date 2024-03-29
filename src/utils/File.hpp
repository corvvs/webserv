#ifndef FILE_HPP
#define FILE_HPP

#include <string>

namespace file {

enum ErrorType { NONE, NO_SUCH_FILE_OR_DIR, IS_A_DIR, PERMISSION, UNKNOWN };

ErrorType check(const std::string &path);
std::string read(const std::string &path);
std::string error_message(const ErrorType &type);
std::string get_directory_name(const std::string &file_path);
time_t get_last_update_time(const std::string &path);
long get_size(const std::string &path);

bool is_dir(const std::string &path);
bool is_file(const std::string &path);
bool is_readable(const std::string &path);
bool is_writable(const std::string &path);
bool is_executable(const std::string &path);

} // namespace file

#endif
