#include "test_common.hpp"

std::ostream &debug_out(const char *filename, const int linenumber, const char *func) {
    return (std::cout << "[" << filename << ":" << linenumber << " " << func << "] ");
}

std::ostream &debug_err(const char *filename, const int linenumber, const char *func) {
    return (std::cerr << "[" << filename << ":" << linenumber << " " << func << "] ");
}
