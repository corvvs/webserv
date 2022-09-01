#include "RequestMatcher.hpp"
#include "../communication/RequestHTTP.hpp"
#include "../config/Config.hpp"
#include "../utils/File.hpp"
#include "../utils/HTTPError.hpp"
#include "../utils/LightString.hpp"
#include "../utils/UtilsString.hpp"
#include <sys/stat.h>

bool is_acceptable_form(const RequestTarget &target) {
    switch (target.form) {
        case RequestTarget::FORM_ORIGIN:
        case RequestTarget::FORM_ABSOLUTE:
            return true;
        default:
            break;
    }
    return false;
}

RequestMatchingResult::RequestMatchingResult(const RequestTarget *target_) : target(target_), client_max_body_size(0) {}

RequestMatcher::RequestMatcher() {}
RequestMatcher::~RequestMatcher() {}

/**
 * - schemaのバリデーション
 * - 不正なリクエストターゲットのバリデーション
 * - リクエストメソッドのバリデーション
 * - リダイレクトされるかどうかの判定
 * - CGIか通常のファイルでそれぞれのルーティング処理
 */
RequestMatchingResult RequestMatcher::request_match(const std::vector<config::Config> &configs,
                                                    const IRequestMatchingParam &rp) {
    const config::Config conf = get_config(configs, rp);

    const RequestTarget &target = rp.get_request_target();
    RequestMatchingResult res(&target);

    res.server_name          = get_server_name(conf, rp);
    res.client_max_body_size = get_client_max_body_size(target, conf);
    res.status_page_dict     = get_status_page_dict(target, conf);

    res.error = check_routable(rp, conf);
    if (res.error.is_error()) {
        return res;
    }

    if (is_redirect(target, conf)) {
        RequestMatcher::redirect_pair pair = get_redirect(target, conf);
        res.status_code                    = pair.first;
        res.redirect_location              = pair.second;
        res.result_type                    = RequestMatchingResult::RT_EXTERNAL_REDIRECTION;
        return res;
    }
    res.client_max_body_size = get_client_max_body_size(target, conf);
    res.status_page_dict     = get_status_page_dict(target, conf);
    if (is_cgi(target, conf)) {
        return routing_cgi(res, target, conf);
    }

    return routing_default(res, target, rp.get_http_method(), conf);
}

HTTP::byte_string RequestMatcher::get_server_name(const config::Config &conf, const IRequestMatchingParam &rp) const {
    const std::vector<std::string> &server_names = conf.get_server_name();
    const HTTP::CH::Host host_data               = rp.get_host();
    const std::string host                       = HTTP::restrfy(host_data.host);

    std::vector<std::string>::const_iterator srv_name_it = std::find(server_names.begin(), server_names.end(), host);
    if (srv_name_it == server_names.end()) {
        std::stringstream ss;
        ss << conf.get_port();
        // hostが指定されていない場合は, hostnameは `host:port` の形式となる
        const std::string res = conf.get_host() + ":" + ss.str();
        return HTTP::strfy(res);
    }
    return HTTP::strfy(*srv_name_it);
}

RequestMatchingResult
RequestMatcher::routing_cgi(RequestMatchingResult res, const RequestTarget &target, const config::Config &conf) {
    res.cgi_resource = make_cgi_resource(target, conf);
    if (res.cgi_resource.script_name.empty()) {
        res.error = minor_error::make("file not found", HTTP::STATUS_NOT_FOUND);
        return res;
    }
    res.path_cgi_executor = get_path_cgi_executor(target, conf, res.cgi_resource.script_name);
    res.result_type       = RequestMatchingResult::RT_CGI;
    return res;
}

/**
 * 以下の形式でパスを分割する
 * リクエスト: `/cgi-bin/cgi.rb/pathinfo`
 * fullpath   : /cgi-bin/cgi.rb (local path)
 * root       : /cgi-bin
 * script_name: /cgi-bin/cgi.rb (request path)
 * path_info  : /pathinfo
 */
