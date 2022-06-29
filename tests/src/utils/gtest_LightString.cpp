#include "../../../src/utils/LightString.hpp"
#include "gtest/gtest.h"

TEST(find_first_of, sample) {
    std::string s = "first";
    LightString<char> ls(s);

    EXPECT_EQ(ls.find_first_of("f"), s.find_first_of("f"));
    EXPECT_EQ(ls.find_first_of("i"), s.find_first_of("i"));
    EXPECT_EQ(ls.find_first_of("r"), s.find_first_of("r"));
    EXPECT_EQ(ls.find_first_of("s"), s.find_first_of("s"));
    EXPECT_EQ(ls.find_first_of("t"), s.find_first_of("t"));
    EXPECT_EQ(ls.find_first_of(""), s.find_first_of(""));
}

TEST(find_first_of, pos) {
    std::string s = "first";
    LightString<char> ls(s);
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
    LightString<char> ls(s);
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
    LightString<char> ls(s);
    EXPECT_EQ(ls.find_first_of("n"), s.find_first_of("n"));
}

TEST(find_first_of, empty) {
    std::string s;
    LightString<char> ls(s);

    EXPECT_EQ(ls.find_first_of("f"), s.find_first_of("f"));
    EXPECT_EQ(ls.find_first_of(""), s.find_first_of(""));
}

TEST(find_first_of, duplicate) {
    std::string s("aa");
    LightString<char> ls(s);

    EXPECT_EQ(ls.find_first_of("a"), s.find_first_of("a"));
    EXPECT_EQ(ls.find_first_of(""), s.find_first_of(""));
}

TEST(find_last_of, sample) {
    std::string s = "first";
    LightString<char> ls(s);

    EXPECT_EQ(ls.find_last_of("f"), s.find_last_of("f"));
    EXPECT_EQ(ls.find_last_of("i"), s.find_last_of("i"));
    EXPECT_EQ(ls.find_last_of("r"), s.find_last_of("r"));
    EXPECT_EQ(ls.find_last_of("s"), s.find_last_of("s"));
    EXPECT_EQ(ls.find_last_of("t"), s.find_last_of("t"));
    EXPECT_EQ(ls.find_last_of(""), s.find_last_of(""));
}

TEST(find_last_of, pos) {
    std::string s = "first";
    LightString<char> ls(s);
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
    LightString<char> ls(s);
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
    LightString<char> ls(s);
    EXPECT_EQ(ls.find_last_of("n"), s.find_last_of("n"));
}

TEST(find_last_of, empty) {
    std::string s;
    LightString<char> ls(s);

    EXPECT_EQ(ls.find_last_of("f"), s.find_last_of("f"));
    EXPECT_EQ(ls.find_last_of(""), s.find_last_of(""));
}

TEST(find_last_of, duplicate) {
    std::string s("aa");
    LightString<char> ls(s);

    EXPECT_EQ(ls.find_last_of("a"), s.find_last_of("a"));
    EXPECT_EQ(ls.find_last_of(""), s.find_last_of(""));
}

TEST(find_first_not_of, sample) {
    std::string s = "first";
    LightString<char> ls(s);

    EXPECT_EQ(ls.find_first_not_of("f"), s.find_first_not_of("f"));
    EXPECT_EQ(ls.find_first_not_of("i"), s.find_first_not_of("i"));
    EXPECT_EQ(ls.find_first_not_of("r"), s.find_first_not_of("r"));
    EXPECT_EQ(ls.find_first_not_of("s"), s.find_first_not_of("s"));
    EXPECT_EQ(ls.find_first_not_of("t"), s.find_first_not_of("t"));
    EXPECT_EQ(ls.find_first_not_of(""), s.find_first_not_of(""));
}

TEST(find_first_not_of, pos) {
    std::string s = "first";
    LightString<char> ls(s);
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
    LightString<char> ls(s);
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
    LightString<char> ls(s);
    EXPECT_EQ(ls.find_first_not_of("n"), s.find_first_not_of("n"));
}

TEST(find_first_not_of, empty) {
    std::string s;
    LightString<char> ls(s);

    EXPECT_EQ(ls.find_first_not_of("f"), s.find_first_not_of("f"));
    EXPECT_EQ(ls.find_first_not_of(""), s.find_first_not_of(""));
}

TEST(find_first_not_of, duplicate) {
    std::string s("aa");
    LightString<char> ls(s);

    EXPECT_EQ(ls.find_first_not_of("a"), s.find_first_not_of("a"));
    EXPECT_EQ(ls.find_first_not_of(""), s.find_first_not_of(""));
}

