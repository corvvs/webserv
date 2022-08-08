#ifndef ROUTINGPARAMETER_HPP
#define ROUTINGPARAMETER_HPP
#include "../utils/HTTPError.hpp"
#include "../utils/LightString.hpp"
#include "../utils/UtilsString.hpp"
#include "../utils/http.hpp"
#include "../utils/test_common.hpp"
#include "ControlHeaderHTTP.hpp"
#include "HeaderHTTP.hpp"
#include "ParserHelper.hpp"
#include "ValidatorHTTP.hpp"
#include <iostream>
#include <list>
#include <map>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

struct ARoutingParameters {
    typedef HTTP::byte_string byte_string;
    typedef HTTP::light_string light_string;

    size_t body_size;

    // "セミコロン分割key-valueリスト" をパースして辞書に詰める
    // その後, パースできた部分以降を返す
    static light_string decompose_semicoron_separated_kvlist(const light_string &list_str, HTTP::IDictHolder &holder);

    // `lhost`の中身を`Host`構造体に詰める.
    // `lhost`はHost:ヘッダとしてvalidでなければならない.
    static void pack_host(HTTP::Term::Host &host_item, const light_string &lhost);

    // val_lstr のコメント部分を抽出して返す.
    // val_lstr はコメント部分を通過した状態になる.
    light_string extract_comment(light_string &val_lstr);

    // HTTP における`comment`を取り出す.
    light_string extract_comment(const light_string &str);
};

#endif
