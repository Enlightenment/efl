/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include <config.h>
#ifdef HAVE_WINDOWS_H
# define WIN32_LEAN_AND_MEAN
# include <windows.h>
# undef WIN32_LEAN_AND_MEAN
#endif
#ifdef HAVE_SYS_TIME_H
# include <sys/time.h>
#endif
#include "ecore_private.h"
#include "Ecore.h"

/* FIXME: clock_gettime() is an option... */

/**
 * Retrieves the current system time as a floating point value in seconds.
 * @return  The number of seconds since 12.00AM 1st January 1970.
 * @ingroup Ecore_Time_Group
 */
EAPI double
ecore_time_get(void)
{
#ifdef _WIN32
   FILETIME ft;
   double     time;

   GetSystemTimeAsFileTime(&ft);
   time = (double)ft.dwLowDateTime + 4294967296.0 * (double)ft.dwHighDateTime;

   return time / 10000000;
#else
# ifdef HAVE_GETTIMEOFDAY
   struct timeval      timev;

   gettimeofday(&timev, NULL);
   return (double)timev.tv_sec + (((double)timev.tv_usec) / 1000000);
# else
#  error "Your platform isn't supported yet"
# endif
#endif /* _WIN32 */
}
