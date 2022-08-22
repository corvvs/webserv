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

t_fd Channel::get_fd() const throw() {
    return sock->get_fd();
}

t_port Channel::get_port() const throw() {
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
            ObjectHolder<SocketConnected> sh(connected);
            // [メモリ安全にするメカニズム]
            // 1. new した Connection をまず ObjectHolder で受ける.
            //    これで ObjectHolder が所有権(= delete する責任)を持つ.
            // 2. ObjectHolder に所有権を持たせたまま reserve_hold する
            //    もし reserve_hold が例外を投げた場合, ObjectHolder がスコープを抜けるときに
            //    ObjectHolder が delete を呼ぶ.
            // 3. reserve_hold が成功すると, 所有権は observer に以降するので,
            //    waive を呼んで ObjectHolder の所有権を放棄させる.
            //    以降, 例外が発生しても ObjectHolder は delete を呼ばない.
            ObjectHolder<Connection> ch(new Connection(router_, sh.value(), configs_, cacher_));
            sh.waive();
            observer.reserve_hold(ch.value());
            ch.waive();
            observer.reserve_set(ch.value(), IObserver::OT_READ);
        }
    } catch (...) {
        DXOUT("[!!!!] failed to accept socket: fd: " << sock->get_fd());
    }
}

Channel::t_channel_id Channel::get_id() const throw() {
    return Channel::t_channel_id(sock->get_domain(), sock->get_port());
}
