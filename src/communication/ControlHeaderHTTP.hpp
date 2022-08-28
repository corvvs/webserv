#ifndef CONTROLHEADERHTTP_HPP
#define CONTROLHEADERHTTP_HPP
#include "../utils/HTTPError.hpp"
#include "../utils/LightString.hpp"
#include "../utils/http.hpp"
#include "HeaderHTTP.hpp"
#include <map>
#include <set>
#include <vector>

namespace HTTP {
class IDictHolder {
public:
    typedef HTTP::byte_string parameter_key_type;
    typedef HTTP::light_string parameter_value_type;
    typedef std::map<parameter_key_type, parameter_value_type> parameter_dict;
    virtual ~IDictHolder() {}

    // key と value を受け取って何かする関数
    // 何をするかは実装クラス次第
    virtual void store_list_item(const parameter_key_type &key, const parameter_value_type &val) = 0;
};

class IControlHeader {
public:
    virtual ~IControlHeader() {}

    virtual minor_error determine(const AHeaderHolder &holder) = 0;
};

namespace Term {
struct TransferCoding : public IDictHolder {
    byte_string coding;
    IDictHolder::parameter_dict parameters;
    unsigned int quality_int; // q の値(あれば)を1000倍したもの
    void store_list_item(const parameter_key_type &key, const parameter_value_type &val);
    static TransferCoding init();
};

struct Host {
    // Host: の値全体
    HTTP::byte_string value;
    HTTP::byte_string host;
    HTTP::byte_string port;
};

struct Protocol {
    light_string name;
    light_string version;
};

struct Received {
    Protocol protocol;
    Host host;
};
} // namespace Term

// Control-Header
namespace CH {
typedef std::string parameter_key_type;
typedef HTTP::light_string parameter_value_type;
typedef std::map<parameter_key_type, parameter_value_type> parameter_dict;

typedef HTTP::Term::Host Host;

struct TransferEncoding : public IControlHeader {
    // 指定されたTransferCodingが登場順に入る.
    std::vector<HTTP::Term::TransferCoding> transfer_codings;
    // 現在のTransferCodingが "chunked" かどうか.
    bool currently_chunked;
    minor_error merror;

    // 指定がないかどうか
    bool empty() const;
    // 現在のTransferCoding; empty() == true の時に呼び出してはならない.
    const Term::TransferCoding &current_coding() const;
    minor_error determine(const AHeaderHolder &holder);
    static HTTP::byte_string normalize(const HTTP::byte_string &str);

    TransferEncoding();
};

struct ContentLength : public IControlHeader {
    std::set<size_t> lengths;
    minor_error merror;
    size_t value;

    // 指定がないかどうか
    bool empty() const;
    minor_error determine(const AHeaderHolder &holder);

    ContentLength();
};

struct ContentType : public IControlHeader, public IDictHolder {

    HTTP::byte_string value;
    parameter_dict parameters;
    Nullable<HTTP::byte_string> charset;
    Nullable<HTTP::byte_string> default_charset;
    // light_string なのはリクエストからの入力のみを想定しているから
    Nullable<HTTP::light_string> boundary;

    // "application/octet-stream"
    // 値がないときはこれに設定するのではなく, この値と**みなす**
    static const HTTP::byte_string default_value;

    minor_error determine(const AHeaderHolder &holder);
    void store_list_item(const parameter_key_type &key, const parameter_value_type &val);
    static HTTP::byte_string normalize(const HTTP::byte_string &str);
    void set_default_charset(const HTTP::byte_string &default_val);

    // 現在保持しているデータを, HTTPヘッダの値として文字列化して返す
    HTTP::byte_string serialize() const;
};

struct ContentDisposition : public IControlHeader, public IDictHolder {

    HTTP::byte_string value;
    parameter_dict parameters;

    minor_error determine(const AHeaderHolder &holder);
    void store_list_item(const parameter_key_type &key, const parameter_value_type &val);
};

struct Connection : public IControlHeader {
    std::vector<byte_string> connection_options;
    bool keep_alive_; // keep-alive が true == 持続的接続を行う とは限らないことに注意.
    bool close_;

    minor_error determine(const AHeaderHolder &holder);
    bool will_keep_alive() const;
    bool will_close() const;
};

struct TE : public IControlHeader {
    std::vector<HTTP::Term::TransferCoding> transfer_codings;
    minor_error determine(const AHeaderHolder &holder);
};

struct Upgrade : public IControlHeader {
    std::vector<HTTP::Term::Protocol> protocols;
    minor_error determine(const AHeaderHolder &holder);
};

struct Via : public IControlHeader {
    std::vector<HTTP::Term::Received> receiveds;
    minor_error determine(const AHeaderHolder &holder);
};

struct Date : public IControlHeader {
    t_time_epoch_ms value;
    minor_error merror;

