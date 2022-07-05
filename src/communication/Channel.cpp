#include "Channel.hpp"
#include "Connection.hpp"

Channel::Channel() {
    throw std::runtime_error("forbidden");
}

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

void Channel::notify(IObserver &observer) {
    // Channelがnotifyを受ける
    // -> accept ready
    // -> Connectionを生成してread監視させる
    try {
        for (; true;) {
            SocketConnected *connected = sock->accept();
            if (connected == NULL) {
                // acceptするものが残っていない場合 NULL が返ってくる
                break;
            }
            observer.reserve_set(new Connection(router_, connected), IObserver::OT_READ);
        }
    } catch (...) {
        DXOUT("[!!!!] failed to accept socket: fd: " << sock->get_fd());
    }
}

void Channel::timeout(IObserver &observer, t_time_epoch_ms epoch) {
    // * DO NOTHING *
    (void)observer;
    (void)epoch;
    // DXOUT("* DO NOTHING *: " << get_fd());
}

Channel::t_channel_id Channel::get_id() const {
    return Channel::t_channel_id(sock->get_domain(), sock->get_port());
}
