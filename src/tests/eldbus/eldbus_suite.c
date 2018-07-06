#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "eldbus_suite.h"
#include "../efl_check.h"
#include <Eldbus.h>

static const Efl_Test_Case etc[] = {
  { "eldbus_init", eldbus_test_eldbus_init },
  { "eldbus_model", eldbus_test_eldbus_model },
  { "eldbus_model_connection", eldbus_test_eldbus_model_connection },
  { "eldbus_model_object", eldbus_test_eldbus_model_object },
  { "eldbus_model_proxy", eldbus_test_eldbus_model_proxy },
  { "eldbus_test_fake_server_eldbus_model_proxy", eldbus_test_fake_server_eldbus_model_proxy },
  { "eldbus_model_method", eldbus_test_eldbus_model_method },
  { "eldbus_model_signal", eldbus_test_eldbus_model_signal },
  { "eldbus_test_eldbus_object", eldbus_test_eldbus_object },
  { "eldbus_test_eldbus_message", eldbus_test_eldbus_message },
  { "eldbus_test_eldbus_signal_handler", eldbus_test_eldbus_signal_handler },
  { "eldbus_test_eldbus_proxy", eldbus_test_eldbus_proxy },
  { "eldbus_test_eldbus_pending_cancel", eldbus_test_eldbus_pending_cancel },
  { NULL, NULL }
};

SUITE_INIT(eldbus)
{
   ck_assert_int_eq(eldbus_init(), 1);
}

SUITE_SHUTDOWN(eldbus)
{
   //T6814
   ck_assert_int_eq(eldbus_shutdown(), 0);
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
                                           "Eldbus", etc, SUITE_INIT_FN(eldbus), SUITE_SHUTDOWN_FN(eldbus));

   return (failed_count == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
