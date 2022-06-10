#ifndef HTTPSERVER_HPP
#define HTTPSERVER_HPP

class HTTPServer {

public:
    typedef int protocol_t;
    typedef int port_t;
    typedef int socket_t;
    typedef int error_t;

    // 接続受付
    // -> ソケット
    error_t listen(protocol_t protocol, port_t port);
    socket_t accept(socket_t socket);

    // データの受信
    // -> ソケット
    // -> バッファ(受信したデータを持つ)
    int receive();

    // データをリクエストとして解釈(デコード)
    // -> 受信バッファ
    // -> リクエスト

    // レスポンスの生成
    // -> リクエスト
    // -> レスポンス

    // レスポンスをデータにエンコード
    // -> レスポンス
    // -> 送信バッファ

    // データの送信
    // -> 送信バッファ
    // -> ソケット

    // [必要なもの](メンバ変数)
    // ソケット
    // 受信バッファ
    // 送信バッファ
    // リクエスト
    // レスポンス

private:

};

#endif
