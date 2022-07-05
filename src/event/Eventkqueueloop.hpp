#ifndef EVENT_KQUEUE_LOOP_HPP
#define EVENT_KQUEUE_LOOP_HPP

#include "../interface/IObserver.hpp"
#include "../interface/ISocketlike.hpp"
#include <cerrno>
#include <ctime>
#include <map>
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

    socket_map sockmap;
    update_queue upqueue;
    event_list evlist;
    static const int nev;
    t_kqueue kq;

    t_kfilter filter(t_observation_target t);

    void reserve(ISocketLike *socket, t_observation_target from, t_observation_target to);
    void update();

public:
    EventKqueueLoop();
    ~EventKqueueLoop();

    void loop();
    void reserve_clear(ISocketLike *socket, t_observation_target from);
    void reserve_set(ISocketLike *socket, t_observation_target to);
    void reserve_transit(ISocketLike *socket, t_observation_target from, t_observation_target to);
};

#endif
