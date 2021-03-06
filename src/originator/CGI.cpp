#include "CGI.hpp"
#include "../communication/RoundTrip.hpp"
#include <cstring>
#include <signal.h>
#include <sys/stat.h>
#include <unistd.h>
#define MAX_SEND_SIZE 1024
#define MAX_RECEIVE_SIZE 1024

extern char **environ;

// fd`to`をfd`from`と同一視する
int redirect_fd(t_fd from, t_fd to) {
    if (from == to) {
        return 0;
    }
    // TODO: closeのエラーチェック
    close(to);
    return dup2(from, to);
}

CGI::ParserStatus::ParserStatus() : parse_progress(PP_HEADER_SECTION_END), start_of_header(0), is_freezed(false) {}

CGI::Status::Status() : is_started(false), to_script_content_sent_(0), is_responsive(false), is_complete(false) {}

CGI::Attribute::Attribute(const CGI::byte_string &script_path, const CGI::byte_string &query_string)
    : script_path_(script_path), query_string_(query_string), observer(NULL), master(NULL), cgi_pid(0), sock(NULL) {}

const CGI::byte_string CGI::META_GATEWAY_INTERFACE = HTTP::strfy("GATEWAY_INTERFACE");
const CGI::byte_string CGI::META_REQUEST_METHOD    = HTTP::strfy("REQUEST_METHOD");
const CGI::byte_string CGI::META_SERVER_PROTOCOL   = HTTP::strfy("SERVER_PROTOCOL");
const CGI::byte_string CGI::META_CONTENT_TYPE      = HTTP::strfy("CONTENT_TYPE");
const CGI::byte_string CGI::META_SERVER_PORT       = HTTP::strfy("SERVER_PORT");
const CGI::byte_string CGI::META_CONTENT_LENGTH    = HTTP::strfy("CONTENT_LENGTH");

CGI::CGI(const byte_string &script_path, const byte_string &query_string, const RequestHTTP &request)
    : attr(Attribute(script_path, query_string))
    , metavar_(request.get_cgi_http_vars())
    , to_script_content_length_(0)
    , mid(0) {
    ps.start_of_header               = 0;
    metavar_[META_GATEWAY_INTERFACE] = HTTP::strfy("CGI/1.1");
    metavar_[META_REQUEST_METHOD]    = HTTP::method_str(request.get_method());
    metavar_[META_SERVER_PROTOCOL]   = HTTP::version_str(request.get_http_version());
    metavar_[META_CONTENT_TYPE]      = request.get_content_type();
    set_content(request.get_plain_message());
}

CGI::~CGI() {
    if (attr.cgi_pid != 0) {
        // TODO: 条件付き?
        ::kill(attr.cgi_pid, SIGKILL);
        int wstatus;
        pid_t pid = waitpid(attr.cgi_pid, &wstatus, 0);
        VOUT(pid);
        assert(pid > 0);
        VOUT(WIFEXITED(wstatus));
        VOUT(WEXITSTATUS(wstatus));
        VOUT(WIFSIGNALED(wstatus));
        VOUT(WTERMSIG(wstatus));
        attr.cgi_pid = 0;
    }
    delete attr.sock;
    DXOUT("DESTROYED: " << this);
}

void CGI::inject_socketlike(ISocketLike *socket_like) {
    VOUT(socket_like);
    attr.master                = socket_like;
    metavar_[META_SERVER_PORT] = ParserHelper::utos(attr.master->get_port(), 10);
}

void CGI::check_executable() const {
    // 存在 -> stat のエラー
    // 通常ファイルか -> stat S_ISREF
    // 実行権限 -> stat
    struct stat st;
    errno            = 0;
    const int result = stat(HTTP::restrfy(attr.script_path_).c_str(), &st);
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
    const bool is_regular_file = S_ISREG(st.st_mode);
    if (!is_regular_file) {
        throw http_error("is not normal file", HTTP::STATUS_FORBIDDEN);
    }
    const bool is_executable = (st.st_mode & S_IXUSR);
    if (!is_executable) {
        throw http_error("is not executable", HTTP::STATUS_FORBIDDEN);
    }
}

