#ifndef UNPOSIX_LIMITS_H
#define UNPOSIX_LIMITS_H


#ifdef _WIN32
# ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
# endif
# include <windows.h>
# undef WIN32_LEAN_AND_MEAN
# include_next <limits.h>
# ifndef PATH_MAX
#  define PATH_MAX MAX_PATH
# endif
#endif

#endif
