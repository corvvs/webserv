#ifndef ASOCKET_HPP
#define ASOCKET_HPP
#include "SocketType.hpp"

// [抽象ソケットクラス]
// ソケットを保持するクラス, の抽象クラス.
// listenするか通信するかで2種類に分かれる.
class ASocket {
protected:
    t_fd fd;
    t_socket_domain domain;
    t_socket_type type;
    t_port port;

private:
    // コンストラクタの直接呼び出しは禁止
    // Socketはfactoryメソッドbind,
    // connectおよびインスタンスメソッドacceptによってのみ生成される

protected:
    ASocket(t_socket_domain sdomain, t_socket_type stype);

    ASocket(int fd, t_socket_domain sdomain, t_socket_type stype);

    ASocket(const ASocket &other);
    void destroy();

public:
    virtual ~ASocket();
    ASocket &operator=(const ASocket &rhs);

    void set_nonblock();
    int get_fd() const;
    t_socket_domain get_domain() const;
    t_socket_type get_type() const;
    t_port get_port() const;
};

#endif