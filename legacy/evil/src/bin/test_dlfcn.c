#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>

#include <dlfcn.h>

#include <windows.h>



int
main(int argc, char *argv[])
{
   void *module;
   void *symbol;

   if (argc < 3)
     {
        printf ("Usage: %s file.dll symbol\n\n", argv[0]);
        return EXIT_FAILURE;
     }

   printf ("opening module %s\n", argv[1]);
   module = dlopen(argv[1], 0);
   if (!module)
     {
        printf ("%s\n", dlerror());
        return EXIT_FAILURE;
     }
   printf ("opening module %s successful\n", argv[1]);

   printf ("getting symbol %s\n", argv[2]);
   symbol = dlsym(module, argv[2]);
   if (!symbol)
     {
        printf ("%s\n", dlerror());
        if (dlclose(module))
          printf ("%s\n", dlerror());
        return EXIT_FAILURE;
     }
   printf ("getting symbol %s successful\n", argv[2]);

   printf ("closing module %s\n", argv[1]);
   if (dlclose(module))
     {
        printf ("%s\n", dlerror());
        return EXIT_FAILURE;
     }
   printf ("closing module %s successful\n", argv[1]);

   return EXIT_SUCCESS;
}
