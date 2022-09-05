#include "Connection.hpp"
#include "Channel.hpp"
#include "RoundTrip.hpp"
#include <cassert>
#define MAX_REQLINE_END 8192
#define MAX_EXTRA_AMOUNT 1048576

Connection::Attribute::Attribute(SocketConnected *sock) : sock(sock) {}

Connection::Attribute::~Attribute() {
    delete sock;
}

// [[Status]]
Connection::Status::Status() : phase(CONNECTION_ESTABLISHED), dying(false), spilling_readside(false) {}

bool Connection::Status::is_readside_active() const throw() {
    return !dying && !spilling_readside;
}

// [[NetwotkBuffer]]

Connection::NetworkBuffer::NetworkBuffer() : read_size(0), extra_amount(0) {
    read_buffer.resize(MAX_REQLINE_END);
}

ssize_t Connection::NetworkBuffer::spill(SocketConnected &sock) {
    u8t buf[MAX_REQLINE_END];
    const ssize_t received_size = sock.receive(&buf, MAX_REQLINE_END, 0);
    return received_size;
}

ssize_t Connection::NetworkBuffer::receive(SocketConnected &sock) {
    if (read_size > 0) {
        // read_buffer にデータがあるならそれを待避させる
        extra_buffer.push_back(read_buffer);
        extra_amount += extra_buffer.back().size();
        check_extra_overflow();
        read_size = 0;
        read_buffer.resize(read_size);
        return extra_buffer.front().size();
    }

    read_buffer.resize(MAX_REQLINE_END);
    byte_string::value_type *rb = &read_buffer.front();
    read_size                   = sock.receive(rb, MAX_REQLINE_END, 0);
    if (read_size >= 0) {
        read_buffer.resize(read_size);
    }
    if (extra_buffer.size() > 0) {
        return extra_buffer.front().size();
    }
    return read_size;
}

const Connection::byte_string &Connection::NetworkBuffer::top_front() const {
    if (extra_buffer.size() > 0) {
        VOUT(extra_buffer.front().size());
        BVOUT(extra_buffer.front());
        return extra_buffer.front();
    }
    return read_buffer;
}

void Connection::NetworkBuffer::pop_front() {
    if (extra_buffer.size() > 0) {
        extra_amount -= extra_buffer.front().size();
        extra_buffer.pop_front();
        VOUT(extra_amount);
    } else {
        read_buffer.resize(0);
        read_size = 0;
    }
}

void Connection::NetworkBuffer::push_front(const light_string &data) {
    if (data.size() > 0) {
        extra_buffer.push_front(data.str());
        extra_amount += extra_buffer.front().size();
        check_extra_overflow();
    }
}

bool Connection::NetworkBuffer::should_redo() const {
    return extra_buffer.size() > 0 || read_size > 0;
}

void Connection::NetworkBuffer::check_extra_overflow() {
    VOUT(extra_amount);
    if (extra_amount > MAX_EXTRA_AMOUNT) {
        throw http_error("pipeline is full", HTTP::STATUS_PAYLOAD_TOO_LARGE);
    }
}

// [[Connection]]

Connection::Connection(SocketConnected *sock_given,
                       IRouter *router,

                       const config::config_vector &configs,
                       FileCacher &cacher)
    : attr(sock_given), status(Status()), rt(*router, configs, cacher), lifetime(Lifetime::make_connection()) {
    DXOUT("[established] " << attr.sock->get_fd());
}

Connection::~Connection() {
    DXOUT("DESTROYED: " << this);
}

bool Connection::is_extra_readable_cat(IObserver::observation_category cat) throw() {
    switch (cat) {
        case IObserver::OT_READ:
        case IObserver::OT_WRITE:
            return true;
        default:
            return false;
    }
}

t_fd Connection::get_fd() const {
    return attr.sock->get_fd();
}

t_port Connection::get_port() const {
    return attr.sock->get_port();
}

