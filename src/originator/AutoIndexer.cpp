#include "AutoIndexer.hpp"
#include <algorithm>
#include <ctime>
#include <unistd.h>
#define READ_SIZE 1024

typedef struct dirent t_dirent;

bool compare_entry_by_name(const AutoIndexer::Entry &s1, const AutoIndexer::Entry &s2) {
    return s1.name < s2.name;
}

bool compare_entry_by_mod_time(const AutoIndexer::Entry &s1, const AutoIndexer::Entry &s2) {
    if (s1.st_mtim.tv_sec == s2.st_mtim.tv_sec) {
        return s1.st_mtim.tv_nsec > s2.st_mtim.tv_nsec;
    }
    return s1.st_mtim.tv_sec > s2.st_mtim.tv_sec;
}

bool compare_entry_by_type(const AutoIndexer::Entry &s1, const AutoIndexer::Entry &s2) {
    return s1.is_dir > s2.is_dir;
}

HTTP::byte_string AutoIndexer::Entry::serialize(const HTTP::char_string &requested_path) const {
    HTTP::byte_string str;
    HTTP::char_string s;
    HTTP::byte_string basename = name;
    if (is_dir) {
        basename += HTTP::strfy("/");
    }
    std::stringstream ss;
    str += HTTP::strfy("\t<tr class=\"file-entry\">\n");
    str += HTTP::strfy("\t\t<td class=\"name\">\n");
    str += HTTP::strfy("\t\t\t");
    // 名前とアンカー
    {
        // URL
        HTTP::byte_string relative_url;
        relative_url += HTTP::strfy("<a href=\"");
        relative_url += HTTP::strfy(requested_path);
        if (requested_path.size() > 1 && requested_path[requested_path.size() - 1] != '/') {
            relative_url += HTTP::strfy("/");
        }
        relative_url += basename;
        relative_url += HTTP::strfy("\">");
        str += relative_url;
        str += basename;
        str += HTTP::strfy("</a>");
        str += HTTP::strfy("\n");
        str += HTTP::strfy("\t\t</td>\n");
    }
    // 最終更新時刻
    {
        str += HTTP::strfy("\t\t<td class=\"last-modified\">\n");
        ss << st_mtim.tv_sec;
        ss >> s;
        str += HTTP::strfy("\t\t\t");

        size_t mlen = 52;
        tm *tms     = localtime(&st_mtim.tv_sec);
        char *tstr  = (char *)malloc(sizeof(char) * (mlen + 1));
        if (tstr != NULL) {
            std::strftime(tstr, mlen, "%d-%m-%Y %H:%M:%S", tms);
            str += HTTP::strfy(tstr);
            free(tstr);
        }
        str += HTTP::strfy("\n");
        ss.clear();
        str += HTTP::strfy("\t\t</td>\n");
    }
    // ファイルサイズ
    {
        str += HTTP::strfy("\t\t<td class=\"size\">\n");
        str += HTTP::strfy("\t\t\t");
        if (is_dir) {
            str += HTTP::strfy("-");
        } else {
            ss << size;
            ss >> s;
            ss.clear();
            str += HTTP::strfy(s);
        }
        str += HTTP::strfy("\n");
        str += HTTP::strfy("\t\t</td>\n");
    }
    str += HTTP::strfy("\t</tr>\n");
    return str;
}

