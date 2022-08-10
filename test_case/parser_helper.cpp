#include "../../src/communication/ParserHelper.hpp"
#include "gtest/gtest.h"
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
    const HTTP::byte_string str       = HTTP::strfy("0");
    std::pair<bool, unsigned int> res = ParserHelper::str_to_u(str);
    EXPECT_EQ(true, res.first);
    EXPECT_EQ(0, res.second);
}

TEST(parser_helper_str_to_u, is_1) {
    const HTTP::byte_string str       = HTTP::strfy("1");
    std::pair<bool, unsigned int> res = ParserHelper::str_to_u(str);
    EXPECT_EQ(true, res.first);
    EXPECT_EQ(1, res.second);
}

TEST(parser_helper_str_to_u, is_UINT_MAX_less_1) {
    const HTTP::byte_string str       = HTTP::strfy("4294967294");
    std::pair<bool, unsigned int> res = ParserHelper::str_to_u(str);
    EXPECT_EQ(true, res.first);
    EXPECT_EQ(UINT_MAX - 1, res.second);
}

TEST(parser_helper_str_to_u, is_UINT_MAX) {
    const HTTP::byte_string str       = HTTP::strfy("4294967295");
    std::pair<bool, unsigned int> res = ParserHelper::str_to_u(str);
    EXPECT_EQ(true, res.first);
    EXPECT_EQ(UINT_MAX, res.second);
}

TEST(parser_helper_str_to_u, is_UINT_MAX_with_leading_0) {
    const HTTP::byte_string str       = HTTP::strfy("0000000004294967295");
    std::pair<bool, unsigned int> res = ParserHelper::str_to_u(str);
    EXPECT_EQ(false, res.first);
}

TEST(parser_helper_str_to_u, is_UINT_MAX_plus_1) {
    const HTTP::byte_string str       = HTTP::strfy("4294967296");
    std::pair<bool, unsigned int> res = ParserHelper::str_to_u(str);
    EXPECT_EQ(false, res.first);
}

TEST(parser_helper_str_to_u, is_minus_1) {
    const HTTP::byte_string str       = HTTP::strfy("-1");
    std::pair<bool, unsigned int> res = ParserHelper::str_to_u(str);
    EXPECT_EQ(false, res.first);
}

// [quality_to_u]

// [extract_quoted_or_token]

// [str_to_http_date]

TEST(parser_helper_str_to_http_date, imf_fixdate_ok) {
    const HTTP::byte_string str          = HTTP::strfy("Sun, 06 Nov 1994 08:49:37 GMT");
    std::pair<bool, t_time_epoch_ms> res = ParserHelper::str_to_http_date(str);
    EXPECT_TRUE(res.first);
}

TEST(parser_helper_str_to_http_date, imf_fixdate_ok_unixtime_origin) {
    const HTTP::byte_string str          = HTTP::strfy("Sun, 01 Jan 1970 00:00:00 GMT");
    std::pair<bool, t_time_epoch_ms> res = ParserHelper::str_to_http_date(str);
    EXPECT_TRUE(res.first);
    EXPECT_EQ(0, res.second);
}

TEST(parser_helper_str_to_http_date, imf_fixdate_ok_unixtime_origin_1) {
    const HTTP::byte_string str          = HTTP::strfy("Sun, 01 Jan 1970 00:00:01 GMT");
    std::pair<bool, t_time_epoch_ms> res = ParserHelper::str_to_http_date(str);
    EXPECT_TRUE(res.first);
    EXPECT_EQ(1000, res.second);
}

TEST(parser_helper_str_to_http_date, imf_fixdate_ok_unixtime_now) {
    const HTTP::byte_string str          = HTTP::strfy("Wed, 10 Aug 2022 02:09:46 GMT");
    std::pair<bool, t_time_epoch_ms> res = ParserHelper::str_to_http_date(str);
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
        std::pair<bool, t_time_epoch_ms> res = ParserHelper::str_to_http_date(str);
        EXPECT_FALSE(res.first);
    }
}

TEST(parser_helper_str_to_http_date, asctime_ok1) {
    const HTTP::byte_string str          = HTTP::strfy("Sun Nov  6 08:49:37 1994");
    std::pair<bool, t_time_epoch_ms> res = ParserHelper::str_to_http_date(str);
    EXPECT_TRUE(res.first);
}

TEST(parser_helper_str_to_http_date, asctime_ok_unixtime_origin) {
    const HTTP::byte_string str          = HTTP::strfy("Sun Jan  1 00:00:00 1970");
    std::pair<bool, t_time_epoch_ms> res = ParserHelper::str_to_http_date(str);
    EXPECT_TRUE(res.first);
    EXPECT_EQ(0, res.second);
}

TEST(parser_helper_str_to_http_date, asctime_ok_unixtime_origin_1) {
    const HTTP::byte_string str          = HTTP::strfy("Sun Jan  1 00:00:01 1970");
    std::pair<bool, t_time_epoch_ms> res = ParserHelper::str_to_http_date(str);
    EXPECT_TRUE(res.first);
    EXPECT_EQ(1000, res.second);
}

TEST(parser_helper_str_to_http_date, asctime_ok_unixtime_now) {
    const HTTP::byte_string str          = HTTP::strfy("Wed Aug 10 02:09:46 2022");
    std::pair<bool, t_time_epoch_ms> res = ParserHelper::str_to_http_date(str);
    EXPECT_TRUE(res.first);
    EXPECT_EQ(1660097386000, res.second);
}

TEST(parser_helper_str_to_http_date, asctime_ko) {
    const char *strs[] = {"Sun Nov 006 08:49:37 1994", "Sun Nov  6 08:49:37 1994 GMT", NULL};
    for (int i = 0; strs[i]; ++i) {
        const HTTP::byte_string str          = HTTP::strfy(strs[i]);
        std::pair<bool, t_time_epoch_ms> res = ParserHelper::str_to_http_date(str);
        EXPECT_FALSE(res.first);
    }
}

TEST(parser_helper_str_to_http_date, rfc850_date_ok1) {
    const HTTP::byte_string str          = HTTP::strfy("Sunday, 06-Nov-94 08:49:37 GMT");
    std::pair<bool, t_time_epoch_ms> res = ParserHelper::str_to_http_date(str);
    EXPECT_TRUE(res.first);
}

TEST(parser_helper_str_to_http_date, rfc850_date_ok_unixtime_origin) {
    const HTTP::byte_string str          = HTTP::strfy("Sunday, 01-Jan-70 00:00:00 GMT");
    std::pair<bool, t_time_epoch_ms> res = ParserHelper::str_to_http_date(str);
    EXPECT_TRUE(res.first);
    EXPECT_EQ(0, res.second);
}

TEST(parser_helper_str_to_http_date, rfc850_date_ok_unixtime_origin_1) {
    const HTTP::byte_string str          = HTTP::strfy("Sunday, 01-Jan-70 00:00:01 GMT");
    std::pair<bool, t_time_epoch_ms> res = ParserHelper::str_to_http_date(str);
    EXPECT_TRUE(res.first);
    EXPECT_EQ(1000, res.second);
}

TEST(parser_helper_str_to_http_date, rfc850_date_ok_unixtime_now) {
    const HTTP::byte_string str          = HTTP::strfy("Wednesday, 10-Aug-22 02:09:46 GMT");
    std::pair<bool, t_time_epoch_ms> res = ParserHelper::str_to_http_date(str);
    EXPECT_TRUE(res.first);
    EXPECT_EQ(1660097386000, res.second);
}
