#ifndef EVENT_POLL_LOOP_HPP
#define EVENT_POLL_LOOP_HPP
#include "../interface/IObserver.hpp"
#include "../interface/ISocketlike.hpp"
#include <errno.h>
#include <map>
#include <poll.h>
#include <set>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

class ISocketLike;

typedef short t_poll_eventmask;

// pollを使ったソケット監視者の実装
class EventPollLoop : public IObserver {
private:
    typedef std::vector<pollfd> fd_vector;
    typedef std::map<t_fd, ISocketLike *> socket_map;
    typedef std::map<t_fd, size_t> index_map;
    typedef std::set<size_t> gap_set;
    typedef std::vector<t_socket_reservation> update_queue;

    fd_vector fds;
    socket_map sockmap;
    index_map indexmap;
    gap_set gapset;
    int nfds;

    update_queue clearqueue;
    update_queue movequeue;
    update_queue setqueue;

    t_poll_eventmask mask(t_observation_target t);

    void reserve(ISocketLike *socket, t_observation_target from, t_observation_target to);
    void update();

public:
    EventPollLoop();
    ~EventPollLoop();

    void loop();
    void reserve_clear(ISocketLike *socket, t_observation_target from);
    void reserve_set(ISocketLike *socket, t_observation_target to);
    void reserve_transit(ISocketLike *socket, t_observation_target from, t_observation_target to);
};

#endif
