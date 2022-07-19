#ifndef TEST_COMMON_HPP
#define TEST_COMMON_HPP

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#ifdef NDEBUG
#define DXOUT(expr) ((void)0)
#define DXERR(expr) ((void)0)
#else
#define DXOUT(expr)                                                                                                    \
    do {                                                                                                               \
        debug_out(__FILE__, __LINE__, __func__) << expr << std::endl;                                                  \
    } while (0)
#define DXERR(expr)                                                                                                    \
    do {                                                                                                               \
        debug_err(__FILE__, __LINE__, __func__) << expr << std::endl;                                                  \
    } while (0)
#endif
// 変数の中身の表示
#define VOUT(expr) DXOUT(#expr ": " << expr)
// 変数の中身の表示(クオート付き)
#define QVOUT(expr) DXOUT(#expr ": \"" << expr << "\"")

std::ostream &debug_out(const char *filename, const int linenumber, const char *func);

std::ostream &debug_err(const char *filename, const int linenumber, const char *func);

template <class T, class Allocator>
std::ostream &operator<<(std::ostream &stream, const std::vector<T, Allocator> &value) {
    stream << "[";
    for (typename std::vector<T, Allocator>::const_iterator it = value.begin(); it != value.end(); ++it) {
        if (it != value.begin()) {
            stream << ", ";
        }
        stream << *it;
    }
    stream << "]";
    return stream;
}

#endif
