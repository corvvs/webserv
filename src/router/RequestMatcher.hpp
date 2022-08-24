#ifndef REQUESTMATCHER_HPP
#define REQUESTMATCHER_HPP
#include "../config/Config.hpp"
#include "../interface/IRequestMatcher.hpp"
#include "../utils/HTTPError.hpp"
#include "../utils/LightString.hpp"

class RequestMatcher : public IRequestMatcher {
public:
    typedef HTTP::light_string light_string;
    typedef std::pair<HTTP::t_status, HTTP::byte_string> redirect_pair;

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

    minor_error check_routable(const IRequestMatchingParam &rp, const config::Config &conf);
    bool is_valid_scheme(const RequestTarget &target);
    bool is_valid_path(const RequestTarget &target);
    bool is_valid_request_method(const RequestTarget &target, const HTTP::t_method &method, const config::Config &conf);
    bool is_redirect(const RequestTarget &target, const config::Config &conf);
    bool is_cgi(const RequestTarget &target, const config::Config &conf);
    bool is_post(const RequestTarget &target, const HTTP::t_method &method, const config::Config &conf);

    bool get_is_executable(const RequestTarget &target, const HTTP::t_method &method, const config::Config &conf);
    bool get_is_autoindex(const RequestTarget &target, const config::Config &conf);
    long get_client_max_body_size(const RequestTarget &target, const config::Config &conf) const;
    redirect_pair get_redirect(const RequestTarget &target, const config::Config &conf) const;
    RequestMatchingResult::status_dict_type get_status_page_dict(const RequestTarget &target,
                                                                 const config::Config &conf) const;
    HTTP::byte_string get_path_cgi_executor(const RequestTarget &target,
                                            const config::Config &conf,
                                            const HTTP::byte_string &cgi_path) const;

    std::pair<HTTP::byte_string, bool> make_resource_path(const RequestTarget &target,
                                                          const config::Config &conf) const;
    std::pair<HTTP::byte_string, bool> make_post_path(const RequestTarget &target, const config::Config &conf) const;
    RequestMatchingResult::CGIResource make_cgi_resource(const RequestTarget &target, const config::Config &conf) const;
};

#endif
