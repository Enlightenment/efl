#include <Ecore.h>

int
main(int argc, char *argv[])
{
   int ret = 0;

   ecore_init();
   ecore_app_args_set(argc, (const char **)argv);

   ecore_shutdown();
   return ret;
}

