#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Ecore.h>
#include "ecore_suite.h"

static void
_pipe_handler(void *data, void *buf EINA_UNUSED, unsigned int len EINA_UNUSED)
{
   Eina_Bool *bob = data;

   *bob = EINA_TRUE;
   ecore_main_loop_quit();
}

EFL_START_TEST(ecore_test_pipe)
{
   Ecore_Pipe *pipe;
   Eina_Bool bob = EINA_FALSE;

   pipe = ecore_pipe_add(_pipe_handler, &bob);
   fail_if(!pipe);

   ecore_pipe_write(pipe, &bob, sizeof(Eina_Bool));
   ecore_main_loop_begin();
   ck_assert_int_eq(bob, EINA_TRUE);

   bob = EINA_FALSE;
   ecore_pipe_write(pipe, NULL, 0);
   ecore_main_loop_begin();
   ck_assert_int_eq(bob, EINA_TRUE);

   ecore_pipe_del(pipe);
}
EFL_END_TEST

void ecore_test_ecore_pipe(TCase *tc)
{
   tcase_add_test(tc, ecore_test_pipe);
}
