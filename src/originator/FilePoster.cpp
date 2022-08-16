#include "FilePoster.hpp"
#include "../event/time.hpp"
#include "../utils/File.hpp"
#include <sys/stat.h>
#include <unistd.h>
#define WRITE_SIZE 1024
#define NON_FD -1

// アップロードされるファイルの名前を決定する
HTTP::byte_string new_file_name(const HTTP::CH::ContentDisposition &content_disposition, size_t serial) {
    HTTP::byte_string file_name;
    // 現在時刻を文字列化したものを接頭辞とする
    file_name = ParserHelper::utos(WSTime::get_epoch_ms(), 10) + ParserHelper::utos(WSTime::get_epoch_us(), 10);
    file_name += HTTP::strfy("0");
    file_name += ParserHelper::utos(serial, 10);
    // Content-Disposition: がパラメータ filename を持っていれば、その値を _ で繋いで追記する
    const HTTP::IDictHolder::parameter_dict &params       = content_disposition.parameters;
    HTTP::IDictHolder::parameter_dict::const_iterator res = params.find(HTTP::strfy("filename"));
    if (res != params.end()) {
        const HTTP::byte_string decoded = ParserHelper::decode_pct_encoded(res->second.unquote());
        file_name += HTTP::strfy("_");
        file_name += decoded;
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
    : directory_path_(HTTP::restrfy(match_result.path_local)), originated_(false), content_provider(request) {
    // リクエストの Content-Type: が "multipart/form-data" でかつ正しい boundary パラメータがあれば,
    // マルチパートとみなして処理する.
    const HTTP::CH::ContentType &ct = content_provider.get_content_type_item();
    boundary                        = ct.boundary;
}

FilePoster::~FilePoster() {}

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
    extract_file_entries();
    // FileEntryを1つずつアップロード
    for (size_t i = 0; i < entries.size(); ++i) {
        write_file(entries[i]);
    }
    response_data.inject("", 0, true);
    originated_ = true;
}

void FilePoster::check_target_directory() {
    QVOUT(directory_path_);
    errno = 0;
    if (file::is_dir(directory_path_)) {
        return;
    }
    switch (errno) {
        case ENOENT:
            throw http_error("file not found", HTTP::STATUS_NOT_FOUND);
        case EACCES:
            throw http_error("can't search file", HTTP::STATUS_FORBIDDEN);
        default:
            throw http_error("something wrong", HTTP::STATUS_INTERNAL_SERVER_ERROR);
    }
}

void FilePoster::extract_file_entries() {
    body = content_provider.get_body();
    if (boundary.size() > 0) {
        decompose_multipart(body, boundary);
    } else {
        byte_string file_name = new_file_name(content_provider.get_content_disposition_item(), 1);
        entries.push_back(FileEntry(file_name, body));
    }
    // ファイル名にディレクトリパスを結合
    for (size_t i = 0; i < entries.size(); ++i) {
        entries[i].name = HTTP::Utils::join_path(HTTP::strfy(directory_path_), entries[i].name);
    }
}

void FilePoster::decompose_multipart(const light_string &body, const light_string &boundary) {
    light_string str(body);
    light_string::size_type part_from       = light_string::npos;
    light_string::size_type part_to         = light_string::npos;
    const HTTP::byte_string boundary_prefix = HTTP::strfy("--");
    const HTTP::byte_string boundary_suffix = HTTP::strfy("--");
    // リクエストボディを boundary で区切る.
    while (true) {
        light_string::size_type i = str.find(ParserHelper::CRLF);
        if (i == light_string::npos) {
            break;
        }
        const light_string line = str.substr(0, i);
        light_string working    = line;
        bool detected_closer    = false;
        if (working.starts_with(boundary_prefix)) {
            working = working.substr(boundary_prefix.size());
            if (working.starts_with(boundary)) {
                working = working.substr(boundary.size());
                if (working.starts_with(boundary_suffix)) {
                    // 閉じ境界区切子行?
                    DXOUT("is close?");
                    working         = working.substr(boundary_suffix.size());
                    detected_closer = true;
                } else {
                    DXOUT("is ordinary?");
                }
                if (working.find_first_not_of(ParserHelper::LWS) == light_string::npos) {
                    // 行末までに空白でない文字がない -> boundaryである, と考える
                    part_to = line.get_first() - ParserHelper::CRLF.size();
                    if (part_from != light_string::npos) {
                        // [part_from, part_to) をサブパートとして解析
                        const light_string subpart(body, part_from, part_to);
                        analyze_subpart(subpart);
                    }
                    part_from = line.get_last() + ParserHelper::CRLF.size();
                }
            }
        }
        str = str.substr(line.size() + ParserHelper::CRLF.size());
        if (detected_closer) {
            break;
        }
    }
}

void FilePoster::analyze_subpart(const light_string &subpart) {
    // ヘッダと本文の境界を見つける
    byte_string crlfcrlf      = ParserHelper::CRLF + ParserHelper::CRLF;
    light_string::size_type i = subpart.find(crlfcrlf);
    if (i == light_string::npos) {
        throw http_error("bad subpart", HTTP::STATUS_BAD_REQUEST);
    }
    // ヘッダ部を解析
    const light_string header_part = subpart.substr(0, i + crlfcrlf.size() / 2);
    HeaderHolderSubpart holder;
    holder.parse_header_lines(header_part, &holder);

    MultiPart::CH::ContentType content_type;
    MultiPart::CH::ContentDisposition content_disposition;

    content_type.determine(holder);
    content_disposition.determine(holder);
    if (content_disposition.value != HTTP::strfy("form-data")) {
        return;
    }
    // ヘッダ部の解析結果をもとにファイル名を決定
    byte_string file_name = new_file_name(content_disposition, entries.size() + 1);
    // FileEntryを生成
    const light_string body_part = subpart.substr(i + crlfcrlf.size());
    entries.push_back(FileEntry(file_name, body_part, content_type, content_disposition));
}

void FilePoster::write_file(const FileEntry &file) const {
    int fd = open(HTTP::restrfy(file.name).c_str(), O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0444);
    // 0444 なのはアップロードされたファイルを即時実行させないため
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
        light_string::size_type write_max = 8192;
        if (write_max > file_size - written) {
            write_max = file_size - written;
        }
        ssize_t written_size = write(fd, &file.content[0] + written, write_max);
        if (written_size < 0) {
            close(fd);
            throw http_error("write error", HTTP::STATUS_FORBIDDEN);
        }
        if (written_size == 0) {
            DXOUT("Imcomplete?");
            break;
        }
        written += written_size;
    }
    close(fd);
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

ResponseHTTP *FilePoster::respond(const RequestHTTP *request) {
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
