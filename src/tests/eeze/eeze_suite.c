#ifdef HAVE_CONFIG_H
# include <config.h>
#endif /* ifdef HAVE_CONFIG_H */

#include "eeze_suite.h"
#include "../efl_check.h"
#include <Eeze.h>

static const Efl_Test_Case etc[] = {
  { "Init", eeze_test_init },
  { "UDev", eeze_test_udev },
  { "Net", eeze_test_net },
  { "Sensor", eeze_test_sensor },
  { NULL, NULL }
};

SUITE_INIT(eeze)
{
   ck_assert_int_eq(eeze_init(), 1);
}

SUITE_SHUTDOWN(eeze)
{
   ck_assert_int_eq(eeze_shutdown(), 0);
}

int
main(int argc, char *argv[])
{
   int failed_count;

   if (!_efl_test_option_disp(argc, argv, etc))
     return 0;

#ifdef NEED_RUN_IN_TREE
   putenv("EFL_RUN_IN_TREE=1");
#endif

   failed_count = _efl_suite_build_and_run(argc - 1, (const char **)argv + 1,
                                           "Eeze", etc, SUITE_INIT_FN(eeze), SUITE_SHUTDOWN_FN(eeze));

   return (failed_count == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

