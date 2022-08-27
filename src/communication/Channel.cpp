#include "Channel.hpp"
#include "../utils/ObjectHolder.hpp"
#include "Connection.hpp"

Channel::Channel(IRouter *router,
                 t_socket_domain sdomain,
                 t_socket_type stype,
                 t_port port,
                 const config::config_vector &configs,
                 FileCacher &cacher)
    : sock(SocketListening::bind(sdomain, stype, port)), router_(router), configs_(configs), cacher_(cacher) {
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
            ObjectHolder<Connection> con_holder = new Connection(router_, connected, configs_, cacher_);
            Connection *con                     = con_holder.value();
            observer.reserve_hold(con_holder.release());
            observer.reserve_set(con, IObserver::OT_READ);
        }
    } catch (...) {
        DXOUT("[!!!!] failed to accept socket: fd: " << sock->get_fd());
    }
}

Channel::t_channel_id Channel::get_id() const {
    return Channel::t_channel_id(sock->get_domain(), sock->get_port());
}
