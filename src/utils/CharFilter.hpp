#ifndef CHARFILTER_HPP
# define CHARFILTER_HPP
# include <iostream>
# include <string>
# include <map>
# include "http.hpp"

// 全体で共通して使うenum, 型, 定数, フリー関数など

namespace HTTP {

    // 単純な文字集合クラス
    class CharFilter {
    private:
        unsigned char   filter[32];

    public:
        CharFilter(const byte_string& chars);
        CharFilter(const char* chars);
        CharFilter(const CharFilter& other);
        // by exclusive char-range
        CharFilter(byte_type from, byte_type to);

        CharFilter& operator=(const CharFilter& rhs);
        CharFilter& operator=(const byte_string& rhs);

        // set union
        CharFilter  operator|(const CharFilter& rhs) const;
        // set intersection
        CharFilter  operator&(const CharFilter& rhs) const;
        // set symmetric difference
        CharFilter  operator^(const CharFilter& rhs) const;
        // set complement
        CharFilter  operator~() const;
        CharFilter  operator-(const CharFilter& rhs) const;

        // 文字集合を文字列`chars`を使って初期化する
        void        fill(const byte_string& chars);
        void        fill(byte_type from, byte_type to);
        // `c` が文字集合に含まれるかどうか
        bool        includes(byte_type c) const;
        // 文字集合のサイズ
        byte_string::size_type  size() const;

        // アルファベット・小文字
        static const CharFilter alpha_low;
        // アルファベット・大文字
        static const CharFilter alpha_up;
        // アルファベット
        static const CharFilter alpha;
        // 数字
        static const CharFilter digit;
        // 16進数における数字
        static const CharFilter hexdig;
        // HTTPにおける非予約文字
        static const CharFilter unreserved;
        static const CharFilter gen_delims;
        static const CharFilter sub_delims;
        static const CharFilter tchar;
        static const CharFilter sp;
        static const CharFilter ws;
        static const CharFilter crlf;
        static const CharFilter cr;
        static const CharFilter lf;
        static const CharFilter htab;
        static const CharFilter dquote;
        static const CharFilter bslash;
        static const CharFilter obs_text;
        static const CharFilter vchar;
        static const CharFilter qdtext;
        static const CharFilter quoted_right;

        byte_string str() const;
    };
}

std::ostream&   operator<<(std::ostream& ost, const HTTP::CharFilter& f);

#endif
