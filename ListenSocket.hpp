#ifndef LISTENSOCKET_HPP
#define LISTENSOCKET_HPP
#include "Socket.hpp"

class ListenSocket : public Socket
{
public:
	// こいつがbind, listenするか
	ListenSocket();
	~ListenSocket();

// 実際にやり取りするようのソケットが戻り値
//	Socket* accept(void);

private:

};

#endif
