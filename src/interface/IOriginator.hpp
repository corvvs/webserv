#ifndef IORIGINATOR_HPP
#define IORIGINATOR_HPP
#include "../communication/RequestHTTP.hpp"
#include "../communication/ResponseHTTP.hpp"
#include "../utils/http.hpp"
#include "IObserver.hpp"
#include "ISocketLike.hpp"

// [オリジネータインターフェース]
// オリジンサーバとしてのタスク = レスポンスをoriginateするクラス
// [責務]
// - Connectionに保持されること
// - 必要に応じてConnectionからデータを受け取ること
// - 必要に応じてConnectionにデータを送ること
//   - この2つはConnectionからのメソッド呼び出しの形をとる
//   - オリジネーション対象(ファイル, CGI)との通信はオリジネータが主体となるが,
//     Connection絡みの操作はConnectionが主体となるようにする.
//   - オリジネータ側の準備ができてから, オリジネータ側がConnectionに通知してこれらの操作を"させる", という形をとる.

// このインターフェースは"Connectionに対する"インターフェースなので,
// オリジネーション対象との相互作用はこのインターフェースには現れない(実装クラスの内部にある).
// 以下はオリジネータとして想定されているオリジネーション対象に対する責務:
// - オリジネーション対象に対してデータを送れること
//   - 送る必要がないこともある
//     - ファイル読み込みの場合など
// - オリジネーション対象にからデータを受け取れること
//   - 受け取る必要がないこともある
//     - ファイル書き込みの場合など
class IOriginator {
public:
    typedef HTTP::byte_string byte_string;

    virtual ~IOriginator(){};

    // 自身がソケットライクでもあるような場合は何かをするが, そうでない場合は呼び出されること自体が誤り
    virtual void notify(IObserver &observer, IObserver::observation_category cat, t_time_epoch_ms epoch) = 0;
    virtual void inject_socketlike(ISocketLike *socket_like)                                             = 0;

    // オリジネーションを開始できるかどうか
    virtual bool is_originatable() const = 0;
    // オリジネーション開始済みかどうか
    virtual bool is_origination_started() const = 0;
    // 再ルーティングすべきか
    virtual bool is_reroutable() const = 0;
    // レスポンスを作成できるかどうか
    virtual bool is_responsive() const = 0;

    // オリジネーションを開始する
    virtual void start_origination(IObserver &observer) = 0;

    virtual ResponseHTTP *respond(const RequestHTTP *request) = 0;

    // オリジネータ自身の破壊手続きを行う
    // 大抵は`delete this;だがソケットライクだったりすると違う
    virtual void leave() = 0;
};

#endif
