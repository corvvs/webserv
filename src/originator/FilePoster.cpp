#include "FilePoster.hpp"
#include "../event/time.hpp"
#include "../utils/File.hpp"
#include "../utils/ObjectHolder.hpp"
#include <cerrno>
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
    const HTTP::IDictHolder::parameter_dict &params = content_disposition.parameters;
    HTTP::IDictHolder::parameter_dict::const_iterator res
        = params.find(ParserHelper::normalize_header_key(HTTP::strfy("filename")));
    if (res != params.end()) {
        const HTTP::byte_string decoded = ParserHelper::decode_pct_encoded(res->second.unquote());
        file_name += HTTP::strfy("_");
        file_name += decoded;
    }
    return file_name;
}

// [FileEntry InnerClasses]

FilePoster::FileEntry::Attribute::Attribute() : observer(NULL), master(NULL), fd_(-1) {}

void FilePoster::FileEntry::Attribute::close_fd() throw() {
    if (fd_ >= 0) {
        close(fd_);
        DXOUT("closed: " << fd_);
        fd_ = -1;
    } else {
        DXOUT("no close");
    }
}

t_fd FilePoster::FileEntry::get_fd() const {
    return attr.fd_;
}

t_port FilePoster::FileEntry::get_port() const {
    return 0;
}

FilePoster::FileEntry::Status::Status() : leaving(false), written_size(0), originated(false), is_over(false) {}

// [FileEntry]

FilePoster::FileEntry::FileEntry(const byte_string &name_,
                                 const light_string &content_,
                                 IObserver *observer,
                                 ISocketLike *master)
    : name(name_), content(content_) {
    attr.master   = master;
    attr.observer = observer;
}

FilePoster::FileEntry::FileEntry(const byte_string &name_,
                                 const light_string &content_,
                                 const MultiPart::CH::ContentType &content_type_,
                                 const MultiPart::CH::ContentDisposition &content_disposition_,
                                 IObserver *observer,
                                 ISocketLike *master)
    : name(name_), content(content_), content_type(content_type_), content_disposition(content_disposition_) {
    attr.master   = master;
    attr.observer = observer;
}

FilePoster::FileEntry::~FileEntry() {
    attr.close_fd();
    DXOUT("left.");
}

void FilePoster::FileEntry::leave() {
    DXOUT("now leaving...");
    if (status.leaving) {
        DXOUT("now already leaving.");
        return;
    }
    DXOUT("leaving.");
    status.leaving = true;
    VOUT(attr.observer);
    const bool is_under_observation = (attr.observer != NULL && status.originated);
    if (is_under_observation) {
        DXOUT("leaving indirectly.");
        attr.observer->reserve_unhold(this);
    } else {
        // Observerに渡される前に leave されることがある
        DXOUT("leaving immediately.");
        delete this;
    }
}

void FilePoster::FileEntry::start_origination(IObserver &observer) {
    if (status.originated) {
        DXOUT("already orginated!!");
        return;
    }
    prepare_writing();
    attr.observer     = &observer;
    status.originated = true;
}

