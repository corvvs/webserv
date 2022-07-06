#ifndef HTTP_HPP
#define HTTP_HPP
#include "test_common.hpp"
#include "types.hpp"
#include <iostream>
#include <map>
#include <string>
#include <vector>

// 全体で共通して使うenum, 型, 定数, フリー関数など

namespace HTTP {
// ステータスコード
enum t_status {
    STATUS_OK = 200,

    STATUS_FOUND = 302,

    STATUS_BAD_REQUEST        = 400,
    STATUS_UNAUTHORIZED       = 401,
    STATUS_FORBIDDEN          = 403,
    STATUS_NOT_FOUND          = 404,
    STATUS_METHOD_NOT_ALLOWED = 405,
    STATUS_URI_TOO_LONG       = 414,
    STATUS_IM_A_TEAPOT        = 418,

    STATUS_INTERNAL_SERVER_ERROR = 500,
    STATUS_NOT_IMPLEMENTED       = 501,
    STATUS_BAD_GATEWAY           = 502,
    STATUS_SERVICE_UNAVAILABLE   = 503,
    STATUS_VERSION_NOT_SUPPORTED = 505,

    STATUS_DUMMY = 0
};

// リクエストメソッド
enum t_method {
    METHOD_UNKNOWN,

    METHOD_GET,
    METHOD_POST,
    METHOD_DELETE,

    METHOD_ERROR
};

// HTTPバージョン
enum t_version {
    V_UNKNOWN,

    V_0_9,
    V_1_0,
    V_1_1,

    V_ERROR
};

typedef char char_type;
typedef u8t byte_type;
// バイト列
typedef std::vector<char_type> byte_string;
typedef std::basic_string<char_type> char_string;
typedef std::string::size_type size_type;
const size_type npos = std::string::npos;
// ヘッダのキーの型
typedef byte_string header_key_type;
// ヘッダの値の型
typedef byte_string header_val_type;
// ヘッダのキー・値の組
typedef std::pair<header_key_type, header_val_type> header_kvpair_type;
// ヘッダを格納する辞書
typedef std::map<header_key_type, header_val_type> header_dict_type;

// サーバのデフォルトのHTTPバージョン
extern const t_version DEFAULT_HTTP_VERSION;
const byte_string version_str(t_version version);
const byte_string reason(t_status status);

// 文字集合
namespace Charset {

// アルファベット・小文字
extern const byte_string alpha_low;
// アルファベット・大文字
extern const byte_string alpha_up;
// アルファベット
extern const byte_string alpha;
// 数字
extern const byte_string digit;
// 16進数における数字
extern const byte_string hexdig;
// HTTPにおける非予約文字
extern const byte_string unreserved;
extern const byte_string gen_delims;
extern const byte_string sub_delims;
// token 構成文字
// 空白, ":", ";", "/", "@", "?" を含まない.
// ".", "&" は含む.
extern const byte_string tchar;
extern const byte_string sp;
extern const byte_string ws;
extern const byte_string crlf;
extern const byte_string lf;
} // namespace Charset

// 文字列をbyte_stringに変換する
byte_string strfy(const char_string &str);
// byte_stringを文字列に変換する
char_string restrfy(const byte_string &str);

size_type find(const byte_string &hay, const byte_string &needle);

} // namespace HTTP

std::ostream &operator<<(std::ostream &ost, const HTTP::byte_string &f);
bool operator==(const HTTP::byte_string &lhs, const char *rhs);
bool operator==(const char *lhs, const HTTP::byte_string &rhs);
HTTP::byte_string operator+(const HTTP::byte_string &lhs, const HTTP::byte_string &rhs);
HTTP::byte_string &operator+=(HTTP::byte_string &lhs, const HTTP::byte_string &rhs);
HTTP::byte_string operator+(const HTTP::byte_string &lhs, const char *rhs);

#endif
