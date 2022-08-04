#ifndef REQUESTMATCHER_HPP
#define REQUESTMATCHER_HPP
#include "../config/Config.hpp"
#include "../interface/IRequestMatcher.hpp"
#include "../utils/LightString.hpp"

class RequestMatcher : public IRequestMatcher {
public:
    typedef HTTP::light_string light_string;
    typedef std::pair<HTTP::byte_string, HTTP::byte_string> cgi_resource_pair;
    typedef std::pair<int, HTTP::byte_string> redirect_pair;

public:
    RequestMatcher();
    ~RequestMatcher();

    virtual RequestMatchingResult request_match(const std::vector<config::Config> &configs,
                                                const IRequestMatchingParam &rp);

private:
    RequestMatchingResult
    routing_cgi(RequestMatchingResult res, const RequestTarget &target, const config::Config &conf);
    RequestMatchingResult routing_default(RequestMatchingResult res,
                                          const RequestTarget &target,
                                          const HTTP::t_method &method,
                                          const config::Config &conf);

    config::Config get_config(const std::vector<config::Config> &configs, const IRequestMatchingParam &rp);

    void check_routable(const IRequestMatchingParam &rp, const config::Config &conf);
    bool is_valid_scheme(const RequestTarget &target);
    bool is_valid_path(const RequestTarget &target);
    bool is_valid_request_method(const RequestTarget &target, const HTTP::t_method &method, const config::Config &conf);

    bool is_redirect(const RequestTarget &target, const config::Config &conf);
    bool is_cgi(const RequestTarget &target, const config::Config &conf);
    bool get_is_executable(const RequestTarget &target, const HTTP::t_method &method, const config::Config &conf);
    bool get_is_autoindex(const RequestTarget &target, const config::Config &conf);

    bool is_regular_file(const std::string &path) const;

    long get_client_max_body_size(const RequestTarget &target, const config::Config &conf) const;
    redirect_pair get_redirect(const RequestTarget &target, const config::Config &conf) const;
    cgi_resource_pair get_cgi_resource(const RequestTarget &target) const;
    RequestMatchingResult::status_dict_type get_status_page_dict(const RequestTarget &target,
                                                                 const config::Config &conf) const;
    HTTP::byte_string get_path_cgi_executor(const RequestTarget &target,
                                            const config::Config &conf,
                                            const HTTP::byte_string &cgi_path) const;

    HTTP::byte_string make_resource_path(const RequestTarget &target, const config::Config &conf) const;
};

#endif
