#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include <string>
#include "HTTPServer.hpp"

typedef std::string buffer_t;

class HTTPRequest{
public:


    typedef buffer_t    start_line_t;
    typedef buffer_t    header_t;
    typedef buffer_t    body_t;

    // 変数群
    start_line_t    start_line;
    header_t        header;
    body_t          body;

};

class RequestBuilder {
public:
    typedef enum ParsingPhase {
        START_LINE_START, // 0個以上の連続した改行
        START_LINE_END,
        HEADER,
        BODY,
        DUMMY
    };

    buffer_t    buffer;

    // 関数群
    HTTPRequest::start_line_t   parse_start_line();
    HTTPRequest::header_t       parse_header();
    HTTPRequest::body_t         parse_body();

    HTTPRequest request;

    // データ受け取りは外部がする
    ParsingPhase   parse_data(HTTPServer::read_buffer_t read_buffer);

    // 開始行の開始位置を見つける(改行を飛ばす)

    // 途中の状態保存

};

#endif
