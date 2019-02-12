#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <unistd.h>
#define EFL_NOLEGACY_API_SUPPORT
#include <Efl_Core.h>
#include "efl_app_suite.h"
#include "../efl_check.h"

EFL_START_TEST(efl_app_test_efl_build_version)
{
   const Efl_Version *ver;
   Eo *app;

   ecore_init();

   app = efl_app_main_get(EFL_APP_CLASS);
   fail_if(!efl_isa(app, EFL_APP_CLASS));

   efl_build_version_set(EFL_VERSION_MAJOR, EFL_VERSION_MINOR, 0, 0, NULL, EFL_BUILD_ID);
   ver = efl_app_build_efl_version_get(app);
   fail_if(!ver);
   fail_if(ver->major != EFL_VERSION_MAJOR);
   fail_if(ver->minor != EFL_VERSION_MINOR);
   fail_if(ver->micro != 0);
   fail_if(ver->revision != 0);
   fail_if(ver->flavor);
   fail_if(!eina_streq(ver->build_id, EFL_BUILD_ID));

   ver = efl_app_efl_version_get(app);
   fail_if(!ver);
   fail_if(ver->major != EFL_VERSION_MAJOR);
   fail_if(ver->minor != EFL_VERSION_MINOR);

   ecore_shutdown();
}
EFL_END_TEST

void efl_test_efl_app(TCase *tc)
{
   tcase_add_test(tc, efl_app_test_efl_build_version);
}

static const Efl_Test_Case etc[] = {
  { "App", efl_test_efl_app },
  { "Loop", efl_app_test_efl_loop },
  { "Loop_Timer", efl_app_test_efl_loop_timer },
  { "Loop_FD", efl_app_test_efl_loop_fd },
  { "Promise", efl_app_test_promise },
  { "Promise", efl_app_test_promise_2 },
  { "Promise", efl_app_test_promise_3 },
  { "Promise", efl_app_test_promise_safety },
  { "Env", efl_test_efl_env },
  { "CML", efl_test_efl_cml },
  { NULL, NULL }
};

SUITE_INIT(efl_app)
{
   ck_assert_int_eq(ecore_init(), 1);
}

SUITE_SHUTDOWN(efl_app)
{
   ck_assert_int_eq(ecore_shutdown(), 0);
}

int
main(int argc, char **argv)
{
   int failed_count;

   if (!_efl_test_option_disp(argc, argv, etc))
     return 0;

   eina_init();
   efl_object_init();
#ifdef NEED_RUN_IN_TREE
   putenv("EFL_RUN_IN_TREE=1");
#endif

   failed_count = _efl_suite_build_and_run(argc - 1, (const char **)argv + 1,
                                           "Efl_App", etc, SUITE_INIT_FN(efl_app), SUITE_SHUTDOWN_FN(efl_app));

   efl_object_shutdown();
   eina_shutdown();
   return (failed_count == 0) ? 0 : 255;
}
