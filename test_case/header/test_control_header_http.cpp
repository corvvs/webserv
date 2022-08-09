#include "../../src/communication/ControlHeaderHTTP.hpp"
#include "gtest/gtest.h"

TEST(control_header_http, transfer_encoding_basic_ok) {
    const char *strs[] = {"chunked", "compress", "deflate", "gzip", NULL};
    for (size_t i = 0; strs[i]; ++i) {
        const HTTP::byte_string item = HTTP::strfy(strs[i]);
        HeaderHolderHTTP holder;
        minor_error me;
        me = holder.parse_header_line(HTTP::strfy("transfer-encoding: ") + item, &holder);
        EXPECT_TRUE(me.is_ok());
        HTTP::CH::TransferEncoding ch;
        me = ch.determine(holder);
        EXPECT_TRUE(me.is_ok());
        VOUT(ch.current_coding().coding);
        EXPECT_EQ(item, ch.current_coding().coding);
    }
}

TEST(control_header_http, transfer_encoding_multiple_ok) {
    const HTTP::byte_string item = HTTP::strfy("transfer-encoding: compress, deflate, CHUnkeD");
    HeaderHolderHTTP holder;
    minor_error me;
    me = holder.parse_header_line(item, &holder);
    EXPECT_TRUE(me.is_ok());
    HTTP::CH::TransferEncoding ch;
    me = ch.determine(holder);
    EXPECT_TRUE(me.is_ok());
    ch.transfer_codings.size();
    EXPECT_TRUE(ch.currently_chunked);
    EXPECT_EQ(3, ch.transfer_codings.size());
    EXPECT_EQ(HTTP::strfy("chunked"), ch.current_coding().coding);
}

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
    const char *strs[] = {"text", "text/", "text/ html", NULL};
    for (size_t i = 0; strs[i]; ++i) {
        const HTTP::byte_string item = HTTP::strfy(strs[i]);
        HeaderHolderHTTP holder;
        minor_error me;
        me = holder.parse_header_line(HTTP::strfy("content-type: ") + item, &holder);
        EXPECT_TRUE(me.is_ok());
        HTTP::CH::ContentType ch;
        me = ch.determine(holder);
        EXPECT_TRUE(me.is_ok());
        EXPECT_EQ(HTTP::strfy(""), ch.value);
    }
}

TEST(control_header_http, content_type_params) {
    minor_error me;
    {
        const HTTP::byte_string item = HTTP::strfy("content-type: multipart/form-data; key=value");
        HeaderHolderHTTP holder;
        me = holder.parse_header_line(item, &holder);
        EXPECT_TRUE(me.is_ok());
        HTTP::CH::ContentType ch;
        me = ch.determine(holder);
        EXPECT_TRUE(me.is_ok());
        EXPECT_EQ(HTTP::strfy("multipart/form-data"), ch.value);
        EXPECT_EQ(HTTP::strfy("value"), ch.parameters[HTTP::strfy("key")].str());
    }

    {
        const HTTP::byte_string item = HTTP::strfy("content-type: multipart/form-data;key=value,");
        HeaderHolderHTTP holder;
        me = holder.parse_header_line(item, &holder);
        EXPECT_TRUE(me.is_ok());
        HTTP::CH::ContentType ch;
        me = ch.determine(holder);
        EXPECT_TRUE(me.is_ok());
        EXPECT_EQ(HTTP::strfy("multipart/form-data"), ch.value);
        EXPECT_EQ(HTTP::strfy("value"), ch.parameters[HTTP::strfy("key")].str());
    }

    {
        const HTTP::byte_string item = HTTP::strfy("content-type: multipart/form-data;KEY=value,");
        HeaderHolderHTTP holder;
        me = holder.parse_header_line(item, &holder);
        EXPECT_TRUE(me.is_ok());
        HTTP::CH::ContentType ch;
        me = ch.determine(holder);
        EXPECT_TRUE(me.is_ok());
        EXPECT_EQ(HTTP::strfy("multipart/form-data"), ch.value);
        EXPECT_EQ(HTTP::strfy("value"), ch.parameters[HTTP::strfy("key")].str());
    }

    {
        const HTTP::byte_string item = HTTP::strfy("content-type: multipart/form-data;key1=val1 ;  key2=val2    ;");
        HeaderHolderHTTP holder;
        me = holder.parse_header_line(item, &holder);
        EXPECT_TRUE(me.is_ok());
        HTTP::CH::ContentType ch;
        me = ch.determine(holder);
        EXPECT_TRUE(me.is_ok());
        EXPECT_EQ(HTTP::strfy("multipart/form-data"), ch.value);
        EXPECT_EQ(HTTP::strfy("val1"), ch.parameters[HTTP::strfy("key1")].str());
        EXPECT_EQ(HTTP::strfy("val2"), ch.parameters[HTTP::strfy("key2")].str());
    }
}

TEST(control_header_http, content_type_bounary) {
    minor_error me;
    {
        const HTTP::byte_string item = HTTP::strfy("content-type: multipart/form-data; boundary=abcdefg;");
        HeaderHolderHTTP holder;
        me = holder.parse_header_line(item, &holder);
        EXPECT_TRUE(me.is_ok());
        HTTP::CH::ContentType ch;
        me = ch.determine(holder);
        EXPECT_TRUE(me.is_ok());
        EXPECT_EQ(HTTP::strfy("multipart/form-data"), ch.value);
        EXPECT_EQ(HTTP::strfy("abcdefg"), ch.parameters[HTTP::strfy("boundary")].str());
        EXPECT_EQ(HTTP::strfy("abcdefg"), ch.boundary.str());
    }

    {
        const HTTP::byte_string item
            = HTTP::strfy("content-type: multipart/form-data; "
                          "boundary=----------------------------------------------------------------------;");
        HeaderHolderHTTP holder;
        me = holder.parse_header_line(item, &holder);
        EXPECT_TRUE(me.is_ok());
        HTTP::CH::ContentType ch;
        me = ch.determine(holder);
        EXPECT_TRUE(me.is_ok());
        EXPECT_EQ(HTTP::strfy("multipart/form-data"), ch.value);
        EXPECT_EQ(HTTP::strfy("----------------------------------------------------------------------"),
                  ch.boundary.str());
    }

    {
        // boundaryは1文字~70文字
        const HTTP::byte_string item
            = HTTP::strfy("content-type: multipart/form-data; "
                          "boundary=-----------------------------------------------------------------------;");
        HeaderHolderHTTP holder;
        me = holder.parse_header_line(item, &holder);
        EXPECT_TRUE(me.is_ok());
        HTTP::CH::ContentType ch;
        me = ch.determine(holder);
        EXPECT_TRUE(me.is_ok());
        EXPECT_EQ(HTTP::strfy("multipart/form-data"), ch.value);
        EXPECT_EQ(HTTP::strfy(""), ch.boundary.str());
    }

    {
        // boundaryは1文字~70文字
        const HTTP::byte_string item = HTTP::strfy("content-type: multipart/form-data; boundary=");
        HeaderHolderHTTP holder;
        me = holder.parse_header_line(item, &holder);
        EXPECT_TRUE(me.is_ok());
        HTTP::CH::ContentType ch;
        me = ch.determine(holder);
        EXPECT_TRUE(me.is_ok());
        EXPECT_EQ(HTTP::strfy("multipart/form-data"), ch.value);
        // 無意味では・・・
        EXPECT_EQ(HTTP::strfy(""), ch.boundary.str());
    }
}
