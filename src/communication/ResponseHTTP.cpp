#include "ResponseHTTP.hpp"

ResponseHTTP::ResponseHTTP(HTTP::t_version version, HTTP::t_status status)
    : version_(version), status_(status), is_error(false), sent_size(0) {}

ResponseHTTP::ResponseHTTP(HTTP::t_version version, http_error error)
    : version_(version), status_(error.get_status()), is_error(true), sent_size(0) {}

void ResponseHTTP::feed_header(const HTTP::header_key_type &key, const HTTP::header_val_type &val) {
    if (header_dict.find(key) != header_dict.end() && !is_error) {
        throw std::runtime_error("Invalid Response: Duplicate header");
    }
    header_dict[key] = val;
    header_list.push_back(HTTP::header_kvpair_type(key, val));
}

void ResponseHTTP::feed_body(const byte_string &str) {
    body = str;
}

void ResponseHTTP::feed_body(const light_string &str) {
    body = str.str();
}

void ResponseHTTP::render() {
    // 状態行
    message_text = HTTP::version_str(version_) + ParserHelper::SP + ParserHelper::utos(status_) + ParserHelper::SP
                   + HTTP::reason(status_) + ParserHelper::CRLF;
    // content-length がない場合, かつbodyをもつメソッドの場合(TODO),
    // bodyのサイズをcontent-lengthとして出力
    if (header_dict.find(HeaderHTTP::content_length) == header_dict.end()) {
        header_list.insert(header_list.begin(),
                           HTTP::header_kvpair_type(HeaderHTTP::content_length, ParserHelper::utos(body.size())));
    }
    // ヘッダ
    for (std::vector<HTTP::header_kvpair_type>::iterator it = header_list.begin(); it != header_list.end(); it++) {
        message_text += it->first + ParserHelper::HEADER_KV_SPLITTER + it->second + ParserHelper::CRLF;
    }
    // 空行
    message_text += ParserHelper::CRLF;
    // ボディ
    message_text += body;
}

const ResponseHTTP::byte_string &ResponseHTTP::get_message_text() const {
    return message_text;
}

const char *ResponseHTTP::get_unsent_head() const {
    return &(message_text.front()) + sent_size;
}

void ResponseHTTP::mark_sent(ssize_t sent) {
    if (sent <= 0) {
        return;
    }
    sent_size += sent;
    if (message_text.size() < sent_size) {
        sent_size = message_text.size();
    }
}

size_t ResponseHTTP::get_unsent_size() const {
    return message_text.size() - sent_size;
}

bool ResponseHTTP::is_over_sending() const {
    return get_unsent_size() == 0;
}
