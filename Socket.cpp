#include "Socket.hpp"

#include <unistd.h>
#include "stdexcept"

Socket::Socket(int fd) : fd_(fd) {
  if (fd == -1) {
    throw std::runtime_error("failed to construct Socket");
  }
}

Socket::~Socket() { close(fd_); }

int Socket::get_fd(void) const { return fd_; }
