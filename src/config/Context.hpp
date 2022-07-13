#ifndef CONTEXT_HPP
#define CONTEXT_HPP
#include <map>
#include <set>
#include <string>
#include <vector>

/**
 * [directives]
 * autoindex            bool
 * error_page           vector<map<int, string> > -> (status, path)
 * index                vector<string>
 * root                 string
 * client_max_body_size int
 * host_port            pair<string, int>
 * redirect             pair<int, string> -> (status, path)
 * server_name          vector<string>
 * upload_store         string
 * default_server       bool
 */

namespace config {

class IContext {
public:
    virtual ~IContext() {}
};

typedef std::string host_type;
typedef int port_type;
typedef std::pair<host_type, port_type> host_port_pair;

class ContextMain : public IContext {
public:
    ContextMain(void);
    ~ContextMain(void);
    int client_max_body_size;
    bool autoindex;
    std::string root;
    std::vector<std::string> indexes;
    std::map<int, std::string> error_pages;

private:
};

class ContextServer : public IContext {
public:
    ContextServer(const ContextMain &main);
    ~ContextServer(void);

    // mainから継承する
    int client_max_body_size;
    bool autoindex;
    std::string root;
    std::vector<std::string> indexes;
    std::map<int, std::string> error_pages;

    // 新たに追加する
    std::vector<std::string> server_names;
    std::string upload_store;
    std::pair<int, std::string> redirect;

    // std::vector<std::pair<std::string, int>> host_ports;
    std::vector<host_port_pair> host_ports;

    bool default_server;

    // 内部にlocationを持つ可能性がある
    std::vector<class ContextLocation> locations;

private:
};

enum Methods { GET, POST, DELETE };
class ContextLimitExcept : public IContext {
public:
    ContextLimitExcept(void);
    ~ContextLimitExcept(void);

    std::set<enum Methods> allowed_methods;
};

class ContextLocation : public IContext {
public:
    ContextLocation(const ContextServer &server);
    ~ContextLocation(void);

    /// 親から継承する
    int client_max_body_size;
    bool autoindex;
    std::string root; //
    std::vector<std::string> indexes;
    std::map<int, std::string> error_pages;
    std::pair<int, std::string> redirect;

    /// 継承しない
    std::string path;
    std::vector<class ContextLocation> locations;
    ContextLimitExcept *limit_except; // 持たない可能性もあるのでポインタにする(デストラクタでデリートする)

private:
};

} // namespace config
#endif
