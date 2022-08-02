#include "RequestMatcher.hpp"
#include "../communication/RequestHTTP.hpp"
#include "../config/Config.hpp"
#include "../utils/HTTPError.hpp"
#include <sys/stat.h>
#define debug(e)                                                                                                       \
    { std::cout << #e << " : " << e << std::endl; }

RequestMatcher::RequestMatcher() {}
RequestMatcher::~RequestMatcher() {}

/**
 * - [x] schemaのバリデーション
 * - [x] 不正なリクエストターゲットのバリデーション
 * - [x] リクエストメソッドのバリデーション
 * - [x] リダイレクトされるかどうかの判定
 * - [x] client_max_body_sizeを変数に入れる
 * - [x] rootを連結して適切なリソースへのパスを生成する
 */
RequestMatchingResult RequestMatcher::request_match(const std::vector<config::Config> &configs,
                                                    const RequestHTTP &request) {
    RequestMatchingResult res;
    const config::Config &config = get_config(configs, request.get_request_matching_param());

    // 対応しているschemeか(この課題では `http` のみ)
    if (!is_valid_scheme(request.get_request_matching_param())) {
        throw http_error("not valid scheme", HTTP::STATUS_NOT_FOUND);
    }
    // 対応しているリクエストメソッドか(locationによっても異なる)
    if (is_valid_request_method(request.get_request_matching_param(), config)) {
        throw http_error("not valid scheme", HTTP::STATUS_NOT_FOUND);
    }
    // URIターゲットが適切か
    if (is_valid_path(request.get_request_matching_param())) {
        throw http_error("not valid scheme", HTTP::STATUS_NOT_FOUND);
    }
    res.client_max_body_size = get_client_max_body_size(request.get_request_matching_param(), config);
    res.status_page_dict     = get_status_page_dict(request.get_request_matching_param(), config);

    // リダイレクトが行われるか
    if (is_redirect(request.get_request_matching_param(), config)) {
        res.redirect = get_redirect(request.get_request_matching_param(), config);
        return res;
    }

    // CGIか
    if (is_cgi(request.get_request_matching_param(), config)) {
        // TODO: 関数にする
        cgi_resource_pair cgi_resource;
        cgi_resource          = get_cgi_resource(request.get_request_matching_param());
        res.path_local        = cgi_resource.first;
        res.path_after        = cgi_resource.second;
        res.path_cgi_executor = get_path_cgi_executor(request.get_request_matching_param(), config, res.path_local);
        res.is_cgi            = true;
    } else {
        // 通常の処理
        // rootをつなげる
        HTTP::byte_string path = make_resource_path(request.get_request_matching_param(), config);
        if (path.empty()) {
            throw http_error("file not found", HTTP::STATUS_NOT_FOUND);
        }
    }
    return res;
}

// hostから利用するconfigを探す
// hostがマッチしたらそれを利用する
// default_serverがあったらそれを利用する
config::Config RequestMatcher::get_config(const std::vector<config::Config> &configs, const IRequestMatchingParam &rp) {
    const HTTP::CH::Host host_data = rp.get_host();
    const std::string host         = HTTP::restrfy(host_data.host);

    // host_nameが一致した場合
    for (std::vector<config::Config>::const_iterator it = configs.begin(); it != configs.end(); ++it) {
        if (it->get_host() == host) {
            return *it;
        }
    }
    // default_serverの指定があった場合
    for (std::vector<config::Config>::const_iterator it = configs.begin(); it != configs.end(); ++it) {
        if (it->get_default_server()) {
            return *it;
        }
    }
    // どちらも該当しない場合は先頭のconfigを使う
    return configs.front();
}

bool RequestMatcher::is_valid_scheme(const IRequestMatchingParam &rp) {
    const RequestTarget &target = rp.get_request_target();
    const light_string &scheme  = target.scheme;

    return scheme == "http" || scheme == "https";
}

bool RequestMatcher::is_redirect(const IRequestMatchingParam &rp, const config::Config &config) {
    const RequestTarget &target = rp.get_request_target();
    const std::string &path     = HTTP::restrfy(target.path.str());

    std::pair<int, std::string> p = config.get_redirect(path);
    return p.first != -1;
}

bool RequestMatcher::is_valid_path(const IRequestMatchingParam &rp) {
    const RequestTarget &target               = rp.get_request_target();
    const std::vector<light_string> &splitted = target.path.split("/");

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

bool RequestMatcher::is_valid_request_method(const IRequestMatchingParam &rp, const config::Config &config) {
    const HTTP::t_method &method              = rp.get_http_method();
    const RequestTarget &target               = rp.get_request_target();
    const std::string &path                   = HTTP::restrfy(target.path.str());
    const std::set<enum config::Methods> &lmt = config.get_limit_except(path);

    // TODO: parser側でメソッドの定義を `HTTP::` に変更する
    switch (method) {
        case HTTP::METHOD_GET:
            if (lmt.find(config::GET) != lmt.end()) {
                return true;
            }
            break;
        case HTTP::METHOD_DELETE:
            if (lmt.find(config::DELETE) != lmt.end()) {
                if (config.get_exec_delete(path)) {
                    return true;
                }
            }
            break;
        case HTTP::METHOD_POST:
            if (lmt.find(config::POST) != lmt.end()) {
                return true;
            }
            break;
        default:;
            return false;
    }
    // メソッドが正しい かつ 制限がかかっていない場合は true を返す
    return lmt.empty();
}

bool RequestMatcher::is_cgi(const IRequestMatchingParam &rp, const config::Config &config) {
    const RequestTarget &target = rp.get_request_target();
    const std::string &path     = HTTP::restrfy(target.path.str());

    return config.get_exec_cgi(path);
}

bool RequestMatcher::is_regular_file(const std::string &path) const {
    struct stat st;

    if (stat(path.c_str(), &st) < 0) {
        return false;
    }
    return S_ISREG(st.st_mode);
}

RequestMatcher::redirect_pair RequestMatcher::get_redirect(const IRequestMatchingParam &rp,
                                                           const config::Config &config) const {
    const RequestTarget &target = rp.get_request_target();
    const std::string &path     = HTTP::restrfy(target.path.str());

    std::pair<int, std::string> p = config.get_redirect(path);
    return std::make_pair(p.first, HTTP::strfy(p.second));
}

// ファイルの権限を順番に見ていき、ファイルが存在した時点で、分割する
RequestMatcher::cgi_resource_pair RequestMatcher::get_cgi_resource(const IRequestMatchingParam &rp) const {
    const RequestTarget &target = rp.get_request_target();
    HTTP::byte_string resource_path;
    HTTP::byte_string path_info;

    light_string path = target.path;
    for (size_t i = 0;; i = path.find("/", i)) {
        light_string cur = path.substr(0, i);
        if (is_regular_file(HTTP::restrfy(cur.str()))) {
            resource_path = cur.str();
            if (i != HTTP::npos) {
                path_info = path.substr(i, path.size()).str();
            }
            break;
        }
        if (i == HTTP::npos) {
            break;
        }
        i += 1;
    }
    if (resource_path.empty()) {
        throw http_error("file not found", HTTP::STATUS_NOT_FOUND);
    }
    return std::make_pair(resource_path, path_info);
}

RequestMatchingResult::status_dict_type get_error_page(const IRequestMatchingParam &rp,
                                                       const config::Config &config) const {
    const RequestTarget &target = rp.get_request_target();
    const std::string &path     = HTTP::restrfy(target.path.str());

    const std::map<int, std::string> error_pages = config.get_error_page(path);
    RequestMatchingResult::status_dict_type res;
    for (const std::map<int, std::string>::const_iterator it = error_pages.begin(); it != error_pages.end(); ++it) {
        res[static_cast<HTTP::t_status>(it->first)] = HTTP::strfy(it->second);
    }
    return res;
}

// 対応するrootを後ろにくっつける + indexに対応するファイルを探す
// exec_deleteもチェック
HTTP::byte_string RequestMatcher::make_resource_path(const IRequestMatchingParam &rp,
                                                     const config::Config &config) const {
    const RequestTarget &target = rp.get_request_target();
    const std::string &path     = HTTP::restrfy(target.path.str());

    std::string root                 = config.get_root(path);
    std::string resource_path        = root + path;
    std::vector<std::string> indexes = config.get_index(path);

    for (std::vector<std::string>::iterator it = indexes.begin(); it != indexes.end(); ++it) {
        const std::string &cur = resource_path + *it;
        if (is_regular_file(cur)) {
            return HTTP::strfy(cur);
        }
    }
    return HTTP::strfy("");
}

HTTP::byte_string RequestMatcher::get_path_cgi_executor(const IRequestMatchingParam &rp,
                                                        const config::Config &config,
                                                        const HTTP::byte_string &cgi_path) const {
    std::string s = HTTP::restrfy(cgi_path);
    size_t pos    = s.rfind(".");
    if (pos == std::string::npos) {
        return HTTP::strfy("");
    }

    const std::string &extension   = s.substr(pos + 1);
    const RequestTarget &target    = rp.get_request_target();
    const std::string &path        = HTTP::restrfy(target.path.str());
    config::cgi_path_map cgi_paths = config.get_cgi_path(path);
    return HTTP::strfy(cgi_paths[extension]);
}
