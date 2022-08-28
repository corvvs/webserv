#ifndef PARSERHELPER_HPP
#define PARSERHELPER_HPP
#include "../utils/IndexRange.hpp"
#include "../utils/LightString.hpp"
#include "../utils/UtilsString.hpp"
#include "../utils/http.hpp"
#include "../utils/test_common.hpp"
#include "ValidatorHTTP.hpp"
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace ParserHelper {
typedef HTTP::byte_string byte_string;
typedef HTTP::light_string light_string;

extern const byte_string SP;
extern const byte_string OWS;
extern const byte_string HEADER_KV_SPLITTER;
extern const byte_string CRLF;
extern const byte_string LF;
extern const byte_string LWS;

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

std::pair<bool, unsigned long> xtou(const HTTP::light_string &str);
byte_string utos(unsigned long u, unsigned int base);

// 確実に変換できることがわかっている時に使うこと
long latoi(const HTTP::light_string &str);

// string to size_t 変換
std::pair<bool, unsigned long> str_to_u(const byte_string &str);
std::pair<bool, unsigned long> str_to_u(const HTTP::light_string &str);

// quality("q=0.05" の右辺側みたいな形式の文字列)を1000倍したunsigned intに変換
// qualityとしてvalidなもののみ渡すこと.
unsigned int quality_to_u(HTTP::light_string &quality);

light_string extract_quoted_or_token(const light_string &str);

// 与えられた文字列が HTTP-dateとして解釈可能なら, epoch に変換して返す.
std::pair<bool, t_time_epoch_ms> http_date_to_time(const light_string &str);

// 与えられた時刻を HTTP-date 形式(IMF-fixdate)の文字列として出力する
byte_string time_to_http_date(t_time_epoch_ms t);

// パーセントエンコードされた文字列をデコードする
// ただしエンコード後の文字が`exclude`に含まれる場合はデコードしない
byte_string decode_pct_encoded(const byte_string &str, const HTTP::CharFilter &exclude = HTTP::CharFilter::empty);
// パーセントエンコードされた文字列をデコードする
// ただしエンコード後の文字が`exclude`に含まれる場合はデコードしない
byte_string decode_pct_encoded(const light_string &str, const HTTP::CharFilter &exclude = HTTP::CharFilter::empty);
} // namespace ParserHelper

#endif
