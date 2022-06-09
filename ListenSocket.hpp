#ifndef LISTENSOCKET_HPP
#define LISTENSOCKET_HPP

#include "Socket.hpp"
#include <arpa/inet.h>
#include <cstring>
#include <netdb.h>
#include <netinet/in.h>
#include <stdexcept>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

class ListenSocket : public Socket {
public:
    // こいつがbind, listenするか
    ListenSocket(const char *port) : Socket(try_open_listen_fd(port)) {
    }
    ~ListenSocket();


private:
    static addrinfo init_addrinfo() {
        addrinfo ret;
        memset(&ret, 0, sizeof ret);
        ret.ai_socktype = SOCK_STREAM;
        ret.ai_flags = AI_PASSIVE;
        ret.ai_flags |= AI_NUMERICSERV;
        ret.ai_flags |= AI_ADDRCONFIG;
        return ret;
    }

    static int try_bind_fd(const char *port, addrinfo hints) {
        addrinfo *listp;

        if (getaddrinfo(NULL, port, &hints, &listp)) {
            throw std::runtime_error("failed to getaddrinfo");
        }
        for (addrinfo *p = listp; p != NULL; p = p->ai_next) {
            int fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
            if (fd < 0) {
                continue;
            }
            int opt = 1;
            setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const void *) &opt, sizeof opt);
            if (::bind(fd, p->ai_addr, p->ai_addrlen) == 0) {
                freeaddrinfo(listp);
                return fd;
            }
            close(fd);
        }
        throw std::runtime_error("failed to bind fd");
    }
    static int try_open_listen_fd(const char *port) {
        int fd = try_bind_fd(port, init_addrinfo());

        if (::listen(fd, SOMAXCONN) < 0) {
            throw std::runtime_error("failed to listen fd");
        }
        return fd;
    }


    // 実際にやり取りするようのソケットが戻り値
    //	Socket* accept(void);
};

#endif
