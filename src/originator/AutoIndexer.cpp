#include "AutoIndexer.hpp"
#include "../utils/CSS.hpp"
#include "../utils/File.hpp"
#include "../utils/HTML.hpp"
#include <algorithm>
#include <cerrno>
#include <ctime>
#include <sys/stat.h>
#include <unistd.h>
#define READ_SIZE 1024

typedef struct dirent t_dirent;

// [ソート用比較関数]

// 名前 昇順
bool compare_entry_by_name(const AutoIndexer::Entry &s1, const AutoIndexer::Entry &s2) {
    return s1.name < s2.name;
}

// 最終変更時刻 降順
bool compare_entry_by_mod_time(const AutoIndexer::Entry &s1, const AutoIndexer::Entry &s2) {
    return s1.time > s2.time;
}

// ディレクトリが先, それ以外が後
bool compare_entry_by_type(const AutoIndexer::Entry &s1, const AutoIndexer::Entry &s2) {
    return s1.is_dir > s2.is_dir;
}

HTTP::byte_string AutoIndexer::Entry::serialize(const HTTP::char_string &requested_path) const {
    HTTP::byte_string str;
    HTTP::char_string s;
    HTTP::byte_string basename = HTML::escape_html(name);
    if (is_dir) {
        basename += HTTP::strfy("/");
    }
    std::stringstream ss;
    str += HTTP::strfy("  <tr class=\"file-entry\">\n"
                       "    <td class=\"name\">\n");
    // 名前とアンカー
    {
        // URL
        HTTP::byte_string relative_url;
        relative_url += HTTP::strfy("      <a href=\"");
        relative_url += HTTP::strfy(HTML::escape_html(requested_path));
        if (requested_path.size() > 1 && requested_path[requested_path.size() - 1] != '/') {
            relative_url += HTTP::strfy("/");
        }
        relative_url += basename;
        relative_url += HTTP::strfy("\">");
        str += relative_url;
        str += basename;
        str += HTTP::strfy("</a>\n");
        str += HTTP::strfy("    </td>\n");
    }
    // 最終更新時刻
    {
        str += HTTP::strfy("    <td class=\"last-modified\">\n");
        ss << time;
        ss >> s;
        str += HTTP::strfy("      ");

        size_t mlen = 52;
        tm *tms     = localtime(&time);
        char *tstr  = (char *)malloc(sizeof(char) * (mlen + 1));
        if (tstr != NULL) {
            std::strftime(tstr, mlen, "%d-%m-%Y %H:%M:%S", tms);
            str += HTTP::strfy(tstr);
            free(tstr);
        }
        str += HTTP::strfy("\n");
        ss.clear();
        str += HTTP::strfy("    </td>\n");
    }
    // ファイルサイズ
    {
        if (is_dir) {
            str += HTTP::strfy("    <td class=\"size dir\">\n");
        } else {
            str += HTTP::strfy("    <td class=\"size file\">\n");
        }
        str += HTTP::strfy("      ");
        if (is_dir) {
            str += HTTP::strfy("-");
        } else {
            ss << size;
            ss >> s;
            ss.clear();
            str += HTTP::strfy(s);
        }
        str += HTTP::strfy("\n");
        str += HTTP::strfy("    </td>\n");
    }
    str += HTTP::strfy("  </tr>\n");
    return str;
}

AutoIndexer::Attribute::Attribute(char_string directory_path,
                                  char_string requested_path,
                                  char_string effective_requested_path)
    : directory_path(directory_path)
    , requested_path(requested_path)
    , effective_requested_path(effective_requested_path) {}

AutoIndexer::Status::Status() : dir(NULL), originated(false) {}

AutoIndexer::AutoIndexer(const RequestMatchingResult &match_result)
    : attr(HTTP::restrfy(match_result.path_local),
           HTTP::restrfy(match_result.target->dpath()),
           HTTP::restrfy(match_result.target->dpath_slash_reduced())) {}

AutoIndexer::~AutoIndexer() {
    close_if_needed();
}

void AutoIndexer::notify(IObserver &observer, IObserver::observation_category cat, t_time_epoch_ms epoch) {
    (void)observer;
    (void)cat;
    (void)epoch;
    assert(false);
}

void AutoIndexer::scan_from_directory() {
    // TODO: C++ way に書き直す
    if (status.originated) {
        return;
    }
    errno     = 0;
    DIR *dird = opendir(attr.directory_path.c_str());
    if (dird == NULL) {
        switch (errno) {
            case ENOENT:
                throw http_error("file not found", HTTP::STATUS_NOT_FOUND);
            case EACCES:
                throw http_error("permission denied", HTTP::STATUS_FORBIDDEN);
            case EMFILE:
            case ENFILE:
                throw http_error("exceeding fd limits", HTTP::STATUS_SERVICE_UNAVAILABLE);
            default:
                VOUT(errno);
                throw http_error("can't open", HTTP::STATUS_FORBIDDEN);
        }
    }
    status.dir = dird;
    for (;;) {
        t_dirent *ent = readdir(status.dir);
        if (ent == NULL) {
            break;
        }
        t_stat st;
        HTTP::char_string fname(ent->d_name);
        if (fname == "." || fname == "..") {
            continue;
        }
        HTTP::char_string fpath
            = HTTP::restrfy(HTTP::Utils::join_path(HTTP::strfy(attr.directory_path), HTTP::strfy(fname)));
        const int result = stat(fpath.c_str(), &st);
        if (result < 0) {
            continue;
        }
        entries.resize(entries.size() + 1);
        entries.back().name   = HTTP::strfy(ent->d_name);
        entries.back().size   = st.st_size;
        entries.back().time   = st.st_mtime;
        entries.back().is_dir = S_ISDIR(st.st_mode);
    }
    render_html();
    status.originated = true;
    close_if_needed();
}