void FilePoster::FileEntry::notify(IObserver &observer, IObserver::observation_category cat, t_time_epoch_ms epoch) {
    if (status.leaving) {
        return;
    }
    if (attr.master) {
        switch (cat) {
            case IObserver::OT_WRITE:
            case IObserver::OT_EXCEPTION:
            case IObserver::OT_TIMEOUT:
                retransmit(observer, cat, epoch);
                return;
            default:
                break;
        }
    }
    switch (cat) {
        case IObserver::OT_WRITE:
        case IObserver::OT_ORIGINATOR_WRITE:
            perform_sending(observer);
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

void FilePoster::FileEntry::retransmit(IObserver &observer,
                                       IObserver::observation_category cat,
                                       t_time_epoch_ms epoch) {
    assert(attr.master != NULL);
    switch (cat) {
        case IObserver::OT_WRITE:
            cat = IObserver::OT_ORIGINATOR_WRITE;
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

void FilePoster::FileEntry::prepare_writing() {
    attr.fd_      = open(HTTP::restrfy(name).c_str(), O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0444);
    const t_fd fd = attr.fd_;
    DXOUT("opened: " << fd);
    // 0444 なのはアップロードされたファイルを即時実行させないため
    if (fd >= 0) {
        return;
    }
    switch (errno) {
        case EACCES:
            throw http_error("permission denied", HTTP::STATUS_FORBIDDEN);
        case EMFILE:
        case ENFILE:
            throw http_error("exceeding fd limits", HTTP::STATUS_SERVICE_UNAVAILABLE);
        default:
            QVOUT(strerror(errno));
            throw http_error("can't open", HTTP::STATUS_FORBIDDEN);
    }
}

void FilePoster::FileEntry::perform_sending(IObserver &observer) {
    // DXOUT("FP on Write");
    const light_string::size_type file_size = content.size();

    light_string::size_type write_max = 8192;
    if (write_max > file_size - status.written_size) {
        write_max = file_size - status.written_size;
    }
    assert(attr.fd_ >= 0);
    const HTTP::byte_string &base = content.get_base();
    void *b                       = (void *)(&(base.front()) + content.get_first());
    const ssize_t written_size    = write(attr.fd_, (char *)b + status.written_size, write_max);
    if (written_size < 0) {
        QVOUT(strerror(errno));
        throw http_error("write error", HTTP::STATUS_FORBIDDEN);
    }
    const bool is_zero_sent = (written_size == 0);
    if (is_zero_sent) {
        DXOUT("Imcomplete?");
    }
    status.written_size += written_size;
    const bool is_over = (status.written_size >= content.size());
    if (!is_over && !is_zero_sent) {
        return;
    }
    DXOUT("is over!!");
    status.is_over = true;
    observer.reserve_unset(this, IObserver::OT_WRITE);
    // ここで attr.close_fd() を呼びたくなるところだが, そうすると IObserver が困ってしまうので我慢
}

bool FilePoster::FileEntry::is_over() const throw() {
    return status.is_over;
}

// [FilePoster InnerClasses]

FilePoster::Attribute::Attribute() : observer(NULL), master(NULL) {}

FilePoster::Status::Status() : originated(false), leaving(false), is_responsive(false), running_entry(NULL) {}

// [FilePoster]

FilePoster::FilePoster(const RequestMatchingResult &match_result, const IContentProvider &request)
    : directory_path_(HTTP::restrfy(match_result.path_local))
    , request_path(match_result.target->path)
    , content_provider(request) {
    // リクエストの Content-Type: が "multipart/form-data" でかつ正しい boundary パラメータがあれば,
    // マルチパートとみなして処理する.
    const HTTP::CH::ContentType &ct = content_provider.get_content_type_item();
    if (!ct.boundary.is_null()) {
        boundary = ct.boundary.value();
    }
}

FilePoster::~FilePoster() {
    clear_entries();
    if (status.running_entry != NULL) {
        status.running_entry->leave();
    }
}

void FilePoster::notify(IObserver &observer, IObserver::observation_category cat, t_time_epoch_ms epoch) {
    if (status.leaving) {
        return;
    }
    if (!status.running_entry) {
        return;
    }
    switch (cat) {
        case IObserver::OT_WRITE:
        case IObserver::OT_ORIGINATOR_WRITE:
            status.running_entry->notify(observer, cat, epoch);
            shift_entry_if_needed();
            return;
        case IObserver::OT_TIMEOUT:
        case IObserver::OT_ORIGINATOR_TIMEOUT:
            status.running_entry->notify(observer, cat, epoch);
            return;
        default:
            break;
    }
    DXOUT("unexpected notification: " << cat);
    assert(false);
}

void FilePoster::clear_entries() {
    while (entries.size() > 0) {
        entries.front()->leave();
        entries.pop_front();
    }
}

void FilePoster::prepare_posting() {
    // ターゲットがディレクトリであることを確認
    check_target_directory();
    // ボディの情報をメンバに保持しておく.
    body = content_provider.generate_body_data();
    // ボディから FileEntry のリストを生成
    extract_file_entries();
    VOUT(entries.size());
    if (entries.size() == 0) {
        throw http_error("no file data", HTTP::STATUS_BAD_REQUEST);
    }
    // 先頭の FileEntry をキックする
    shift_entry_if_needed();
}

void FilePoster::shift_entry_if_needed() {
    if (status.is_responsive) {
        return;
    }
    FileEntry *f = status.running_entry;
    if (f != NULL) {
        if (!f->is_over()) {
            return;
        }
        QVOUT(f->name);
        attr.observer->reserve_unhold(f);
        status.running_entry = NULL;
    }
    VOUT(entries.size());
    if (entries.size() > 0) {
        // 未完の entry が残っている場合, 先頭の entry を取り出して処理を開始するc
        FileEntry *nextf = entries.front();
        QVOUT(nextf->name);
        nextf->start_origination(*attr.observer);
        attr.observer->reserve_hold(nextf);
        status.running_entry = nextf;
        entries.pop_front();
        attr.observer->reserve_set(nextf, IObserver::OT_WRITE);
    } else {
        // 全 entry が処理完了している場合, レスポンス可能にする
        status.is_responsive = true;
        response_data.inject("", 0, true);
    }
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
    clear_entries();
    if (boundary.size() > 0) {
        decompose_multipart_into_entries(body, boundary);
    } else {
        byte_string file_name = new_file_name(content_provider.get_content_disposition_item(), 1);
        entries.push_back(new FileEntry(file_name, body, attr.observer, attr.master));
    }
    // ファイル名にディレクトリパスを結合
    for (size_t i = 0; i < entries.size(); ++i) {
        entries[i]->name = HTTP::Utils::join_path(HTTP::strfy(directory_path_), entries[i]->name);
    }
}

void FilePoster::decompose_multipart_into_entries(const light_string &body, const light_string &boundary) {
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
                        FileEntry *ent = subpart_to_entry(subpart);
                        if (ent != NULL) {
                            entries.push_back(ent);
                        }
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

FilePoster::FileEntry *FilePoster::subpart_to_entry(const light_string &subpart) {
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
    // Content-Disposition: が "form-data" でない場合は無視する
    if (content_disposition.value != HTTP::strfy("form-data")) {
        return NULL;
    }
    // ヘッダ部の解析結果をもとにファイル名を決定
    byte_string file_name = new_file_name(content_disposition, entries.size() + 1);
    // FileEntryを生成
    const light_string body_part = subpart.substr(i + crlfcrlf.size());
    return new FileEntry(file_name, body_part, content_type, content_disposition, attr.observer, attr.master);
}

void FilePoster::inject_socketlike(ISocketLike *socket_like) {
    attr.master = socket_like;
}

bool FilePoster::is_originatable() const {
    return !status.originated && content_provider.is_complete();
}

bool FilePoster::is_origination_started() const {
    return status.originated;
}

bool FilePoster::is_reroutable() const {
    return false;
}

HTTP::byte_string FilePoster::reroute_path() const {
    assert(false);
    return HTTP::byte_string();
}

bool FilePoster::is_responsive() const {
    return status.is_responsive;
}

void FilePoster::start_origination(IObserver &observer) {
    if (status.originated) {
        return;
    }
    attr.observer = &observer;
    prepare_posting();
    status.originated = true;
}

void FilePoster::leave() {
    delete this;
}

ResponseHTTP::header_list_type
FilePoster::determine_response_headers(const IResponseDataConsumer::t_sending_mode sm) const {
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
    headers.push_back(std::make_pair(HeaderHTTP::location, request_path.str()));
    return headers;
}

ResponseHTTP *FilePoster::respond(const RequestHTTP *request, bool should_close) {
    const IResponseDataConsumer::t_sending_mode sm = response_data.determine_sending_mode();
    ResponseHTTP::header_list_type headers         = determine_response_headers(sm);
    ResponseHTTP *res
        = new ResponseHTTP(request->get_http_version(), HTTP::STATUS_CREATED, &headers, &response_data, should_close);
    return res;
}
