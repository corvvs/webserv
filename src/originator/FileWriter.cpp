#include "FileWriter.hpp"
#include "../utils/ObjectHolder.hpp"
#include <unistd.h>
#define WRITE_SIZE 1024

FileWriter::FileWriter(const RequestMatchingResult &match_result, const byte_string &content_to_write)
    : file_path_(HTTP::restrfy(match_result.path_local)), content_to_write_(content_to_write), originated_(false) {}

FileWriter::~FileWriter() {}

void FileWriter::notify(IObserver &observer, IObserver::observation_category cat, t_time_epoch_ms epoch) {
    (void)observer;
    (void)cat;
    (void)epoch;
    assert(false);
}
void FileWriter::write_to_file() {
    // TODO: C++ way に書き直す
    if (originated_) {
        return;
    }
    // ファイルを上書きモードで開く.
    // 開けなかったらエラー.
    FDHolder fd_holder(open(file_path_.c_str(), O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0644));
    t_fd fd = fd_holder.value();
    if (fd < 0) {
        switch (errno) {
            case EACCES:
                throw http_error("permission denied", HTTP::STATUS_FORBIDDEN);
            default:
                VOUT(errno);
                throw http_error("can't open", HTTP::STATUS_FORBIDDEN);
        }
    }
    // データを一気に書き込む
    byte_string::size_type write_head = 0;
    ssize_t written_size              = 0;
    for (; write_head < content_to_write_.size();) {
        byte_string::size_type write_max
            = std::min((byte_string::size_type)WRITE_SIZE, content_to_write_.size() - write_head);

        written_size = write(fd, &content_to_write_[write_head], write_max);
        if (written_size < 0) {
            throw http_error("write error", HTTP::STATUS_FORBIDDEN);
        }
        write_head += written_size;
        if (written_size == 0) {
            DXOUT("short?");
            break;
        }
    }
    // 書き込んだサイズを文字列変換してボディに突っ込む
    HTTP::byte_string written_size_str = ParserHelper::utos(write_head, 10);
    response_data.inject(written_size_str, true);
    originated_ = true;
}

void FileWriter::inject_socketlike(ISocketLike *socket_like) {
    (void)socket_like;
}

bool FileWriter::is_originatable() const {
    return !originated_;
}

bool FileWriter::is_origination_started() const {
    return originated_;
}

bool FileWriter::is_reroutable() const {
    return false;
}

HTTP::byte_string FileWriter::reroute_path() const {
    assert(false);
    return HTTP::byte_string();
}

bool FileWriter::is_responsive() const {
    return originated_;
}

void FileWriter::start_origination(IObserver &observer) {
    (void)observer;
    write_to_file();
}

void FileWriter::leave() {
    delete this;
}

ResponseHTTP *FileWriter::respond(const RequestHTTP *request, bool should_close) {
    response_data.determine_sending_mode();
    ResponseHTTP *res
        = new ResponseHTTP(request->get_http_version(), HTTP::STATUS_OK, NULL, &response_data, should_close);
    return res;
}
