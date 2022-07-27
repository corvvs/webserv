#ifndef VALIDATORHTTP_HPP
#define VALIDATORHTTP_HPP
#include "../utils/LightString.hpp"
#include "../utils/http.hpp"
#include <list>
#include <map>
#include <string>

namespace HTTP {
namespace Validator {
bool is_valid_header_host(const HTTP::light_string &str);
// URI
bool is_uri_host(const HTTP::light_string &str);
bool is_ip_literal(const HTTP::light_string &str);
bool is_ipv6address(const HTTP::light_string &str);
bool is_ipvfuture(const HTTP::light_string &str);
bool is_ipv4address(const HTTP::light_string &str);
bool is_reg_name(const HTTP::light_string &str);
bool is_port(const HTTP::light_string &str);
bool is_h16(const HTTP::light_string &str);
bool is_ls32(const HTTP::light_string &str);
bool is_uri_authority(const HTTP::light_string &str);
bool is_uri_path(const HTTP::light_string &str, const HTTP::CharFilter &segment_filter);
bool is_segment(const HTTP::light_string &str, const HTTP::CharFilter &segment_filter);

// rank: 小数点以下10進3桁以内, 0以上1以下の実数.
bool is_valid_rank(const HTTP::light_string &str);
} // namespace Validator
} // namespace HTTP

#endif
