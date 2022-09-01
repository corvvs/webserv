#ifndef MIME_HPP
#define MIME_HPP

#include "UtilsString.hpp"
#include "http.hpp"
#include <map>

namespace HTTP {
namespace MIME {
typedef std::map<byte_string, byte_string> mime_map_type;

void setup_mime_map();
byte_string mime_type_for_extension(const byte_string &ext);
} // namespace MIME
} // namespace HTTP

#endif
