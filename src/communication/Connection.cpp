#include "Connection.hpp"
#include "Channel.hpp"
#include "RoundTrip.hpp"
#include <cassert>
#define MAX_REQLINE_END 8192

// [[Attribute]]

Connection::Attribute::Attribute() {
    http_version  = HTTP::DEFAULT_HTTP_VERSION;
    is_persistent = true;
    timeout       = 60 * 1000;
}

// [[NetwotkBuffer]]

Connection::NetworkBuffer::NetworkBuffer() : read_size(0) {
    read_buffer.resize(MAX_REQLINE_END);
}

ssize_t Connection::NetworkBuffer::receive(SocketConnected &sock) {
    if (read_size > 0) {
        // read_buffer にデータがあるならそれを待避させる
        extra_data_buffer.push_back(read_buffer);
        read_size = 0;
        read_buffer.resize(read_size);
        return extra_data_buffer.front().size();
    }

    read_buffer.resize(MAX_REQLINE_END);
    char *rb  = &read_buffer.front();
    read_size = sock.receive(rb, MAX_REQLINE_END, 0);
    if (read_size >= 0) {
        read_buffer.resize(read_size);
    }
    if (extra_data_buffer.size() > 0) {
        return extra_data_buffer.front().size();
    }
    return read_size;
}

const Connection::byte_string &Connection::NetworkBuffer::top_front() const {
    if (extra_data_buffer.size() > 0) {
        return extra_data_buffer.front();
    }
    return read_buffer;
}

void Connection::NetworkBuffer::pop_front() {
    if (extra_data_buffer.size() > 0) {
        extra_data_buffer.pop_front();
    } else {
        read_buffer.resize(0);
        read_size = 0;
    }
}

void Connection::NetworkBuffer::push_front(const light_string &data) {
    if (data.size() > 0) {
        extra_data_buffer.push_front(data.str());
    }
}

bool Connection::NetworkBuffer::should_redo() const {
    return extra_data_buffer.size() > 0 || read_size > 0;
}

// [[Connection]]

Connection::Connection(IRouter *router, SocketConnected *sock_given, const config::config_vector &configs)
    : attr(Attribute())
    , phase(CONNECTION_ESTABLISHED)
    , dying(false)
    , sock(sock_given)
    , rt(*router, configs)
    , lifetime(Lifetime::make_connection()) {
    DXOUT("[established] " << sock->get_fd());
}

Connection::~Connection() {
    delete sock;
    DXOUT("DESTROYED: " << this);
}

t_fd Connection::get_fd() const {
    return sock->get_fd();
}

t_port Connection::get_port() const {
    return sock->get_port();
}

void Connection::notify(IObserver &observer, IObserver::observation_category cat, t_time_epoch_ms epoch) {
    try {
        do {
            if (dying) {
                return;
            }
            switch (phase) {
                case CONNECTION_ESTABLISHED:
                    perform_reaction(observer, cat, epoch);
                    if (cat == IObserver::OT_TIMEOUT) {
                        return;
                    }
                    detect_update(observer);
                    if (!rt.is_freezed() && net_buffer.should_redo()) {
                        // - リクエストが凍結されていない
                        // - NetworkBuffer に余ったデータがある
                        // なら, イベントハンドラをもう一度やり直す.
                        // ただし, イベントを Read に切り替える.
                        // (HTTPパイプライン)
                        DXOUT("REDO");
                        cat = IObserver::OT_READ;
                        continue;
                    }
                    break;
                case CONNECTION_SHUTTING_DOWN: // コネクション 切断中
                    perform_shutting_down(observer);
                    break;
                default:
                    DXOUT("unexpected phase: " << phase);
                    assert(false);
            }
            break;
        } while (1);
    } catch (const http_error &err) { // 受信中のHTTPエラー
        DXOUT("error occurred");
        if (phase == CONNECTION_SHUTTING_DOWN || rt.is_responding()) {
            // レスポンス送信中のHTTPエラー -> 全てを諦めて終了
            shutdown_gracefully(observer);
        } else {
            // レスポンス送信前のHTTPエラー -> エラーレスポンス送信開始
            rt.respond_error(err);
            observer.reserve_set(this, IObserver::OT_WRITE);
        }
    }
}

