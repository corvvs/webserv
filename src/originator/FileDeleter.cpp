#include "FileDeleter.hpp"
#include <cerrno>
#include <unistd.h>
#define WRITE_SIZE 1024
#define NON_FD -1

FileDeleter::FileDeleter(const RequestMatchingResult &match_result)
    : file_path_(HTTP::restrfy(match_result.path_local)), originated_(false) {}

FileDeleter::~FileDeleter() {}

void FileDeleter::notify(IObserver &observer, IObserver::observation_category cat, t_time_epoch_ms epoch) {
    (void)observer;
    (void)cat;
    (void)epoch;
    assert(false);
}

void FileDeleter::delete_file() {
    // TODO: C++ way に書き直す
    if (originated_) {
        return;
    }

    // ターゲットパスに対して削除を試みる.
    // できなかったらエラー.
    int rv = unlink(file_path_.c_str());
    if (rv < 0) {
        switch (errno) {
            case ENOENT:
                throw http_error("file not found", HTTP::STATUS_NOT_FOUND);
            case EACCES:
                throw http_error("permission denied", HTTP::STATUS_FORBIDDEN);
            default:
                VOUT(errno);
                throw http_error("can't delete", HTTP::STATUS_FORBIDDEN);
        }
        return;
    }
    const byte_string msg = HTTP::strfy("\"" + file_path_ + "\"" + " was successfully deleted.\n");
    response_data.inject(msg, false);
    response_data.inject("", 0, true);
    originated_ = true;
}

void FileDeleter::inject_socketlike(ISocketLike *socket_like) {
    (void)socket_like;
}

bool FileDeleter::is_originatable() const {
    return !originated_;
}

bool FileDeleter::is_origination_started() const {
    return originated_;
}

bool FileDeleter::is_reroutable() const {
    return false;
}

HTTP::byte_string FileDeleter::reroute_path() const {
    assert(false);
    return HTTP::byte_string();
}

bool FileDeleter::is_responsive() const {
    return originated_;
}

void FileDeleter::start_origination(IObserver &observer) {
    (void)observer;
    delete_file();
}

void FileDeleter::leave() {
    delete this;
}

ResponseHTTP *FileDeleter::respond(const RequestHTTP *request, bool should_close) {
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
    ResponseHTTP *res
        = new ResponseHTTP(request->get_http_version(), HTTP::STATUS_OK, &headers, &response_data, should_close);
    return res;
}
