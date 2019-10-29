#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <unistd.h>
#define EFL_NOLEGACY_API_SUPPORT
#include <Efl_Core.h>
#include "efl_app_suite.h"
#include "../efl_check.h"

/*
static Eina_Bool
_test_time_cb(void *data)
{
   Eina_Bool *run = data;

   *run = EINA_TRUE;

   return EINA_TRUE;
}

static void
_test_death_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Eina_Bool *die = data;

   *die = EINA_TRUE;
}

static void
_test_run_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   _test_time_cb(data);
}

EFL_START_TEST(ecore_test_timer_lifecycle)
{
   Eina_Bool rl = EINA_FALSE, re = EINA_FALSE;
   Eina_Bool dl = EINA_FALSE, de = EINA_FALSE;
   Ecore_Timer *t;
   Eo *et;

   efl_object_init();

   t = ecore_timer_add(1.0, _test_time_cb, &rl);
   efl_event_callback_add((Eo*) t, EFL_EVENT_DEL, _test_death_cb, &dl);

   et = efl_add(EFL_LOOP_TIMER_CLASS, efl_main_loop_get(),
               efl_event_callback_add(efl_added, EFL_LOOP_TIMER_EVENT_TIMER_TICK, _test_run_cb, &re),
               efl_event_callback_add(efl_added, EFL_EVENT_DEL, _test_death_cb, &de),
               efl_loop_timer_interval_set(efl_added, 1.0));
   efl_ref(et);

   fail_if(re == EINA_TRUE && rl == EINA_TRUE);
   fail_if(dl == EINA_FALSE);
   fail_if(de == EINA_TRUE);

   efl_del(et);
   fail_if(de == EINA_FALSE);

   efl_object_shutdown();
}
EFL_END_TEST
*/

EFL_START_TEST(efl_app_test_loop_timer_invalid)
{
   Efl_Object *t = efl_add(EFL_LOOP_TIMER_CLASS, efl_app_main_get());
   fail_if(t != NULL);
}
EFL_END_TEST



static int count = 0;

static void
_timer_cb(void *data EINA_UNUSED, const Efl_Event *ev EINA_UNUSED)
{
   count++;
}

static void
array_clean(Eina_Array *arr)
{
   Eo *t;

   while ((t = eina_array_pop(arr)))
     efl_del(t);
}

EFL_START_TEST(efl_loop_test_loop_timer_iteration)
{
   Eo *t, *loop;
   Eina_Array *arr;

   loop = efl_main_loop_get();
   count = 0;
   arr = eina_array_new(5);
#define TIMER(duration) \
  t = efl_add(EFL_LOOP_TIMER_CLASS, loop, \
    efl_loop_timer_interval_set(efl_added, (duration)), \
    efl_event_callback_add(efl_added, EFL_LOOP_TIMER_EVENT_TIMER_TICK, _timer_cb, NULL) \
  ); \
  eina_array_push(arr, t)
   /* verify that timers expire after exactly one loop iteration */
   TIMER(0);
   efl_loop_iterate(loop);
   /* timers should not expire for one loop iteration */
   ck_assert_int_eq(count, 0);
   efl_loop_iterate(loop);
   /* timers should expire after one loop iteration */
   ck_assert_int_eq(count, 1);
   array_clean(arr);

   count = 0;
   /* verify multiple timer expiration in single mainloop iteration */
   TIMER(0);
   TIMER(0);
   efl_loop_iterate(loop);
   /* timers should not expire for one loop iteration */
   ck_assert_int_eq(count, 0);
   TIMER(0);
   efl_loop_iterate(loop);
   /* all pending and instantiated timers should expire after exactly one loop iteration */
   ck_assert_int_eq(count, 2);
   efl_loop_iterate(loop);
   /* this should not interfere with successive timer processing */
   ck_assert_int_eq(count, 5);
   array_clean(arr);

   count = 0;
   /* verify out-of-order timer processing solely based on timer times */
   TIMER(1);
   efl_loop_iterate(loop);
   ck_assert_int_eq(count, 0);
   TIMER(0);
   efl_loop_iterate(loop);
   ck_assert_int_eq(count, 0);
   /* timer should expire after exactly 2 iterations */
   efl_loop_iterate(loop);
   ck_assert_int_eq(count, 1);
   efl_loop_timer_interval_set(eina_array_data_get(arr, 0), 0);
   efl_loop_timer_reset(eina_array_data_get(arr, 0));
   /* timer should expire after exactly 2 iterations since it becomes un-instantiated now */
   efl_loop_iterate(loop);
   efl_loop_iterate(loop);
   ck_assert_int_eq(count, 4);
   array_clean(arr);
}
EFL_END_TEST

