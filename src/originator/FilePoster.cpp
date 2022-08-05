#include "FilePoster.hpp"
#include "../event/time.hpp"
#include <sys/stat.h>
#include <unistd.h>
#define WRITE_SIZE 1024
#define NON_FD -1

HTTP::byte_string new_file_name(const HTTP::CH::ContentDisposition &content_disposition, size_t serial) {
    HTTP::byte_string file_name;
    const HTTP::IDictHolder::parameter_dict &params = content_disposition.parameters;
    file_name = ParserHelper::utos(WSTime::get_epoch_ms(), 10) + ParserHelper::utos(WSTime::get_epoch_us(), 10);
    HTTP::IDictHolder::parameter_dict::const_iterator res = params.find(HTTP::strfy("filename"));
    file_name += HTTP::strfy("0");
    file_name += ParserHelper::utos(serial, 10);
    if (res != params.end()) {
        file_name += HTTP::strfy("_");
        file_name += res->second.unquote().str();
    }
    return file_name;
}

FilePoster::FileEntry::FileEntry(const byte_string &name_, const light_string &content_)
    : name(name_), content(content_) {}

FilePoster::FileEntry::FileEntry(const byte_string &name_,
                                 const light_string &content_,
                                 const MultiPart::CH::ContentType &content_type_,
                                 const MultiPart::CH::ContentDisposition &content_disposition_)
    : name(name_), content(content_), content_type(content_type_), content_disposition(content_disposition_) {}

FilePoster::FilePoster(const RequestMatchingResult &match_result, const IContentProvider &request)
    : directory_path_(HTTP::restrfy(match_result.path_local))
    , originated_(false)
    , fd_(NON_FD)
    , content_provider(request) {
    const HTTP::CH::ContentType &ct                       = content_provider.get_content_type_item();
    HTTP::IDictHolder::parameter_dict::const_iterator res = ct.parameters.find(HTTP::strfy("boundary"));
    if (ct.value == "multipart/form-data" && res != ct.parameters.end() && res->second.size() > 0) {
        // マルチパートである
        boundary = res->second;
    }
}

FilePoster::~FilePoster() {
    close_if_needed();
}

void FilePoster::notify(IObserver &observer, IObserver::observation_category cat, t_time_epoch_ms epoch) {
    (void)observer;
    (void)cat;
    (void)epoch;
    assert(false);
}

void FilePoster::post_files() {
    // ターゲットがディレクトリであることを確認
    check_target_directory();
    // ボディからFileEntryのリストを生成
    analyze_body();
    // FileEntryを1つずつアップロード
    for (size_t i = 0; i < entries.size(); ++i) {
        post_file(entries[i]);
    }
    response_data.inject("", 0, true);
    originated_ = true;
}

void FilePoster::check_target_directory() {
    struct stat st;
    errno = 0;
    QVOUT(directory_path_);
    const int result = stat(directory_path_.c_str(), &st);
    if (result != 0) {
        switch (errno) {
            case ENOENT:
                throw http_error("file not found", HTTP::STATUS_NOT_FOUND);
            case EACCES:
                throw http_error("can't search file", HTTP::STATUS_FORBIDDEN);
            default:
                throw http_error("something wrong", HTTP::STATUS_INTERNAL_SERVER_ERROR);
        }
    }
    if (!S_ISDIR(st.st_mode)) {
        throw http_error("not for a directory", HTTP::STATUS_INTERNAL_SERVER_ERROR);
    }
}

void FilePoster::analyze_body() {
    body = content_provider.get_body();
    if (boundary.size() > 0) {
        decompose_multipart(body, boundary);
    } else {
        byte_string file_name = new_file_name(content_provider.get_content_disposition_item(), 1);
        entries.push_back(FileEntry(file_name, body));
    }
    for (size_t i = 0; i < entries.size(); ++i) {
        entries[i].name = HTTP::Utils::join_path(HTTP::strfy(directory_path_), entries[i].name);
        VOUT(entries[i].name);
        VOUT(entries[i].content_type.value);
        VOUT(entries[i].content.size());
    }
}

