#include "../../src/utils/LightString.hpp"
#include "gtest/gtest.h"

TEST(find_first_of, sample) {
    std::string s = "first";
    HTTP::byte_string bs(HTTP::strfy(s));
    LightString<char> ls(bs);

    EXPECT_EQ(ls.find_first_of("f"), s.find_first_of("f"));
    EXPECT_EQ(ls.find_first_of("i"), s.find_first_of("i"));
    EXPECT_EQ(ls.find_first_of("r"), s.find_first_of("r"));
    EXPECT_EQ(ls.find_first_of("s"), s.find_first_of("s"));
    EXPECT_EQ(ls.find_first_of("t"), s.find_first_of("t"));
    EXPECT_EQ(ls.find_first_of(""), s.find_first_of(""));
}

TEST(find_first_of, pos) {
    std::string s = "first";
    HTTP::byte_string bs(HTTP::strfy(s));
    LightString<char> ls(bs);
    size_t pos = s.size() / 2;

    EXPECT_EQ(ls.find_first_of("f", pos), s.find_first_of("f", pos));
    EXPECT_EQ(ls.find_first_of("i", pos), s.find_first_of("i", pos));
    EXPECT_EQ(ls.find_first_of("r", pos), s.find_first_of("r", pos));
    EXPECT_EQ(ls.find_first_of("s", pos), s.find_first_of("s", pos));
    EXPECT_EQ(ls.find_first_of("t", pos), s.find_first_of("t", pos));
    EXPECT_EQ(ls.find_first_of("", pos), s.find_first_of("", pos));
}

TEST(find_first_of, over_pos) {
    std::string s = "first";
    HTTP::byte_string bs(HTTP::strfy(s));
    LightString<char> ls(bs);
    size_t pos = s.size() * 2;

    EXPECT_EQ(ls.find_first_of("f", pos), s.find_first_of("f", pos));
    EXPECT_EQ(ls.find_first_of("i", pos), s.find_first_of("i", pos));
    EXPECT_EQ(ls.find_first_of("r", pos), s.find_first_of("r", pos));
    EXPECT_EQ(ls.find_first_of("s", pos), s.find_first_of("s", pos));
    EXPECT_EQ(ls.find_first_of("t", pos), s.find_first_of("t", pos));
    EXPECT_EQ(ls.find_first_of(""), s.find_first_of(""));
}

TEST(find_first_of, not_found) {
    std::string s = "first";
    HTTP::byte_string bs(HTTP::strfy(s));
    LightString<char> ls(bs);
    EXPECT_EQ(ls.find_first_of("n"), s.find_first_of("n"));
}

TEST(find_first_of, empty) {
    std::string s;
    HTTP::byte_string bs(HTTP::strfy(s));
    LightString<char> ls(bs);

    EXPECT_EQ(ls.find_first_of("f"), s.find_first_of("f"));
    EXPECT_EQ(ls.find_first_of(""), s.find_first_of(""));
}

TEST(find_first_of, duplicate) {
    std::string s("aa");
    HTTP::byte_string bs(HTTP::strfy(s));
    LightString<char> ls(bs);

    EXPECT_EQ(ls.find_first_of("a"), s.find_first_of("a"));
    EXPECT_EQ(ls.find_first_of(""), s.find_first_of(""));
}

TEST(find_last_of, sample) {
    std::string s = "first";
    HTTP::byte_string bs(HTTP::strfy(s));
    LightString<char> ls(bs);

    EXPECT_EQ(ls.find_last_of("f"), s.find_last_of("f"));
    EXPECT_EQ(ls.find_last_of("i"), s.find_last_of("i"));
    EXPECT_EQ(ls.find_last_of("r"), s.find_last_of("r"));
    EXPECT_EQ(ls.find_last_of("s"), s.find_last_of("s"));
    EXPECT_EQ(ls.find_last_of("t"), s.find_last_of("t"));
    EXPECT_EQ(ls.find_last_of(""), s.find_last_of(""));
}

