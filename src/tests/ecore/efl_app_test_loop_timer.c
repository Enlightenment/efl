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

void efl_app_test_efl_loop_timer(TCase *tc EINA_UNUSED)
{
  /* XXX: this seems a silly test - that we del the loop object?
    tcase_add_test(tc, ecore_test_timer_lifecycle);
   */

   tcase_add_test(tc, efl_app_test_loop_timer_invalid);
}
