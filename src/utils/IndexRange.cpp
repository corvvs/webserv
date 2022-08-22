#include "IndexRange.hpp"

IndexRange::IndexRange() throw() : pair(0, 0) {}

IndexRange::IndexRange(ssize_t f, ssize_t t) throw() : pair(f, t) {}

bool IndexRange::is_invalid() const throw() {
    return first > second;
}

ssize_t IndexRange::length() const throw() {
    return second - first;
}

IndexRange::first_type &IndexRange::from() throw() {
    return first;
}

const IndexRange::first_type &IndexRange::from() const throw() {
    return first;
}

IndexRange::first_type &IndexRange::to() throw() {
    return second;
}

const IndexRange::first_type &IndexRange::to() const throw() {
    return second;
}

std::ostream &operator<<(std::ostream &out, const IndexRange &r) {
    return out << "[" << r.first << "," << r.second << ")";
}
