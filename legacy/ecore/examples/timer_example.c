/* Timer example.
 */

#include <Ecore.h>

#include <stdlib.h>
#include <stdio.h>
 
Ecore_Timer *timer1 = NULL;
Ecore_Timer *timer2 = NULL;
Ecore_Timer *timer3 = NULL;

char *data1 = "data1";
char *data2 = "data2";
char *data3 = "data3";
   
int timer3_tick(void *data) {
  printf("Tick timer %3.2f\n", ecore_time_get());
  return 1;
}
 
int timer2_tick(void *data) {
  printf("Repeat timer called at %3.2f seconds, data %p\n",
         ecore_time_get(), data);
  return 1;
}
 
int timer1_tick(void *data) {
  printf("Once only timer called at %3.2f seconds, data %p\n", 
          ecore_time_get(), data);
  ecore_timer_del(timer2);
  return 0;
}
 
int main(int argc, char **argv) {
  ecore_init();
  timer1 = ecore_timer_add(5.0, timer1_tick, data1);
  timer2 = ecore_timer_add(0.5, timer2_tick, data2);
  timer3 = ecore_timer_add(1.0, timer3_tick, data3);
  ecore_main_loop_begin();
  ecore_shutdown();
  return 0;
}
