#ifndef IROUTER_HPP
#define IROUTER_HPP
#include "IOriginator.hpp"
#include "../communication/RequestHTTP.hpp"
#include "../communication/ResponseHTTP.hpp"

// [ルータインターフェース]
// [責務]
// - 受け取ったリクエストをルーティングし, オリジネータを作成して返すこと
class IRouter {
public:
    virtual ~IRouter(){};

    virtual IOriginator *route_origin(RequestHTTP *request) = 0;

    // リクエストからレスポンスを生成する
    virtual ResponseHTTP *route(RequestHTTP *request) = 0;

    // HTTPエラーからレスポンスを生成する
    virtual ResponseHTTP *respond_error(RequestHTTP *request, http_error error) = 0;
};

#endif
