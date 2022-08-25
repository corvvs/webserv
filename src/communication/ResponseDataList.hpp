#ifndef RESPONSE_DATA_LIST_HPP
#define RESPONSE_DATA_LIST_HPP
#include "../Interfaces.hpp"
#include "../utils/LightString.hpp"
#include "../utils/http.hpp"
#include "ParserHelper.hpp"
#include <assert.h>
#include <list>
#include <vector>

// レスポンスデータを生産するオブジェクトのインターフェース
class IResponseDataProducer {
public:
    virtual ~IResponseDataProducer() {}

    // 長さ n のバイト列を注入する
    virtual void inject(const char *src, size_t n, bool is_completed)     = 0;
    virtual void inject(const HTTP::byte_string &src, bool is_completed)  = 0;
    virtual void inject(const HTTP::light_string &src, bool is_completed) = 0;
    // データ注入完了
    virtual bool is_injection_closed() const = 0;
};

// レスポンスデータを供給するオブジェクトのインターフェース
class IResponseDataConsumer {
public:
    enum t_sending_mode { SM_CHUNKED, SM_NOT_CHUNKED };

    virtual ~IResponseDataConsumer() {}

    virtual t_sending_mode determine_sending_mode() = 0;
    // 初期データ与えて送信開始
    virtual void start(const HTTP::byte_string &initial_data) = 0;

    // 現行シリアライズデータ送信中
    virtual bool is_sending_current() const throw() = 0;
    // 現行シリアライズデータの送信完了
    virtual bool is_sent_current() const throw() = 0;
    // 送信が完了したかどうか
    virtual bool is_sending_over() const throw() = 0;

    virtual ssize_t send(IDataSender &sender, int flags) throw() = 0;

    virtual size_t current_total_size() const throw() = 0;

    // 現在バケットが空かどうか
    virtual bool empty() const throw() = 0;
    // 先頭のバケットの中身の要素を返す
    virtual const HTTP::byte_string &top() const = 0;
};

// レスポンスのchunked本文のchunk1つ分
struct ResponseDataBucket {
    HTTP::byte_string buffer; // データバッファ
    bool is_completed;

    ResponseDataBucket() throw();
};

// レスポンスのchunked本文全体
class ResponseDataList : public IResponseDataProducer, public IResponseDataConsumer {
public:
    typedef std::list<ResponseDataBucket> list_type;

private:
    list_type list;                    // バケットリスト
    HTTP::byte_string serialized_data; // サイズ行など込みで展開されたデータ
    size_t total;
    size_t sent_serialized;                      // serialized_data のうち送信済みのバイト数
    HTTP::Optional<t_sending_mode> sending_mode; // 送信モード

    void set_mode(t_sending_mode mode) throw();

    // シリアライズ実行可能
    bool is_serializable() const;
    // 全chunkシリアライズ完了
    bool is_all_serialized() const throw();

public:
    ResponseDataList();

    // [状態]
    // データ注入完了
    bool is_injection_closed() const;
    // 現行シリアライズデータ送信中
    bool is_sending_current() const throw();
    // 現行シリアライズデータの送信完了
    bool is_sent_current() const throw();
    // 全データ送信完了
    bool is_sending_over() const throw();

    size_t current_total_size() const throw();

    // 長さ n のデータを注入
    void inject(const char *src, size_t n, bool is_completed);
    void inject(const HTTP::byte_string &src, bool is_completed);
    void inject(const HTTP::light_string &src, bool is_completed);

    // 閉じたバケットをバイト列(HTTPレスポンスとして送信できる形式)にシリアライズ
private:
    HTTP::byte_string serialize_bucket(const ResponseDataBucket &bucket);
    // 送信したシリアライズデータのバイト数を記録
    void mark_sent(size_t n) throw();

public:
    // 可能かつ必要ならバケットを1つ取ってシリアライズする
    void serialize_if_needed();

    t_sending_mode determine_sending_mode();
    // 送信を開始する
    void start(const HTTP::byte_string &initial_data);

    // シリアライズデータの未送信部分の先頭を取得
    const HTTP::byte_string::value_type *serialized_head() const;
    // シリアライズデータの未送信部分のサイズを取得
    size_t rest_serialized() const throw();

    ssize_t send(IDataSender &sender, int flags) throw();

    // 現在バケットが空かどうか
    bool empty() const throw();
    // 先頭のバケットの中身の要素を返す
    const HTTP::byte_string &top() const;
};

class SendFileAgent : public IResponseDataConsumer {
public:
private:
    HTTP::char_string path_;
    // macOS の sendfile の仕様上, ファイルサイズは INT_MAX バイトまで.
    int total_size;
    int sent_size;
    HTTP::Optional<t_sending_mode> sending_mode; // 送信モード

    HTTP::byte_string former_data;
    bool sending_former;

    HTTP::byte_string leading_data;

    // 送信したシリアライズデータのバイト数を記録
    void mark_sent(size_t n) throw();

public:
    SendFileAgent(const HTTP::char_string &path, int file_size);

    t_sending_mode determine_sending_mode();
    // 初期データ与えて送信開始
    void start(const HTTP::byte_string &initial_data);

    // 現行シリアライズデータ送信中
    bool is_sending_current() const throw();
    // 現行シリアライズデータの送信完了
    bool is_sent_current() const throw();
    // 送信が完了したかどうか
    bool is_sending_over() const throw();

    ssize_t send(IDataSender &sender, int flags) throw();

    size_t current_total_size() const throw();
    // 現在バケットが空かどうか
    bool empty() const throw();
    // 先頭のバケットの中身の要素を返す
    const HTTP::byte_string &top() const;
};

#endif
