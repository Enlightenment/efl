#ifdef HAVE_CONFIG_H
# include <config.h>
#endif /* ifdef HAVE_CONFIG_H */

#include "eeze_suite.h"
#include "../efl_check.h"

static const Efl_Test_Case etc[] = {
  { "Init", eeze_test_init },
  { "UDev", eeze_test_udev },
  { "Net", eeze_test_net },
  { "Sensor", eeze_test_sensor },
  { NULL, NULL }
};

int
main(int argc, char *argv[])
{
   int failed_count;

   if (!_efl_test_option_disp(argc, argv, etc))
     return 0;

   putenv("EFL_RUN_IN_TREE=1");

   failed_count = _efl_suite_build_and_run(argc - 1, (const char **)argv + 1,
                                           "Eeze", etc);

   return (failed_count == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

