#ifndef HTTP_HPP
#define HTTP_HPP
#include "test_common.hpp"
#include "types.hpp"
#include <iostream>
#include <map>
#include <string>
#include <vector>
#define MAX_REQLINE_END 8192

// 全体で共通して使うenum, 型, 定数, フリー関数など

namespace HTTP {
// ステータスコード
enum t_status {
    // 1**
    STATUS_UNSPECIFIED = 1,
    // 2**
    STATUS_OK      = 200,
    STATUS_CREATED = 201,
    // 3**
    STATUS_MOVED_PERMANENTLY  = 301,
    STATUS_FOUND              = 302,
    STATUS_NOT_MODIFIED       = 304,
    STATUS_TEMPORARY_REDIRECT = 307,
    STATUS_PERMANENT_REDIRECT = 308,
    // 4**
    STATUS_BAD_REQUEST        = 400,
    STATUS_UNAUTHORIZED       = 401,
    STATUS_FORBIDDEN          = 403,
    STATUS_NOT_FOUND          = 404,
    STATUS_METHOD_NOT_ALLOWED = 405,
    STATUS_TIMEOUT            = 408,
    STATUS_PAYLOAD_TOO_LARGE  = 413,
    STATUS_URI_TOO_LONG       = 414,
    STATUS_IM_A_TEAPOT        = 418,
    STATUS_HEADER_TOO_LARGE   = 431,
    // 5**
    STATUS_INTERNAL_SERVER_ERROR = 500,
    STATUS_NOT_IMPLEMENTED       = 501,
    STATUS_BAD_GATEWAY           = 502,
    STATUS_SERVICE_UNAVAILABLE   = 503,
    STATUS_VERSION_NOT_SUPPORTED = 505,

    STATUS_DUMMY         = 0,
    STATUS_REDIRECT_INIT = -1
};

// リクエストメソッド
enum t_method {
    METHOD_UNKNOWN,

    METHOD_GET,
    METHOD_POST,
    METHOD_DELETE,
    METHOD_OPTIONS,
    METHOD_PUT,

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
const byte_string method_str(t_method method);
const byte_string version_str(t_version version);
const byte_string reason(t_status status);

// 文字列をbyte_stringに変換する
byte_string strfy(const char_string &str);
// byte_stringを文字列に変換する
char_string restrfy(const byte_string &str);

size_type find(const byte_string &hay, const byte_string &needle);

template <class T>
class Nullable {
private:
    bool is_null_;
    T val_;

public:
    // Null状態でデフォルト構築
    Nullable() : is_null_(true) {}
    // 値が入った状態で構築
    Nullable(const T &val) : is_null_(false), val_(val) {}

    // Nullかどうか
    bool is_null() const {
        return is_null_;
    }

    // 値を取得
    // (Null状態のときに呼ばないこと)
    const T &value() const {
        assert(!is_null());
        return val_;
    }

    // Null状態に設定
    void unset() {
        is_null_ = true;
    }

    // 値を設定
    void set(const T &val) {
        is_null_ = false;
        val_     = val;
    }
};

} // namespace HTTP

std::ostream &operator<<(std::ostream &ost, const HTTP::byte_string &f);
template <class K, class V>
std::ostream &operator<<(std::ostream &ost, const std::map<K, V> &f) {
    ost << "{ " << std::endl;
    for (typename std::map<K, V>::const_iterator it = f.begin(); it != f.end(); ++it) {
        ost << "  " << it->first << ": " << it->second << "," << std::endl;
    }
    ost << " }" << std::endl;
    return ost;
}
bool operator==(const HTTP::byte_string &lhs, const char *rhs);
bool operator==(const char *lhs, const HTTP::byte_string &rhs);
HTTP::byte_string operator+(const HTTP::byte_string &lhs, const HTTP::byte_string &rhs);
HTTP::byte_string &operator+=(HTTP::byte_string &lhs, const HTTP::byte_string &rhs);
HTTP::byte_string operator+(const HTTP::byte_string &lhs, const char *rhs);

#endif
