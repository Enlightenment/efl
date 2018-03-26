#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <unistd.h>
#define EFL_NOLEGACY_API_SUPPORT
#include <Efl_Core.h>
#include <Efl_Net.h>
#include "efl_app_suite.h"
#include "../efl_check.h"

EFL_START_TEST(efl_app_test_efl_loop_register)
{
   Efl_Object *t, *n;

   ecore_init();

   t = efl_provider_find(efl_app_get(), EFL_LOOP_CLASS);
   fail_if(!efl_isa(t, EFL_LOOP_CLASS));
   fail_if(!efl_isa(t, EFL_APP_CLASS));

   t = efl_provider_find(efl_app_get(), EFL_LOOP_TIMER_CLASS);
   fail_if(t != NULL);

   n = efl_add(EFL_LOOP_TIMER_CLASS, efl_app_get());
   fail_if(n != NULL);

   n = efl_add(EFL_LOOP_TIMER_CLASS, efl_app_get(),
               efl_loop_timer_interval_set(efl_added, 1.0));
   efl_loop_register(efl_app_get(), EFL_LOOP_TIMER_CLASS, n);

   t = efl_provider_find(efl_app_get(), EFL_LOOP_TIMER_CLASS);
   fail_if(!efl_isa(t, EFL_LOOP_TIMER_CLASS));
   fail_if(t != n);

   efl_loop_unregister(efl_app_get(), EFL_LOOP_TIMER_CLASS, n);

   t = efl_provider_find(efl_app_get(), EFL_LOOP_TIMER_CLASS);
   fail_if(t != NULL);

   ecore_shutdown();
}
EFL_END_TEST

EFL_START_TEST(efl_app_test_efl_build_version)
{
   const Efl_Version *ver;
   Eo *app;

   ecore_init();

   app = efl_app_get();
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
   tcase_add_test(tc, efl_app_test_efl_loop_register);
   tcase_add_test(tc, efl_app_test_efl_build_version);
}


static const Efl_Test_Case etc[] = {
  { "Efl_App", efl_test_efl_app },
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
                                           "Efl_App", etc);

   return (failed_count == 0) ? 0 : 255;
}
