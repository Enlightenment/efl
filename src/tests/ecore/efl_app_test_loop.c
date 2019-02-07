#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <unistd.h>
#define EFL_NOLEGACY_API_SUPPORT
#include <Efl_Core.h>
#include "efl_app_suite.h"
#include "../efl_check.h"

EFL_START_TEST(efl_app_test_efl_loop_register)
{
   Efl_Object *t, *n;

   ecore_init();

   t = efl_provider_find(efl_app_main_get(EFL_APP_CLASS), EFL_LOOP_CLASS);
   fail_if(!efl_isa(t, EFL_LOOP_CLASS));
   fail_if(!efl_isa(t, EFL_APP_CLASS));

   t = efl_provider_find(efl_app_main_get(EFL_APP_CLASS), EFL_LOOP_TIMER_CLASS);
   fail_if(t != NULL);

   n = efl_add(EFL_LOOP_TIMER_CLASS, efl_app_main_get(EFL_APP_CLASS),
               efl_loop_timer_interval_set(efl_added, 1.0));
   efl_loop_register(efl_app_main_get(EFL_APP_CLASS), EFL_LOOP_TIMER_CLASS, n);

   t = efl_provider_find(efl_app_main_get(EFL_APP_CLASS), EFL_LOOP_TIMER_CLASS);
   fail_if(!efl_isa(t, EFL_LOOP_TIMER_CLASS));
   fail_if(t != n);

   efl_loop_unregister(efl_app_main_get(EFL_APP_CLASS), EFL_LOOP_TIMER_CLASS, n);

   t = efl_provider_find(efl_app_main_get(EFL_APP_CLASS), EFL_LOOP_TIMER_CLASS);
   fail_if(t != NULL);

   ecore_shutdown();
}
EFL_END_TEST

static void
efl_app_test_efl_loop_concentric_fail(void *data EINA_UNUSED, const Efl_Event *ev EINA_UNUSED)
{
   ck_abort_msg("test timeout");
}

static void
loop_idle_enter(void *data EINA_UNUSED, const Efl_Event *ev EINA_UNUSED)
{
   static int num = 0;

   if (num++ == 5) efl_loop_quit(efl_main_loop_get(), eina_value_int_init(0));
}

static void
loop_idle(void *data, const Efl_Event *ev EINA_UNUSED)
{
   efl_loop_iterate(data);
}

static void
loop_timer_tick(void *data, const Efl_Event *ev EINA_UNUSED)
{
   efl_loop_iterate(data);
}

EFL_START_TEST(efl_app_test_efl_loop_concentric)
{
   Eo *loop, *loop2, *timer, *timer2;
   int exitcode;

   loop = efl_main_loop_get();
   loop2 = efl_add(EFL_LOOP_CLASS, loop);
   efl_event_callback_add(loop, EFL_LOOP_EVENT_IDLE, loop_idle, loop2);
   efl_event_callback_add(loop, EFL_LOOP_EVENT_IDLE_ENTER, loop_idle_enter, NULL);
   timer = efl_add(EFL_LOOP_TIMER_CLASS, loop2,
     efl_loop_timer_interval_set(efl_added, 0.01),
     efl_event_callback_add(efl_added, EFL_LOOP_TIMER_EVENT_TICK, loop_timer_tick, loop)
     );
   timer2 = efl_add(EFL_LOOP_TIMER_CLASS, loop,
     efl_loop_timer_interval_set(efl_added, 0.5),
     efl_event_callback_add(efl_added, EFL_LOOP_TIMER_EVENT_TICK, efl_app_test_efl_loop_concentric_fail, NULL)
     );
   exitcode = efl_loop_exit_code_process(efl_loop_begin(loop));
   ck_assert_int_eq(exitcode, 0);
   efl_del(timer2);
   efl_del(timer);
   efl_del(loop2);
}
EFL_END_TEST

void efl_app_test_efl_loop(TCase *tc)
{
   tcase_add_test(tc, efl_app_test_efl_loop_register);
   tcase_add_test(tc, efl_app_test_efl_loop_concentric);
}
