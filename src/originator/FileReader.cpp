#include "FileReader.hpp"
#include <unistd.h>
#define READ_SIZE 1024

FileReader::FileReader(const RequestMatchingResult &match_result)
    : file_path_(HTTP::restrfy(match_result.path_local)), originated_(false) {}

FileReader::FileReader(const char_string &path) : file_path_(path), originated_(false) {}

FileReader::~FileReader() {}

void FileReader::notify(IObserver &observer, IObserver::observation_category cat, t_time_epoch_ms epoch) {
    (void)observer;
    (void)cat;
    (void)epoch;
    assert(false);
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
    char read_buf[READ_SIZE];
    ssize_t read_size;
    for (;;) {
        read_size = read(fd, read_buf, READ_SIZE);
        if (read_size < 0) {
            close(fd);
            return minor_error::make("read error", HTTP::STATUS_FORBIDDEN);
        }
        response_data.inject(read_buf, read_size, read_size == 0);
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

bool FileReader::is_responsive() const {
    return originated_;
}

void FileReader::start_origination(IObserver *observer) {
    (void)observer;
    if (originated_) {
        return;
    }
    const minor_error me = read_from_file();
    if (me.is_error()) {
        throw http_error(me);
    }
    originated_ = true;
}

void FileReader::leave() {
    delete this;
}

ResponseHTTP *FileReader::respond(const RequestHTTP *request) {
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
    ResponseHTTP *res = new ResponseHTTP(request->get_http_version(), HTTP::STATUS_OK, &headers, &response_data, false);
    res->start();
    return res;
}