void CGI::start_origination(IObserver &observer) {
    check_executable();

    std::pair<SocketUNIX *, t_fd> socks = SocketUNIX::socket_pair();
    socks.first->set_nonblock();

    pid_t pid = fork();
    VOUT(pid);
    if (pid < 0) {
        // 500出しとく
        // この時点ではUNIXソケットはholdされていないので, ここで消して良い
        delete socks.first;
        close(socks.second);
        throw http_error("failed to fork", HTTP::STATUS_INTERNAL_SERVER_ERROR);
    }
    if (pid == 0) {
        // child: CGI process
        delete socks.first;
        // 引数の準備
        char **argv = flatten_argv(attr.script_path_);
        if (argv == NULL) {
            exit(1);
        }
        char **mvs = flatten_metavar(metavar_);
        if (mvs == NULL) {
            exit(1);
        }
        // 子プロセス側ソケットに標準入出力をマップ
        if (redirect_fd(socks.second, STDIN_FILENO) < 0) {
            exit(1);
        }
        if (redirect_fd(socks.second, STDOUT_FILENO) < 0) {
            exit(1);
        }
        // TODO: CGIのstderrをどこに向けるか
        // if (redirect_fd(socks.second, STDERR_FILENO) < 0) {
        //     exit(1);
        // }

        // 起動
        errno  = 0;
        int rv = execve(HTTP::restrfy(attr.script_path_).c_str(), argv, mvs);
        VOUT(rv);
        VOUT(errno);
        QVOUT(strerror(errno));
        exit(rv);
    }
    // parent: server process
    attr.cgi_pid = pid;
    attr.sock    = socks.first;
    close(socks.second);
    attr.observer = &observer;
    DXOUT("START OBSERVATION");
    observer.reserve_hold(this);
    DXOUT("< START OBSERVATION");
    observer.reserve_set(this, IObserver::OT_READ);
    observer.reserve_set(this, IObserver::OT_WRITE);
    status.is_started = true;
}

void CGI::capture_script_termination() {
    if (attr.cgi_pid == 0) {
        return;
    }
    int wstatus;
    const pid_t rv = waitpid(attr.cgi_pid, &wstatus, WNOHANG);
    if (rv < 0) {
        VOUT(strerror(errno));
    } else if (rv > 0) {
        attr.cgi_pid = 0;
        // 終了している
        if (WIFEXITED(wstatus)) {
            int cstatus = WEXITSTATUS(wstatus);
            VOUT(cstatus);
            if (cstatus != 0) {
                throw http_error("CGI script finished with error", HTTP::STATUS_INTERNAL_SERVER_ERROR);
            }
        } else if (WIFSIGNALED(wstatus)) {
            int signal = WTERMSIG(wstatus);
            VOUT(signal);
            // throw http_error("CGI script finished by signal", HTTP::STATUS_INTERNAL_SERVER_ERROR);
        }
    }
}

CGI::byte_string CGI::draw_data() const {
    return from_script_content_;
}

char **CGI::flatten_argv(const byte_string &script_path) {
    size_t n     = 2;
    char **frame = (char **)malloc(sizeof(char *) * n);
    if (frame == NULL) {
        return frame;
    }
    frame[0] = strdup(HTTP::restrfy(script_path).c_str());
    if (frame[0] == NULL) {
        free(frame);
        return NULL;
    }
    frame[1] = NULL;
    return frame;
}

