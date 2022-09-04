#include "ResponseHTTP.hpp"
#include "ControlHeaderHTTP.hpp"
#include <unistd.h>

ResponseHTTP::Attribute::Attribute(HTTP::t_version version,
                                   HTTP::t_status status,
                                   bool should_close,
                                   IResponseDataConsumer *data_consumer)
    : version(version), status(status), should_close(should_close), data_consumer(data_consumer) {}

ResponseHTTP::Status::Status() {}

ResponseHTTP::ResponseHTTP(HTTP::t_version http_version,
                           HTTP::t_status http_status,
                           const header_list_type *headers,
                           IResponseDataConsumer *data_consumer,
                           bool should_close)
    : attr(http_version, http_status, should_close, data_consumer), lifetime(Lifetime::make_response()) {
    if (headers != NULL) {
        for (header_list_type::const_iterator it = headers->begin(); it != headers->end(); ++it) {
            feed_header(it->first, it->second);
        }
    }
    if (status.header_dict.find(HeaderHTTP::date) == status.header_dict.end()) {
        feed_header(HeaderHTTP::date, HTTP::CH::Date::now().serialize());
    }
    lifetime.activate();
    start();
}

ResponseHTTP::~ResponseHTTP() {}

void ResponseHTTP::feed_header(const HTTP::header_key_type &key, const HTTP::header_val_type &val, bool overwrite) {
    if (!overwrite && status.header_dict.find(key) != status.header_dict.end() && !is_error()) {
        DXOUT("warning: duplicate header: " << key);
    }
    status.header_dict[key] = val;
    status.header_list.push_back(HTTP::header_kvpair_type(key, val));
}

HTTP::byte_string ResponseHTTP::serialize_former_part() {
    // 状態行
    status.message_text = HTTP::version_str(attr.version) + ParserHelper::SP + ParserHelper::utos(attr.status, 10)
                          + ParserHelper::SP + HTTP::reason(attr.status) + ParserHelper::CRLF;
    // ヘッダ
    for (std::vector<HTTP::header_kvpair_type>::iterator it = status.header_list.begin();
         it != status.header_list.end();
         ++it) {
        status.message_text
            += it->first + ParserHelper::HEADER_KV_SPLITTER + ParserHelper::SP + it->second + ParserHelper::CRLF;
    }
    // 空行
    status.message_text += ParserHelper::CRLF;
    return status.message_text;
}

void ResponseHTTP::start() {
    if (attr.should_close) {
        feed_header(HeaderHTTP::connection, HTTP::strfy("close"));
    }
    consumer()->start(serialize_former_part());
}

const ResponseHTTP::byte_string &ResponseHTTP::get_message_text() const {
    return status.message_text;
}

const ResponseHTTP::byte_string::value_type *ResponseHTTP::get_unsent_head() {
    consumer()->serialize_if_needed();
    return consumer()->serialized_head();
}

void ResponseHTTP::mark_sent(ssize_t sent) {
    if (sent < 0) {
        return;
    }
    consumer()->mark_sent(sent);
    if (is_complete()) {
        lifetime.deactivate();
    }
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

bool ResponseHTTP::is_error() const {
    return status.merror.is_error();
}

bool ResponseHTTP::is_timeout(t_time_epoch_ms now) const {
    return lifetime.is_timeout(now);
}

bool ResponseHTTP::should_close() const {
    return attr.should_close;
}

IResponseDataConsumer *ResponseHTTP::consumer() {
    return (attr.data_consumer != NULL) ? attr.data_consumer : &status.local_datalist;
}

const IResponseDataConsumer *ResponseHTTP::consumer() const {
    return (attr.data_consumer != NULL) ? attr.data_consumer : &status.local_datalist;
}
