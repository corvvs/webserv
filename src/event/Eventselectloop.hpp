#ifndef EVENT_SELECT_LOOP_HPP
#define EVENT_SELECT_LOOP_HPP

#include "../interface/IObserver.hpp"
#include "../interface/ISocketlike.hpp"
#include <cerrno>
#include <ctime>
#include <map>
#include <sys/select.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

class ISocketLike;

// selectを使ったソケット監視者の実装
class EventSelectLoop : public IObserver {
private:
    typedef std::map<t_fd, ISocketLike *> socket_map;
    typedef std::vector<t_socket_reservation> update_queue;
    socket_map read_map;
    socket_map write_map;
    socket_map exception_map;
    socket_map holding_map;
    update_queue up_queue;

    void prepare_fd_set(socket_map &sockmap, fd_set *sockset);
    void scan_fd_set(socket_map &sockmap, fd_set *sockset, t_time_epoch_ms now, IObserver::observation_category cat);
    void reserve(ISocketLike *socket, observation_category cat, bool in);
    void update();

    void watch(ISocketLike *socket, observation_category map_type);
    void unwatch(ISocketLike *socket, observation_category map_type);

    void destroy_all(socket_map &m);

public:
    EventSelectLoop();
    ~EventSelectLoop();

    void loop();
    void reserve_hold(ISocketLike *socket);
    void reserve_unhold(ISocketLike *socket);
    void reserve_unset(ISocketLike *socket, observation_category cat);
    void reserve_set(ISocketLike *socket, observation_category to);
};

#endif
