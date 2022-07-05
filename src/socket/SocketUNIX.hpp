#ifndef SOCKETUNIX_HPP
#define SOCKETUNIX_HPP
#include "ASocket.hpp"
#include <utility>

// [UNIXソケットクラス]
// [責務]
class SocketUNIX : public ASocket {
private:
    SocketUNIX();
    SocketUNIX(t_socket_domain sdomain, t_socket_type stype);

public:
    SocketUNIX(int fd);
    SocketUNIX(const SocketUNIX &other);
    SocketUNIX &operator=(const SocketUNIX &rhs);

    static std::pair<SocketUNIX *, t_fd> socket_pair();
};

#endif
