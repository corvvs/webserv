#include "CGI.hpp"
#include "../communication/RoundTrip.hpp"
#include "../utils/File.hpp"
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

CGI::Attribute::Attribute(const RequestMatchingResult &matching_result,
                          const ICGIConfigurationProvider &configuration_provider)
    : configuration_provider_(configuration_provider)
    , executor_path_(matching_result.path_cgi_executor)
    , script_path_(matching_result.cgi_resource.fullpath)
    , query_string_(configuration_provider.get_request_matching_param().get_request_target().query.str())
    , observer(NULL)
    , master(NULL)
    , cgi_pid(0)
    , sock(NULL) {}

const CGI::byte_string CGI::META_GATEWAY_INTERFACE = HTTP::strfy("GATEWAY_INTERFACE");
const CGI::byte_string CGI::META_REQUEST_METHOD    = HTTP::strfy("REQUEST_METHOD");
const CGI::byte_string CGI::META_SERVER_PROTOCOL   = HTTP::strfy("SERVER_PROTOCOL");
const CGI::byte_string CGI::META_CONTENT_TYPE      = HTTP::strfy("CONTENT_TYPE");
const CGI::byte_string CGI::META_SERVER_PORT       = HTTP::strfy("SERVER_PORT");
const CGI::byte_string CGI::META_CONTENT_LENGTH    = HTTP::strfy("CONTENT_LENGTH");
const CGI::byte_string CGI::META_PATH_INFO         = HTTP::strfy("PATH_INFO");
const CGI::byte_string CGI::META_SCRIPT_NAME       = HTTP::strfy("SCRIPT_NAME");
const CGI::byte_string CGI::META_QUERY_STRING      = HTTP::strfy("QUERY_STRING");

CGI::CGI(const RequestMatchingResult &match_result, const ICGIConfigurationProvider &request)
    : attr(Attribute(match_result, request))
    , lifetime(Lifetime::make_response())
    , metavar_(request.get_cgi_meta_vars())
    , to_script_content_length_(0)
    , mid(0) {
    ps.start_of_header               = 0;
    metavar_[META_GATEWAY_INTERFACE] = HTTP::strfy("CGI/1.1");
    metavar_[META_REQUEST_METHOD]    = HTTP::method_str(attr.configuration_provider_.get_method());
    metavar_[META_SERVER_PROTOCOL]   = HTTP::version_str(attr.configuration_provider_.get_http_version());
    metavar_[META_CONTENT_TYPE]      = attr.configuration_provider_.get_content_type();
    if (match_result.cgi_resource.script_name == HTTP::strfy("/")) {
        metavar_[META_SCRIPT_NAME] = HTTP::strfy("");
    } else {
        metavar_[META_SCRIPT_NAME] = match_result.cgi_resource.script_name;
    }
    metavar_[META_PATH_INFO]    = match_result.cgi_resource.path_info;
    metavar_[META_QUERY_STRING] = match_result.target->query.str();
}

CGI::~CGI() {
    if (attr.cgi_pid != 0) {
        // TODO: 条件付き?
        ::kill(attr.cgi_pid, SIGKILL);
        int wstatus;
        pid_t pid = waitpid(attr.cgi_pid, &wstatus, 0);
        lifetime.deactivate();
        // VOUT(pid);
        assert(pid > 0);
        // VOUT(WIFEXITED(wstatus));
        // VOUT(WEXITSTATUS(wstatus));
        // VOUT(WIFSIGNALED(wstatus));
        // VOUT(WTERMSIG(wstatus));
        attr.cgi_pid = 0;
    }
    delete attr.sock;
    // DXOUT("DESTROYED: " << this);
}

void CGI::inject_socketlike(ISocketLike *socket_like) {
    attr.master                = socket_like;
    metavar_[META_SERVER_PORT] = ParserHelper::utos(attr.master->get_port(), 10);
}

void CGI::check_executable() const {
    errno = 0;
    if (file::is_executable(HTTP::restrfy(attr.script_path_))) {
        return;
    }
    switch (errno) {
        case ENOENT:
            throw http_error("file not found", HTTP::STATUS_NOT_FOUND);
        case EACCES:
            throw http_error("can't search file", HTTP::STATUS_FORBIDDEN);
        default:
            VOUT(errno);
            throw http_error("something wrong", HTTP::STATUS_INTERNAL_SERVER_ERROR);
    }
}

