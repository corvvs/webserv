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

class IResponseDataConsumer;

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
    const HTTP::t_version version_;
    const HTTP::t_status status_;
    minor_error merror;
    Lifetime lifetime;
    // 送信済みマーカー
    header_list_type header_list;
    header_dict_type header_dict;
    IResponseDataConsumer &data_consumer_;
    const bool should_close_;

    // HTTPヘッダを追加する
    void feed_header(const HTTP::header_key_type &key, const HTTP::header_val_type &val, bool overwrite = false);
    // 状態行 + ヘッダ部をバイト列に展開する
    byte_string serialize_former_part();
    void start();

    IResponseDataConsumer &consumer() throw();
    const IResponseDataConsumer &consumer() const throw();
    // 送信済みバイト数を増やす
    void mark_sent(ssize_t sent) throw();

public:
    // 通常(エラーでない)応答を構築する
    ResponseHTTP(HTTP::t_version version,
                 HTTP::t_status status,
                 const header_list_type *headers,
                 IResponseDataConsumer &data_consumer,
                 bool should_close);
    ~ResponseHTTP();

    // predicate: メッセージ全体の送信が完了したかどうか
    bool is_complete() const throw();
    // predicate: エラー応答かどうか
    bool is_error() const throw();
    // predicate: タイムアウトしているかどうか
    bool is_timeout(t_time_epoch_ms now) const throw();
    // predicate: このレスポンスを送り終わった後, HTTP接続を閉じるべきかどうか
    bool should_close() const throw();

    bool send_data(IDataSender &sender) throw();
};

#endif
