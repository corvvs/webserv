#include "HTML.hpp"
#include "../communication/ParserHelper.hpp"

HTTP::char_string HTML::escape_html(const HTTP::char_string &str) {
    HTTP::char_string rv;
    rv.reserve(str.size());
    for (HTTP::char_string::size_type i = 0; i < str.size(); ++i) {
        if (HTTP::CharFilter::escape_html.includes(str[i])) {
            rv += "&#";
            rv += HTTP::restrfy(ParserHelper::utos(str[i], 10));
            rv += ";";
        } else {
            rv.push_back(str[i]);
        }
    }
    return rv;
}

HTTP::byte_string HTML::escape_html(const HTTP::byte_string &str) {
    HTTP::byte_string rv;
    rv.reserve(str.size());
    for (HTTP::byte_string::size_type i = 0; i < str.size(); ++i) {
        if (HTTP::CharFilter::escape_html.includes(str[i])) {
            rv += HTTP::strfy("&#");
            rv += ParserHelper::utos(str[i], 10);
            rv += HTTP::strfy(";");
        } else {
            rv.push_back(str[i]);
        }
    }
    return rv;
}
