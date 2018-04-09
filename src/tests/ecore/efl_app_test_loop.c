#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <unistd.h>
#define EFL_NOLEGACY_API_SUPPORT
#include <Efl_Core.h>
#include <Efl_Net.h>
#include "efl_app_suite.h"
#include "../efl_check.h"

EFL_START_TEST(efl_app_test_efl_loop_register)
{
   Efl_Object *t, *n;

   ecore_init();

   t = efl_provider_find(efl_app_get(), EFL_LOOP_CLASS);
   fail_if(!efl_isa(t, EFL_LOOP_CLASS));
   fail_if(!efl_isa(t, EFL_APP_CLASS));

   t = efl_provider_find(efl_app_get(), EFL_LOOP_TIMER_CLASS);
   fail_if(t != NULL);

   n = efl_add(EFL_LOOP_TIMER_CLASS, efl_app_get(),
               efl_loop_timer_interval_set(efl_added, 1.0));
   efl_loop_register(efl_app_get(), EFL_LOOP_TIMER_CLASS, n);

   t = efl_provider_find(efl_app_get(), EFL_LOOP_TIMER_CLASS);
   fail_if(!efl_isa(t, EFL_LOOP_TIMER_CLASS));
   fail_if(t != n);

   efl_loop_unregister(efl_app_get(), EFL_LOOP_TIMER_CLASS, n);

   t = efl_provider_find(efl_app_get(), EFL_LOOP_TIMER_CLASS);
   fail_if(t != NULL);

   ecore_shutdown();
}
EFL_END_TEST

void efl_app_test_efl_loop(TCase *tc)
{
   tcase_add_test(tc, efl_app_test_efl_loop_register);
}
