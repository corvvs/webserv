#include "RoundTrip.hpp"
#include "../Originators.hpp"
#include "Connection.hpp"
#include <cassert>

RoundTrip::Attribute::Attribute(IRouter &router, FileCacher &cacher, const config::config_vector &configs)
    : router(router), cacher(cacher), configs(configs) {}

RoundTrip::Status::Status() : reroute_count(0), in_error_responding(false) {}

RoundTrip::RoundTrip(IRouter &router, const config::config_vector &configs, FileCacher &cacher)
    : attr(router, cacher, configs)
    , request_(NULL)
    , originator_(NULL)
    , response_(NULL)
    , lifetime(Lifetime::make_round_trip()) {}

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
    lifetime.activate();
}

bool RoundTrip::inject_data(const char *received_buffer, ssize_t received_size) {
    start_if_needed();
    request_->inject_bytestring(received_buffer, received_buffer + received_size);
    return received_size == 0;
}

bool RoundTrip::inject_data(const light_string &received_buffer) {
    start_if_needed();
    request_->inject_bytestring(received_buffer.begin(), received_buffer.end());
    return received_buffer.size() == 0;
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

IOriginator *RoundTrip::make_originator(const RequestMatchingResult &result, const RequestHTTP &request) {
    switch (result.result_type) {
        case RequestMatchingResult::RT_CGI:
            return new CGI(result, request);
        case RequestMatchingResult::RT_FILE_DELETE:
            return new FileDeleter(result);
        case RequestMatchingResult::RT_FILE_POST:
            return new FilePoster(result, request);
        case RequestMatchingResult::RT_AUTO_INDEX:
            return new AutoIndexer(result);
        case RequestMatchingResult::RT_EXTERNAL_REDIRECTION:
            return new Redirector(result);
        case RequestMatchingResult::RT_ECHO:
            return new Echoer(result);
        default:
            break;
    }
    return new FileReader(result, attr.cacher, &request);
}

void RoundTrip::route(Connection &connection) {
    DXOUT("[route]");
    assert(request_ != NULL);
    status.reroute_count += 1;
    const RequestMatchingResult result = attr.router.route(request_->get_request_matching_param(), attr.configs);

    // エラーページの情報をRoundTripに引き継ぐ
    status_page_dict_ = result.status_page_dict;
    if (request_->current_error().is_error()) {
        const minor_error me = request_->purge_error();
        originator_          = new ErrorPageGenerator(me, status_page_dict_, attr.cacher);
        DXOUT("purged error: " << me);
    } else if (result.error.is_error()) {
        originator_ = new ErrorPageGenerator(result.error, status_page_dict_, attr.cacher);
    } else {
        originator_ = make_originator(result, *request_);
        request_->set_max_body_size(result.client_max_body_size);
    }
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
    status.reroute_count += 1;
    if (status.reroute_count >= 10) {
        // リルートしすぎな時
        throw http_error("too many redirect", HTTP::STATUS_INTERNAL_SERVER_ERROR);
    }

    // TODO: 古いオリジネータの内容を考慮する
    assert(originator_ != NULL);
    request_->inject_reroute_path(originator_->reroute_path());

    const RequestMatchingResult result = attr.router.route(request_->get_request_matching_param(), attr.configs);

    VOUT(result.result_type);
    IOriginator *reoriginator = make_originator(result, *request_);
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

bool RoundTrip::is_timeout(t_time_epoch_ms now) const {
    if (request_ != NULL && request_->is_timeout(now)) {
        return true;
    }
    if (response_ != NULL && response_->is_timeout(now)) {
        return true;
    }
    return lifetime.is_timeout(now);
}

void RoundTrip::respond() {
    DXOUT("[respond]");
    const bool should_close = request_ == NULL || !request_->should_keep_in_touch();
    response_               = originator_->respond(request_, should_close);
}

void RoundTrip::respond_unrecoverable_error(IObserver &observer, const http_error &err) {
    DXOUT("[respond_unrecoverable_error]");
    DXOUT(err.get_status() << ":" << err.what());
    VOUT(response_);
    destroy_response();
    status.in_error_responding = true;

    destroy_originator();
    originator_ = new ErrorPageGenerator(err, status_page_dict_, attr.cacher, true);
    originator_->start_origination(observer);
    response_ = originator_->respond(request_, true);
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
    destroy_response();
    destroy_originator();
    status = Status();
    lifetime.deactivate();
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

void RoundTrip::emit_fatal_error() {
    // レスポンスがすでに存在する状態でリクエストがエラーを抱えているなら,
    // それを http_error に変えて送出
    if (response_ != NULL && request_ != NULL && request_->current_error().is_error()) {
        throw http_error(request_->purge_error());
    }
}
