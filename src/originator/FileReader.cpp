#include "FileReader.hpp"
#include "../utils/MIME.hpp"
#include <unistd.h>
#include <cerrno>
#define READ_SIZE 1048576

FileReader::FileReader(const RequestMatchingResult &match_result, FileCacher &cacher)
    : file_path_(HTTP::restrfy(match_result.path_local)), originated_(false), cacher_(cacher) {}

FileReader::FileReader(const char_string &path, FileCacher &cacher)
    : file_path_(path), originated_(false), cacher_(cacher) {}

FileReader::~FileReader() {}

void FileReader::notify(IObserver &observer, IObserver::observation_category cat, t_time_epoch_ms epoch) {
    (void)observer;
    (void)cat;
    (void)epoch;
    assert(false);
}

bool FileReader::read_from_cache() {
    std::pair<minor_error, const FileCacher::entry_type *> res = cacher_.fetch(file_path_.c_str());
    if (res.first.is_ok()) {
        // Deep copyする
        byte_string file_data = res.second->data;
        size_t file_size      = res.second->size;
        response_data.inject(HTTP::restrfy(file_data).c_str(), file_size, true);
        originated_ = true;
        return true;
    }

    // ファイルのサイズがキャシュできる上限を超えている以外のエラーが発生した場合は例外を投げる
    const minor_error merror = res.first;
    if (merror.second != HTTP::STATUS_BAD_REQUEST) {
        throw http_error(merror.first.c_str(), merror.second);
    }
    return false;
}

minor_error FileReader::read_from_file() {
    // TODO: C++ way に書き直す
    errno = 0;
    // ファイルを読み込み用に開く
    // 開けなかったらエラー
    int fd = open(file_path_.c_str(), O_RDONLY | O_CLOEXEC);
    if (fd < 0) {
        switch (errno) {
            case ENOENT:
                return minor_error::make("file not found", HTTP::STATUS_NOT_FOUND);
            case EACCES:
                return minor_error::make("permission denied", HTTP::STATUS_FORBIDDEN);
            default:
                VOUT(errno);
                return minor_error::make("can't open", HTTP::STATUS_FORBIDDEN);
        }
    }
    // 読んでデータリストに注入
    byte_string read_buf;
    read_buf.resize(READ_SIZE);

    ssize_t read_size;
    for (;;) {
        read_size = read(fd, &read_buf.front(), read_buf.size());
        if (read_size < 0) {
            close(fd);
            return minor_error::make("read error", HTTP::STATUS_FORBIDDEN);
        }
        read_buf.resize(read_size);
        response_data.inject(read_buf, read_size == 0);
        if (read_size == 0) {
            break;
        }
    }
    close(fd);
    return minor_error::ok();
}

void FileReader::inject_socketlike(ISocketLike *socket_like) {
    (void)socket_like;
}

bool FileReader::is_originatable() const {
    return !originated_;
}

bool FileReader::is_origination_started() const {
    return originated_;
}

bool FileReader::is_reroutable() const {
    return false;
}

HTTP::byte_string FileReader::reroute_path() const {
    assert(false);
    return HTTP::byte_string();
}

bool FileReader::is_responsive() const {
    return originated_;
}

// キャッシュデータが存在するならデータリストにinjectするだけ
// なかったらファイルを読みこんでキャッシュを更新する
void FileReader::start_origination(IObserver &observer) {
    (void)observer;
    if (originated_) {
        return;
    }
    // if (read_from_cache()) {
    //     originated_ = true;
    //     return;
    // }
    const minor_error me = read_from_file();
    if (me.is_error()) {
        throw http_error(me);
    }
    originated_ = true;
}

void FileReader::leave() {
    delete this;
}

HTTP::byte_string FileReader::infer_content_type() const {
    HTTP::CH::ContentType ct;
    HTTP::char_string::size_type dot = file_path_.find_last_of(".");
    HTTP::byte_string mt;
    if (dot != HTTP::char_string::npos) {
        mt = HTTP::MIME::mime_type_for_extension(HTTP::strfy(file_path_.substr(dot + 1)));
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
    // Content-Type: の推測
    headers.push_back(std::make_pair(HeaderHTTP::content_type, infer_content_type()));
    return headers;
}

ResponseHTTP *FileReader::respond(const RequestHTTP *request) {
    const IResponseDataConsumer::t_sending_mode sm = response_data.determine_sending_mode();
    ResponseHTTP::header_list_type headers         = determine_response_headers(sm);
    ResponseHTTP *res = new ResponseHTTP(request->get_http_version(), HTTP::STATUS_OK, &headers, &response_data, false);
    res->start();
    return res;
}
