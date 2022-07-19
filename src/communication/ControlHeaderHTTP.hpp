#ifndef CONTROLHEADERHTTP_HPP
#define CONTROLHEADERHTTP_HPP
#include "../utils/LightString.hpp"
#include "../utils/http.hpp"
#include <map>
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

struct TransferEncoding {
    // 指定されたTransferCodingが登場順に入る.
    std::vector<HTTP::Term::TransferCoding> transfer_codings;
    // 現在のTransferCodingが "chunked" かどうか.
    bool currently_chunked;

    // 指定がないかどうか
    bool empty() const;
    // 現在のTransferCoding; empty() == true の時に呼び出してはならない.
    const Term::TransferCoding &current_coding() const;
};

struct ContentType : public IDictHolder {

    HTTP::byte_string value;
    parameter_dict parameters;

    // "application/octet-stream"
    static const HTTP::byte_string default_value;

    void store_list_item(const parameter_key_type &key, const parameter_value_type &val);
};

struct Connection {
    std::vector<byte_string> connection_options;
    bool keep_alive_; // keep-alive が true == 持続的接続を行う とは限らないことに注意.
    bool close_;

    bool will_keep_alive() const;
    bool will_close() const;
};

struct TE {
    std::vector<HTTP::Term::TransferCoding> transfer_codings;
};

struct Upgrade {
    std::vector<HTTP::Term::Protocol> protocols;
};

struct Via {
    std::vector<HTTP::Term::Received> receiveds;
};

} // namespace CH
} // namespace HTTP

namespace CGIP {
namespace CH {
typedef std::string parameter_key_type;
typedef HTTP::light_string parameter_value_type;
typedef std::map<parameter_key_type, parameter_value_type> parameter_dict;

struct ContentType : public HTTP::CH::ContentType {};

struct Status {
    int code;
    HTTP::byte_string reason;
};

struct Location {
    HTTP::byte_string value;
    HTTP::light_string abs_path;
    HTTP::light_string query_string;
    HTTP::light_string fragment;
    HTTP::light_string authority;
    bool is_local;
};

} // namespace CH
} // namespace CGIP

#endif
