#include "ecore_evas_test.h"
   
#ifdef BUILD_ECORE_EVAS

int
main(int argc, const char **argv)
{
   if (app_start(argc, argv) < 1) return -1;
   
   bg_start();
   
   calibrate_start();

   ecore_main_loop_begin();
   
   app_finish();
   
   return 0;
}
#else
int
main(int argc, const char **argv)
{
   printf("Ecore_evas module not compiled. This program is empty.\n");
   return -1;
}
#endif