TEST(find_last_of, pos) {
    std::string s = "first";
    HTTP::byte_string bs(HTTP::strfy(s));
    LightString<char> ls(bs);
    size_t pos = s.size() / 2;

    EXPECT_EQ(ls.find_last_of("f", pos), s.find_last_of("f", pos));
    EXPECT_EQ(ls.find_last_of("i", pos), s.find_last_of("i", pos));
    EXPECT_EQ(ls.find_last_of("r", pos), s.find_last_of("r", pos));
    EXPECT_EQ(ls.find_last_of("s", pos), s.find_last_of("s", pos));
    EXPECT_EQ(ls.find_last_of("t", pos), s.find_last_of("t", pos));
    EXPECT_EQ(ls.find_last_of("", pos), s.find_last_of("", pos));
}

TEST(find_last_of, over_pos) {
    std::string s = "first";
    HTTP::byte_string bs(HTTP::strfy(s));
    LightString<char> ls(bs);
    size_t pos = s.size() * 2;

    EXPECT_EQ(ls.find_last_of("f", pos), s.find_last_of("f", pos));
    EXPECT_EQ(ls.find_last_of("i", pos), s.find_last_of("i", pos));
    EXPECT_EQ(ls.find_last_of("r", pos), s.find_last_of("r", pos));
    EXPECT_EQ(ls.find_last_of("s", pos), s.find_last_of("s", pos));
    EXPECT_EQ(ls.find_last_of("t", pos), s.find_last_of("t", pos));
    EXPECT_EQ(ls.find_last_of("", pos), s.find_last_of("", pos));
}

TEST(find_last_of, not_found) {
    std::string s = "first";
    HTTP::byte_string bs(HTTP::strfy(s));
    LightString<char> ls(bs);
    EXPECT_EQ(ls.find_last_of("n"), s.find_last_of("n"));
}

TEST(find_last_of, empty) {
    std::string s;
    HTTP::byte_string bs(HTTP::strfy(s));
    LightString<char> ls(bs);

    EXPECT_EQ(ls.find_last_of("f"), s.find_last_of("f"));
    EXPECT_EQ(ls.find_last_of(""), s.find_last_of(""));
}

TEST(find_last_of, duplicate) {
    std::string s("aa");
    HTTP::byte_string bs(HTTP::strfy(s));
    LightString<char> ls(bs);

    EXPECT_EQ(ls.find_last_of("a"), s.find_last_of("a"));
    EXPECT_EQ(ls.find_last_of(""), s.find_last_of(""));
}

TEST(find_first_not_of, sample) {
    std::string s = "first";
    HTTP::byte_string bs(HTTP::strfy(s));
    LightString<char> ls(bs);

    EXPECT_EQ(ls.find_first_not_of("f"), s.find_first_not_of("f"));
    EXPECT_EQ(ls.find_first_not_of("i"), s.find_first_not_of("i"));
    EXPECT_EQ(ls.find_first_not_of("r"), s.find_first_not_of("r"));
    EXPECT_EQ(ls.find_first_not_of("s"), s.find_first_not_of("s"));
    EXPECT_EQ(ls.find_first_not_of("t"), s.find_first_not_of("t"));
    EXPECT_EQ(ls.find_first_not_of(""), s.find_first_not_of(""));
}

TEST(find_first_not_of, pos) {
    std::string s = "first";
    HTTP::byte_string bs(HTTP::strfy(s));
    LightString<char> ls(bs);
    size_t pos = s.size() / 2;

    EXPECT_EQ(ls.find_first_not_of("f", pos), s.find_first_not_of("f", pos));
    EXPECT_EQ(ls.find_first_not_of("i", pos), s.find_first_not_of("i", pos));
    EXPECT_EQ(ls.find_first_not_of("r", pos), s.find_first_not_of("r", pos));
    EXPECT_EQ(ls.find_first_not_of("s", pos), s.find_first_not_of("s", pos));
    EXPECT_EQ(ls.find_first_not_of("t", pos), s.find_first_not_of("t", pos));
    EXPECT_EQ(ls.find_first_not_of(""), s.find_first_not_of(""));
}

TEST(find_first_not_of, over_pos) {
    std::string s = "first";
    HTTP::byte_string bs(HTTP::strfy(s));
    LightString<char> ls(bs);
    size_t pos = s.size() * 2;

    EXPECT_EQ(ls.find_first_not_of("f", pos), s.find_first_not_of("f", pos));
    EXPECT_EQ(ls.find_first_not_of("i", pos), s.find_first_not_of("i", pos));
    EXPECT_EQ(ls.find_first_not_of("r", pos), s.find_first_not_of("r", pos));
    EXPECT_EQ(ls.find_first_not_of("s", pos), s.find_first_not_of("s", pos));
    EXPECT_EQ(ls.find_first_not_of("t", pos), s.find_first_not_of("t", pos));
    EXPECT_EQ(ls.find_first_not_of(""), s.find_first_not_of(""));
}

