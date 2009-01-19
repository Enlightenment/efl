#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#include <string.h>
#include <math.h>

#include <Evil.h>

#include "evil_suite.h"

static int
test_time_test_gettimeofday(void)
{
   struct timeval tp1;
   struct timeval tp2;
   double         delta;

   gettimeofday (&tp1, NULL);

   Sleep(1000);

   gettimeofday (&tp2, NULL);

   delta = (double)(tp2.tv_sec - tp1.tv_sec) + (double)(tp2.tv_usec - tp1.tv_usec) / 1000000.0;
   if (fabs(delta - 1) > 0.005)
     {
        return 0;
     }

   return 1;
}

static int
test_time_tests_run(suite *s)
{
   int res;

   res = test_time_test_gettimeofday();

   return res;
}

int
test_gettimeofday(suite *s)
{

   return test_time_tests_run(s);
}
