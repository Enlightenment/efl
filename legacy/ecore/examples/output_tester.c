/* output and error tester.
 */

#include <Ecore.h>

#include <stdlib.h>
#include <stdio.h>
 
Ecore_Timer *timer1 = NULL;

char *data1 = "data1";
   
int
timer1_tick(void *data)
{
   printf("Once only timer called at %3.2f seconds, data %p\n", 
          ecore_time_get(), data);
   fprintf(stderr, "This is an error message from the timer callback.\n");   
   fprintf(stdout, "This is an output message from the timer callback.\n");   
   ecore_main_loop_quit();
   return 0;
}
 
int
main(int argc, char **argv)
{
   ecore_init();
   timer1 = ecore_timer_add(5.0, timer1_tick, data1);
   fprintf(stdout, "This is an output message from the main function.\n");   
   fprintf(stderr, "This is an error message from the main function.\n");   
   ecore_main_loop_begin();
   ecore_shutdown();
   return 123;
}
