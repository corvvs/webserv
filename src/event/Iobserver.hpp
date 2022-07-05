#ifndef IOSERVER_HPP
#define IOSERVER_HPP

#include "Isocketlike.hpp"
#include "time.hpp"

// [ソケット監視者インターフェース]
// [責務]
// - ソケットライクオブジェクト(ISocketLike)を保持すること
// - ソケットライクオブジェクトの状態変化を監視し, 変化があったら通知を出すこと
class IObserver {
public:
    enum t_observation_target { OT_NONE, OT_READ, OT_WRITE, OT_EXCEPTION };

    struct t_socket_reservation {
        ISocketLike *sock;
        t_observation_target from;
        t_observation_target to;
    };
    virtual ~IObserver(){};

    // ソケット監視ループ
    virtual void loop() = 0;
    // 指定されたソケットライクを, 次回のループ実行前に監視対象から除外し,
    // 破棄する
    virtual void reserve_clear(ISocketLike *socket, t_observation_target from) = 0;
    // 指定されたソケットライクを, 次回のループ実行前に監視対象へ追加する
    virtual void reserve_set(ISocketLike *socket, t_observation_target to) = 0;
    // 指定されたソケットライクについて, 次回のループ実行前に監視方法を変更する
    virtual void reserve_transit(ISocketLike *socket, t_observation_target from, t_observation_target to) = 0;
};

#endif
