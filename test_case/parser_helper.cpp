#include "../../src/communication/ParserHelper.hpp"
#include "gtest/gtest.h"
#include <climits>
#include <vector>

// [find_crlf]

TEST(parser_helper_find_crlf, basic) {
    const HTTP::byte_string str = HTTP::strfy("012\r456\n890\r\r345\r\r\r901\r\n\r567\n\r\n123\n\n\n");
    EXPECT_EQ(IndexRange(7, 8), ParserHelper::find_crlf(str, 0, str.size()));
    EXPECT_EQ(IndexRange(6, 5), ParserHelper::find_crlf(str, 0, 5));
    EXPECT_EQ(IndexRange(7, 8), ParserHelper::find_crlf(str, 7, str.size()));
    EXPECT_EQ(IndexRange(22, 24), ParserHelper::find_crlf(str, 10, str.size()));
    EXPECT_EQ(IndexRange(36, 37), ParserHelper::find_crlf(str, 36, str.size()));
    EXPECT_EQ(IndexRange(38, 37), ParserHelper::find_crlf(str, 37, str.size()));
}

TEST(parser_helper_find_crlf, backward) {
    const HTTP::byte_string str = HTTP::strfy("\r\n\n");
    EXPECT_EQ(IndexRange(0, 2), ParserHelper::find_crlf(str, 0, str.size()));
    EXPECT_EQ(IndexRange(2, 1), ParserHelper::find_crlf(str, 0, 1));
    EXPECT_EQ(IndexRange(0, 2), ParserHelper::find_crlf(str, 1, str.size()));
    EXPECT_EQ(IndexRange(2, 3), ParserHelper::find_crlf(str, 2, str.size()));
}

TEST(parser_helper_find_crlf, nothing) {
    const HTTP::byte_string str = HTTP::strfy("aiueo");
    EXPECT_EQ(IndexRange(1, 0), ParserHelper::find_crlf(str, 0, 0));
    EXPECT_EQ(IndexRange(str.size() + 1, str.size()), ParserHelper::find_crlf(str, 0, str.size()));
}

// [find_crlf_header_value]

// [find_obs_fold]

// [ignore_crlf]

// [is_sp]
TEST(parser_helper_is_sp, is_sp) {
    EXPECT_EQ(true, ParserHelper::is_sp(ParserHelper::SP[0]));
    EXPECT_EQ(false, ParserHelper::is_sp('\t'));
    EXPECT_EQ(false, ParserHelper::is_sp('a'));
    EXPECT_EQ(false, ParserHelper::is_sp('\v'));
    EXPECT_EQ(false, ParserHelper::is_sp('\n'));
    EXPECT_EQ(false, ParserHelper::is_sp('\r'));
}

// [ignore_sp]

// [ignore_not_sp]

// [find_leading_crlf]

// [split_by_sp]

// [split]

// [normalize_header_key]

// [xtou]

// [str_to_u]
TEST(parser_helper_str_to_u, is_0) {
    const HTTP::byte_string str        = HTTP::strfy("0");
    std::pair<bool, unsigned long> res = ParserHelper::str_to_u(str);
    EXPECT_EQ(true, res.first);
    EXPECT_EQ(0, res.second);
}

TEST(parser_helper_str_to_u, is_1) {
    const HTTP::byte_string str        = HTTP::strfy("1");
    std::pair<bool, unsigned long> res = ParserHelper::str_to_u(str);
    EXPECT_EQ(true, res.first);
    EXPECT_EQ(1, res.second);
}

TEST(parser_helper_str_to_u, is_ULONG_MAX_less_1) {
    const HTTP::byte_string str        = HTTP::strfy("18446744073709551614");
    std::pair<bool, unsigned long> res = ParserHelper::str_to_u(str);
    EXPECT_EQ(true, res.first);
    EXPECT_EQ(ULONG_MAX - 1, res.second);
}

TEST(parser_helper_str_to_u, is_ULONG_MAX) {
    const HTTP::byte_string str        = HTTP::strfy("18446744073709551615");
    std::pair<bool, unsigned long> res = ParserHelper::str_to_u(str);
    EXPECT_EQ(true, res.first);
    EXPECT_EQ(ULONG_MAX, res.second);
}

