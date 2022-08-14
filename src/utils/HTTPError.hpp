#ifndef HTTPERROR_HPP
#define HTTPERROR_HPP
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
class minor_error : public std::pair<std::string, HTTP::t_status> {
public:
    minor_error();
    minor_error(first_type message, second_type status_code);
    minor_error(const minor_error &other);
    minor_error &operator=(const minor_error &rhs);

    bool is_ok() const;
    bool is_error() const;

    first_type message() const;
    second_type status_code() const;

    static minor_error ok();
    static minor_error make(const std::string &message, HTTP::t_status status_code);
};

std::ostream &operator<<(std::ostream &ost, const minor_error &f);

// 2つの引数のうち, 左側がエラーなら左側を, そうでないなら右側を返す.
// (確実に短絡評価させないために関数にしている)
const minor_error &erroneous(const minor_error &e1, const minor_error &e2);

#endif
