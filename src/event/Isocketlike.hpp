#ifndef ISOCKET_LIKE_HPP
#define ISOCKET_LIKE_HPP

#include "../socket/SocketType.hpp"
#include "Iobserver.hpp"
#include "time.hpp"

class IObserver;

// [ソケットライクインターフェース]
// [責務]
// - ある1つのソケットに紐づいていること
// - ソケット監視者からの通知を受け取り, しかるべき処理を行うこと
class ISocketLike {
public:
    virtual ~ISocketLike(){};
    // 紐づいているソケットのfdを返す
    virtual t_fd get_fd() const = 0;
    // ソケット監視者からの通知を受け取る
    virtual void notify(IObserver &observer) = 0;
    // タイムアウトが疑われる時の処理; timeout
    // が呼ばれたからと言って即タイムアウトではないことに注意
    virtual void timeout(IObserver &observer, t_time_epoch_ms epoch) = 0;
};

#endif