TEST(parser_helper_str_to_u, is_ULONG_MAX_with_leading_0) {
    const HTTP::byte_string str        = HTTP::strfy("00000000018446744073709551615");
    std::pair<bool, unsigned long> res = ParserHelper::str_to_u(str);
    EXPECT_EQ(false, res.first);
}

TEST(parser_helper_str_to_u, is_ULONG_MAX_plus_1) {
    const HTTP::byte_string str        = HTTP::strfy("18446744073709551616");
    std::pair<bool, unsigned long> res = ParserHelper::str_to_u(str);
    EXPECT_EQ(false, res.first);
}

TEST(parser_helper_str_to_u, is_minus_1) {
    const HTTP::byte_string str        = HTTP::strfy("-1");
    std::pair<bool, unsigned long> res = ParserHelper::str_to_u(str);
    EXPECT_EQ(false, res.first);
}

// [quality_to_u]

// [extract_quoted_or_token]

// [http_date_to_time]

TEST(parser_helper_str_to_http_date, imf_fixdate_ok) {
    const HTTP::byte_string str          = HTTP::strfy("Sun, 06 Nov 1994 08:49:37 GMT");
    std::pair<bool, t_time_epoch_ms> res = ParserHelper::http_date_to_time(str);
    EXPECT_TRUE(res.first);
}

TEST(parser_helper_str_to_http_date, imf_fixdate_ok_unixtime_origin) {
    const HTTP::byte_string str          = HTTP::strfy("Sun, 01 Jan 1970 00:00:00 GMT");
    std::pair<bool, t_time_epoch_ms> res = ParserHelper::http_date_to_time(str);
    EXPECT_TRUE(res.first);
    EXPECT_EQ(0, res.second);
}

TEST(parser_helper_str_to_http_date, imf_fixdate_ok_unixtime_origin_1) {
    const HTTP::byte_string str          = HTTP::strfy("Sun, 01 Jan 1970 00:00:01 GMT");
    std::pair<bool, t_time_epoch_ms> res = ParserHelper::http_date_to_time(str);
    EXPECT_TRUE(res.first);
    EXPECT_EQ(1000, res.second);
}

TEST(parser_helper_str_to_http_date, imf_fixdate_ok_unixtime_now) {
    const HTTP::byte_string str          = HTTP::strfy("Wed, 10 Aug 2022 02:09:46 GMT");
    std::pair<bool, t_time_epoch_ms> res = ParserHelper::http_date_to_time(str);
    EXPECT_TRUE(res.first);
    EXPECT_EQ(1660097386000, res.second);
}

TEST(parser_helper_str_to_http_date, imf_fixdate_ko) {
    const char *strs[] = {"",
                          "  ",
                          "Max, 01 Nov 1994 08:49:37 GMT",
                          "sun, 06 Nov 1994 08:49:37 GMT",
                          "Sun 06 Nov 1994 08:49:37 GMT",
                          "Sun,06 Nov 1994 08:49:37 GMT",
                          "Sun, 6 Nov 1994 08:49:37 GMT",
                          "Sun,  6 Nov 1994 08:49:37 GMT",
                          "Sun, 99 Nov 1994 08:49:37 GMT",
                          "Sun, 32 Nov 1994 08:49:37 GMT",
                          " Sun, 06 Nov 1994 08:49:37 JST",
                          "Sun , 06 Nov 1994 08:49:37 JST",
                          "Sun, 06 Nov 1994 24:49:37 JST",
                          "Sun, 06 Nov 1994 08:60:37 JST",
                          "Sun, 06 Nov 1994 08:49:60 JST",
                          "Sun, 06 Nov 1994 08:49:37 JST",
                          "Sun, 06 Nov 1994 08:49:37 GMT ",
                          "Sun, 06 Nov 1994 08:49:37 gmt",
                          NULL};
    for (int i = 0; strs[i]; ++i) {
        const HTTP::byte_string str          = HTTP::strfy(strs[i]);
        std::pair<bool, t_time_epoch_ms> res = ParserHelper::http_date_to_time(str);
        EXPECT_FALSE(res.first);
    }
}

TEST(parser_helper_str_to_http_date, asctime_ok1) {
    const HTTP::byte_string str          = HTTP::strfy("Sun Nov  6 08:49:37 1994");
    std::pair<bool, t_time_epoch_ms> res = ParserHelper::http_date_to_time(str);
    EXPECT_TRUE(res.first);
}

