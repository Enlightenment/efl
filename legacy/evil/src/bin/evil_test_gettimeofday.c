

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <string.h>

#include <Evil.h>

#include "evil_suite.h"

static int
test_time_tests_run(suite *s)
{
   struct timeval tp1;
   struct timeval tp2;

   gettimeofday (&tp1, NULL);

   Sleep(20);

   gettimeofday (&tp2, NULL);

   printf ("time : %ld %ld\n", tp1.tv_sec, tp1.tv_usec);
   printf ("time : %ld %ld\n", tp2.tv_sec, tp2.tv_usec);
   printf ("time : %f\n", (double)tp2.tv_sec - tp1.tv_sec + (tp2.tv_usec - tp1.tv_usec) / 1000000.0);

   return 1;
}

int
test_gettimeofday(suite *s)
{

   return test_time_tests_run(s);
}
