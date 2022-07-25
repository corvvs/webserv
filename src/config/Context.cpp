#include "Context.hpp"

namespace config {

ContextMain::ContextMain(void) {
    client_max_body_size = 1024;
    autoindex            = false;
}
ContextMain::~ContextMain(void) {}

ContextServer::ContextServer(void) : redirect(std::make_pair(REDIRECT_INITIAL_VALUE, "")) {}
ContextServer::~ContextServer(void) {}

ContextLocation::ContextLocation(void) : redirect(std::make_pair(REDIRECT_INITIAL_VALUE, "")) {}
ContextLocation::ContextLocation(const ContextServer &server) {
    client_max_body_size = server.client_max_body_size;
    autoindex            = server.autoindex;
    root                 = server.root;
    indexes              = server.indexes;
    error_pages          = server.error_pages;
    redirect             = server.redirect;
    upload_store         = server.upload_store;
}
ContextLocation::~ContextLocation(void) {}

ContextLimitExcept::ContextLimitExcept(void) {}
ContextLimitExcept::~ContextLimitExcept(void) {}

} // namespace config
