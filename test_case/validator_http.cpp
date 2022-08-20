#include "../../src/communication/ValidatorHTTP.hpp"
#include "gtest/gtest.h"
#include <vector>
#define LS(c_str) (HTTP::light_string(HTTP::strfy(c_str)))

// [is_valid_header_host]
TEST(is_valid_header_host, basic) {
    EXPECT_FALSE(HTTP::Validator::is_valid_header_host(LS("")));
}

// [is_uri_host]

// [is_ip_literal]
TEST(is_ip_literal, basic) {
    EXPECT_EQ(true, HTTP::Validator::is_ip_literal(LS("[2001:0db8:1234:5678:90ab:cdef:0000:0000]")));
    EXPECT_EQ(true, HTTP::Validator::is_ip_literal(LS("[2001:0db8:0000:0000:3456:0000:0000:0000]")));
    EXPECT_EQ(true, HTTP::Validator::is_ip_literal(LS("[2001:0db8:0:0:3456:0:0:0]")));
    EXPECT_EQ(true, HTTP::Validator::is_ip_literal(LS("[2001:0db8:0:0:3456::0]")));
    EXPECT_EQ(true, HTTP::Validator::is_ip_literal(LS("[2001:0db8:0:0:3456::]")));
    EXPECT_EQ(true, HTTP::Validator::is_ip_literal(LS("[2001:0db8:0000:0000:3456::]")));
    EXPECT_EQ(true, HTTP::Validator::is_ip_literal(LS("[2001:0db8::3456:0000:0000:0000]")));
    EXPECT_EQ(true, HTTP::Validator::is_ip_literal(LS("[2001:db8::3456:0000:0000:0000]")));
    EXPECT_EQ(true, HTTP::Validator::is_ip_literal(LS("[2001:db8::3456:0000:0000:0]")));
    EXPECT_EQ(true, HTTP::Validator::is_ip_literal(LS("[2001:db8::3456:0:0:0]")));
    // --
    EXPECT_EQ(false, HTTP::Validator::is_ip_literal(LS("[]")));
    EXPECT_EQ(false, HTTP::Validator::is_ip_literal(LS("[aaaa]")));
    EXPECT_EQ(false, HTTP::Validator::is_ip_literal(LS("[a]")));
    EXPECT_EQ(false, HTTP::Validator::is_ip_literal(LS("[127.0.0.1]")));
    EXPECT_EQ(false, HTTP::Validator::is_ip_literal(LS("2001:0db8:1234:5678:90ab:cdef:0000:0000")));
    EXPECT_EQ(false, HTTP::Validator::is_ip_literal(LS("2001:0db8:0000:0000:3456:0000:0000:0000")));
    EXPECT_EQ(false, HTTP::Validator::is_ip_literal(LS("2001:0db8:0:0:3456:0:0:0")));
    EXPECT_EQ(false, HTTP::Validator::is_ip_literal(LS("2001:0db8:0:0:3456::0")));
    EXPECT_EQ(false, HTTP::Validator::is_ip_literal(LS("2001:0db8:0:0:3456::")));
    EXPECT_EQ(false, HTTP::Validator::is_ip_literal(LS("2001:0db8:0000:0000:3456::")));
    EXPECT_EQ(false, HTTP::Validator::is_ip_literal(LS("2001:0db8::3456:0000:0000:0000")));
    EXPECT_EQ(false, HTTP::Validator::is_ip_literal(LS("2001:db8::3456:0000:0000:0000")));
    EXPECT_EQ(false, HTTP::Validator::is_ip_literal(LS("2001:db8::3456:0000:0000:0")));
    EXPECT_EQ(false, HTTP::Validator::is_ip_literal(LS("2001:db8::3456:0:0:0")));
    EXPECT_EQ(false, HTTP::Validator::is_ip_literal(LS("2001:0db8:1234:5678:90ab:cdef:0000:0000]")));
    EXPECT_EQ(false, HTTP::Validator::is_ip_literal(LS("2001:0db8:0000:0000:3456:0000:0000:0000]")));
    EXPECT_EQ(false, HTTP::Validator::is_ip_literal(LS("2001:0db8:0:0:3456:0:0:0]")));
    EXPECT_EQ(false, HTTP::Validator::is_ip_literal(LS("2001:0db8:0:0:3456::0]")));
    EXPECT_EQ(false, HTTP::Validator::is_ip_literal(LS("2001:0db8:0:0:3456::]")));
    EXPECT_EQ(false, HTTP::Validator::is_ip_literal(LS("2001:0db8:0000:0000:3456::]")));
    EXPECT_EQ(false, HTTP::Validator::is_ip_literal(LS("2001:0db8::3456:0000:0000:0000]")));
    EXPECT_EQ(false, HTTP::Validator::is_ip_literal(LS("2001:db8::3456:0000:0000:0000]")));
    EXPECT_EQ(false, HTTP::Validator::is_ip_literal(LS("2001:db8::3456:0000:0000:0]")));
    EXPECT_EQ(false, HTTP::Validator::is_ip_literal(LS("2001:db8::3456:0:0:0]")));
    EXPECT_EQ(false, HTTP::Validator::is_ip_literal(LS("[2001:0db8:1234:5678:90ab:cdef:0000:0000")));
    EXPECT_EQ(false, HTTP::Validator::is_ip_literal(LS("[2001:0db8:0000:0000:3456:0000:0000:0000")));
    EXPECT_EQ(false, HTTP::Validator::is_ip_literal(LS("[2001:0db8:0:0:3456:0:0:0")));
    EXPECT_EQ(false, HTTP::Validator::is_ip_literal(LS("[2001:0db8:0:0:3456::0")));
    EXPECT_EQ(false, HTTP::Validator::is_ip_literal(LS("[2001:0db8:0:0:3456::")));
    EXPECT_EQ(false, HTTP::Validator::is_ip_literal(LS("[2001:0db8:0000:0000:3456::")));
    EXPECT_EQ(false, HTTP::Validator::is_ip_literal(LS("[2001:0db8::3456:0000:0000:0000")));
    EXPECT_EQ(false, HTTP::Validator::is_ip_literal(LS("[2001:db8::3456:0000:0000:0000")));
    EXPECT_EQ(false, HTTP::Validator::is_ip_literal(LS("[2001:db8::3456:0000:0000:0")));
    EXPECT_EQ(false, HTTP::Validator::is_ip_literal(LS("[2001:db8::3456:0:0:0")));
    EXPECT_EQ(false, HTTP::Validator::is_ip_literal(LS("]2001:0db8:1234:5678:90ab:cdef:0000:0000[")));
    EXPECT_EQ(false, HTTP::Validator::is_ip_literal(LS("[[2001:0db8:1234:5678:90ab:cdef:0000:0000]]")));
    EXPECT_EQ(false, HTTP::Validator::is_ip_literal(LS("[2001:0db8:1234:5678:90ab:cdef:0000:0000]]")));
}

