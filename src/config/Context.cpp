#include "Context.hpp"

namespace config {

ContextMain::ContextMain(void) {
    client_max_body_size = 1024;
    autoindex            = false;
}
ContextMain::~ContextMain(void) {}

ContextServer::ContextServer(const ContextMain &main) {
    client_max_body_size = main.client_max_body_size;
    autoindex            = main.autoindex;
    root                 = main.root;
    indexes              = main.indexes;
    error_pages          = main.error_pages;
    redirect             = std::make_pair(-1, "");
}
ContextServer::~ContextServer(void) {}

ContextLocation::ContextLocation(const ContextServer &server) {
    client_max_body_size = server.client_max_body_size;
    autoindex            = server.autoindex;
    root                 = server.root;
    indexes              = server.indexes;
    error_pages          = server.error_pages;
    redirect             = server.redirect;
}
ContextLocation::~ContextLocation(void) {}

ContextLimitExcept::ContextLimitExcept(void) {}
ContextLimitExcept::~ContextLimitExcept(void) {}

} // namespace config
