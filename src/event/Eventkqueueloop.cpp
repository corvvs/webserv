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
    for (socket_map::iterator it = holding_map.begin(); it != holding_map.end(); it++) {
        delete it->second;
    }
}

EventKqueueLoop::t_kfilter EventKqueueLoop::filter(observation_category t) {
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

IObserver::observation_category EventKqueueLoop::filter_to_cat(t_kfilter f) {
    switch (f) {
        case EVFILT_READ:
            return IObserver::OT_READ;
        case EVFILT_WRITE:
            return IObserver::OT_WRITE;
        case EVFILT_EXCEPT:
            return IObserver::OT_EXCEPTION;
        default:
            throw std::runtime_error("unexpected map_type");
    }
}

void EventKqueueLoop::loop() {
    while (true) {
        update();

        int count = kevent(kq, NULL, 0, &*evlist.begin(), nev, NULL);
        VOUT(count);
        if (count < 0) {
            throw std::runtime_error("kevent error");
        } else if (count == 0) {
            t_time_epoch_ms now = WSTime::get_epoch_ms();
            for (int i = 0; i < count; i++) {
                int fd            = static_cast<int>(evlist[i].ident);
                ISocketLike *sock = holding_map[fd];
                sock->timeout(*this, now);
            }
        } else {
            for (int i = 0; i < count; i++) {
                int fd            = static_cast<int>(evlist[i].ident);
                ISocketLike *sock = holding_map[fd];
                observation_category cat = filter_to_cat(evlist[i].filter);
                sock->notify(*this, cat);
            }
        }
    }
}

void EventKqueueLoop::reserve_hold(ISocketLike *socket) {
    reserve(socket, OT_NONE, true);
}

void EventKqueueLoop::reserve_unhold(ISocketLike *socket) {
    reserve(socket, OT_NONE, false);
}

void EventKqueueLoop::reserve(ISocketLike *socket, observation_category cat, bool in) {
    t_socket_reservation pre = {socket->get_fd(), socket, cat, in};
    upqueue.push_back(pre);
}

// 次の kevent の前に, このソケットを監視対象から除外する
void EventKqueueLoop::reserve_unset(ISocketLike *socket, observation_category from) {
    reserve(socket, from, false);
}

// 次の kevent の前に, このソケットを監視対象に追加する
void EventKqueueLoop::reserve_set(ISocketLike *socket, observation_category to) {
    reserve(socket, to, true);
}

void EventKqueueLoop::update() {
    std::vector<t_kevent> changelist;
    changelist.reserve(upqueue.size());
    if (upqueue.empty()) {
        return;
    }
    // exec hold
    for (update_queue::iterator it = upqueue.begin(); it != upqueue.end(); it++) {
        ISocketLike *sock = it->sock;
        t_fd fd           = sock->get_fd();
        if (it->cat == OT_NONE && it->in) {
            holding_map.insert(socket_map::value_type(fd, sock));
        }
    }

    // exec set or unset
    int n = 0;
    for (update_queue::iterator it = upqueue.begin(); it != upqueue.end(); it++) {
        if (it->cat == OT_NONE) { continue; }
        t_fd fd           = it->fd;
        t_kevent ke;
        ISocketLike *sock = it->sock;
        switch (it->cat) {
            case OT_READ: {
                if (it->in) {
                    read_map.insert(socket_map::value_type(fd, sock));
                } else {
                    read_map.erase(fd);
                }
                break;
            }
            case OT_WRITE: {
                if (it->in) {
                    write_map.insert(socket_map::value_type(fd, sock));
                } else {
                    write_map.erase(fd);
                }
                break;
            }
            case OT_EXCEPTION: {
                if (it->in) {
                    exception_map.insert(socket_map::value_type(fd, sock));
                } else {
                    exception_map.erase(fd);
                }
                break;
            }
            default:
                throw std::runtime_error("unexpected cat");
        }
        changelist.push_back(ke);
        if (it->in) {
            EV_SET(&*changelist.rbegin(), fd, filter(it->cat), EV_ADD, 0, 0, NULL);
        } else if (!it->in) {
            EV_SET(&*changelist.rbegin(), fd, filter(it->cat), EV_DELETE, 0, 0, NULL);
        }
        n++;
    }
    if (n > 0) {
        errno     = 0;
        int count = kevent(kq, &*changelist.begin(), changelist.size(), NULL, 0, NULL);
        if (errno) {
            VOUT(errno);
            QVOUT(strerror(errno));
            DXOUT(changelist.size() << ", " << n << ", " << count);
        }
    }
    // exec unhold

    for (update_queue::iterator it = upqueue.begin(); it != upqueue.end(); it++) {
        ISocketLike *sock = it->sock;
        t_fd fd           = sock->get_fd();
        if (it->cat == OT_NONE && !it->in) {
            holding_map.erase(fd);
            delete sock;
        }
    }
    upqueue.clear();
}
