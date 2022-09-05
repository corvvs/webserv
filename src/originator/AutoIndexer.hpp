#ifndef AUTOINDEXER_HPP
#define AUTOINDEXER_HPP
#include "../Interfaces.hpp"
#include "../communication/RequestHTTP.hpp"
#include "../utils/http.hpp"
#include <dirent.h>
#include <fstream>
#include <map>
#include <sys/stat.h>

class AutoIndexer : public IOriginator {
public:
    typedef HTTP::byte_string byte_string;
    typedef HTTP::char_string char_string;
    typedef HTTP::light_string light_string;
    typedef byte_string::size_type size_type;
    typedef struct stat t_stat;

    struct Entry {
        HTTP::byte_string name;
        off_t size;
        time_t time;
        bool is_dir;

        // 1つのテーブル行としてHTMLに起こす
        HTTP::byte_string serialize(const HTTP::char_string &requested_path) const;
    };

    typedef std::vector<Entry> entry_list;

    struct Attribute {
        // ファイルシステム上のディレクトリパス
        const char_string directory_path;
        // リクエストされたパス(パーセントデコード済み)
        const char_string requested_path;
        // リクエストされたパス(パーセントデコード済み & スラッシュ縮約済み)
        const char_string effective_requested_path;

        Attribute(char_string directory_path, char_string requested_path, char_string effective_requested_path);
    };

    struct Status {
        DIR *dir;
        bool originated;

        Status();
    };

private:
    const Attribute attr;
    Status status;
    ResponseDataList response_data;
    entry_list entries;

    // ターゲットのディレクトリ内をスキャンして entries を取り出す
    void scan_from_directory();
    // entries をHTMLに起こす
    void render_html();
    void close_if_needed();
    ResponseHTTP::header_list_type determine_response_headers(const IResponseDataConsumer::t_sending_mode sm) const;

public:
    AutoIndexer(const RequestMatchingResult &match_result);
    ~AutoIndexer();

    void notify(IObserver &observer, IObserver::observation_category cat, t_time_epoch_ms epoch);
    void inject_socketlike(ISocketLike *socket_like);
    bool is_originatable() const;
    bool is_reroutable() const;
    HTTP::byte_string reroute_path() const;
    bool is_responsive() const;
    bool is_origination_started() const;
    void start_origination(IObserver &observer);
    void leave();
    ResponseHTTP *respond(const RequestHTTP *request, bool should_close);
};

#endif
