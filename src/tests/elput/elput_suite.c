#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "elput_suite.h"
#include "../efl_check.h"
#include <Elput.h>

static const Efl_Test_Case etc[] =
{
#if HAVE_ELPUT
   { "Elput", elput_test_elput },
#endif
   { NULL, NULL }
};

SUITE_INIT(elput)
{
   ck_assert_int_eq(elput_init(), 1);
}

SUITE_SHUTDOWN(elput)
{
   ck_assert_int_eq(elput_shutdown(), 0);
}

int
main(int argc, char **argv)
{
   int count;

   if (!_efl_test_option_disp(argc, argv, etc)) return 0;

#ifdef NEED_RUN_IN_TREE
   putenv("EFL_RUN_IN_TREE=1");
#endif

   count =
     _efl_suite_build_and_run(argc - 1, (const char **)argv + 1, "Elput", etc, SUITE_INIT_FN(elput), SUITE_SHUTDOWN_FN(elput));

   return (count == 0) ? 0 : 255;
}