TEST(find_first_not_of, not_found) {
    std::string s = "first";
    HTTP::byte_string bs(HTTP::strfy(s));
    LightString<char> ls(bs);
    EXPECT_EQ(ls.find_first_not_of("n"), s.find_first_not_of("n"));
}

TEST(find_first_not_of, empty) {
    std::string s;
    HTTP::byte_string bs(HTTP::strfy(s));
    LightString<char> ls(bs);

    EXPECT_EQ(ls.find_first_not_of("f"), s.find_first_not_of("f"));
    EXPECT_EQ(ls.find_first_not_of(""), s.find_first_not_of(""));
}

TEST(find_first_not_of, duplicate) {
    std::string s("aa");
    HTTP::byte_string bs(HTTP::strfy(s));
    LightString<char> ls(bs);

    EXPECT_EQ(ls.find_first_not_of("a"), s.find_first_not_of("a"));
    EXPECT_EQ(ls.find_first_not_of(""), s.find_first_not_of(""));
}

TEST(find_last_not_of, sample) {
    std::string s = "first";
    HTTP::byte_string bs(HTTP::strfy(s));
    LightString<char> ls(bs);

    EXPECT_EQ(ls.find_last_not_of("f"), s.find_last_not_of("f"));
    EXPECT_EQ(ls.find_last_not_of("i"), s.find_last_not_of("i"));
    EXPECT_EQ(ls.find_last_not_of("r"), s.find_last_not_of("r"));
    EXPECT_EQ(ls.find_last_not_of("s"), s.find_last_not_of("s"));
    EXPECT_EQ(ls.find_last_not_of("t"), s.find_last_not_of("t"));
    EXPECT_EQ(ls.find_last_not_of(""), s.find_last_not_of(""));
}

TEST(find_last_not_of, pos) {
    std::string s = "first";
    HTTP::byte_string bs(HTTP::strfy(s));
    LightString<char> ls(bs);
    size_t pos = s.size() / 2;

    EXPECT_EQ(ls.find_last_not_of("f", pos), s.find_last_not_of("f", pos));
    EXPECT_EQ(ls.find_last_not_of("i", pos), s.find_last_not_of("i", pos));
    EXPECT_EQ(ls.find_last_not_of("r", pos), s.find_last_not_of("r", pos));
    EXPECT_EQ(ls.find_last_not_of("s", pos), s.find_last_not_of("s", pos));
    EXPECT_EQ(ls.find_last_not_of("t", pos), s.find_last_not_of("t", pos));
    EXPECT_EQ(ls.find_last_not_of(""), s.find_last_not_of(""));
}

TEST(find_last_not_of, over_pos) {
    std::string s = "first";
    HTTP::byte_string bs(HTTP::strfy(s));
    LightString<char> ls(bs);
    size_t pos = s.size() * 2;

    EXPECT_EQ(ls.find_last_not_of("f", pos), s.find_last_not_of("f", pos));
    EXPECT_EQ(ls.find_last_not_of("i", pos), s.find_last_not_of("i", pos));
    EXPECT_EQ(ls.find_last_not_of("r", pos), s.find_last_not_of("r", pos));
    EXPECT_EQ(ls.find_last_not_of("s", pos), s.find_last_not_of("s", pos));
    EXPECT_EQ(ls.find_last_not_of("t", pos), s.find_last_not_of("t", pos));
    EXPECT_EQ(ls.find_last_not_of(""), s.find_last_not_of(""));
}

TEST(find_last_not_of, not_found) {
    std::string s = "first";
    HTTP::byte_string bs(HTTP::strfy(s));
    LightString<char> ls(bs);
    EXPECT_EQ(ls.find_last_not_of("n"), s.find_last_not_of("n"));
}

TEST(find_last_not_of, empty) {
    std::string s;
    HTTP::byte_string bs(HTTP::strfy(s));
    LightString<char> ls(bs);

    EXPECT_EQ(ls.find_last_not_of("f"), s.find_last_not_of("f"));
    EXPECT_EQ(ls.find_last_not_of(""), s.find_last_not_of(""));
}

