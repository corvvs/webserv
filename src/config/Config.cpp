#include "Config.hpp"

namespace config {

Config::Config(){};
Config::~Config(){};

std::vector<ContextServer> Config::get_config(void) {
    return server_ctx_;
}

ContextMain::ContextMain(void) {
    client_max_body_size = 1024;
    autoindex            = false;
    //    indexes.push_back("index.html"); TODO: 最後にindex.htmlがなかったら追加する(優先度の関係上)
}

ContextServer::ContextServer(const ContextMain &main) {
    client_max_body_size = main.client_max_body_size;
    autoindex            = main.autoindex;
    allow                = main.allow;
    deny                 = main.deny;
    root                 = main.root;
    indexes              = main.indexes;
    error_pages          = main.error_pages;
    redirect             = std::make_pair(-1, "");
}

ContextLocation::ContextLocation(const ContextServer &server) {
    client_max_body_size = server.client_max_body_size;
    autoindex            = server.autoindex;
    allow                = server.allow;
    deny                 = server.deny;
    root                 = server.root;
    indexes              = server.indexes;
    error_pages          = server.error_pages;
    redirect             = server.redirect;
    limit_except         = NULL;
}

ContextLimitExcept::ContextLimitExcept(void) {
    allow = "";
    deny  = "";
}

} // namespace config
