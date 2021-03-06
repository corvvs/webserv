#ifndef INDEXRANGE_HPP
#define INDEXRANGE_HPP
#include "test_common.hpp"
#include <iostream>
#include <string>
#include <utility>

struct IndexRange : public std::pair<ssize_t, ssize_t> {
    IndexRange();
    IndexRange(ssize_t f, ssize_t t);
    bool is_invalid() const;
    ssize_t length() const;

    first_type &from();
    const first_type &from() const;

    second_type &to();
    const second_type &to() const;
};

std::ostream &operator<<(std::ostream &out, const IndexRange &r);

#endif