TEST(find_last_not_of, duplicate) {
    std::string s("aa");
    HTTP::byte_string bs(HTTP::strfy(s));
    LightString<char> ls(bs);

    EXPECT_EQ(ls.find_last_not_of("a"), s.find_last_not_of("a"));
    EXPECT_EQ(ls.find_last_not_of(""), s.find_last_not_of(""));
}

TEST(find, sample) {
    std::string s = "first";
    HTTP::byte_string bs(HTTP::strfy(s));
    LightString<char> ls(bs);

    EXPECT_EQ(ls.find("f"), s.find("f"));
    EXPECT_EQ(ls.find("i"), s.find("i"));
    EXPECT_EQ(ls.find("r"), s.find("r"));
    EXPECT_EQ(ls.find("s"), s.find("s"));
    EXPECT_EQ(ls.find("t"), s.find("t"));
    EXPECT_EQ(ls.find(""), s.find(""));
}

TEST(find, pos) {
    std::string s = "first";
    HTTP::byte_string bs(HTTP::strfy(s));
    LightString<char> ls(bs);
    size_t pos = s.size() / 2;

    EXPECT_EQ(ls.find("f", pos), s.find("f", pos));
    EXPECT_EQ(ls.find("i", pos), s.find("i", pos));
    EXPECT_EQ(ls.find("r", pos), s.find("r", pos));
    EXPECT_EQ(ls.find("s", pos), s.find("s", pos));
    EXPECT_EQ(ls.find("t", pos), s.find("t", pos));
    EXPECT_EQ(ls.find(""), s.find(""));
}

TEST(find, over_pos) {
    std::string s = "first";
    HTTP::byte_string bs(HTTP::strfy(s));
    LightString<char> ls(bs);
    size_t pos = s.size() * 2;

    EXPECT_EQ(ls.find("f", pos), s.find("f", pos));
    EXPECT_EQ(ls.find("i", pos), s.find("i", pos));
    EXPECT_EQ(ls.find("r", pos), s.find("r", pos));
    EXPECT_EQ(ls.find("s", pos), s.find("s", pos));
    EXPECT_EQ(ls.find("t", pos), s.find("t", pos));
    EXPECT_EQ(ls.find(""), s.find(""));
}

TEST(find, not_found) {
    std::string s = "first";
    HTTP::byte_string bs(HTTP::strfy(s));
    LightString<char> ls(bs);
    EXPECT_EQ(ls.find("n"), s.find("n"));
}

TEST(find, empty) {
    std::string s;
    HTTP::byte_string bs(HTTP::strfy(s));
    LightString<char> ls(bs);

    EXPECT_EQ(ls.find("f"), s.find("f"));
    EXPECT_EQ(ls.find(""), s.find(""));
}

TEST(find, duplicate) {
    std::string s("aa");
    HTTP::byte_string bs(HTTP::strfy(s));
    LightString<char> ls(bs);

    EXPECT_EQ(ls.find("a"), s.find("a"));
    EXPECT_EQ(ls.find(""), s.find(""));
}

TEST(rfind, sample) {
    std::string s = "first";
    HTTP::byte_string bs(HTTP::strfy(s));
    LightString<char> ls(bs);

    EXPECT_EQ(ls.rfind("f"), s.rfind("f"));
    EXPECT_EQ(ls.rfind("i"), s.rfind("i"));
    EXPECT_EQ(ls.rfind("r"), s.rfind("r"));
    EXPECT_EQ(ls.rfind("s"), s.rfind("s"));
    EXPECT_EQ(ls.rfind("t"), s.rfind("t"));
    EXPECT_EQ(ls.rfind(""), s.rfind(""));
}

TEST(rfind, pos) {
    std::string s = "first";
    HTTP::byte_string bs(HTTP::strfy(s));
    LightString<char> ls(bs);
    size_t pos = s.size() / 2;

    EXPECT_EQ(ls.rfind("f", pos), s.rfind("f", pos));
    EXPECT_EQ(ls.rfind("i", pos), s.rfind("i", pos));
    EXPECT_EQ(ls.rfind("r", pos), s.rfind("r", pos));
    EXPECT_EQ(ls.rfind("s", pos), s.rfind("s", pos));
    EXPECT_EQ(ls.rfind("t", pos), s.rfind("t", pos));
    EXPECT_EQ(ls.rfind(""), s.rfind(""));
}