// [is_ipv6address]
TEST(is_ipv6address, basic) {
    EXPECT_EQ(true, HTTP::Validator::is_ipv6address(LS("2001:0db8:1234:5678:90ab:cdef:0000:0000")));
    EXPECT_EQ(true, HTTP::Validator::is_ipv6address(LS("2001:0db8:0000:0000:3456:0000:0000:0000")));
    EXPECT_EQ(true, HTTP::Validator::is_ipv6address(LS("2001:0db8:0:0:3456:0:0:0")));
    EXPECT_EQ(true, HTTP::Validator::is_ipv6address(LS("2001:0db8:0:0:3456::0")));
    EXPECT_EQ(true, HTTP::Validator::is_ipv6address(LS("2001:0db8:0:0:3456::")));
    EXPECT_EQ(true, HTTP::Validator::is_ipv6address(LS("2001:0db8:0000:0000:3456::")));
    EXPECT_EQ(true, HTTP::Validator::is_ipv6address(LS("2001:0db8::3456:0000:0000:0000")));
    EXPECT_EQ(true, HTTP::Validator::is_ipv6address(LS("2001:db8::3456:0000:0000:0000")));
    EXPECT_EQ(true, HTTP::Validator::is_ipv6address(LS("2001:db8::3456:0000:0000:0")));
    EXPECT_EQ(true, HTTP::Validator::is_ipv6address(LS("2001:db8::3456:0:0:0")));
    // --
    EXPECT_EQ(false, HTTP::Validator::is_ipv6address(LS("2001:0db8::3456::")));
    EXPECT_EQ(false, HTTP::Validator::is_ipv6address(LS("")));
    EXPECT_EQ(false, HTTP::Validator::is_ipv6address(LS("2001:0db8:1234:5678:90ab:cdef:g000:0000")));
    EXPECT_EQ(false, HTTP::Validator::is_ipv6address(LS("2001:0db8:1234:5678:90ab:cdef:0000:cafe:0000")));
}

