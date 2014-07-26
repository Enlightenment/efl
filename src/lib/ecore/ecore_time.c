#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>
#include <sys/time.h>

#ifdef HAVE_EVIL
# include <Evil.h>
#endif

#if defined(__APPLE__) && defined(__MACH__)
# include <mach/mach_time.h>
#endif

#include <time.h>

#include "Ecore.h"
#include "ecore_private.h"

#if defined (HAVE_CLOCK_GETTIME) || defined (EXOTIC_PROVIDE_CLOCK_GETTIME)
static clockid_t _ecore_time_clock_id = -1;
#elif defined(__APPLE__) && defined(__MACH__)
static double _ecore_time_clock_conversion = 1e-9;
#endif
double _ecore_time_loop_time = -1.0;

EAPI double
ecore_time_get(void)
{
#if defined (HAVE_CLOCK_GETTIME) || defined (EXOTIC_PROVIDE_CLOCK_GETTIME) 
   struct timespec t;

   if (EINA_UNLIKELY(_ecore_time_clock_id < 0))
     return ecore_time_unix_get();

   if (EINA_UNLIKELY(clock_gettime(_ecore_time_clock_id, &t)))
     {
        CRI("Cannot get current time.");
        /* Try to at least return the latest value retrieved*/
        return _ecore_time_loop_time;
     }

   return (double)t.tv_sec + (((double)t.tv_nsec) / 1000000000.0);
#elif defined(HAVE_EVIL)
   return evil_time_get();
#elif defined(__APPLE__) && defined(__MACH__)
   return _ecore_time_clock_conversion * (double)mach_absolute_time();
#else
   return ecore_time_unix_get();
#endif
}

EAPI double
ecore_time_unix_get(void)
{
#ifdef HAVE_GETTIMEOFDAY
   struct timeval timev;

   gettimeofday(&timev, NULL);
   return (double)timev.tv_sec + (((double)timev.tv_usec) / 1000000);
#else
# error "Your platform isn't supported yet"
#endif
}

EAPI double
ecore_loop_time_get(void)
{
   return _ecore_time_loop_time;
}

EAPI void
ecore_loop_time_set(double t)
{
   _ecore_time_loop_time = t;
}

/*-********************   Internal methods   ********************************/

/* TODO: Documentation says "All  implementations  support  the  system-wide
 * real-time clock, which is identified by CLOCK_REALTIME. Check if the fallback
 * to unix time (without specifying the resolution) might be removed
 */
void
_ecore_time_init(void)
{
#if defined (HAVE_CLOCK_GETTIME) || defined (EXOTIC_PROVIDE_CLOCK_GETTIME)
   struct timespec t;

   if (_ecore_time_clock_id != -1) return;

   if (!clock_gettime(CLOCK_MONOTONIC, &t))
     {
        _ecore_time_clock_id = CLOCK_MONOTONIC;
        DBG("using CLOCK_MONOTONIC.");
     }
   else if (!clock_gettime(CLOCK_REALTIME, &t))
     {
        /* may go backwards */
         _ecore_time_clock_id = CLOCK_REALTIME;
         WRN("CLOCK_MONOTONIC not available. Fallback to CLOCK_REALTIME.");
     }
   else
     {
        _ecore_time_clock_id = -2;
        CRI("Cannot get a valid clock_gettime() clock id! "
             "Fallback to unix time.");
     }
#else
# ifndef HAVE_EVIL
#  if defined(__APPLE__) && defined(__MACH__)
   mach_timebase_info_data_t info;
   kern_return_t err = mach_timebase_info(&info);
   if (err == 0)
     {
        _ecore_time_clock_conversion = 1e-9 * (double)info.numer / (double)info.denom;
     }
   else
     {
        WRN("Unable to get timebase info. Fallback to nanoseconds.");
     }
#  else
#  warning "Your platform isn't supported yet"
   CRI("Platform does not support clock_gettime. "
       "Fallback to unix time.");
#  endif
# endif
#endif

   _ecore_time_loop_time = ecore_time_get();
}

