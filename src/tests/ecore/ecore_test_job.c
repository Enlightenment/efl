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

void ecore_test_ecore_job(TCase *tc)
{
   tcase_add_test(tc, ecore_test_job);
}
