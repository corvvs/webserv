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

    bool is_active() const throw();

public:
    Lifetime(const t_time_epoch_ms &active, const t_time_epoch_ms &inactive) throw();

    void activate() throw();
    void deactivate() throw();
    bool is_timeout(t_time_epoch_ms now) const throw();

    // [Factory関数群]

    // コネクションができてからすぐにデータを送ってこない場合のための設定
    static const t_time_epoch_ms LIFETIME_CONNECTION_ACTIVE;
    static const t_time_epoch_ms LIFETIME_CONNECTION_INACTIVE;
    static Lifetime make_connection() throw();

    // ラウンドトリップの持続時間が長すぎる場合のための設定
    static const t_time_epoch_ms LIFETIME_ROUNDTRIP_ACTIVE;
    static const t_time_epoch_ms LIFETIME_ROUNDTRIP_INACTIVE;
    static Lifetime make_round_trip() throw();

    // リクエストの処理時間が長すぎる場合のための設定
    static const t_time_epoch_ms LIFETIME_REQUEST_HEADER_ACTIVE;
    static const t_time_epoch_ms LIFETIME_REQUEST_HEADER_INACTIVE;
    static Lifetime make_request() throw();

    // リクエストのヘッダの処理時間が長すぎる場合のための設定
    static const t_time_epoch_ms LIFETIME_REQUEST_ACTIVE;
    static const t_time_epoch_ms LIFETIME_REQUEST_INACTIVE;
    static Lifetime make_request_header() throw();

    // レスポンスの処理時間が長すぎる場合のための設定
    static const t_time_epoch_ms LIFETIME_RESPONSE_ACTIVE;
    static const t_time_epoch_ms LIFETIME_RESPONSE_INACTIVE;
    static Lifetime make_response() throw();

    // CGIの処理時間が長すぎる場合のための設定
    static const t_time_epoch_ms LIFETIME_CGI_ACTIVE;
    static const t_time_epoch_ms LIFETIME_CGI_INACTIVE;
    static Lifetime make_cgi() throw();
};

#endif
