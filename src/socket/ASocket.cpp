#include "ASocket.hpp"

ASocket::ASocket(t_socket_domain sdomain, t_socket_type stype) : port(0) {
    int d = sockdomain(sdomain);
    int t = socktype(stype);

    t_fd sock = socket(d, t, 0);
    if (sock == -1) {
        throw std::runtime_error("failed to initialize asocket");
    }
    fd     = sock;
    domain = sdomain;
    type   = stype;

    int yes;
    yes = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char *)&yes, sizeof(yes));
    yes = 1;
    setsockopt(sock, SOL_SOCKET, SO_NOSIGPIPE, (const char *)&yes, sizeof(yes));
}

ASocket::ASocket(t_fd sock_fd, t_socket_domain sdomain, t_socket_type stype) throw() : fd(sock_fd), port(0) {
    domain = sdomain;
    type   = stype;
    int yes;
    yes = 1;
    setsockopt(sock_fd, SOL_SOCKET, SO_NOSIGPIPE, (const char *)&yes, sizeof(yes));
}

ASocket::ASocket(const ASocket &other) {
    *this = other;
}

ASocket &ASocket::operator=(const ASocket &rhs) {
    if (this != &rhs) {
        fd     = rhs.fd;
        domain = rhs.domain;
        type   = rhs.type;
    }
    return *this;
}

ASocket::~ASocket() {
    destroy();
}

void ASocket::set_nonblock() {
    int rv;
    rv = fcntl(fd, F_SETFL, O_NONBLOCK);
    if (rv < 0) {
        throw std::runtime_error("failed to fcntl");
    }
}

t_fd ASocket::get_fd() const throw() {
    return fd;
}

t_socket_domain ASocket::get_domain() const throw() {
    return domain;
}

t_socket_type ASocket::get_type() const throw() {
    return type;
}

t_port ASocket::get_port() const throw() {
    return port;
}

void ASocket::destroy() throw() {
    close(fd);
}
