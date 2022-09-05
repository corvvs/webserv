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

    // ソケットには読み込み側と書き込み側があり,
    // それぞれ個別に閉じることができる(両方を閉じるとソケット自体が閉じたことになる).
    // また, こちら側と相手側のどちらが閉じたかで意味も変わってくる:
    // - 読み込み側 を 相手側 が閉じた
    //   -> 相手側に通信の意思なしとし, 直ちに接続を閉じる.
    //      グレースフルシャットダウンは最終的にこの状態になる
    // - 読み込み側 を こちら側 が閉じる
    //   -> 基本やらない; 送信エラー時にやってもいいかもしれない
    // - 書き込み側 を 相手側 が閉じた
    //   -> 特に何もしない; グレースフルシャットダウンに移行してもいいかもしれない
    // - 書き込み側 を こちら側 が閉じる
    //   -> グレースフルシャットダウンに移行
    //
    // 読み込み側と書き込み側の状態について:
    // - 読み込み open / 書き込み open
    //   -> 通常状態
    // - 読み込み close / 書き込み open
    //   -> 書き込み側もただちに閉じる.
    // - 読み込み open / 書き込み close
    //   -> こちら側が閉じた場合はグレースフルシャットダウンに移行する
    // - 読み込み close / 書き込み close
    //   -> 接続を破棄
    //

    // コネクションオブジェクトの内部状態
    enum t_phase {
        // 通常の状態:
        // ソケットの読み書き両方が使える.
        CONNECTION_ESTABLISHED,
        // グレースフルシャットダウン中の状態:
        // ソケットの書き込み側が閉鎖されている; 読み込み側は使えるが, 読み込んだデータは解釈せず捨てる.
        CONNECTION_SHUTTING_DOWN
    };

    struct Attribute {
        // 通信用ソケット
        SocketConnected *sock;

        Attribute(SocketConnected *sock);
        ~Attribute();
    };

    struct Status {
        t_phase phase;
        // 今切断中かどうか
        bool dying;
        // 読み込み側のデータを破棄するかどうか
        bool spilling_readside;

        Status();

        bool is_readside_active() const throw();
    };

    class NetworkBuffer {
    public:
        typedef std::list<byte_string> extra_buffer_type;

        struct Status {
            ssize_t read_size;
            // extra_buffer のサイズ合計
            size_t extra_amount;

            Status();
        };

    private:
        Status status;
        // ソケットからのデータが入るバッファ
        byte_string read_buffer;
        // 「余ったデータ」が入るバッファ
        extra_buffer_type extra_buffer;

        void check_extra_overflow();

    public:
        NetworkBuffer();

        // ソケットから内部バッファにデータを受信する
        ssize_t receive(SocketConnected &sock);
        // ソケットからデータを受信してそれを捨てる
        ssize_t spill(SocketConnected &sock);
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
    Status status;
    // ラウンドトリップ
    RoundTrip rt;
    // タイムアウト管理
    Lifetime lifetime;
    NetworkBuffer net_buffer;

    static bool is_extra_readable_cat(IObserver::observation_category cat) throw();

    void perform_reaction(IObserver &observer, IObserver::observation_category cat, t_time_epoch_ms epoch);
    void perform_receiving(IObserver &observer);
    void perform_sending(IObserver &observer);
    void perform_shutting_down(IObserver &observer);
    // 状態変化すべき状況を検知して状態変化を実施する
    void detect_update(IObserver &observer);

    // gracefulに接続を閉じるモードに移行する
    void start_shutdown(IObserver &observer);

    // 次のupdateで接続を完全に閉じる
    // 以降すべての通知をシャットアウトする
    void die(IObserver &observer);

    bool is_extra_readable() const throw();

public:
    Connection(SocketConnected *sock_given, IRouter *router, const config::config_vector &configs, FileCacher &cacher);
    ~Connection();

    t_fd get_fd() const;
    t_port get_port() const;
    void notify(IObserver &observer, IObserver::observation_category cat, t_time_epoch_ms epoch);
};

#endif
