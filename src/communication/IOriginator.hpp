#ifndef IORIGINATOR_HPP
#define IORIGINATOR_HPP
#include "../utils/http.hpp"

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

    // オリジネータがデータを引き出すことができるかどうか
    // = `draw_data()` を呼び出すことができるかどうか
    virtual bool is_ready_to_draw() const = 0;

    // オリジネータからデータを引き出す
    // おそらく送信目的だと思うが, べつにそうでなくてもよい
    virtual byte_string draw_data() const = 0;
};

#endif