void CGI::start_origination(IObserver &observer) {
    check_executable();
    set_content(attr.configuration_provider_.get_body());

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
        char **argv = flatten_argv(attr.executor_path_, attr.script_path_);
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
        std::string dir_name = file::get_directory_name(HTTP::restrfy(attr.script_path_));
        if (chdir(dir_name.c_str()) != 0) {
            exit(1);
        }
        // 起動
        errno  = 0;
        int rv = execve(HTTP::restrfy(attr.executor_path_).c_str(), argv, mvs);
        VOUT(rv);
        VOUT(errno);
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
    lifetime.activate();
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
        lifetime.deactivate();
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

char **CGI::flatten_argv(const byte_string &executor_path, const byte_string &script_path) {
    size_t n     = 3;
    char **frame = (char **)malloc(sizeof(char *) * n);
    if (frame == NULL) {
        return frame;
    }
    frame[0] = strdup(HTTP::restrfy(executor_path).c_str());
    frame[1] = strdup(HTTP::restrfy(script_path).c_str());
    if (frame[0] == NULL || frame[1] == NULL) {
        free(frame[0]);
        free(frame[1]);
        free(frame);
        return NULL;
    }
    frame[2] = NULL;
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
        // VOUT(frame[i]);
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
    const byte_string val
        = to_script_content_length_ > 0 ? ParserHelper::utos(to_script_content_length_, 10) : HTTP::strfy("");
    metavar_[META_CONTENT_LENGTH] = val;
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
    // DXOUT("CGI received: " << cat);
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
            if (lifetime.is_timeout(epoch)) {
                DXOUT("TIMEOUT!!");
                throw http_error("cgi-script timed out", HTTP::STATUS_TIMEOUT);
            }
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
    VOUT(status.is_responsive);
    VOUT(rp.get_response_type());
    return status.is_responsive && rp.get_response_type() != CGIRES_UNKNOWN
           && rp.get_response_type() != CGIRES_REDIRECT_LOCAL;
}

HTTP::byte_string CGI::reroute_path() const {
    return rp.location.value;
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

bool CGI::is_timeout(t_time_epoch_ms now) const {
    return lifetime.is_timeout(now);
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
    this->from_script_header_holder.parse_header_lines(header_lines, &this->from_script_header_holder);
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
                VOUT(rp.get_response_type());
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
                // VOUT(ps.start_of_body);
                // VOUT(ps.end_of_body);
                // BVOUT(bytebuffer);
                return;
            }

            default:
                throw http_error("not implemented yet", HTTP::STATUS_NOT_IMPLEMENTED);
        }
    } while (len > 0 || this->ps.parse_progress == PP_OVER);
}

void CGI::extract_control_headers() {
    // 取得したヘッダから制御用の情報を抽出する.
    minor_error me;
    me = erroneous(me, this->rp.content_type.determine(from_script_header_holder));
    me = erroneous(me, this->rp.status.determine(from_script_header_holder));
    me = erroneous(me, this->rp.location.determine(from_script_header_holder));
    me = erroneous(me, this->rp.set_cookie.determine(from_script_header_holder));
    this->rp.determine_body_size(from_script_header_holder);
    if (me.is_error()) {
        throw http_error(me);
    }
    VOUT(rp.get_response_type());
}

