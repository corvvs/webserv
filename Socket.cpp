#include "Socket.hpp"

#include <unistd.h>

Socket::Socket(int fd) : fd_(fd)
{
}

Socket::~Socket()
{
	close(fd_);
}

int Socket::get_fd(void) const
{
	return fd_;
}
