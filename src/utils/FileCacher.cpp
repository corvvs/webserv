#include "FileCacher.hpp"
#include "File.hpp"
#include <ctime>
#include <utility>

FileCacher::FileCacher() {}
FileCacher::~FileCacher() {}

void FileCacher::erase_cache(const std::string &path) {
    std::map<std::string, FileCacheData>::iterator it = cache_data_.find(path);
    if (it != cache_data_.end()) {
        cache_data_.erase(it);
    }
}

std::pair<bool, FileCacher::FileCacheData> FileCacher::create_cache(const std::string &path) {
    const long file_size = file::get_size(path);
    if (file_size < 0) {
        return std::make_pair(false, FileCacheData());
    }
    // キャッシュできる最大サイズを超えている
    if (file_size > MAX_CACHE_SIZE) {
        return std::make_pair(false, FileCacheData());
    }

    // ファイルを読み取りキャッシュデータを入れる
    byte_string file_data = HTTP::strfy(file::read(path));
    FileCacheData res;
    res.size      = file_size;
    res.cached_at = std::time(NULL);
    res.data      = file_data;
    return std::make_pair(true, res);
}

/**
 * ファイルキャッシュの処理フロー
 * - パスにファイルが存在していない -> キャッシュがあるなら削除し, 404を返す。
 * - ファイルはあるが読み取り権限がない, 通常ファイルではない -> キャッシュがあるなら削除し, 403を返す
 * - キャッシュが存在して, 最終修正時刻がキャッシュ時刻より古い -> キャッシュデータを返す
 * - 以上のいずれでもない(= ファイルが存在して読み取り可能だが, キャッシュが存在しないか,
 *
 * キャッシュ時刻より後に更新されている)なら、以下を実行
 *   - ファイルを開いて中身を読み取り、キャッシュデータを作成する。
 *   - 「ファイルサイズが一定値以下」という条件を
 *     - 満たす場合:
 *         - キャッシュデータを更新する。
 *     - 満たさない場合:
 *         - キャッシュがあるならキャッシュデータを削除する。
 *       - 作成したキャッシュデータを返す。
 */
std::pair<minor_error, const FileCacher::entry_type *> FileCacher::fetch(const std::string &path) {
    // パスにファイルが存在していない -> キャッシュがあるなら削除し, 404を返す。
    if (!file::is_file(path)) {
        erase_cache(path);
        return std::make_pair(minor_error::make("file not found", HTTP::STATUS_NOT_FOUND), (entry_type *)NULL);
    }

    //   ファイルはあるが読み取り権限がない, 通常ファイルではない -> キャッシュがあるなら削除し, 403を返す
    if (!file::is_readable(path)) {
        erase_cache(path);
        return std::make_pair(minor_error::make("forbidden", HTTP::STATUS_FORBIDDEN), (entry_type *)NULL);
    }

    // キャッシュが存在して, 最終修正時刻がキャッシュ時刻より古い -> キャッシュデータを返す
    if (cache_data_.find(path) != cache_data_.end()) {
        const time_t last_update = file::get_last_update_time(path);
        if (last_update == -1) {
            return std::make_pair(minor_error::make("server error", HTTP::STATUS_INTERNAL_SERVER_ERROR),
                                  (entry_type *)NULL);
        }
        // 最終修正時刻がキャッシュ時刻より古い
        if (cache_data_[path].cached_at > last_update) {
            return std::make_pair(minor_error::ok(), &cache_data_[path]);
        }
    }

    // 以上のいずれでもない(= ファイルが存在して読み取り可能だが, キャッシュが存在しないか,
    // キャッシュ時刻より後に更新されている)なら、以下を実行
    std::pair<bool, FileCacheData> p = create_cache(path);
    if (p.first == false) {
        return std::make_pair(minor_error::make("server error", HTTP::STATUS_INTERNAL_SERVER_ERROR),
                              (entry_type *)NULL);
    }
    FileCacheData new_cache = p.second;
    cache_data_[path]       = new_cache;
    return std::make_pair(minor_error::ok(), &cache_data_[path]);
}
