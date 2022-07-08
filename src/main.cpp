#include "event/Eventkqueueloop.hpp"
#include "event/Eventpollloop.hpp"
#include "event/Eventselectloop.hpp"
#include "server/HTTPServer.hpp"
#define BUILD_BUG_ON(condition) ((void)sizeof(char[1 - 2 * !!(condition)]))

void assert_sizeoftype() {
    BUILD_BUG_ON(sizeof(u8t) != 1);
    BUILD_BUG_ON(sizeof(u16t) != 2);
    BUILD_BUG_ON(sizeof(u32t) != 4);
    BUILD_BUG_ON(sizeof(u64t) != 8);
}

int main() {
    assert_sizeoftype();

    HTTPServer  http_server(new EventKqueueLoop());
    // HTTPServer http_server(new EventPollLoop());
    // HTTPServer  http_server(new EventSelectLoop());

    http_server.listen(SD_IP4, ST_TCP, 8080);
    http_server.listen(SD_IP4, ST_TCP, 8081);
    http_server.listen(SD_IP4, ST_TCP, 8082);
    http_server.listen(SD_IP4, ST_TCP, 8083);
    http_server.listen(SD_IP4, ST_TCP, 8084);

    http_server.run();
}
