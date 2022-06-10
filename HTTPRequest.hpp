#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP
#include <string>

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
    buffer_t    buffer;

    // 関数群
    HTTPRequest::start_line_t   parse_start_line();
    HTTPRequest::header_t       parse_header();
    HTTPRequest::body_t         parse_body();

    HTTPRequest request;
};

#endif
