#include "event/Eventkqueueloop.hpp"
#include "event/Eventpollloop.hpp"
#include "event/Eventselectloop.hpp"
#include "server/HTTPServer.hpp"
#include "utils/MIME.hpp"
#define BUILD_BUG_ON(condition) ((void)sizeof(char[1 - 2 * !!(condition)]))

static void assert_sizeoftype() {
    BUILD_BUG_ON(sizeof(u8t) != 1);
    BUILD_BUG_ON(sizeof(u16t) != 2);
    BUILD_BUG_ON(sizeof(u32t) != 4);
    BUILD_BUG_ON(sizeof(u64t) != 8);
}

int main(int argc, char **argv) {
    assert_sizeoftype();

    if (argc == 3 && strcmp(argv[1], "-t") == 0) {
        return HTTPServer::test_configuration(argv[2]);
    }

    if (argc > 2) {
        std::cerr << "Usage: ./webserv [filename] [-t filename]" << std::endl;
        return EXIT_FAILURE;
    }
    const std::string &conf_path = (argc == 2) ? argv[1] : "./conf/default.conf";

    // HTTPServer http_server(new EventKqueueLoop());
    // HTTPServer http_server(new EventSelectLoop());
    HTTPServer http_server(new EventPollLoop());
    try {
        HTTP::MIME::setup_mime_map();
        http_server.init(conf_path);
        http_server.run();
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}
