#ifdef _WIN32
# include <windows.h>
#else
# include <sys/time.h>
#endif
#include "ecore_private.h"
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
   SYSTEMTIME st;
   double     time;
   int        days_nbr = 0;
   int        i;

   GetSystemTime(&st);
   for (i = 1; i < st.wMonth; i++)
     {
       if ((i == 1) ||
           (i == 3) ||
           (i == 5) ||
           (i == 7) ||
           (i == 8) ||
           (i == 10) ||
           (i == 12))
         days_nbr += 31;
       else if ((i == 4) ||
                (i == 6) ||
                (i == 5) ||
                (i == 9) ||
                (i == 11))
         days_nbr += 30;
       else
         {
           /* need to fix that... */
           if ((st.wYear % 4) == 0)
             {
                if ((st.wYear % 400) == 0)
                  {
                    if ((st.wYear % 1000) == 0)
                      days_nbr += 29;
                    else
                      days_nbr += 28;
                  }
                else
                  days_nbr += 29;
             }
           else
             days_nbr += 28;
         }
     }
   time =
     (double)st.wMilliseconds / 1000.0 +
     (double)st.wSecond +
     (double)st.wMinute * 60.0 +
     (double)st.wHour * 3600.0 +
     (double)(st.wDay + days_nbr) * 86400.0 +
     (double)(st.wYear - 1970) * 31536000.0;

   return time;
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
