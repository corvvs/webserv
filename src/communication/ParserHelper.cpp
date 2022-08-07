#include "ParserHelper.hpp"
#include <iomanip>
#include <limits>

const ParserHelper::byte_string ParserHelper::SP                 = HTTP::strfy(" ");
const ParserHelper::byte_string ParserHelper::OWS                = HTTP::strfy(" \t");
const ParserHelper::byte_string ParserHelper::HEADER_KV_SPLITTER = HTTP::strfy(":");
const ParserHelper::byte_string ParserHelper::CRLF               = HTTP::strfy("\r\n");
const ParserHelper::byte_string ParserHelper::LF                 = HTTP::strfy("\n");
const ParserHelper::byte_string ParserHelper::LWS                = HTTP::strfy(" \t");

IndexRange ParserHelper::find_crlf(const byte_string &str, ssize_t from, ssize_t len) {
    if ((size_t)(from + len) > str.size()) {
        len = str.size() - from;
    }

    for (ssize_t i = from; i - from < len; i++) {
        // iは絶対インデックス; strの先頭からの位置
        // DSOUT() << from << ", " << i << ", " << len << ": " << str[i] << "-" << int(str[i]) << std::endl;
        if (str[i] == '\n') {
            if (0 < i && str[i - 1] == '\r') {
                return IndexRange(i - 1, i + 1);
            }
            return IndexRange(i, i + 1);
        }
    }
    return IndexRange(from + len + 1, from + len);
}

IndexRange ParserHelper::find_crlf_header_value(const byte_string &str, ssize_t from, ssize_t len) {
    // DSOUT() << "target: \"" << byte_string(str.begin() + from, str.begin() + from + len) << "\"" << std::endl;
    ssize_t movement = 0;
    while (true) {
        ssize_t rfrom = from + movement;
        ssize_t rlen  = len - movement;
        // DSOUT() << "finding from: " << rfrom << ", len: " << rlen << std::endl;
        IndexRange r = find_crlf(str, rfrom, rlen);
        if (!r.is_invalid()) {
            // is obs-fold ?
            // DSOUT() << "is obs-fold?? " << r << std::endl;
            if (r.second < from + len && is_sp(str[r.second])) {
                DXOUT("is obs-fold!!");
                movement = r.second - from;
                continue;
            }
            return r;
        }
        break;
    }
    return IndexRange(from + len + 1, from + len);
}

IndexRange ParserHelper::find_crlf_header_value(const light_string &str) {
    IndexRange res = find_crlf_header_value(str.get_base(), str.get_first(), str.length());
    if (res.is_invalid()) {
        return res;
    }
    return IndexRange(res.first - str.get_first(), res.second - str.get_first());
}

IndexRange ParserHelper::find_obs_fold(const byte_string &str, ssize_t from, ssize_t len) {
    ssize_t movement = 0;
    while (true) {
        ssize_t rfrom = from + movement;
        ssize_t rlen  = len - movement;
        IndexRange r  = find_crlf(str, rfrom, rlen);
        if (!r.is_invalid()) {
            ssize_t i;
            for (i = 0; r.second + i < from + len && is_sp(str[r.second + i]); ++i) {}
            DXOUT(r << " i: " << i);
            if (i < 1) {
                // from + movement = r.second
                movement = r.second - from;
                continue;
            }
            // obs-fold!!
            return IndexRange(r.first, r.second + i);
        }
        return r;
    }
}

ssize_t ParserHelper::ignore_crlf(const byte_string &str, ssize_t from, ssize_t len) {
    ssize_t i = 0;
    for (; i < len; i++) {
        // DOUT() << i << ": " << str[i + from] << std::endl;
        if (str[i + from] == '\n') {
            continue;
        } else if (str[i + from] == '\r') {
            if (i + 1 < len && str[i + from] == '\n') {
                continue;
            }
        }
        break;
    }
    // DOUT() << "result: " << i << std::endl;
    return i;
}

