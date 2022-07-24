#include "Channel.hpp"
#include "Connection.hpp"

Channel::Channel(IRouter *router, t_socket_domain sdomain, t_socket_type stype, t_port port)
    : sock(SocketListening::bind(sdomain, stype, port)), router_(router) {
    sock->listen(1024);
}

Channel::~Channel() {
    delete sock;
}

t_fd Channel::get_fd() const {
    return sock->get_fd();
}

t_port Channel::get_port() const {
    return sock->get_port();
}

void Channel::notify(IObserver &observer, IObserver::observation_category cat, t_time_epoch_ms epoch) {
    (void)epoch;
    // Channelがnotifyを受ける
    // -> accept ready
    // -> Connectionを生成してread監視させる
    if (cat != IObserver::OT_READ) {
        return;
    }
    try {
        for (;;) {
            SocketConnected *connected = sock->accept();
            if (connected == NULL) {
                // acceptするものが残っていない場合 NULL が返ってくる
                break;
            }
            Connection *con = new Connection(router_, connected);
            observer.reserve_hold(con);
            observer.reserve_set(con, IObserver::OT_READ);
        }
    } catch (...) {
        QVOUT(strerror(errno));
        DXOUT("[!!!!] failed to accept socket: fd: " << sock->get_fd());
    }
}

Channel::t_channel_id Channel::get_id() const {
    return Channel::t_channel_id(sock->get_domain(), sock->get_port());
}
