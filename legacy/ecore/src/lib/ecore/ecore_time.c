#include <sys/time.h>
#ifdef WIN32
#include <sys/timeb.h>
#endif
#include "ecore_private.h"
#include "Ecore.h"

#ifndef HAVE_GETTIMEOFDAY
#ifdef WIN32

static int
gettimeofday(struct timeval *tv, void *unused)
{
   struct _timeb t;
   
   if (!tv) return -1;

   _ftime (&t);
   
   tv->tv_sec = t.time;
   tv->tv_usec = t.millitm * 1000;
   
   return 0;
}
#else
# error "Your platform isn't supported yet"
#endif
#endif

/**
 * Retrieves the current system time as a floating point value in seconds.
 * @return  The number of seconds since 12.00AM 1st January 1970.
 * @ingroup Ecore_Time_Group
 */
double
ecore_time_get(void)
{
   struct timeval      timev;

   gettimeofday(&timev, NULL);
   return (double)timev.tv_sec + (((double)timev.tv_usec) / 1000000);
}
