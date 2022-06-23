#include "IndexRange.hpp"

IndexRange::IndexRange():
    pair(0, 0) {}

IndexRange::IndexRange(ssize_t f, ssize_t t):
    pair(f, t) {}

bool        IndexRange::is_invalid() const {
    return first > second;
}

ssize_t     IndexRange::length() const {
    return second - first;
}

IndexRange::first_type& IndexRange::from() {
    return first;
}

const IndexRange::first_type& IndexRange::from() const {
    return first;
}

IndexRange::first_type& IndexRange::to() {
    return second;
}

const IndexRange::first_type& IndexRange::to() const {
    return second;
}

std::ostream&   operator<<(std::ostream& out, const IndexRange& r) {
    return out << "[" << r.first << "," << r.second << ")";
}
