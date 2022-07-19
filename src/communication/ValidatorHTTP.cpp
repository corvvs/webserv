#include "ValidatorHTTP.hpp"
#include "ParserHelper.hpp"

bool HTTP::Validator::is_valid_header_host(const light_string &str) {
    // host = uri-host [ ":" port ]

    byte_string::size_type ket = str.find_last_of("]");
    byte_string::size_type sep = str.find_last_of(":", ket == npos ? 0 : ket);
    if (sep != npos && 0 < sep && str[sep - 1] != ':') {
        // ":"がある -> portとしての妥当性チェック
        if (!is_port(str.substr(sep + 1))) {
            DXOUT("non digit char in port part");
            // throw http_error("invalid char in port part of host", HTTP::STATUS_BAD_REQUEST);
            return false;
        }
    } else {
        // ":" がない
        sep = str.size();
    }
    // この時点で, 区間 [0,sep) が uri-host となっているはず
    // uri-host の妥当性を調べる
    light_string uri_host(str, 0, sep);
    return is_uri_host(uri_host);
}

bool HTTP::Validator::is_uri_host(const light_string &str) {
    return is_ip_literal(str) || is_ipv4address(str) || is_reg_name(str);
}

bool HTTP::Validator::is_ip_literal(const light_string &str) {
    // IP-literal    = "[" ( IPv6address / IPvFuture  ) "]"
    if (str.size() < 2) {
        return false;
    }
    if (str[0] != '[') {
        return false;
    }
    if (str[str.size() - 1] != ']') {
        return false;
    }
    const light_string inner = str.substr(1, str.size() - 2);
    // DXOUT("inner: " << inner);
    return is_ipv6address(inner) || is_ipvfuture(inner);
}

bool HTTP::Validator::is_ipv6address(const light_string &str) {
    // IPv6address   =                            6( h16 ":" ) ls32
    //               /                       "::" 5( h16 ":" ) ls32
    //               / [ *0( h16 ":" ) h16 ] "::" 4( h16 ":" ) ls32
    //               / [ *1( h16 ":" ) h16 ] "::" 3( h16 ":" ) ls32
    //               / [ *2( h16 ":" ) h16 ] "::" 2( h16 ":" ) ls32
    //               / [ *3( h16 ":" ) h16 ] "::" 1( h16 ":" ) ls32
    //               / [ *4( h16 ":" ) h16 ] "::" 0( h16 ":" ) ls32
    //               / [ *5( h16 ":" ) h16 ] "::"    h16
    //               / [ *6( h16 ":" ) h16 ] "::"
    // h16           = 1*4HEXDIG; 1~4桁の16進数
    // ls32          = ( h16 ":" h16 ) / IPv4address
    // "::" は /:(0:)+/の略記. ただし "::" が登場できるのは1回まで(復元できなくなる)
    // IPv6アドレスは128ビット = 16オクテット.
    // h16は2オクテット, ls32は4オクテットあるので, ls32はh16にして2個分の価値を持つ.

    // 1. "::" が高々1度しか出現しないことを確認
    //   - "::"がある場合, 2個以上の要素が省略されている
    //   - "::"がない場合, 省略された要素はない -> h16とls32あわせて16オクテットがすべて顕になっているはず
    light_string::size_type first_coroncoron = str.find("::");
    light_string::size_type last_coroncoron  = str.rfind("::");
    // DXOUT(first_coroncoron << " - " << last_coroncoron);
    if (first_coroncoron != last_coroncoron) {
        DXOUT("[KO] multiple \"::\"");
        return false;
    }
    // 2. ":" で(空文字列ありで)分割
    std::vector<light_string> splitted = ParserHelper::split(str, ":");
    // DXOUT("splitted.size() = " << splitted.size());
    if (splitted.size() < 1 || 8 < splitted.size()) {
        return false;
    }
    // 3. 最後以外の要素が空文字列かh16であることを確認
    int octets = 0;
    for (unsigned int i = 0; i < splitted.size() - 1; ++i) {
        if (splitted[i].size() > 0) {
            if (!is_h16(splitted[i])) {
                // DXOUT("it's not a h16: " << splitted[i]);
                return false;
            }
            octets += 2;
        }
    }
    // 4. 最後の要素が空文字列かh16かどうか確認
    //   - 空文字列である
    //     - "::"がある
    //       - h16の数が0個以上6個以下であることを確認
    //     - "::"がない
    //       - NG
    //   - h16である
    //     - "::"がある
    //       - h16の数が1個以上6個以下であることを確認
    //     - "::"がない
    //       - h16の数が8個であることを確認
    //   - h16ではない
    //     - 最後の要素がIPv4アドレスとしてvalidかどうか確かめる -> invalidならNG
    //     - "::"がある
    //       - h16の数が1個以上5個以下であることを確認
    //     - "::"がない
    //       - h16の数が6個であることを確認
    if (is_ls32(splitted.back())) {
        octets += 4;
    } else if (is_h16(splitted.back())) {
        octets += 2;
    } else {
        // DXOUT("it's not a ls32 or h16: " << splitted[i]);
        return false;
    }
    // DXOUT("octets: " << octets);
    return octets <= 16;
}

