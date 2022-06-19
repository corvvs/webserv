#include "test_common.hpp"

std::ostream&   debug_out(
    const char *filename,
    const int linenumber
) {
    return (std::cout << "[" << filename << ":" << linenumber  << "] ");
}

std::ostream&   debug_err(
    const char *filename,
    const int linenumber
) {
    return (std::cerr << "[" << filename << ":" << linenumber << "] ");
}
