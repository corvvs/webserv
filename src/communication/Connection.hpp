#ifndef CONNECTION_HPP
#define CONNECTION_HPP
#include "../interface/IObserver.hpp"
#include "../interface/IRouter.hpp"
#include "../interface/ISocketLike.hpp"
#include "../socket/SocketConnected.hpp"
#include "RequestHTTP.hpp"
#include "ResponseHTTP.hpp"
#include "RoundTrip.hpp"
#include <deque>
#include <iostream>
#include <string>

// [コネクションクラス]
// [責務]
// - 通信可能ソケット1つを保持すること; ISocketLike
// - このソケット経由の双方向通信を管理すること
//  - リクエストの受信と解釈
//  - レスポンスの送信
//    - 生成は ルーティングクラス(IRouter) が行う
class Connection : public ISocketLike {
public:
    typedef HTTP::byte_string byte_string;
    typedef HTTP::light_string light_string;
    typedef std::deque<HTTP::char_type> extra_buffer_type;

    // コネクションオブジェクトの内部状態
    enum t_phase {
        // 通常モード
        CONNECTION_ESTABLISHED,
        // Gracefulに接続を切っていくモード
        CONNECTION_SHUTTING_DOWN,
        CONNECTION_DUMMY
    };

    // 準静的なコネクションの性質
    // "準静的" = リクエストの内容次第で変更されうるが, それ以外(=時間変化など)によっては変わらない
    struct Attribute {
        HTTP::t_version http_version;
        bool is_persistent;
        t_time_epoch_ms timeout;

        Attribute();
    };

private:
    Attribute attr;

    t_phase phase;
    // 今切断中かどうか
    bool dying;

    // 通信用ソケット
    SocketConnected *sock;

    RoundTrip rt;

    // 最終操作時刻
    t_time_epoch_ms latest_operated_at;

    // 余剰データバッファ
    // あるリクエストが終端以降のデータを持っている場合, それを受け取って保持しておく
    // 次のリクエストがきたら, 受信データより優先的にこのデータを使ってリクエストを処理する
    extra_buffer_type extra_data_buffer;

    // 最終操作時刻を更新する
    void touch();

    void perform_reaction(IObserver &observer, IObserver::observation_category cat, t_time_epoch_ms epoch);
    void perform_receiving(IObserver &observer);
    void perform_sending(IObserver &observer);
    void perform_shutting_down(IObserver &observer);
    // 状態変化すべき状況を検知して状態変化を実施する
    void detect_update(IObserver &observer);

    // gracefulに接続を閉じるモードに移行する
    void shutdown_gracefully(IObserver &observer);

    // 次のupdateで接続を完全に閉じる
    // 以降すべての通知をシャットアウトする
    void die(IObserver &observer);

public:
    Connection(IRouter *router, SocketConnected *sock_given);
    ~Connection();

    t_fd get_fd() const;
    void notify(IObserver &observer, IObserver::observation_category cat, t_time_epoch_ms epoch);
};

#endif
