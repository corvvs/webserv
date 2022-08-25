#ifndef HTML_HPP
#define HTML_HPP

#include "LightString.hpp"
#include "UtilsString.hpp"
#include "http.hpp"
#include <vector>

namespace HTML {

// 文字列中の文字`<>&'"`を数値実体参照に置き換えたものを返す
HTTP::char_string escape_html(const HTTP::char_string &str);
HTTP::byte_string escape_html(const HTTP::byte_string &str);

} // namespace HTML

#endif
