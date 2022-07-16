#ifndef ISOCKET_LIKE_HPP
#define ISOCKET_LIKE_HPP

#include "../event/time.hpp"
#include "../socket/SocketType.hpp"
#include "IObserver.hpp"

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
    virtual void notify(IObserver &observer, IObserver::observation_category cat, t_time_epoch_ms epoch) = 0;
};

#endif
