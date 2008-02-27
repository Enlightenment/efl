#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>

#include <dlfcn.h>

#include <windows.h>



int
main()
{
   struct timeval tv;
   void          *module;
   double         t1 = 0.0;
   double         t2 = 0.0;

   int (*time)(struct timeval *tv, void *tz);

   module = dlopen("d:\\msys\\1.0\\local\\bin\\libevil-0.dll", 0);
   if (!module)
     {
        printf ("%s\n", dlerror());
        return EXIT_FAILURE;
     }

   time = dlsym(module, "gettimeofday");
   if (!time)
     {
        printf ("%s\n", dlerror());
        if (dlclose(module))
          printf ("%s\n", dlerror());
        return EXIT_FAILURE;
     }

   if (time(&tv, NULL) == 0)
     t1 = tv.tv_sec + tv.tv_usec / 1000000.0;

   Sleep(3000);

   if (time(&tv, NULL) == 0)
     t2 = tv.tv_sec + tv.tv_usec / 1000000.0;

   printf ("3 seconds ? %f\n", t2 - t1);

   if (dlclose(module))
     {
        printf ("%s\n", dlerror());
        return EXIT_FAILURE;
     }

   return EXIT_SUCCESS;
}