char **CGI::flatten_metavar(const metavar_dict_type &metavar) {
    metavar_dict_type prodvar;
    for (char **e = environ; *e; ++e) {
        const char *item = *e;
        HTTP::char_string str(item);
        HTTP::char_string::size_type eq = str.find_first_of('=');
        if (eq == HTTP::char_string::npos) {
            // something wrong
            HTTP::byte_string key = HTTP::strfy(item);
            prodvar[key]          = HTTP::strfy("");
            continue;
        }
        HTTP::byte_string key = HTTP::strfy(str.substr(0, eq));
        HTTP::byte_string val = HTTP::strfy(str.substr(eq + 1));
        prodvar[key]          = val;
    }
    for (metavar_dict_type::const_iterator it = metavar.begin(); it != metavar.end(); ++it) {
        prodvar[it->first] = it->second;
    }
    size_t n     = prodvar.size();
    char **frame = (char **)malloc(sizeof(char *) * (n + 1));
    if (frame == NULL) {
        return frame;
    }
    size_t i = 0;
    for (metavar_dict_type::const_iterator it = prodvar.begin(); it != prodvar.end(); ++it, ++i) {
        size_t j   = it->first.size() + 1 + it->second.size();
        char *item = (char *)malloc(sizeof(char) * (j + 1));
        if (item == NULL) {
            for (size_t k = 0; k < i; ++k) {
                free(frame[k]);
            }
            free(frame);
            return NULL;
        }
        memcpy(item, &(it->first.front()), it->first.size());
        item[it->first.size()] = '=';
        memcpy(item + it->first.size() + 1, &(it->second.front()), it->second.size());
        frame[i]    = item;
        frame[i][j] = '\0';
        VOUT(frame[i]);
    }
    frame[n] = NULL;
    return frame;
}

IResponseDataProducer &CGI::response_data_producer() {
    return status.response_data;
}

void CGI::set_content(const byte_string &content) {
    to_script_content_        = content;
    to_script_content_length_ = content.size();
    VOUT(to_script_content_length_);
    BVOUT(to_script_content_);
}

CGI::metavar_dict_type CGI::make_metavars_from_envp(char **envp) {
    metavar_dict_type metavars;
    if (envp) {
        while (*envp) {
            const HTTP::byte_string var(*envp, *envp + strlen(*envp));
            HTTP::light_string lvar(var);
            HTTP::light_string key_part = lvar.substr_before("=");
            HTTP::light_string val_part;
            if (key_part.length() < lvar.length()) {
                val_part = lvar.substr(key_part.length() + 1);
            }

            metavars.insert(metavar_dict_type::value_type(key_part.str(), val_part.str()));
            ++envp;
        }
    }
    return metavars;
}

t_fd CGI::get_fd() const {
    return attr.sock ? attr.sock->get_fd() : -1;
}

t_port CGI::get_port() const {
    return attr.master && attr.master != this ? attr.master->get_port() : 0;
}