#undef TIMER
#define TIMER(duration) \
  efl_add(EFL_LOOP_TIMER_CLASS, loop, \
    efl_loop_timer_interval_set(efl_added, (duration)), \
    efl_event_callback_add(efl_added, EFL_LOOP_TIMER_EVENT_TIMER_TICK, _timer_del_cb, NULL) \
  ); \

static void
_timer_del_cb(void *data EINA_UNUSED, const Efl_Event *ev)
{
   count++;
   efl_del(ev->object);
}


static void
_recursion(void *data EINA_UNUSED, const Efl_Event *ev)
{
   static unsigned int recurse = 0;
   static Eo *timer;
   Eo *loop = efl_main_loop_get();

   switch (recurse++)
     {
      case 0:
        /* verify multiple timer expiration in single mainloop iteration */
        TIMER(0);
        TIMER(0);
        efl_loop_iterate(loop);
        efl_loop_iterate(loop);
        ck_assert_int_eq(count, 6);
        efl_loop_quit(loop, EINA_VALUE_EMPTY);
        break;
      case 1:
        /* timers should not expire for one loop iteration */
        ck_assert_int_eq(count, 1);
        TIMER(0);
        efl_loop_iterate(loop);
        efl_loop_iterate(loop);
        break;
      case 2:
        /* all pending and instantiated timers should expire after exactly one loop iteration */
        ck_assert_int_eq(count, 3);
        efl_loop_iterate(loop);
        efl_loop_iterate(loop);
        break;
      case 3:
        /* this should not interfere with successive timer processing */
        ck_assert_int_eq(count, 4);

        /* verify out-of-order timer processing solely based on timer times */
        timer = TIMER(1);
        efl_loop_iterate(loop);
        efl_loop_iterate(loop);
        break;
      case 4:
        ck_assert_int_eq(count, 4);
        TIMER(0);
        efl_loop_iterate(loop);
        efl_loop_iterate(loop);
        break;
      case 5:
        ck_assert_int_eq(count, 4);
        /* timer should expire after exactly 2 iterations */
        efl_loop_iterate(loop);
        efl_loop_iterate(loop);
        break;
      case 6:
        ck_assert_int_eq(count, 5);
        efl_loop_timer_interval_set(timer, 0);
        efl_loop_timer_reset(timer);
        /* timer should expire after exactly 2 iterations since it becomes un-instantiated now */
        efl_loop_iterate(loop);
        efl_loop_iterate(loop);
        break;
      case 7:
        efl_loop_iterate(loop);
        efl_loop_iterate(loop);
        efl_del(ev->object);
        break;
     }
}

EFL_START_TEST(efl_loop_test_loop_timer_recursion)
{
   Eina_Bool crit = eina_log_abort_on_critical_get();
   int critlevel = eina_log_abort_on_critical_level_get();
   Eo *loop = efl_main_loop_get();
   count = 1;
   eina_log_abort_on_critical_set(1);
   eina_log_abort_on_critical_level_set(1);
   efl_add(EFL_LOOP_TIMER_CLASS, loop,
    efl_loop_timer_interval_set(efl_added, 0),
    efl_event_callback_add(efl_added, EFL_LOOP_TIMER_EVENT_TIMER_TICK, _recursion, NULL)
   );
   efl_loop_begin(loop);
   ck_assert_int_eq(count, 6);
   eina_log_abort_on_critical_set(crit);
   eina_log_abort_on_critical_level_set(critlevel);
}
EFL_END_TEST

void efl_app_test_efl_loop_timer(TCase *tc EINA_UNUSED)
{
  /* XXX: this seems a silly test - that we del the loop object?
    tcase_add_test(tc, ecore_test_timer_lifecycle);
   */

   tcase_add_test(tc, efl_app_test_loop_timer_invalid);
   tcase_add_test(tc, efl_loop_test_loop_timer_iteration);
   tcase_add_test(tc, efl_loop_test_loop_timer_recursion);
}
