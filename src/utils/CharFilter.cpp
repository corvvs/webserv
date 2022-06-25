#include "CharFilter.hpp"
#define ELEMS 32

HTTP::CharFilter::CharFilter(const byte_string& chars) {
    fill(chars);
}

HTTP::CharFilter::CharFilter(const char *chars) {
    fill(byte_string(chars));
}

HTTP::CharFilter::CharFilter(byte_type from, byte_type to) {
    fill(from, to);
}

HTTP::CharFilter::CharFilter(const CharFilter& other) {
    *this = other;
}

HTTP::CharFilter& HTTP::CharFilter::operator=(const CharFilter& rhs) {
    if (this != &rhs) {
        memcpy(filter, rhs.filter, ELEMS);
    }
    return *this;
}

HTTP::CharFilter& HTTP::CharFilter::operator=(const byte_string& rhs) {
    fill(rhs);
    return *this;
}

HTTP::CharFilter HTTP::CharFilter::operator|(const CharFilter& rhs) const {
    CharFilter n(*this);
    for (int i = 0; i < ELEMS; ++i) {
        n.filter[i] |= rhs.filter[i];
    }
    return n;
}

HTTP::CharFilter HTTP::CharFilter::operator&(const CharFilter& rhs) const {
    CharFilter n(*this);
    for (int i = 0; i < ELEMS; ++i) {
        n.filter[i] &= rhs.filter[i];
    }
    return n;
}

HTTP::CharFilter HTTP::CharFilter::operator^(const CharFilter& rhs) const {
    CharFilter n(*this);
    for (int i = 0; i < ELEMS; ++i) {
        n.filter[i] ^= rhs.filter[i];
    }
    return n;
}

HTTP::CharFilter HTTP::CharFilter::operator~() const {
    CharFilter n(*this);
    for (int i = 0; i < ELEMS; ++i) {
        n.filter[i] = ~n.filter[i];
    }
    return n;
}

HTTP::CharFilter HTTP::CharFilter::operator-(const CharFilter& rhs) const {
    return *this & ~rhs;
}

void HTTP::CharFilter::fill(const byte_string& chars) {
    memset(filter, 0, ELEMS);
    // 3 の出どころは 2^3 = 8.
    for (byte_string::size_type i = 0; i < chars.size(); ++i) {
        byte_type c = chars[i];
        filter[(c >> 3)] |= (unsigned int)1 << (c & (((unsigned int)1 << 3) - 1));
    }
}

void HTTP::CharFilter::fill(byte_type from, byte_type to) {
    memset(filter, 0, ELEMS);
    for (; from <= to; ++from) {
        byte_type c = from;
        filter[(c >> 3)] |= (unsigned int)1 << (c & (((unsigned int)1 << 3) - 1));
        if (from == to) { break; }
    }
}

bool HTTP::CharFilter::includes(byte_type c) const {
    int x = (filter[(c >> 3)] & ((unsigned int)1 << (c & (((unsigned int)1 << 3) - 1))));
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

HTTP::byte_string::size_type    HTTP::CharFilter::size() const {
    byte_string::size_type n = 0;
    for (int i = 0; i < ELEMS; ++i) {
        unsigned int x = filter[i];
        x = (x & 0x55U) + ((x & 0xaaU) >> 1);
        x = (x & 0x33U) + ((x & 0xccU) >> 2);
        x = (x & 0x0fU) + ((x & 0xf0U) >> 4);
        n += x;
    }
    return n;
}

HTTP::byte_string   HTTP::CharFilter::str() const {
    HTTP::byte_string   out;
    out.reserve(size());
    for (unsigned int i = 0; i < 256; ++i) {
        byte_type c = i;
        if (includes(c)) { out.push_back(c); }
    }
    return out;
}

std::ostream&   operator<<(std::ostream& ost, const HTTP::CharFilter& f) {
    return ost << "(" << f.size() << "):[" << f.str() << "]";
}

const HTTP::CharFilter HTTP::CharFilter::alpha_low  = HTTP::Charset::alpha_low;
const HTTP::CharFilter HTTP::CharFilter::alpha_up   = HTTP::Charset::alpha_up;
const HTTP::CharFilter HTTP::CharFilter::alpha      = HTTP::Charset::alpha;
const HTTP::CharFilter HTTP::CharFilter::digit      = HTTP::Charset::digit;
const HTTP::CharFilter HTTP::CharFilter::hexdig     = HTTP::Charset::hexdig;
const HTTP::CharFilter HTTP::CharFilter::unreserved = HTTP::Charset::unreserved;
const HTTP::CharFilter HTTP::CharFilter::gen_delims = HTTP::Charset::gen_delims;
const HTTP::CharFilter HTTP::CharFilter::sub_delims = HTTP::Charset::sub_delims;
const HTTP::CharFilter HTTP::CharFilter::tchar      = HTTP::Charset::tchar;
const HTTP::CharFilter HTTP::CharFilter::sp         = HTTP::Charset::sp;
const HTTP::CharFilter HTTP::CharFilter::ws         = HTTP::Charset::ws;
const HTTP::CharFilter HTTP::CharFilter::crlf       = HTTP::Charset::crlf;
const HTTP::CharFilter HTTP::CharFilter::cr         = "\r";
const HTTP::CharFilter HTTP::CharFilter::lf         = HTTP::Charset::lf;
const HTTP::CharFilter HTTP::CharFilter::htab       = "\t";
const HTTP::CharFilter HTTP::CharFilter::dquote     = "\"";
const HTTP::CharFilter HTTP::CharFilter::bslash     = "\\";
const HTTP::CharFilter HTTP::CharFilter::obs_text   = HTTP::CharFilter(0x80, 0xff);
const HTTP::CharFilter HTTP::CharFilter::vchar      = HTTP::CharFilter(0x21, 0x7e);
const HTTP::CharFilter HTTP::CharFilter::qdtext     = HTTP::CharFilter::vchar | HTTP::CharFilter::htab | HTTP::CharFilter::obs_text - "\"\\";
const HTTP::CharFilter HTTP::CharFilter::quoted_right = HTTP::CharFilter::vchar | " \t" | HTTP::CharFilter::obs_text;

// parameter      = token "=" ( token / quoted-string )
// quoted-string  = DQUOTE *( qdtext / quoted-pair ) DQUOTE
// qdtext         = HTAB / SP / %x21 / %x23-5B / %x5D-7E / obs-text
//                ;               !     #-[        ]-~
//                ; HTAB + 表示可能文字, ただし "(ダブルクオート) と \(バッスラ) を除く
// obs-text       = %x80-FF ; extended ASCII
// quoted-pair    = "\" ( HTAB / SP / VCHAR / obs-text )