void FilePoster::decompose_multipart(const light_string &body, const light_string &boundary) {
    light_string str(body);
    light_string::size_type part_from = light_string::npos;
    light_string::size_type part_to   = light_string::npos;
    while (true) {
        light_string::size_type i = str.find(ParserHelper::CRLF);
        if (i == light_string::npos) {
            break;
        }
        const light_string whole_line = str.substr(0, i);
        light_string line             = whole_line;
        if (line.substr(0, 2) == "--") {
            line = line.substr(2);
            if (line.substr(0, boundary.size()) == boundary) {
                line = line.substr(boundary.size());
                if (line.substr(0, 2) == "--") {
                    // 閉じ境界区切子行?
                    DXOUT("is close?");
                    line = line.substr(2);
                } else {
                    DXOUT("is ordinary?");
                }
                if (line.find_first_not_of(ParserHelper::LWS) != light_string::npos) {
                    // 行末までに空白でない文字がある
                    // -> Bad
                    throw http_error("invalid boundary delimiter line", HTTP::STATUS_BAD_REQUEST);
                }
                part_to = whole_line.get_first() - 2;
                if (part_from != light_string::npos) {
                    // [part_from, part_to) をサブパートとして解析
                    const light_string subpart(body, part_from, part_to);
                    analyze_subpart(subpart);
                }
                part_from = whole_line.get_last() + 2;
            }
        }
        str = str.substr(i + ParserHelper::CRLF.size());
    }
}

void FilePoster::analyze_subpart(const light_string &subpart) {
    byte_string crlfcrlf      = ParserHelper::CRLF + ParserHelper::CRLF;
    light_string::size_type i = subpart.find(crlfcrlf);
    if (i == light_string::npos) {
        throw http_error("bad subpart", HTTP::STATUS_BAD_REQUEST);
    }
    const light_string header_part = subpart.substr(0, i + crlfcrlf.size() / 2);
    const light_string body_part   = subpart.substr(i + crlfcrlf.size());
    HeaderHolderSubpart holder;
    holder.parse_header_lines(header_part, &holder);

    MultiPart::CH::ContentType content_type;
    MultiPart::CH::ContentDisposition content_disposition;

    content_type.determine(holder);
    VOUT(content_type.value);
    content_disposition.determine(holder);
    if (content_disposition.value != HTTP::strfy("form-data")) {
        return;
    }
    byte_string file_name = new_file_name(content_disposition, entries.size() + 1);
    entries.push_back(FileEntry(file_name, body_part, content_type, content_disposition));
}

void FilePoster::post_file(const FileEntry &file) const {
    int fd = open(HTTP::restrfy(file.name).c_str(), O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0444);
    if (fd < 0) {
        switch (errno) {
            case EACCES:
                throw http_error("permission denied", HTTP::STATUS_FORBIDDEN);
            default:
                QVOUT(strerror(errno));
                throw http_error("can't open", HTTP::STATUS_FORBIDDEN);
        }
    }
    const light_string::size_type file_size = file.content.size();
    light_string::size_type written         = 0;
    for (; written < file_size;) {
        VOUT(written);
        BVOUT(file.content);
        light_string::size_type write_max = 8192;
        if (write_max > file_size - written) {
            write_max = file_size - written;
        }
        ssize_t written_size = write(fd, &file.content[0] + written, write_max);
        if (written_size < 0) {
            close(fd);
            throw http_error("read error", HTTP::STATUS_FORBIDDEN);
        }
        if (written_size == 0) {
            DXOUT("Imcomplete?");
            break;
        }
        written += written_size;
    }
    close(fd);
}

void FilePoster::close_if_needed() {
    if (fd_ < 0) {
        return;
    }
    close(fd_);
    fd_ = NON_FD;
}

void FilePoster::inject_socketlike(ISocketLike *socket_like) {
    (void)socket_like;
}

bool FilePoster::is_originatable() const {
    return !originated_ && content_provider.is_complete();
}

bool FilePoster::is_origination_started() const {
    return originated_;
}

bool FilePoster::is_reroutable() const {
    return false;
}

bool FilePoster::is_responsive() const {
    return originated_;
}

void FilePoster::start_origination(IObserver &observer) {
    (void)observer;
    post_files();
}

void FilePoster::leave() {
    delete this;
}

ResponseHTTP *FilePoster::respond(const RequestHTTP &request) {
    response_data.determine_sending_mode();
    ResponseHTTP *res = new ResponseHTTP(request.get_http_version(), HTTP::STATUS_OK, NULL, &response_data);
    res->start();
    return res;
}