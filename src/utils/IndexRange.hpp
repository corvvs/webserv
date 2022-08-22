#ifndef INDEXRANGE_HPP
#define INDEXRANGE_HPP
#include "test_common.hpp"
#include <iostream>
#include <string>
#include <utility>

struct IndexRange : public std::pair<ssize_t, ssize_t> {
    IndexRange() throw();
    IndexRange(ssize_t f, ssize_t t) throw();
    bool is_invalid() const throw();
    ssize_t length() const throw();

    first_type &from() throw();
    const first_type &from() const throw();

    second_type &to() throw();
    const second_type &to() const throw();
};

std::ostream &operator<<(std::ostream &out, const IndexRange &r);

#endif