AutoIndexer::AutoIndexer(const RequestMatchingResult &match_result)
    : directory_path_(HTTP::restrfy(match_result.path_local))
    , requested_path_(HTTP::restrfy(match_result.target->path.str()))
    , originated_(false)
    , dir(NULL) {}

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
    if (originated_) {
        return;
    }
    errno     = 0;
    DIR *dird = opendir(directory_path_.c_str());
    if (dird == NULL) {
        switch (errno) {
            case ENOENT:
                throw http_error("file not found", HTTP::STATUS_NOT_FOUND);
            case EACCES:
                throw http_error("permission denied", HTTP::STATUS_FORBIDDEN);
            default:
                QVOUT(strerror(errno));
                throw http_error("can't open", HTTP::STATUS_FORBIDDEN);
        }
    }
    dir = dird;
    for (;;) {
        t_dirent *ent = readdir(dir);
        if (ent == NULL) {
            break;
        }
        t_stat st;
        HTTP::char_string fname(ent->d_name);
        HTTP::char_string fpath = (directory_path_.size() > 1 ? directory_path_ + "/" : directory_path_) + fname;
        const int result        = stat(fpath.c_str(), &st);
        DXOUT(fpath << " " << result);
        if (result < 0) {
            QVOUT(strerror(errno));
            continue;
        }
        entries.resize(entries.size() + 1);
        entries.back().name    = HTTP::strfy(ent->d_name);
        entries.back().size    = st.st_size;
        entries.back().st_mtim = st.st_mtimespec;
        entries.back().is_dir  = S_ISDIR(st.st_mode);
    }
    render_html();
    originated_ = true;
    close_if_needed();
}

void AutoIndexer::render_html() {
    // ソート
    std::stable_sort(entries.begin(), entries.end(), compare_entry_by_name);
    std::stable_sort(entries.begin(), entries.end(), compare_entry_by_mod_time);
    std::stable_sort(entries.begin(), entries.end(), compare_entry_by_type);
    // 出力データ生成
    response_data.inject(HTTP::strfy("<html>\n<body>\n<table>\n"), false);

    // 見出し部
    response_data.inject(HTTP::strfy("<h2>\n"), false);
    response_data.inject(HTTP::strfy("Index of "), false);
    response_data.inject(HTTP::strfy(requested_path_), false);
    response_data.inject(HTTP::strfy("</h2>\n"), false);

    // テーブル部
    response_data.inject(HTTP::strfy("\t<thead>\n\t\t<tr>\n"), false);
    response_data.inject(HTTP::strfy("\t\t\t<th class=\"name\">Name</th>\n"), false);
    response_data.inject(HTTP::strfy("\t\t\t<th class=\"last-modified\">Last Modified</th>\n"), false);
    response_data.inject(HTTP::strfy("\t\t\t<th class=\"size\">Size</th>\n"), false);
    response_data.inject(HTTP::strfy("\t\t</tr>\n\t</thead>\n"), false);
    response_data.inject(HTTP::strfy("<tbody>\n"), false);
    for (entry_list::size_type i = 0; i < entries.size(); ++i) {
        const Entry &ent = entries[i];
        if (ent.name == "." || ent.name == "..") {
            continue;
        }
        HTTP::byte_string serialized = ent.serialize(requested_path_);
        response_data.inject(serialized, false);
    }
    response_data.inject(HTTP::strfy("</tbody>\n"), false);
    response_data.inject(HTTP::strfy("</table>\n</body>\n</html>\n"), false);
    response_data.inject("", 0, true);
}

void AutoIndexer::close_if_needed() {
    if (dir == NULL) {
        return;
    }
    closedir(dir);
    dir = NULL;
}

void AutoIndexer::inject_socketlike(ISocketLike *socket_like) {
    (void)socket_like;
}

bool AutoIndexer::is_originatable() const {
    return !originated_;
}

bool AutoIndexer::is_origination_started() const {
    return originated_;
}

bool AutoIndexer::is_reroutable() const {
    return false;
}

bool AutoIndexer::is_responsive() const {
    return originated_;
}

void AutoIndexer::start_origination(IObserver &observer) {
    (void)observer;
    scan_from_directory();
}

void AutoIndexer::leave() {
    delete this;
}

ResponseHTTP *AutoIndexer::respond(const RequestHTTP &request) {
    ResponseHTTP::header_list_type headers;
    IResponseDataConsumer::t_sending_mode sm = response_data.determine_sending_mode();
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
    headers.push_back(std::make_pair(HeaderHTTP::content_type, HTTP::strfy("text/html")));
    ResponseHTTP *res = new ResponseHTTP(request.get_http_version(), HTTP::STATUS_OK, &headers, &response_data);
    res->start();
    return res;
}
