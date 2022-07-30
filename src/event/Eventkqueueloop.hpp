#ifndef EVENT_KQUEUE_LOOP_HPP
#define EVENT_KQUEUE_LOOP_HPP

#ifdef __APPLE__

#include "../interface/IObserver.hpp"
#include "../interface/ISocketLike.hpp"
#include <cerrno>
#include <ctime>
#include <map>
#include <set>
#include <sys/event.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

class ISocketLike;

// kqueueを使ったソケット監視者の実装
class EventKqueueLoop : public IObserver {
private:
    typedef std::map<t_fd, ISocketLike *> socket_map;
    typedef std::vector<t_socket_reservation> update_queue;
    typedef struct kevent t_kevent;
    typedef std::vector<t_kevent> event_list;
    typedef short t_kfilter;
    typedef int t_kqueue;

    socket_map read_map;
    socket_map write_map;
    socket_map exception_map;
    update_queue upqueue;
    socket_map holding_map;
    event_list evlist;
    static const int nev;
    t_kqueue kq;
    t_time_epoch_ms latest_timeout_checked;

    t_kfilter filter(observation_category t);
    observation_category filter_to_cat(t_kfilter f);

    void reserve(ISocketLike *socket, observation_category cat, bool in);
    void update();

public:
    EventKqueueLoop();
    ~EventKqueueLoop();

    const static t_time_epoch_ms timeout_interval;
    void loop();
    void reserve_hold(ISocketLike *socket);
    void reserve_unhold(ISocketLike *socket);
    void reserve_unset(ISocketLike *socket, observation_category cat);
    void reserve_set(ISocketLike *socket, observation_category to);
};

#endif

#endif
