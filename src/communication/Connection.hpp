#ifndef CONNECTION_HPP
#define CONNECTION_HPP
#include "../Interfaces.hpp"
#include "../socket/SocketConnected.hpp"
#include "Lifetime.hpp"
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
        bool is_persistent;
        t_time_epoch_ms timeout;

        Attribute();
    };

    class NetworkBuffer {
    public:
        typedef std::list<byte_string> extra_buffer_type;

    private:
        ssize_t read_size;
        // ソケットからのデータが入るバッファ
        byte_string read_buffer;
        // 「余ったデータ」が入るバッファ
        extra_buffer_type extra_buffer;
        // extra_buffer のサイズ合計
        size_t extra_amount;

        void check_extra_overflow();

    public:
        NetworkBuffer();

        // ソケットから内部バッファにデータを受信する
        // discard == true なら, 受信したデータを保存しない
        ssize_t receive(SocketConnected &sock, bool discard = false);
        // 内部バッファの先頭チャンクを取り出す
        const byte_string &top_front() const;
        // 内部バッファの先頭チャンクを除去する
        void pop_front();
        // 内部バッファの先頭にデータを追加する
        void push_front(const light_string &data);
        // 再実行すべきか
        bool should_redo() const;
    };

private:
    Attribute attr;

    t_phase phase;

    // 今切断中かどうか
    bool dying;
    bool unrecoverable;

    // 通信用ソケット
    SocketConnected *sock;

    // ラウンドトリップ
    RoundTrip rt;

    Lifetime lifetime;

    NetworkBuffer net_buffer;

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

    bool is_timeout(t_time_epoch_ms now) const;

public:
    Connection(IRouter *router, SocketConnected *sock_given, const config::config_vector &configs, FileCacher &cacher);
    ~Connection();

    t_fd get_fd() const;
    t_port get_port() const;
    void notify(IObserver &observer, IObserver::observation_category cat, t_time_epoch_ms epoch);
};

#endif