    minor_error determine(const AHeaderHolder &holder);

    // 現在時刻をあらわす Date オブジェクトを生成
    static Date now();
    // 現在保持しているデータを, HTTPヘッダの値として文字列化して返す
    HTTP::byte_string serialize() const;
};

struct Location : public IControlHeader {
    HTTP::byte_string value;
    HTTP::light_string abs_path;
    HTTP::light_string query_string;
    HTTP::light_string fragment;
    HTTP::light_string authority;
    bool is_local;

    minor_error determine(const AHeaderHolder &holder);
};

struct CookieEntry {
    minor_error error;
    HTTP::byte_string name;
    HTTP::byte_string value;

    // https://developer.mozilla.org/ja/docs/Web/HTTP/Headers/Set-Cookie#%E5%B1%9E%E6%80%A7

    // > クッキーの有効期限を示す、 HTTP の日時タイムスタンプです。
    HTTP::Nullable<t_time_epoch_ms> expires;
    // > クッキーの期限までの秒数を示します。ゼロまたは負の数値の場合は、クッキーは直ちに期限切れになります。
    // > Expires および Max-Age の両方が設定されていたら、 Max-Age が優先されます。
    HTTP::Nullable<long> max_age;
    // > クッキーを送信する先のホストを定義します。
    // > 指定されなかった場合は、この属性は既定で現在の文書の URL
    // におけるホスト名の部分になり、サブドメインを含みません。
    HTTP::byte_string domain;
    // > リクエストの URL に含む必要があるパスを示します。含まれていないと、ブラウザーは Cookie ヘッダーを送信しません。
    // > スラッシュ (/) の文字はディレクトリー区切りとして解釈され、サブディレクトリーも同様に一致します。
    HTTP::byte_string path;
    // > クッキーが、リクエストが SSL と HTTPS
    // プロトコルを使用して行われた場合にのみサーバーに送信されることを示します。
    bool secure;
    // > JavaScript が Document.cookie プロパティなどを介してこのクッキーにアクセスすることを禁止します。
    bool http_only;

    enum t_same_site {
        SAMESITE_LAX, // lax: ゆるい, てきとう; 既定の動作
        SAMESITE_STRICT,
        SAMESITE_NONE
    };
    // > クロスサイトリクエストでクッキーを送信するかどうかを制御し、クロスサイトリクエストフォージェリ攻撃 (CSRF)
    // に対するある程度の防御を提供します。
    HTTP::Nullable<t_same_site> same_site;

    // name=value の解析
    light_string parse_name_value(const light_string &str);
    // Expire=... の解析
    light_string parse_expire(const light_string &str);
    // Max-Age=... の解析
    light_string parse_max_age(const light_string &str);
    // Domain=... の解析
    light_string parse_domain(const light_string &str);
    // Path=... の解析
    light_string parse_path(const light_string &str);
    // Secure の解析
    light_string parse_secure(const light_string &str);
    // HttpOnly の解析
    light_string parse_http_only(const light_string &str);
    // SameSite の解析
    light_string parse_same_site(const light_string &str);
    CookieEntry();
};

struct Cookie : public IControlHeader {
    typedef HTTP::byte_string name_type;
    typedef std::map<name_type, CookieEntry> cookie_map_type;

    cookie_map_type values;
    minor_error merror;
    minor_error determine(const AHeaderHolder &holder);
};

struct SetCookie : public IControlHeader {
    typedef HTTP::byte_string name_type;
    typedef std::map<name_type, CookieEntry> cookie_map_type;

    cookie_map_type values;
    minor_error merror;
    minor_error determine(const AHeaderHolder &holder);
};

} // namespace CH
} // namespace HTTP

namespace CGIP {
namespace CH {
typedef std::string parameter_key_type;
typedef HTTP::light_string parameter_value_type;
typedef std::map<parameter_key_type, parameter_value_type> parameter_dict;

struct ContentType : public HTTP::CH::ContentType {};

struct Status : public HTTP::IControlHeader {
    int code;
    HTTP::byte_string reason;

    minor_error determine(const AHeaderHolder &holder);
};

struct Location : public HTTP::CH::Location {};

struct SetCookie : public HTTP::CH::SetCookie {};

} // namespace CH
} // namespace CGIP

namespace MultiPart {
namespace CH {
struct ContentType : public HTTP::CH::ContentType {};
struct ContentDisposition : public HTTP::CH::ContentDisposition {};

} // namespace CH
} // namespace MultiPart

#endif