// [is_ipvfuture]
TEST(is_ipvfuture, basic) {
    EXPECT_EQ(true, HTTP::Validator::is_ipvfuture(LS("v6.fe80::a+en1")));
    EXPECT_EQ(true, HTTP::Validator::is_ipvfuture(LS("v1.fe80::a_en1")));
    // --
    EXPECT_EQ(false, HTTP::Validator::is_ipvfuture(LS("0.0.0.0")));
    EXPECT_EQ(false, HTTP::Validator::is_ipvfuture(LS("255.255.255.255")));
    EXPECT_EQ(false, HTTP::Validator::is_ipvfuture(LS("")));
}

// [is_ipv4address]
TEST(is_ipv4address, basic) {
    EXPECT_EQ(true, HTTP::Validator::is_ipv4address(LS("0.0.0.0")));
    EXPECT_EQ(true, HTTP::Validator::is_ipv4address(LS("255.255.255.255")));
    EXPECT_EQ(true, HTTP::Validator::is_ipv4address(LS("127.0.0.1")));
    EXPECT_EQ(true, HTTP::Validator::is_ipv4address(LS("192.168.0.1")));
    EXPECT_EQ(true, HTTP::Validator::is_ipv4address(LS("1.1.1.10")));
    EXPECT_EQ(true, HTTP::Validator::is_ipv4address(LS("1.1.1.100")));
    EXPECT_EQ(true, HTTP::Validator::is_ipv4address(LS("1.1.1.255")));
    // --
    EXPECT_EQ(false, HTTP::Validator::is_ipv4address(LS("0.0.0.0.0")));
    EXPECT_EQ(false, HTTP::Validator::is_ipv4address(LS("0.0.0.0.")));
    EXPECT_EQ(false, HTTP::Validator::is_ipv4address(LS(".0.0.0.0")));
    EXPECT_EQ(false, HTTP::Validator::is_ipv4address(LS(".0.0.0.0.")));
    EXPECT_EQ(false, HTTP::Validator::is_ipv4address(LS("1.1.1.1.1")));
    EXPECT_EQ(false, HTTP::Validator::is_ipv4address(LS("1.1.1.1.10")));
    EXPECT_EQ(false, HTTP::Validator::is_ipv4address(LS("1.1.1.1.10.")));
    EXPECT_EQ(false, HTTP::Validator::is_ipv4address(LS("1.1.1.010")));
    EXPECT_EQ(false, HTTP::Validator::is_ipv4address(LS("1.1.1.999")));
    EXPECT_EQ(false, HTTP::Validator::is_ipv4address(LS("1.1.1.256")));
    EXPECT_EQ(false, HTTP::Validator::is_ipv4address(LS("1.1.256.1")));
    EXPECT_EQ(false, HTTP::Validator::is_ipv4address(LS("1.256.1.1")));
    EXPECT_EQ(false, HTTP::Validator::is_ipv4address(LS("256.1.1.1")));
    EXPECT_EQ(false, HTTP::Validator::is_ipv4address(LS("1111.111.11.1")));
    EXPECT_EQ(false, HTTP::Validator::is_ipv4address(LS("ap.p.l.e")));
    EXPECT_EQ(false, HTTP::Validator::is_ipv4address(LS("oh yeah")));
    EXPECT_EQ(false, HTTP::Validator::is_ipv4address(LS("...")));
    EXPECT_EQ(false, HTTP::Validator::is_ipv4address(LS("1..1.1.1")));
    EXPECT_EQ(false, HTTP::Validator::is_ipv4address(LS("1..1.1")));
    EXPECT_EQ(false, HTTP::Validator::is_ipv4address(LS("1....1.1.1")));
    EXPECT_EQ(false, HTTP::Validator::is_ipv4address(LS("")));
}

