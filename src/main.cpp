#include "event/Eventselectloop.hpp"
#include "socket/SocketConnected.hpp"
#include "socket/SocketListening.hpp"
#include "string"
#include <iostream>

int main(void) {
    IObserver *i_observer = new EventSelectLoop;
    (void)i_observer;
}