bool ParserHelper::is_sp(char c) {
    return std::find(SP.begin(), SP.end(), c) != SP.end();
}

ssize_t ParserHelper::ignore_sp(const byte_string &str, ssize_t from, ssize_t len) {
    ssize_t i = 0;
    for (; i < len; i++) {
        if (is_sp(str[i + from])) {
            continue;
        }
        break;
    }
    return i;
}

ssize_t ParserHelper::ignore_not_sp(const byte_string &str, ssize_t from, ssize_t len) {
    ssize_t i = 0;
    for (; i < len; i++) {
        if (!is_sp(str[i + from])) {
            continue;
        }
        break;
    }
    return i;
}

IndexRange ParserHelper::find_leading_crlf(const byte_string &str, ssize_t from, ssize_t len, bool is_terminated) {
    if (len >= 2 && str[from] == '\r' && str[from + 1] == '\n') {
        // CRLF
        return IndexRange(from, from + 2);
    } else if (len >= 1 && str[from] == '\n') {
        // LF
        return IndexRange(from, from + 1);
    } else if (len >= 2 && str[from] == '\r') {
        // CR
        return IndexRange(from, from + 1);
    } else if (len == 1 && str[from] == '\r' && is_terminated) {
        // CR
        return IndexRange(from, from + 1);
    } else if (len == 1 && str[from] == '\r') {
        // CRだがCRLFかもしれない
        return IndexRange(from, from);
    } else if (len == 0 && !is_terminated) {
        // CR, LF, CRLFかもしれない
        return IndexRange(from, from);
    } else {
        // 確実にマッチしない
        return IndexRange(from, from - 1);
    }
}

std::vector<ParserHelper::byte_string> ParserHelper::split_by_sp(ParserHelper::byte_string::const_iterator first,
                                                                 ParserHelper::byte_string::const_iterator last) {
    typedef ParserHelper::byte_string str_type;
    std::vector<str_type> rv;
    str_type::size_type len       = std::distance(first, last);
    str_type::size_type i         = 0;
    str_type::size_type word_from = 0;
    bool prev_is_sp               = true;
    for (; i <= len; i++) {
        bool cur_is_sp = i == len || is_sp(*(first + i));
        if (cur_is_sp && !prev_is_sp) {
            rv.push_back(str_type(first + word_from, first + i));
        } else if (!cur_is_sp && prev_is_sp) {
            word_from = i;
        }
        prev_is_sp = cur_is_sp;
    }
    return rv;
}

std::vector<ParserHelper::light_string> ParserHelper::split_by_sp(const light_string &str) {
    std::vector<light_string> rv;
    light_string::size_type len       = str.length();
    light_string::size_type i         = 0;
    light_string::size_type word_from = 0;
    bool prev_is_sp                   = true;
    for (; i <= len; i++) {
        bool cur_is_sp = i == len || is_sp(str[i]);
        if (cur_is_sp && !prev_is_sp) {
            rv.push_back(str.substr(word_from, i - word_from));
        } else if (!cur_is_sp && prev_is_sp) {
            word_from = i;
        }
        prev_is_sp = cur_is_sp;
    }
    return rv;
}

std::vector<HTTP::light_string> ParserHelper::split(const HTTP::light_string &lstr, const char *charset) {
    return lstr.split(charset);
}

std::vector<HTTP::light_string> ParserHelper::split(const HTTP::light_string &lstr, const byte_string &charset) {
    return lstr.split(charset);
}

ParserHelper::byte_string ParserHelper::normalize_header_key(const byte_string &key) {
    return HTTP::Utils::downcase(key);
}

ParserHelper::byte_string ParserHelper::normalize_header_key(const HTTP::light_string &key) {
    return normalize_header_key(key.str());
}

