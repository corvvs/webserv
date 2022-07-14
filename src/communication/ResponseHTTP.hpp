#ifndef RESPONSEHTTP_HPP
#define RESPONSEHTTP_HPP
#include "../utils/HTTPError.hpp"
#include "../utils/http.hpp"
#include "HeaderHTTP.hpp"
#include "ParserHelper.hpp"
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

private:
    HTTP::t_version version_;
    HTTP::t_status status_;
    bool is_error;
    // 送信済みマーカー
    size_t sent_size;

    std::vector<HTTP::header_kvpair_type> header_list;
    header_dict_type header_dict;
    byte_string body;

    byte_string message_text;

public:
    // 通常(エラーでない)応答を構築する
    ResponseHTTP(HTTP::t_version version, HTTP::t_status status);

    // エラー応答を構築する
    ResponseHTTP(HTTP::t_version version, http_error error);

    // HTTPヘッダを追加する
    void feed_header(const HTTP::header_key_type &key, const HTTP::header_val_type &val);

    // ボディを追加する
    void feed_body(const byte_string &str);
    // ボディを追加する
    void feed_body(const light_string &str);

    // 保持している情報をもとにHTTPメッセージのテキストデータを生成し,
    // message_text に入れる.
    void render();

    // renderされたHTTPメッセージデータ全体を返す
    const byte_string &get_message_text() const;
    // 未送信のHTTPメッセージデータを返す
    const char *get_unsent_head() const;
    // 送信済みバイト数を増やす
    void mark_sent(ssize_t sent);
    // 未送信のHTTPメッセージデータのサイズを返す
    size_t get_unsent_size() const;

    // predicate: メッセージ全体の送信が完了したかどうか
    bool is_over_sending() const;
};

#endif
