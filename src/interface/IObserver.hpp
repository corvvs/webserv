#ifndef IOSERVER_HPP
#define IOSERVER_HPP
#include "../socket/SocketType.hpp"

// [ソケット監視者インターフェース]
// [責務]
// - ソケットライクオブジェクト(ISocketLike)を保持すること
// - ソケットライクオブジェクトの状態変化を監視し, 変化があったら通知を出すこと

class ISocketLike;

class IObserver {
public:
    enum observation_category {
        OT_NONE,
        OT_READ,
        OT_WRITE,
        OT_EXCEPTION,
        OT_TIMEOUT,
        // 以下は, オリジネータに対するイベント通知をコネクションが受け取る時のカテゴリ.
        // 再送信の際に上のカテゴリから変換されるものであり,
        // オブザーバがこれを直接通知することはない.
        OT_ORIGINATOR_READ,
        OT_ORIGINATOR_WRITE,
        OT_ORIGINATOR_EXCEPTION,
        OT_ORIGINATOR_TIMEOUT
    };

    struct t_socket_reservation {
        t_fd fd;
        ISocketLike *sock;
        observation_category cat;
        bool in;
    };
    virtual ~IObserver(){};

    // ソケット監視ループ
    virtual void loop() = 0;
    // 指定されたソケットライクを, 次回のループ実行前に保持する
    virtual void reserve_hold(ISocketLike *socket) = 0;
    // 指定されたソケットライクを, 次回のループ実行前に破棄する
    virtual void reserve_unhold(ISocketLike *socket) = 0;
    // 指定されたソケットライクを, 次回のループ実行前に監視対象へ追加する
    // 保持していなければ自動的に保持する
    virtual void reserve_set(ISocketLike *socket, observation_category cat) = 0;
    // 指定されたソケットライクを, 次回のループ実行前に監視停止する
    virtual void reserve_unset(ISocketLike *socket, observation_category cat) = 0;
};

#endif
