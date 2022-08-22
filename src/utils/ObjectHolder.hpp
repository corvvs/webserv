#ifndef OBJECT_HOLDER_HPP
#define OBJECT_HOLDER_HPP

template <class OBJ>
class ObjectHolder {
public:
    typedef OBJ object_type;

private:
    object_type *ptr_;
    bool holding;

public:
    ObjectHolder(OBJ *ptr) throw() : ptr_(ptr), holding(ptr != NULL) {}
    ~ObjectHolder() {
        if (holding) {
            delete ptr_;
        }
        ptr_    = NULL;
        holding = false;
    }

    // 保持している値を, 所有権を持ったまま返す.
    object_type *value() const throw() {
        return ptr_;
    }

    // 保持している値の所有権を放棄する
    void waive() throw() {
        holding = false;
    }

    // 保持している値を, 所有権を手放して返す.
    object_type *release() throw() {
        waive();
        return value();
    }
};

#endif
