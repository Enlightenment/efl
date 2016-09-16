#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Ecore.h>
#include "ecore_suite.h"

static void
_ecore_promise_quit(void *data, const Efl_Event *ev)
{
   Efl_Future_Event_Success *success = ev->info;
   Eina_Bool *bob = data;
   void *value = success->value;

   fail_if(data != value);
   *bob = EINA_TRUE;
   ecore_main_loop_quit();
}

START_TEST(ecore_test_job_promise)
{
   Eina_Bool bob = EINA_FALSE;

   ecore_init();

   Efl_Object *promise = efl_loop_job(ecore_main_loop_get(), &bob);
   efl_future_then(promise, &_ecore_promise_quit, NULL, NULL, &bob);

   ecore_main_loop_begin();

   fail_if(bob != EINA_TRUE);

   ecore_shutdown();
}
END_TEST

static void
_ecore_quit(void *data)
{
   Eina_Bool *bob = data;

   *bob = EINA_TRUE;
   ecore_main_loop_quit();
}

START_TEST(ecore_test_job)
{
   Eina_Bool bob = EINA_FALSE;
   Ecore_Job *job;

   ecore_init();

   job = ecore_job_add(&_ecore_quit, &bob);
   fail_if(!job);

   ecore_main_loop_begin();

   fail_if(bob != EINA_TRUE);

   ecore_shutdown();
}
END_TEST

void ecore_test_ecore_job(TCase *tc)
{
   tcase_add_test(tc, ecore_test_job);
   tcase_add_test(tc, ecore_test_job_promise);
}
