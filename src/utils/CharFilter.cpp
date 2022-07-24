#include "CharFilter.hpp"
#define BITS_IN_ELEM (sizeof(u64t) * 8) // 64
#define ELEMS (256 / BITS_IN_ELEM)      // 4
#define OCTETS (sizeof(u64t) * ELEMS)   // 32

HTTP::CharFilter::CharFilter(const byte_string &chars) {
    fill(chars);
}

HTTP::CharFilter::CharFilter(const char *chars) {
    fill(byte_string(chars, chars + strlen(chars)));
}

HTTP::CharFilter::CharFilter(byte_type from, byte_type to) {
    fill(from, to);
}

HTTP::CharFilter::CharFilter(const CharFilter &other) {
    *this = other;
}

HTTP::CharFilter &HTTP::CharFilter::operator=(const CharFilter &rhs) {
    if (this != &rhs) {
        memcpy(filter, rhs.filter, OCTETS);
    }
    return *this;
}

HTTP::CharFilter &HTTP::CharFilter::operator=(const byte_string &rhs) {
    fill(rhs);
    return *this;
}

HTTP::CharFilter HTTP::CharFilter::operator|(const CharFilter &rhs) const {
    CharFilter n(*this);
    for (unsigned int i = 0; i < ELEMS; ++i) {
        n.filter[i] |= rhs.filter[i];
    }
    return n;
}

HTTP::CharFilter HTTP::CharFilter::operator&(const CharFilter &rhs) const {
    CharFilter n(*this);
    for (unsigned int i = 0; i < ELEMS; ++i) {
        n.filter[i] &= rhs.filter[i];
    }
    return n;
}

HTTP::CharFilter HTTP::CharFilter::operator^(const CharFilter &rhs) const {
    CharFilter n(*this);
    for (unsigned int i = 0; i < ELEMS; ++i) {
        n.filter[i] ^= rhs.filter[i];
    }
    return n;
}

HTTP::CharFilter HTTP::CharFilter::operator~() const {
    CharFilter n(*this);
    for (unsigned int i = 0; i < ELEMS; ++i) {
        n.filter[i] = ~n.filter[i];
    }
    return n;
}

HTTP::CharFilter HTTP::CharFilter::operator-(const CharFilter &rhs) const {
    return *this & ~rhs;
}

void HTTP::CharFilter::fill(const byte_string &chars) {
    memset(filter, 0, OCTETS);
    for (byte_string::size_type i = 0; i < chars.size(); ++i) {
        byte_type c                = chars[i];
        unsigned int element_index = c / BITS_IN_ELEM;
        unsigned int bit_index     = c & (BITS_IN_ELEM - 1);
        filter[element_index] |= (u64t)1 << bit_index;
    }
}

void HTTP::CharFilter::fill(byte_type from, byte_type to) {
    memset(filter, 0, OCTETS);
    for (; from <= to; ++from) {
        byte_type c                = from;
        unsigned int element_index = c / BITS_IN_ELEM;
        unsigned int bit_index     = c & (BITS_IN_ELEM - 1);
        filter[element_index] |= (u64t)1 << bit_index;
        if (from == to) {
            break;
        }
    }
}

bool HTTP::CharFilter::includes(byte_type c) const {
    unsigned int element_index = c / BITS_IN_ELEM;
    unsigned int bit_index     = c & (BITS_IN_ELEM - 1);
    int x                      = !!(filter[element_index] & ((u64t)1 << bit_index));
    return x;
}

// 0x5555..
// 0x5  = 0b0101
// 0x55 = 0b01010101
// 0xa  = 0b1010
// 0xaa = 0b10101010

// byte_type x;
// x & 0x55 = x & 0b01010101 -> x の偶数ビットのみ残す
// x & 0xaa = x & 0b10101010 -> x の奇数ビットのみ残す
// (x & 0xaa) >> 1 -> x の奇数ビットを偶数ビットにずらす
// (x & 0x55) + ((x & 0xaa) >> 1) -> xの"隣り合うビット" [0,1] [2,3] [4,5] [6,7] の和

// x = 167         = 0b 10 10 01 11
// (x & 0x55)      = 0b 00 00 01 01
// (x & 0xaa)      = 0b 10 10 00 10
// (x & 0xaa) >> 1 = 0b 01 01 00 01
// (x & 0x55) + ((x & 0xaa) >> 1)
//                 = 0b 01 01 01 10 = y (popcount in 2bit)

// 0x33            = 0b 00 11 00 11
// 0xcc            = 0b 11 00 11 00
// (y & 0x33)      = 0b 00 01 00 10
// (y & 0xcc)      = 0b 01 00 01 00
// (y & 0xcc) >> 2 = 0b 00 01 00 01
// (y & 0x33) + ((y & 0xcc) >> 2)
//                 = 0b 00 10 00 11 = z (popcount in 4bit)

// 0x0f            = 0b 00 00 11 11
// 0xf0            = 0b 11 11 00 00

// 0x5555 = 0b0101010101010101 0xaaaa = 0b1010101010101010 1
// 0x3333 = 0b0011001100110011 0xcccc = 0b1100110011001100 2
// 0x0f0f = 0b0000111100001111 0xf0f0 = 0b1111000011110000 4
// 0x00ff = 0b0000000011111111 0xff00 = 0b1111111100000000 8

