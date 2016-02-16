#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "eio_suite.h"
#include "../efl_check.h"

static const Efl_Test_Case etc[] = {
  {"Eio_Monitor", eio_test_monitor},
  {"Eio Model", eio_model_test_file},
  {"Eio Model Monitor", eio_model_test_monitor_add},
  {"Eio File", eio_test_file},
#ifdef XATTR_TEST_DIR
  {"Eio_Xattr", eio_test_xattr},
#endif
  {NULL, NULL}
};

int
main(int argc, char **argv)
{
   int failed_count;

   if (!_efl_test_option_disp(argc, argv, etc))
     return 0;

   putenv("EFL_RUN_IN_TREE=1");

   failed_count = _efl_suite_build_and_run(argc - 1, (const char **)argv + 1,
                                           "Eio", etc);

   return (failed_count == 0) ? 0 : 255;
}
