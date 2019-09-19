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
static void
_ecore_signal_job(void *data EINA_UNUSED)
{
   EXPECT_ERROR_START;
   for (unsigned int i = 0; i < 1000; i++)
     raise(SIGUSR2);
   ecore_main_loop_quit();
   EXPECT_ERROR_END;
}

EFL_START_TEST(ecore_test_job_signal)
{
   ecore_job_add(_ecore_signal_job, NULL);

   ecore_main_loop_begin();
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
