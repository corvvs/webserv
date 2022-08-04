#ifndef REQUESTMATCHER_HPP
#define REQUESTMATCHER_HPP
#include "../config/Config.hpp"
#include "../utils/LightString.hpp"
#include "IRequestMatcher.hpp"

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
    config::Config get_config(const std::vector<config::Config> &configs, const IRequestMatchingParam &rp);

    void check_routable(const IRequestMatchingParam &rp, const config::Config conf);
    bool is_valid_scheme(const RequestTarget &target);
    bool is_valid_path(const RequestTarget &target);
    bool
    is_valid_request_method(const RequestTarget &target, const HTTP::t_method &method, const config::Config &config);

    bool is_redirect(const IRequestMatchingParam &rp, const config::Config &config);
    bool is_cgi(const IRequestMatchingParam &rp, const config::Config &config);
    bool is_method_executable(const IRequestMatchingParam &rp, const config::Config &conf);
    bool is_regular_file(const std::string &path) const;

    RequestMatchingResult
    routing_cgi(RequestMatchingResult res, const IRequestMatchingParam &rp, const config::Config &conf);

    RequestMatchingResult
    routing_default(RequestMatchingResult res, const IRequestMatchingParam &rp, const config::Config &conf);

    long get_client_max_body_size(const IRequestMatchingParam &rp, const config::Config &config) const;
    RequestMatchingResult::status_dict_type get_status_page_dict(const IRequestMatchingParam &rp,
                                                                 const config::Config &config) const;
    redirect_pair get_redirect(const IRequestMatchingParam &rp, const config::Config &config) const;
    cgi_resource_pair get_cgi_resource(const IRequestMatchingParam &rp) const;
    HTTP::byte_string get_path_cgi_executor(const IRequestMatchingParam &rp,
                                            const config::Config &config,
                                            const HTTP::byte_string &cgi_path) const;

    HTTP::byte_string make_resource_path(const IRequestMatchingParam &rp, const config::Config &config) const;
};

#endif
