#ifndef FILECACHER_HPP
#define FILECACHER_HPP

#include "HTTPError.hpp"
#include "LRUCache.hpp"
#include "http.hpp"
#include "types.hpp"
#include <string>
#include <vector>

class FileCacher {
public:
    typedef HTTP::byte_string byte_string;

    struct FileCacheData {
        // ファイルパス
        const std::string path;
        // キャッシュされた時刻
        time_t cached_at;
        // ファイルサイズ
        size_t size;
        // ファイルデータ
        byte_string data;

        FileCacheData(const std::string &path_ = "");
        FileCacheData(const std::string &path_,
                      const time_t &cached_at_,
                      const size_t &size_,
                      const byte_string &data_);
    };

    typedef FileCacheData entry_type;

    FileCacher();
    ~FileCacher();

    // 指定したパスのキャッシュデータを保持していれば返す
    // その過程でエラーが起きた場合, それをfirstに入れて返す(secondはNULL).
    std::pair<minor_error, const entry_type *> fetch(const std::string &path);

    void erase(const std::string &path);
    bool add(const std::string &path);

private:
    typedef std::string path_type;
    typedef std::string data_type;
    typedef LRUCache<path_type, FileCacheData> cache_type;
    typedef cache_type::const_iterator cache_const_iterator;

    // キャッシュするデータの上限値
    static const long MAX_CACHE_DATA_SIZE = 1024 * 5;
    // キャッシュする配列の上限値
    static const long MAX_CACHE_ARRAY_SIZE = 2;

    // 利用順にもとづいてキャッシュを作る
    cache_type cache_;
};
#endif
