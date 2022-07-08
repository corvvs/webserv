#include "Eventpollloop.hpp"
#include "../utils/test_common.hpp"

EventPollLoop::EventPollLoop() : nfds(0) {}

EventPollLoop::~EventPollLoop() {
    for (socket_map::iterator it = sockmap.begin(); it != sockmap.end(); it++) {
        delete it->second;
    }
}

// イベントループ
void EventPollLoop::loop() {
    while (true) {
        update();

        const int count = poll(&*fds.begin(), fds.size(), 10 * 1000);

        if (count < 0) {
            throw std::runtime_error("poll error");
        } else if (count == 0) {
            t_time_epoch_ms now = WSTime::get_epoch_ms();
            for (socket_map::iterator it = sockmap.begin(); it != sockmap.end(); it++) {
                size_t i = indexmap[it->first];
                if (fds[i].fd >= 0) {
                    it->second->timeout(*this, now);
                }
            }
        } else {
            for (socket_map::iterator it = sockmap.begin(); it != sockmap.end(); it++) {
                size_t i = indexmap[it->first];
                if (fds[i].fd >= 0 && fds[i].revents) {
                    DXOUT("[S]FD-" << it->first << ": revents: " << fds[i].revents);
                    if (mask(IObserver::OT_READ) & fds[i].revents) {
                        it->second->notify(*this, OT_READ);
                    }
                    if (mask(IObserver::OT_WRITE) & fds[i].revents) {
                        it->second->notify(*this, OT_WRITE);
                    }
                    if (mask(IObserver::OT_EXCEPTION) & fds[i].revents) {
                        it->second->notify(*this, OT_EXCEPTION);
                    }
                }
            }
        }
    }
}

void EventPollLoop::reserve(ISocketLike *socket, observation_category cat, bool in) {
    t_socket_reservation pre = {socket->get_fd(), socket, cat, in};
    if (cat == OT_NONE && in) {
        holdqueue.push_back(pre);
    } else if (cat == OT_NONE && !in) {
        unholdqueue.push_back(pre);
    } else {
        movequeue.push_back(pre);
    }
}

void EventPollLoop::reserve_hold(ISocketLike *socket) {
    reserve(socket, OT_NONE, true);
}

void EventPollLoop::reserve_unhold(ISocketLike *socket) {
    reserve(socket, OT_NONE, false);
}

// 次の kevent の前に, このソケットを監視対象から除外する
void EventPollLoop::reserve_unset(ISocketLike *socket, observation_category from) {
    reserve(socket, from, false);
}

// 次の kevent の前に, このソケットを監視対象に追加する
void EventPollLoop::reserve_set(ISocketLike *socket, observation_category to) {
    reserve(socket, to, true);
}

t_poll_eventmask EventPollLoop::mask(observation_category t) {
    switch (t) {
        case OT_READ:
            return POLLIN;
        case OT_WRITE:
            return POLLOUT;
        case OT_EXCEPTION:
            return POLLPRI;
        case OT_NONE:
            return 0;
        default:
            throw std::runtime_error("unexpected map_type");
    }
}

// ソケットの監視状態変更予約を実施する
void EventPollLoop::update() {
    // exec unhold

    EventPollLoop::update_queue::iterator it;
    for (it = unholdqueue.begin(); it != unholdqueue.end(); it++) {
        ISocketLike *sock = it->sock;
        size_t i          = indexmap[sock->get_fd()];
        fds[i].fd         = -1;
        sockmap.erase(sock->get_fd());
        indexmap.erase(sock->get_fd());
        gapset.insert(i);
        delete sock;
        nfds--;
    }
    // exec hold
    for (it = holdqueue.begin(); it != holdqueue.end(); it++) {
        ISocketLike *sock = it->sock;
        size_t i;
        if (gapset.empty()) {
            pollfd p = {};
            p.fd     = sock->get_fd();
            i        = fds.size();
            fds.push_back(p);
        } else {
            i         = *(gapset.begin());
            fds[i].fd = sock->get_fd();
            gapset.erase(gapset.begin());
        }
        fds[i].events            = 0;
        sockmap[sock->get_fd()]  = sock;
        indexmap[sock->get_fd()] = i;
        nfds++;
    }
    // exec set / unset
    for (it = movequeue.begin(); it != movequeue.end(); it++) {
        ISocketLike *sock = it->sock;
        size_t i          = indexmap[sock->get_fd()];
        fds[i].events     = fds[i].events | mask(it->cat);
        if (!it->in) {
            fds[i].events     = fds[i].events ^ mask(it->cat);
        }
    }
    unholdqueue.clear();
    movequeue.clear();
    holdqueue.clear();

    // exec unhold

}
