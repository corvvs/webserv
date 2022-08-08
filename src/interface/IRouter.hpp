#ifndef IROUTER_HPP
#define IROUTER_HPP
#include "../communication/RequestHTTP.hpp"
#include "../communication/ResponseHTTP.hpp"
#include "IOriginator.hpp"
#include "IRequestMatcher.hpp"

// [ルータインターフェース]
// [責務]
// - 受け取ったリクエストをルーティングし, オリジネータを作成して返すこと
class IRouter {
public:
    virtual ~IRouter(){};

    virtual RequestMatchingResult route(const IRequestMatchingParam &request) = 0;
};

#endif