TEST(parser_helper_str_to_http_date, asctime_ok_unixtime_origin) {
    const HTTP::byte_string str          = HTTP::strfy("Sun Jan  1 00:00:00 1970");
    std::pair<bool, t_time_epoch_ms> res = ParserHelper::http_date_to_time(str);
    EXPECT_TRUE(res.first);
    EXPECT_EQ(0, res.second);
}

TEST(parser_helper_str_to_http_date, asctime_ok_unixtime_origin_1) {
    const HTTP::byte_string str          = HTTP::strfy("Sun Jan  1 00:00:01 1970");
    std::pair<bool, t_time_epoch_ms> res = ParserHelper::http_date_to_time(str);
    EXPECT_TRUE(res.first);
    EXPECT_EQ(1000, res.second);
}

TEST(parser_helper_str_to_http_date, asctime_ok_unixtime_now) {
    const HTTP::byte_string str          = HTTP::strfy("Wed Aug 10 02:09:46 2022");
    std::pair<bool, t_time_epoch_ms> res = ParserHelper::http_date_to_time(str);
    EXPECT_TRUE(res.first);
    EXPECT_EQ(1660097386000, res.second);
}

TEST(parser_helper_str_to_http_date, asctime_ko) {
    const char *strs[] = {"Sun Nov 006 08:49:37 1994", "Sun Nov  6 08:49:37 1994 GMT", NULL};
    for (int i = 0; strs[i]; ++i) {
        const HTTP::byte_string str          = HTTP::strfy(strs[i]);
        std::pair<bool, t_time_epoch_ms> res = ParserHelper::http_date_to_time(str);
        EXPECT_FALSE(res.first);
    }
}

TEST(parser_helper_str_to_http_date, rfc850_date_ok1) {
    const HTTP::byte_string str          = HTTP::strfy("Sunday, 06-Nov-94 08:49:37 GMT");
    std::pair<bool, t_time_epoch_ms> res = ParserHelper::http_date_to_time(str);
    EXPECT_TRUE(res.first);
}

TEST(parser_helper_str_to_http_date, rfc850_date_ok_unixtime_origin) {
    const HTTP::byte_string str          = HTTP::strfy("Sunday, 01-Jan-70 00:00:00 GMT");
    std::pair<bool, t_time_epoch_ms> res = ParserHelper::http_date_to_time(str);
    EXPECT_TRUE(res.first);
    EXPECT_EQ(0, res.second);
}

TEST(parser_helper_str_to_http_date, rfc850_date_ok_unixtime_origin_1) {
    const HTTP::byte_string str          = HTTP::strfy("Sunday, 01-Jan-70 00:00:01 GMT");
    std::pair<bool, t_time_epoch_ms> res = ParserHelper::http_date_to_time(str);
    EXPECT_TRUE(res.first);
    EXPECT_EQ(1000, res.second);
}

TEST(parser_helper_str_to_http_date, rfc850_date_ok_unixtime_now) {
    const HTTP::byte_string str          = HTTP::strfy("Wednesday, 10-Aug-22 02:09:46 GMT");
    std::pair<bool, t_time_epoch_ms> res = ParserHelper::http_date_to_time(str);
    EXPECT_TRUE(res.first);
    EXPECT_EQ(1660097386000, res.second);
}

// [decode_pct_encoded]

TEST(parser_helper_decode_pct_encoded, basic_1) {
    const HTTP::byte_string str = HTTP::strfy("apple");
    const HTTP::byte_string exp = HTTP::strfy("apple");
    const HTTP::byte_string ans = ParserHelper::decode_pct_encoded(str);
    EXPECT_EQ(exp, ans);
}

TEST(parser_helper_decode_pct_encoded, basic_2) {
    const HTTP::byte_string str = HTTP::strfy("https%3A%2F%2Fprofile.intra.42.fr%2F");
    const HTTP::byte_string exp = HTTP::strfy("https://profile.intra.42.fr/");
    const HTTP::byte_string ans = ParserHelper::decode_pct_encoded(str);
    EXPECT_EQ(exp, ans);
}

TEST(parser_helper_decode_pct_encoded, basic_blank) {
    const HTTP::byte_string str = HTTP::strfy("");
    const HTTP::byte_string exp = HTTP::strfy("");
    const HTTP::byte_string ans = ParserHelper::decode_pct_encoded(str);
    EXPECT_EQ(exp, ans);
}