TEST(rfind, over_pos) {
    std::string s = "first";
    HTTP::byte_string bs(HTTP::strfy(s));
    LightString<char> ls(bs);
    size_t pos = s.size() * 2;

    EXPECT_EQ(ls.rfind("f", pos), s.rfind("f", pos));
    EXPECT_EQ(ls.rfind("i", pos), s.rfind("i", pos));
    EXPECT_EQ(ls.rfind("r", pos), s.rfind("r", pos));
    EXPECT_EQ(ls.rfind("s", pos), s.rfind("s", pos));
    EXPECT_EQ(ls.rfind("t", pos), s.rfind("t", pos));
    EXPECT_EQ(ls.rfind(""), s.rfind(""));
}

TEST(rfind, not_found) {
    std::string s = "first";
    HTTP::byte_string bs(HTTP::strfy(s));
    LightString<char> ls(bs);
    EXPECT_EQ(ls.rfind("n"), s.rfind("n"));
}

TEST(rfind, empty) {
    std::string s;
    HTTP::byte_string bs(HTTP::strfy(s));
    LightString<char> ls(bs);

    EXPECT_EQ(ls.rfind("f"), s.rfind("f"));
    EXPECT_EQ(ls.rfind(""), s.rfind(""));
}

TEST(rfind, duplicate) {
    std::string s("aa");
    HTTP::byte_string bs(HTTP::strfy(s));
    LightString<char> ls(bs);

    EXPECT_EQ(ls.rfind("a"), s.rfind("a"));
    EXPECT_EQ(ls.rfind(""), s.rfind(""));
}

TEST(substr, sample) {
    std::string s = "first";
    HTTP::byte_string bs(HTTP::strfy(s));
    LightString<char> ls(bs);

    for (size_t i = 0; i < s.size(); i++) {
        EXPECT_EQ(ls.substr(i), s.substr(i));
    }
}

TEST(substr, pos) {
    std::string s = "first";
    HTTP::byte_string bs(HTTP::strfy(s));
    LightString<char> ls(bs);
    size_t pos = s.size() / 2;

    EXPECT_EQ(ls.substr(pos, 0), "");
    EXPECT_EQ(ls.substr(pos, 1), "r");
    EXPECT_EQ(ls.substr(pos, 2), "rs");
    EXPECT_EQ(ls.substr(pos, 3), "rst");
    EXPECT_EQ(ls.substr(pos, 4), "rst");
}

TEST(substr, over_pos) {
    std::string s = "first";
    HTTP::byte_string bs(HTTP::strfy(s));
    LightString<char> ls(bs);
    size_t pos = s.size() * 2;

    for (size_t i = 0; i < s.size(); i++) {
        EXPECT_EQ(ls.substr(pos, i), "");
    }
}

TEST(substr, empty) {
    std::string s;
    HTTP::byte_string bs(HTTP::strfy(s));
    LightString<char> ls(bs);

    for (size_t i = 0; i < 10; i++) {
        EXPECT_EQ(ls.substr(i), s.substr(i));
    }
}

TEST(substr_while, sample) {
    std::string s = "first";
    HTTP::byte_string bs(HTTP::strfy(s));
    LightString<char> ls(bs);

    EXPECT_EQ(ls.substr_while("f"), "f");
    EXPECT_EQ(ls.substr_while("fi"), "fi");
    EXPECT_EQ(ls.substr_while("fir"), "fir");
    EXPECT_EQ(ls.substr_while("firs"), "firs");
    EXPECT_EQ(ls.substr_while("first"), "first");
    EXPECT_EQ(ls.substr_while(""), "");
}

TEST(substr_while, pos) {
    std::string s = "first";
    HTTP::byte_string bs(HTTP::strfy(s));
    LightString<char> ls(bs);
    size_t pos = s.size() / 2;

    EXPECT_EQ(ls.substr_while("f", pos), "");
    EXPECT_EQ(ls.substr_while("fi", pos), "");
    EXPECT_EQ(ls.substr_while("fir", pos), "r");
    EXPECT_EQ(ls.substr_while("firs", pos), "rs");
    EXPECT_EQ(ls.substr_while("first", pos), "rst");
    EXPECT_EQ(ls.substr_while("", pos), "");
}

