#include "../../src/utils/UtilsString.hpp"
#include "gtest/gtest.h"
#include <vector>

// [downcase]
TEST(utils_string_downcase, basic_non_destructive) {
    const HTTP::byte_string str
        = HTTP::strfy("RFC 2616 defines the Content-Disposition response header field (Section 19.5.1 of [RFC2616]) "
                      "but points out that it is not part of the HTTP/1.1 Standard (Section 15.5):");
    HTTP::byte_string expected
        = HTTP::strfy("rfc 2616 defines the content-disposition response header field (section 19.5.1 of [rfc2616]) "
                      "but points out that it is not part of the http/1.1 standard (section 15.5):");
    HTTP::byte_string str2   = str;
    HTTP::byte_string downed = HTTP::Utils::downcase(str);
    EXPECT_EQ(str2, str);
    EXPECT_EQ(expected, downed);
}

TEST(utils_string_downcase, basic_destructive) {
    HTTP::byte_string str
        = HTTP::strfy("RFC 2616 defines the Content-Disposition response header field (Section 19.5.1 of [RFC2616]) "
                      "but points out that it is not part of the HTTP/1.1 Standard (Section 15.5):");
    HTTP::byte_string expected
        = HTTP::strfy("rfc 2616 defines the content-disposition response header field (section 19.5.1 of [rfc2616]) "
                      "but points out that it is not part of the http/1.1 standard (section 15.5):");
    HTTP::byte_string str2 = str;
    HTTP::Utils::downcase(str);
    EXPECT_NE(str2, str);
    EXPECT_EQ(expected, str);
}

// [upcase(non-destructive)]

TEST(utils_string_upcase, basic_non_destructive) {
    const HTTP::byte_string str
        = HTTP::strfy("RFC 2616 defines the Content-Disposition response header field (Section 19.5.1 of [RFC2616]) "
                      "but points out that it is not part of the HTTP/1.1 Standard (Section 15.5):");
    HTTP::byte_string expected
        = HTTP::strfy("RFC 2616 DEFINES THE CONTENT-DISPOSITION RESPONSE HEADER FIELD (SECTION 19.5.1 OF [RFC2616]) "
                      "BUT POINTS OUT THAT IT IS NOT PART OF THE HTTP/1.1 STANDARD (SECTION 15.5):");
    HTTP::byte_string str2  = str;
    HTTP::byte_string upped = HTTP::Utils::upcase(str);
    EXPECT_EQ(str2, str);
    EXPECT_EQ(expected, upped);
}

// [upcase(destructive)]

TEST(utils_string_upcase, basic_destructive) {
    HTTP::byte_string str
        = HTTP::strfy("RFC 2616 defines the Content-Disposition response header field (Section 19.5.1 of [RFC2616]) "
                      "but points out that it is not part of the HTTP/1.1 Standard (Section 15.5):");
    HTTP::byte_string expected
        = HTTP::strfy("RFC 2616 DEFINES THE CONTENT-DISPOSITION RESPONSE HEADER FIELD (SECTION 19.5.1 OF [RFC2616]) "
                      "BUT POINTS OUT THAT IT IS NOT PART OF THE HTTP/1.1 STANDARD (SECTION 15.5):");
    HTTP::byte_string str2 = str;
    HTTP::Utils::upcase(str);
    EXPECT_NE(str2, str);
    EXPECT_EQ(expected, str);
}

// [normalize_cgi_metavar_key]
TEST(utils_string_normalize_cgi_metavar_key, basic) {
    HTTP::byte_string str;
    str = HTTP::strfy("Via");
    HTTP::Utils::normalize_cgi_metavar_key(str);
    EXPECT_EQ(HTTP::strfy("VIA"), str);
    str = HTTP::strfy("v-i-a");
    HTTP::Utils::normalize_cgi_metavar_key(str);
    EXPECT_EQ(HTTP::strfy("V_I_A"), str);
    str = HTTP::strfy("-----");
    HTTP::Utils::normalize_cgi_metavar_key(str);
    EXPECT_EQ(HTTP::strfy("_____"), str);
    str = HTTP::strfy("Content-Type");
    HTTP::Utils::normalize_cgi_metavar_key(str);
    EXPECT_EQ(HTTP::strfy("CONTENT_TYPE"), str);
    str = HTTP::strfy("CONTENT_TYPE");
    HTTP::Utils::normalize_cgi_metavar_key(str);
    EXPECT_EQ(HTTP::strfy("CONTENT_TYPE"), str);
    str = HTTP::strfy("CONTENT-TYPE");
    HTTP::Utils::normalize_cgi_metavar_key(str);
    EXPECT_EQ(HTTP::strfy("CONTENT_TYPE"), str);
}

// [join_path]

