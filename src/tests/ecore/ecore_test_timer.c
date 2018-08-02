#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <Ecore.h>

#include <math.h>

#include "ecore_suite.h"

#define TIMEOUT_1 0.01 // interval for timer1
#define TIMEOUT_2 0.02 // timer2
#define TIMEOUT_3 0.06 // timer3
#define TIMEOUT_4 0.1  // timer4
#define TIMEOUT_5 0.5  // timer5 - end ecore_main_loop_begin()
#define SIZE 3

static int freeze_thaw = 0;   // 1 - freeze timer, 0 - thaw

struct _timers           // timer struct
{
   Ecore_Timer *timer1; // timer 1
   Ecore_Timer *timer2;
   Ecore_Timer *timer3;
   Ecore_Timer *timer4;
   Ecore_Timer *timer5;
   int count_timer1;
   int count_timer3;    // check count timer 3
   int add_timer2;      // flag add timer 2
   int check_freeze_thaw_timer3;
   int num_elem;
   double delay_1[3];
   double interval_1[3];
   double precision[3];
};

static Eina_Bool
_timer1_cb(void *data)
{
   struct _timers *timer = data;
   timer->count_timer1++;

   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_timer2_cb(void *data)
{
   struct _timers *timer = (struct _timers*)data;

   // choose next settings (delay, interval, precision) for timer 1
   if (++timer->num_elem > 2)
     timer->num_elem = 0;

   // check add/delay timer 2
   fail_if(timer->add_timer2 > 1, "Error add/delay timer");

   // check set new delay for timer 1
   ecore_timer_delay(timer->timer1, timer->delay_1[timer->num_elem]);

   // check set new interval for timer 1
   ecore_timer_interval_set(timer->timer1, timer->interval_1[timer->num_elem]);
   fail_if(timer->interval_1[timer->num_elem] != ecore_timer_interval_get(timer->timer1), "Error set new interval");

   // check set new precision
   ecore_timer_precision_set(timer->precision[timer->num_elem]);
   fail_if(timer->precision[timer->num_elem] != ecore_timer_precision_get(), "Error set new precision");

   // check removal timer 2
   if (ecore_timer_del(timer->timer2))
   { 
     timer->add_timer2 = 0;
     timer->timer2 = NULL;
   }
   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_timer3_cb(void *data)
{
   struct _timers *timer = (struct _timers*)data;

   timer->count_timer3++; // number of starts timer 3

   // check add timer 2
   if (!timer->timer2)
   {
     timer->add_timer2++;   // amount of added timer
     timer->timer2 = ecore_timer_add(TIMEOUT_2, _timer2_cb, timer);
   }

   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_timer4_cb(void *data)
{
   struct _timers *timer = (struct _timers*)data;

   // check frezze/thaw timer 3
   if (freeze_thaw)
   {
      fail_if(timer->check_freeze_thaw_timer3 != timer->count_timer3, "Error frezze/thaw timer");

      ecore_timer_thaw(timer->timer3);
      freeze_thaw = 0;
   }
   else
   {
      ecore_timer_freeze(timer->timer3);
      freeze_thaw = 1;
      timer->check_freeze_thaw_timer3 = timer->count_timer3;
   }

   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_timer5_cb(void *data)
{
   struct _timers *timer = (struct _timers*)data;
   if (timer->timer2) timer->timer2 = NULL;
   ecore_main_loop_quit();
   return ECORE_CALLBACK_RENEW;
}

/*
  Timer2 sets new parameters (delay, interval and precision) for Timer1 and checks them.
  Timer3 creates and runs Timer2 and increases parameter add_timer2 to 1. Timer2 removes himself
  and sets parameter add_timer2 to 0. Inside callback of Timer2 parameter add_timer2 should be
  equal to 1 otherwise "Error add/delay timer". 
  Timer4 controls 'freezing' or 'thaw' for Timer3. Timer4 sets flag freeze_thaw to state: 'freeze' or 'thaw'.
  When Timer3 is frozen counter of Timer3 saved in the parameter check_freeze_thaw_timer3.
  Otherwise (thaw mode) check that parameter check_freeze_thaw_timer3 equals to counter Timer3.
  If not equal then "Error frezze/thaw timer".
  Timer 5 finishes testing.
*/

EFL_START_TEST(ecore_test_timers)
{
   struct _timers timer = \
   {
      .num_elem = 0,
      .delay_1 = {0.01, 0.05, 0.1},
      .interval_1 = {0.01, 0.05, 0.1},
      .precision = {0.01, 0.02, 0.03}
   };

   timer.timer1 = ecore_timer_add(TIMEOUT_1, _timer1_cb, &timer);
   timer.timer2 = ecore_timer_add(TIMEOUT_2, _timer2_cb, &timer);
   timer.add_timer2++;
   timer.timer3 = ecore_timer_add(TIMEOUT_3, _timer3_cb, &timer);
   timer.timer4 = ecore_timer_add(TIMEOUT_4, _timer4_cb, &timer);
   timer.timer5 = ecore_timer_add(TIMEOUT_5, _timer5_cb, &timer);

   fail_if((!timer.timer1 || !timer.timer2 || !timer.timer3 || !timer.timer4 || !timer.timer5), "Error add timer\n");

   ecore_main_loop_begin();

   if (timer.timer1)
     ecore_timer_del(timer.timer1);
   if (timer.timer2)
     ecore_timer_del(timer.timer2);
   if (timer.timer3)
     ecore_timer_del(timer.timer3);
   if (timer.timer4)
     ecore_timer_del(timer.timer4);
   if (timer.timer5)
     ecore_timer_del(timer.timer5);
}
EFL_END_TEST

typedef struct _Test_Inside_Call
{
   Ecore_Timer *t;
   int it;
} Test_Inside_Call;

static Eina_Bool
_timeri_cb(void *data)
{
   Test_Inside_Call *c = data;

   ecore_timer_reset(c->t);

   c->it--;

   if (c->it > 0) return EINA_TRUE;
   free(c);
   ecore_main_loop_quit();
   return EINA_FALSE;
}

static Eina_Bool
timeout_timer_cb()
{
   ck_abort();
   return EINA_FALSE;
}

EFL_START_TEST(ecore_test_timer_inside_call)
{
   Test_Inside_Call *c;

   c = malloc(sizeof(Test_Inside_Call));
   c->it = 5;
   c->t = ecore_timer_add(0.01, _timeri_cb, c);
   ecore_timer_add(1.0, timeout_timer_cb, NULL);

   fail_if(!c->t, "Error add timer\n");

   ecore_main_loop_begin();
}
EFL_END_TEST


EFL_START_TEST(ecore_test_timer_valid_callbackfunc)
{
   Ecore_Timer *t = NULL;
   fail_if((t = ecore_timer_add(0.5, NULL, NULL)), "ERROR: Invalid callback func!\n");
}
EFL_END_TEST

static Eina_Bool
_quit_cb(void *data)
{
   Eina_Bool *val = data;
   if (val) *val = EINA_TRUE;
   ecore_main_loop_quit();
   return EINA_FALSE;
}

EFL_START_TEST(ecore_test_ecore_main_loop_timer)
{
   Eina_Bool did = EINA_FALSE;
   Ecore_Timer *timer;
   double start, end, elapsed;


   timer = ecore_timer_add(0.1, _quit_cb, &did);
   fail_if(timer == NULL);

   start = ecore_time_get();
   ecore_main_loop_begin();
   end = ecore_time_get();
   elapsed = end - start;

   fail_if(did == EINA_FALSE);
   fail_if(elapsed < 0.05);
   fail_if(elapsed > 0.15); /* .05 second "error margin" */

}
EFL_END_TEST

static int count = 0;

static Eina_Bool
_timer_cb(void *data)
{
   count++;
   int num = (intptr_t) data;
   fail_if (num != count, "Error timer is called out of order");
   if (count == 8) ecore_main_loop_quit();
   return ECORE_CALLBACK_CANCEL;
}

EFL_START_TEST(ecore_test_timer_in_order)
{
   Ecore_Timer *timer;
   timer = ecore_timer_add(0.001, _timer_cb, (void *) 1);
   fail_if(timer == NULL);
   timer = ecore_timer_add(0.001, _timer_cb, (void *) 2);
   fail_if(timer == NULL);
   timer = ecore_timer_add(0.001, _timer_cb, (void *) 3);
   fail_if(timer == NULL);
   timer = ecore_timer_add(0.001, _timer_cb, (void *) 4);
   fail_if(timer == NULL);
   timer = ecore_timer_add(0.001, _timer_cb, (void *) 5);
   fail_if(timer == NULL);
   timer = ecore_timer_add(0.001, _timer_cb, (void *) 6);
   fail_if(timer == NULL);
   timer = ecore_timer_add(0.001, _timer_cb, (void *) 7);
   fail_if(timer == NULL);
   timer = ecore_timer_add(0.001, _timer_cb, (void *) 8);
   fail_if(timer == NULL);

   ecore_main_loop_begin();
}
EFL_END_TEST

void ecore_test_timer(TCase *tc)
{
  tcase_add_test(tc, ecore_test_timers);
  tcase_add_test(tc, ecore_test_timer_inside_call);
  tcase_add_test(tc, ecore_test_timer_valid_callbackfunc);
  tcase_add_test(tc, ecore_test_ecore_main_loop_timer);
  tcase_add_test(tc, ecore_test_timer_in_order);
}