TEST(substr_while, over_pos) {
    std::string s = "first";
    HTTP::byte_string bs(HTTP::strfy(s));
    LightString<char> ls(bs);
    size_t pos = s.size() * 2;

    EXPECT_EQ(ls.substr_while("f", pos), "");
    EXPECT_EQ(ls.substr_while("fi", pos), "");
    EXPECT_EQ(ls.substr_while("fir", pos), "");
    EXPECT_EQ(ls.substr_while("firs", pos), "");
    EXPECT_EQ(ls.substr_while("first", pos), "");
    EXPECT_EQ(ls.substr_while("", pos), "");
}

TEST(substr_while, not_found) {
    std::string s = "first";
    HTTP::byte_string bs(HTTP::strfy(s));
    LightString<char> ls(bs);
    EXPECT_EQ(ls.substr_while("n"), "");
}

TEST(substr_while, empty) {
    std::string s;
    HTTP::byte_string bs(HTTP::strfy(s));
    LightString<char> ls(bs);

    EXPECT_EQ(ls.substr_while("f"), "");
    EXPECT_EQ(ls.substr_while(""), "");
}

TEST(substr_after, sample) {
    std::string s = "first";
    HTTP::byte_string bs(HTTP::strfy(s));
    LightString<char> ls(bs);

    EXPECT_EQ(ls.substr_after("f"), "irst");
    EXPECT_EQ(ls.substr_after("fi"), "rst");
    EXPECT_EQ(ls.substr_after("fir"), "st");
    EXPECT_EQ(ls.substr_after("firs"), "t");
    EXPECT_EQ(ls.substr_after("first"), "");
    EXPECT_EQ(ls.substr_after(""), "first");
}

TEST(substr_after, pos) {
    std::string s = "first";
    HTTP::byte_string bs(HTTP::strfy(s));
    LightString<char> ls(bs);
    size_t pos = s.size() / 2;

    EXPECT_EQ(ls.substr_after("f", pos), "rst");
    EXPECT_EQ(ls.substr_after("fi", pos), "rst");
    EXPECT_EQ(ls.substr_after("fir", pos), "st");
    EXPECT_EQ(ls.substr_after("firs", pos), "t");
    EXPECT_EQ(ls.substr_after("first", pos), "");
    EXPECT_EQ(ls.substr_after("", pos), "rst");
}

TEST(substr_after, over_pos) {
    std::string s = "first";
    HTTP::byte_string bs(HTTP::strfy(s));
    LightString<char> ls(bs);
    size_t pos = s.size() * 2;

    EXPECT_EQ(ls.substr_after("f", pos), "");
    EXPECT_EQ(ls.substr_after("fi", pos), "");
    EXPECT_EQ(ls.substr_after("fir", pos), "");
    EXPECT_EQ(ls.substr_after("firs", pos), "");
    EXPECT_EQ(ls.substr_after("first", pos), "");
    EXPECT_EQ(ls.substr_after("", pos), "");
}

TEST(substr_after, not_found) {
    std::string s = "first";
    HTTP::byte_string bs(HTTP::strfy(s));
    LightString<char> ls(bs);
    EXPECT_EQ(ls.substr_after("n"), "first");
}

TEST(substr_after, empty) {
    std::string s;
    HTTP::byte_string bs(HTTP::strfy(s));
    LightString<char> ls(bs);

    EXPECT_EQ(ls.substr_after("f"), "");
    EXPECT_EQ(ls.substr_after(""), "");
}

TEST(substr_before, sample) {
    std::string s = "first";
    HTTP::byte_string bs(HTTP::strfy(s));
    LightString<char> ls(bs);

    EXPECT_EQ(ls.substr_before("f"), "");
    EXPECT_EQ(ls.substr_before("fi"), "");
    EXPECT_EQ(ls.substr_before("fir"), "");
    EXPECT_EQ(ls.substr_before("firs"), "");
    EXPECT_EQ(ls.substr_before("first"), "");
    EXPECT_EQ(ls.substr_before(""), "first");
}

