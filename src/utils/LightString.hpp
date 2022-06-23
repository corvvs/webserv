#ifndef LIGHTSTRING_HPP
# define LIGHTSTRING_HPP
# include <string>
# include <algorithm>
# include "CharFilter.hpp"
# include "IndexRange.hpp"

const std::string blank_str = "";

// 別のstringの一部分をiteratorペアとして参照する軽量string
// C++17以降にある string_view と思えば良いか
template <class T>
class LightString {
public:

    typedef T                                       element;
    typedef std::basic_string<T>                    string_class;
    typedef HTTP::CharFilter                        filter_type;
    typedef typename string_class::iterator         iterator;
    typedef typename string_class::const_iterator   const_iterator;
    typedef typename string_class::size_type        size_type;
    static const typename string_class::size_type   npos = string_class::npos;
    typedef typename string_class::reference        reference;
    typedef typename string_class::const_reference  const_reference;

private:

    // 参照先string
    const string_class& base;
    // `base`のこのオブジェクトが参照している部分の最初のインデックス
    // first <= last が成り立つ
    size_type           first;
    // `base`のこのオブジェクトが参照している部分の最後のインデックス + 1
    // first <= last が成り立つ
    // last <= base.size() が成り立つ
    size_type           last;

public:

    LightString(): base(blank_str) {
        first = last;
    }

    LightString(const string_class& str):
        base(str),
        first(0),
        last(str.length()) {}

    LightString(const string_class& str, const_iterator f, const_iterator l):
        base(str),
        first(std::distance(str.begin(), f)),
        last(std::max(first, std::min(str.size(), (size_type)std::distance(str.begin(), l)))) {}

    LightString(const string_class& str, size_type fi, size_type li = npos):
        base(str),
        first(fi),
        last(std::max(first, std::min(str.size(), li))) {
            DXOUT(first << ":" << last);
        }

    LightString(const string_class& str, const IndexRange& range):
        base(str),
        first(range.first),
        last(std::max(first, std::min(str.size(), range.second))) {}

    LightString(const LightString& lstr, size_type fi, size_type li = npos):
        base(lstr.base),
        first(lstr.first + fi),
        last(std::max(first, lstr.first + std::min(lstr.size(), li))) {}

    LightString& operator=(const LightString& rhs) {
        const_cast<string_class&>(base) = rhs.base;
        first = rhs.first;
        last = rhs.last;
        return *this;
    }

    LightString& operator=(const string_class& rhs) {
        const_cast<string_class&>(base) = rhs;
        first = 0;
        last = rhs.length();
        return *this;
    }

    // 参照先文字列を取得
    const string_class& get_base() const {
        return base;
    }

    // std::string を生成
    string_class    str() const {
        if (first == last) { return ""; }
        return string_class(base.begin() + first, base.begin() + last);
    }

    size_type       size() const {
        return last - first;
    }

    size_type       length() const {
        return last - first;
    }

    iterator        begin() {
        return base.begin() + first;
    }

    const_iterator  begin() const {
        return base.begin() + first;
    }

    iterator        end() {
        return base.begin() + last;
    }

    const_iterator  end() const {
        return base.begin() + last;
    }

    const_reference operator[](size_type pos) const {
        return base[first + pos];
    }

    reference       operator[](size_type pos) {
        return base[first + pos];
    }

    element         cat(size_type pos) {
        return base[first + pos];
    }

    // `str` に含まれる文字が(LightString内で)最初に出現する位置を返す
    // `pos`が指定された場合, 位置`pos`以降のみを検索する
    // 位置は参照先文字列ではなく LightString 先頭からの相対位置
    size_type       find_first_of(const filter_type& filter, size_type pos = 0) const {
        size_type d = length();
        if (pos >= d) {
            return npos;
        }
        for (typename string_class::size_type i = pos; i < size(); ++i) {
            if (filter.includes(operator[](i))) {
                return i;
            }
        }
        return npos;
    }

    // `str` に含まれる文字が(LightString内で)最後に出現する位置を返す
    // `pos`が指定された場合, 位置`pos`以降のみを検索する
    // 位置は参照先文字列ではなく LightString 先頭からの相対位置
    size_type       find_last_of(const filter_type& filter, size_type pos = 0) const {
        size_type d = length();
        if (pos >= d) {
            return npos;
        }
        for (typename string_class::size_type i = last; first + pos < i;) {
            --i;
            if (filter.includes(base[i])) {
                return i - first;
            }
        }
        return npos;
    }

    // `str` に含まれない文字が(LightString内で)最初に出現する位置を返す
    // `pos`が指定された場合, 位置`pos`以降のみを検索する
    // 位置は参照先文字列ではなく LightString 先頭からの相対位置
    size_type       find_first_not_of(const filter_type& filter, size_type pos = 0) const {
        size_type d = length();
        if (pos >= d) {
            return npos;
        }
        if (first == last) {
            return npos;
        }
        for (size_type i = pos; i < size(); ++i) {
            if (!filter.includes(operator[](i))) {
                return i;
            }
        }
        return npos;
    }

    // `str` に含まれる文字が(LightString内で)最後に出現する位置を返す
    // `pos`が指定された場合, 位置`pos`以降のみを検索する
    // 位置は参照先文字列ではなく LightString 先頭からの相対位置
    size_type       find_last_not_of(const filter_type& filter, size_type pos = 0) const {
        size_type d = length();
        if (pos >= d) {
            return npos;
        }
        if (first == last) {
            return npos;
        }
        for (size_type i = size(); 0 + pos < i;) {
            --i;
            if (!filter.includes(operator[](i))) {
                return i;
            }
        }
        return npos;
    }