// [is_reg_name]
TEST(is_reg_name, basic) {
    EXPECT_EQ(true, HTTP::Validator::is_reg_name(LS("dino")));
    EXPECT_EQ(true, HTTP::Validator::is_reg_name(LS("42.fr")));
    EXPECT_EQ(true, HTTP::Validator::is_reg_name(LS("google.com")));
    EXPECT_EQ(true, HTTP::Validator::is_reg_name(LS("")));
    EXPECT_EQ(true, HTTP::Validator::is_reg_name(LS(".")));
    EXPECT_EQ(true, HTTP::Validator::is_reg_name(LS("127.0.0.1"))); // IPv4 is also reg-name
    EXPECT_EQ(true, HTTP::Validator::is_reg_name(LS("42.%80%81%82.fr")));
    // -
    EXPECT_EQ(false, HTTP::Validator::is_reg_name(LS("?")));
}

// [is_port]
TEST(is_port, basic) {
    EXPECT_EQ(true, HTTP::Validator::is_port(LS("0")));
    EXPECT_EQ(true, HTTP::Validator::is_port(LS("00")));
    EXPECT_EQ(true, HTTP::Validator::is_port(LS("000")));
    EXPECT_EQ(true, HTTP::Validator::is_port(LS("0000")));
    EXPECT_EQ(true, HTTP::Validator::is_port(LS("00000")));
    EXPECT_EQ(true, HTTP::Validator::is_port(LS("000000")));
    EXPECT_EQ(true, HTTP::Validator::is_port(LS("1")));
    EXPECT_EQ(true, HTTP::Validator::is_port(LS("80")));
    EXPECT_EQ(true, HTTP::Validator::is_port(LS("443")));
    EXPECT_EQ(true, HTTP::Validator::is_port(LS("8080")));
    EXPECT_EQ(true, HTTP::Validator::is_port(LS("")));
    // --
    EXPECT_EQ(false, HTTP::Validator::is_port(LS(" ")));
    EXPECT_EQ(false, HTTP::Validator::is_port(LS("port")));
    EXPECT_EQ(false, HTTP::Validator::is_port(LS("cafe")));
    EXPECT_EQ(false, HTTP::Validator::is_port(LS("-")));
    EXPECT_EQ(false, HTTP::Validator::is_port(LS(":")));
    EXPECT_EQ(false, HTTP::Validator::is_port(LS("+")));
    EXPECT_EQ(false, HTTP::Validator::is_port(LS("\t")));
    EXPECT_EQ(false, HTTP::Validator::is_port(LS("\n")));
    EXPECT_EQ(false, HTTP::Validator::is_port(LS("\f")));
}

// [is_h16]
TEST(is_h16, single_digit) {
    EXPECT_EQ(true, HTTP::Validator::is_h16(LS("0")));
    EXPECT_EQ(true, HTTP::Validator::is_h16(LS("8")));
    EXPECT_EQ(true, HTTP::Validator::is_h16(LS("9")));
    EXPECT_EQ(true, HTTP::Validator::is_h16(LS("a")));
    EXPECT_EQ(true, HTTP::Validator::is_h16(LS("A")));
    EXPECT_EQ(true, HTTP::Validator::is_h16(LS("f")));
    EXPECT_EQ(true, HTTP::Validator::is_h16(LS("F")));
    // --
    EXPECT_EQ(false, HTTP::Validator::is_h16(LS("x")));
    EXPECT_EQ(false, HTTP::Validator::is_h16(LS("X")));
    EXPECT_EQ(false, HTTP::Validator::is_h16(LS("g")));
    EXPECT_EQ(false, HTTP::Validator::is_h16(LS("G")));
    EXPECT_EQ(false, HTTP::Validator::is_h16(LS("-")));
    EXPECT_EQ(false, HTTP::Validator::is_h16(LS("\t")));
    EXPECT_EQ(false, HTTP::Validator::is_h16(LS("\n")));
}