TEST(find_last_not_of, sample) {
    std::string s = "first";
    LightString<char> ls(s);

    EXPECT_EQ(ls.find_last_not_of("f"), s.find_last_not_of("f"));
    EXPECT_EQ(ls.find_last_not_of("i"), s.find_last_not_of("i"));
    EXPECT_EQ(ls.find_last_not_of("r"), s.find_last_not_of("r"));
    EXPECT_EQ(ls.find_last_not_of("s"), s.find_last_not_of("s"));
    EXPECT_EQ(ls.find_last_not_of("t"), s.find_last_not_of("t"));
    EXPECT_EQ(ls.find_last_not_of(""), s.find_last_not_of(""));
}

TEST(find_last_not_of, pos) {
    std::string s = "first";
    LightString<char> ls(s);
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
    LightString<char> ls(s);
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
    LightString<char> ls(s);
    EXPECT_EQ(ls.find_last_not_of("n"), s.find_last_not_of("n"));
}

TEST(find_last_not_of, empty) {
    std::string s;
    LightString<char> ls(s);

    EXPECT_EQ(ls.find_last_not_of("f"), s.find_last_not_of("f"));
    EXPECT_EQ(ls.find_last_not_of(""), s.find_last_not_of(""));
}

TEST(find_last_not_of, duplicate) {
    std::string s("aa");
    LightString<char> ls(s);

    EXPECT_EQ(ls.find_last_not_of("a"), s.find_last_not_of("a"));
    EXPECT_EQ(ls.find_last_not_of(""), s.find_last_not_of(""));
}

TEST(find, sample) {
    std::string s = "first";
    LightString<char> ls(s);

    EXPECT_EQ(ls.find("f"), s.find("f"));
    EXPECT_EQ(ls.find("i"), s.find("i"));
    EXPECT_EQ(ls.find("r"), s.find("r"));
    EXPECT_EQ(ls.find("s"), s.find("s"));
    EXPECT_EQ(ls.find("t"), s.find("t"));
    EXPECT_EQ(ls.find(""), s.find(""));
}

TEST(find, pos) {
    std::string s = "first";
    LightString<char> ls(s);
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
    LightString<char> ls(s);
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
    LightString<char> ls(s);
    EXPECT_EQ(ls.find("n"), s.find("n"));
}

TEST(find, empty) {
    std::string s;
    LightString<char> ls(s);

    EXPECT_EQ(ls.find("f"), s.find("f"));
    EXPECT_EQ(ls.find(""), s.find(""));
}

TEST(find, duplicate) {
    std::string s("aa");
    LightString<char> ls(s);

    EXPECT_EQ(ls.find("a"), s.find("a"));
    EXPECT_EQ(ls.find(""), s.find(""));
}

TEST(rfind, sample) {
    std::string s = "first";
    LightString<char> ls(s);

    EXPECT_EQ(ls.rfind("f"), s.rfind("f"));
    EXPECT_EQ(ls.rfind("i"), s.rfind("i"));
    EXPECT_EQ(ls.rfind("r"), s.rfind("r"));
    EXPECT_EQ(ls.rfind("s"), s.rfind("s"));
    EXPECT_EQ(ls.rfind("t"), s.rfind("t"));
    EXPECT_EQ(ls.rfind(""), s.rfind(""));
}

TEST(rfind, pos) {
    std::string s = "first";
    LightString<char> ls(s);
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
    LightString<char> ls(s);
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
    LightString<char> ls(s);
    EXPECT_EQ(ls.rfind("n"), s.rfind("n"));
}

TEST(rfind, empty) {
    std::string s;
    LightString<char> ls(s);

    EXPECT_EQ(ls.rfind("f"), s.rfind("f"));
    EXPECT_EQ(ls.rfind(""), s.rfind(""));
}

TEST(rfind, duplicate) {
    std::string s("aa");
    LightString<char> ls(s);

    EXPECT_EQ(ls.rfind("a"), s.rfind("a"));
    EXPECT_EQ(ls.rfind(""), s.rfind(""));
}

TEST(substr, sample) {
    std::string s = "first";
    LightString<char> ls(s);

    for (size_t i = 0; i < s.size(); i++) {
        EXPECT_EQ(ls.substr(i).str(), s.substr(i));
    }
}

