#include "Eventkqueueloop.hpp"
#include "../utils/test_common.hpp"

const int EventKqueueLoop::nev = 10;

EventKqueueLoop::EventKqueueLoop() {
    evlist.resize(10);
    t_kqueue q = kqueue();
    if (q < 0) {
        throw std::runtime_error("failed to create kqueue");
    }
    kq = q;
}

EventKqueueLoop::~EventKqueueLoop() {
    for (socket_map::iterator it = sockmap.begin(); it != sockmap.end(); it++) {
        delete it->second;
    }
}

EventKqueueLoop::t_kfilter EventKqueueLoop::filter(t_observation_target t) {
    switch (t) {
        case OT_READ:
            return EVFILT_READ;
        case OT_WRITE:
            return EVFILT_WRITE;
        case OT_EXCEPTION:
            return EVFILT_EXCEPT;
        case OT_NONE:
            return 0;
        default:
            throw std::runtime_error("unexpected map_type");
    }
}

void EventKqueueLoop::loop() {
    while (true) {
        update();

        int count = kevent(kq, NULL, 0, &*evlist.begin(), nev, NULL);
        if (count < 0) {
            throw std::runtime_error("kevent error");
        } else if (count == 0) {
            t_time_epoch_ms now = WSTime::get_epoch_ms();
            for (int i = 0; i < count; i++) {
                int fd            = static_cast<int>(evlist[i].ident);
                ISocketLike *sock = sockmap[fd];
                sock->timeout(*this, now);
            }
        } else {
            for (int i = 0; i < count; i++) {
                int fd            = static_cast<int>(evlist[i].ident);
                ISocketLike *sock = sockmap[fd];
                sock->notify(*this);
            }
        }
    }
}

void EventKqueueLoop::reserve(ISocketLike *socket, t_observation_target from, t_observation_target to) {
    t_socket_reservation pre = {socket, from, to};
    upqueue.push_back(pre);
}

// 次の kevent の前に, このソケットを監視対象から除外する
// (その際ソケットはdeleteされる)
void EventKqueueLoop::reserve_clear(ISocketLike *socket, t_observation_target from) {
    reserve(socket, from, OT_NONE);
}

// 次の kevent の前に, このソケットを監視対象に追加する
void EventKqueueLoop::reserve_set(ISocketLike *socket, t_observation_target to) {
    reserve(socket, OT_NONE, to);
}

// 次の kevent の前に, このソケットの監視方法を変更する
void EventKqueueLoop::reserve_transit(ISocketLike *socket, t_observation_target from, t_observation_target to) {
    reserve(socket, from, to);
}

void EventKqueueLoop::update() {
    std::vector<t_kevent> changelist;
    changelist.reserve(upqueue.size());
    if (upqueue.empty()) {
        return;
    }
    int n = 0;
    for (update_queue::iterator it = upqueue.begin(); it != upqueue.end(); it++) {
        t_kevent ke;
        ISocketLike *sock = it->sock;
        t_fd fd           = sock->get_fd();
        if (it->to == OT_NONE) {
            sockmap.erase(fd);
            delete sock;
        } else {
            changelist.push_back(ke);
            EV_SET(&*changelist.rbegin(), sock->get_fd(), filter(it->to), EV_ADD, 0, 0, NULL);
            sockmap[fd] = sock;
            n++;
        }
    }
    if (n > 0) {
        errno     = 0;
        int count = kevent(kq, &*changelist.begin(), changelist.size(), NULL, 0, NULL);
        if (errno) {
            DXOUT("errno: " << errno << ", " << changelist.size() << ", " << n << ", " << count << std::endl);
        }
    }
    upqueue.clear();
}