RequestMatchingResult::CGIResource RequestMatcher::make_cgi_resource(const RequestTarget &target,
                                                                     const config::Config &conf) const {
    const HTTP::byte_string root = HTTP::strfy(conf.get_root(HTTP::restrfy(target.dpath_slash_reduced())));
    // ルートとパスをくっつける
    const HTTP::light_string root_stripped    = HTTP::Utils::rstrip_path(root);
    const HTTP::light_string path_stripped    = HTTP::Utils::lstrip_path(target.dpath_slash_reduced());
    const HTTP::byte_string full_request_path = HTTP::Utils::join_path(root_stripped, path_stripped);
    const light_string path                   = full_request_path;

    RequestMatchingResult::CGIResource resource;
    resource.root = root_stripped.str();
    for (size_t i = root_stripped.size();; i = path.find("/", i)) {
        HTTP::light_string cur = path.substr(0, i);
        HTTP::byte_string cb   = cur.str();
        if (file::is_file(HTTP::restrfy(cb))) {
            resource.fullpath    = cb;
            resource.script_name = cur.substr(root_stripped.size()).str();
            if (i != HTTP::npos) {
                resource.path_info = path.substr(i).str();
            }
            break;
        }
        if (i == HTTP::npos) {
            break;
        }
        i += 1;
    }
    return resource;
}

bool RequestMatcher::is_post(const RequestTarget &target, const HTTP::t_method &method, const config::Config &conf) {
    if (method != HTTP::METHOD_POST) {
        return false;
    }
    const std::string &upload_store = conf.get_upload_store(HTTP::restrfy(target.dpath_slash_reduced()));
    return !upload_store.empty();
}

std::pair<HTTP::byte_string, bool> RequestMatcher::make_post_path(const RequestTarget &target,
                                                                  const config::Config &conf) const {
    const std::string upload_store = conf.get_upload_store(HTTP::restrfy(target.dpath_slash_reduced()));
    if (file::is_dir(upload_store)) {
        return std::make_pair(HTTP::strfy(upload_store), true);
    }
    return std::make_pair(HTTP::strfy(""), false);
}

RequestMatchingResult::ResultType RequestMatcher::get_result_type_from_dir(const RequestTarget &target,
                                                                           const HTTP::t_method &method,
                                                                           const config::Config &conf) const {
    if (is_executable(target, method, conf) && method == HTTP::METHOD_POST) {
        return RequestMatchingResult::RT_FILE_POST;
    }
    if (is_autoindex(target, conf)) {
        return RequestMatchingResult::RT_AUTO_INDEX;
    }
    return RequestMatchingResult::RT_ERROR;
}

RequestMatchingResult::ResultType RequestMatcher::get_result_type_from_file(const RequestTarget &target,
                                                                            const HTTP::t_method &method,
                                                                            const config::Config &conf) const {
    if (is_executable(target, method, conf)) {
        switch (method) {
            case HTTP::METHOD_DELETE:
                return RequestMatchingResult::RT_FILE_DELETE;
            case HTTP::METHOD_PUT:
                return RequestMatchingResult::RT_FILE_PUT;
            default:;
        }
    }
    return RequestMatchingResult::RT_FILE;
}

RequestMatchingResult::ResultType RequestMatcher::get_result_type(const RequestTarget &target,
                                                                  const HTTP::t_method &method,
                                                                  const config::Config &conf,
                                                                  const bool &isdir) const {
    if (isdir) {
        return get_result_type_from_dir(target, method, conf);
    }
    return get_result_type_from_file(target, method, conf);
}

RequestMatchingResult RequestMatcher::routing_default(RequestMatchingResult res,
                                                      const RequestTarget &target,
                                                      const HTTP::t_method &method,
                                                      const config::Config &conf) {
    std::pair<HTTP::byte_string, bool> path_isdir;
    if (is_post(target, method, conf)) {
        path_isdir = make_post_path(target, conf);
    } else {
        path_isdir = make_resource_path(target, conf);
    }

    if (!is_acceptable_form(target)) {
        res.error = minor_error::make("form of a target is not acceptable", HTTP::STATUS_BAD_REQUEST);
        return res;
    }
    if (path_isdir.first.empty()) {
        res.error = minor_error::make("file not found", HTTP::STATUS_NOT_FOUND);
        return res;
    }
    res.path_local   = path_isdir.first;
    const bool isdir = path_isdir.second;
    res.result_type  = get_result_type(target, method, conf, isdir);
    if (res.result_type == RequestMatchingResult::RT_ERROR) {
        res.error = minor_error::make("permission denied", HTTP::STATUS_FORBIDDEN);
    }
    return res;
}