void Connection::perform_reaction(IObserver &observer, IObserver::observation_category cat, t_time_epoch_ms epoch) {
    switch (cat) {
        case IObserver::OT_TIMEOUT:
            if (lifetime.is_timeout(epoch) || rt.is_timeout(epoch)) {
                throw http_error("connection timed out", HTTP::STATUS_TIMEOUT);
            }
            break;
        // オリジネータへの通知
        case IObserver::OT_ORIGINATOR_WRITE:
        case IObserver::OT_ORIGINATOR_READ:
        case IObserver::OT_ORIGINATOR_EXCEPTION:
        case IObserver::OT_ORIGINATOR_TIMEOUT:
            rt.notify_originator(observer, cat, epoch);
            break;

        case IObserver::OT_READ:
            perform_receiving(observer);
            break;

        case IObserver::OT_WRITE:
            perform_sending(observer);
            break;

        default:
            DXOUT("unexpected cat: " << cat);
            throw std::runtime_error("????");
    }
}

void Connection::perform_receiving(IObserver &observer) {
    // データ受信
    const ssize_t received_size = net_buffer.receive(*sock);
    if (received_size <= 0) {
        // なにも受信できなかったか, 受信エラーが起きた場合
        die(observer);
        return;
    }

    // データ注入
    // インバウンド許容
    // -> 受信したデータをリクエストに注入する
    if (!rt.is_freezed()) {
        rt.start_if_needed();
        lifetime.activate();
        const light_string buf = net_buffer.top_front();
        bool is_disconnected   = rt.inject_data(&buf[0], buf.size());
        net_buffer.pop_front();
        rt.req()->after_injection(is_disconnected);
        rt.req()->check_size_limitation();
    }
}

void Connection::detect_update(IObserver &observer) {
    for (;;) {
        // 何もなければループは1度だけ走る.
        // どこかで状態変化が起きた場合はもう1度最初から.
        if (rt.is_routable()) { // リクエストの解析が完了したら応答開始
            rt.route(*this);
        } else if (rt.is_freezable()) { // リクエストが凍結可能なら凍結実施
            const light_string extra_data = rt.freeze_request();
            // 注入したデータのうち余った分を extra_data_buffer に入れる
            net_buffer.push_front(extra_data);
        } else if (rt.is_originatable()) { // オリジネーション開始
            rt.originate(observer);
        } else if (rt.is_reroutable()) { // 再ルーティング
            rt.reroute(*this);
        } else if (rt.is_responsive()) { // レスポンス開始
            rt.respond();
            observer.reserve_set(this, IObserver::OT_WRITE);
        } else if (rt.is_terminatable()) { // レスポンスを終了できる場合 -> ラウンドトリップ終了
            if (rt.res() == NULL || rt.res()->should_close()) {
                DXOUT("CLOSE");
                shutdown_gracefully(observer);
            } else {
                DXOUT("KEEP");
                observer.reserve_unset(this, IObserver::OT_WRITE);
            }
            rt.wipeout();
        } else {
            break;
        }
    }
}

void Connection::perform_sending(IObserver &observer) {
    // レスポンスが存在しない状態でここに来る場合、何かの間違い.
    assert(rt.res() != NULL);

    if (rt.is_terminatable()) {
        return;
    }

    const ssize_t sent = sock->send(rt.res()->get_unsent_head(), rt.res()->get_unsent_size(), 0);
    // 送信ができなかったか, エラーが起きた場合
    if (sent < 0) {
        // TODO: とりあえず接続を閉じておくが, 本当はどうするべき？
        DXOUT("die?");
        die(observer);
        return;
    }
    rt.res()->mark_sent(sent);
}

void Connection::perform_shutting_down(IObserver &observer) {
    u8t buf[MAX_REQLINE_END];
    const ssize_t received_size = sock->receive(&buf, MAX_REQLINE_END, 0);
    if (received_size > 0) {
        return;
    }
    die(observer);
}

void Connection::shutdown_gracefully(IObserver &observer) {
    observer.reserve_unset(this, IObserver::OT_WRITE);
    observer.reserve_set(this, IObserver::OT_READ);
    sock->shutdown_write();
    phase = CONNECTION_SHUTTING_DOWN;
}

void Connection::die(IObserver &observer) {
    observer.reserve_unhold(this);
    sock->shutdown();
    dying = true;
}
