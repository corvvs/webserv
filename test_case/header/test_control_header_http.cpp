#include "../../src/communication/ControlHeaderHTTP.hpp"
#include "gtest/gtest.h"

TEST(control_header_http, content_type_basic_ok) {
    const char *strs[] = {"text/plain",
                          "text/html",
                          "application/octet-stream",
                          "application/3gpp-ims+xml",
                          "application/vnd.3gpp.pic-bw-large",
                          "chemical/x-cdx",
                          "image/gif",
                          "audio/mp4",
                          NULL};
    for (size_t i = 0; strs[i]; ++i) {
        const HTTP::byte_string item = HTTP::strfy(strs[i]);
        HeaderHolderHTTP holder;
        minor_error me;
        me = holder.parse_header_line(HTTP::strfy("content-type: ") + item, &holder);
        EXPECT_TRUE(me.is_ok());
        HTTP::CH::ContentType ch;
        me = ch.determine(holder);
        EXPECT_TRUE(me.is_ok());
        EXPECT_EQ(item, ch.value);
    }
}

// type/subtype は正規化される
// 正規化手法はdowncase(小文字化)とする.
TEST(control_header_http, content_type_normalize) {
    const char *strs[] = {"application/octet-stream",
                          "APPLICATION/octet-stream",
                          "application/OCTET-streaM",
                          "apPlication/oCtet-strEam",
                          NULL};
    for (size_t i = 0; strs[i]; ++i) {
        const HTTP::byte_string item = HTTP::strfy(strs[i]);
        HeaderHolderHTTP holder;
        minor_error me;
        me = holder.parse_header_line(HTTP::strfy("content-type: ") + item, &holder);
        EXPECT_TRUE(me.is_ok());
        HTTP::CH::ContentType ch;
        me = ch.determine(holder);
        EXPECT_TRUE(me.is_ok());
        EXPECT_EQ(HTTP::strfy("application/octet-stream"), ch.value);
    }
}

TEST(control_header_http, content_type_basic_ko) {
    const char *strs[] = {"", "text", "text/", "text/ html", NULL};
    for (size_t i = 0; strs[i]; ++i) {
        const HTTP::byte_string item = HTTP::strfy(strs[i]);
        HeaderHolderHTTP holder;
        minor_error me;
        me = holder.parse_header_line(HTTP::strfy("content-type: ") + item, &holder);
        EXPECT_TRUE(me.is_ok());
        HTTP::CH::ContentType ch;
        me = ch.determine(holder);
        QVOUT(item);
        EXPECT_TRUE(me.is_error());
    }
}
