#ifndef LRUCACHE_HPP
#define LRUCACHE_HPP

#include <iostream>
#include <list>
#include <map>

template <class Key, class Value>
class LRUCache {
public:
    typedef Key key_type;
    typedef Value value_type;

    typedef std::list<std::pair<key_type, value_type> > list_type;
    typedef typename list_type::iterator iterator;
    typedef typename list_type::const_iterator const_iterator;

    // key と listのイテレータをvalueにもつmap
    typedef std::map<key_type, typename list_type::const_iterator> iterator_map_type;

private:
    // キャッシュできる上限値
    long capacity_;

    // アクセス順に並んでいるリスト
    list_type access_list_;

    // key と listのイテレータをvalueにもつmap
    iterator_map_type iter_map_;

public:
    LRUCache(long capacity) : capacity_(capacity) {}

    const_iterator begin() const {
        return access_list_.begin();
    }

    const_iterator end() const {
        return access_list_.end();
    }

    // キーが存在する場合はキーの値を返す
    // ない場合は-1を返す
    const_iterator fetch(const key_type &key) {
        if (iter_map_.count(key) == 0) {
            std::cout << "fetch: \"" << key << "\" -> "
                      << "fail" << std::endl;
            return end();
        }

        // 存在する場合
        const_iterator it   = iter_map_[key];
        const key_type &k   = it->first;
        const value_type &v = it->second;

        // アクセスリストから削除
        access_list_.erase(it);
        // 新たにアクセスリストに追加する
        access_list_.push_front(std::make_pair(k, v));
        // イテレータを更新する
        iter_map_[key] = access_list_.begin();
        std::cout << "fetch: \"" << k << "\" -> " << v << std::endl;
        return iter_map_[key];
    }

    // キーが存在する場合は更新する
    // ない場合はキャッシュに追加する
    // capacityを超える場合は使用頻度が最も低いcacheを削除する
    void add(const key_type &key, const value_type &value) {
        // 存在した場合は既存のアクセスリストから削除してから更新する
        if (iter_map_.count(key) != 0) {
            access_list_.erase(iter_map_[key]);
        }
        std::cout << "add: \"" << key << "\" -> " << value << std::endl;

        // キャッシュを追加する
        access_list_.push_front(std::make_pair(key, value));
        iter_map_[key] = access_list_.begin();

        // キャシュの上限を超えている場合
        if (access_list_.size() > capacity_) {
            // capacityを超える場合は使用頻度が最も低いcacheを削除する
            const_iterator last = (--access_list_.end());
            const key_type &k   = last->first;
            iter_map_.erase(k);
            access_list_.pop_back();
        }
    }
};

#endif
