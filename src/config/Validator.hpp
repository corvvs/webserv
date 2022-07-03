#ifndef ANALYZER_HPP
#define ANALYZER_HPP
#include "Parser.hpp"
#include <map>
#include <string>
#include <vector>

namespace config {

typedef std::string error_type;
error_type validate(Directive dire, std::string term, std::vector<std::string> ctx);
std::string validation_error(const std::string &message, const size_t &line, const std::string directive = "");

class SyntaxError : public std::runtime_error {
public:
    SyntaxError(void) : std::runtime_error("config: validation error") {}
    SyntaxError(const std::string &what) : std::runtime_error(what) {}
};

// bit masks for different directive argument styles
const int NOARGS = 0x00000001; // 0 args
const int TAKE1  = 0x00000002; // 1 args
const int TAKE2  = 0x00000004; // 2 args
const int TAKE3  = 0x00000008; // 3 args
const int TAKE4  = 0x00000010; // 4 args
const int TAKE5  = 0x00000020; // 5 args
const int TAKE6  = 0x00000040; // 6 args
const int TAKE7  = 0x00000080; // 7 args (currently unused)
const int BLOCK  = 0x00000100; // followed by block
const int FLAG   = 0x00000200; // 'on' or 'off'
const int ANY    = 0x00000400; // >=0 args
const int MORE1  = 0x00000800; // >=1 args
const int MORE2  = 0x00001000; // >=2 args

// some helpful argument style aliases
const int TAKE12   = (TAKE1 | TAKE2);
const int TAKE23   = (TAKE2 | TAKE3);
const int TAKE34   = (TAKE3 | TAKE4);
const int TAKE123  = (TAKE12 | TAKE3);
const int TAKE1234 = (TAKE123 | TAKE4);

// bit masks for different directive locations
const int MAIN      = 0x00040000; // main context
const int HTTP_MAIN = 0x02000000; // http
const int HTTP_SRV  = 0x04000000; // http > server
const int HTTP_LOC  = 0x08000000; // http > location
const int HTTP_LMT  = 0x80000000; // http > location > limit_except

} // namespace config
#endif
