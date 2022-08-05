#ifndef IROUTER_HPP
#define IROUTER_HPP
#include "../communication/RequestHTTP.hpp"
#include "../communication/ResponseHTTP.hpp"
#include "../config/Config.hpp"
#include "IOriginator.hpp"

// [ルータインターフェース]
// [責務]
// - 受け取ったリクエストをルーティングし, オリジネータを作成して返すこと
class IRouter {
public:
    virtual ~IRouter(){};

    virtual IOriginator *route(const RequestHTTP &request, const config::config_vector &configs) = 0;
};

#endif
