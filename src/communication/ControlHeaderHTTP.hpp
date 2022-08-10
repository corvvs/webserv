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
    HTTP::light_string boundary;

    // "application/octet-stream"
    // 値がないときはこれに設定するのではなく, この値と**みなす**
    static const HTTP::byte_string default_value;

    minor_error determine(const AHeaderHolder &holder);
    void store_list_item(const parameter_key_type &key, const parameter_value_type &val);
    static HTTP::byte_string normalize(const HTTP::byte_string &str);
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

struct Location : public IControlHeader {
    HTTP::byte_string value;
    HTTP::light_string abs_path;
    HTTP::light_string query_string;
    HTTP::light_string fragment;
    HTTP::light_string authority;
    bool is_local;

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

} // namespace CH
} // namespace CGIP

namespace MultiPart {
namespace CH {
struct ContentType : public HTTP::CH::ContentType {};
struct ContentDisposition : public HTTP::CH::ContentDisposition {};

} // namespace CH
} // namespace MultiPart

#endif