void CGI::check_cgi_response_consistensy() {
    VOUT(rp.get_response_type());
    switch (rp.get_response_type()) {
        case CGIRES_DOCUMENT: {
            // 特に何もチェックしなくて良さそう
            break;
        }
        case CGIRES_REDIRECT_LOCAL: {
            // CGIヘッダが2つ以上ある
            DXOUT("LOCAL?");
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
    // QVOUT(content_type.value);
    // QVOUT(location.value);
    // QVOUT(location.is_local);
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

    (void)holder;
}

size_t CGI::parsed_body_size() const {
    return this->mid - this->ps.start_of_body;
}

HTTP::t_status CGI::determine_response_status() const {
    switch (rp.get_response_type()) {
        case CGIRES_DOCUMENT:
            // ドキュメント応答
            if (rp.status.code == HTTP::STATUS_UNSPECIFIED) {
                return HTTP::STATUS_OK;
            } else {
                return (HTTP::t_status)rp.status.code;
            }
        case CGIRES_REDIRECT_CLIENT:
            // クライアントリダイレクト
            return HTTP::STATUS_FOUND;
        case CGIRES_REDIRECT_CLIENT_DOCUMENT:
            // クライアントリダイレクト ドキュメント付き
            return (HTTP::t_status)rp.status.code;
        default:
            assert(false);
    }
    return HTTP::STATUS_DUMMY;
}

ResponseHTTP::header_list_type CGI::determine_response_headers_destructively() {
    ResponseHTTP::header_list_type headers;
    // [伝送に関する決め事]
    // CGIが送ってきた Transfer-Encoding: や Content-Length: を破棄する
    // それはそうとして, chunkedで送るか, そうでないかを決める
    // chunkedでない場合, Content-Length: を与える
    // 本文がある場合は Content-Type: をセット
    // (なければ何もセットしない)
    from_script_header_holder.erase_vals(HeaderHTTP::transfer_encoding);
    from_script_header_holder.erase_vals(HeaderHTTP::content_length);
    IResponseDataConsumer::t_sending_mode sm = status.response_data.determine_sending_mode();
    const t_cgi_response_type response_type  = rp.get_response_type();
    const bool is_redirection
        = response_type == CGIRES_REDIRECT_CLIENT || response_type == CGIRES_REDIRECT_CLIENT_DOCUMENT;
    const bool must_have_body = response_type == CGIRES_DOCUMENT || response_type == CGIRES_REDIRECT_CLIENT_DOCUMENT;
    if (is_redirection) {
        headers.push_back(std::make_pair(HeaderHTTP::location, rp.location.value));
    }
    // Content-Type:
    if (must_have_body) {
        const HeaderHolderCGI::header_val_type *val = from_script_header_holder.get_back_val(HeaderHTTP::content_type);
        if (val) {
            headers.push_back(std::make_pair(HeaderHTTP::content_type, *val));
        } else {
            headers.push_back(std::make_pair(HeaderHTTP::content_type, CGIP::CH::ContentType::default_value));
        }
        // 伝送方式
        switch (sm) {
            case ResponseDataList::SM_CHUNKED:
                headers.push_back(std::make_pair(HeaderHTTP::transfer_encoding, HTTP::strfy("chunked")));
                break;
            case ResponseDataList::SM_NOT_CHUNKED:
                headers.push_back(std::make_pair(HeaderHTTP::content_length,
                                                 ParserHelper::utos(status.response_data.current_total_size(), 10)));
                break;
            default:
                break;
        }
    }
    // その他のヘッダ
    from_script_header_holder.erase_vals(HeaderHTTP::transfer_encoding);
    from_script_header_holder.erase_vals(HeaderHTTP::content_length);
    from_script_header_holder.erase_vals(HeaderHTTP::status);
    from_script_header_holder.erase_vals(HeaderHTTP::location);
    from_script_header_holder.erase_vals(HeaderHTTP::content_type);
    if (response_type != CGIRES_REDIRECT_CLIENT) {
        const AHeaderHolder::list_type &header_list = from_script_header_holder.get_list();
        for (AHeaderHolder::list_type::const_iterator hit = header_list.begin(); hit != header_list.end(); ++hit) {
            const HeaderItem::value_list_type &val_list = hit->get_vals();
            for (HeaderItem::value_list_type::const_iterator vit = val_list.begin(); vit != val_list.end(); ++vit) {
                headers.push_back(std::make_pair(hit->get_key(), *vit));
            }
        }
    }
    return headers;
}

ResponseHTTP *CGI::respond(const RequestHTTP *request) {
    // ローカルリダイレクトの場合ここに来てはいけない
    assert(rp.get_response_type() != CGIRES_REDIRECT_LOCAL);

    // 応答ステータスを決める
    HTTP::t_status response_status = determine_response_status();

    // HTTPレスポンスヘッダを生成する
    ResponseHTTP::header_list_type headers = determine_response_headers_destructively();
    ResponseHTTP res(request->get_http_version(), response_status, &headers, &status.response_data, false);

    // 例外安全のための copy and swap
    ResponseHTTP *r = new ResponseHTTP(request->get_http_version(), HTTP::STATUS_OK, NULL, NULL, false);
    ResponseHTTP::swap(res, *r);
    r->start();
    return r;
}
