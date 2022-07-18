#include "RoundTrip.hpp"
#include "Connection.hpp"

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
    // リクエストが is_routable で, オリジネータが存在しない時
    return request_ != NULL && request_->is_routable() && originator_ == NULL;
}

void RoundTrip::notify_originator(IObserver &observer, IObserver::observation_category cat, t_time_epoch_ms epoch) {
    assert(originator_ != NULL);
    originator_->notify(observer, cat, epoch);
}

void RoundTrip::route(Connection &connection) {
    DXOUT("[route]");
    reroute_count += 1;
    originator_ = router.route_origin(request_);
    originator_->inject_socketlike(&connection);
}

bool RoundTrip::is_originatable() const {
    // [オリジネーション可能]
    // オリジネータが存在して,
    // オリジネーション開始可能(is_originatable)で
    // オリジネーションがまだ開始されていない(!is_origination_started)
    return originator_ != NULL && originator_->is_originatable() && !originator_->is_origination_started();
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
    return request_ != NULL && request_->is_complete();
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
    IOriginator *reoriginator = router.route_origin(request_);

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
    return response_ != NULL;
}

void RoundTrip::respond() {
    DXOUT("[respond]");
    response_ = router.route(request_);
    response_->feed_body(originator_->draw_data());
    response_->render();
}

void RoundTrip::respond_error(const http_error &err) {
    DXOUT("[respond_error]");
    DXOUT(err.get_status() << ":" << err.what());
    destroy_response();
    response_ = router.respond_error(request_, err);
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
