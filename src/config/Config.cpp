#include "Config.hpp"

namespace config {
ContextMain::ContextMain(void) {
    client_max_body_size = 1024;
    autoindex            = false;
}

ContextServer::ContextServer(const ContextMain &main) {
    client_max_body_size = main.client_max_body_size;
    autoindex            = main.autoindex;
    allow                = main.allow;
    deny                 = main.deny;
    root                 = main.root;
    indexes              = main.indexes;
    error_pages          = main.error_pages;

    port = 80;
    host = "0.0.0.0";
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
}

ContextLimitExpect::ContextLimitExpect(void) {
    allow = "";
    deny  = "";
}
} // namespace config
