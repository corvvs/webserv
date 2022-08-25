#ifndef IDATA_SENDER_HPP
#define IDATA_SENDER_HPP

#include "../socket/SocketType.hpp"

class IDataSender {
public:
    virtual ~IDataSender(){};
    virtual ssize_t send(const void *buffer, size_t len, int flags) throw() = 0;
    virtual t_fd get_fd() const throw()                                     = 0;
};

#endif
