#include "SocketType.hpp"

int sockdomain(t_socket_domain d) throw() {
    switch (d) {
        case SD_IP4:
            return AF_INET;
        case SD_IP6:
            return AF_INET6;
        case SD_UNIX:
            return AF_UNIX;
        default:
            assert(false);
            return AF_UNSPEC;
    }
}

int socktype(t_socket_type t) throw() {
    switch (t) {
        case ST_TCP:
        case ST_STREAM:
            return SOCK_STREAM;
        case ST_UDP:
            return SOCK_DGRAM;
        default:
            assert(false);
            return AF_UNSPEC;
    }
}
