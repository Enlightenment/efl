#include "ecore_private.h"
#include "Ecore.h"

#include <sys/time.h>

/**
 * Get the current system time as a floating point value in seconds.
 * @return The current time (since the epoch start) in seconds
 * 
 * This function returns the current system time in seconds from 12:00am 
 * 1st Janruary 1970. The time is returned as a double precision floating point
 * value to allow for fractions of a second to be determined.
 */
double
ecore_time_get(void)
{
   struct timeval      timev;

   gettimeofday(&timev, NULL);
   return (double)timev.tv_sec + (((double)timev.tv_usec) / 1000000);
}