TEST(substr_before, pos) {
    std::string s = "first";
    HTTP::byte_string bs(HTTP::strfy(s));
    LightString<char> ls(bs);
    size_t pos = s.size() / 2;

    EXPECT_EQ(ls.substr_before("f", pos), "rst");
    EXPECT_EQ(ls.substr_before("fi", pos), "rst");
    EXPECT_EQ(ls.substr_before("fir", pos), "");
    EXPECT_EQ(ls.substr_before("firs", pos), "");
    EXPECT_EQ(ls.substr_before("first", pos), "");
    EXPECT_EQ(ls.substr_before("", pos), "rst");
}

TEST(substr_before, over_pos) {
    std::string s = "first";
    HTTP::byte_string bs(HTTP::strfy(s));
    LightString<char> ls(bs);
    size_t pos = s.size() * 2;

    EXPECT_EQ(ls.substr_before("f", pos), "");
    EXPECT_EQ(ls.substr_before("fi", pos), "");
    EXPECT_EQ(ls.substr_before("fir", pos), "");
    EXPECT_EQ(ls.substr_before("firs", pos), "");
    EXPECT_EQ(ls.substr_before("first", pos), "");
    EXPECT_EQ(ls.substr_before("", pos), "");
}

TEST(substr_before, not_found) {
    std::string s = "first";
    HTTP::byte_string bs(HTTP::strfy(s));
    LightString<char> ls(bs);
    EXPECT_EQ(ls.substr_before("n"), "first");
}

TEST(substr_before, empty) {
    std::string s;
    HTTP::byte_string bs(HTTP::strfy(s));
    LightString<char> ls(bs);

    EXPECT_EQ(ls.substr_before("f"), "");
    EXPECT_EQ(ls.substr_before(""), "");
}

TEST(substr_from, sample) {
    std::string s = "first";
    HTTP::byte_string bs(HTTP::strfy(s));
    LightString<char> ls(bs);

    EXPECT_EQ(ls.substr_from("f"), "first");
    EXPECT_EQ(ls.substr_from("fi"), "first");
    EXPECT_EQ(ls.substr_from("fir"), "first");
    EXPECT_EQ(ls.substr_from("firs"), "first");
    EXPECT_EQ(ls.substr_from("first"), "first");
    EXPECT_EQ(ls.substr_from(""), "");
}

TEST(substr_from, pos) {
    std::string s = "first";
    HTTP::byte_string bs(HTTP::strfy(s));
    LightString<char> ls(bs);
    size_t pos = s.size() / 2;

    EXPECT_EQ(ls.substr_from("f", pos), "");
    EXPECT_EQ(ls.substr_from("fi", pos), "");
    EXPECT_EQ(ls.substr_from("fir", pos), "rst");
    EXPECT_EQ(ls.substr_from("firs", pos), "rst");
    EXPECT_EQ(ls.substr_from("first", pos), "rst");
    EXPECT_EQ(ls.substr_from("", pos), "");
}

TEST(substr_from, over_pos) {
    std::string s = "first";
    HTTP::byte_string bs(HTTP::strfy(s));
    LightString<char> ls(bs);
    size_t pos = s.size() * 2;

    EXPECT_EQ(ls.substr_from("f", pos), "");
    EXPECT_EQ(ls.substr_from("fi", pos), "");
    EXPECT_EQ(ls.substr_from("fir", pos), "");
    EXPECT_EQ(ls.substr_from("firs", pos), "");
    EXPECT_EQ(ls.substr_from("first", pos), "");
    EXPECT_EQ(ls.substr_from("", pos), "");
}

TEST(substr_from, not_found) {
    std::string s = "first";
    HTTP::byte_string bs(HTTP::strfy(s));
    LightString<char> ls(bs);
    EXPECT_EQ(ls.substr_from("n"), "");
}

TEST(substr_from, empty) {
    std::string s;
    HTTP::byte_string bs(HTTP::strfy(s));
    LightString<char> ls(bs);

    EXPECT_EQ(ls.substr_from("f"), "");
    EXPECT_EQ(ls.substr_from(""), "");
}

TEST(ltrim, sample) {
    std::string s = "first";
    HTTP::byte_string bs(HTTP::strfy(s));
    LightString<char> ls(bs);

    EXPECT_EQ(ls.ltrim("f"), "irst");
    EXPECT_EQ(ls.ltrim("fi"), "rst");
    EXPECT_EQ(ls.ltrim("fir"), "st");
    EXPECT_EQ(ls.ltrim("firs"), "t");
    EXPECT_EQ(ls.ltrim("first"), "");
    EXPECT_EQ(ls.ltrim(""), "first");
}

