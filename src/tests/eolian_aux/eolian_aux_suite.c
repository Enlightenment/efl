#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef _WIN32
# include <Evil.h> /* setenv */
#endif

#include "eolian_aux_suite.h"
#include "../efl_check.h"
#include <Eolian.h>

static const Efl_Test_Case etc[] = {
  { "Eolian Aux", eolian_aux_test},
  { NULL, NULL }
};

SUITE_INIT(eolian_aux)
{
   ck_assert_int_eq(eolian_init(), 1);
}

SUITE_SHUTDOWN(eolian_aux)
{
   ck_assert_int_eq(eolian_shutdown(), 0);
}

int
main(int argc, char **argv)
{
   int failed_count;

   setenv("CK_FORK", "no", 0);

   if (!_efl_test_option_disp(argc, argv, etc))
     return 0;

#ifdef NEED_RUN_IN_TREE
   putenv("EFL_RUN_IN_TREE=1");
#endif

   failed_count = _efl_suite_build_and_run(argc - 1, (const char **)argv + 1,
                                           "Eolian_Aux", etc, SUITE_INIT_FN(eolian_aux), SUITE_SHUTDOWN_FN(eolian_aux));

   return (failed_count == 0) ? 0 : 255;
}
