#include "RoutingParameters.hpp"

ARoutingParameters::light_string ARoutingParameters::extract_comment(light_string &val_lstr) {
    // comment        = "(" *( ctext / quoted-pair / comment ) ")"
    // ctext          = HTAB / SP / %x21-27 / %x2A-5B / %x5D-7E / obs-text
    //                ; HTAB + SP + 表示可能文字, ただしカッコとバッスラを除く
    if (val_lstr.size() == 0 || val_lstr[0] != '(') {
        return val_lstr.substr(0, 0);
    }
    int paren                 = 0;
    bool quoted               = false;
    light_string comment_from = val_lstr;
    light_string::size_type n = 0;
    for (; val_lstr.size() > 0; ++n, val_lstr = val_lstr.substr(1)) {
        const char c = val_lstr[0];
        if (quoted) {
            if (!HTTP::CharFilter::qdright.includes(c)) {
                DXOUT("[KO] quoting non-quotable char: " << val_lstr.qstr());
                break;
            }
            quoted = false;
        } else {
            if (c == '(') {
                paren += 1;
            } else if (c == ')') {
                if (paren <= 0) {
                    DXOUT("[KO] too much ): " << val_lstr.qstr());
                    break;
                }
                paren -= 1;
                if (paren == 0) {
                    DXOUT("finish comment");
                    val_lstr = val_lstr.substr(1);
                    ++n;
                    break;
                }
            } else if (c == '\\') {
                quoted = true;
            } else if (!HTTP::CharFilter::ctext.includes(c)) {
                DXOUT("[KO] non ctext in comment: " << val_lstr.qstr());
                break;
            }
        }
    }
    return comment_from.substr(0, n);
}

ARoutingParameters::light_string
ARoutingParameters::decompose_semicoron_separated_kvlist(const light_string &kvlist_str, HTTP::IDictHolder &holder) {
    light_string list_str = kvlist_str;
    // *( OWS ";" OWS parameter )
    for (;;) {
        light_string params_str = list_str;
        QVOUT(params_str);
        params_str = params_str.substr_after(HTTP::CharFilter::sp);
        QVOUT(params_str);
        if (params_str.size() == 0) {
            DXOUT("away, there's only sp.");
            return params_str;
        }
        if (params_str[0] != ';') {
            DXOUT("away");
            return params_str;
        }
        params_str = params_str.substr_after(HTTP::CharFilter::sp, 1);
        // DXOUT("param_sep2: " << sep_pos);
        if (params_str.size() == 0) {
            DXOUT("[KO] there's only sp after ';'");
            return params_str;
        }
        // ここからparameterを取得
        // token
        const light_string key_lstr = params_str.substr_while(HTTP::CharFilter::tchar);
        if (key_lstr.size() == params_str.size() || params_str[key_lstr.size()] != '=') {
            DXOUT("[KO] no equal");
            return params_str.substr(params_str.size());
        }
        // [引数名]
        // 引数名はcase-insensitive
        // https://wiki.suikawiki.org/n/Content-Type#anchor-11
        // > 引数名は、大文字・小文字の区別なしで定義されています。
        params_str = params_str.substr(key_lstr.size() + 1);

        byte_string key_str = ParserHelper::normalize_header_key(key_lstr);
        QVOUT(key_str);
        {
            const light_string just_value_str = ParserHelper::extract_quoted_or_token(params_str);
            holder.store_list_item(key_str, just_value_str);
            list_str = params_str.substr(just_value_str.size());
        }
    }
    return list_str;
}

void ARoutingParameters::pack_host(HTTP::Term::Host &host_item, const light_string &lhost) {
    host_item.value = lhost.str();
    // この時点で lhost は Host: として妥当
    // -> 1文字目が [ かどうかで ipv6(vfuture) かどうかを判別する.
    if (lhost[0] == '[') {
        // ipv6 or ipvfuture
        const light_string host = lhost.substr_before("]", 1);
        host_item.host          = host.str();
        if (host.size() < lhost.size()) {
            host_item.port = lhost.substr(1 + host.size() + 2).str();
        }
    } else {
        // ipv4 or reg-name
        const light_string host = lhost.substr_before(":");
        host_item.host          = host.str();
        if (host.size() < lhost.size()) {
            light_string port = lhost.substr(host.size() + 1);
            host_item.port    = port.str();
        }
    }
    // DXOUT("host: \"" << host_item.host << "\"");
    // DXOUT("port: \"" << host_item.port << "\"");
}