void Connection::notify(IObserver &observer, IObserver::observation_category cat, t_time_epoch_ms epoch) {
    try {
        if (status.dying) {
            return;
        }
        switch (status.phase) {
            case CONNECTION_ESTABLISHED:
                perform_reaction(observer, cat, epoch);
                if (cat == IObserver::OT_TIMEOUT) {
                    return;
                }
                detect_update(observer);

                // ネットワークバッファに残余データがある場合,
                // それを使って新しいHTTPリクエストを構成する必要がある(HTTPパイプライン対応).
                // - 残余データが増加するのは:
                //   -> HTTPリクエストが凍結された時に余分なデータがある場合.
                // - 残余データが消費されるのは:
                //   -> Connection 自身についてのI/Oイベントの後で残余データが存在し,
                //      かつラウンドトリップにHTTPリクエストがないか, あっても凍結していない場合.
                if (is_extra_readable_cat(cat)) {
                    while (is_extra_readable()) {
                        DXOUT("Extra Read");
                        perform_reaction(observer, IObserver::OT_READ, epoch);
                        detect_update(observer);
                    }
                }
                return;
            case CONNECTION_SHUTTING_DOWN: // コネクション 切断中
                if (cat == IObserver::OT_READ) {
                    // read していい場合だけ
                    perform_shutting_down(observer);
                }
                return;
            default:
                DXOUT("unexpected phase: " << status.phase);
                assert(false);
        }
        return;
    } catch (const http_error &err) {
        // [回復不可能なHTTPエラーが発生]
        DXOUT("UNRECOVERABLE ERROR occurred");
        // この状態になったら:
        // - 受信データはすべて捨てる
        // - リクエストの状態は変化させず, 状態検査もしない
        status.spilling_readside = true;
        if (status.phase == CONNECTION_SHUTTING_DOWN) {
            return;
        }
        if (!rt.is_responding()) {
            // 送信中のレスポンスがないなら, エラーレスポンスを作成して送信する
            try {
                rt.respond_unrecoverable_error(observer, err);
                observer.reserve_set(this, IObserver::OT_WRITE);
                return;
            } catch (const http_error &err) {
                // エラー送信でもう1回エラー
                DXOUT("double error: " << err.get_status() << " - " << err.what());
            }
        }
        // レスポンス送信中のHTTPエラー -> 全てを諦めて終了
        start_shutdown(observer);
    }
}

void Connection::perform_reaction(IObserver &observer, IObserver::observation_category cat, t_time_epoch_ms epoch) {
    switch (cat) {
        case IObserver::OT_TIMEOUT:
            if (lifetime.is_timeout(epoch)) {
                throw http_error("connection timed out", HTTP::STATUS_TIMEOUT);
            }
            if (rt.is_timeout(epoch)) {
                throw http_error("roundtrip timed out", HTTP::STATUS_TIMEOUT);
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
            if (status.spilling_readside) {
                perform_shutting_down(observer);
            } else {
                perform_receiving(observer);
            }
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
    const ssize_t received_size = net_buffer.receive(*attr.sock);
    if (received_size <= 0) {
        // なにも受信できなかったか, 受信エラーが起きた場合
        die(observer);
        return;
    }

    // データ注入
    // インバウンド許容
    // -> 受信したデータをリクエストに注入する
    if (rt.is_freezed()) {
        return;
    }
    rt.start_if_needed();
    lifetime.activate();
    const light_string buf     = net_buffer.top_front();
    const bool is_disconnected = rt.inject_data(buf);
    net_buffer.pop_front();
    rt.req()->after_injection(is_disconnected);
    rt.req()->check_size_limitation();
}

void Connection::detect_update(IObserver &observer) {
    for (;;) {
        // 何もなければループは1度だけ走る.
        // どこかで状態変化が起きた場合はもう1度最初から.
        if (rt.is_routable()) { // リクエストの解析が完了したら応答開始
            rt.route(*this);
        } else if (rt.is_freezable()) { // リクエストが凍結可能なら凍結実施
            const light_string extra_data = rt.freeze_request();
            // 注入したデータのうち余った分を extra_buffer に入れる
            net_buffer.push_front(extra_data);
        } else if (rt.is_originatable()) { // オリジネーション開始
            rt.originate(observer);
        } else if (rt.is_reroutable()) { // 再ルーティング
            rt.reroute(*this);
        } else if (rt.is_responsive()) { // レスポンス開始
            rt.respond();
            observer.reserve_set(this, IObserver::OT_WRITE);
        } else if (rt.is_terminatable()) { // ラウンドトリップが終了できる場合 -> ラウンドトリップ終了
            if (rt.res() == NULL || rt.res()->should_close()) {
                DXOUT("CLOSE");
                start_shutdown(observer);
            } else {
                DXOUT("KEEP");
                observer.reserve_unset(this, IObserver::OT_WRITE);
            }
            rt.wipeout();
        } else {
            break;
        }

        rt.emit_fatal_error();
    }
}

void Connection::perform_sending(IObserver &observer) {
    // レスポンスが存在しない状態でここに来る場合、何かの間違い.
    assert(rt.res() != NULL);

    if (rt.is_terminatable()) {
        return;
    }

    const ssize_t sent = attr.sock->send(rt.res()->get_unsent_head(), rt.res()->get_unsent_size(), 0);
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
    const ssize_t received_size = net_buffer.spill(*attr.sock);
    if (received_size <= 0) {
        die(observer);
    }
}

void Connection::start_shutdown(IObserver &observer) {
    observer.reserve_unset(this, IObserver::OT_WRITE);
    observer.reserve_set(this, IObserver::OT_READ);
    attr.sock->shutdown_write();
    status.phase = CONNECTION_SHUTTING_DOWN;
}

void Connection::die(IObserver &observer) {
    observer.reserve_unhold(this);
    attr.sock->shutdown();
    status.dying = true;
}

bool Connection::is_extra_readable() const throw() {
    return status.is_readside_active() && !rt.is_freezed() && net_buffer.should_redo();
}
