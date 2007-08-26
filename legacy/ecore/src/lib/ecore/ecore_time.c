#ifdef _WIN32
# include <windows.h>
#else
# include <sys/time.h>
#endif
#include "Ecore.h"

/**
 * Retrieves the current system time as a floating point value in seconds.
 * @return  The number of seconds since 12.00AM 1st January 1970.
 * @ingroup Ecore_Time_Group
 */
EAPI double
ecore_time_get(void)
{
#ifdef _WIN32
   return (double)GetTickCount() / 1000.0;
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
