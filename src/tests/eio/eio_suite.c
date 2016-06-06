#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Eina.h>

#include "eio_suite.h"
#include "../efl_check.h"

static const Efl_Test_Case etc[] = {
  {"Eio_Monitor", eio_test_monitor},
  {"Eio_Sentry", eio_test_sentry},
  {"Eio Model", eio_model_test_file},
  {"Eio Model Monitor", eio_model_test_monitor_add},
  {"Eio File", eio_test_file},
  {"Eio Job", eio_test_job},
#ifdef XATTR_TEST_DIR
  {"Eio_Xattr", eio_test_xattr},
  {"Eio Job Xattr", eio_test_job_xattr},
#endif
  {"Eio_Map", eio_test_map},
  {"Eio_Eet", eio_test_eet},
  {NULL, NULL}
};

int
main(int argc, char **argv)
{
   int failed_count;

   if (!_efl_test_option_disp(argc, argv, etc))
     return 0;

   putenv("EFL_RUN_IN_TREE=1");

   eina_init();

   failed_count = _efl_suite_build_and_run(argc - 1, (const char **)argv + 1,
                                           "Eio", etc);

   eina_shutdown();

   return (failed_count == 0) ? 0 : 255;
}
