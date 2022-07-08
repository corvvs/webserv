#ifndef CONNECTION_HPP
#define CONNECTION_HPP
#include "IRouter.hpp"
#include "../event/Iobserver.hpp"
#include "../event/Isocketlike.hpp"
#include "RequestHTTP.hpp"
#include "ResponseHTTP.hpp"
#include "../socket/SocketConnected.hpp"
#include <iostream>
#include <string>

// 準静的なコネクションの性質
// "準静的" = リクエストの内容次第で変更されうるが, それ以外(=時間変化など)によっては変わらない
struct ConnectionAttribute {
    HTTP::t_version http_version;
    bool is_persistent;
    t_time_epoch_ms timeout;

    ConnectionAttribute();
};

// [コネクションクラス]
// [責務]
// - 通信可能ソケット1つを保持すること; ISocketLike
// - このソケット経由の双方向通信を管理すること
//  - リクエストの受信と解釈
//  - レスポンスの送信
//    - 生成は ルーティングクラス(IRouter) が行う
class Connection : public ISocketLike {
public:
    // コネクションオブジェクトの内部状態
    enum t_phase {
        // 受信モード
        CONNECTION_RECEIVING,
        // 送信モード(通常応答)
        CONNECTION_RESPONDING,
        // 送信モード(エラー応答)
        CONNECTION_ERROR_RESPONDING,
        // Gracefulに接続を切っていくモード
        CONNECTION_SHUTTING_DOWN,
        CONNECTION_DUMMY
    };

private:
    IRouter *router_;
    ConnectionAttribute attr;

    t_phase phase;
    bool dying;

    SocketConnected *sock;

    RequestHTTP *current_req;
    ResponseHTTP *current_res;
    // 最終操作時刻
    t_time_epoch_ms latest_operated_at;

    int started_;

    // 最終操作時刻を更新する
    void touch();

    // もう一度受信状態に戻る
    void ready_receiving(IObserver &observer);

    // 送信状態に移る
    void ready_sending(IObserver &observer);

    // gracefulに接続を閉じるモードに移行する
    void ready_shutting_down(IObserver &observer);

    // 次のupdateで接続を完全に閉じる
    void die(IObserver &observer);

public:
    Connection(IRouter *router, SocketConnected *sock_given);
    ~Connection();

    t_fd get_fd() const;
    void notify(IObserver &observer);
    void timeout(IObserver &observer, t_time_epoch_ms epoch);
};

#endif