TEST(ltrim, not_found) {
    std::string s = "first";
    HTTP::byte_string bs(HTTP::strfy(s));
    LightString<char> ls(bs);
    EXPECT_EQ(ls.ltrim("n"), "first");
}

TEST(ltrim, empty) {
    std::string s;
    HTTP::byte_string bs(HTTP::strfy(s));
    LightString<char> ls(bs);

    EXPECT_EQ(ls.ltrim("f"), "");
    EXPECT_EQ(ls.ltrim(""), "");
}

TEST(rtrim, sample) {
    std::string s = "first";
    HTTP::byte_string bs(HTTP::strfy(s));
    LightString<char> ls(bs);

    EXPECT_EQ(ls.rtrim("f"), "first");
    EXPECT_EQ(ls.rtrim("fi"), "first");
    EXPECT_EQ(ls.rtrim("fir"), "first");
    EXPECT_EQ(ls.rtrim("firs"), "first");
    EXPECT_EQ(ls.rtrim("first"), "");
    EXPECT_EQ(ls.rtrim(""), "first");
}

TEST(rtrim, not_found) {
    std::string s = "first";
    HTTP::byte_string bs(HTTP::strfy(s));
    LightString<char> ls(bs);
    EXPECT_EQ(ls.rtrim("n"), "first");
}

TEST(rtrim, empty) {
    std::string s;
    HTTP::byte_string bs(HTTP::strfy(s));
    LightString<char> ls(bs);

    EXPECT_EQ(ls.rtrim("f"), "");
    EXPECT_EQ(ls.rtrim(""), "");
}

TEST(trim, sample) {
    std::string s = "first";
    HTTP::byte_string bs(HTTP::strfy(s));
    LightString<char> ls(bs);

    EXPECT_EQ(ls.trim("f"), "irst");
    EXPECT_EQ(ls.trim("fi"), "rst");
    EXPECT_EQ(ls.trim("fir"), "st");
    EXPECT_EQ(ls.trim("firs"), "t");
    EXPECT_EQ(ls.trim("first"), "");
    EXPECT_EQ(ls.trim(""), "first");
}

TEST(trim, not_found) {
    std::string s = "first";
    HTTP::byte_string bs(HTTP::strfy(s));
    LightString<char> ls(bs);
    EXPECT_EQ(ls.trim("n"), "first");
}

TEST(trim, empty) {
    std::string s;
    HTTP::byte_string bs(HTTP::strfy(s));
    LightString<char> ls(bs);

    EXPECT_EQ(ls.trim("f"), "");
    EXPECT_EQ(ls.trim(""), "");
}

TEST(split, sample) {
    std::string s = "first second third";
    HTTP::byte_string bs(HTTP::strfy(s));
    LightString<char> ls(bs);
    std::vector<LightString<char> > v = ls.split(" ");

    EXPECT_EQ(v[0], "first");
    EXPECT_EQ(v[1], "second");
    EXPECT_EQ(v[2], "third");
}

TEST(split, consecutive_delimiter) {
    std::string s = "first second  third";
    HTTP::byte_string bs(HTTP::strfy(s));
    LightString<char> ls(bs);
    std::vector<LightString<char> > v = ls.split(" ");

    EXPECT_EQ(v[0], "first");
    EXPECT_EQ(v[1], "second");
    EXPECT_EQ(v[2], "");
    EXPECT_EQ(v[3], "third");
}

TEST(split, multiple_delimiter) {
    std::string s = "first second,third";
    HTTP::byte_string bs(HTTP::strfy(s));
    LightString<char> ls(bs);
    std::vector<LightString<char> > v = ls.split(", ");

    EXPECT_EQ(v[0], "first");
    EXPECT_EQ(v[1], "second");
    EXPECT_EQ(v[2], "third");
}

TEST(split, consecutive_multiple_delimiter) {
    std::string s = "first second ,third";
    HTTP::byte_string bs(HTTP::strfy(s));
    LightString<char> ls(bs);
    std::vector<LightString<char> > v = ls.split(" ,");

    EXPECT_EQ(v[0], "first");
    EXPECT_EQ(v[1], "second");
    EXPECT_EQ(v[2], "");
    EXPECT_EQ(v[3], "third");
}
