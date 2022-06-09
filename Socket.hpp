#ifndef SOCKET_HPP
#define SOCKET_HPP

class Socket
{
public:
	Socket(int fd);
	virtual ~Socket();

	int get_fd(void) const;

private:
	int fd_;
};

#endif
