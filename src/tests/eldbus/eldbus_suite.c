#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "eldbus_suite.h"
#include "../efl_check.h"

static const Efl_Test_Case etc[] = {
  { "eldbus_init", eldbus_test_eldbus_init },
  { "eldbus_model", eldbus_test_eldbus_model },
  { "eldbus_model_connection", eldbus_test_eldbus_model_connection },
  { "eldbus_model_object", eldbus_test_eldbus_model_object },
  { "eldbus_model_proxy", eldbus_test_eldbus_model_proxy },
  { "eldbus_test_fake_server_eldbus_model_proxy", eldbus_test_fake_server_eldbus_model_proxy },
  { "eldbus_model_method", eldbus_test_eldbus_model_method },
  { "eldbus_model_signal", eldbus_test_eldbus_model_signal },
  { NULL, NULL }
};

int
main(int argc, char **argv)
{
   int failed_count;

   if (!_efl_test_option_disp(argc, argv, etc))
     return 0;

   putenv("EFL_RUN_IN_TREE=1");

   failed_count = _efl_suite_build_and_run(argc - 1, (const char **)argv + 1,
                                           "Eldbus", etc);

   return (failed_count == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
