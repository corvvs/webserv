#include "RequestTarget.hpp"

RequestTarget::RequestTarget() : is_error(false) {}

RequestTarget::RequestTarget(const light_string &target) {
    decompose(target);
    decode_pct_encoded();
}

void RequestTarget::decompose(const light_string &target) {
    given    = target;
    is_error = false;
    form     = FORM_UNKNOWN;
    // TODO:
    // https://wiki.suikawiki.org/n/%E8%A6%81%E6%B1%82%E5%AF%BE%E8%B1%A1#anchor-22
    // 要求対象が // から始まるとき、 Apache も nginx も、 absolute-form と解釈するようです。
    // ↑ これを実装

    // formの識別
    assert(target.size() > 0);
    light_string temp = given;
    if (temp[0] == '/') {
        form = FORM_ORIGIN;
    } else if (temp == "*") {
        form = FORM_ASTERISK;
    } else {
        const light_string scheme_ = temp.substr_while(HTTP::CharFilter::uri_scheme);
        if (scheme_.size() < temp.size() && temp[scheme_.size()] == ':') {
            form   = FORM_ABSOLUTE;
            scheme = scheme_;
            temp   = temp.substr(scheme_.size() + 1);
        } else {
            form = FORM_AUTHORITY;
        }
    }
    // formを見ながらバリデーション
    if (form == FORM_ABSOLUTE) {
        const light_string ss = temp.substr(0, 2);
        if (ss != "//") {
            is_error = true;
            DXOUT("!! NG !!");
            return;
        }
        temp = temp.substr(ss.size());
    }
    // authority
    if (form == FORM_ABSOLUTE || form == FORM_AUTHORITY) {
        const light_string authority_ = temp.substr_before("/");
        if (!HTTP::Validator::is_uri_authority(authority_)) {
            is_error = true;
            DXOUT("!! authority NG !!");
            return;
        }
        authority = authority_;
        temp      = temp.substr(authority_.size());
    }
    // path
    if (form == FORM_ORIGIN || form == FORM_ABSOLUTE) {
        const light_string path_ = temp.substr_before("?");
        if (!HTTP::Validator::is_uri_path(path_, HTTP::CharFilter::pchar_without_pct)) {
            is_error = true;
            DXOUT("!! path NG !!");
            return;
        }
        path = path_;
        temp = temp.substr(path_.size());
    }
    // query
    if (form == FORM_ORIGIN || form == FORM_ABSOLUTE) {
        if (temp.size() > 0 && temp[0] == '?') {
            temp                      = temp.substr(1);
            const light_string query_ = temp.substr_before("#");
            if (!HTTP::Validator::is_segment(query_, HTTP::CharFilter::pchar_without_pct)) {
                is_error = true;
                DXOUT("!! query NG !!");
                return;
            }
            temp  = temp.substr(query_.size());
            query = query_;
        }
    }
    // fragment
    // -> 送信されないはずだが, もしあったらチェックして捨てる
    if (form == FORM_ORIGIN || form == FORM_ABSOLUTE) {
        if (temp.size() > 0 && temp[0] == '#') {
            temp = temp.substr(0, 0);
        }
        const light_string fragment_ = temp;
        if (!HTTP::Validator::is_segment(fragment_, HTTP::CharFilter::pchar_without_pct)) {
            is_error = true;
            DXOUT("!! fragment NG !!");
            return;
        }
    }
}

void RequestTarget::decode_pct_encoded() {
    decoded_parts.authority = ParserHelper::decode_pct_encoded(authority);
    decoded_parts.path      = ParserHelper::decode_pct_encoded(path, HTTP::CharFilter::reserved);
    decoded_parts.query     = ParserHelper::decode_pct_encoded(query);
    decoded_parts.path_slash_reduced.clear();
    for (HTTP::byte_string::size_type i = 0; i < decoded_parts.path.size(); ++i) {
        if (decoded_parts.path[i] != '/' || i == 0 || decoded_parts.path[i - 1] != '/') {
            decoded_parts.path_slash_reduced.push_back(decoded_parts.path[i]);
        }
    }
}

const RequestTarget::byte_string &RequestTarget::dauthority() const {
    return decoded_parts.authority;
}

const RequestTarget::byte_string &RequestTarget::dpath() const {
    return decoded_parts.path;
}

const RequestTarget::byte_string &RequestTarget::dquery() const {
    return decoded_parts.query;
}

const RequestTarget::byte_string &RequestTarget::dpath_slash_reduced() const {
    return decoded_parts.path_slash_reduced;
}

bool RequestTarget::decoded_target_has_unacceptable() const {
    const light_string::size_type unacceptable_pos
        = light_string(decoded_parts.path).find_first_of(HTTP::CharFilter::request_path_unacceptable);
    return unacceptable_pos != light_string::npos;
}

std::ostream &operator<<(std::ostream &ost, const RequestTarget &f) {
    return ost << "(" << f.form << (f.is_error ? "E" : "") << ") \"" << f.given << "\", scheme: \"" << f.scheme
               << "\", authority: \"" << f.authority << "\", path: \"" << f.path << "\", query: \"" << f.query;
}
