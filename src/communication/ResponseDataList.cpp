#include "ResponseDataList.hpp"
#include <cassert>

ResponseDataBucket::ResponseDataBucket() : is_completed(false) {}

ResponseDataList::ResponseDataList() : sent_serialized(0), sending_mode(SM_UNKNOWN) {
    list.push_back(ResponseDataBucket());
}

// [As IResponseDataProducer]

void ResponseDataList::inject(const char *src, size_t n, bool is_completed) {
    assert(list.size() > 0);
    VOUT(n);
    // BVOUT(HTTP::byte_string(src, src + n));

    ResponseDataBucket &bucket = list.back();
    bucket.buffer.reserve(n);
    bucket.buffer.assign(src, src + n);
    bucket.is_completed = true;
    if (n > 0) {
        list.push_back(ResponseDataBucket());
        if (is_completed) {
            list.back().is_completed = true;
        }
    }
}

bool ResponseDataList::is_injection_closed() const {
    return is_all_serialized() || list.back().is_completed;
}

void ResponseDataList::set_mode(t_sending_mode mode) {
    assert(mode != SM_UNKNOWN);
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
    // BVOUT(initial_data);
    VOUT(sent_serialized);
    serialized_data = initial_data;
    sent_serialized = 0;
}

void ResponseDataList::serialize_if_needed() {
    // BVOUT(serialized_data);
    // - シリアライズ可能で
    // - 現行のシリアライズデータが送信済みなら
    // シリアライズを実行する
    // VOUT(is_serializable());
    // VOUT(is_sent_current());
    // VOUT(is_sending_over());
    bool do_serialize = is_serializable() && is_sent_current() && !is_sending_over();
    VOUT(do_serialize);
    if (!do_serialize) {
        return;
    }

    const ResponseDataBucket &bucket = list.front();
    serialized_data                  = serialize_bucket(bucket);
    sent_serialized                  = 0;
    list.pop_front();
}

const char *ResponseDataList::serialized_head() const {
    return &serialized_data.front();
}

size_t ResponseDataList::rest_serialized() const {
    return serialized_data.size() - sent_serialized;
}

void ResponseDataList::mark_sent(size_t n) {
    sent_serialized += n;
}

bool ResponseDataList::is_sending_current() const {
    return sending_mode != SM_UNKNOWN && sent_serialized < serialized_data.size();
}

bool ResponseDataList::is_sent_current() const {
    return sending_mode != SM_UNKNOWN && !is_sending_current();
}

bool ResponseDataList::is_sending_over() const {
    return is_all_serialized() && is_sent_current();
}

//

bool ResponseDataList::is_serializable() const {
    return list.size() > 0 && list.front().is_completed;
}

bool ResponseDataList::is_all_serialized() const {
    return list.size() == 0;
}

HTTP::byte_string ResponseDataList::serialize_bucket(const ResponseDataBucket &bucket) {
    VOUT(sending_mode);
    HTTP::byte_string serialized;
    switch (sending_mode) {
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
        default:
            throw std::logic_error("unexpected sending_mode");
    }
    return serialized;
}

ResponseDataList::t_sending_mode ResponseDataList::get_sending_mode() const {
    return sending_mode;
}
