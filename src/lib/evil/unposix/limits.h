#ifndef UNPOSIX_LIMITS_H
#define UNPOSIX_LIMITS_H

#include <evil_windows.h>
#include_next <limits.h>
#ifndef PATH_MAX
# define PATH_MAX MAX_PATH
#endif

#endif
