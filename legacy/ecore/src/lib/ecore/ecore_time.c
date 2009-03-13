/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>

#ifdef HAVE_SYS_TIME_H
# include <sys/time.h>
#endif

#ifdef HAVE_EVIL
# include <Evil.h>
#endif

#include "Ecore.h"
#include "ecore_private.h"



/* FIXME: clock_gettime() is an option... */

/**
 * Retrieves the current system time as a floating point value in seconds.
 *
 * Also see ecore_loop_time_get().
 *
 * @return  The number of seconds since 12.00AM 1st January 1970.
 * @ingroup Ecore_Time_Group
 */
EAPI double
ecore_time_get(void)
{
#ifdef HAVE_EVIL
  return evil_time_get();
#else
# ifdef HAVE_GETTIMEOFDAY
   struct timeval      timev;

   gettimeofday(&timev, NULL);
   return (double)timev.tv_sec + (((double)timev.tv_usec) / 1000000);
# else
#  error "Your platform isn't supported yet"
# endif
#endif
}

double _ecore_loop_time = -1.0;

/**
 * Retrieves the time at which the last loop stopped waiting for timeouts or events
 *
 * This gets the time (since Jan 1st, 1970, 12:00AM) that the main loop ceased
 * waiting for timouts and/or events to come in or for signals or any other
 * interrupt source. This should be considered a reference point for all
 * time based activity that should calculate its timepoint from the return
 * of ecore_loop_time_get(). use this UNLESS you absolutely must get the
 * current actual timepoint - then use ecore_time_get(). If this is called
 * before any loop has ever been run, then it will call ecore_time_get() for
 * you the first time and thus have an initial time reference.
 *
 * @return  The number of seconds since 12.00AM 1st January 1970.
 * @ingroup Ecore_Time_Group
 */
EAPI double
ecore_loop_time_get(void)
{
   return _ecore_loop_time;
}
