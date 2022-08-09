#ifndef ERRORHTTP_HPP
#define ERRORHTTP_HPP
#include "http.hpp"
#include "test_common.hpp"
#include <exception>
#include <stdexcept>
#include <utility>

// [回復不能なHTTPエラー例外クラス]
// 「回復不能なHTTPエラー」が発生した場合,
// クライアントには即座にHTTPエラー応答を送信し, その後HTTP接続を切る.
class http_error : public std::runtime_error {
private:
    HTTP::t_status status_;

public:
    http_error(const char *_Message, HTTP::t_status status);

    HTTP::t_status get_status() const;
};

// [軽微なHTTPエラークラス]
// 即例外を出すほどではないが, 最終的にはエラー応答を返すようなエラー.
class minor_error : public std::pair<const std::string, HTTP::t_status> {
public:
    minor_error(first_type message, second_type status_code);
    bool is_ok() const;
    bool is_error() const;

    first_type message() const;
    second_type status_code() const;

    static minor_error ok();
    static minor_error make(const std::string &message, HTTP::t_status status_code);
};

#endif
