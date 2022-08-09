#include "../../src/communication/HeaderHTTP.hpp"
#include "gtest/gtest.h"

// ヘッダ名の前, ':'の後, 値の前後のスペースは不問
TEST(header_http, spaces_ok) {
    const char* strs[] = {
        "Host: aaa",
        "Host: ",
        "Host:",
        " Host: aaa",
        "Host: 123123",
        NULL
    };
    for (size_t i = 0; strs[i]; ++i) {
        HTTP::byte_string s = HTTP::strfy(strs[i]);
        const HTTP::light_string l(s);
        HeaderHolderHTTP holder;
        minor_error result = holder.parse_header_line(l, &holder);
        EXPECT_EQ("", result.message());
    }
}

// ':'の前のスペース, 空のヘッダ名はNG
TEST(header_http, spaces_ko) {
    const char* strs[] = {
        "",
        "    ",
        ":",
        "Host : aaa",
        NULL
    };
    for (size_t i = 0; strs[i]; ++i) {
        HTTP::byte_string s = HTTP::strfy(strs[i]);
        const HTTP::light_string l(s);
        HeaderHolderHTTP holder;
        minor_error result = holder.parse_header_line(l, &holder);
        EXPECT_NE("", result.message());
    }
}

TEST(header_http, token_ok) {
    const char* strs[] = {
        "Host:",
        "host:",
        "hos1:",
        "123:",
        "$$$:",
        "!#$%&'*+-.^_`|~:",
        // 本当はKOだけどブラウザが許可してるっぽいので...
        "Ho st:",
        "Ho@st:",
        NULL
    };
    for (size_t i = 0; strs[i]; ++i) {
        HTTP::byte_string s = HTTP::strfy(strs[i]);
        const HTTP::light_string l(s);
        HeaderHolderHTTP holder;
        minor_error result = holder.parse_header_line(l, &holder);
        EXPECT_EQ("", result.message());
    }
}

TEST(header_http, size_ok) {
    HTTP::byte_string s;
    for (size_t i = 0; i < 1500; ++i) {
        s += HTTP::strfy("apple");
    }
    s += HTTP::strfy(": juice");
    const HTTP::light_string l(s);
    HeaderHolderHTTP holder;
    EXPECT_NO_THROW(holder.parse_header_line(l, &holder));
}

// 長すぎるヘッダー行は回復不能エラーを起こす
TEST(header_http, size_ko) {
    HTTP::byte_string s;
    for (size_t i = 0; i < 2000; ++i) {
        s += HTTP::strfy("apple");
    }
    s += HTTP::strfy(": juice");
    const HTTP::light_string l(s);
    HeaderHolderHTTP holder;
    EXPECT_THROW(holder.parse_header_line(l, &holder), http_error);
}

// ヘッダ値の前後の空白は値ではない
TEST(header_http, value_ok) {
    const char* strs[] = {
        "Host: aiueo",
        "Host:aiueo",
        "Host:aiueo    ",
        "host: aiueo",
        NULL
    };
    for (size_t i = 0; strs[i]; ++i) {
        const HTTP::byte_string s = HTTP::strfy(strs[i]);
        const HTTP::light_string l(s);
        HeaderHolderHTTP holder;
        minor_error result = holder.parse_header_line(l, &holder);
        EXPECT_EQ("", result.message());
        const AHeaderHolder::value_list_type *vals = holder.get_vals(HeaderHTTP::host);
        EXPECT_TRUE(!!vals);
        EXPECT_EQ(1, vals->size());
        EXPECT_EQ(HTTP::strfy("aiueo"), vals->front());
    }
}

// ヘッダ値内部の空白は値
TEST(header_http, value_ok_2) {
    const char* strs[] = {
        "Host:a i u e o",
        "Host: a i u e o",
        "Host:a i u e o    ",
        NULL
    };
    for (size_t i = 0; strs[i]; ++i) {
        const HTTP::byte_string s = HTTP::strfy(strs[i]);
        const HTTP::light_string l(s);
        HeaderHolderHTTP holder;
        minor_error result = holder.parse_header_line(l, &holder);
        EXPECT_EQ("", result.message());
        const AHeaderHolder::value_list_type *vals = holder.get_vals(HeaderHTTP::host);
        EXPECT_TRUE(!!vals);
        EXPECT_EQ(1, vals->size());
        EXPECT_EQ(HTTP::strfy("a i u e o"), vals->front());
    }
}

// ヘッダの重複
TEST(header_http, duplication) {
    const char* strs[] = {
        "Host:aiueo",
        "Host: 1234",
        "Host: xyz",
        " hoST:beepbeep",
        NULL
    };
    HeaderHolderHTTP holder;
    for (size_t i = 0; strs[i]; ++i) {
        const HTTP::byte_string s = HTTP::strfy(strs[i]);
        const HTTP::light_string l(s);
        minor_error result = holder.parse_header_line(l, &holder);
        EXPECT_EQ("", result.message());
        const AHeaderHolder::value_list_type *vals = holder.get_vals(HeaderHTTP::host);
        EXPECT_TRUE(!!vals);
        EXPECT_EQ(i + 1, vals->size());
    }
    const AHeaderHolder::value_list_type *vals = holder.get_vals(HeaderHTTP::host);
    EXPECT_TRUE(!!vals);
    EXPECT_EQ(HTTP::strfy("aiueo"), vals->front());
    EXPECT_EQ(HTTP::strfy("beepbeep"), vals->back());
}
