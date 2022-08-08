#ifndef ANALYZER_HPP
#define ANALYZER_HPP
#include "Parser.hpp"
#include <map>
#include <string>
#include <vector>

namespace config {

typedef std::string ErrorMsg;
class SyntaxError : public std::runtime_error {
public:
    SyntaxError(void) : std::runtime_error("config: validation error") {}
    SyntaxError(const std::string &what) : std::runtime_error(what) {}
};

namespace Validator {

bool is_valid_integer(const std::string &s);
bool is_ipaddr(const std::string &s);
bool is_host(const std::string &s);
bool is_port(const std::string &arg);
bool is_valid_error_page(const std::vector<std::string> &args);
bool is_valid_return(const std::vector<std::string> &args);
bool is_valid_client_max_body_size(const std::vector<std::string> &args);
bool is_valid_listen(const std::vector<std::string> &args);
bool is_valid_flag(std::string s);
bool is_must_be_on_off(Directive dire, int mask);

std::map<std::vector<std::string>, int> setting_contexts(void);
std::map<std::string, int> setting_directives(void);
int get_directive_mask(std::string dire);
int get_context_mask(std::vector<std::string> ctx);

ErrorMsg validate(Directive dire, std::string term, std::vector<std::string> ctx);
std::string validation_error(const std::string &message, const size_t &line, const std::string directive = "");

// constant variables
const unsigned int NOARGS = 0x00000001; // 0 args
const unsigned int TAKE1  = 0x00000002; // 1 args
const unsigned int TAKE2  = 0x00000004; // 2 args
const unsigned int TAKE3  = 0x00000008; // 3 args
const unsigned int BLOCK  = 0x00000100; // followed by block
const unsigned int FLAG   = 0x00000200; // 'on' or 'off'
const unsigned int ANY    = 0x00000400; // >=0 args
const unsigned int MORE1  = 0x00000800; // >=1 args
const unsigned int MORE2  = 0x00001000; // >=2 args

const unsigned int TAKE12 = (TAKE1 | TAKE2);

// bit masks for different directive locations
const unsigned int GLOBAL    = 0x00040000; // main context
const unsigned int HTTP_MAIN = 0x02000000; // http
const unsigned int HTTP_SRV  = 0x04000000; // http > server
const unsigned int HTTP_LOC  = 0x08000000; // http > location
const unsigned int HTTP_LMT  = 0x80000000; // http > location > limit_except
} // namespace Validator
} // namespace config
#endif