bool RequestMatcher::is_executable(const RequestTarget &target,
                                   const HTTP::t_method &method,
                                   const config::Config &conf) const {
    const std::string &path = HTTP::restrfy(target.dpath_slash_reduced());

    switch (method) {
        case HTTP::METHOD_GET:
            return true;
        case HTTP::METHOD_POST:
            return conf.get_upload_store(path) != "";
        case HTTP::METHOD_DELETE:
            return conf.get_exec_delete(path);
        default:;
    }
    return false;
}

bool RequestMatcher::is_autoindex(const RequestTarget &target, const config::Config &conf) const {
    const std::string &path = HTTP::restrfy(target.dpath_slash_reduced());
    return conf.get_autoindex(path);
}

config::Config RequestMatcher::get_config(const std::vector<config::Config> &configs, const IRequestMatchingParam &rp) {
    const HTTP::CH::Host host_data = rp.get_host();
    const std::string host         = HTTP::restrfy(host_data.host);

    // サーバーコンテキストの中から、server_nameがマッチしているものを返す
    for (std::vector<config::Config>::const_iterator conf_it = configs.begin(); conf_it != configs.end(); ++conf_it) {
        const std::vector<std::string> &servers = conf_it->get_server_name();

        // server_nameが一致するか判定する
        std::vector<std::string>::const_iterator srv_it = std::find(servers.begin(), servers.end(), host);
        if (srv_it != servers.end()) {
            return *conf_it;
        }
    }
    // 1つも該当しない場合は先頭のconfを使う
    return configs.front();
}

minor_error RequestMatcher::check_routable(const IRequestMatchingParam &rp, const config::Config &conf) {
    const RequestTarget &target = rp.get_request_target();

    if (target.is_error) {
        return minor_error::make("target has an error", HTTP::STATUS_BAD_REQUEST);
    }
    if (!is_valid_scheme(target)) {
        return minor_error::make("invalid scheme", HTTP::STATUS_BAD_REQUEST);
    }
    if (!is_valid_path(target)) {
        return minor_error::make("invalid url target", HTTP::STATUS_BAD_REQUEST);
    }
    if (!is_valid_request_method(target, rp.get_http_method(), conf)) {
        return minor_error::make("method not allowed", HTTP::STATUS_METHOD_NOT_ALLOWED);
    }
    return minor_error::ok();
}

bool RequestMatcher::is_valid_scheme(const RequestTarget &target) {
    return target.scheme == "http" || target.scheme == "";
}

bool RequestMatcher::is_valid_path(const RequestTarget &target) {
    const std::vector<light_string> &splitted = light_string(target.dpath_slash_reduced()).split("/");

    int depth = 0;
    for (std::vector<light_string>::const_iterator it = splitted.begin(); it != splitted.end(); ++it) {
        if (*it == "..") {
            depth -= 1;
        } else if (*it != "" && *it != ".") {
            depth += 1;
        }
        if (depth < 0) {
            return false;
        }
    }
    return true;
}

bool RequestMatcher::is_valid_request_method(const RequestTarget &target,
                                             const HTTP::t_method &method,
                                             const config::Config &conf) {
    const std::string &path                   = HTTP::restrfy(target.dpath_slash_reduced());
    const std::set<enum config::Methods> &lmt = conf.get_limit_except(path);

    switch (method) {
        // limit_except自体が定義されていない or limit_exceptに対応するメソッドが定義されていたら対応可能
        case HTTP::METHOD_GET:
            return (lmt.empty() || lmt.find(config::GET) != lmt.end());
        case HTTP::METHOD_DELETE:
            return (lmt.empty() || lmt.find(config::DELETE) != lmt.end());
        case HTTP::METHOD_POST:
            return (lmt.empty() || lmt.find(config::POST) != lmt.end());
        default:;
    }
    return false;
}

