#ifndef TEST_COMMON_HPP
# define TEST_COMMON_HPP

# include <iostream>
# include <string>
# include <cstdlib>
# define DSOUT() debug_out(__FILE__, __LINE__)
# define DOUT()  debug_err(__FILE__, __LINE__)

#define debug(var)  do{std::cout << "[" << __FILE__ << ":" << __LINE__ << "] " << #var << " : ";view(var);}while(0)
template<typename T> void view(T e){std::cout << e << std::endl;}

std::ostream&   debug_out(
    const char *filename,
    const int linenumber
);

std::ostream&   debug_err(
    const char *filename,
    const int linenumber
);

#endif
