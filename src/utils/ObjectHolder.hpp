#ifndef OBJECT_HOLDER_HPP
#define OBJECT_HOLDER_HPP
#include "../socket/SocketType.hpp"

// [RAIIクラス(*Holder)について]
// なんらかのリソースを「所有」した状態で構築される.
// 「所有」とは, 「所有者が破壊される時, 所有対象もまた破壊される」ことを意味する.
// *Holder の末路は3つ:
//
// 1. リソースを所有したままスコープアウトし, リソースごと破壊される
// 2. スコープアウトする前にリソースを破壊(destroy)する
// 3. スコープアウトする前にリソースの所有権を放棄(release)する
//
// 正常系では3. 異常系では1. となることが想定される.
// 2. はちょっと特殊な状況.

// ファイルディスクリプタ用RAIIクラス
class FDHolder {
public:
    typedef t_fd value_type;

private:
    value_type v_;

    FDHolder(const FDHolder &other) {
        *this = other;
    }

    FDHolder &operator=(const FDHolder &right) {
        (void)right;
        assert(false);
        return *this;
    }

    // 所有権を手放す
    void waive() throw() {
        v_ = -1;
    }

public:
    FDHolder(value_type v) throw() : v_(v) {}
    ~FDHolder() {
        destroy();
    }

    // 保持している値を返す.
    value_type value() const throw() {
        return v_;
    }

    // 保持している値を, 所有権を手放して返す.
    value_type release() throw() {
        value_type rv = v_;
        waive();
        return rv;
    }

    // 所有しているオブジェクトを破壊する
    void destroy() throw() {
        if (v_ >= 0) {
            close(v_);
        }
        waive();
    }
};

// オブジェクト用RAIIクラス
template <class U>
class ObjectHolder {
public:
    typedef U *value_type;

private:
    value_type v_;

    ObjectHolder(const ObjectHolder &other) {
        *this = other;
    }

    ObjectHolder &operator=(const ObjectHolder &right) {
        (void)right;
        assert(false);
        return *this;
    }

    // 所有権を手放す
    void waive() throw() {
        v_ = NULL;
    }

public:
    ObjectHolder(value_type v) throw() : v_(v) {}
    ~ObjectHolder() {
        destroy();
    }

    // 保持している値を返す.
    value_type value() const throw() {
        return v_;
    }

    // 保持している値を, 所有権を手放して返す.
    value_type release() throw() {
        value_type rv = v_;
        waive();
        return rv;
    }

    // 所有しているオブジェクトを破壊する
    void destroy() throw() {
        delete v_;
        waive();
    }
};

#endif
