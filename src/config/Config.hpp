#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <map>
#include <set>
#include <string>
#include <vector>

// 最終的に返すconfigクラスを定義する
namespace config {

class ContextServer;

class Config {
public:
    Config();
    ~Config();

    std::vector<ContextServer> get_config(void);

private:
    std::vector<ContextServer> server_ctx_;
};

/**
 * [directives]
 * allow                string
 * deny                 string
 * autoindex            bool
 * error_page           vector<map<int, string> > -> (status, path)
 * index                vector<string>
 * root                 string
 * client_max_body_size int
 * host                 string
 * port                 int
 * redirect             pair<int, string> -> (status, path)
 * server_name          vector<string>
 * upload_store         string
 * default_server       bool
 */

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

    int client_max_body_size;
    bool autoindex;
    std::string allow;
    std::string deny;
    std::string root;
    std::vector<std::string> indexes;
    std::map<int, std::string> error_pages;
    // private:
};

class ContextServer : public IContext {
public:
    ContextServer(const ContextMain &main);

    // mainから継承する
    int client_max_body_size;
    bool autoindex;
    std::string allow;
    std::string deny;
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

    std::string allow;
    std::string deny;
    std::set<enum Methods> allowed_methods;
};

class ContextLocation : public IContext {
public:
    ContextLocation(const ContextServer &server);

    // serverから継承する
    int client_max_body_size;
    bool autoindex;
    std::string allow;
    std::string deny;
    std::string root;
    std::vector<std::string> indexes;
    std::map<int, std::string> error_pages;
    std::pair<int, std::string> redirect;

    // locationのパス
    std::string path;
    // locationを内包する可能性もある
    std::vector<class ContextLocation> locations;
    ContextLimitExcept *limit_except; // 持たない可能性もあるのでポインタにする(デストラクタでデリートする)

private:
};

} // namespace config
#endif
