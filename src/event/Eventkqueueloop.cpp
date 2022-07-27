#include "Eventkqueueloop.hpp"
#include "../utils/test_common.hpp"

const int EventKqueueLoop::nev                          = 10;
const t_time_epoch_ms EventKqueueLoop::timeout_interval = 1 * 1000;

EventKqueueLoop::EventKqueueLoop() : latest_timeout_checked(WSTime::get_epoch_ms()) {
    evlist.resize(10);
    t_kqueue q = kqueue();
    if (q < 0) {
        throw std::runtime_error("failed to create kqueue");
    }
    kq = q;
}

EventKqueueLoop::~EventKqueueLoop() {
    for (socket_map::iterator it = holding_map.begin(); it != holding_map.end(); ++it) {
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
        }
        t_time_epoch_ms now = WSTime::get_epoch_ms();
        if (count == 0 || now - latest_timeout_checked > timeout_interval) {
            for (int i = 0; i < count; i++) {
                const t_fd fd = static_cast<int>(evlist[i].ident);
                holding_map[fd]->notify(*this, OT_TIMEOUT, now);
            }
            latest_timeout_checked = now;
        }
        if (count > 0) {
            for (int i = 0; i < count; i++) {
                const t_fd fd            = static_cast<int>(evlist[i].ident);
                observation_category cat = filter_to_cat(evlist[i].filter);
                holding_map[fd]->notify(*this, cat, 0);
            }
        }
    }
}

void EventKqueueLoop::reserve(ISocketLike *socket, observation_category cat, bool in) {
    t_socket_reservation pre = {socket->get_fd(), socket, cat, in};
    upqueue.push_back(pre);
}

void EventKqueueLoop::reserve_hold(ISocketLike *socket) {
    reserve(socket, OT_NONE, true);
}

void EventKqueueLoop::reserve_unhold(ISocketLike *socket) {
    reserve(socket, OT_NONE, false);
    DXOUT("reserved unhold: " << socket->get_fd());
}

// 次の kevent の前に, このソケットを監視対象から除外する
void EventKqueueLoop::reserve_unset(ISocketLike *socket, observation_category cat) {
    reserve(socket, cat, false);
    DXOUT("reserved unset: " << socket->get_fd() << " " << cat);
}

// 次の kevent の前に, このソケットを監視対象に追加する
void EventKqueueLoop::reserve_set(ISocketLike *socket, observation_category cat) {
    reserve(socket, cat, true);
}

void EventKqueueLoop::update() {
    std::vector<t_kevent> changelist;
    changelist.reserve(upqueue.size());
    if (upqueue.empty()) {
        return;
    }
    // exec hold
    for (update_queue::size_type i = 0; i < upqueue.size(); ++i) {
        if (upqueue[i].cat == OT_NONE && upqueue[i].in) {
            const t_fd fd = upqueue[i].fd;
            holding_map.insert(socket_map::value_type(fd, upqueue[i].sock));
            DXOUT("HOLDED: " << fd);
        }
    }

    // exec set or unset
    int n = 0;
    for (update_queue::size_type i = 0; i < upqueue.size(); ++i) {
        if (upqueue[i].cat == OT_NONE) {
            continue;
        }
        const t_fd fd                  = upqueue[i].fd;
        const bool in                  = upqueue[i].in;
        const observation_category cat = upqueue[i].cat;
        t_kevent ke;
        ISocketLike *sock = upqueue[i].sock;
        switch (cat) {
            case OT_READ: {
                if (in) {
                    read_map.insert(socket_map::value_type(fd, sock));
                } else {
                    read_map.erase(fd);
                }
                break;
            }
            case OT_WRITE: {
                if (in) {
                    write_map.insert(socket_map::value_type(fd, sock));
                } else {
                    write_map.erase(fd);
                }
                break;
            }
            case OT_EXCEPTION: {
                if (upqueue[i].in) {
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
        if (in) {
            DXOUT("ADDING " << fd << " " << cat);
            EV_SET(&*changelist.rbegin(), fd, filter(cat), EV_ADD, 0, 0, NULL);
        } else if (!in) {
            DXOUT("DELETING " << fd << " " << cat);
            EV_SET(&*changelist.rbegin(), fd, filter(cat), EV_DISABLE, 0, 0, NULL);
        }
        n++;
    }
    if (n > 0) {
        errno     = 0;
        int count = kevent(kq, &*changelist.begin(), changelist.size(), NULL, 0, NULL);
        if (errno) {
            VOUT(count);
            QVOUT(strerror(errno));
            (void)count; // DXOUT を無効化すると`count`がunusedになるため
        }
    }

    // exec unhold
    for (update_queue::size_type i = 0; i < upqueue.size(); ++i) {
        if (upqueue[i].cat == OT_NONE && !upqueue[i].in) {
            const t_fd fd = upqueue[i].fd;
            holding_map.erase(fd);
            delete upqueue[i].sock;
            DXOUT("UNHOLDED: " << fd);
        }
    }
    upqueue.clear();
}
