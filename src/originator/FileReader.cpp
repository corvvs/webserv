#include "FileReader.hpp"
#include <unistd.h>
#define READ_SIZE 1024

FileReader::FileReader(const byte_string &path) : file_path_(HTTP::restrfy(path)), is_ready_to_draw_(false) {}
FileReader::FileReader(const char_string &path) : file_path_(path), is_ready_to_draw_(false) {}

void FileReader::read_from_file() {
    // TODO: C++ way に書き直す
    if (is_ready_to_draw_) {
        return;
    }
    int fd = open(file_path_.c_str(), O_RDONLY);
    if (fd < 0) {
        throw http_error("can't open", HTTP::STATUS_FORBIDDEN);
    }
    char read_buf[READ_SIZE];
    ssize_t read_size;
    data_.clear();
    for (;;) {
        read_size = read(fd, read_buf, READ_SIZE);
        if (read_size < 0) {
            throw http_error("read error", HTTP::STATUS_FORBIDDEN);
        }
        if (read_size == 0) {
            break;
        }
        data_.insert(data_.end(), read_buf, read_buf + read_size);
    }
    is_ready_to_draw_ = true;
}

bool FileReader::is_ready_to_draw() const {
    return is_ready_to_draw_;
}

FileReader::byte_string FileReader::draw_data() const {
    return data_;
}