TEST(is_h16, multiple_digits) {
    EXPECT_EQ(true, HTTP::Validator::is_h16(LS("00")));
    EXPECT_EQ(true, HTTP::Validator::is_h16(LS("000")));
    EXPECT_EQ(true, HTTP::Validator::is_h16(LS("0000")));
    EXPECT_EQ(true, HTTP::Validator::is_h16(LS("ffff")));
    EXPECT_EQ(true, HTTP::Validator::is_h16(LS("01ab")));
    EXPECT_EQ(true, HTTP::Validator::is_h16(LS("AbCd")));
    // --
    EXPECT_EQ(false, HTTP::Validator::is_h16(LS("00000")));
    EXPECT_EQ(false, HTTP::Validator::is_h16(LS(" ffff")));
    EXPECT_EQ(false, HTTP::Validator::is_h16(LS("ffff ")));
    EXPECT_EQ(false, HTTP::Validator::is_h16(LS("ff f")));
    EXPECT_EQ(false, HTTP::Validator::is_h16(LS("ff_f")));
}

TEST(is_h16, irregular) {
    EXPECT_EQ(false, HTTP::Validator::is_h16(LS("")));
    EXPECT_EQ(false, HTTP::Validator::is_h16(LS("0000000000000000000000000")));
}

// [is_ls32]

// [is_uri_authority]

// [is_uri_path]

// [is_segment]
TEST(is_segment, pchar_without_pct) {
    const HTTP::CharFilter &filter = HTTP::CharFilter::pchar_without_pct;
    EXPECT_EQ(true, HTTP::Validator::is_segment(LS(""), filter));
    EXPECT_EQ(true, HTTP::Validator::is_segment(LS("a"), filter));
    EXPECT_EQ(true, HTTP::Validator::is_segment(LS("-"), filter));
    EXPECT_EQ(true, HTTP::Validator::is_segment(LS("%80"), filter));
    EXPECT_EQ(true, HTTP::Validator::is_segment(LS("%800"), filter));
    EXPECT_EQ(true,
              HTTP::Validator::is_segment(
                  LS("%E3%82%A2%E3%83%B3%E3%83%91%E3%83%B3%E3%83%81+and+%E3%82%A2%E3%83%B3%E3%82%AD%E3%83%83%E3%82%AF"),
                  HTTP::CharFilter::pchar_without_pct));
    EXPECT_EQ(true, HTTP::Validator::is_segment(LS(""), filter));
    // --
    EXPECT_EQ(false, HTTP::Validator::is_segment(LS("%%80"), filter));
    EXPECT_EQ(false, HTTP::Validator::is_segment(LS("%8%80"), filter));
    EXPECT_EQ(false, HTTP::Validator::is_segment(LS("[apple]"), filter));
}

TEST(is_segment, cgi) {
    const HTTP::CharFilter uric = HTTP::CharFilter::cgi_reserved | HTTP::CharFilter::cgi_unreserved;
    EXPECT_EQ(true, HTTP::Validator::is_segment(LS(""), uric));
    EXPECT_EQ(true, HTTP::Validator::is_segment(LS("?"), uric));
    EXPECT_EQ(true, HTTP::Validator::is_segment(LS("???"), uric));
    EXPECT_EQ(true, HTTP::Validator::is_segment(LS("?/?/?/"), uric));
    EXPECT_EQ(true,
              HTTP::Validator::is_segment(
                  LS("%E3%82%A2%E3%83%B3%E3%83%91%E3%83%B3%E3%83%81+and+%E3%82%A2%E3%83%B3%E3%82%AD%E3%83%83%E3%82%AF"),
                  uric));
    EXPECT_EQ(true, HTTP::Validator::is_segment(LS("[apple]"), uric));
    // [] を含むセグメントはHTTPの仕様では非合法だがCGIの仕様では合法
    // --
    EXPECT_EQ(false, HTTP::Validator::is_segment(LS("k=#"), uric));
}
