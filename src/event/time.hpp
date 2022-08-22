#ifndef TIME_HPP
#define TIME_HPP

#include <ctime>
#include <sys/time.h>

typedef unsigned long t_time_epoch_ms;
typedef unsigned long t_time_epoch_us;

namespace WSTime {
t_time_epoch_ms get_epoch_ms() throw();
t_time_epoch_us get_epoch_us() throw();
} // namespace WSTime

#ifdef NDEBUG
#define CLOCK(name, expr)                                                                                              \
    do {                                                                                                               \
        expr                                                                                                           \
    } while (0)
#else
#define CLOCK(name, expr)                                                                                              \
    do {                                                                                                               \
        t_time_epoch_us ts        = WSTime::get_epoch_us();                                                            \
        {expr} t_time_epoch_us tf = WSTime::get_epoch_us();                                                            \
        DXOUT(name << ": " << (tf - ts) << "us");                                                                      \
    } while (0)
#endif

#endif
