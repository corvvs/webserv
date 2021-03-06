#ifndef ROUNDTRIP_HPP
#define ROUNDTRIP_HPP
#include "../Interfaces.hpp"
#include "../socket/SocketConnected.hpp"
#include "RequestHTTP.hpp"
#include "ResponseHTTP.hpp"
#include <deque>
#include <iostream>
#include <string>

class Connection;

// 前提:
// HTTPリクエストの受信開始から最終レスポンスの送信完了までの一連の流れを"RoundTrip"(ラウンドトリップ)と呼ぶことにする.
// 1つのラウンドトリップに関わる3要素をまとめたstruct.
class RoundTrip {
public:
    typedef HTTP::byte_string byte_string;
    typedef HTTP::light_string light_string;
    typedef std::deque<HTTP::char_type> extra_buffer_type;

private:
    IRouter &router;

    RequestHTTP *request_;
    // 注意:
    // オリジネーションが終わっても, ラウンドトリップが終わるまでオリジネータを破棄しないこと.
    IOriginator *originator_;
    ResponseHTTP *response_;

    // ルーティング実施回数
    // 通常は1で終わるが, 再ルーティングが起きると増えていく
    unsigned int reroute_count;

    // エラーレスポンス中かどうか
    bool in_error_responding;

    void destroy_request();
    void destroy_originator();
    void destroy_response();

public:
    RoundTrip(IRouter &router);
    ~RoundTrip();

    // [getters]

    RequestHTTP *req();
    IOriginator *orig();
    ResponseHTTP *res();

    // [predicates]

    // predicate: ルーティング開始可能かどうか
    bool is_routable() const;
    // predicate: オリジネーション開始可能かどうか
    bool is_originatable() const;
    // predicate: ラウンドトリップのフリーズ(インバウンドデータの拒否)を実施可能か
    bool is_freezable() const;
    // predicate: ラウンドトリップがフリーズ実施済みかどうか
    bool is_freezed() const;
    // predicate: 再ルーティングすべきか
    // `is_responsive`とは両立しない
    bool is_reroutable() const;
    // predicate: レスポンスデータを作成可能か
    // `is_reroutable`とは両立しない
    bool is_responsive() const;
    // predicate: ラウンドトリップを終了可能か
    bool is_terminatable() const;
    // predicate: レスポンスを送信中か
    bool is_responding() const;

    // predicate: ラウンドトリップがリクエストを持っていないならリクエストを作成する
    void start_if_needed();

    // データを注入する
    // received_buffer があるならそれが注入される.
    // そうでないなら extra_buffer の先頭から一定量が注入される.
    // (この時, extra_buffer が空であってはならない)
    // 接続が閉じたとみられるかどうかを返す
    bool inject_data(const u8t *received_buffer, ssize_t received_size, extra_buffer_type &extra_buffer);

    void notify_originator(IObserver &observer, IObserver::observation_category cat, t_time_epoch_ms epoch);

    // ルーティングを行い, オリジネータを生成する.
    void route(Connection &connection);
    // オリジネーションを開始する
    void originate(IObserver &observer);
    // リクエストを凍結, つまりデータの注入を受け付けなくする.
    // 余ったデータを返す
    light_string freeze_request();
    // 再ルーティングを行う
    void reroute(Connection &connection);
    // レスポンスを生成し, 送信を開始する
    void respond();

    // エラーレスポンスを生成し, 送信を開始する
    void respond_error(const http_error &err);

    // 構成要素全てをdeleteし, 初期状態に戻す.
    void wipeout();
};

#endif
