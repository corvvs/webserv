#include "FileReader.hpp"
#include "../utils/File.hpp"
#include "../utils/MIME.hpp"
#include "../utils/ObjectHolder.hpp"
#include <cerrno>
#include <unistd.h>
#define READ_SIZE 1048576

FileReader::Attribute::Attribute(FileCacher &cacher_,
                                 const ICacheInfoProvider *cache_info_provider,
                                 char_string file_path_)
    : cacher_(cacher_)
    , cache_info_provider(cache_info_provider)
    , file_path_(file_path_)
    , observer(NULL)
    , master(NULL)
    , fd_(-1) {}

void FileReader::Attribute::close_fd() throw() {
    if (fd_ >= 0) {
        close(fd_);
        fd_ = -1;
    }
}

FileReader::Status::Status()
    : last_modified(0), originated(false), is_not_modified(false), leaving(false), is_responsive(false) {}

FileReader::FileReader(const RequestMatchingResult &match_result,
                       FileCacher &cacher,
                       const ICacheInfoProvider *info_provider)
    : attr(cacher, info_provider, HTTP::restrfy(match_result.path_local)) {}

FileReader::FileReader(const char_string &path, FileCacher &cacher) : attr(cacher, NULL, path) {}

FileReader::~FileReader() {
    attr.close_fd();
}

t_fd FileReader::get_fd() const {
    return attr.fd_;
}

t_port FileReader::get_port() const {
    return 0;
}

void FileReader::notify(IObserver &observer, IObserver::observation_category cat, t_time_epoch_ms epoch) {
    if (status.leaving) {
        return;
    }
    if (attr.master) {
        switch (cat) {
            case IObserver::OT_READ:
            case IObserver::OT_EXCEPTION:
            case IObserver::OT_TIMEOUT:
                retransmit(observer, cat, epoch);
                return;
            default:
                break;
        }
    }
    switch (cat) {
        case IObserver::OT_READ:
        case IObserver::OT_ORIGINATOR_READ:
            perform_receiving(observer);
            return;
        case IObserver::OT_TIMEOUT:
        case IObserver::OT_ORIGINATOR_TIMEOUT:
            // if (lifetime.is_timeout(epoch)) {
            //     DXOUT("TIMEOUT!!");
            //     throw http_error("cgi-script timed out", HTTP::STATUS_TIMEOUT);
            // }
            return;
        default:
            DXOUT("unexpected cat: " << cat);
            assert(false);
    }
}

void FileReader::retransmit(IObserver &observer, IObserver::observation_category cat, t_time_epoch_ms epoch) {
    assert(attr.master != NULL);
    switch (cat) {
        case IObserver::OT_READ:
            cat = IObserver::OT_ORIGINATOR_READ;
            break;
        case IObserver::OT_EXCEPTION:
            cat = IObserver::OT_ORIGINATOR_EXCEPTION;
            break;
        case IObserver::OT_TIMEOUT:
            cat = IObserver::OT_ORIGINATOR_TIMEOUT;
            break;
        default:
            assert(false);
    }
    attr.master->notify(observer, cat, epoch);
}

bool FileReader::read_from_cache() {
    std::pair<minor_error, const FileCacher::entry_type *> res = attr.cacher_.fetch(attr.file_path_.c_str());
    if (res.first.is_ok()) {
        // Deep copyする
        byte_string file_data = res.second->data;
        size_t file_size      = res.second->size;
        response_data.inject(HTTP::restrfy(file_data).c_str(), file_size, true);
        status.originated = true;
        return true;
    }

    // ファイルのサイズがキャシュできる上限を超えている以外のエラーが発生した場合は例外を投げる
    const minor_error merror = res.first;
    if (merror.second != HTTP::STATUS_BAD_REQUEST) {
        throw http_error(merror.first.c_str(), merror.second);
    }
    return false;
}

