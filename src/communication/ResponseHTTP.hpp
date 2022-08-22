#ifndef RESPONSEHTTP_HPP
#define RESPONSEHTTP_HPP
#include "../utils/HTTPError.hpp"
#include "../utils/http.hpp"
#include "HeaderHTTP.hpp"
#include "Lifetime.hpp"
#include "ParserHelper.hpp"
#include "ResponseDataList.hpp"
#include <string>
#include <vector>

// [HTTPレスポンスクラス]
// [責務]
// - HTTPレスポンスを構成する情報をまとめ, HTTPメッセージデータ(テキストデータ)を生成すること
class ResponseHTTP {
public:
    typedef HTTP::byte_string byte_string;
    typedef HTTP::light_string light_string;
    typedef HTTP::header_dict_type header_dict_type;
    typedef std::vector<HTTP::header_kvpair_type> header_list_type;

private:
    HTTP::t_version version_;
    HTTP::t_status status_;
    minor_error merror;
    Lifetime lifetime;
    // 送信済みマーカー
    size_t sent_size;
    header_list_type header_list;
    header_dict_type header_dict;
    byte_string body;
    byte_string message_text;
    ResponseDataList local_datalist;
    IResponseDataConsumer *data_consumer_;
    bool should_close_;

    IResponseDataConsumer *consumer() throw();
    const IResponseDataConsumer *consumer() const throw();

public:
    // 通常(エラーでない)応答を構築する
    ResponseHTTP(HTTP::t_version version,
                 HTTP::t_status status,
                 const header_list_type *headers,
                 IResponseDataConsumer *data_consumer,
                 bool should_close);
    // unrecovarable エラー応答を構築する
    ResponseHTTP(HTTP::t_version version, const http_error &error, bool should_close);
    // recovarable エラー応答を構築する
    ResponseHTTP(HTTP::t_version version, const minor_error &error, bool should_close);

    ~ResponseHTTP();

    // HTTPヘッダを追加する
    void feed_header(const HTTP::header_key_type &key, const HTTP::header_val_type &val, bool overwrite = false);

    // 状態行 + ヘッダ部をバイト列に展開する
    byte_string serialize_former_part();
    void start();

    // renderされたHTTPメッセージデータ全体を返す
    const byte_string &get_message_text() const throw();
    // 未送信のHTTPメッセージデータを返す
    const byte_string::value_type *get_unsent_head();
    // 送信済みバイト数を増やす
    void mark_sent(ssize_t sent) throw();
    // 未送信のHTTPメッセージデータのサイズを返す
    size_t get_unsent_size() const throw();

    // predicate: メッセージ全体の送信が完了したかどうか
    bool is_complete() const throw();
    // predicate: エラー応答かどうか
    bool is_error() const throw();
    // predicate: タイムアウトしているかどうか
    bool is_timeout(t_time_epoch_ms now) const throw();
    // predicate: このレスポンスを送り終わった後, HTTP接続を閉じるべきかどうか
    bool should_close() const throw();

    static void swap(ResponseHTTP &lhs, ResponseHTTP &rhs) throw();
};

#endif
