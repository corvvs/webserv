#include "FileReader.hpp"
#include <unistd.h>
#define READ_SIZE 1024

FileReader::FileReader(const RequestMatchingResult &match_result, FileCacher *cacher)
    : file_path_(HTTP::restrfy(match_result.path_local)), originated_(false), cacher_(cacher) {}

FileReader::~FileReader() {}

void FileReader::notify(IObserver &observer, IObserver::observation_category cat, t_time_epoch_ms epoch) {
    (void)observer;
    (void)cat;
    (void)epoch;
    assert(false);
}

bool FileReader::read_from_cache() {
    std::pair<minor_error, const FileCacher::entry_type *> res = cacher_->fetch(file_path_.c_str());
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

void FileReader::read_from_file() {
    // キャッシュに追加せずに読み込んでいく
    int fd = open(file_path_.c_str(), O_RDONLY | O_CLOEXEC);
    if (fd < 0) {
        throw http_error("can't open", HTTP::STATUS_FORBIDDEN);
    }
    // 読んでデータリストに注入
    char read_buf[READ_SIZE];
    ssize_t read_size;
    for (;;) {
        read_size = read(fd, read_buf, READ_SIZE);
        if (read_size < 0) {
            close(fd);
            throw http_error("read error", HTTP::STATUS_FORBIDDEN);
        }
        response_data.inject(read_buf, read_size, read_size == 0);
        if (read_size == 0) {
            break;
        }
    }
    originated_ = true;
    close(fd);
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

// キャッシュデータが存在するならデータリストにinjectするだけ
// なかったらファイルを読みこんでキャッシュを更新する
void FileReader::start_origination(IObserver &observer) {
    (void)observer;
    if (originated_) {
        return;
    }
    if (!read_from_cache()) {
        read_from_file();
    }
}

void FileReader::leave() {
    delete this;
}

ResponseHTTP *FileReader::respond(const RequestHTTP &request) {
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
    ResponseHTTP *res = new ResponseHTTP(request.get_http_version(), HTTP::STATUS_OK, &headers, &response_data);
    res->start();
    return res;
}