TEST(substr, pos) {
    std::string s = "first";
    LightString<char> ls(s);
    size_t pos = s.size() / 2;

    EXPECT_EQ(ls.substr(pos, 0).str(), "");
    EXPECT_EQ(ls.substr(pos, 1).str(), "r");
    EXPECT_EQ(ls.substr(pos, 2).str(), "rs");
    EXPECT_EQ(ls.substr(pos, 3).str(), "rst");
    EXPECT_EQ(ls.substr(pos, 4).str(), "rst");
}

TEST(substr, over_pos) {
    std::string s = "first";
    LightString<char> ls(s);
    size_t pos = s.size() * 2;

    for (size_t i = 0; i < s.size(); i++) {
        EXPECT_EQ(ls.substr(pos, i).str(), s.substr(pos, i));
    }
}

TEST(substr, empty) {
    std::string s;
    LightString<char> ls(s);

    for (size_t i = 0; i < 10; i++) {
        EXPECT_EQ(ls.substr(i).str(), s.substr(i));
    }
}

TEST(substr_while, sample) {
    std::string s = "first";
    LightString<char> ls(s);

    EXPECT_EQ(ls.substr_while("f").str(), "f");
    EXPECT_EQ(ls.substr_while("fi").str(), "fi");
    EXPECT_EQ(ls.substr_while("fir").str(), "fir");
    EXPECT_EQ(ls.substr_while("firs").str(), "firs");
    EXPECT_EQ(ls.substr_while("first").str(), "first");
    EXPECT_EQ(ls.substr_while("").str(), "");
}

TEST(substr_while, pos) {
    std::string s = "first";
    LightString<char> ls(s);
    size_t pos = s.size() / 2;

    EXPECT_EQ(ls.substr_while("f", pos).str(), "");
    EXPECT_EQ(ls.substr_while("fi", pos).str(), "");
    EXPECT_EQ(ls.substr_while("fir", pos).str(), "r");
    EXPECT_EQ(ls.substr_while("firs", pos).str(), "rs");
    EXPECT_EQ(ls.substr_while("first", pos).str(), "rst");
    EXPECT_EQ(ls.substr_while("", pos).str(), "");
}

TEST(substr_while, over_pos) {
    std::string s = "first";
    LightString<char> ls(s);
    size_t pos = s.size() * 2;

    EXPECT_EQ(ls.substr_while("f", pos).str(), "");
    EXPECT_EQ(ls.substr_while("fi", pos).str(), "");
    EXPECT_EQ(ls.substr_while("fir", pos).str(), "");
    EXPECT_EQ(ls.substr_while("firs", pos).str(), "");
    EXPECT_EQ(ls.substr_while("first", pos).str(), "");
    EXPECT_EQ(ls.substr_while("", pos).str(), "");
}

TEST(substr_while, not_found) {
    std::string s = "first";
    LightString<char> ls(s);
    EXPECT_EQ(ls.substr_while("n").str(), "");
}

TEST(substr_while, empty) {
    std::string s;
    LightString<char> ls(s);

    EXPECT_EQ(ls.substr_while("f").str(), "");
    EXPECT_EQ(ls.substr_while("").str(), "");
}

TEST(substr_after, sample) {
    std::string s = "first";
    LightString<char> ls(s);

    EXPECT_EQ(ls.substr_after("f").str(), "irst");
    EXPECT_EQ(ls.substr_after("fi").str(), "rst");
    EXPECT_EQ(ls.substr_after("fir").str(), "st");
    EXPECT_EQ(ls.substr_after("firs").str(), "t");
    EXPECT_EQ(ls.substr_after("first").str(), "");
    EXPECT_EQ(ls.substr_after("").str(), "first");
}

TEST(substr_after, pos) {
    std::string s = "first";
    LightString<char> ls(s);
    size_t pos = s.size() / 2;

    EXPECT_EQ(ls.substr_after("f", pos).str(), "rst");
    EXPECT_EQ(ls.substr_after("fi", pos).str(), "rst");
    EXPECT_EQ(ls.substr_after("fir", pos).str(), "st");
    EXPECT_EQ(ls.substr_after("firs", pos).str(), "t");
    EXPECT_EQ(ls.substr_after("first", pos).str(), "");
    EXPECT_EQ(ls.substr_after("", pos).str(), "rst");
}

TEST(substr_after, over_pos) {
    std::string s = "first";
    LightString<char> ls(s);
    size_t pos = s.size() * 2;

    EXPECT_EQ(ls.substr_after("f", pos).str(), "");
    EXPECT_EQ(ls.substr_after("fi", pos).str(), "");
    EXPECT_EQ(ls.substr_after("fir", pos).str(), "");
    EXPECT_EQ(ls.substr_after("firs", pos).str(), "");
    EXPECT_EQ(ls.substr_after("first", pos).str(), "");
    EXPECT_EQ(ls.substr_after("", pos).str(), "");
}

