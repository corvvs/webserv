#include "HeaderHTTP.hpp"
#define DEFINE_ATTR(key, list, aggr, uniq)                                                                             \
    {                                                                                                                  \
        HeaderAttribute attr = {                                                                                       \
            list,                                                                                                      \
            aggr,                                                                                                      \
            uniq,                                                                                                      \
        };                                                                                                             \
        predefined_attrs[key] = attr;                                                                                  \
    }

// [HeaderAttribute]

HeaderAttribute::attr_dict_type HeaderAttribute::predefined_attrs = HeaderAttribute::attr_dict_type();

void HeaderAttribute::set_predefined_attrs() {
    // [transfer-encoding]
    // https://triple-underscore.github.io/RFC7230-ja.html#_xref-6-10
    // "TE"ヘッダとは別物(TEは同じメッセージへの応答に対する指定; Transfer-Encodingは同じメッセージに対する指定)
    // - list, multiple
    DEFINE_ATTR(HeaderHTTP::transfer_encoding, 1, 1, 0);
    // [te]
    // https://triple-underscore.github.io/RFC7230-ja.html#section-4.3
    DEFINE_ATTR(HeaderHTTP::te, 1, 1, 0);
    // [set-cookie]
    // https://wiki.suikawiki.org/n/Set-Cookie%3A
    // 複数存在可能 & 集約禁止
    DEFINE_ATTR(HeaderHTTP::set_cookie, 0, 0, 0);
    // [content-length]
    // https://wiki.suikawiki.org/n/Content-Length%3A#anchor-88
    DEFINE_ATTR(HeaderHTTP::content_length, 0, 0, 1);
    // [host]
    // https://triple-underscore.github.io/RFC7230-ja.html#header.host
    DEFINE_ATTR(HeaderHTTP::host, 0, 0, 1);
}

// [HeaderItem]

HeaderItem::HeaderItem(const header_key_type &key) : key(key) {
    // 属性(attr)
    HeaderAttribute::attr_dict_type::iterator it = HeaderAttribute::predefined_attrs.find(key);
    if (it == HeaderAttribute::predefined_attrs.end()) {
        HeaderAttribute a = {};
        attr              = a;
    } else {
        // predefineされているならそれを設定する
        attr = it->second;
    }
}

void HeaderItem::add_val(const header_val_type &val) {
    if (values.size() > 0) {
        // すでにvalueがある場合 -> 必要に応じてしかるべく処理する
        DXOUT("* MULTIPLE VALUES * " << values.size());
    }
    DXOUT("ADDING Val to `" << key << "`: " << val);
    values.push_back(val);
}

const HeaderItem::header_key_type &HeaderItem::get_key() const {
    return key;
}

const HeaderItem::header_val_type *HeaderItem::get_val() const {
    return values.empty() ? NULL : &(values.front());
}

const HeaderItem::header_val_type *HeaderItem::get_back_val() const {
    return values.empty() ? NULL : &(values.back());
}

const HeaderItem::value_list_type &HeaderItem::get_vals() const {
    return values;
}

// [AHeaderHolder]

void AHeaderHolder::add_item(const light_string &key, const light_string &val) {
    // val の obs-foldを除去し, 全体を string に変換する.
    // obs-fold を検知した場合, そのことを記録する

    byte_string sval;
    byte_string pval = val.str();
    ssize_t movement = 0;
    while (true) {
        IndexRange obs_fold = ParserHelper::find_obs_fold(pval, movement, val.length() - movement);
        if (obs_fold.is_invalid()) {
            sval.insert(sval.end(), pval.begin() + movement, pval.begin() + obs_fold.second);
            break;
        }
        sval.insert(sval.end(), pval.begin() + movement, pval.begin() + obs_fold.first);
        sval += ParserHelper::SP;
        VOUT(obs_fold);
        // TODO: obs-foldを検知したことをリクエストに通知する必要がある
        // -> holder に持たせておけばよさそう
        // this->ps.found_obs_fold = true;
        movement = obs_fold.second;
    }

    header_key_type norm_key = ParserHelper::normalize_header_key(key);
    HeaderItem *item         = dict[norm_key];
    if (item == NULL) {
        list.push_back(HeaderItem(norm_key));
        item           = &(list.back());
        dict[norm_key] = item;
    }
    item->add_val(sval);
}

const AHeaderHolder::header_item_type *AHeaderHolder::get_item(const header_key_type &normalized_key) const {
    dict_type::const_iterator it = dict.find(normalized_key);
    if (it == dict.end()) {
        return NULL;
    }
    return it->second;
}

const AHeaderHolder::header_val_type *AHeaderHolder::get_val(const header_key_type &normalized_key) const {
    const HeaderItem *p = get_item(normalized_key);
    return p ? p->get_val() : NULL;
}

const AHeaderHolder::header_val_type *AHeaderHolder::get_back_val(const header_key_type &normalized_key) const {
    const HeaderItem *p = get_item(normalized_key);
    return p ? p->get_back_val() : NULL;
}

const AHeaderHolder::value_list_type *AHeaderHolder::get_vals(const header_key_type &normalized_key) const {
    const HeaderItem *p = get_item(normalized_key);
    return p ? &(p->get_vals()) : NULL;
}

const AHeaderHolder::list_type &AHeaderHolder::get_list() const {
    return list;
}

AHeaderHolder::list_type::size_type AHeaderHolder::get_list_size() const {
    return list.size();
}

AHeaderHolder::dict_type::size_type AHeaderHolder::get_dict_size() const {
    return dict.size();
}

// [HeaderHolderHTTP]

HeaderHolderHTTP::joined_dict_type HeaderHolderHTTP::get_cgi_meta_vars() const {
    HeaderHolderHTTP::joined_dict_type d;
    for (dict_type::const_iterator it = dict.begin(); it != dict.end(); ++it) {
        header_key_type key = HTTP::strfy("HTTP_") + it->first;
        HTTP::Utils::normalize_cgi_metavar_key(key);
        HTTP::byte_string val;
        HeaderItem::value_list_type vals = it->second->get_vals();
        for (HeaderItem::value_list_type::const_iterator it = vals.begin(); it != vals.end(); ++it) {
            if (it != vals.begin()) {
                val += HTTP::strfy(", ");
            }
            val += (*it);
        }
        d.insert(std::make_pair(key, val));
    }
    return d;
}
