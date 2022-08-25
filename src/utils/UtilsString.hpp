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

// ディレクトリパスの末尾をいい感じに処理する(trailing / を消す)
light_string rstrip_path(const light_string &path);

// パスの先頭をいい感じに処理する(leading / を消す)
light_string lstrip_path(const light_string &path);

// ディレクトリパスとファイル名をいい感じに結合する
// rstrip_path(directory_path) + "/" + lstrip_path(basename)
byte_string join_path(const light_string &directory_path, const light_string &basename);

// パスが相対パスかどうかを判定する
bool is_relative_path(const light_string &path);

// パスからbasename部分だけを取り出す
light_string basename(const light_string &path);
} // namespace Utils
} // namespace HTTP

#endif