void CGI::notify(IObserver &observer, IObserver::observation_category cat, t_time_epoch_ms epoch) {
    DXOUT("CGI received: " << cat);
    if (attr.master) {
        switch (cat) {
            case IObserver::OT_WRITE:
            case IObserver::OT_READ:
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
            // CGIスクリプトへのデータ送信
            perform_sending(observer);
            return;
        case IObserver::OT_READ:
        case IObserver::OT_ORIGINATOR_READ:
            perform_receiving(observer);
            return;
        case IObserver::OT_TIMEOUT:
        case IObserver::OT_ORIGINATOR_TIMEOUT:
            VOUT(epoch);
            return;
        default:
            DXOUT("unexpected cat: " << cat);
            assert(false);
    }
}

void CGI::retransmit(IObserver &observer, IObserver::observation_category cat, t_time_epoch_ms epoch) {
    assert(attr.master != NULL);
    switch (cat) {
        case IObserver::OT_READ:
            cat = IObserver::OT_ORIGINATOR_READ;
            break;
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

void CGI::perform_sending(IObserver &observer) {
    DXOUT("CGI on Write");
    ssize_t sent_size = 0;
    if (status.to_script_content_sent_ < to_script_content_length_) {
        ssize_t rest_to_script = to_script_content_length_ - status.to_script_content_sent_;
        const char *head       = &to_script_content_.front() + status.to_script_content_sent_;
        sent_size              = attr.sock->send(head, rest_to_script, 0);
        VOUT(sent_size);
        if (sent_size < 0) {
            throw http_error("failed to send data to CGI script", HTTP::STATUS_INTERNAL_SERVER_ERROR);
        }
        status.to_script_content_sent_ += sent_size;
    }
    if (sent_size > 0 && status.to_script_content_sent_ < to_script_content_length_) {
        return;
    }
    observer.reserve_unset(this, IObserver::OT_WRITE);
    attr.sock->shutdown_write();
}

void CGI::perform_receiving(IObserver &observer) {
    DXOUT("CGI on Read");
    char buf[MAX_RECEIVE_SIZE];
    const ssize_t received_size = attr.sock->receive(buf, MAX_RECEIVE_SIZE, 0);
    VOUT(received_size);
    if (received_size == 0) {
        capture_script_termination();
    }
    if (received_size < 0) {
        throw http_error("failed to receive data from CGI script", HTTP::STATUS_INTERNAL_SERVER_ERROR);
    }
    const bool is_disconnected = (received_size == 0);
    if (this->ps.parse_progress < PP_BODY) {
        inject_bytestring(buf, buf + received_size);
    } else {
        response_data_producer().inject(buf, received_size, is_disconnected);
    }
    if (is_disconnected) {
        // Read側の切断を検知
        observer.reserve_unset(this, IObserver::OT_WRITE);
        observer.reserve_unset(this, IObserver::OT_READ);
    }
    after_injection(is_disconnected);
}

bool CGI::is_originatable() const {
    return !status.is_started;
}

bool CGI::is_origination_started() const {
    return status.is_started;
}

bool CGI::is_reroutable() const {
    // オリジネーションが完了しており, CGIレスポンスタイプがローカルリダイレクトである
    return status.is_complete && rp.get_response_type() == CGIRES_REDIRECT_LOCAL;
}

bool CGI::is_responsive() const {
    // レスポンス可能であり, CGIレスポンスタイプが確定していてローカルリダイレクトでない
    return status.is_responsive && rp.get_response_type() != CGIRES_UNKNOWN
           && rp.get_response_type() != CGIRES_REDIRECT_LOCAL;
}

void CGI::leave() {
    DXOUT("leaving.");
    if (attr.observer != NULL) {
        attr.observer->reserve_unhold(this);
    } else {
        // Observerに渡される前に leave されることがある
        delete this;
    }
}

CGI::t_parse_progress CGI::reach_headers_end(size_t len, bool is_disconnected) {
    // ヘッダ部の終わりを探索する
    // `crlf_in_header` には「最後に見つけたCRLF」のレンジが入っている.
    // mid以降についてCRLFを検索する:
    // - 見つかった
    //   - first == crlf_in_header.second である
    //     -> あたり. このCRLFがヘッダの終わりを示す.
    //   - そうでない
    //     -> はずれ. このCRLFを crlf_in_header として続行.
    // - 見つからなかった
    //   -> もう一度受信する
    IndexRange res = ParserHelper::find_crlf(bytebuffer, this->mid, len);
    this->mid      = res.second;
    if (!is_disconnected && res.is_invalid()) {
        // はずれ: CRLFが見つからなかった
        return PP_UNREACHED;
    }
    if (!is_disconnected && this->ps.crlf_in_header.second != res.first) {
        // はずれ: CRLFとCRLFの間が空いていた
        this->ps.crlf_in_header = res;
        return PP_HEADER_SECTION_END;
    }

    // あたり: ヘッダの終わりが見つかった
    analyze_headers(res);

    // bytebuffer のあまった部分(本文と思われる部分)をProducerに注入
    const size_t rest_size = bytebuffer.size() - this->mid;
    if (rest_size > 0 || is_disconnected) {
        const char *rest_head = &(bytebuffer.front()) + this->mid;
        response_data_producer().inject(rest_head, rest_size, is_disconnected);
    }
    return PP_BODY;
}

void CGI::analyze_headers(IndexRange res) {
    this->ps.end_of_header = res.first;
    this->ps.start_of_body = res.second;
    DXOUT("DETECTED END of HEADER: " << this->ps.end_of_header);
    // -> [start_of_header, end_of_header) を解析する
    VOUT(this->ps.start_of_header);
    const light_string header_lines(bytebuffer, this->ps.start_of_header, this->ps.end_of_header);
    BVOUT(header_lines);
    parse_header_lines(header_lines, &this->from_script_header_holder);
    extract_control_headers();
}

void CGI::after_injection(bool is_disconnected) {
    size_t len;
    t_parse_progress flow;
    VOUT(is_disconnected);
    do {
        len = bytebuffer.size() - this->mid;
        VOUT(len);
        switch (this->ps.parse_progress) {
            case PP_HEADER_SECTION_END: {
                DXOUT("CGI PP_HEADER_SECTION_END");
                flow = reach_headers_end(len, is_disconnected);
                if (flow == PP_UNREACHED) {
                    return;
                }
                // ローカルリダイレクトでなければこの時点で送信可能
                // ※「chunked が使用可能」という条件が本来は必要.
                // なぜなら chunked は最初のtransfer-encodingでなければならないからだが,
                // ここではオリジネーションをしているので必然的に最後になる.
                if (rp.get_response_type() != CGIRES_REDIRECT_LOCAL) {
                    status.is_responsive = true;
                }
                this->ps.parse_progress = flow;
                continue;
            }

            case PP_BODY: {
                DXOUT("CGI PP_BODY");
                flow = reach_fixed_body_end(len, is_disconnected);
                if (flow == PP_UNREACHED) {
                    return;
                }
                DXOUT("over?");
                this->ps.parse_progress = flow;
                VOUT(this->ps.parse_progress);
                continue;
            }

            case PP_OVER: {
                DXOUT("CGI PP_OVER");
                check_cgi_response_consistensy();
                // ローカルリダイレクトならこの時点で送信可能
                if (rp.get_response_type() == CGIRES_REDIRECT_LOCAL) {
                    status.is_responsive = true;
                }
                status.is_complete = true;
                VOUT(ps.start_of_body);
                VOUT(ps.end_of_body);
                BVOUT(bytebuffer);
                return;
            }

            default:
                throw http_error("not implemented yet", HTTP::STATUS_NOT_IMPLEMENTED);
        }
    } while (len > 0 || this->ps.parse_progress == PP_OVER);
}

void CGI::extract_control_headers() {
    // 取得したヘッダから制御用の情報を抽出する.
    this->rp.content_type.determine(from_script_header_holder);
    this->rp.status.determine(from_script_header_holder);
    this->rp.location.determine(from_script_header_holder);
    this->rp.determine_body_size(from_script_header_holder);

    VOUT(rp.get_response_type());
}

void CGI::check_cgi_response_consistensy() {
    switch (rp.get_response_type()) {
        case CGIRES_DOCUMENT: {
            // 特に何もチェックしなくて良さそう
            break;
        }
        case CGIRES_REDIRECT_LOCAL: {
            // CGIヘッダが2つ以上ある
            const AHeaderHolder::list_type &head_list = from_script_header_holder.get_list();
            if (head_list.size() != 1) {
                throw http_error("local-redirection doesn't have only one cgi header",
                                 HTTP::STATUS_INTERNAL_SERVER_ERROR);
            }
            // 唯一のCGIヘッダが Location: でない
            if (head_list.front().get_key() != HeaderHTTP::location) {
                throw http_error("local-redirection has a header that's not Location:",
                                 HTTP::STATUS_INTERNAL_SERVER_ERROR);
            }
            // 本文がある
            if (rp.body_size > 0) {
                throw http_error("local-redirection has effective body:", HTTP::STATUS_INTERNAL_SERVER_ERROR);
            }
            break;
        }
        case CGIRES_REDIRECT_CLIENT: {
            // Location: がない
            if (from_script_header_holder.get_val(HeaderHTTP::location) == NULL) {
                throw http_error("client-redirection has no location:", HTTP::STATUS_INTERNAL_SERVER_ERROR);
            }
            // Content-Type: または Status: がある
            if (from_script_header_holder.get_val(HeaderHTTP::content_type) != NULL) {
                throw http_error("client-redirection has content-type:", HTTP::STATUS_INTERNAL_SERVER_ERROR);
            }
            if (from_script_header_holder.get_val(HeaderHTTP::status) != NULL) {
                throw http_error("client-redirection has status:", HTTP::STATUS_INTERNAL_SERVER_ERROR);
            }
            // 本文がある
            if (rp.body_size > 0) {
                throw http_error("client-redirection has effective body:", HTTP::STATUS_INTERNAL_SERVER_ERROR);
            }
            break;
        }
        case CGIRES_REDIRECT_CLIENT_DOCUMENT: {
            // Location: がない
            if (from_script_header_holder.get_val(HeaderHTTP::location) == NULL) {
                throw http_error("client-redirection has no location:", HTTP::STATUS_INTERNAL_SERVER_ERROR);
            }
            // Content-Type: がない
            if (from_script_header_holder.get_val(HeaderHTTP::content_type) == NULL) {
                throw http_error("client-redirection has no content-type:", HTTP::STATUS_INTERNAL_SERVER_ERROR);
            }
            const HeaderHolderCGI::header_val_type *status = from_script_header_holder.get_val(HeaderHTTP::status);
            if (status == NULL) {
                throw http_error("client-redirection has no status:", HTTP::STATUS_INTERNAL_SERVER_ERROR);
            }
            if (status->size() != 3 || (*status)[0] != '3') {
                throw http_error("client-redirection&s status: is not a valid 3**", HTTP::STATUS_INTERNAL_SERVER_ERROR);
            }
            break;
        }
        default:
            throw http_error("can't determine cgi response type", HTTP::STATUS_INTERNAL_SERVER_ERROR);
    }
}

CGI::t_cgi_response_type CGI::RoutingParameters::get_response_type() const {
    if (content_type.value.size() > 0) {
        if (location.value.size() == 0) {
            return CGIRES_DOCUMENT;
        } else if (status.code > 0 && location.value.size() > 0 && !location.is_local) {
            return CGIRES_REDIRECT_CLIENT_DOCUMENT;
        }
    } else if (location.value.size() > 0) {
        if (location.is_local) {
            return CGIRES_REDIRECT_LOCAL;
        } else {
            return CGIRES_REDIRECT_CLIENT;
        }
    }
    return CGIRES_UNKNOWN;
}

CGI::t_parse_progress CGI::reach_fixed_body_end(size_t len, bool is_disconnected) {
    // https://wiki.suikawiki.org/n/CGI%E5%BF%9C%E7%AD%94$18332#header-section-%E5%87%A6%E7%90%86%E3%83%A2%E3%83%87%E3%83%AB
    // > 鯖は CGIスクリプトが提供したデータをすべて、 EOF に到達するまで読まなければなりません。
    // > それをそのまま無変更で送信するべきです。

    this->mid += len;
    if (!is_disconnected) {
        return PP_UNREACHED;
    }
    this->ps.end_of_body = this->mid;
    this->rp.body_size   = this->ps.end_of_body - this->ps.start_of_body;
    return PP_OVER;
}

void CGI::RoutingParameters::determine_body_size(const header_holder_type &holder) {
    // https://wiki.suikawiki.org/n/CGI%E5%BF%9C%E7%AD%94$18332#header-section-%E5%87%A6%E7%90%86%E3%83%A2%E3%83%87%E3%83%AB
    // > 鯖は CGIスクリプトが提供したデータをすべて、 EOF に到達するまで読まなければなりません。
    // > それをそのまま無変更で送信するべきです。
    //
    // ということで, サイズは事前に決められない.

    // むしろ, CGIヘッダに content-length: がある場合, それを破棄する必要がありそう.
    (void)holder;
}

size_t CGI::parsed_body_size() const {
    return this->mid - this->ps.start_of_body;
}

void CGI::parse_header_lines(const light_string &lines, header_holder_type *holder) const {
    light_string rest(lines);
    while (true) {
        QVOUT(rest);
        const IndexRange res = ParserHelper::find_crlf_header_value(rest);
        if (res.is_invalid()) {
            break;
        }
        const light_string header_line = rest.substr(0, res.first);
        QVOUT(header_line);
        if (header_line.length() > 0) {
            // header_line が空文字列でない
            // -> ヘッダ行としてパースを試みる
            parse_header_line(header_line, holder);
        }
        rest = rest.substr(res.second);
    }
}

void CGI::parse_header_line(const light_string &line, header_holder_type *holder) const {

    const light_string key = line.substr_before(ParserHelper::HEADER_KV_SPLITTER);
    QVOUT(line);
    QVOUT(key);
    if (key.length() == line.length()) {
        // [!] Apache は : が含まれず空白から始まらない行がヘッダー部にあると、 400 応答を返します。 nginx
        // は無視して処理を続けます。
        throw http_error("no coron in a header line", HTTP::STATUS_BAD_REQUEST);
    }
    // ":"があった -> ":"の前後をキーとバリューにする
    if (key.length() == 0) {
        throw http_error("header key is empty", HTTP::STATUS_BAD_REQUEST);
    }
    light_string val = line.substr(key.length() + 1);
    // [!] 欄名と : の間には空白は認められていません。 鯖は、空白がある場合 400 応答を返して拒絶しなければなりません。
    // 串は、下流に転送する前に空白を削除しなければなりません。
    light_string::size_type key_tail = key.find_last_not_of(ParserHelper::OWS);
    if (key_tail + 1 != key.length()) {
        throw http_error("trailing space on header key", HTTP::STATUS_BAD_REQUEST);
    }
    val = val.trim(ParserHelper::OWS);
    // holder があるなら holder に渡す. あとの処理は holder に任せる.
    if (holder != NULL) {
        holder->add_item(key, val);
    } else {
        DXOUT("no holder");
        QVOUT(key);
        QVOUT(val);
    }
}

ResponseHTTP *CGI::respond(const RequestHTTP &request) {
    // ローカルリダイレクトの場合ここに来てはいけない
    assert(rp.get_response_type() != CGIRES_REDIRECT_LOCAL);

    ResponseHTTP res(request.get_http_version(), HTTP::STATUS_OK, &status.response_data);
    CGI::t_cgi_response_type response_type = rp.get_response_type();
    VOUT(response_type);
    VOUT(rp.status.code);

    switch (response_type) {
        case CGIRES_DOCUMENT:
            // ドキュメント応答
            if (rp.status.code == HTTP::STATUS_UNSPECIFIED) {
                res.set_status(HTTP::STATUS_OK);
            } else {
                res.set_status((HTTP::t_status)rp.status.code);
            }
            break;
        case CGIRES_REDIRECT_CLIENT:
            // クライアントリダイレクト
            res.set_status(HTTP::STATUS_FOUND);
            break;
        case CGIRES_REDIRECT_CLIENT_DOCUMENT:
            // クライアントリダイレクト ドキュメント付き
            res.set_status((HTTP::t_status)rp.status.code);
            break;
        default:
            assert(false);
    }

    // 本文がある場合は Transfer-Encoding をセット

    // 本文がある場合は Content-Type をセット
    // (なければ何もセットしない)
    if (response_type == CGIRES_DOCUMENT || response_type == CGIRES_REDIRECT_CLIENT_DOCUMENT) {
        if (from_script_content_.size() > 0) {
            const HeaderHolderCGI::header_val_type *val
                = from_script_header_holder.get_back_val(HeaderHTTP::content_type);
            if (val) {
                res.feed_header(HeaderHTTP::content_type, *val);
            }
        }
    }
    res.start();

    // 例外安全のための copy and swap
    ResponseHTTP *r = new ResponseHTTP(request.get_http_version(), HTTP::STATUS_OK, NULL);
    ResponseHTTP::swap(res, *r);
    return r;
}
