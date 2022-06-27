#include "http.hpp"

const HTTP::t_version HTTP::DEFAULT_HTTP_VERSION = V_1_1;

// アルファベット・小文字
const HTTP::byte_string HTTP::Charset::alpha_low = "abcdefghijklmnopqrstuvwxyz";
// アルファベット・大文字
const HTTP::byte_string HTTP::Charset::alpha_up = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
// アルファベット
const HTTP::byte_string HTTP::Charset::alpha = alpha_low + alpha_up;
// 数字
const HTTP::byte_string HTTP::Charset::digit = "0123456789";
// 16進数における数字
const HTTP::byte_string HTTP::Charset::hexdig = digit + "abcdef" + "ABCDEF";
// HTTPにおける非予約文字
const HTTP::byte_string HTTP::Charset::unreserved = alpha + digit + "-._~";
const HTTP::byte_string HTTP::Charset::gen_delims = ":/?#[]@";
const HTTP::byte_string HTTP::Charset::sub_delims = "!$&'()*+.;=";
// token 構成文字
// 空白, ":", ";", "/", "@", "?" を含まない.
// ".", "&" は含む.
const HTTP::byte_string HTTP::Charset::tchar = alpha + digit + "!#$%&'*+-.^_`|~";
const HTTP::byte_string HTTP::Charset::sp    = " ";
const HTTP::byte_string HTTP::Charset::ws    = " \t";
const HTTP::byte_string HTTP::Charset::crlf  = "\r\n";
const HTTP::byte_string HTTP::Charset::lf    = "\n";

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
            return "HTTP/0.9";
        case V_1_0:
            return "HTTP/1.0";
        case V_1_1:
            return "HTTP/1.1";
        default:
            return "";
    }
}

const HTTP::byte_string HTTP::reason(HTTP::t_status status) {
    switch (status) {
        case HTTP::STATUS_OK:
            return "OK";
        case HTTP::STATUS_FOUND:
            return "Found";
        case HTTP::STATUS_BAD_REQUEST:
            return "Bad Request";
        case HTTP::STATUS_UNAUTHORIZED:
            return "Unauthorized";
        case HTTP::STATUS_FORBIDDEN:
            return "Forbidden";
        case HTTP::STATUS_NOT_FOUND:
            return "Not Found";
        case HTTP::STATUS_METHOD_NOT_ALLOWED:
            return "Method Not Allowed";
        case HTTP::STATUS_IM_A_TEAPOT:
            return "I'm a teapot";
        case HTTP::STATUS_INTERNAL_SERVER_ERROR:
            return "Internal Server Error";
        case HTTP::STATUS_NOT_IMPLEMENTED:
            return "Not Implemented";
        case HTTP::STATUS_BAD_GATEWAY:
            return "Bad Gateway";
        case HTTP::STATUS_SERVICE_UNAVAILABLE:
            return "Service Unavailable";
        case HTTP::STATUS_VERSION_NOT_SUPPORTED:
            return "HTTP Version Not Supported";
        default:
            return "";
    }
}
