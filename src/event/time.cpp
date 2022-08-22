#include "time.hpp"

t_time_epoch_ms WSTime::get_epoch_ms() throw() {
    struct timeval time_now = {};
    gettimeofday(&time_now, NULL);
    return (time_now.tv_sec * 1000) + (time_now.tv_usec / 1000);
}

t_time_epoch_us WSTime::get_epoch_us() throw() {
    struct timeval time_now = {};
    gettimeofday(&time_now, NULL);
    return (time_now.tv_sec * 1000000) + (time_now.tv_usec);
}
