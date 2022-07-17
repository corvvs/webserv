#ifndef CONTEXT_HPP
#define CONTEXT_HPP
#include <map>
#include <set>
#include <string>
#include <vector>

namespace config {

typedef std::string host_type;
typedef int port_type;
typedef std::pair<host_type, port_type> host_port_pair;

class ContextMain {
public:
    ContextMain(void);
    ~ContextMain(void);
    long client_max_body_size;
    bool autoindex;
    std::string root;
    std::vector<std::string> indexes;
    std::map<int, std::string> error_pages;
};

class ContextServer {
public:
    ContextServer(const ContextMain &main);
    ~ContextServer(void);

    // 継承する
    long client_max_body_size;
    bool autoindex;
    std::string root;
    std::vector<std::string> indexes;
    std::map<int, std::string> error_pages;

    // 継承しない
    std::vector<std::string> server_names;
    std::vector<host_port_pair> host_ports;
    std::vector<class ContextLocation> locations;
    std::pair<int, std::string> redirect;
    std::string upload_store;
    bool default_server;

    // 継承するか判定するときに使用する
    std::map<std::string, bool> defined_;
};

enum Methods { GET, POST, DELETE };
class ContextLimitExcept {
public:
    ContextLimitExcept(void);
    ~ContextLimitExcept(void);

    std::set<enum Methods> allowed_methods;
};

class ContextLocation {
public:
    ContextLocation(const ContextServer &server);
    ~ContextLocation(void);

    /// 継承する
    long client_max_body_size;
    bool autoindex;
    std::string root;
    std::vector<std::string> indexes;
    std::map<int, std::string> error_pages;
    std::pair<int, std::string> redirect;

    /// 継承しない
    std::string path;
    std::vector<class ContextLocation> locations;
    ContextLimitExcept limit_except;

    // 継承するか判定するときに使用する
    std::map<std::string, bool> defined_;
};

} // namespace config
#endif
