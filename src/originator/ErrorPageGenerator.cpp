#include "ErrorPageGenerator.hpp"
#include "../utils/CSS.hpp"
#include <unistd.h>

ErrorPageGenerator::ErrorPageGenerator(const minor_error &err,
                                       const RequestMatchingResult::status_dict_type &status_page_dict,
                                       FileCacher &cacher,
                                       bool should_close)
    : FileReader("", cacher), error(err), should_close_(should_close) {
    RequestMatchingResult::status_dict_type::const_iterator res = status_page_dict.find(err.status_code());
    if (res != status_page_dict.end()) {
        // エラーページ定義があった
        file_path_ = HTTP::restrfy(res->second);
    } else {
        file_path_.clear();
    }
}

ErrorPageGenerator::~ErrorPageGenerator() {}

void ErrorPageGenerator::generate_html() {
    // 出力データ生成
    response_data.inject(HTTP::strfy("<html>\n"
                                     "<head>\n"
                                     "<title>webserv error</title>\n"
                                     "  <style>\n" CSS_ERRORPAGE "  </style>\n"
                                     "</head>\n"
                                     "</body>\n"),
                         false);

    // page header

    response_data.inject(HTTP::strfy("<h3 class=\"error-page\">"), false);
    response_data.inject(ParserHelper::utos(error.status_code(), 10), false);
    response_data.inject(HTTP::strfy(" "), false);
    if (error.message().empty()) {
        response_data.inject(HTTP::reason(error.status_code()), false);
    } else {
        response_data.inject(HTTP::strfy(error.message()), false);
    }
    response_data.inject(HTTP::strfy("</h3>\n"
                                     "<hr>\n"),
                         false);

    // page footer

    response_data.inject(HTTP::strfy("</body>"
                                     "</html>\n"),
                         false);
    response_data.inject("", 0, true);
}

void ErrorPageGenerator::start_origination(IObserver &observer) {
    (void)observer;
    if (originated_) {
        return;
    }

    if (file_path_.size() > 0) {
        const minor_error me = read_from_file();
        if (me.is_ok()) {
            originated_ = true;
            return;
        }
    }
    // 簡易ページ生成
    generate_html();
    originated_ = true;
}

void ErrorPageGenerator::leave() {
    delete this;
}

ResponseHTTP::header_list_type
ErrorPageGenerator::determine_response_headers(const IResponseDataConsumer::t_sending_mode sm) const {
    ResponseHTTP::header_list_type headers;
    switch (sm) {
        case ResponseDataList::SM_CHUNKED:
            headers.push_back(std::make_pair(HeaderHTTP::transfer_encoding, HTTP::strfy("chunked")));
            break;
        case ResponseDataList::SM_NOT_CHUNKED:
            headers.push_back(
                std::make_pair(HeaderHTTP::content_length, ParserHelper::utos(response_data.current_total_size(), 10)));
            break;
        default:
            break;
    }
    // MIMEタイプ設定
    {
        HTTP::CH::ContentType ct;
        ct.value   = HTTP::strfy("text/html");
        ct.charset = HTTP::strfy("UTF-8");
        headers.push_back(std::make_pair(HeaderHTTP::content_type, ct.serialize()));
    }
    return headers;
}

ResponseHTTP *ErrorPageGenerator::respond(const RequestHTTP *request) {
    IResponseDataConsumer::t_sending_mode sm = response_data.determine_sending_mode();
    ResponseHTTP::header_list_type headers   = determine_response_headers(sm);
    const HTTP::t_version response_http_v    = request ? request->get_http_version() : HTTP::DEFAULT_HTTP_VERSION;
    ResponseHTTP *res = new ResponseHTTP(response_http_v, error.status_code(), &headers, &response_data, should_close_);
    res->start();
    return res;
}
