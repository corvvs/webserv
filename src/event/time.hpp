#ifndef TIME_HPP
#define TIME_HPP

#include <ctime>
#include <sys/time.h>

typedef unsigned long t_time_epoch_ms;
typedef unsigned long t_time_epoch_us;

namespace WSTime {
t_time_epoch_ms get_epoch_ms();
t_time_epoch_us get_epoch_us();
} // namespace WSTime

#endif
