#include "Eventpollloop.hpp"

EventPollLoop::EventPollLoop(): nfds(0) {
}

EventPollLoop::~EventPollLoop() {
    for (socket_map::iterator it = sockmap.begin(); it != sockmap.end(); it++) {
        delete it->second;
    }
}

// イベントループ
void    EventPollLoop::loop() {
    while (1) {
        update();
        debug_monitor();

        int count = poll(&*fds.begin(), fds.size(), 10 * 1000);

        if (count < 0) {
            throw std::runtime_error("poll error");
        } else if (count == 0) {
            t_time_epoch_ms now = WSTime::get_epoch_ms();
            for (socket_map::iterator it = sockmap.begin(); it != sockmap.end(); it++) {
                int i = indexmap[it->first];
                if (fds[i].fd >= 0) {
                    it->second->timeout(*this, now);
                }
            }
        } else {
            for (socket_map::iterator it = sockmap.begin(); it != sockmap.end(); it++) {
                int i = indexmap[it->first];
                if (fds[i].fd >= 0 && fds[i].revents) {
                    std::cout << "[S]FD-" << it->first << ": revents: " << fds[i].revents << std::endl;
                    it->second->notify(*this);
                }
            }
        }
    }
}

void    EventPollLoop::debug_monitor() {
    // 監視状態の表示
    std::cout << "[S] polling count: " << nfds << std::endl;
    std::cout << "[S]";
    for (fd_vector::iterator it = fds.begin(); it != fds.end(); it++) {
        std::cout << " ";
        if (it->fd >= 0) {
            std::cout << it->fd;
        } else {
            std::cout << "xx";
        }
        std::cout << ":" << it->events;
    }
    std::cout << std::endl;
}

void    EventPollLoop::reserve(ISocketLike* socket, t_socket_operation from, t_socket_operation to) {
    t_socket_reservation  pre = {socket, from, to};
    if (from != SHMT_NONE && to == SHMT_NONE) {
        clearqueue.push_back(pre);
    }
    if (from != SHMT_NONE && to != SHMT_NONE) {
        movequeue.push_back(pre);
    }
    if (from == SHMT_NONE && to != SHMT_NONE) {
        setqueue.push_back(pre);
    }
}

// このソケットを監視対象から除外する
// (その際ソケットはdeleteされる)
void    EventPollLoop::reserve_clear(ISocketLike* socket, t_socket_operation from) {
    reserve(socket, from, SHMT_NONE);
}

// このソケットを監視対象に追加する
void    EventPollLoop::reserve_set(ISocketLike* socket, t_socket_operation to) {
    reserve(socket, SHMT_NONE, to);
}

// このソケットの監視方法を変更する
void    EventPollLoop::reserve_transit(ISocketLike* socket, t_socket_operation from, t_socket_operation to) {
    reserve(socket, from, to);
}

t_poll_eventmask    EventPollLoop::mask(t_socket_operation t) {
    switch (t) {
    case SHMT_READ:
        return POLLIN;
    case SHMT_WRITE:
        return POLLOUT;
    case SHMT_EXCEPTION:
        return POLLPRI;
    default:
        return 0;
    }
}


// ソケットの監視状態変更予約を実施する
void    EventPollLoop::update() {
    EventPollLoop::update_queue::iterator   it;
    for (it = clearqueue.begin(); it != clearqueue.end(); it++) {
        ISocketLike* sock = it->sock;
        int i = indexmap[sock->get_fd()];
        fds[i].fd = -1;
        sockmap.erase(sock->get_fd());
        indexmap.erase(sock->get_fd());
        gapset.insert(i);
        delete sock;
        nfds--;
    }
    for (it = movequeue.begin(); it != movequeue.end(); it++) {
        ISocketLike* sock = it->sock;
        int i = indexmap[sock->get_fd()];
        fds[i].events = mask(it->to);
    }
    for (it = setqueue.begin(); it != setqueue.end(); it++) {
        ISocketLike* sock = it->sock;
        int i;
        if (gapset.empty()) {
            pollfd p = {};
            p.fd = sock->get_fd();
            i = fds.size();
            fds.push_back(p);
        } else {
            i = *(gapset.begin());
            fds[i].fd = sock->get_fd();
            gapset.erase(gapset.begin());
        }
        fds[i].events = mask(it->to);
        sockmap[sock->get_fd()] = sock;
        indexmap[sock->get_fd()] = i;
        nfds++;
    }
    clearqueue.clear();
    movequeue.clear();
    setqueue.clear();
}
