#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ethumb_suite.h"

static const Efl_Test_Case etc[] = {
  { "ethumb_slave", ethumb_test_slave },
  { NULL, NULL }
};

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
                                           "Ethumb", etc, NULL, NULL);

   return (failed_count == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