TEST(substr_after, not_found) {
    std::string s = "first";
    LightString<char> ls(s);
    EXPECT_EQ(ls.substr_after("n").str(), "first");
}

TEST(substr_after, empty) {
    std::string s;
    LightString<char> ls(s);

    EXPECT_EQ(ls.substr_after("f").str(), "");
    EXPECT_EQ(ls.substr_after("").str(), "");
}

TEST(substr_before, sample) {
    std::string s = "first";
    LightString<char> ls(s);

    EXPECT_EQ(ls.substr_before("f").str(), "");
    EXPECT_EQ(ls.substr_before("fi").str(), "");
    EXPECT_EQ(ls.substr_before("fir").str(), "");
    EXPECT_EQ(ls.substr_before("firs").str(), "");
    EXPECT_EQ(ls.substr_before("first").str(), "");
    EXPECT_EQ(ls.substr_before("").str(), "first");
}

TEST(substr_before, pos) {
    std::string s = "first";
    LightString<char> ls(s);
    size_t pos = s.size() / 2;

    EXPECT_EQ(ls.substr_before("f", pos).str(), "rst");
    EXPECT_EQ(ls.substr_before("fi", pos).str(), "rst");
    EXPECT_EQ(ls.substr_before("fir", pos).str(), "");
    EXPECT_EQ(ls.substr_before("firs", pos).str(), "");
    EXPECT_EQ(ls.substr_before("first", pos).str(), "");
    EXPECT_EQ(ls.substr_before("", pos).str(), "rst");
}

TEST(substr_before, over_pos) {
    std::string s = "first";
    LightString<char> ls(s);
    size_t pos = s.size() * 2;

    EXPECT_EQ(ls.substr_before("f", pos).str(), "");
    EXPECT_EQ(ls.substr_before("fi", pos).str(), "");
    EXPECT_EQ(ls.substr_before("fir", pos).str(), "");
    EXPECT_EQ(ls.substr_before("firs", pos).str(), "");
    EXPECT_EQ(ls.substr_before("first", pos).str(), "");
    EXPECT_EQ(ls.substr_before("", pos).str(), "");
}

TEST(substr_before, not_found) {
    std::string s = "first";
    LightString<char> ls(s);
    EXPECT_EQ(ls.substr_before("n").str(), "first");
}

TEST(substr_before, empty) {
    std::string s;
    LightString<char> ls(s);

    EXPECT_EQ(ls.substr_before("f").str(), "");
    EXPECT_EQ(ls.substr_before("").str(), "");
}

TEST(substr_from, sample) {
    std::string s = "first";
    LightString<char> ls(s);

    EXPECT_EQ(ls.substr_from("f").str(), "first");
    EXPECT_EQ(ls.substr_from("fi").str(), "first");
    EXPECT_EQ(ls.substr_from("fir").str(), "first");
    EXPECT_EQ(ls.substr_from("firs").str(), "first");
    EXPECT_EQ(ls.substr_from("first").str(), "first");
    EXPECT_EQ(ls.substr_from("").str(), "");
}

TEST(substr_from, pos) {
    std::string s = "first";
    LightString<char> ls(s);
    size_t pos = s.size() / 2;

    EXPECT_EQ(ls.substr_from("f", pos).str(), "");
    EXPECT_EQ(ls.substr_from("fi", pos).str(), "");
    EXPECT_EQ(ls.substr_from("fir", pos).str(), "rst");
    EXPECT_EQ(ls.substr_from("firs", pos).str(), "rst");
    EXPECT_EQ(ls.substr_from("first", pos).str(), "rst");
    EXPECT_EQ(ls.substr_from("", pos).str(), "");
}

TEST(substr_from, over_pos) {
    std::string s = "first";
    LightString<char> ls(s);
    size_t pos = s.size() * 2;

    EXPECT_EQ(ls.substr_from("f", pos).str(), "");
    EXPECT_EQ(ls.substr_from("fi", pos).str(), "");
    EXPECT_EQ(ls.substr_from("fir", pos).str(), "");
    EXPECT_EQ(ls.substr_from("firs", pos).str(), "");
    EXPECT_EQ(ls.substr_from("first", pos).str(), "");
    EXPECT_EQ(ls.substr_from("", pos).str(), "");
}

TEST(substr_from, not_found) {
    std::string s = "first";
    LightString<char> ls(s);
    EXPECT_EQ(ls.substr_from("n").str(), "");
}

