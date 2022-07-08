#ifndef PARSERHELPER_HPP
#define PARSERHELPER_HPP
#include "../utils/IndexRange.hpp"
#include "../utils/LightString.hpp"
#include "ValidatorHTTP.hpp"
#include "../utils/http.hpp"
#include "../utils/test_common.hpp"
#include "../utils/UtilsString.hpp"
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace ParserHelper {
typedef HTTP::byte_string byte_string;
typedef HTTP::light_string light_string;

const byte_string SP                 = HTTP::strfy(" ");
const byte_string OWS                = HTTP::strfy(" \t");
const byte_string HEADER_KV_SPLITTER = HTTP::strfy(":");
const byte_string CRLF               = HTTP::strfy("\r\n");
const byte_string LF                 = HTTP::strfy("\n");

// LF, または CRLF を見つける.
// 見つかった場合, LFまたはCRLFの [開始位置, 終了位置の次) のペアを(絶対位置で)返す.
// 見つからない場合, [len+1,len] を返す.
IndexRange find_crlf(const byte_string &str, ssize_t from, ssize_t len);

// ヘッダー値用のfind_crlf; obs-fold をスルーする
// 返す range は str の先頭からのものになる
IndexRange find_crlf_header_value(const byte_string &str, ssize_t from, ssize_t len);
// 返す range は str の先頭からのものになる
IndexRange find_crlf_header_value(const light_string &str);

// obs-foldがあれば, そのレンジを返す
IndexRange find_obs_fold(const byte_string &str, ssize_t from, ssize_t len);

// 文字列の先頭から, CRLRおよびLFをすべてスキップした位置のインデックスを返す
ssize_t ignore_crlf(const byte_string &str, ssize_t from, ssize_t len);
bool is_sp(char c);

// 文字列の先頭から, 「空白」をすべてスキップした位置のインデックスを返す
ssize_t ignore_sp(const byte_string &str, ssize_t from, ssize_t len);

// 文字列の先頭から, 「空白」以外をすべてスキップした位置のインデックスを返す
ssize_t ignore_not_sp(const byte_string &str, ssize_t from, ssize_t len);

// 文字列の先頭に CRLF があるなら, その部分を返す.
// ただし, 文字列が完結しておらず, 曖昧性がある場合は空の区間を返す.
// 先頭が確実にCRLFでないなら, Invalid な区間を返す.
IndexRange find_leading_crlf(const byte_string &str, ssize_t from, ssize_t len, bool is_terminated);

// 文字列を「空白」で分割する
std::vector<byte_string> split_by_sp(byte_string::const_iterator first, byte_string::const_iterator last);
std::vector<light_string> split_by_sp(const light_string &str);

std::vector<HTTP::light_string> split(const HTTP::light_string &lstr, const char *charset);
std::vector<HTTP::light_string> split(const HTTP::light_string &lstr, const byte_string &charset);

byte_string normalize_header_key(const byte_string &key);
byte_string normalize_header_key(const HTTP::light_string &key);

std::pair<bool, unsigned int> xtou(const HTTP::light_string &str);
// string to size_t 変換
unsigned int stou(const byte_string &str);
unsigned int stou(const HTTP::light_string &str);
byte_string utos(unsigned int u);

// quality("q=0.05" の右辺側みたいな形式の文字列)を1000倍したunsigned intに変換
// qualityとしてvalidなもののみ渡すこと.
unsigned int quality_to_u(HTTP::light_string &quality);

light_string extract_quoted_or_token(const light_string &str);
} // namespace ParserHelper

#endif
