#ifndef ERRORHTTP_HPP
#define ERRORHTTP_HPP
#include "http.hpp"
#include "test_common.hpp"
#include <exception>
#include <stdexcept>

// [HTTPエラー例外クラス]
// Connectionの受信フェーズでこれをcatchした場合, これを元にエラー応答を作成して返す.
class http_error : public std::runtime_error {
private:
    HTTP::t_status status_;

public:
    http_error(const char *_Message, HTTP::t_status status);

    HTTP::t_status get_status() const;
};

#endif
