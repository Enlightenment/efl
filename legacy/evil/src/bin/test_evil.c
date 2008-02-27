#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>

#include <windows.h>



int
main()
{
   struct timeval tv;
   double         t1 = 0.0;
   double         t2 = 0.0;

   if (gettimeofday(&tv, NULL) == 0)
     t1 = tv.tv_sec + tv.tv_usec / 1000000.0;

   Sleep(3000);

   if (gettimeofday(&tv, NULL) == 0)
     t2 = tv.tv_sec + tv.tv_usec / 1000000.0;

   printf ("3 seconds ? %f\n", t2 - t1);

   return EXIT_SUCCESS;
}
