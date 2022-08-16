#include "ErrorPageGenerator.hpp"
#include <unistd.h>

ErrorPageGenerator::ErrorPageGenerator(const minor_error &err, const RequestMatchingResult &match_result)
    : FileReader(match_result), error(err) {
    const RequestMatchingResult::status_dict_type dict          = match_result.status_page_dict;
    RequestMatchingResult::status_dict_type::const_iterator res = dict.find(err.status_code());
    if (res != dict.end()) {
        // エラーページ定義があった
        file_path_ = HTTP::restrfy(res->second);
    } else {
        file_path_.clear();
    }
}

ErrorPageGenerator::~ErrorPageGenerator() {}

void ErrorPageGenerator::generate_html() {
    if (originated_) {
        return;
    }

    // 出力データ生成
    response_data.inject(HTTP::strfy("<html>\n"), false);
    response_data.inject(HTTP::strfy("<head>\n"), false);
    response_data.inject(HTTP::strfy("<title>webserv error</title>\n"), false);

    // common header

    response_data.inject(HTTP::strfy("</head>\n"), false);
    response_data.inject(HTTP::strfy("<body>\n"), false);

    // page header

    response_data.inject(HTTP::strfy("<h3>\n"), false);
    response_data.inject(ParserHelper::utos(error.status_code(), 10), false);
    response_data.inject(HTTP::strfy(": "), false);
    if (error.message().empty()) {
        response_data.inject(HTTP::reason(error.status_code()), false);
    } else {
        response_data.inject(HTTP::strfy(error.message()), false);
    }
    response_data.inject(HTTP::strfy("</h3>\n"), false);

    // page footer

    response_data.inject(HTTP::strfy("</body>\n"), false);
    response_data.inject(HTTP::strfy("</html>\n"), false);
    response_data.inject("", 0, true);
    originated_ = true;
}

void ErrorPageGenerator::start_origination(IObserver &observer) {
    (void)observer;
    if (file_path_.size() > 0) {
        read_from_file();
    } else {
        // 簡易ページ生成
        generate_html();
    }
}

void ErrorPageGenerator::leave() {
    delete this;
}

ResponseHTTP *ErrorPageGenerator::respond(const RequestHTTP &request) {
    ResponseHTTP::header_list_type headers;
    IResponseDataConsumer::t_sending_mode sm = response_data.determine_sending_mode();
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
    ResponseHTTP *res = new ResponseHTTP(request.get_http_version(), error.status_code(), &headers, &response_data);
    res->start();
    return res;
}
