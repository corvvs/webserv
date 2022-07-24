#ifndef SOCKET_LISTENING_HPP
#define SOCKET_LISTENING_HPP

#include "ASocket.hpp"

class SocketConnected;

// [リスニングソケットクラス]
// [責務]
// - ソケット1つを保持し, オブジェクト破壊時もしくはその前にソケットを閉じること
// - ソケットをリスニング状態にしておくこと
// - 必要に応じて通信可能ソケットクラスを生成すること
class SocketListening : public ASocket {
private:
    SocketListening(t_socket_domain sdomain, t_socket_type stype);

public:
    SocketListening(const SocketListening &other);
    SocketListening &operator=(const SocketListening &rhs);

    static SocketListening *bind(t_socket_domain sdomain, t_socket_type stype, t_port port);
    void listen(int backlog);
    SocketConnected *accept();
};

#endif