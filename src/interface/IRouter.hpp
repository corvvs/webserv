#ifndef IROUTER_HPP
#define IROUTER_HPP
#include "../communication/RequestHTTP.hpp"
#include "../communication/ResponseHTTP.hpp"
#include "IOriginator.hpp"

// [ルータインターフェース]
// [責務]
// - 受け取ったリクエストをルーティングし, オリジネータを作成して返すこと
class IRouter {
public:
    virtual ~IRouter(){};

    virtual IOriginator *route_origin(const RequestHTTP *request) = 0;

    // リクエストからレスポンスを生成する
    virtual ResponseHTTP *route(const RequestHTTP *request) = 0;

    // HTTPエラーからレスポンスを生成する
    virtual ResponseHTTP *respond_error(const RequestHTTP *request, http_error error) = 0;
};

#endif