bool FileReader::prepare_reading() {
    // TODO: C++ way に書き直す
    QVOUT(attr.file_path_);
    {
        status.is_not_modified = false;
        // 最終更新時刻のチェック
        time_t lut = file::get_last_update_time(attr.file_path_);
        if (lut > 0) {
            status.last_modified = lut * 1000;

            if (attr.cache_info_provider != NULL) {
                t_time_epoch_ms if_modified_since = attr.cache_info_provider->get_if_modified_since().value;
                if (if_modified_since > 0 && if_modified_since >= status.last_modified) {
                    // 更新なし!!
                    response_data.inject(NULL, 0, true);
                    status.is_not_modified = true;
                    return false;
                }
            }
        } else {
            status.last_modified = 0;
        }
    }

    errno = 0;
    // ファイルを読み込み用に開く
    // 開けなかったらエラー
    attr.fd_      = open(attr.file_path_.c_str(), O_RDONLY | O_CLOEXEC);
    const t_fd fd = attr.fd_;
    if (fd < 0) {
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
    {
        int rv = fcntl(fd, F_SETFL, O_NONBLOCK);
        if (rv < 0) {
            throw http_error("failed to set nonblock", HTTP::STATUS_INTERNAL_SERVER_ERROR);
        }
    }
    return true;
}

void FileReader::perform_receiving(IObserver &observer) {
    DXOUT("FR on Read");
    // 読んでデータリストに注入
    byte_string read_buf;
    read_buf.resize(READ_SIZE);

    ssize_t read_size = read(attr.fd_, &read_buf.front(), read_buf.size());
    VOUT(read_size);
    if (read_size < 0) {
        throw http_error("read error", HTTP::STATUS_FORBIDDEN);
    }
    read_buf.resize(read_size);
    const bool is_eof = read_size == 0;
    response_data.inject(read_buf, is_eof);
    VOUT(is_eof);
    if (!is_eof) {
        return;
    }
    observer.reserve_unset(this, IObserver::OT_READ);
    status.is_responsive = true;
}

void FileReader::inject_socketlike(ISocketLike *socket_like) {
    attr.master = socket_like;
}

bool FileReader::is_originatable() const {
    return !status.originated;
}

bool FileReader::is_origination_started() const {
    return status.originated;
}

bool FileReader::is_reroutable() const {
    return false;
}

HTTP::byte_string FileReader::reroute_path() const {
    assert(false);
    return HTTP::byte_string();
}

bool FileReader::is_responsive() const {
    return status.is_responsive;
}

// キャッシュデータが存在するならデータリストにinjectするだけ
// なかったらファイルを読みこんでキャッシュを更新する
void FileReader::start_origination(IObserver &observer) {
    if (status.originated) {
        return;
    }
    const bool do_read = prepare_reading();
    if (do_read) {
        attr.observer = &observer;
        observer.reserve_hold(this);
        observer.reserve_set(this, IObserver::OT_READ);
    }
    status.is_responsive = !do_read;
    status.originated    = true;
}

void FileReader::leave() {
    DXOUT("now leaving...");
    if (status.leaving) {
        DXOUT("now already leaving.");
        return;
    }
    DXOUT("leaving.");
    status.leaving = true;
    VOUT(attr.observer);
    const bool is_under_observation = (attr.observer != NULL);
    if (is_under_observation) {
        attr.observer->reserve_unhold(this);
    } else {
        // Observerに渡される前に leave されることがある
        DXOUT("leaving immediately.");
        delete this;
    }
}

HTTP::byte_string FileReader::infer_content_type() const {
    HTTP::CH::ContentType ct;
    HTTP::char_string::size_type dot = attr.file_path_.find_last_of(".");
    HTTP::byte_string mt;
    if (dot != HTTP::char_string::npos) {
        mt = HTTP::MIME::mime_type_for_extension(HTTP::strfy(attr.file_path_.substr(dot + 1)));
    }
    if (!mt.empty()) {
        ct.value = mt;
    } else {
        bool is_text = false;
        if (!response_data.empty()) {
            HTTP::light_string body = response_data.top();
            body                    = body.substr(0, 10000);
            is_text                 = body.find_first_of(HTTP::CharFilter::nul) == HTTP::light_string::npos;
        }
        if (is_text) {
            ct.value = HTTP::strfy("text/plain");
        } else {
            ct.value = HTTP::CH::ContentType::default_value;
        }
    }
    const bool type_is_text = HTTP::light_string(ct.value).starts_with("text/");
    if (type_is_text) {
        bool is_ascii_only = true;
        if (!response_data.empty()) {
            HTTP::light_string body = response_data.top();
            body                    = body.substr(0, 10000);
            is_ascii_only           = body.find_first_not_of(HTTP::CharFilter::ascii) == HTTP::light_string::npos;
        }
        ct.charset = HTTP::strfy(is_ascii_only ? "US-ASCII" : "UTF-8");
    }
    return ct.serialize();
}

ResponseHTTP::header_list_type
FileReader::determine_response_headers(const IResponseDataConsumer::t_sending_mode sm) const {
    ResponseHTTP::header_list_type headers;
    if (!status.is_not_modified) {
        switch (sm) {
            case ResponseDataList::SM_CHUNKED:
                headers.push_back(std::make_pair(HeaderHTTP::transfer_encoding, HTTP::strfy("chunked")));
                break;
            case ResponseDataList::SM_NOT_CHUNKED:
                headers.push_back(std::make_pair(HeaderHTTP::content_length,
                                                 ParserHelper::utos(response_data.current_total_size(), 10)));
                break;
            default:
                break;
        }
    }
    // Content-Type: の推測
    headers.push_back(std::make_pair(HeaderHTTP::content_type, infer_content_type()));
    // Last-Modifiedがもしあれば
    if (status.last_modified > 0) {
        headers.push_back(
            std::make_pair(HeaderHTTP::last_modified, HTTP::CH::LastModified(status.last_modified).serialize()));
    }
    return headers;
}

ResponseHTTP *FileReader::respond(const RequestHTTP *request, bool should_close) {
    const IResponseDataConsumer::t_sending_mode sm = response_data.determine_sending_mode();
    ResponseHTTP::header_list_type headers         = determine_response_headers(sm);
    const HTTP::t_status status_code = status.is_not_modified ? HTTP::STATUS_NOT_MODIFIED : HTTP::STATUS_OK;
    ResponseHTTP *res
        = new ResponseHTTP(request->get_http_version(), status_code, &headers, &response_data, should_close);
    return res;
}
