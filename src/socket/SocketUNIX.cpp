#include "SocketUNIX.hpp"

SocketUNIX::SocketUNIX(int fd) : ASocket(fd, SD_UNIX, ST_STREAM) {}

std::pair<SocketUNIX *, t_fd> SocketUNIX::socket_pair() {
    int fds[2];
    int rv = socketpair(AF_UNIX, SOCK_STREAM, 0, fds);
    if (rv < 0) {
        throw std::runtime_error("failed to make sockerpair");
    }
    return std::pair<SocketUNIX *, t_fd>(new SocketUNIX(fds[0]), fds[1]);
}