bool HTTP::Validator::is_ipvfuture(const light_string &str) {
    // IPvFuture     = "v" 1*HEXDIG "." 1*( unreserved / sub-delims / ":" )
    if (str.size() < 3) {
        return false;
    }
    if (str[0] != 'v') {
        return false;
    }
    light_string::size_type i = 1;
    for (; i < str.size(); ++i) {
        if (!CharFilter::hexdig.includes(str[i])) {
            break;
        }
    }
    if (i < 2) {
        return false;
    }
    const light_string::size_type ihex = i;
    CharFilter filter_others           = CharFilter::unreserved | CharFilter::sub_delims | ":";
    for (; i < str.size(); ++i) {
        if (!filter_others.includes(str[i])) {
            break;
        }
    }
    if (i <= ihex) {
        return false;
    }
    if (i != str.size()) {
        return false;
    }
    return true;
}

bool HTTP::Validator::is_ipv4address(const HTTP::light_string &str) {
    // IPv4address   = dec-octet "." dec-octet "." dec-octet "." dec-octet
    std::vector<light_string> splitted = ParserHelper::split(str, ".");
    if (splitted.size() != 4) {
        // [NG] dotが３つではない
        DXOUT("[KO] splitted size is unexpected: " << splitted.size());
        return false;
    }
    for (unsigned int i = 0; i < splitted.size(); ++i) {
        const light_string &spl = splitted[i];
        if (spl.length() == 0 || 3 < spl.length()) {
            // [NG] 3文字以上または0文字である
            DXOUT("[KO] invalid length for ipv4 addr component: " << spl);
            return false;
        }
        light_string::size_type res = spl.find_first_not_of(CharFilter::digit);
        if (res != light_string::npos) {
            // [NG] 数字でない文字がある
            DXOUT("[KO] non digit char in ipv4 addr component: " << spl);
            return false;
        }
        if (spl[0] == '0') {
            // [NG] leading zeroがある
            DXOUT("[KO] detectec leading zero in ipv4 addr component: " << spl);
            return false;
        }
        unsigned int elem = ParserHelper::stou(spl);
        if (255 < elem) {
            // [NG] 255よりでかい
            DXOUT("[KO] too large ipv4 addr component: " << spl);
            return false;
        }
    }
    return true;
}

bool HTTP::Validator::is_reg_name(const light_string &str) {
    // reg-name      = *( unreserved / pct-encoded / sub-delims )
    // unreserved    = ALPHA / DIGIT / "-" / "." / "_" / "~"
    // pct-encoded   = "%" HEXDIG HEXDIG
    // sub-delims    = "!" / "$" / "&" / "'" / "(" / ")"
    //               / "*" / "+" / "," / ";" / "="
    const CharFilter &hexdig       = CharFilter::hexdig;
    const CharFilter filter_others = CharFilter::unreserved | CharFilter::sub_delims | ":";
    for (light_string::size_type i = 0; i < str.length();) {
        if (filter_others.includes(str[i])) {
            // unreserved or sub-delims
            i += 1;
            continue;
        } else if (str[i] == '%') {
            if (i + 2 < str.length() && hexdig.includes(str[i + 1]) && hexdig.includes(str[i + 2])) {
                i += 3;
                continue;
            }
        }
        // DXOUT("unexpected char detected: '" << str[i] << "'");
        return false;
    }
    return true;
}

