cmake_minimum_required(VERSION 3.23)
project(tests)
add_subdirectory(lib)

add_executable(tests gtest_sample.cpp)

target_link_libraries(tests gtest gtest_main)