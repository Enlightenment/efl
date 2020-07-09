#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#include <errno.h>
#include <direct.h>
# include <sys/time.h>

#ifndef WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#undef WIN32_LEAN_AND_MEAN

#include "evil_private.h"


LONGLONG _evil_time_freq;
LONGLONG _evil_time_count;

/*
 * Time related functions
 *
 */

EVIL_API double
evil_time_get(void)
{
   LARGE_INTEGER count;

   QueryPerformanceCounter(&count);

   return (double)(count.QuadPart - _evil_time_count)/ (double)_evil_time_freq;
}

