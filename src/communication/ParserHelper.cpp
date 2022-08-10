#include "ParserHelper.hpp"
#include <ctime>
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

long ParserHelper::latoi(const light_string &str) {
    long v;
    std::stringstream ss;
    ss << str.str();
    ss >> v;
    return v;
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

const char *day_names[] = {
    "Mon",
    "Tue",
    "Wed",
    "Thu",
    "Fri",
    "Sat",
    "Sun",
    NULL,
};

const char *day_names_l[] = {
    "Monday",
    "Tuesday",
    "Wednesday",
    "Thursday",
    "Friday",
    "Saturday",
    "Sunday",
    NULL,
};

const char *month_names[] = {
    "Jan",
    "Feb",
    "Mar",
    "Apr",
    "May",
    "Jun",
    "Jul",
    "Aug",
    "Sep",
    "Oct",
    "Nov",
    "Dec",
    NULL,
};

// const unsigned int days_of_month[] = {
//     31,
//     28,
//     31,
//     30,
//     31,
//     30,
//     31,
//     31,
//     30,
//     31,
//     30,
//     31,
// };

int find_index(const HTTP::light_string &str, const char **list) {
    for (int i = 0; list[i]; ++i) {
        if (str == list[i]) {
            return i;
        }
    }
    return -1;
}

std::pair<bool, t_time_epoch_ms> ParserHelper::str_to_http_date(const light_string &str) {
    //   HTTP-date    = IMF-fixdate / obs-date
    //   obs-date     = rfc850-date / asctime-date
    //
    //   IMF-fixdate  = day-name "," SP date1 SP time-of-day SP GMT
    //   ; fixed length/zone/capitalization subset of the format
    //   ; see Section 3.3 of [RFC5322]
    //   date1        = day SP month SP year
    //                ; e.g., 02 Jun 1982
    //   day          = 2DIGIT
    //   month        = %s"Jan" / %s"Feb" / %s"Mar" / %s"Apr"
    //                / %s"May" / %s"Jun" / %s"Jul" / %s"Aug"
    //                / %s"Sep" / %s"Oct" / %s"Nov" / %s"Dec"
    //   year         = 4DIGIT
    //   GMT          = %s"GMT"
    //   time-of-day  = hour ":" minute ":" second
    //                ; 00:00:00 - 23:59:60 (leap second)
    //   hour         = 2DIGIT
    //   minute       = 2DIGIT
    //   second       = 2DIGIT
    //
    //   rfc850-date  = day-name-l "," SP date2 SP time-of-day SP GMT
    //   date2        = day "-" month "-" 2DIGIT
    //                ; e.g., 02-Jun-82
    //
    //
    // asctime-date = day-name SP date3 SP time-of-day SP year
    // date3        = month SP ( 2DIGIT / ( SP 1DIGIT ))
    //             ; e.g., Jun  2
    do {
        light_string base                = str;
        const light_string cand_day_name = base.substr_while(HTTP::CharFilter::alpha);
        VOUT(cand_day_name);
        VOUT(find_index(cand_day_name, day_names));
        if (0 <= find_index(cand_day_name, day_names)) {
            //   day-name     = %s"Mon" / %s"Tue" / %s"Wed"
            //                / %s"Thu" / %s"Fri" / %s"Sat" / %s"Sun"
            // IMF-fixdate or asctime?
            base = base.substr(cand_day_name.size());
            QVOUT(base);
            if (base.starts_with(", ")) {
                // IMF-fixdate?
                //   IMF-fixdate  = day-name "," SP date1 SP time-of-day SP GMT
                base                   = base.substr(2);
                const light_string day = base.substr_while(HTTP::CharFilter::digit);
                if (day.size() != 2) {
                    break;
                }
                base = base.substr(day.size());
                if (!base.starts_with(" ")) {
                    break;
                }
                base                     = base.substr(1);
                const light_string month = base.substr(0, 3);
                const int month_index = find_index(month, month_names);
                if (month_index < 0) {
                    break;
                }
                base = base.substr(month.size());
                if (!base.starts_with(" ")) {
                    break;
                }
                base = base.substr(1);
                const light_string years = base.substr_while(HTTP::CharFilter::digit);
                if (years.size() != 4) {
                    break;
                }
                base = base.substr(years.size());
                if (!base.starts_with(" ")) {
                    break;
                }
                base                     = base.substr(1);
                const light_string hours = base.substr_while(HTTP::CharFilter::digit);
                if (hours.size() != 2) {
                    continue;
                }
                base = base.substr(hours.size());
                if (!base.starts_with(":")) {
                    break;
                }
                base                       = base.substr(1);
                const light_string minutes = base.substr_while(HTTP::CharFilter::digit);
                if (minutes.size() != 2) {
                    continue;
                }
                base = base.substr(minutes.size());
                if (!base.starts_with(":")) {
                    break;
                }
                base                       = base.substr(1);
                const light_string seconds = base.substr_while(HTTP::CharFilter::digit);
                if (seconds.size() != 2) {
                    continue;
                }
                base = base.substr(seconds.size());
                if (!base.starts_with(" ")) {
                    break;
                }
                base = base.substr(1);
                if (!base.starts_with("GMT")) {
                    break;
                }
                base = base.substr(3);
                if (base.size() != 0) {
                    break;
                }
                // const unsigned int dd = latoi(day), yy = latoi(years), hh = latoi(hours), mm = latoi(minutes),
                //                    ss = latoi(seconds);
                // // 時
                // if (24 <= hh) {
                //     break;
                // }
                // // 分
                // if (60 <= mm) {
                //     break;
                // }
                // // 秒
                // if (60 <= ss) {
                //     break;
                // }
                // // 日数
                // if (month_index != 1) {
                //     // 2月以外
                //     if (days_of_month[month_index] <= dd) {
                //         break;
                //     }
                // } else {
                //     // 2月
                //     const bool is_leap_year = yy % 400 == 0 || (yy % 4 == 0 && yy % 100 != 0);
                //     if (is_leap_year && 29 < dd) {
                //         break;
                //     }
                //     if (!is_leap_year && 28 < dd) {
                //         break;
                //     }
                // }
                struct tm tmv                   = {};
                const HTTP::char_string charstr = HTTP::restrfy(str.str());
                char *rv                        = strptime(charstr.c_str(), "%a, %d %b %Y %H:%M:%S %Z", &tmv);
                bool failed                     = !(rv && *rv == '\0');
                if (failed) {
                    break;
                }
                t_time_epoch_ms ms = mktime(&tmv) * 1000;
                VOUT(ms);
                return std::make_pair(true, ms);
            } else if (base.starts_with(" ")) {
                // asctime?
                // asctime-date = day-name SP date3 SP time-of-day SP year
                // date3        = month SP ( 2DIGIT / ( SP 1DIGIT ))
                // time-of-day  = hour ":" minute ":" second
                //              ; 00:00:00 - 23:59:60 (leap second)

                // asctime形式の文字列にはタイムゾーンがないので, UTCを補う必要がある.
                // ... が, strptime は "UTC" をタイムゾーンとして解釈しないので, GMT で代用する.
                const HTTP::char_string tz_supplied = HTTP::restrfy(str.str() + " GMT");
                struct tm tmv = {};

                // void *rv    = strptime("Sun Nov  6 08:49:37 1994", "%a %b %e %H:%M:%S %Y", &tmv);
                char *rv    = strptime(tz_supplied.c_str(), "%a %b %e %H:%M:%S %Y %Z", &tmv);
                bool failed = !(rv && *rv == '\0');
                if (failed) {
                    break;
                }
                t_time_epoch_ms ms = mktime(&tmv) * 1000;
                return std::make_pair(true, ms);
            }
        } else if (0 <= find_index(cand_day_name, day_names_l)) {
            //   day-name-l   = %s"Monday" / %s"Tuesday" / %s"Wednesday"
            //                / %s"Thursday" / %s"Friday" / %s"Saturday"
            //                / %s"Sunday"
            // rfc850-date?
        }
    } while (0);
    return std::make_pair(false, 0);
}
