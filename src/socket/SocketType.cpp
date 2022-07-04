#include "SocketType.hpp"

int sockdomain(t_socket_domain d) {
    switch (d) {
        case SD_IP4:
            return AF_INET;
        case SD_IP6:
            return AF_INET6;
        default:
            throw std::runtime_error("unexpected socket domain");
    }
}

int socktype(t_socket_type t) {
    switch (t) {
        case ST_TCP:
            return SOCK_STREAM;
        case ST_UDP:
            return SOCK_DGRAM;
        default:
            throw std::runtime_error("unexpected asocket type");
    }
}