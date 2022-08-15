#include "../../src/utils/LRUCache.hpp"
#include "gtest/gtest.h"
#include <iostream>
#include <vector>

namespace {

TEST(LRU_cache_test, exists_test) {
    LRUCache<std::string, int> cacher(3);
    cacher.add("a", 1);
    cacher.add("b", 2);
    cacher.add("c", 3);
    EXPECT_TRUE(cacher.exists("a"));
    EXPECT_TRUE(cacher.exists("b"));
    EXPECT_TRUE(cacher.exists("c"));
    EXPECT_FALSE(cacher.exists("not_found"));
}

TEST(LRU_cache_test, fetch_test) {
    LRUCache<std::string, int> cacher(2);
    cacher.add("a", 1);
    cacher.add("b", 2);

    // cache [a, b]
    EXPECT_TRUE(cacher.fetch(" ") == cacher.end());
    {
        LRUCache<std::string, int>::const_iterator res = cacher.fetch("a");
        EXPECT_EQ("a", res->first);
        EXPECT_EQ(1, res->second);
    }
    {
        LRUCache<std::string, int>::const_iterator res = cacher.fetch("b");
        EXPECT_EQ("b", res->first);
        EXPECT_EQ(2, res->second);
    }

    // cache [a, b] -> [b, c]
    cacher.add("c", 3);
    EXPECT_TRUE(cacher.fetch("a") == cacher.end());
    {
        LRUCache<std::string, int>::const_iterator res = cacher.fetch("b");
        EXPECT_EQ("b", res->first);
        EXPECT_EQ(2, res->second);
    }
    {
        LRUCache<std::string, int>::const_iterator res = cacher.fetch("c");
        EXPECT_EQ("c", res->first);
        EXPECT_EQ(3, res->second);
    }

    // cache [a, b] -> [c, d]
    cacher.add("d", 4);
    EXPECT_TRUE(cacher.fetch("a") == cacher.end());
    EXPECT_TRUE(cacher.fetch("b") == cacher.end());
    {
        LRUCache<std::string, int>::const_iterator res = cacher.fetch("c");
        EXPECT_EQ("c", res->first);
        EXPECT_EQ(3, res->second);
    }
    {
        LRUCache<std::string, int>::const_iterator res = cacher.fetch("d");
        EXPECT_EQ("d", res->first);
        EXPECT_EQ(4, res->second);
    }
}

TEST(LRU_cache_test, erase_test) {
    LRUCache<std::string, int> cacher(3);
    cacher.add("a", 1);
    cacher.add("b", 2);
    cacher.add("c", 3);
    EXPECT_TRUE(cacher.exists("a"));
    EXPECT_TRUE(cacher.exists("b"));
    EXPECT_TRUE(cacher.exists("c"));

    cacher.erase("a");
    EXPECT_FALSE(cacher.exists("a"));
    cacher.erase("b");
    EXPECT_FALSE(cacher.exists("b"));
    cacher.erase("c");
    EXPECT_FALSE(cacher.exists("c"));
}

} // namespace
