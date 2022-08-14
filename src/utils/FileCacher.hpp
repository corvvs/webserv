#ifndef FILECACHER_HPP
#define FILECACHER_HPP

#include "HTTPError.hpp"
#include "http.hpp"
#include "types.hpp"
#include <string>

class FileCacher {

public:
    struct FileCacheData {
        // ファイルパス
        //        const std::string path;
        //        t_time_epoch_ms cached_at;

        // キャッシュされた時刻
        time_t cached_at;
        // ファイルサイズ
        size_t size;
        // ファイルデータ
        HTTP::byte_string data;
    };

    typedef FileCacheData entry_type;

    FileCacher();
    ~FileCacher();

    // 指定したパスのキャッシュデータを保持していれば返す
    // その過程でエラーが起きた場合, それをfirstに入れて返す(secondはNULL).
    std::pair<minor_error, const entry_type *> fetch(const std::string &path);

    void erase_cache(const std::string &path);
    std::pair<bool, FileCacheData> create_cache(const std::string &path);

private:
    typedef HTTP::byte_string byte_string;

    // キャッシュするデータの上限値
    static const long MAX_CACHE_SIZE = 1024 * 10;

    // パスをキーとしたCacheDataをmapで管理する
    std::map<std::string, FileCacheData> cache_data_;
};
#endif
