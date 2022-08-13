#include "../../src/utils/File.hpp"
#include "gtest/gtest.h"

TEST(get_directory_name, example) {
    EXPECT_EQ(file::get_directory_name("path/file"), "path/");
    EXPECT_EQ(file::get_directory_name("path/to/file"), "path/to/");
    EXPECT_EQ(file::get_directory_name("file"), "./");
    EXPECT_EQ(file::get_directory_name("/file"), "/");
    EXPECT_EQ(file::get_directory_name("/"), "/");
}

TEST(test_file_class, is_file) {
    EXPECT_TRUE(file::is_file("./Makefile"));
    EXPECT_TRUE(file::is_file("./error_page/404.html"));
    EXPECT_FALSE(file::is_file("./error_page"));
    EXPECT_FALSE(file::is_file("./file_not_found"));
}

TEST(test_file_class, is_dir) {
    EXPECT_FALSE(file::is_dir("./Makefile"));
    EXPECT_FALSE(file::is_dir("./error_page/404.html"));
    EXPECT_TRUE(file::is_dir("./error_page"));
    EXPECT_FALSE(file::is_dir("./file_not_found"));
}

TEST(test_file_class, is_readable) {
    EXPECT_TRUE(file::is_readable("./Makefile"));
    EXPECT_TRUE(file::is_readable("./error_page/404.html"));
    EXPECT_FALSE(file::is_readable("./error_page"));
    EXPECT_FALSE(file::is_readable("./file_not_found"));
}

TEST(test_file_class, is_writable) {
    EXPECT_TRUE(file::is_writable("./Makefile"));
    EXPECT_TRUE(file::is_writable("./error_page/404.html"));
    EXPECT_FALSE(file::is_writable("./error_page"));
    EXPECT_FALSE(file::is_writable("./file_not_found"));
}

TEST(test_file_class, is_executable) {
    EXPECT_TRUE(file::is_executable("./unit_tester"));
    EXPECT_FALSE(file::is_executable("./error_page/404.html"));
    EXPECT_FALSE(file::is_executable("./error_page"));
    EXPECT_FALSE(file::is_executable("./file_not_found"));
}