    // `str`が最初に出現する位置(= 先頭文字のインデックス)を返す
    // `pos`が指定された場合, 位置`pos`以降のみを検索する
    // 位置は参照先文字列ではなく LightString 先頭からの相対位置
    size_type       find(const string_class& str, size_type pos = 0) const {
        for (size_type i = pos; i + str.size() <= size(); ++i) {
            size_type j = 0;
            for (; j < str.size() && operator[](i + j) == str[j]; ++j);
            if (j == str.size()) {
                return i;
            }
        }
        return npos;
    }

    // `str`が最後に出現する位置(= 先頭文字のインデックス)を返す
    // `pos`が指定された場合, 位置`pos`以降のみを検索する
    // 位置は参照先文字列ではなく LightString 先頭からの相対位置
    size_type       rfind(const string_class& str, size_type pos = 0) const {
        for (size_type i = size() - str.size(); pos <= i; --i) {
            size_type j = 0;
            for (; j < str.size() && operator[](i + j) == str[j]; ++j);
            if (j == str.size()) {
                return i;
            }
            if (pos == i) { break; }
        }
        return npos;
    }

    // 指定した位置`pos`から始まる(最大)長さ`n`の区間を参照する LightString を生成して返す
    // `n`を指定しなかった場合, 新たな LightString の終端は現在の終端と一致する
    // 位置は参照先文字列ではなく LightString 先頭からの相対位置
    LightString substr(size_type pos = 0, size_type n = std::string::npos) const {
        if (pos == std::string::npos) {
            return LightString(*this, size(), size());
        }
        if (n == std::string::npos) {
            return LightString(*this, pos, size());
        }
        size_type   rlen = size() - pos;
        if (n < rlen) { // pos + n < size()
            rlen = n;
        }
        return LightString(*this, pos, pos + rlen);
    }

    // 「`pos`以降で最初に`filter`にマッチしなくなる位置」の直前までを参照する LightString を生成して返す
    // `pos`以前の部分も含まれることに注意.
    // ※ substr(0, find_first_not_of(filter, pos)) と等価
    LightString substr_while(const filter_type& filter, size_type pos = 0) const {
        size_type n = find_first_not_of(filter, pos);
        return substr(0, n);
    }

    // 「`pos`以降で最初に`filter`にマッチしなくなる位置」から後の部分参照する LightString を生成して返す
    // ※ substr(find_first_not_of(filter, pos)) と等価
    LightString substr_after(const filter_type& filter, size_type pos = 0) const {
        size_type n = find_first_not_of(filter, pos);
        return substr(n);
    }

    // 「`pos`以降で最初に`filter`にマッチする位置」の直前までを参照する LightString を生成して返す
    // `pos`以前の部分も含まれることに注意.
    // ※ substr(0, find_first_of(filter, pos)) と等価
    LightString substr_before(const filter_type& filter, size_type pos = 0) const {
        size_type n = find_first_of(filter, pos);
        return substr(0, n);
    }

    // 「`pos`以降で最初に`filter`にマッチする位置」から後の部分参照する LightString を生成して返す
    // ※ substr(find_first_of(filter, pos)) と等価
    LightString substr_from(const filter_type& filter, size_type pos = 0) const {
        size_type n = find_first_of(filter, pos);
        return substr(n);
    }

    std::vector<LightString>    split(const filter_type& filter) const {
        std::vector<LightString>    rv;
        size_type word_from = 0;
        size_type word_to = 0;
        bool prev_is_sp = true;
        for (size_type i = 0; i <= size(); ++i) {
            if (i == size() || filter.includes(operator[](i))) {
                word_to = i;
                if (prev_is_sp) {
                    word_from = i;
                }
                // DXOUT("substr(" << word_from << ", " << word_to - word_from << ")");
                rv.push_back(substr(word_from, word_to - word_from));
                prev_is_sp = true;
            } else {
                if (prev_is_sp) {
                    word_from = i;
                }
                prev_is_sp = false;
            }
        }
        return rv;
    }

    // 文字集合`fil`内の文字を左側について切り落とした新たなLightStringを返す
    LightString ltrim(const filter_type& fil) const {
        size_type first = find_first_not_of(fil);
        if (first == npos) {
            return LightString(*this, size(), size());
        } else {
            return LightString(*this, first, size());
        }
    }

    // 文字集合`fil`内の文字を右側について切り落とした新たなLightStringを返す
    LightString rtrim(const filter_type& fil) const {
        size_type last = find_last_not_of(fil);
        if (last == npos) {
            return LightString(*this, 0, 0);
        } else {
            return LightString(*this, 0, last + 1);
        }
    }

    // 文字集合`fil`内の文字を左右から切り落とした新たなLightStringを返す
    LightString trim(const filter_type& fil) const {
        return ltrim(fil).rtrim(fil);
    }
};

namespace HTTP {
    typedef LightString<char_type>  light_string;
}

template <class T>
std::ostream&   operator<<(std::ostream& out, const LightString<T>& ls) {
    return out << typename LightString<T>::string_class(ls.begin(), ls.end());
}

#endif