TEST(parser_helper_decode_pct_encoded, basic_a_percent) {
    const HTTP::byte_string str = HTTP::strfy("%");
    const HTTP::byte_string exp = HTTP::strfy("%");
    const HTTP::byte_string ans = ParserHelper::decode_pct_encoded(str);
    EXPECT_EQ(exp, ans);
}

TEST(parser_helper_decode_pct_encoded, basic_2_percents) {
    const HTTP::byte_string str = HTTP::strfy("%%");
    const HTTP::byte_string exp = HTTP::strfy("%%");
    const HTTP::byte_string ans = ParserHelper::decode_pct_encoded(str);
    EXPECT_EQ(exp, ans);
}

TEST(parser_helper_decode_pct_encoded, basic_3_percents) {
    const HTTP::byte_string str = HTTP::strfy("%%%");
    const HTTP::byte_string exp = HTTP::strfy("%%%");
    const HTTP::byte_string ans = ParserHelper::decode_pct_encoded(str);
    EXPECT_EQ(exp, ans);
}

TEST(parser_helper_decode_pct_encoded, basic_hiragana) {
    const HTTP::byte_string str = HTTP::strfy("%E3%82%8A%E3%82%93%E3%81%94");
    const HTTP::byte_string exp = HTTP::strfy("りんご");
    const HTTP::byte_string ans = ParserHelper::decode_pct_encoded(str);
    EXPECT_EQ(exp, ans);
}

TEST(parser_helper_decode_pct_encoded, basic_kanji) {
    const HTTP::byte_string str = HTTP::strfy("%E9%AD%8F%E3%83%BB%E5%91%89%E3%83%BB%E8%9C%80");
    const HTTP::byte_string exp = HTTP::strfy("魏・呉・蜀");
    const HTTP::byte_string ans = ParserHelper::decode_pct_encoded(str);
    EXPECT_EQ(exp, ans);
}

TEST(parser_helper_decode_pct_encoded, basic_kanji_high_low_mix) {
    const HTTP::byte_string str = HTTP::strfy("%E9%aD%8F%E3%83%bb%E5%91%89%E3%83%Bb%E8%9C%80");
    const HTTP::byte_string exp = HTTP::strfy("魏・呉・蜀");
    const HTTP::byte_string ans = ParserHelper::decode_pct_encoded(str);
    EXPECT_EQ(exp, ans);
}

TEST(parser_helper_decode_pct_encoded, invalid_missing) {
    // へんなパーセントエンコードが混じっている場合
    const HTTP::byte_string str = HTTP::strfy("%E3%8%82");
    HTTP::byte_string exp;
    exp.push_back(0xe3);
    exp += HTTP::strfy("%8");
    exp.push_back(0x82);
    const HTTP::byte_string ans = ParserHelper::decode_pct_encoded(str);
    EXPECT_EQ(exp, ans);
}

TEST(parser_helper_decode_pct_encoded, invalid_unexpected) {
    // へんなパーセントエンコードが混じっている場合
    const HTTP::byte_string str = HTTP::strfy("%E3%8g%82");
    HTTP::byte_string exp;
    exp.push_back(0xe3);
    exp += HTTP::strfy("%8g");
    exp.push_back(0x82);
    const HTTP::byte_string ans = ParserHelper::decode_pct_encoded(str);
    EXPECT_EQ(exp, ans);
}

TEST(parser_helper_decode_pct_encoded, natural_google_search) {
    const HTTP::byte_string str = HTTP::strfy(
        "https://www.google.com/search?client=firefox-b-d&q=%E3%82%AB%E3%82%BF%E3%83%A9%E3%83%B3%E6%95%B0");
    const HTTP::byte_string exp = HTTP::strfy("https://www.google.com/search?client=firefox-b-d&q=カタラン数");
    const HTTP::byte_string ans = ParserHelper::decode_pct_encoded(str);
    EXPECT_EQ(exp, ans);
}

TEST(parser_helper_decode_pct_encoded, natural_wikipedia) {
    const HTTP::byte_string str = HTTP::strfy("https://ja.wikipedia.org/wiki/%E5%9C%8F_(%E6%95%B0%E5%AD%A6)");
    const HTTP::byte_string exp = HTTP::strfy("https://ja.wikipedia.org/wiki/圏_(数学)");
    const HTTP::byte_string ans = ParserHelper::decode_pct_encoded(str);
    EXPECT_EQ(exp, ans);
}
