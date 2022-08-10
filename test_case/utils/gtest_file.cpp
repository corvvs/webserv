#include "../../src/utils/File.hpp"
#include "gtest/gtest.h"

TEST(get_directory_name, example) {
    EXPECT_EQ(file::get_directory_name("path/file"), "path/");
    EXPECT_EQ(file::get_directory_name("path/to/file"), "path/to/");
    EXPECT_EQ(file::get_directory_name("file"), "./");
    EXPECT_EQ(file::get_directory_name("/file"), "/");
    EXPECT_EQ(file::get_directory_name("/"), "/");
}