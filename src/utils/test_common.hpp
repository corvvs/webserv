#ifndef TEST_COMMON_HPP
# define TEST_COMMON_HPP

# include <iostream>
# include <string>
# include <cstdlib>
// TODO: この辺レギュレーション的に使えないので注意
# define DSOUT() debug_out(__FILE__, __LINE__, __func__)
# define DOUT()  debug_err(__FILE__, __LINE__, __func__)
# define DXOUT(expr) do { debug_out(__FILE__, __LINE__, __func__) << expr << std::endl; } while(0)
# define DXERR(expr) do { debug_err(__FILE__, __LINE__, __func__) << expr << std::endl; } while(0)
// # define DXOUT(expr) ((void)0)


std::ostream&   debug_out(
    const char *filename,
    const int linenumber,
    const char *func
);

std::ostream&   debug_err(
    const char *filename,
    const int linenumber,
    const char *func
);

#endif
