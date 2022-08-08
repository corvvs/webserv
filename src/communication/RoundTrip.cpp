#include "RoundTrip.hpp"
#include "../Originators.hpp"
#include "Connection.hpp"
#include <cassert>

RoundTrip::RoundTrip(IRouter &router)
    : router(router), request_(NULL), originator_(NULL), response_(NULL), reroute_count(0) {}

RoundTrip::~RoundTrip() {
    wipeout();
}

RequestHTTP *RoundTrip::req() {
    return request_;
}

IOriginator *RoundTrip::orig() {
    return originator_;
}

ResponseHTTP *RoundTrip::res() {
    return response_;
}

void RoundTrip::start_if_needed() {
    if (request_ != NULL) {
        return;
    }
    DXOUT("[start_roundtrip]");
    request_ = new RequestHTTP();
}

bool RoundTrip::inject_data(const u8t *received_buffer, ssize_t received_size, extra_buffer_type &extra_buffer) {
    if (received_buffer != NULL) {
        start_if_needed();
        request_->inject_bytestring(received_buffer, received_buffer + received_size);
        return received_size == 0;
    } else {
        assert(extra_buffer.size() > 0);
        const extra_buffer_type::size_type n = std::min(extra_buffer.size(), (extra_buffer_type::size_type)1024);
        bool is_disconnected                 = (extra_buffer.size() == n);
        request_->inject_bytestring(extra_buffer.begin(), extra_buffer.begin() + n);
        extra_buffer.erase(extra_buffer.begin(), extra_buffer.begin() + n);
        return is_disconnected;
    }
}

bool RoundTrip::is_routable() const {
    // [ルーティング可能]
    // レスポンスが存在しない
    // リクエストが is_routable
    // オリジネータが存在しない
    return response_ == NULL && request_ != NULL && request_->is_routable() && originator_ == NULL;
}

void RoundTrip::notify_originator(IObserver &observer, IObserver::observation_category cat, t_time_epoch_ms epoch) {
    assert(originator_ != NULL);
    originator_->notify(observer, cat, epoch);
}

IOriginator *make_originator(const RequestMatchingResult &result, const RequestHTTP &request) {
    switch (result.result_type) {
        case RequestMatchingResult::RT_CGI:
            return new CGI(result, request);
        case RequestMatchingResult::RT_FILE_DELETE:
            return new FileDeleter(result);
        case RequestMatchingResult::RT_FILE_PUT:
            return new FileWriter(result, request.get_plain_message());
        case RequestMatchingResult::RT_AUTO_INDEX:
            return new AutoIndexer(result);
        case RequestMatchingResult::RT_EXTERNAL_REDIRECTION:
            return new Redirector(result);
        case RequestMatchingResult::RT_ECHO:
            return new Echoer(result);
        default:
            break;
    }
    return new FileReader(result);
}

void RoundTrip::route(Connection &connection) {
    DXOUT("[route]");
    assert(request_ != NULL);
    reroute_count += 1;
    const RequestMatchingResult result = router.route(request_->get_request_matching_param());
    originator_                        = make_originator(result, *request_);
    request_->set_max_body_size(result.client_max_body_size);
    originator_->inject_socketlike(&connection);
}

bool RoundTrip::is_originatable() const {
    // [オリジネーション可能]
    // レスポンスが存在しない
    // オリジネータが存在して,
    // オリジネーション開始可能(is_originatable)で
    // オリジネーションがまだ開始されていない(!is_origination_started)
    return response_ == NULL && originator_ != NULL && originator_->is_originatable()
           && !originator_->is_origination_started();
}

void RoundTrip::originate(IObserver &observer) {
    DXOUT("[originate]");
    originator_->start_origination(observer);
}

bool RoundTrip::is_freezable() const {
    return request_ != NULL && request_->is_complete() && !request_->is_freezed();
}

RoundTrip::light_string RoundTrip::freeze_request() {
    return request_->freeze();
}

bool RoundTrip::is_freezed() const {
    // [インバウンド閉鎖]
    // リクエストが存在して
    // 完結している(is_complete)
    // TODO: または, エラーレスポンスが存在する
    return request_ != NULL && (request_->is_complete() || (response_ && response_->is_error()));
}

bool RoundTrip::is_reroutable() const {
    // [再ルーティング]
    return originator_ != NULL && originator_->is_reroutable();
}

void RoundTrip::reroute(Connection &connection) {
    DXOUT("[reroute]");
    reroute_count += 1;
    if (reroute_count >= 10) {
        // リルートしすぎな時
        throw http_error("too many redirect", HTTP::STATUS_INTERNAL_SERVER_ERROR);
    }

    // TODO: 古いオリジネータの内容を考慮する
    const RequestMatchingResult result = router.route(request_->get_request_matching_param());
    IOriginator *reoriginator          = make_originator(result, *request_);
    request_->set_max_body_size(result.client_max_body_size);

    originator_->leave();
    originator_ = reoriginator;
    originator_->inject_socketlike(&connection);
}

bool RoundTrip::is_responsive() const {
    // [レスポンス可能]
    // レスポンスが存在しなくて,
    // オリジネータが存在して,
    // レスポンス可能(is_responsive)
    return response_ == NULL && originator_ != NULL && originator_->is_responsive();
}

bool RoundTrip::is_responding() const {
    // [レスポンス送信中]
    // レスポンスが存在している
    // (レスポンスは存在 = 送信中なので)
    return response_ != NULL;
}

void RoundTrip::respond() {
    DXOUT("[respond]");
    response_ = originator_->respond(*request_);
}

void RoundTrip::respond_error(const http_error &err) {
    DXOUT("[respond_error]");
    DXOUT(err.get_status() << ":" << err.what());
    destroy_response();
    in_error_responding = true;
    ResponseHTTP *res   = new ResponseHTTP(HTTP::DEFAULT_HTTP_VERSION, err);
    BVOUT(res->get_message_text());
    res->start();
    response_ = res;
}

bool RoundTrip::is_terminatable() const {
    // [レスポンス終了可能]
    // レスポンスが存在して,
    // 完結している(is_complete)
    // ※ これだけ write トリガーされる
    return response_ != NULL && response_->is_complete();
}

void RoundTrip::wipeout() {
    destroy_request();
    destroy_originator();
    destroy_response();
    reroute_count = 0;
}

void RoundTrip::destroy_request() {
    delete request_;
    request_ = NULL;
}

void RoundTrip::destroy_originator() {
    if (originator_) {
        // 破棄の手続きがオリジネータごとに異なるので, オリジネータに任せる
        originator_->leave();
        originator_ = NULL;
    }
}

void RoundTrip::destroy_response() {
    delete response_;
    response_ = NULL;
}
