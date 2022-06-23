#include "socket/SocketConnected.hpp"
#include "socket/SocketListening.hpp"
#include "string"
#include <iostream>

int main(void) {
  SocketListening *l = SocketListening::bind(SD_IP4, ST_TCP, 8080);
  l->listen(1024);
  SocketConnected *c;
  do {
    c = l->accept();
} while (c == NULL);
  std::string s(100, 0);
  c->receive((void *)s.data(), s.size(), 0);
  std::cout << s << std::endl;
  delete l;
  delete c;
}
