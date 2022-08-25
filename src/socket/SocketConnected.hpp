#ifndef SOCKET_CONNECTED_HPP
#define SOCKET_CONNECTED_HPP
#include "../Interfaces.hpp"
#include "ASocket.hpp"

class SocketListening;

// [通信可能ソケットクラス]
// [責務]
// - ソケット1つを保持し, オブジェクト破壊時もしくはその前にソケットを閉じること
// - ソケットを通じてバイト列の送受信を行うこと
class SocketConnected : public ASocket, public IDataSender {
private:
    // クライアント用コンストラクタ
    SocketConnected(t_socket_domain sdomain, t_socket_type stype);
    // サーバ用コンストラクタ
    // accept によって生成される
    SocketConnected(t_fd fd, SocketListening &listening);

public:
    SocketConnected(const SocketConnected &other);
    SocketConnected &operator=(const SocketConnected &rhs);

    // クライアント用connect関数ラッパ
    static SocketConnected *connect(t_socket_domain sdomain, t_socket_type stype, t_port port);

    // サーバ用factory関数
    static SocketConnected *wrap(t_fd fd, SocketListening &listening);

    t_fd get_fd() const throw();
    ssize_t send(const void *buffer, size_t len, int flags) throw();
    ssize_t receive(void *buffer, size_t len, int flags) throw();

    int shutdown() throw();
    int shutdown_write() throw();
    int shutdown_read() throw();
};

#endif
