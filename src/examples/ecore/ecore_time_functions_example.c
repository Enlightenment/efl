//Compile with:
// gcc -o ecore_time_functions_example ecore_time_functions_example.c `pkg-config --libs --cflags ecore`

#include <Ecore.h>
#include <unistd.h>

static Eina_Bool
_timer_cb(void *data EINA_UNUSED)
{
   printf("ecore time: %0.3f\n", ecore_time_get());
   printf("loop time: %0.3f\n", ecore_loop_time_get());
   printf("unix time: %0.3f\n", ecore_time_unix_get());
   printf("\nSleep for 1 second...\n\n");
   sleep(1);
   printf("ecore time: %0.3f\n", ecore_time_get());
   printf("loop time: %0.3f\n", ecore_loop_time_get());
   printf("unix time: %0.3f\n", ecore_time_unix_get());

   ecore_main_loop_quit();

   return EINA_FALSE;
}

int
main(void)
{
   if (!ecore_init())
     {
        printf("ERROR: Cannot init Ecore!\n");
        return -1;
     }

   ecore_timer_add(0.1, _timer_cb, NULL);
   ecore_main_loop_begin();
   ecore_shutdown();
}

