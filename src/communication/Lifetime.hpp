#ifndef COMMUNICATION_LIFETIME_HPP
#define COMMUNICATION_LIFETIME_HPP
#include "../event/time.hpp"

// Lifetime は2つの時刻パラメータを持つ:
// - lifetime_active
// - lifetime_inactive
//
// この2つの大小関係に応じてLifetimeは2つの状態を持つ:
// - lifetime_inactive >= lifetime_active
//   -> 非アクティブ状態
// - lifetime_active > lifetime_inactive
//   -> アクティブ状態
//
// 関数 is_timeout は, 現在時刻に応じて
// - 非アクティブ状態なら, 非アクティブ状態になってから時間が経ち過ぎているかどうか
// - アクティブ状態なら, アクティブ状態になってから時間が経ち過ぎているかどうか
// を判定して返す.
class Lifetime {
private:
    // アクティブ状態になってからタイムアウトするまでの時間
    // ただし0はタイムアウトにならないことを意味する
    const t_time_epoch_ms lifetime_active;
    // 非アクティブ状態になってからタイムアウトするまでの時間
    // ただし0はタイムアウトにならないことを意味する
    const t_time_epoch_ms lifetime_inactive;

    t_time_epoch_ms deactivated_at;
    t_time_epoch_ms activated_at;

    bool is_active() const;

public:
    Lifetime(const t_time_epoch_ms &active, const t_time_epoch_ms &inactive);

    void activate();
    void deactivate();
    bool is_timeout(t_time_epoch_ms now) const;

    // [Factory関数群]
    static const t_time_epoch_ms LIFETIME_CONNECTION_ACTIVE;
    static const t_time_epoch_ms LIFETIME_CONNECTION_INACTIVE;
    static Lifetime make_connection();

    static const t_time_epoch_ms LIFETIME_ROUNDTRIP_ACTIVE;
    static const t_time_epoch_ms LIFETIME_ROUNDTRIP_INACTIVE;
    static Lifetime make_round_trip();

    static const t_time_epoch_ms LIFETIME_REQUEST_HEADER_ACTIVE;
    static const t_time_epoch_ms LIFETIME_REQUEST_HEADER_INACTIVE;
    static Lifetime make_request();

    static const t_time_epoch_ms LIFETIME_REQUEST_ACTIVE;
    static const t_time_epoch_ms LIFETIME_REQUEST_INACTIVE;
    static Lifetime make_request_header();

    static const t_time_epoch_ms LIFETIME_RESPONSE_ACTIVE;
    static const t_time_epoch_ms LIFETIME_RESPONSE_INACTIVE;
    static Lifetime make_response();

    static const t_time_epoch_ms LIFETIME_CGI_ACTIVE;
    static const t_time_epoch_ms LIFETIME_CGI_INACTIVE;
    static Lifetime make_cgi();
};

#endif
