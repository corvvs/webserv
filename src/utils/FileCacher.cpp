#include "FileCacher.hpp"
#include "../utils/test_common.hpp"
#include "File.hpp"
#include "LRUCache.hpp"
#include <ctime>
#include <utility>

FileCacher::FileCacher() : cache_(MAX_CACHE_ARRAY_SIZE) {}
FileCacher::~FileCacher() {}

FileCacher::FileCacheData::FileCacheData(const std::string &path_) : path(path_) {}
FileCacher::FileCacheData::FileCacheData(const std::string &path_,
                                         const time_t &cached_at_,
                                         const size_t &size_,
                                         const byte_string &data_)
    : path(path_), cached_at(cached_at_), size(size_), data(data_) {}

FileCacher::FileCacheData &FileCacher::FileCacheData::operator=(const FileCacheData &rhs) {
    if (this == &rhs) {
        return *this;
    }
    cached_at = rhs.cached_at;
    size      = rhs.size;
    data      = rhs.data;
    return *this;
}

void FileCacher::erase(const std::string &path) {
    cache_.erase(path);
}

bool FileCacher::add(const std::string &path) {
    const long file_size = file::get_size(path);
    if (file_size < 0) {
        return false;
    }
    // キャッシュできる最大サイズを超えている
    if (file_size > MAX_CACHE_DATA_SIZE) {
        return false;
    }

    // ファイルを読み取りキャッシュデータを入れる
    byte_string file_data = HTTP::strfy(file::read(path));
    FileCacheData data(path, std::time(NULL), file_size, file_data);
    cache_.add(data.path, data);
    return true;
}

/**
 * キャッシュの実態がレスポンスを返している途中に削除される可能性があるため,
 * fetchの戻り値である entry_type のポインタからレスポンス用のデータをディープコピーで生成する必要がある
 */
std::pair<minor_error, const FileCacher::entry_type *> FileCacher::fetch(const std::string &path) {
    // パスにファイルが存在していない -> キャッシュがあるなら削除し, 404を返す。
    if (!file::is_file(path)) {
        erase(path);
        return std::make_pair(minor_error::make("file not found", HTTP::STATUS_NOT_FOUND),
                              static_cast<entry_type *>(NULL));
    }

    // ファイルはあるが読み取り権限がない, 通常ファイルではない -> キャッシュがあるなら削除し, 403を返す
    if (!file::is_readable(path)) {
        erase(path);
        return std::make_pair(minor_error::make("forbidden", HTTP::STATUS_FORBIDDEN), static_cast<entry_type *>(NULL));
    }

    // キャッシュが存在して, 最終修正時刻がキャッシュ時刻より古い -> キャッシュデータを返す
    if (cache_.exists(path)) {
        const time_t last_update = file::get_last_update_time(path);
        if (last_update == -1) {
            return std::make_pair(minor_error::make("server error", HTTP::STATUS_INTERNAL_SERVER_ERROR),
                                  static_cast<entry_type *>(NULL));
        }

        cache_const_iterator it         = cache_.fetch(path);
        const FileCacheData &cache_data = it->second;
        // ファイルの最終修正時刻がキャッシュ時刻より古い場合はキャッシュを返す
        if (cache_data.cached_at > last_update) {
            // DXOUT(HTTP::restrfy(cache_data.data));
            return std::make_pair(minor_error::ok(), &cache_data);
        }
    }

    // ファイルを読み取り、キャッシュを追加する
    if (!add(path)) {
        return std::make_pair(minor_error::make("data too large to cache", HTTP::STATUS_BAD_REQUEST),
                              static_cast<entry_type *>(NULL));
    }
    cache_const_iterator res        = cache_.fetch(path);
    const FileCacheData &cache_data = res->second;
    // 作成したキャッシュを返す
    return std::make_pair(minor_error::ok(), &cache_data);
}
