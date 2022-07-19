#include "SocketUNIX.hpp"
#include "../utils/test_common.hpp"

SocketUNIX::SocketUNIX(int fd) : ASocket(fd, SD_UNIX, ST_STREAM) {
    DXOUT("SockUNIX: " << fd);
}

std::pair<SocketUNIX *, t_fd> SocketUNIX::socket_pair() {
    int fds[2];
    int rv = socketpair(AF_UNIX, SOCK_STREAM, 0, fds);
    if (rv < 0) {
        throw std::runtime_error("failed to make sockerpair");
    }
    return std::pair<SocketUNIX *, t_fd>(new SocketUNIX(fds[0]), fds[1]);
}

ssize_t SocketUNIX::send(const void *buffer, size_t len, int flags) {
    return ::send(fd, buffer, len, flags);
}

ssize_t SocketUNIX::receive(void *buffer, size_t len, int flags) {
    return ::recv(fd, buffer, len, flags);
}

int SocketUNIX::shutdown() {
    return ::shutdown(get_fd(), SHUT_RDWR);
}

int SocketUNIX::shutdown_write() {
    return ::shutdown(get_fd(), SHUT_WR);
}

int SocketUNIX::shutdown_read() {
    return ::shutdown(get_fd(), SHUT_RD);
}