#include "Eventselectloop.hpp"
#include "../utils/test_common.hpp"

void EventSelectLoop::destroy_all(EventSelectLoop::socket_map &m) {
    for (EventSelectLoop::socket_map::iterator it = m.begin(); it != m.end(); it++) {
        delete it->second;
    }
}

EventSelectLoop::EventSelectLoop() {}

EventSelectLoop::~EventSelectLoop() {
    destroy_all(read_map);
    destroy_all(write_map);
    destroy_all(exception_map);
}

void EventSelectLoop::watch(ISocketLike *socket, observation_category map_type) {
    switch (map_type) {
        case OT_READ:
            read_map[socket->get_fd()] = socket;
            break;
        case OT_WRITE:
            write_map[socket->get_fd()] = socket;
            break;
        case OT_EXCEPTION:
            exception_map[socket->get_fd()] = socket;
            break;
        default:
            throw std::runtime_error("unexpected map_type");
    }
}

void EventSelectLoop::unwatch(ISocketLike *socket, observation_category map_type) {
    switch (map_type) {
        case OT_READ:
            read_map.erase(socket->get_fd());
            break;
        case OT_WRITE:
            write_map.erase(socket->get_fd());
            break;
        case OT_EXCEPTION:
            exception_map.erase(socket->get_fd());
            break;
        default:
            throw std::runtime_error("unexpected map_type");
    }
}

// ソケットマップ sockmap をFD集合 socketset に変換する
void EventSelectLoop::prepare_fd_set(socket_map &sockmap, fd_set *sockset) {
    FD_ZERO(sockset);
    for (socket_map::iterator it = sockmap.begin(); it != sockmap.end(); it++) {
        FD_SET(it->first, sockset);
    }
}

// sockmap 中のソケットがFD集合 socketset に含まれるかどうかを調べ,
// 含まれている場合はソケットの notify メソッドを実行する
void EventSelectLoop::scan_fd_set(socket_map &sockmap, fd_set *sockset, t_time_epoch_ms now, IObserver::observation_category cat) {
    for (EventSelectLoop::socket_map::iterator it = sockmap.begin(); it != sockmap.end(); it++) {
        if (cat == OT_TIMEOUT) {
            it->second->timeout(*this, now);
        } else {
            if (FD_ISSET(it->first, sockset)) {
                it->second->notify(*this, cat);
            }
        }
    }
}

// イベントループ
void EventSelectLoop::loop() {
    while (true) {
        update();

        fd_set read_set;
        fd_set write_set;
        fd_set exception_set;
        prepare_fd_set(read_map, &read_set);
        prepare_fd_set(write_map, &write_set);
        prepare_fd_set(exception_map, &exception_set);

        t_fd max_fd = -1;
        if (!read_map.empty()) {
            max_fd = std::max(max_fd, read_map.rbegin()->first);
        }
        if (!write_map.empty()) {
            max_fd = std::max(max_fd, write_map.rbegin()->first);
        }
        if (!exception_map.empty()) {
            max_fd = std::max(max_fd, exception_map.rbegin()->first);
        }

        timeval tv = {10, 0};
        int count  = select(max_fd + 1, &read_set, &write_set, &exception_set, &tv);
        if (count < 0) {
            VOUT(strerror(errno));
            throw std::runtime_error("select error");
        } else if (count == 0) {
            t_time_epoch_ms now = WSTime::get_epoch_ms();
            DXOUT("timeout?: " << now);
            scan_fd_set(read_map, &read_set, now, OT_TIMEOUT);
            scan_fd_set(write_map, &write_set, now, OT_TIMEOUT);
            scan_fd_set(exception_map, &exception_set, now, OT_TIMEOUT);
        } else {
            scan_fd_set(read_map, &read_set, 0, OT_READ);
            scan_fd_set(write_map, &write_set, 0, OT_WRITE);
            scan_fd_set(exception_map, &exception_set, 0, OT_EXCEPTION);
        }
    }
}

void EventSelectLoop::reserve(ISocketLike *socket, observation_category cat, bool in) {
    t_socket_reservation pre = {socket->get_fd(), socket, cat, in};
    up_queue.push_back(pre);
}

void EventSelectLoop::reserve_hold(ISocketLike *socket) {
    reserve(socket, OT_NONE, true);
}

void EventSelectLoop::reserve_unhold(ISocketLike *socket) {
    reserve(socket, OT_READ, false);
    reserve(socket, OT_WRITE, false);
    reserve(socket, OT_EXCEPTION, false);
    reserve(socket, OT_NONE, false);
}

void EventSelectLoop::reserve_unset(ISocketLike *socket, observation_category cat) {
    reserve(socket, cat, false);
}

void EventSelectLoop::reserve_set(ISocketLike *socket, observation_category cat) {
    reserve(socket, cat, true);
}

// ソケットの監視状態変更予約を実施する
void EventSelectLoop::update() {
    // exec hold
    for (update_queue::iterator it = up_queue.begin(); it != up_queue.end(); it++) {
        ISocketLike *sock = it->sock;
        t_fd fd           = sock->get_fd();
        if (it->cat == OT_NONE && it->in) {
            holding_map.insert(socket_map::value_type(fd, sock));
        }
    }

    // exec set or unset
    for (EventSelectLoop::update_queue::iterator it = up_queue.begin(); it != up_queue.end(); it++) {
        if (it->cat == OT_NONE) { continue; }
        if (it->in) {
            watch(it->sock, it->cat);
        } else {
            unwatch(it->sock, it->cat);
        }
    }

    // exec unhold
    for (update_queue::iterator it = up_queue.begin(); it != up_queue.end(); it++) {
        ISocketLike *sock = it->sock;
        t_fd fd           = sock->get_fd();
        if (it->cat == OT_NONE && !it->in) {
            holding_map.erase(fd);
            delete sock;
        }
    }
    up_queue.clear();
}
