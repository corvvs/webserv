#ifndef OBJECT_HOLDER_HPP
#define OBJECT_HOLDER_HPP
#include "../socket/SocketType.hpp"

class FDHolder {
public:
    typedef t_fd value_type;

private:
    value_type v_;

public:
    FDHolder(value_type v) throw() : v_(v) {}
    ~FDHolder() {
        destroy();
        waive();
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

    void destroy() throw() {
        if (v_ >= 0) {
            close(v_);
        }
    }

    void waive() throw() {
        v_ = -1;
    }
};

// new されたオブジェクトの所有権を一時的に保持するためのオブジェクト
template <class U>
class ObjectHolder {
public:
    typedef U *value_type;

private:
    value_type v_;

public:
    ObjectHolder(value_type v) throw() : v_(v) {}
    ~ObjectHolder() {
        destroy();
        waive();
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

    void destroy() throw() {
        delete v_;
    }

    void waive() throw() {
        v_ = NULL;
    }
};

#endif
