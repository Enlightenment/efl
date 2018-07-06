#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "edje_suite.h"
#include "../efl_check.h"
#include <Edje.h>

static const Efl_Test_Case etc[] = {
  { "Edje", edje_test_edje },
  { NULL, NULL }
};

SUITE_INIT(edje)
{
   ck_assert_int_eq(edje_init(), 1);
}

SUITE_SHUTDOWN(edje)
{
   ck_assert_int_eq(edje_shutdown(), 0);
}

int
main(int argc, char **argv)
{
   int failed_count;

   if (!_efl_test_option_disp(argc, argv, etc))
     return 0;

#ifdef NEED_RUN_IN_TREE
   putenv("EFL_RUN_IN_TREE=1");
#endif

   failed_count = _efl_suite_build_and_run(argc - 1, (const char **)argv + 1,
                                           "Edje", etc, SUITE_INIT_FN(edje), SUITE_SHUTDOWN_FN(edje));

   return (failed_count == 0) ? 0 : 255;
}