std::pair<bool, unsigned int> ParserHelper::xtou(const HTTP::light_string &str) {
    unsigned int n             = 0;
    const HTTP::byte_string xs = HTTP::strfy("0123456789abcdef");
    unsigned int max_val       = std::numeric_limits<unsigned int>::max();
    for (HTTP::light_string::size_type i = 0; i < str.size(); ++i) {
        char c                               = str[i];
        HTTP::byte_string::const_iterator it = std::find(xs.begin(), xs.end(), (HTTP::char_type)tolower(c));
        HTTP::byte_string::size_type j       = it != xs.end() ? std::distance(xs.begin(), it) : HTTP::npos;
        if (j == HTTP::npos) {
            return std::pair<bool, unsigned int>(false, n);
        }
        // n * xs.size() + j > std::numeric_limits<int>::max();
        if (n > max_val / xs.size() || j > max_val - n * xs.size()) {
            return std::pair<bool, unsigned int>(false, n);
        }
        n = n * xs.size() + j;
    }
    return std::pair<bool, unsigned int>(true, n);
}

ParserHelper::byte_string ParserHelper::utos(unsigned int u, unsigned int base) {
    // std::setbase の引数は 8, 10, 16のみ
    assert(base == 8 || base == 10 || base == 16);
    std::stringstream ss;
    if (base != 10) {
        ss << std::setbase(base);
    }
    ss << u;
    return HTTP::strfy(ss.str());
}

std::pair<bool, unsigned int> ParserHelper::str_to_u(const byte_string &str) {
    std::stringstream ss;
    std::string rr;
    unsigned int v;

    ss << str;
    ss >> v;
    ss.clear();
    ss << v;
    ss >> rr;
    byte_string r(HTTP::strfy(rr));
    if (str == r) {
        return std::make_pair(true, v);
    } else {
        return std::make_pair(false, 0);
    }
}

std::pair<bool, unsigned int> ParserHelper::str_to_u(const HTTP::light_string &str) {
    return str_to_u(str.str());
}

unsigned int ParserHelper::quality_to_u(HTTP::light_string &quality) {
    unsigned int v = 0;
    unsigned int j = 0;
    for (int i = 0; i < 4;) {
        if (j < quality.size()) {
            if (isdigit(quality[j])) {
                v = v * 10 + quality[j] - '0';
                ++i;
            }
            ++j;
        } else {
            v = v * 10;
            ++i;
        }
    }
    return v;
}

ParserHelper::light_string ParserHelper::extract_quoted_or_token(const light_string &str) {
    if (str.size() == 0) {
        return light_string(str);
    }
    //                  [key]            [value]
    // parameter      = token "=" ( token / quoted-string )
    if (HTTP::CharFilter::dquote.includes(str[0])) {
        // quoted-string  = DQUOTE *( qdtext / quoted-pair ) DQUOTE
        // qdtext         = HTAB / SP / %x21 / %x23-5B / %x5D-7E / obs-text
        //                ;               !     #-[        ]-~
        //                ; HTAB + 表示可能文字, ただし "(ダブルクオート) と \(バッスラ) を除く
        // obs-text       = %x80-FF ; extended ASCII
        // quoted-pair    = "\" ( HTAB / SP / VCHAR / obs-text )
        light_string::size_type i = 1;
        bool quoted               = false;
        for (; i < str.size(); ++i) {
            const HTTP::byte_type c = str[i];
            if (!quoted && HTTP::CharFilter::dquote.includes(c)) {
                // クオートされてないダブルクオート
                // -> ここで終わり
                break;
            }
            if (quoted && !HTTP::CharFilter::qdright.includes(c)) {
                // クオートの右がquoted_rightではない
                // -> 不適格
                DXOUT("[KO] not suitable for quote");
                return str.substr(str.size());
            }
            if (!quoted && HTTP::CharFilter::bslash.includes(c)) {
                // クオートされてないバックスラッシュ
                // -> クオートフラグ立てる
                quoted = true;
            } else {
                quoted = false;
            }
        }
        // DXOUT("quoted-string: \"" << light_string(value_str, 0, i + 1) << "\"");
        return str.substr(0, i + 1);
    } else {
        // token          = 1*tchar
        return str.substr_while(HTTP::CharFilter::tchar);
    }
}
