#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <check.h>
#define EFL_NOLEGACY_API_SUPPORT
#include <Efl_Ui.h>
#include "../efl_check.h"

EAPI_MAIN void
efl_main(void *data EINA_UNUSED,
         const Efl_Event *ev)
{
   Efl_Loop_Arguments *arge = ev->info;

   fail_if(!arge->initialization);
   fprintf(stderr, "ARGC %d\n", eina_array_count(arge->argv));
   fail_if(eina_array_count(arge->argv) != 2);
   fail_if(!eina_streq(eina_array_data_get(arge->argv, 1), "test"));

   efl_loop_quit(ev->object, eina_value_string_init("success"));
}

EFL_START_TEST(efl_ui_test_init)
{
   /* EFL_MAIN */
   Eina_Value *ret__;
   int real__;

   int argc = 2;
   char *argv[] = { "efl_ui_suite", "test" };
   _efl_startup_time = ecore_time_unix_get();
   _EFL_APP_VERSION_SET();
   fail_if(!ecore_init());
   efl_event_callback_add(efl_app_main_get(EFL_APP_CLASS), EFL_LOOP_EVENT_ARGUMENTS, efl_main, NULL);
   fail_if(!ecore_init_ex(argc, argv));
   __EFL_MAIN_CONSTRUCTOR;
   ret__ = efl_loop_begin(efl_app_main_get(EFL_APP_CLASS));
   real__ = efl_loop_exit_code_process(ret__);
   fail_if(real__ != 0);
   __EFL_MAIN_DESTRUCTOR;
   ecore_shutdown_ex();
   ecore_shutdown();
}
EFL_END_TEST

void efl_ui_test(TCase *tc)
{
   tcase_add_test(tc, efl_ui_test_init);
}


static const Efl_Test_Case etc[] = {
  { "Efl_Ui", efl_ui_test },
  { NULL, NULL }
};

SUITE_INIT(efl_ui)
{
   //???
}

SUITE_SHUTDOWN(efl_ui)
{

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
                                           "Efl_Ui", etc, SUITE_INIT_FN(efl_ui), SUITE_SHUTDOWN_FN(efl_ui));

   return (failed_count == 0) ? 0 : 255;
}
