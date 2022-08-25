#include "ResponseDataList.hpp"
#include <cassert>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/uio.h>

ResponseDataBucket::ResponseDataBucket() throw() : is_completed(false) {}

ResponseDataList::ResponseDataList() : total(0), sent_serialized(0) {
    list.push_back(ResponseDataBucket());
}

// [As IResponseDataProducer]

void ResponseDataList::inject(const char *src, size_t n, bool is_completed) {
    assert(list.size() > 0);

    ResponseDataBucket &bucket = list.back();
    bucket.buffer.reserve(n);
    bucket.buffer.assign(src, src + n);
    bucket.is_completed = true;
    total += n;
    if (n > 0) {
        // 次に使うバケットを用意しておく
        list.push_back(ResponseDataBucket());
        if (is_completed) {
            list.back().is_completed = true;
        }
    }
}

void ResponseDataList::inject(const HTTP::byte_string &src, bool is_completed) {
    const HTTP::light_string l(src);
    inject(l, is_completed);
}

void ResponseDataList::inject(const HTTP::light_string &src, bool is_completed) {
    assert(list.size() > 0);

    ResponseDataBucket &bucket = list.back();
    bucket.buffer.reserve(src.size());
    bucket.buffer.assign(src.begin(), src.end());
    bucket.is_completed = true;
    total += src.size();
    if (src.size() > 0) {
        // 次に使うバケットを用意しておく
        list.push_back(ResponseDataBucket());
        if (is_completed) {
            list.back().is_completed = true;
        }
    }
}

bool ResponseDataList::is_injection_closed() const {
    return is_all_serialized() || list.back().is_completed;
}

void ResponseDataList::set_mode(t_sending_mode mode) throw() {
    sending_mode = mode;
}

// [As IResponseDataConsumer]
ResponseDataList::t_sending_mode ResponseDataList::determine_sending_mode() {
    t_sending_mode mode;
    if (is_injection_closed()) {
        // オリジネーションが完了している -> chunked送信しない
        mode = ResponseDataList::SM_NOT_CHUNKED;
    } else {
        // オリジネーションが未完了 -> chunked送信する
        mode = ResponseDataList::SM_CHUNKED;
    }
    set_mode(mode);
    return mode;
}

void ResponseDataList::start(const HTTP::byte_string &initial_data) {
    serialized_data = initial_data;
    sent_serialized = 0;
}

void ResponseDataList::serialize_if_needed() {
    // - シリアライズ可能で
    // - 現行のシリアライズデータが送信済みなら
    // シリアライズを実行する
    const bool do_serialize = is_serializable() && is_sent_current() && !is_sending_over();
    if (!do_serialize) {
        return;
    }

    const ResponseDataBucket &bucket = list.front();
    serialized_data                  = serialize_bucket(bucket);
    sent_serialized                  = 0;
    list.pop_front();
}

const HTTP::byte_string::value_type *ResponseDataList::serialized_head() const {
    return &serialized_data.front() + sent_serialized;
}

size_t ResponseDataList::rest_serialized() const throw() {
    return serialized_data.size() - sent_serialized;
}

void ResponseDataList::mark_sent(size_t n) throw() {
    sent_serialized += n;
}

bool ResponseDataList::is_sending_current() const throw() {
    return !sending_mode.is_null() && sent_serialized < serialized_data.size();
}

bool ResponseDataList::is_sent_current() const throw() {
    return !sending_mode.is_null() && !is_sending_current();
}

bool ResponseDataList::is_sending_over() const throw() {
    return is_all_serialized() && is_sent_current();
}

size_t ResponseDataList::current_total_size() const throw() {
    return total;
}

bool ResponseDataList::is_serializable() const {
    return list.size() > 0 && list.front().is_completed;
}

bool ResponseDataList::is_all_serialized() const throw() {
    return list.size() == 0;
}

