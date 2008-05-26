/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef HAVE_SYS_TIME_H
# include <sys/time.h>
#endif

#include "Ecore.h"
#include "ecore_private.h"

/* FIXME: clock_gettime() is an option... */

/**
 * Retrieves the current system time as a floating point value in seconds.
 * @return  The number of seconds since 12.00AM 1st January 1970.
 * @ingroup Ecore_Time_Group
 */
EAPI double
ecore_time_get(void)
{
#ifdef HAVE_GETTIMEOFDAY
   struct timeval      timev;

   gettimeofday(&timev, NULL);
   return (double)timev.tv_sec + (((double)timev.tv_usec) / 1000000);
#else
# error "Your platform isn't supported yet"
#endif
}
