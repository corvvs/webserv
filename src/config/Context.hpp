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
typedef std::string extension_type;
typedef std::string executer_path_type;
typedef std::map<extension_type, executer_path_type> cgi_path_map;

static const int REDIRECT_INITIAL_VALUE = -1;

class ContextMain {
public:
    ContextMain(void);
    ~ContextMain(void);
    long client_max_body_size;
    bool autoindex;
    std::string root;
    std::vector<std::string> indexes;
    std::map<int, std::string> error_pages;

    // 重複していいディレクティブか判定するときに使用する
    std::map<std::string, bool> defined_;
};

class ContextServer {
public:
    ContextServer(void);
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
    std::vector<bool> is_default_servers;

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
    ContextLocation(void);
    ~ContextLocation(void);

    /// 継承する
    long client_max_body_size;
    bool autoindex;
    std::string root;
    std::vector<std::string> indexes;
    std::map<int, std::string> error_pages;
    std::pair<int, std::string> redirect;
    std::string upload_store;

    /// 継承しない
    std::string path;
    std::vector<class ContextLocation> locations;
    ContextLimitExcept limit_except;
    cgi_path_map cgi_paths;
    bool exec_cgi;
    bool exec_delete;
    // 継承するか判定するときに使用する
    std::map<std::string, bool> defined_;
};

} // namespace config
#endif
