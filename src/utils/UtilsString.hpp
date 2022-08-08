#ifndef UTILS_STRING_HPP
#define UTILS_STRING_HPP
#include "LightString.hpp"
#include "http.hpp"

namespace HTTP {
namespace Utils {
// 文字列を非破壊的に小文字化する
byte_string downcase(const byte_string &str);
// 文字列を**破壊的**に小文字化する
void downcase(byte_string &str);
// 文字列を非破壊的に大文字化する
byte_string upcase(const byte_string &str);
// 文字列を**破壊的**に大文字化する
void upcase(byte_string &str);
// HTTPヘッダをCGIメタ変数に変換するときのキーの正規化を行う
void normalize_cgi_metavar_key(byte_string &str);

// ディレクトリパスとファイル名をいい感じに結合する
byte_string join_path(const light_string &directory_path, const light_string &basename);
} // namespace Utils
} // namespace HTTP

#endif
