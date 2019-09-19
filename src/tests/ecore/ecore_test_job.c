#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Ecore.h>
#include "ecore_suite.h"

static void
_ecore_quit(void *data)
{
   Eina_Bool *bob = data;

   *bob = EINA_TRUE;
   ecore_main_loop_quit();
}

EFL_START_TEST(ecore_test_job)
{
   Eina_Bool bob = EINA_FALSE;
   Ecore_Job *job;

   job = ecore_job_add(&_ecore_quit, &bob);
   fail_if(!job);

   ecore_main_loop_begin();

   fail_if(bob != EINA_TRUE);
}
EFL_END_TEST

#ifndef _WIN32
#define NUM_SIGNALS 2000
static Eina_Bool
_signal_cb(void *data, int t EINA_UNUSED, void *ev EINA_UNUSED)
{
   int *called = data;
   (*called)++;
   if (*called == NUM_SIGNALS) ecore_main_loop_quit();
   return ECORE_CALLBACK_RENEW;
}

static void
_ecore_signal_job(void *data EINA_UNUSED)
{
   for (unsigned int i = 0; i < NUM_SIGNALS; i++)
     raise(SIGUSR2);
}

EFL_START_TEST(ecore_test_job_signal)
{
   int called = 0;
   ecore_job_add(_ecore_signal_job, NULL);
   ecore_event_handler_add(ECORE_EVENT_SIGNAL_USER, _signal_cb, &called);

   ecore_main_loop_begin();
   ck_assert_int_eq(called, NUM_SIGNALS);
}
EFL_END_TEST
#endif

void ecore_test_ecore_job(TCase *tc)
{
   tcase_add_test(tc, ecore_test_job);
#ifndef _WIN32
   tcase_add_test(tc, ecore_test_job_signal);
#endif
}
