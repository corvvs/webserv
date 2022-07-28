#include "SocketConnected.hpp"
#include "SocketListening.hpp"
#include "strings.h"

SocketConnected::SocketConnected(t_socket_domain sdomain, t_socket_type stype) : ASocket(sdomain, stype) {}

SocketConnected::SocketConnected(t_fd accepted_fd, SocketListening &listening_socket)
    : ASocket(accepted_fd, listening_socket.get_domain(), listening_socket.get_type()) {
    port = listening_socket.get_port();
}

SocketConnected &SocketConnected::operator=(const SocketConnected &rhs) {
    static_cast<ASocket &>(*this) = static_cast<const ASocket &>(rhs);
    return *this;
}

SocketConnected *SocketConnected::connect(t_socket_domain sdomain, t_socket_type stype, t_port port) {
    SocketConnected *sock = new SocketConnected(sdomain, stype);
    t_fd fd               = sock->fd;

    sockaddr_in sa;
    bzero(&sa, sizeof(sa));
    int d         = sockdomain(sdomain);
    sa.sin_family = d;
    sa.sin_port   = htons(port);

    // localhost の IP アドレスを引く
    hostent *hostent = gethostbyname("localhost");
    if (hostent == NULL) {
        throw std::runtime_error("failed to gethostbyname");
    }
    std::memcpy(&sa.sin_addr, hostent->h_addr_list[0], sizeof(sa.sin_addr));
    // 接続
    if (::connect(fd, (struct sockaddr *)&sa, sizeof(struct sockaddr_in)) == -1) {
        throw std::runtime_error("failed to connect");
    }
    sock->port = port;
    return sock;
}

SocketConnected *SocketConnected::wrap(t_fd fd, SocketListening &listening) {
    SocketConnected *sock = new SocketConnected(fd, listening);
    sock->set_nonblock();
    return sock;
}

ssize_t SocketConnected::send(const void *buffer, size_t len, int flags) {
    return ::send(fd, buffer, len, flags);
}

ssize_t SocketConnected::receive(void *buffer, size_t len, int flags) {
    return ::recv(fd, buffer, len, flags);
}

int SocketConnected::shutdown() {
    return ::shutdown(get_fd(), SHUT_RDWR);
}

int SocketConnected::shutdown_write() {
    return ::shutdown(get_fd(), SHUT_WR);
}

int SocketConnected::shutdown_read() {
    return ::shutdown(get_fd(), SHUT_RD);
}