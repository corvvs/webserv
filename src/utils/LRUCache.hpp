#ifndef LRUCACHE_HPP
#define LRUCACHE_HPP
#include "test_common.hpp"
#include <iostream>
#include <list>
#include <map>
template <class Key, class Value>
class LRUCache {
public:
    typedef Key key_type;
    typedef Value data_type;
    typedef std::list<key_type> list_type;
    typedef typename list_type::iterator iterator;

    typedef std::pair<key_type, data_type> value_type;
    typedef std::map<key_type, data_type> data_map_type;
    typedef typename data_map_type::const_iterator const_iterator;
    typedef std::map<key_type, typename list_type::iterator> iterator_map_type;

private:
    // キャッシュができる上限値
    size_t capacity_;

    // アクセス順に並んでいるリスト(keyだけを持つ)
    list_type access_list_;

    // key と list のイテレータを value にもつ map
    iterator_map_type iter_map_;

    // key と キャッシュしたデータを持つ map
    data_map_type data_map_;

public:
    LRUCache(size_t capacity) : capacity_(capacity) {}

    const_iterator begin() const {
        return data_map_.begin();
    }

    const_iterator end() const {
        return data_map_.end();
    }

    /**
     * キーが存在する場合は対応するキャッシュデータを返す
     * 存在しない場合は end イテレータを返す
     */
    const_iterator fetch(const key_type &key) {
        if (iter_map_.count(key) == 0) {
            return end();
        }

        // 存在する場合
        typename list_type::iterator it = iter_map_[key];
        const key_type k                      = *it;

        // アクセスリストから削除
        access_list_.erase(it);
        // 新たにアクセスリストに追加する

        access_list_.push_front(k);

        // イテレータを更新する
        iter_map_[key] = access_list_.begin();
        return data_map_.find(key);
    }

    /**
     * キャッシュがすでに存在する場合は更新する
     * 存在しない場合は新しくキャッシュに追加する
     * capacityを超える場合は使用頻度が最も低いキャッシュを削除する
     */
    void add(const key_type &key, const data_type &value) {
        // 存在する場合は既存のアクセスリストから削除してから更新する
        if (iter_map_.count(key) != 0) {
            access_list_.erase(iter_map_[key]);
        }
        // キャッシュを追加する
        access_list_.push_front(key);
        iter_map_[key] = access_list_.begin();
        data_map_[key] = value;

        // capacityを超える場合は使用頻度が最も低いcacheを削除する
        if (access_list_.size() > capacity_) {
            typename list_type::const_iterator last = (--access_list_.end());
            const key_type &k                       = *last;
            iter_map_.erase(k);
            data_map_.erase(k);
            access_list_.pop_back();
        }
    }

    // キャッシュからデータを削除する
    void erase(const key_type &key) {
        if (iter_map_.count(key) == 0) {
            return;
        }
        access_list_.erase(iter_map_[key]);
        iter_map_.erase(key);
        data_map_.erase(key);
    }

    // キャッシュが存在するか
    bool exists(const key_type &key) const {
        return iter_map_.count(key) > 0;
    }
};

#endif
