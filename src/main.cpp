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

int main(int argc, char **argv) {
    assert_sizeoftype();

    if (argc > 2) {
        std::cerr << "Usage: ./webserv [config_path]" << std::endl;
        return EXIT_FAILURE;
    }
    const std::string &config_path = (argc == 2) ? argv[1] : "./conf/default.conf";

    // HTTPServer  http_server(new EventKqueueLoop());
    HTTPServer http_server(new EventPollLoop());
    // HTTPServer  http_server(new EventSelectLoop());

    // conf群の流れ Channel -> Connection -> RoundTrip -> IRouter

    try {
        http_server.init(config_path);
        http_server.run();
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    // http_server.listen(SD_IP4, ST_TCP, 8080);
    // http_server.listen(SD_IP4, ST_TCP, 8081);
    // http_server.listen(SD_IP4, ST_TCP, 8082);
    // http_server.listen(SD_IP4, ST_TCP, 8083);
    // http_server.listen(SD_IP4, ST_TCP, 8084);
}
