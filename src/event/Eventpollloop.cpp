#include "Eventpollloop.hpp"
#include "../utils/test_common.hpp"

const t_time_epoch_ms EventPollLoop::timeout_interval = 1 * 1000;

EventPollLoop::EventPollLoop() : nfds(0), latest_timeout_checked(WSTime::get_epoch_ms()) {}

EventPollLoop::~EventPollLoop() {
    DXOUT("destroying... " << sockmap.size());
    for (socket_map::iterator it = sockmap.begin(); it != sockmap.end(); ++it) {
        DXOUT("delete " << it->second);
        delete it->second;
    }
    DXOUT("destroyed");
}

// イベントループ
void EventPollLoop::loop() {
    while (true) {
        update();

        const int count = poll(&*fds.begin(), fds.size(), 10 * 1000);

        if (count < 0) {
            throw std::runtime_error("poll error");
        }
        t_time_epoch_ms now = WSTime::get_epoch_ms();
        if (count == 0 || now - latest_timeout_checked > timeout_interval) {
            for (socket_map::iterator it = sockmap.begin(); it != sockmap.end(); ++it) {
                index_map::mapped_type i = indexmap[it->first];
                if (fds[i].fd >= 0) {
                    it->second->notify(*this, OT_TIMEOUT, now);
                }
            }
            latest_timeout_checked = now;
        }
        if (count > 0) {
            for (socket_map::iterator it = sockmap.begin(); it != sockmap.end(); ++it) {
                index_map::mapped_type i = indexmap[it->first];
                if (fds[i].fd >= 0 && fds[i].revents) {
                    DXOUT("[S]FD-" << it->first << ": revents: " << fds[i].revents);
                    if (mask(IObserver::OT_READ) & fds[i].revents) {
                        it->second->notify(*this, OT_READ, now);
                    }
                    if (mask(IObserver::OT_WRITE) & fds[i].revents) {
                        it->second->notify(*this, OT_WRITE, now);
                    }
                    if (mask(IObserver::OT_EXCEPTION) & fds[i].revents) {
                        it->second->notify(*this, OT_EXCEPTION, now);
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
    DXOUT("reserve_hold: " << socket->get_fd());
    reserve(socket, OT_NONE, true);
}

void EventPollLoop::reserve_unhold(ISocketLike *socket) {
    DXOUT("reserve_unhold: " << socket->get_fd());
    reserve(socket, OT_NONE, false);
}

// 次の poll の前に, このソケットを監視対象から除外する
void EventPollLoop::reserve_unset(ISocketLike *socket, observation_category cat) {
    reserve(socket, cat, false);
}

// 次の poll の前に, このソケットを監視対象に追加する
void EventPollLoop::reserve_set(ISocketLike *socket, observation_category cat) {
    reserve(socket, cat, true);
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
    for (EventPollLoop::update_queue::size_type i = 0; i < unholdqueue.size(); ++i) {
        const t_fd fd                    = unholdqueue[i].fd;
        const index_map::mapped_type idx = indexmap[fd];

        fds[idx].fd = -1;
        sockmap.erase(fd);
        indexmap.erase(fd);
        gapset.insert(idx);
        delete unholdqueue[i].sock;
        nfds--;
        DXOUT("unholding: " << fd);
    }
    // exec hold
    for (EventPollLoop::update_queue::size_type i = 0; i < holdqueue.size(); ++i) {
        const t_fd fd = holdqueue[i].fd;
        index_map::mapped_type idx;
        if (gapset.empty()) {
            pollfd p = {};
            p.fd     = fd;
            idx      = fds.size();
            fds.push_back(p);
        } else {
            idx         = *(gapset.begin());
            fds[idx].fd = fd;
            gapset.erase(gapset.begin());
        }
        fds[idx].events = 0;
        sockmap[fd]     = holdqueue[i].sock;
        indexmap[fd]    = idx;
        DXOUT("holding: " << fd);
        nfds++;
    }
    // exec set / unset
    for (EventPollLoop::update_queue::size_type i = 0; i < movequeue.size(); ++i) {
        const t_fd fd                    = movequeue[i].fd;
        const index_map::mapped_type idx = indexmap[fd];
        fds[idx].events                  = fds[idx].events | mask(movequeue[i].cat);
        if (!movequeue[i].in) {
            fds[idx].events = fds[idx].events ^ mask(movequeue[i].cat);
        }
    }
    unholdqueue.clear();
    movequeue.clear();
    holdqueue.clear();
}