TEST(utils_string_join_path, basic) {
    EXPECT_EQ(HTTP::strfy("./apple.txt"), HTTP::Utils::join_path(HTTP::strfy("./"), HTTP::strfy("/apple.txt")));
    EXPECT_EQ(HTTP::strfy("./apple.txt"), HTTP::Utils::join_path(HTTP::strfy("./"), HTTP::strfy("apple.txt")));
    EXPECT_EQ(HTTP::strfy("./apple.txt"), HTTP::Utils::join_path(HTTP::strfy("."), HTTP::strfy("/apple.txt")));
    EXPECT_EQ(HTTP::strfy("./apple.txt"), HTTP::Utils::join_path(HTTP::strfy("."), HTTP::strfy("apple.txt")));
    EXPECT_EQ(HTTP::strfy("/apple.txt"), HTTP::Utils::join_path(HTTP::strfy(""), HTTP::strfy("apple.txt")));
    EXPECT_EQ(HTTP::strfy("/apple.txt"), HTTP::Utils::join_path(HTTP::strfy(""), HTTP::strfy("/apple.txt")));
    EXPECT_EQ(HTTP::strfy("/apple.txt"), HTTP::Utils::join_path(HTTP::strfy("/"), HTTP::strfy("apple.txt")));
    EXPECT_EQ(HTTP::strfy("/apple.txt"), HTTP::Utils::join_path(HTTP::strfy("/"), HTTP::strfy("/apple.txt")));
}

TEST(utils_string_join_path, serial) {
    EXPECT_EQ(HTTP::strfy("/apple.txt"), HTTP::Utils::join_path(HTTP::strfy("//////"), HTTP::strfy("apple.txt")));
    EXPECT_EQ(HTTP::strfy("/apple.txt"), HTTP::Utils::join_path(HTTP::strfy("//////"), HTTP::strfy("/////apple.txt")));
    EXPECT_EQ(HTTP::strfy("/apple.txt"), HTTP::Utils::join_path(HTTP::strfy(""), HTTP::strfy("/////apple.txt")));
}

TEST(utils_string_join_path, extreme) {
    EXPECT_EQ(HTTP::strfy("/"), HTTP::Utils::join_path(HTTP::strfy(""), HTTP::strfy("/////")));
    EXPECT_EQ(HTTP::strfy("/"), HTTP::Utils::join_path(HTTP::strfy(""), HTTP::strfy("")));
    EXPECT_EQ(HTTP::strfy("/"), HTTP::Utils::join_path(HTTP::strfy("/////"), HTTP::strfy("")));
    EXPECT_EQ(HTTP::strfy("/"), HTTP::Utils::join_path(HTTP::strfy("/////"), HTTP::strfy("/")));
}

// [is_relative_path]
TEST(utils_is_relative_path, is_true) {
    EXPECT_TRUE(HTTP::Utils::is_relative_path(HTTP::strfy("./")));
    EXPECT_TRUE(HTTP::Utils::is_relative_path(HTTP::strfy("hello")));
    EXPECT_TRUE(HTTP::Utils::is_relative_path(HTTP::strfy("%")));
    EXPECT_TRUE(HTTP::Utils::is_relative_path(HTTP::strfy("../extra.cgi")));
}

TEST(utils_is_relative_path, is_false) {
    EXPECT_FALSE(HTTP::Utils::is_relative_path(HTTP::strfy("/")));
    EXPECT_FALSE(HTTP::Utils::is_relative_path(HTTP::strfy("//")));
    EXPECT_FALSE(HTTP::Utils::is_relative_path(HTTP::strfy("/usr/bin/cat")));
    EXPECT_FALSE(HTTP::Utils::is_relative_path(HTTP::strfy("/usr/bin/cat/")));
}

// [basename]
TEST(utils_basenane, basic) {
    EXPECT_EQ(HTTP::light_string(HTTP::strfy(".")), HTTP::Utils::basename(HTTP::strfy("./")));
    EXPECT_EQ(HTTP::light_string(HTTP::strfy("hello")), HTTP::Utils::basename(HTTP::strfy("hello")));
    EXPECT_EQ(HTTP::light_string(HTTP::strfy("")), HTTP::Utils::basename(HTTP::strfy("")));
    EXPECT_EQ(HTTP::light_string(HTTP::strfy("")), HTTP::Utils::basename(HTTP::strfy("/")));
    EXPECT_EQ(HTTP::light_string(HTTP::strfy("")), HTTP::Utils::basename(HTTP::strfy("///////")));
    EXPECT_EQ(HTTP::light_string(HTTP::strfy("warhead")), HTTP::Utils::basename(HTTP::strfy("/w/warhead/")));
    EXPECT_EQ(HTTP::light_string(HTTP::strfy("warhead")), HTTP::Utils::basename(HTTP::strfy("warhead/")));
    EXPECT_EQ(HTTP::light_string(HTTP::strfy("..")), HTTP::Utils::basename(HTTP::strfy("/warhead/../////")));
}
