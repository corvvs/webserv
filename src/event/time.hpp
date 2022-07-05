#ifndef TIME_HPP
#define TIME_HPP

#include <ctime>
#include <sys/time.h>

typedef unsigned long t_time_epoch_ms;

namespace WSTime {
t_time_epoch_ms get_epoch_ms();
}

#endif