bool HTTP::Validator::is_port(const light_string &str) {
    // port          = *DIGIT
    return str.find_first_not_of(CharFilter::digit) == light_string::npos;
}

bool HTTP::Validator::is_h16(const light_string &str) {
    // h16           = 1*4HEXDIG; 1~4桁の16進数
    if (str.size() < 1) {
        return false;
    }
    if (str.size() > 4) {
        return false;
    }
    return str.find_first_not_of(CharFilter::hexdig) == light_string::npos;
}

bool HTTP::Validator::is_ls32(const light_string &str) {
    // ls32          = ( h16 ":" h16 ) / IPv4address
    if (is_ipv4address(str)) {
        return true;
    }
    light_string::size_type sep = str.find_first_of(":");
    if (sep == light_string::npos) {
        return false;
    }
    return is_h16(str.substr(0, sep)) && is_h16(str.substr(sep + 1));
}

bool HTTP::Validator::is_uri_authority(const HTTP::light_string &authority) {
    const light_string tmp      = authority.substr_before("@");
    const light_string userinfo = tmp.size() < authority.size() ? tmp : HTTP::strfy("");
    QVOUT(userinfo);
    {
        // validate `userinfo`
        // userinfo = *( unreserved / pct-encoded / sub-delims / ":" )
        const HTTP::CharFilter ftr_userinfo = HTTP::CharFilter::unreserved | HTTP::CharFilter::sub_delims | ":";
        if (!HTTP::Validator::is_segment(userinfo, ftr_userinfo)) {
            return false;
        }
        DXOUT("userinfo is valid");
    }
    // validate `host` and `port`
    const light_string host_port = 0 < userinfo.size() ? authority.substr(userinfo.size() + 1) : authority;
    QVOUT(host_port);
    if (!HTTP::Validator::is_valid_header_host(host_port)) {
        return false;
    }
    return true;
}

bool HTTP::Validator::is_uri_path(const HTTP::light_string &str, const HTTP::CharFilter &segment_filter) {
    light_string tmp = str.size() > 0 && str[0] == '/' ? str.substr(1) : str;
    for (; tmp.size() > 0;) {
        QVOUT(tmp);
        if (tmp[0] == '/') {
            tmp = tmp.substr(1);
            continue;
        }
        const light_string segment = tmp.substr_before("/");
        if (is_segment(segment, segment_filter)) {
            tmp = tmp.substr(segment.size());
            continue;
        }
        return false;
    }
    return true;
}

bool HTTP::Validator::is_segment(const HTTP::light_string &str, const HTTP::CharFilter &segment_filter) {
    light_string tmp = str;
    for (; tmp.size() > 0;) {
        QVOUT(tmp);
        if (tmp[0] == '%') {
            if (tmp.size() >= 3 && HTTP::CharFilter::hexdig.includes(tmp[1])
                && HTTP::CharFilter::hexdig.includes(tmp[2])) {
                tmp = tmp.substr(3);
                continue;
            }
        } else if (segment_filter.includes(tmp[0])) {
            tmp = tmp.substr(1);
            continue;
        }
        return false;
    }
    return true;
}

bool HTTP::Validator::is_valid_rank(const light_string &str) {
    const light_string integral = str.substr_before(".");
    if (integral.size() != 1) {
        // 整数部なし
        DXOUT("[KO] no integral: " << integral);
        return false;
    }
    if (!isdigit(integral[0])) {
        // 整数部に数字以外がある
        DXOUT("[KO] non-digit in integral: " << integral);
        return false;
    }
    const light_string fraction = str.substr_after(".", integral.size());
    if (fraction.size() > 3) {
        // 小数部が長い
        DXOUT("[KO] long fraction: " << fraction);
        return false;
    }
    if (fraction.find_first_not_of(HTTP::CharFilter::digit) != light_string::npos) {
        // 小数部に数字以外がある
        DXOUT("[KO] non-digit in fraction: " << fraction);
        return false;
    }
    return true;
}