HTTP::byte_string::size_type HTTP::CharFilter::size() const {
    byte_string::size_type n = 0;
    for (unsigned int i = 0; i < ELEMS; ++i) {
        u64t x = filter[i];
        x      = (x & 0x5555555555555555U) + ((x & 0xaaaaaaaaaaaaaaaaU) >> 1);
        x      = (x & 0x3333333333333333U) + ((x & 0xccccccccccccccccU) >> 2);
        x      = (x & 0x0f0f0f0f0f0f0f0fU) + ((x & 0xf0f0f0f0f0f0f0f0U) >> 4);
        x      = (x & 0x00ff00ff00ff00ffU) + ((x & 0xff00ff00ff00ff00U) >> 8);
        x      = (x & 0x0000ffff0000ffffU) + ((x & 0xffff0000ffff0000U) >> 16);
        x      = (x & 0x00000000ffffffffU) + ((x & 0xffffffff00000000U) >> 32);
        n += x;
    }
    return n;
}

HTTP::byte_string HTTP::CharFilter::str() const {
    HTTP::byte_string out;
    out.reserve(size());
    for (unsigned int i = 0; i < 256; ++i) {
        byte_type c = i;
        if (includes(c)) {
            out.push_back(c);
        }
    }
    return out;
}

std::ostream &operator<<(std::ostream &ost, const HTTP::CharFilter &f) {
    return ost << "(" << f.size() << "):[" << f.str() << "]";
}

const HTTP::CharFilter HTTP::CharFilter::alpha_low         = HTTP::strfy("abcdefghijklmnopqrstuvwxyz");
const HTTP::CharFilter HTTP::CharFilter::alpha_up          = HTTP::strfy("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
const HTTP::CharFilter HTTP::CharFilter::alpha             = alpha_low | alpha_up;
const HTTP::CharFilter HTTP::CharFilter::digit             = HTTP::strfy("0123456789");
const HTTP::CharFilter HTTP::CharFilter::hexdig            = digit | "abcdef" | "ABCDEF";
const HTTP::CharFilter HTTP::CharFilter::unreserved        = alpha | digit | "-._~";
const HTTP::CharFilter HTTP::CharFilter::gen_delims        = HTTP::strfy(":/?#[]@");
const HTTP::CharFilter HTTP::CharFilter::sub_delims        = HTTP::strfy("!$&'()*+.;=");
const HTTP::CharFilter HTTP::CharFilter::tchar             = alpha | digit | "!#$%&'*+-.^_`|~";
const HTTP::CharFilter HTTP::CharFilter::sp                = HTTP::strfy(" ");
const HTTP::CharFilter HTTP::CharFilter::bad_sp            = sp;
const HTTP::CharFilter HTTP::CharFilter::ws                = HTTP::strfy(" \t");
const HTTP::CharFilter HTTP::CharFilter::crlf              = HTTP::strfy("\r\n");
const HTTP::CharFilter HTTP::CharFilter::cr                = HTTP::strfy("\r");
const HTTP::CharFilter HTTP::CharFilter::lf                = HTTP::strfy("\n");
const HTTP::CharFilter HTTP::CharFilter::htab              = HTTP::strfy("\t");
const HTTP::CharFilter HTTP::CharFilter::dquote            = HTTP::strfy("\"");
const HTTP::CharFilter HTTP::CharFilter::bslash            = HTTP::strfy("\\");
const HTTP::CharFilter HTTP::CharFilter::obs_text          = HTTP::CharFilter(0x80, 0xff);
const HTTP::CharFilter HTTP::CharFilter::vchar             = HTTP::CharFilter(0x21, 0x7e);
const HTTP::CharFilter HTTP::CharFilter::printables        = HTTP::CharFilter(0x20, 0x7e);
const HTTP::CharFilter HTTP::CharFilter::qdtext            = vchar | sp | htab | obs_text - "\"\\";
const HTTP::CharFilter HTTP::CharFilter::qdright           = vchar | sp | htab | obs_text;
const HTTP::CharFilter HTTP::CharFilter::ctext             = vchar | sp | htab | obs_text - "()\\";
const HTTP::CharFilter HTTP::CharFilter::reserved          = gen_delims | sub_delims;
const HTTP::CharFilter HTTP::CharFilter::cgi_mark          = "-_.!~*'()";
const HTTP::CharFilter HTTP::CharFilter::cgi_reserved      = ";/?:@&=+$,[]";
const HTTP::CharFilter HTTP::CharFilter::cgi_unreserved    = alpha | digit | cgi_mark;
const HTTP::CharFilter HTTP::CharFilter::cgi_sp            = HTTP::strfy(" ");
const HTTP::CharFilter HTTP::CharFilter::cgi_extra         = HTTP::strfy(":@&=+$,");
const HTTP::CharFilter HTTP::CharFilter::pchar_without_pct = unreserved | sub_delims | ":@";
const HTTP::CharFilter HTTP::CharFilter::uri_scheme        = alpha | digit | "+-.";

// parameter      = token "=" ( token / quoted-string )
// quoted-string  = DQUOTE *( qdtext / quoted-pair ) DQUOTE
// qdtext         = HTAB / SP / %x21 / %x23-5B / %x5D-7E / obs-text
//                ;               !     #-[        ]-~
//                ; HTAB + SP + 表示可能文字, ただし "(ダブルクオート) と \(バッスラ) を除く
// obs-text       = %x80-FF ; extended ASCII
// quoted-pair    = "\" ( HTAB / SP / VCHAR / obs-text )

// comment        = "(" *( ctext / quoted-pair / comment ) ")"
// ctext          = HTAB / SP / %x21-27 / %x2A-5B / %x5D-7E / obs-text
//                ; HTAB + SP + 表示可能文字, ただしカッコとバッスラを除く
