#include "FileReader.hpp"
#include <unistd.h>
#define READ_SIZE 1024

FileReader::FileReader(const byte_string &path) : file_path_(HTTP::restrfy(path)), originated_(false), fd_(-1) {}
FileReader::FileReader(const char_string &path) : file_path_(path), originated_(false), fd_(-1) {}

FileReader::~FileReader() {
    close_if_needed();
}

void FileReader::notify(IObserver &observer, IObserver::observation_category cat, t_time_epoch_ms epoch) {
    (void)observer;
    (void)cat;
    (void)epoch;
    assert(false);
}

void FileReader::read_from_file() {
    // TODO: C++ way に書き直す
    if (originated_) {
        return;
    }
    errno  = 0;
    int fd = open(file_path_.c_str(), O_RDONLY | O_CLOEXEC);
    if (fd < 0) {
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
    fd_ = fd;
    char read_buf[READ_SIZE];
    ssize_t read_size;
    data_.clear();
    for (;;) {
        read_size = read(fd, read_buf, READ_SIZE);
        if (read_size < 0) {
            throw http_error("read error", HTTP::STATUS_FORBIDDEN);
        }
        response_data.inject(read_buf, read_size, read_size == 0);
        data_.insert(data_.end(), read_buf, read_buf + read_size);
        if (read_size == 0) {
            break;
        }
    }
    originated_ = true;
    close_if_needed();
}

void FileReader::close_if_needed() {
    if (fd_ < 0) {
        return;
    }
    close(fd_);
    fd_ = -1;
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

void FileReader::start_origination(IObserver &observer) {
    (void)observer;
    read_from_file();
}

void FileReader::leave() {
    delete this;
}

ResponseHTTP *FileReader::respond(const RequestHTTP &request) {
    ResponseHTTP *res = new ResponseHTTP(request.get_http_version(), HTTP::STATUS_OK, &response_data);
    res->start();
    return res;
}