TEST(substr_from, empty) {
    std::string s;
    LightString<char> ls(s);

    EXPECT_EQ(ls.substr_from("f").str(), "");
    EXPECT_EQ(ls.substr_from("").str(), "");
}

TEST(ltrim, sample) {
    std::string s = "first";
    LightString<char> ls(s);

    EXPECT_EQ(ls.ltrim("f").str(), "irst");
    EXPECT_EQ(ls.ltrim("fi").str(), "rst");
    EXPECT_EQ(ls.ltrim("fir").str(), "st");
    EXPECT_EQ(ls.ltrim("firs").str(), "t");
    EXPECT_EQ(ls.ltrim("first").str(), "");
    EXPECT_EQ(ls.ltrim("").str(), "first");
}

TEST(ltrim, not_found) {
    std::string s = "first";
    LightString<char> ls(s);
    EXPECT_EQ(ls.ltrim("n").str(), "first");
}

TEST(ltrim, empty) {
    std::string s;
    LightString<char> ls(s);

    EXPECT_EQ(ls.ltrim("f").str(), "");
    EXPECT_EQ(ls.ltrim("").str(), "");
}

TEST(rtrim, sample) {
    std::string s = "first";
    LightString<char> ls(s);

    EXPECT_EQ(ls.rtrim("f").str(), "first");
    EXPECT_EQ(ls.rtrim("fi").str(), "first");
    EXPECT_EQ(ls.rtrim("fir").str(), "first");
    EXPECT_EQ(ls.rtrim("firs").str(), "first");
    EXPECT_EQ(ls.rtrim("first").str(), "");
    EXPECT_EQ(ls.rtrim("").str(), "first");
}

TEST(rtrim, not_found) {
    std::string s = "first";
    LightString<char> ls(s);
    EXPECT_EQ(ls.rtrim("n").str(), "first");
}

TEST(rtrim, empty) {
    std::string s;
    LightString<char> ls(s);

    EXPECT_EQ(ls.rtrim("f").str(), "");
    EXPECT_EQ(ls.rtrim("").str(), "");
}

TEST(trim, sample) {
    std::string s = "first";
    LightString<char> ls(s);

    EXPECT_EQ(ls.trim("f").str(), "irst");
    EXPECT_EQ(ls.trim("fi").str(), "rst");
    EXPECT_EQ(ls.trim("fir").str(), "st");
    EXPECT_EQ(ls.trim("firs").str(), "t");
    EXPECT_EQ(ls.trim("first").str(), "");
    EXPECT_EQ(ls.trim("").str(), "first");
}

TEST(trim, not_found) {
    std::string s = "first";
    LightString<char> ls(s);
    EXPECT_EQ(ls.trim("n").str(), "first");
}

TEST(trim, empty) {
    std::string s;
    LightString<char> ls(s);

    EXPECT_EQ(ls.trim("f").str(), "");
    EXPECT_EQ(ls.trim("").str(), "");
}

TEST(split, sample) {
    std::string s = "hoge hoge hoge";
    LightString<char> ls(s);
    std::vector<LightString<char> > v = ls.split(" ");

    EXPECT_EQ(v[0].str(), "hoge");
    EXPECT_EQ(v[1].str(), "hoge");
    EXPECT_EQ(v[2].str(), "hoge");
}

TEST(split, consecutive_delimiter) {
    std::string s = "hoge hoge  hoge";
    LightString<char> ls(s);
    std::vector<LightString<char> > v = ls.split(" ");

    EXPECT_EQ(v[0].str(), "hoge");
    EXPECT_EQ(v[1].str(), "hoge");
    EXPECT_EQ(v[2].str(), "");
    EXPECT_EQ(v[3].str(), "hoge");
}

TEST(split, multiple_delimiter) {
    std::string s = "hoge hoge,hoge";
    LightString<char> ls(s);
    std::vector<LightString<char> > v = ls.split(", ");

    EXPECT_EQ(v[0].str(), "hoge");
    EXPECT_EQ(v[1].str(), "hoge");
    EXPECT_EQ(v[2].str(), "hoge");
}

TEST(split, consecutive_multiple_delimiter) {
    std::string s = "hoge hoge ,hoge";
    LightString<char> ls(s);
    std::vector<LightString<char> > v = ls.split(" ,");

    EXPECT_EQ(v[0].str(), "hoge");
    EXPECT_EQ(v[1].str(), "hoge");
    EXPECT_EQ(v[2].str(), "");
    EXPECT_EQ(v[3].str(), "hoge");
}
