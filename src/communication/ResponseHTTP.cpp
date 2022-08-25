#include "ResponseHTTP.hpp"
#include "../Interfaces.hpp"
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
ResponseHTTP::ResponseHTTP(HTTP::t_version version,
                           HTTP::t_status status,
                           const header_list_type *headers,
                           IResponseDataConsumer &data_consumer,
                           bool should_close)
    : version_(version)
    , status_(status)
    , lifetime(Lifetime::make_response())
    , data_consumer_(data_consumer)
    , should_close_(should_close) {
    if (headers != NULL) {
        for (header_list_type::const_iterator it = headers->begin(); it != headers->end(); ++it) {
            DXOUT(it->first << " - " << it->second);
            feed_header(it->first, it->second);
        }
    }
    start();
    lifetime.activate();
}

ResponseHTTP::~ResponseHTTP() {}

void ResponseHTTP::feed_header(const HTTP::header_key_type &key, const HTTP::header_val_type &val, bool overwrite) {
    if (!overwrite && header_dict.find(key) != header_dict.end() && !is_error()) {
        DXOUT("warning: duplicate header: " << key);
    }
    header_dict[key] = val;
    header_list.push_back(HTTP::header_kvpair_type(key, val));
}

HTTP::byte_string ResponseHTTP::serialize_former_part() {
    // 状態行
    HTTP::byte_string message_text = HTTP::version_str(version_) + ParserHelper::SP + ParserHelper::utos(status_, 10)
                                     + ParserHelper::SP + HTTP::reason(status_) + ParserHelper::CRLF;
    // ヘッダ
    for (std::vector<HTTP::header_kvpair_type>::iterator it = header_list.begin(); it != header_list.end(); ++it) {
        message_text
            += it->first + ParserHelper::HEADER_KV_SPLITTER + ParserHelper::SP + it->second + ParserHelper::CRLF;
    }
    // 空行
    message_text += ParserHelper::CRLF;
    return message_text;
}

void ResponseHTTP::start() {
    if (should_close_) {
        feed_header(HeaderHTTP::connection, HTTP::strfy("close"));
    }
    consumer().start(serialize_former_part());
}

void ResponseHTTP::mark_sent(ssize_t sent) throw() {
    if (sent < 0) {
        return;
    }
    if (is_complete()) {
        lifetime.deactivate();
    }
}

bool ResponseHTTP::is_complete() const throw() {
    return consumer().is_sending_over();
}

bool ResponseHTTP::is_error() const throw() {
    return merror.is_error();
}

bool ResponseHTTP::is_timeout(t_time_epoch_ms now) const throw() {
    return lifetime.is_timeout(now);
}

bool ResponseHTTP::should_close() const throw() {
    return should_close_;
}

IResponseDataConsumer &ResponseHTTP::consumer() throw() {
    return data_consumer_;
}

const IResponseDataConsumer &ResponseHTTP::consumer() const throw() {
    return data_consumer_;
}

bool ResponseHTTP::send_data(IDataSender &sender) throw() {
    const ssize_t sent = consumer().send(sender, 0);
    // VOUT(sent);
    const bool succeeded = sent >= 0;
    // VOUT(succeeded);
    // 送信ができなかったか, エラーが起きた場合
    if (succeeded) {
        mark_sent(sent);
    }
    return succeeded;
}
