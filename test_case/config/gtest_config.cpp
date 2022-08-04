#include "Config.h"
#include "gtest/gtest.h"
#include "Lexer.h"
#include "Token.h"
#include "TokenList.h"


TEST(tokenize, string_double_quote) {
    auto bs(HTTP::strfy("\"string\""));
    Lexer lexer(bs);
    auto token_list = lexer.tokenize();
    auto token = token_list.get();
    ASSERT_EQ(token.get_str(), "string");
    ASSERT_EQ(token.get_kind(), Token::STRING);
    ASSERT_EQ(token_list.is_end(), true);
}

TEST(tokenize, string_single_quote) {
    auto bs(HTTP::strfy("'string'"));
    Lexer lexer(bs);
    auto token_list = lexer.tokenize();
    auto token = token_list.get();
    ASSERT_EQ(token.get_str(), "string");
    ASSERT_EQ(token.get_kind(), Token::STRING);
    ASSERT_EQ(token_list.is_end(), true);
}

TEST(tokenize, left_brace) {
    auto bs(HTTP::strfy("{"));
    Lexer lexer(bs);
    auto token_list = lexer.tokenize();
    auto token = token_list.get();
    ASSERT_EQ(token.get_str(), "{");
    ASSERT_EQ(token.get_kind(), Token::LEFT_BRACE);
    ASSERT_EQ(token_list.is_end(), true);
}

TEST(tokenize, right_brace) {
    auto bs(HTTP::strfy("}"));
    Lexer lexer(bs);
    auto token_list = lexer.tokenize();
    auto token = token_list.get();
    ASSERT_EQ(token.get_str(), "}");
    ASSERT_EQ(token.get_kind(), Token::RIGHT_BRACE);
    ASSERT_EQ(token_list.is_end(), true);
}

TEST(tokenize, semi_colon) {
    auto bs(HTTP::strfy(";"));
    Lexer lexer(bs);
    auto token_list = lexer.tokenize();
    auto token = token_list.get();
    ASSERT_EQ(token.get_str(), ";");
    ASSERT_EQ(token.get_kind(), Token::SEMI_COLON);
    ASSERT_EQ(token_list.is_end(), true);
}

TEST(tokenize, eof) {
    auto bs(HTTP::strfy(""));
    Lexer lexer(bs);
    auto token_list = lexer.tokenize();
    auto token = token_list.get();
    ASSERT_EQ(token.get_str(), "");
    ASSERT_EQ(token.get_kind(), Token::END);
}

TEST(tokenize, sample) {
        auto bs(HTTP::strfy("# curl localhost:80 \nhttp {server {listen 80;server_name \"server1\";location / {root /data/server1/;index index.html index.htm;}}}"));
        Lexer lexer(bs);
        auto token_list = lexer.tokenize();
        ASSERT_EQ(token_list.get().get_str(), "http");
        ASSERT_EQ(token_list.get().get_str(), "{");
        ASSERT_EQ(token_list.get().get_str(), "server");
        ASSERT_EQ(token_list.get().get_str(), "{");
        ASSERT_EQ(token_list.get().get_str(), "listen");
        ASSERT_EQ(token_list.get().get_str(), "80");
        ASSERT_EQ(token_list.get().get_str(), ";");
        ASSERT_EQ(token_list.get().get_str(), "server_name");
        ASSERT_EQ(token_list.get().get_str(), "server1");
        ASSERT_EQ(token_list.get().get_str(), ";");
        ASSERT_EQ(token_list.get().get_str(), "location");
        ASSERT_EQ(token_list.get().get_str(), "/");
        ASSERT_EQ(token_list.get().get_str(), "{");
        ASSERT_EQ(token_list.get().get_str(), "root");
        ASSERT_EQ(token_list.get().get_str(), "/data/server1/");
        ASSERT_EQ(token_list.get().get_str(), ";");
        ASSERT_EQ(token_list.get().get_str(), "index");
        ASSERT_EQ(token_list.get().get_str(), "index.html");
        ASSERT_EQ(token_list.get().get_str(), "index.htm");
        ASSERT_EQ(token_list.get().get_str(), ";");
        ASSERT_EQ(token_list.get().get_str(), "}");
        ASSERT_EQ(token_list.get().get_str(), "}");
        ASSERT_EQ(token_list.get().get_str(), "}");
        ASSERT_EQ(token_list.is_end(), true);
}
