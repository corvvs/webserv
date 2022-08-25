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
class IMaybe {
public:
    ~IMaybe() {}

    // 値を保持していない場合はtrueを返す
    virtual bool is_null() const = 0;

    // 値を保持しているならその値を返す
    // 値を保持していない時に呼ばれた場合は未定義動作
    virtual T &value() = 0;

    // 値を保持しているならその値を返す
    // 値を保持していない時に呼ばれた場合は未定義動作
    virtual const T &value() const = 0;

    // 保持している値を破棄する
    virtual void unset() = 0;

    // 与えられた値をコピーして保持する
    virtual void set(const T &value) = 0;
};

template <class T>
class Nullable : public IMaybe<T> {
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
    T &value() {
        assert(!is_null());
        return val_;
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

template <class T>
class Optional : public IMaybe<T> {
private:
    u8t mem[sizeof(T)];
    std::allocator<T> alloc;
    bool is_null_;

public:
    // Null状態でデフォルト構築
    Optional() : is_null_(true) {}
    // 値が入った状態で構築
    Optional(const T &val) : is_null_(true) {
        set(val);
    }

    // Nullかどうか
    bool is_null() const {
        return is_null_;
    }

    // 値を取得
    // (Null状態のときに呼ばないこと)
    T &value() {
        assert(!is_null());
        return *((T *)mem);
    }

    // 値を取得
    // (Null状態のときに呼ばないこと)
    const T &value() const {
        assert(!is_null());
        return *((T *)mem);
    }

    // Null状態に設定
    void unset() {
        if (is_null()) {
            return;
        }
        alloc.destroy((T *)mem);
        is_null_ = true;
    }

    // 値を設定
    void set(const T &val) {
        if (is_null()) {
            alloc.construct((T *)mem, val);
            is_null_ = false;
        } else {
            *((T *)mem) = val;
        }
    }

    Optional<T> &operator=(const T &rhs) {
        set(rhs);
        return *this;
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

template <class T>
bool operator==(const HTTP::Optional<T> &lhs, const T &rhs) {
    return !lhs.is_null() && lhs.value() == rhs;
}

template <class T>
bool operator==(const T &lhs, const HTTP::Optional<T> &rhs) {
    return rhs == lhs;
}

#endif
