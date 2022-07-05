#ifndef UTILS_STRING_HPP
#define UTILS_STRING_HPP
#include "http.hpp"

namespace HTTP {
namespace Utils {
// 文字列を非破壊的に小文字化する
byte_string downcase(const byte_string &str);
// 文字列を**破壊的**に小文字化する
byte_string &downcase(byte_string &str);
// 文字列を非破壊的に大文字化する
byte_string upcase(const byte_string &str);
// 文字列を**破壊的**に大文字化する
void upcase(byte_string &str);
// HTTPヘッダをCGIメタ変数に変換するときのキーの正規化を行う
void normalize_cgi_metavar_key(byte_string &str);
} // namespace Utils
} // namespace HTTP

#endif