void AutoIndexer::render_html() {
    // ソート
    std::stable_sort(entries.begin(), entries.end(), compare_entry_by_name);
    std::stable_sort(entries.begin(), entries.end(), compare_entry_by_mod_time);
    std::stable_sort(entries.begin(), entries.end(), compare_entry_by_type);

    // 出力データ生成
    response_data.inject(HTTP::strfy("<html>\n"
                                     "<head>\n"
                                     "  <title>\n"),
                         false);
    response_data.inject(HTTP::strfy(HTML::escape_html(attr.effective_requested_path)), false);
    response_data.inject(HTTP::strfy("  </title>\n"
                                     "  <style>\n" CSS_AUTOINDEXER "  </style>\n"
                                     "</head>\n"
                                     "<body>\n"),
                         false);

    // 見出し部
    response_data.inject(HTTP::strfy("<h2>Index of " + HTML::escape_html(attr.requested_path) + "</h2>\n"), false);
    response_data.inject(HTTP::strfy("<hr>"), false);
    // テーブル部
    response_data.inject(HTTP::strfy("<table class=\"autoindex-table\">\n"
                                     "  <thead>\n"
                                     "    <tr>\n"
                                     "      <th class=\"name\">Name</th>\n"
                                     "      <th class=\"last-modified\">Last Modified</th>\n"
                                     "      <th class=\"size\">Size</th>\n"
                                     "    </tr>\n"
                                     "  </thead>\n"
                                     "  <tbody>\n"),
                         false);
    for (entry_list::size_type i = 0; i < entries.size(); ++i) {
        const Entry &ent             = entries[i];
        HTTP::byte_string serialized = ent.serialize(attr.effective_requested_path);
        response_data.inject(serialized, false);
    }
    response_data.inject(HTTP::strfy("  </tbody>\n"
                                     "</table>\n"),
                         false);
    response_data.inject(HTTP::strfy("</body>\n"
                                     "</html>\n"),
                         false);
    response_data.inject("", 0, true);
}

void AutoIndexer::close_if_needed() {
    if (status.dir == NULL) {
        return;
    }
    closedir(status.dir);
    status.dir = NULL;
}

void AutoIndexer::inject_socketlike(ISocketLike *socket_like) {
    (void)socket_like;
}

bool AutoIndexer::is_originatable() const {
    return !status.originated;
}

bool AutoIndexer::is_origination_started() const {
    return status.originated;
}

bool AutoIndexer::is_reroutable() const {
    return false;
}

HTTP::byte_string AutoIndexer::reroute_path() const {
    assert(false);
    return HTTP::byte_string();
}

bool AutoIndexer::is_responsive() const {
    return status.originated;
}

void AutoIndexer::start_origination(IObserver &observer) {
    (void)observer;
    scan_from_directory();
}

void AutoIndexer::leave() {
    delete this;
}

ResponseHTTP::header_list_type
AutoIndexer::determine_response_headers(const IResponseDataConsumer::t_sending_mode sm) const {
    ResponseHTTP::header_list_type headers;
    switch (sm) {
        case ResponseDataList::SM_CHUNKED:
            headers.push_back(std::make_pair(HeaderHTTP::transfer_encoding, HTTP::strfy("chunked")));
            break;
        case ResponseDataList::SM_NOT_CHUNKED:
            headers.push_back(
                std::make_pair(HeaderHTTP::content_length, ParserHelper::utos(response_data.current_total_size(), 10)));
            break;
        default:
            break;
    }
    // MIMEタイプ設定
    {
        HTTP::CH::ContentType ct;
        ct.value   = HTTP::strfy("text/html");
        ct.charset = HTTP::strfy("UTF-8");
        headers.push_back(std::make_pair(HeaderHTTP::content_type, ct.serialize()));
    }
    return headers;
}

ResponseHTTP *AutoIndexer::respond(const RequestHTTP *request, bool should_close) {
    const IResponseDataConsumer::t_sending_mode sm = response_data.determine_sending_mode();
    ResponseHTTP::header_list_type headers         = determine_response_headers(sm);
    ResponseHTTP *res
        = new ResponseHTTP(request->get_http_version(), HTTP::STATUS_OK, &headers, &response_data, should_close);
    return res;
}
