#ifndef REQUEST_TARGET_HPP
#define REQUEST_TARGET_HPP
#include "../utils/HTTPError.hpp"
#include "../utils/LightString.hpp"
#include "../utils/UtilsString.hpp"
#include "../utils/http.hpp"
#include "../utils/test_common.hpp"
#include "ChunkedBody.hpp"
#include "ControlHeaderHTTP.hpp"
#include "HeaderHTTP.hpp"
#include "Lifetime.hpp"
#include "ParserHelper.hpp"
#include "RoutingParameters.hpp"
#include "ValidatorHTTP.hpp"

#include <iostream>
#include <list>
#include <map>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

struct RequestTarget {
    typedef HTTP::byte_string byte_string;
    typedef HTTP::light_string light_string;

    // リクエストの"form"
    // - origin-form: "/"から始まるパスだけのやつ
    // - authority-form: ドメイン部だけのやつ
    // - absolute-form: URLまるごと
    // - asterisk-form: "*"のみ
    enum t_form { FORM_UNKNOWN, FORM_ORIGIN, FORM_AUTHORITY, FORM_ABSOLUTE, FORM_ASTERISK };

    // 与えられたリクエストターゲット
    light_string given;
    // エラーかどうか
    bool is_error;
    // 何formか
    t_form form;

    // cf. https://datatracker.ietf.org/doc/html/rfc3986#section-3
    //
    //     foo://example.com:8042/over/there?name=ferret#nose
    //     \_/   \______________/\_________/ \_________/ \__/
    //     |           |            |            |        |
    // scheme     authority       path        query   fragment
    //     |   _____________________|__
    //     urn:example:animal:ferret:nose

    light_string scheme;
    light_string authority;
    light_string path;
    light_string query;

    // パーセントエンコーディングをデコードしたもの
    struct Decoded {
        // scheme はパーセントエンコーディングされない
        byte_string authority;
        byte_string path;
        byte_string query;
        byte_string path_slash_reduced;
    };

    const byte_string &dauthority() const;
    const byte_string &dpath() const;
    const byte_string &dquery() const;
    const byte_string &dpath_slash_reduced() const;

    // デコード後のpathが使用不能文字を含んでいるかどうか
    bool decoded_target_has_unacceptable() const;

    RequestTarget();
    RequestTarget(const light_string &target);

private:
    Decoded decoded_parts;

    void decompose(const light_string &target);
    void decode_pct_encoded();
};

std::ostream &operator<<(std::ostream &ost, const RequestTarget &f);

#endif
