#include "cgi.hpp"
#include <cstring>
#include <unistd.h>
#define MAX_SEND_SIZE 1024
#define MAX_RECEIVE_SIZE 1024

// fd`to`をfd`from`と同一視する
int redirect_fd(t_fd from, t_fd to) {
    if (from == to) {
        return 0;
    }
    // TODO: closeのエラーチェック
    close(to);
    return dup2(from, to);
}

CGI::CGI(pid_t pid,
         SocketUNIX *sock,
         byte_string &script_path,
         byte_string &query_string,
         metavar_dict_type &metavar,
         size_type content_length)
    : cgi_pid(pid)
    , sock(sock)
    , script_path_(script_path)
    , query_string_(query_string)
    , metavar_(metavar)
    , content_length_(content_length) {
    memset(&status, 0, sizeof(Status));
}

CGI::~CGI() {
    delete sock;
    // TODO: ちゃんとトラップする
    waitpid(cgi_pid, NULL, 0);
}

CGI *CGI::start_cgi(byte_string &script_path,
                    byte_string &query_string,
                    metavar_dict_type &metavar,
                    size_type content_length) {

    // TODO: CGIが起動できるかどうかチェックする

    std::pair<SocketUNIX *, t_fd> socks = SocketUNIX::socket_pair();

    pid_t pid = fork();
    VOUT(pid);
    if (pid < 0) {
        throw std::runtime_error("failed to fork");
    }
    if (pid == 0) {
        // child: CGI process
        delete socks.first;
        // 引数の準備
        char **argv = flatten_argv(script_path);
        if (argv == NULL) {
            exit(1);
        }
        char **mvs = flatten_metavar(metavar);
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
        // if (redirect_fd(socks.second, STDERR_FILENO) < 0) {
        //     exit(1);
        // }
        // 起動
        errno  = 0;
        int rv = execve(HTTP::restrfy(script_path).c_str(), argv, mvs);
        VOUT(rv);
        VOUT(errno);
        QVOUT(strerror(errno));
        exit(0);
    }
    // parent: server process
    close(socks.second);
    CGI *cgi = new CGI(pid, socks.first, script_path, query_string, metavar, content_length);
    return cgi;
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
    size_t n     = metavar.size();
    char **frame = (char **)malloc(sizeof(char *) * (n + 1));
    if (frame == NULL) {
        return frame;
    }
    size_t i = 0;
    for (metavar_dict_type::const_iterator it = metavar.begin(); it != metavar.end(); ++it, ++i) {
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

void CGI::set_content(byte_string &content) {
    content_request_    = content;
    status.content_sent = 0;
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
            metavars.insert(std::pair<byte_string, byte_string>(key_part.str(), val_part.str()));
            ++envp;
        }
    }
    return metavars;
}

void CGI::send_content() {
    if (content_request_.size() <= status.content_sent) {
        return;
    }
    size_type send_size = content_request_.size() - status.content_sent;
    if (send_size > MAX_SEND_SIZE) {
        send_size = MAX_SEND_SIZE;
    }
    VOUT(send_size);
    errno             = 0;
    ssize_t sent_size = ::send(get_fd(), (&content_request_.front()) + status.content_sent, send_size, 0);
    VOUT(sent_size);
    if (errno != 0) {
        QVOUT(strerror(errno));
    }
    if (sent_size > 0) {
        status.content_sent += sent_size;
    }
}

void CGI::receive() {
    unsigned char buf[MAX_RECEIVE_SIZE];
    errno                 = 0;
    ssize_t received_size = ::recv(get_fd(), buf, MAX_RECEIVE_SIZE, 0);
    VOUT(received_size);
    if (errno != 0) {
        QVOUT(strerror(errno));
    }
    if (received_size > 0) {
        QVOUT(byte_string(buf, buf + received_size));
    }
}

t_fd CGI::get_fd() const {
    return sock ? sock->get_fd() : -1;
}

void CGI::notify(IObserver &observer) {
    (void)observer;
}

void CGI::timeout(IObserver &observer, t_time_epoch_ms epoch) {
    (void)observer;
    (void)epoch;
}