HTTP::byte_string ResponseDataList::serialize_bucket(const ResponseDataBucket &bucket) {
    // VOUT(sending_mode);
    if (sending_mode.is_null()) {
        throw std::logic_error("unexpected sending_mode");
    }
    HTTP::byte_string serialized;
    switch (sending_mode.value()) {
        case SM_CHUNKED: {
            if (bucket.buffer.empty()) {
                // 終端チャンク
                serialized += HTTP::strfy("0");
                serialized += ParserHelper::CRLF;
                // ほんとはここにトレイラーフィールドが入る
                serialized += ParserHelper::CRLF;
            } else {
                // 通常チャンク
                serialized += ParserHelper::utos(bucket.buffer.size(), 16);
                serialized += ParserHelper::CRLF;
                serialized += bucket.buffer;
                serialized += ParserHelper::CRLF;
            }
            break;
        }
        case SM_NOT_CHUNKED: {
            serialized += bucket.buffer;
            break;
        }
    }
    return serialized;
}

ssize_t ResponseDataList::send(IDataSender &sender, int flags) throw() {
    serialize_if_needed();
    const HTTP::byte_string::value_type *send_buffer = serialized_head();
    const size_t send_size                           = rest_serialized();
    const ssize_t sent                               = sender.send(send_buffer, send_size, flags);
    if (sent >= 0) {
        mark_sent(sent);
    }
    return sent;
}

bool ResponseDataList::empty() const throw() {
    return list.empty();
}

const HTTP::byte_string &ResponseDataList::top() const {
    return list.front().buffer;
}

// [SendFileAgent]

SendFileAgent::SendFileAgent(const HTTP::char_string &path, int file_size)
    : path_(path), total_size(file_size), sent_size(0), sending_former(true) {
    int fd = open(path_.c_str(), O_RDONLY | O_CLOEXEC);
    if (fd < 0) {
        throw http_error("failed to open", HTTP::STATUS_FORBIDDEN);
    }
    leading_data.resize(10000);
    ssize_t read_size = read(fd, &leading_data.front(), 10000);
    if (read_size >= 0) {
        leading_data.resize(read_size);
    }
    if (read_size < 0) {
        throw http_error("failed to read", HTTP::STATUS_FORBIDDEN);
    }
    close(fd);
}

SendFileAgent::t_sending_mode SendFileAgent::determine_sending_mode() {
    // 問答無用で非チャンク送信
    sending_mode = SM_NOT_CHUNKED;
    return sending_mode.value();
}

void SendFileAgent::start(const HTTP::byte_string &initial_data) {
    former_data = initial_data;
}

bool SendFileAgent::is_sending_current() const throw() {
    if (sending_mode.is_null()) {
        return false;
    }
    if (sending_former) {
        return (HTTP::byte_string::size_type)sent_size < former_data.size();
    } else {
        return sent_size < total_size;
    }
}

bool SendFileAgent::is_sent_current() const throw() {
    return !sending_mode.is_null() && !is_sending_current();
}

bool SendFileAgent::is_sending_over() const throw() {
    return !sending_former && is_sent_current();
}

void SendFileAgent::mark_sent(size_t n) throw() {
    sent_size += n;
    VOUT(n);
    VOUT(sent_size);
    VOUT(total_size);
    if (sending_former) {
        if ((size_t)sent_size < former_data.size()) {
            return;
        }
        sending_former = false;
        sent_size      = 0;
    } else {
        if (sent_size < total_size) {
            return;
        }
    }
}

ssize_t SendFileAgent::send(IDataSender &sender, int flags) throw() {
    ssize_t sent = -1;
    VOUT(sending_former);
    if (sending_former) {
        int send_size = former_data.size() - sent_size;
        sent          = sender.send(&former_data.front() + sent_size, send_size, flags);
    } else {
        t_fd fd = open(path_.c_str(), O_RDONLY | O_CLOEXEC);
        if (fd >= 0) {
            sent       = sent_size;
            off_t s    = 0;
            errno      = 0;
            int failed = sendfile(fd, sender.get_fd(), sent, &s, NULL, flags);
            close(fd);
            if (failed && errno != EAGAIN) {
                close(fd);
                return -1;
            }
            sent = s;
            close(fd);
        }
    }
    if (sent >= 0) {
        mark_sent(sent);
    }
    return sent;
}

size_t SendFileAgent::current_total_size() const throw() {
    return total_size;
}

// 現在バケットが空かどうか
bool SendFileAgent::empty() const throw() {
    return sent_size < total_size;
}

// 先頭のバケットの中身の要素を返す
const HTTP::byte_string &SendFileAgent::top() const {
    return leading_data;
}
