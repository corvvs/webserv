#include "http.hpp"

const HTTP::t_version HTTP::DEFAULT_HTTP_VERSION = V_1_1;

// アルファベット・小文字
const HTTP::byte_string HTTP::Charset::alpha_low = HTTP::strfy("abcdefghijklmnopqrstuvwxyz");
// アルファベット・大文字
const HTTP::byte_string HTTP::Charset::alpha_up = HTTP::strfy("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
// アルファベット
const HTTP::byte_string HTTP::Charset::alpha = alpha_low + alpha_up;
// 数字
const HTTP::byte_string HTTP::Charset::digit = HTTP::strfy("0123456789");
// 16進数における数字
const HTTP::byte_string HTTP::Charset::hexdig = digit + "abcdef" + "ABCDEF";
// HTTPにおける非予約文字
const HTTP::byte_string HTTP::Charset::unreserved = alpha + digit + "-._~";
const HTTP::byte_string HTTP::Charset::gen_delims = HTTP::strfy(":/?#[]@");
const HTTP::byte_string HTTP::Charset::sub_delims = HTTP::strfy("!$&'()*+.;=");
// token 構成文字
// 空白, ":", ";", "/", "@", "?" を含まない.
// ".", "&" は含む.
const HTTP::byte_string HTTP::Charset::tchar = alpha + digit + "!#$%&'*+-.^_`|~";
const HTTP::byte_string HTTP::Charset::sp    = HTTP::strfy(" ");
const HTTP::byte_string HTTP::Charset::ws    = HTTP::strfy(" \t");
const HTTP::byte_string HTTP::Charset::crlf  = HTTP::strfy("\r\n");
const HTTP::byte_string HTTP::Charset::lf    = HTTP::strfy("\n");

// parameter      = token "=" ( token / quoted-string )
// quoted-string  = DQUOTE *( qdtext / quoted-pair ) DQUOTE
// qdtext         = HTAB / SP / %x21 / %x23-5B / %x5D-7E / obs-text
//                ;               !     #-[        ]-~
//                ; HTAB + 表示可能文字, ただし "(ダブルクオート) と \(バッスラ) を除く
// obs-text       = %x80-FF ; extended ASCII
// quoted-pair    = "\" ( HTAB / SP / VCHAR / obs-text )

const HTTP::byte_string HTTP::version_str(HTTP::t_version version) {
    switch (version) {
        case V_0_9:
            return strfy("HTTP/0.9");
        case V_1_0:
            return strfy("HTTP/1.0");
        case V_1_1:
            return strfy("HTTP/1.1");
        default:
            return strfy("");
    }
}

const HTTP::byte_string HTTP::reason(HTTP::t_status status) {
    switch (status) {
        case HTTP::STATUS_OK:
            return strfy("OK");
        case HTTP::STATUS_FOUND:
            return strfy("Found");
        case HTTP::STATUS_BAD_REQUEST:
            return strfy("Bad Request");
        case HTTP::STATUS_UNAUTHORIZED:
            return strfy("Unauthorized");
        case HTTP::STATUS_FORBIDDEN:
            return strfy("Forbidden");
        case HTTP::STATUS_NOT_FOUND:
            return strfy("Not Found");
        case HTTP::STATUS_METHOD_NOT_ALLOWED:
            return strfy("Method Not Allowed");
        case HTTP::STATUS_IM_A_TEAPOT:
            return strfy("I'm a teapot");
        case HTTP::STATUS_INTERNAL_SERVER_ERROR:
            return strfy("Internal Server Error");
        case HTTP::STATUS_NOT_IMPLEMENTED:
            return strfy("Not Implemented");
        case HTTP::STATUS_BAD_GATEWAY:
            return strfy("Bad Gateway");
        case HTTP::STATUS_SERVICE_UNAVAILABLE:
            return strfy("Service Unavailable");
        case HTTP::STATUS_VERSION_NOT_SUPPORTED:
            return strfy("HTTP Version Not Supported");
        default:
            return strfy("");
    }
}

HTTP::byte_string HTTP::strfy(const char_string &str) {
    return HTTP::byte_string(str.begin(), str.end());
}

HTTP::char_string HTTP::restrfy(const byte_string &str) {
    return HTTP::char_string(str.begin(), str.end());
}

HTTP::size_type HTTP::find(const byte_string &hay, const byte_string &needle) {
    for (size_type i = 0; i + needle.size() <= hay.size(); ++i) {
        size_type j = 0;
        for (; j < needle.size(); ++j) {
            if (needle[i + j] != hay[j]) {
                break;
            }
        }
        if (j == needle.size()) {
            return i;
        }
    }
    return npos;
}

std::ostream &operator<<(std::ostream &ost, const HTTP::byte_string &f) {
    return ost << std::string(f.begin(), f.end());
}

bool operator==(const HTTP::byte_string &lhs, const char *rhs) {
    for (HTTP::byte_string::size_type i = 0; i < lhs.size(); ++i) {
        if (!rhs[i] || lhs[i] != rhs[i]) {
            return false;
        }
    }
    return true;
}

bool operator==(const char *lhs, const HTTP::byte_string &rhs) {
    return rhs == lhs;
}

HTTP::byte_string operator+(const HTTP::byte_string &lhs, const HTTP::byte_string &rhs) {
    HTTP::byte_string rv(lhs);
    rv.insert(rv.end(), rhs.begin(), rhs.end());
    return rv;
}

HTTP::byte_string &operator+=(HTTP::byte_string &lhs, const HTTP::byte_string &rhs) {
    lhs.insert(lhs.end(), rhs.begin(), rhs.end());
    return lhs;
}

HTTP::byte_string operator+(const HTTP::byte_string &lhs, const char *rhs) {
    HTTP::byte_string rv(lhs);
    rv.insert(rv.end(), rhs, rhs + strlen(rhs));
    return rv;
}
