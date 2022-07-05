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
 * return               pair<int, string> -> (status, path)
 * server_name          vector<string>
 * upload_store         string
 */

class IContext {
public:
    virtual ~IContext() {}
};

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
    int port;
    std::string host;
    std::vector<std::string> server_names;
    std::string upload_store;
    std::pair<int, std::string> redirect;

    // 内部にlocationを持つ可能性がある
    std::vector<class ContextLocation> locations;

private:
};

enum Methods { GET, POST, DELETE };
class ContextLimitExpect : public IContext {
public:
    ContextLimitExpect(void);

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
    ContextLimitExpect *limit_expect; // 持たない可能性もあるのでポインタにする(デストラクタでデリートする)

private:
};

} // namespace config
#endif