bool RequestMatcher::is_redirect(const RequestTarget &target, const config::Config &conf) {
    if (!is_acceptable_form(target)) {
        return false;
    }
    const std::string &path                         = HTTP::restrfy(target.dpath_slash_reduced());
    std::pair<HTTP::t_status, std::string> redirect = conf.get_redirect(path);
    return redirect.first != HTTP::STATUS_REDIRECT_INIT;
}

bool RequestMatcher::is_cgi(const RequestTarget &target, const config::Config &conf) {
    if (!is_acceptable_form(target)) {
        return false;
    }
    const std::string &path = HTTP::restrfy(target.dpath_slash_reduced());
    return conf.get_exec_cgi(path);
}

RequestMatcher::redirect_pair RequestMatcher::get_redirect(const RequestTarget &target,
                                                           const config::Config &conf) const {
    const std::string &path                         = HTTP::restrfy(target.dpath_slash_reduced());
    std::pair<HTTP::t_status, std::string> redirect = conf.get_redirect(path);
    return std::make_pair(redirect.first, HTTP::strfy(redirect.second));
}

RequestMatchingResult::status_dict_type RequestMatcher::get_status_page_dict(const RequestTarget &target,
                                                                             const config::Config &conf) const {
    const std::string &path = HTTP::restrfy(target.dpath_slash_reduced());

    const std::map<HTTP::t_status, std::string> error_pages = conf.get_error_page(path);
    RequestMatchingResult::status_dict_type res;
    for (std::map<HTTP::t_status, std::string>::const_iterator it = error_pages.begin(); it != error_pages.end();
         ++it) {
        res[static_cast<HTTP::t_status>(it->first)] = HTTP::strfy(it->second);
    }
    return res;
}

long RequestMatcher::get_client_max_body_size(const RequestTarget &target, const config::Config &conf) const {
    const std::string &path = HTTP::restrfy(target.dpath_slash_reduced());
    return conf.get_client_max_body_size(path);
}

// 対応するrootを連結する + indexに対応するファイルを探す
std::pair<HTTP::byte_string, bool> RequestMatcher::make_resource_path(const RequestTarget &target,
                                                                      const config::Config &conf) const {
    const std::string &path = HTTP::restrfy(target.dpath_slash_reduced());

    const std::string root                   = conf.get_root(path);
    const HTTP::byte_string resource_path_bs = HTTP::Utils::join_path(HTTP::strfy(root), target.dpath_slash_reduced());
    const std::string resource_path          = HTTP::restrfy(resource_path_bs);
    if (!file::is_dir(resource_path)) {
        if (file::is_file(resource_path)) {
            return std::make_pair(resource_path_bs, false);
        }
        return std::make_pair(HTTP::strfy(""), false);
    }

    std::vector<std::string> indexes = conf.get_index(path);
    for (std::vector<std::string>::iterator it = indexes.begin(); it != indexes.end(); ++it) {
        const HTTP::byte_string cur_bs = HTTP::Utils::join_path(resource_path_bs, HTTP::strfy(*it));
        const std::string cur          = HTTP::restrfy(cur_bs);
        if (file::is_file(cur)) {
            return std::make_pair(cur_bs, false);
        }
    }
    return std::make_pair(resource_path_bs, true);
}

HTTP::byte_string RequestMatcher::get_path_cgi_executor(const RequestTarget &target,
                                                        const config::Config &conf,
                                                        const HTTP::byte_string &cgi_path) const {
    const std::string s = HTTP::restrfy(cgi_path);
    const size_t pos    = s.rfind(".");
    if (pos == std::string::npos) {
        return HTTP::strfy("");
    }
    // cgi_executers のキーは `.` を含む
    const std::string &extension                      = s.substr(pos);
    const std::string &path                           = HTTP::restrfy(target.dpath_slash_reduced());
    const config::cgi_executer_map cgi_executers      = conf.get_cgi_path(path);
    const config::cgi_executer_map::const_iterator it = cgi_executers.find(extension);
    if (it == cgi_executers.end()) {
        return HTTP::strfy("");
    }
    return HTTP::strfy(it->second);
}
