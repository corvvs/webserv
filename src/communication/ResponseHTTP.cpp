#include "ResponseHTTP.hpp"
#include <unistd.h>

ResponseHTTP::ResponseHTTP(HTTP::t_version version, HTTP::t_status status, IResponseDataConsumer *data_consumer)
    : version_(version), status_(status), is_error_(false), sent_size(0), data_consumer_(data_consumer) {
    VOUT(data_consumer_);
}

ResponseHTTP::ResponseHTTP(HTTP::t_version version, http_error error)
    : version_(version), status_(error.get_status()), is_error_(true), sent_size(0), data_consumer_(NULL) {
    local_datalist.inject("", 0, true);
}

ResponseHTTP::~ResponseHTTP() {
    VOUT(this);
}

void ResponseHTTP::set_version(HTTP::t_version version) {
    version_ = version;
}

void ResponseHTTP::set_status(HTTP::t_status status) {
    status_ = status;
}

void ResponseHTTP::feed_header(const HTTP::header_key_type &key, const HTTP::header_val_type &val) {
    if (header_dict.find(key) != header_dict.end() && !is_error_) {
        throw std::runtime_error("Invalid Response: Duplicate header");
    }
    header_dict[key] = val;
    header_list.push_back(HTTP::header_kvpair_type(key, val));
}

HTTP::byte_string ResponseHTTP::serialize_former_part() {
    // 状態行
    message_text = HTTP::version_str(version_) + ParserHelper::SP + ParserHelper::utos(status_, 10) + ParserHelper::SP
                   + HTTP::reason(status_) + ParserHelper::CRLF;
    // ヘッダ
    for (std::vector<HTTP::header_kvpair_type>::iterator it = header_list.begin(); it != header_list.end(); ++it) {
        message_text += it->first + ParserHelper::HEADER_KV_SPLITTER + it->second + ParserHelper::CRLF;
    }
    // 空行
    message_text += ParserHelper::CRLF;
    return message_text;
}

void ResponseHTTP::start() {
    const ResponseDataList::t_sending_mode mode = consumer()->determine_sending_mode();
    if (mode == ResponseDataList::SM_CHUNKED) {
        feed_header(HeaderHTTP::transfer_encoding, HTTP::strfy("chunked"));
    }
    consumer()->start(serialize_former_part());
}

const ResponseHTTP::byte_string &ResponseHTTP::get_message_text() const {
    return message_text;
}

const char *ResponseHTTP::get_unsent_head() {
    consumer()->serialize_if_needed();
    return consumer()->serialized_head();
}

void ResponseHTTP::mark_sent(ssize_t sent) {
    if (sent < 0) {
        return;
    }
    consumer()->mark_sent(sent);
}

size_t ResponseHTTP::get_unsent_size() const {
    return consumer()->rest_serialized();
}

bool ResponseHTTP::is_complete() const {
    // VOUT(status_);
    // VOUT(getpid());
    // VOUT(this);
    // VOUT(consumer());
    return consumer() != NULL && consumer()->is_sending_over();
}

void ResponseHTTP::swap(ResponseHTTP &lhs, ResponseHTTP &rhs) {
    std::swap(lhs.version_, rhs.version_);
    std::swap(lhs.status_, rhs.status_);
    std::swap(lhs.is_error_, rhs.is_error_);
    std::swap(lhs.sent_size, rhs.sent_size);
    std::swap(lhs.header_list, rhs.header_list);
    std::swap(lhs.header_dict, rhs.header_dict);
    std::swap(lhs.body, rhs.body);
    std::swap(lhs.message_text, rhs.message_text);
    std::swap(lhs.local_datalist, rhs.local_datalist);
    std::swap(lhs.data_consumer_, rhs.data_consumer_);
}

bool ResponseHTTP::is_error() const {
    return is_error_;
}

IResponseDataConsumer *ResponseHTTP::consumer() {
    return (data_consumer_ != NULL) ? data_consumer_ : &local_datalist;
}

const IResponseDataConsumer *ResponseHTTP::consumer() const {
    return (data_consumer_ != NULL) ? data_consumer_ : &local_datalist;
}
