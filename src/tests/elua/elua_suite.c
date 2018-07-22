#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "elua_suite.h"
#include "../efl_check.h"
#include <Elua.h>

static const Efl_Test_Case etc[] = {
  { "Elua Library", elua_lib_test},
  { NULL, NULL }
};

SUITE_INIT(elua)
{
   ck_assert_int_eq(elua_init(), 1);
}

SUITE_SHUTDOWN(elua)
{
   ck_assert_int_eq(elua_shutdown(), 0);
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
                                           "Elua", etc, SUITE_INIT_FN(elua), SUITE_SHUTDOWN_FN(elua));

   return (failed_count == 0) ? 0 : 255;
}
