#include "ecore_private.h"
#include "Ecore.h"

#include <sys/time.h>

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
