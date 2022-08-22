#include "../../src/config/Parser.hpp"
#include "../../src/config/Validator.hpp"
#include "gtest/gtest.h"
#include <iostream>
#include <vector>

namespace {

TEST(parse_test, valid_case) {
    const std::string data = "\
http { \
    server { \
        listen 80; \
        location / { \
            return 200 'OK'; \
        } \
    } \
} \
";

    config::Parser parser;
    EXPECT_NO_THROW(parser.parse(data));
}

TEST(parse_test, empty) {
    const std::string data = "";

    config::Parser parser;
    EXPECT_THROW(parser.parse(data), config::SyntaxError);
}

// `}` が余分にある
TEST(parse_test, unexpected_brace) {
    const std::string data = "\
http { \
    server { \
        listen 80; \
        server_name server1; \
        location / { \
            root / data / server1 / ; \
            index index.html index.htm; \
        } \
    } \
    } \
} \
";
    config::Parser parser;
    EXPECT_THROW(parser.parse(data), config::SyntaxError);
}

// `}` が足りない
TEST(parse_test, unexpected_eof) {
    const std::string data = "\
http { \
    server { \
        listen 80; \
        server_name server1; \
        location / { \
            root / data / server1 / ; \
            index index.html index.htm; \
        } \
    } \
";
    config::Parser parser;
    EXPECT_THROW(parser.parse(data), config::SyntaxError);
}

// `#` の後ろに文字が存在する場合はコメントとして扱われない
TEST(parse_test, wrong_comment) {
    const std::string data = "\
http { \
    server { \
        li#sten 80; \
        server_name server1; \
        location / { \
            root /data/server1/; \
            index index.html index.htm; \
        } \
    } \
} \
";
    config::Parser parser;
    EXPECT_THROW(parser.parse(data), config::SyntaxError);
}

// `"` の後ろに文字が存在する場合はクォートして扱われない
TEST(parse_test, wrong_quote) {
    const std::string data = "\
http { \
    server { \
        li\"sten 80; \
        server_name server1; \
        location / { \
            root /data/server1/; \
            index index.html index.htm; \
        } \
    } \
} \
";
    config::Parser parser;
    EXPECT_THROW(parser.parse(data), config::SyntaxError);
}

// クォートが閉じられていない
TEST(parse_test, unclosed_literal) {
    const std::string data = "\
http { \
    server { \
        listen 80; \
        server_name \"server1; \
        location / { \
            root /data/server1/; \
            index index.html index.htm; \
        } \
    } \
} \
";
    config::Parser parser;
    EXPECT_THROW(parser.parse(data), config::SyntaxError);
}

// server_nameをhttpコンテキストに設置できない
TEST(parse_test, not_allowed) {
    const std::string data = "\
http { \
    server_name server1; \
    server { \
        listen 80; \
    } \
} \
";
    config::Parser parser;
    EXPECT_THROW(parser.parse(data), config::SyntaxError);
}

// serverディレクティブの後に `{` が続いていない
TEST(parse_test, no_opening) {
    const std::string data = "\
http { \
    server; \
        listen 80; \
        server_name server1; \
} \
";
    config::Parser parser;
    EXPECT_THROW(parser.parse(data), config::SyntaxError);
}

// listenディレクティブが `;` で終端されていない
TEST(parse_test, not_terminated) {
    const std::string data = "\
http { \
    server { \
        listen 80 \
    } \
} \
";
    config::Parser parser;
    EXPECT_THROW(parser.parse(data), config::SyntaxError);
}

// auto_indexの引数が `on` or `off` 以外
TEST(parse_test, not_on_off) {
    const std::string data = "\
http { \
    server { \
        listen 80; \
        server_name server1; \
        location / { \
            autoindex true; \
        } \
    } \
} \
";
    config::Parser parser;
    EXPECT_THROW(parser.parse(data), config::SyntaxError);
}

// rootの引数が多い
TEST(parse_test, invalid_number_of_args) {
    const std::string data = "\
http { \
    server { \
        listen 80; \
        server_name server1; \
        location / { \
            root /data/server1/ /hoge/; \
        } \
    } \
} \
";
    config::Parser parser;
    EXPECT_THROW(parser.parse(data), config::SyntaxError);
}
} // namespace
