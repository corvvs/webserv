#ifndef FILEREADER_HPP
#define FILEREADER_HPP
#include "../Interfaces.hpp"
#include "../communication/RequestHTTP.hpp"
#include "../utils/FileCacher.hpp"
#include "../utils/http.hpp"
#include <fstream>
#include <map>

class FileReader : public IOriginator, public ISocketLike {
public:
    typedef HTTP::byte_string byte_string;
    typedef HTTP::char_string char_string;
    typedef HTTP::light_string light_string;
    typedef byte_string::size_type size_type;

    struct Attribute {
        FileCacher &cacher_;
        const ICacheInfoProvider *cache_info_provider;
        char_string file_path_;
        IObserver *observer;
        ISocketLike *master;
        t_fd fd_;

        void close_fd() throw();

        Attribute(FileCacher &cacher_, const ICacheInfoProvider *cache_info_provider, char_string file_path_);
    };

    struct Status {
        t_time_epoch_ms last_modified;
        bool originated;
        bool is_not_modified;
        bool leaving;
        bool is_responsive;

        Status();
    };

protected:
    ResponseDataList response_data;
    Attribute attr;
    Status status;

    // ファイルからデータを読み出す準備をする
    // 続けて読み出し処理をする場合は true を返す.
    // 失敗した場合は例外を投げる
    bool prepare_reading();
    // ファイルからデータを読み出し, response_data に入れる

    void close_fd() throw();

    // キャッシュデータを読み込む
    bool read_from_cache();
    byte_string infer_content_type() const;
    ResponseHTTP::header_list_type determine_response_headers(const IResponseDataConsumer::t_sending_mode sm) const;

    void perform_receiving(IObserver &observer);
    // (ISocketLikeのメソッドではない)
    // 自分に来たイベント通知を, 他のISocketLikeに飛ばす.
    // その際イベント通知に変換を施すかもしれない.
    void retransmit(IObserver &observer, IObserver::observation_category cat, t_time_epoch_ms epoch);

public:
    FileReader(const RequestMatchingResult &match_result, FileCacher &cacher, const ICacheInfoProvider *info_provider);
    FileReader(const char_string &path, FileCacher &cacher);
    ~FileReader();

    virtual t_fd get_fd() const;
    virtual t_port get_port() const;
    virtual void notify(IObserver &observer, IObserver::observation_category cat, t_time_epoch_ms epoch);
    virtual void inject_socketlike(ISocketLike *socket_like);
    virtual bool is_originatable() const;
    virtual bool is_reroutable() const;
    HTTP::byte_string reroute_path() const;
    virtual bool is_responsive() const;
    virtual bool is_origination_started() const;
    virtual void start_origination(IObserver &observer);
    virtual void leave();
    virtual ResponseHTTP *respond(const RequestHTTP *request, bool should_close);
};

#endif
