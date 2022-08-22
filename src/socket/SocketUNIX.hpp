#ifndef SOCKETUNIX_HPP
#define SOCKETUNIX_HPP
#include "ASocket.hpp"
#include <utility>

// [UNIXソケットクラス]
// [責務]
class SocketUNIX : public ASocket {
private:
    SocketUNIX(const SocketUNIX &other);
    SocketUNIX &operator=(const SocketUNIX &rhs);

public:
    SocketUNIX(int fd);

    static std::pair<SocketUNIX *, t_fd> socket_pair();

    ssize_t send(const void *buffer, size_t len, int flags) throw();
    ssize_t receive(void *buffer, size_t len, int flags) throw();

    int shutdown() throw();
    int shutdown_write() throw();
    int shutdown_read() throw();
};

#endif
