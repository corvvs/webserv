#ifndef FILE_HPP
#define FILE_HPP

#include <string>

namespace file {

enum error_type { NONE, NO_SUCH_FILE_OR_DIR, IS_A_DIR, PERMISSION, UNKNOWN };

error_type check(const std::string &path);
std::string read(const std::string &path);
std::string error_message(const error_type &type);

} // namespace file

#endif
