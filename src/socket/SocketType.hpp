#ifndef SOCKET_TYPE_HPP
#define SOCKET_TYPE_HPP

#include <arpa/inet.h>
#include <cstdlib>
#include <exception>
#include <fcntl.h>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <stdexcept>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

enum t_socket_domain { SD_IP4, SD_IP6 };

enum t_socket_type { ST_TCP, ST_UDP };

typedef int t_fd;
typedef unsigned t_port;
typedef unsigned t_addressv4;

class ISocketLike;

int sockdomain(t_socket_domain d);

int socktype(t_socket_type t);

#endif