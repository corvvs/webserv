#include "Connection.hpp"
#include "Channel.hpp"

int started  = 0;
int finished = 0;

ConnectionAttribute::ConnectionAttribute() {
    http_version  = HTTP::DEFAULT_HTTP_VERSION;
    is_persistent = true;
    timeout       = 60 * 1000;
}

Connection::Connection() {
    throw std::runtime_error("forbidden");
}

Connection::Connection(IRouter *router, SocketConnected *sock_given)
    : router_(router)
    , attr(ConnectionAttribute())
    , phase(CONNECTION_RECEIVING)
    , dying(false)
    , sock(sock_given)
    , current_req(NULL)
    , current_res(NULL)
    , latest_operated_at(0) {
    started_ = started++;
    DXOUT("started_: " << started_);
    touch();
}

Connection::~Connection() {
    delete sock;
    delete current_req;
    delete current_res;
    DXOUT("finished: " << finished++ << " - " << started_);
}

t_fd Connection::get_fd() const {
    return sock->get_fd();
}

void Connection::notify(IObserver &observer, IObserver::observation_category cat) {
    (void)cat;
    if (dying) {
        return;
    }

    const size_t read_buffer_size = HTTP::MAX_REQLINE_END;
    char buf[read_buffer_size];

    switch (phase) {
        case CONNECTION_RECEIVING: {
            // [データ受信]

            try {

                ssize_t receipt = sock->receive(&buf, read_buffer_size, 0);
                if (receipt <= 0) {
                    // なにも受信できなかったか, 受信エラーが起きた場合
                    die(observer);
                    return;
                }
                touch();

                if (current_req == NULL) {
                    // リクエストオブジェクトを作成して解析開始
                    current_req = new RequestHTTP();
                }
                current_req->feed_bytestring(buf, receipt);
                if (!current_req->is_ready_to_originate()) {
                    return;
                }

                // リクエストの解析が完了したら応答開始
                current_res = router_->route(current_req);
                ready_sending(observer);

            } catch (http_error err) {

                // 受信中のHTTPエラー
                DXOUT(err.get_status() << ":" << err.what());
                current_res = router_->respond_error(current_req, err);
                ready_sending(observer);
            }
            return;
        }

        case CONNECTION_ERROR_RESPONDING:
        case CONNECTION_RESPONDING: {
            // [データ送信]

            try {

                ssize_t sent = sock->send(current_res->get_unsent_head(), current_res->get_unsent_size(), 0);

                // 送信ができなかったか, エラーが起きた場合
                if (sent <= 0) {
                    // TODO: とりあえず接続を閉じておくが, 本当はどうするべき？
                    die(observer);
                    return;
                }
                touch();
                current_res->mark_sent(sent);
                if (!current_res->is_over_sending()) {
                    return;
                }

                // 送信完了
                if (current_req->should_keep_in_touch()) {
                    // 接続を維持する
                    ready_receiving(observer);
                    return;
                } else {
                    ready_shutting_down(observer);
                    // die(observer);
                    return;
                }

            } catch (http_error err) {

                // 送信中のHTTPエラー -> もうだめ
                DXOUT(err.get_status() << ":" << err.what());
                die(observer);
            }
            return;
        }

        case CONNECTION_SHUTTING_DOWN: {
            // [graceful切断]

            try {

                ssize_t receipt = sock->receive(&buf, read_buffer_size, 0);
                if (receipt > 0) {
                    return;
                }
                die(observer);

            } catch (http_error err) {

                DXOUT(err.get_status() << ":" << err.what());
                die(observer);
            }
            return;
        }

        default: {
            DXOUT("unexpected phase: " << phase);
            throw std::runtime_error("????");
        }
    }
}

void Connection::timeout(IObserver &observer, t_time_epoch_ms epoch) {
    if (dying) {
        return;
    }
    if (epoch < attr.timeout + latest_operated_at) {
        return;
    }
    // タイムアウト処理
    DXOUT("timeout!!: " << get_fd());
    die(observer);
}

void Connection::touch() {
    t_time_epoch_ms t = WSTime::get_epoch_ms();
    DXOUT("operated_at: " << latest_operated_at << " -> " << t);
    latest_operated_at = t;
}

void Connection::ready_receiving(IObserver &observer) {
    delete current_res;
    delete current_req;
    current_res = NULL;
    current_req = NULL;
    observer.reserve_transit(this, IObserver::OT_WRITE, IObserver::OT_READ);
    phase = CONNECTION_RECEIVING;
}

void Connection::ready_sending(IObserver &observer) {
    observer.reserve_transit(this, IObserver::OT_READ, IObserver::OT_WRITE);
    phase = CONNECTION_RESPONDING;
}

void Connection::ready_shutting_down(IObserver &observer) {
    observer.reserve_transit(this, IObserver::OT_WRITE, IObserver::OT_READ);
    sock->shutdown_write();
    phase = CONNECTION_SHUTTING_DOWN;
}

void Connection::die(IObserver &observer) {
    switch (phase) {
        case CONNECTION_RESPONDING:
        case CONNECTION_ERROR_RESPONDING: {
            observer.reserve_clear(this, IObserver::OT_WRITE);
            break;
        }
        default: {
            observer.reserve_clear(this, IObserver::OT_READ);
            break;
        }
    }
    sock->shutdown();
    dying = true;
}
