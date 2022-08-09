#ifndef HEADERITEMHTTP_HPP
#define HEADERITEMHTTP_HPP
#include "../utils/HTTPError.hpp"
#include "../utils/LightString.hpp"
#include "../utils/http.hpp"
#include "ParserHelper.hpp"
#include <list>
#include <map>
#include <string>

namespace HeaderHTTP {
const HTTP::byte_string host                = ParserHelper::normalize_header_key(HTTP::strfy("Host"));
const HTTP::byte_string connection          = ParserHelper::normalize_header_key(HTTP::strfy("Connection"));
const HTTP::byte_string cookie              = ParserHelper::normalize_header_key(HTTP::strfy("Cookie"));
const HTTP::byte_string set_cookie          = ParserHelper::normalize_header_key(HTTP::strfy("Set-Cookie"));
const HTTP::byte_string pragma              = ParserHelper::normalize_header_key(HTTP::strfy("Pragma"));
const HTTP::byte_string user_agent          = ParserHelper::normalize_header_key(HTTP::strfy("User-Agent"));
const HTTP::byte_string cache_control       = ParserHelper::normalize_header_key(HTTP::strfy("Cache-Control"));
const HTTP::byte_string authorization       = ParserHelper::normalize_header_key(HTTP::strfy("Authorization"));
const HTTP::byte_string www_authenticate    = ParserHelper::normalize_header_key(HTTP::strfy("www-Authenticate"));
const HTTP::byte_string keep_alive          = ParserHelper::normalize_header_key(HTTP::strfy("Keep-Alive"));
const HTTP::byte_string content_type        = ParserHelper::normalize_header_key(HTTP::strfy("Content-Type"));
const HTTP::byte_string content_disposition = ParserHelper::normalize_header_key(HTTP::strfy("Content-Disposition"));
const HTTP::byte_string content_length      = ParserHelper::normalize_header_key(HTTP::strfy("Content-Length"));
const HTTP::byte_string transfer_encoding   = ParserHelper::normalize_header_key(HTTP::strfy("Transfer-Encoding"));
const HTTP::byte_string te                  = ParserHelper::normalize_header_key(HTTP::strfy("TE"));
const HTTP::byte_string vary                = ParserHelper::normalize_header_key(HTTP::strfy("Vary"));
const HTTP::byte_string upgrade             = ParserHelper::normalize_header_key(HTTP::strfy("Upgrade"));
const HTTP::byte_string via                 = ParserHelper::normalize_header_key(HTTP::strfy("Via"));
// for CGI
const HTTP::byte_string status   = HTTP::strfy("status");
const HTTP::byte_string location = HTTP::strfy("location");
} // namespace HeaderHTTP

// あるヘッダキーの属性
struct HeaderAttribute {
    typedef HTTP::header_key_type header_key_type;
    typedef std::map<header_key_type, HeaderAttribute> attr_dict_type;

    // [属性]

    // リスト値である
    bool is_list;
    // 複数の同名ヘッダを1つにまとめてよい
    bool is_aggregatable;
    // 複数指定されていてはならない
    bool must_be_unique;

    // 定義済み属性
    static attr_dict_type predefined_attrs;
    // 定義済み属性を設定する
    static void set_predefined_attrs();
};

// ヘッダーのkeyとvalue(s)を保持する
// 再確保が起きないコンテナ(list)で保持し, mapにポインタを保持する
class HeaderItem {
public:
    typedef HTTP::header_key_type header_key_type;
    typedef HTTP::header_val_type header_val_type;
    // ここのlistはvectorでいいかも?
    typedef std::list<header_val_type> value_list_type;

private:
    const header_key_type key;
    value_list_type values;
    HeaderAttribute attr;

public:
    // キーを与えて構築
    HeaderItem(const header_key_type &key);

    // 値を与える
    void add_val(const header_val_type &val);

    const header_key_type &get_key() const;
    const header_val_type *get_val() const;
    const header_val_type *get_back_val() const;
    const value_list_type &get_vals() const;
};

// HeaderItem を保持する
class AHeaderHolder {
public:
    typedef HTTP::header_key_type header_key_type;
    typedef HTTP::header_val_type header_val_type;
    typedef HTTP::byte_string byte_string;
    typedef HTTP::light_string light_string;
    typedef HeaderItem header_item_type;
    // なぜ vector などではなく list を使うのかというと, 再確保を防ぐため.
    // 再確保を防ぐのは, dict で HeaderItem のポインタを保持するから.
    typedef std::list<HeaderItem> list_type;
    typedef std::map<header_key_type, HeaderItem *> dict_type;
    typedef HeaderItem::value_list_type value_list_type;
    typedef std::map<byte_string, byte_string> joined_dict_type;

protected:
    list_type list;
    dict_type dict;

public:
    // 指定したキーに値を追加する
    void add_item(const light_string &key, const light_string &val);
    // 指定したキーの値オブジェクトを取得する
    const header_item_type *get_item(const header_key_type &normalized_key) const;
    // 指定したキーの値を取得する; 複数ある場合は先頭を取得する
    const header_val_type *get_val(const header_key_type &normalized_key) const;
    // 指定したキーの末尾の値を取得する
    const header_val_type *get_back_val(const header_key_type &normalized_key) const;
    // 指定したキーの値をすべて取得する
    const value_list_type *get_vals(const header_key_type &normalized_key) const;

    const list_type &get_list() const;
    list_type::size_type get_list_size() const;
    dict_type::size_type get_dict_size() const;

    static void parse_header_lines(const light_string &lines, AHeaderHolder *holder);
    static minor_error parse_header_line(const light_string &line, AHeaderHolder *holder);
};

class HeaderHolderHTTP : public AHeaderHolder {

public:
    // HTTPヘッダをCGIメタ変数に加工した辞書を返す
    // 同じキーの値はすべて ", " で連結する
    joined_dict_type get_cgi_meta_vars() const;
};

class HeaderHolderCGI : public AHeaderHolder {};

class HeaderHolderSubpart : public AHeaderHolder {};

#endif
