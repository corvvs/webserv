#include "../../src/utils/UtilsString.hpp"
#include "gtest/gtest.h"
#include <vector>

// [downcase]

// [upcase(non-destructive)]

// [upcase(destructive)]

// [normalize_cgi_metavar_key]

// [join_path]

TEST(utils_string_join_path, basic) {
    EXPECT_EQ(HTTP::strfy("./apple.txt"),
        HTTP::Utils::join_path(HTTP::strfy("./"), HTTP::strfy("/apple.txt")));
    EXPECT_EQ(HTTP::strfy("./apple.txt"),
        HTTP::Utils::join_path(HTTP::strfy("./"), HTTP::strfy("apple.txt")));
    EXPECT_EQ(HTTP::strfy("./apple.txt"),
        HTTP::Utils::join_path(HTTP::strfy("."), HTTP::strfy("/apple.txt")));
    EXPECT_EQ(HTTP::strfy("./apple.txt"),
        HTTP::Utils::join_path(HTTP::strfy("."), HTTP::strfy("apple.txt")));
    EXPECT_EQ(HTTP::strfy("/apple.txt"),
        HTTP::Utils::join_path(HTTP::strfy(""), HTTP::strfy("apple.txt")));
    EXPECT_EQ(HTTP::strfy("/apple.txt"),
        HTTP::Utils::join_path(HTTP::strfy(""), HTTP::strfy("/apple.txt")));
    EXPECT_EQ(HTTP::strfy("/apple.txt"),
        HTTP::Utils::join_path(HTTP::strfy("/"), HTTP::strfy("apple.txt")));
    EXPECT_EQ(HTTP::strfy("/apple.txt"),
        HTTP::Utils::join_path(HTTP::strfy("/"), HTTP::strfy("/apple.txt")));
}

TEST(utils_string_join_path, serial) {
    EXPECT_EQ(HTTP::strfy("/apple.txt"),
        HTTP::Utils::join_path(HTTP::strfy("//////"), HTTP::strfy("apple.txt")));
    EXPECT_EQ(HTTP::strfy("/apple.txt"),
        HTTP::Utils::join_path(HTTP::strfy("//////"), HTTP::strfy("/////apple.txt")));
    EXPECT_EQ(HTTP::strfy("/apple.txt"),
        HTTP::Utils::join_path(HTTP::strfy(""), HTTP::strfy("/////apple.txt")));
}

TEST(utils_string_join_path, extreme) {
    EXPECT_EQ(HTTP::strfy("/"),
        HTTP::Utils::join_path(HTTP::strfy(""), HTTP::strfy("/////")));
    EXPECT_EQ(HTTP::strfy("/"),
        HTTP::Utils::join_path(HTTP::strfy(""), HTTP::strfy("")));
    EXPECT_EQ(HTTP::strfy("/"),
        HTTP::Utils::join_path(HTTP::strfy("/////"), HTTP::strfy("")));
    EXPECT_EQ(HTTP::strfy("/"),
        HTTP::Utils::join_path(HTTP::strfy("/////"), HTTP::strfy("/")));
}
