#include "SocketListening.hpp"
#include "../utils//ObjectHolder.hpp"
#include "../utils//test_common.hpp"
#include "SocketConnected.hpp"
#include "strings.h"
#include <cerrno>

SocketListening::SocketListening(t_socket_domain sdomain, t_socket_type stype) : ASocket(sdomain, stype) {}

SocketListening &SocketListening::operator=(const SocketListening &rhs) {
    static_cast<ASocket &>(*this) = static_cast<const ASocket &>(rhs);
    return *this;
}

SocketListening *SocketListening::bind(t_socket_domain sdomain, t_socket_type stype, t_port port) {
    ObjectHolder<SocketListening> sock_holder(new SocketListening(sdomain, stype));
    SocketListening *sock = sock_holder.value();
    sock->set_nonblock();
    t_fd fd = sock->fd;

    sockaddr_in sa;
    bzero(&sa, sizeof(sa));
    int d              = sockdomain(sdomain);
    sa.sin_family      = d;
    sa.sin_port        = htons(port);
    sa.sin_addr.s_addr = htonl(INADDR_ANY);
    DXOUT("binding asocket for: " << port << ", " << sa.sin_addr.s_addr << "...");
    if (::bind(fd, (struct sockaddr *)&sa, sizeof(struct sockaddr_in)) == -1) {
        //        std::cerr << strerror(errno) << std::endl;
        throw std::runtime_error("failed to bind a asocket");
    }
    DXOUT("bound asocket.");
    sock->port = port;
    return sock_holder.release();
}

void SocketListening::listen(int backlog) {
    // DOUT() << "making asocket listening in backlog: " << backlog << std::endl;
    if (::listen(fd, backlog) == -1) {
        throw std::runtime_error("failed to listen");
    }
    DXOUT("now listening...");
}

SocketConnected *SocketListening::accept() {
    t_fd accepted_fd = ::accept(fd, NULL, NULL);

    if (accepted_fd < 0) {
        if (errno == EWOULDBLOCK) {
            // これ以上 acceptできない
            return NULL;
        }
        throw std::runtime_error("failed to accept");
    }
    return SocketConnected::wrap(accepted_fd, *this);
}
