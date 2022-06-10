#ifndef HTTPSERVER_HPP
#define HTTPSERVER_HPP

class HTTPServer {

public:
    typedef int protocol_t;
    typedef int port_t;
    typedef int socket_t;
    typedef int error_t;
    typedef int read_buffer_t;
    typedef int request_t;
    typedef int response_t;
    typedef int write_buffer_t;

    // 接続受付
    // -> ソケット
    void listen_socket(protocol_t protocol, port_t port);
    socket_t accept_connection(socket_t socket);

    // データの受信
    // -> ソケット
    // -> バッファ(受信したデータを持つ)
    read_buffer_t receive_data(socket_t socket);

    // データをリクエストとして解釈(デコード)
    // -> 受信バッファ
    // -> リクエスト
    request_t decode_to_request(read_buffer_t read_buffer);

    // レスポンスの生成
    // -> リクエスト
    // -> レスポンス
    response_t create_response(request_t request);

    // レスポンスをデータにエンコード
    // -> レスポンス
    // -> 送信バッファ
    write_buffer_t encode_to_write_data(response_t response);


    // データの送信
    // -> 送信バッファ
    // -> ソケット
    void send_data(socket_t socket, write_buffer_t write_buffer);

    // [必要なもの](メンバ変数)
    // ソケット
    // 受信バッファ
    // 送信バッファ
    // リクエスト
    // レスポンス

private:

};

#endif
