#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "eo_suite.h"
#include "../../efl_check.h"
#include <eo_private.h>
#include <Eo.h>
#include <Eina.h>

int _eo_log_dom = -1;

static const Efl_Test_Case etc[] = {
  { "Eo init", eo_test_init },
  { "Eo general", eo_test_general },
  { "Eo class errors", eo_test_class_errors },
  { "Eo class behaviour errors", eo_test_class_behaviour_errors },
  { "Eo call errors", eo_test_call_errors },
  { "Eo eina value", eo_test_value },
  { "Eo threaded eo calls", eo_test_threaded_calls },
  { "Eo event calls", eo_test_event},
  { "Eo lifecycle", eo_test_lifecycle},
  { NULL, NULL }
};

SUITE_INIT(efl_object)
{
   ck_assert_int_eq(efl_object_init(), 1);
   _eo_log_dom = eina_log_domain_register("eo-log", EINA_COLOR_LIGHTBLUE);
}

SUITE_SHUTDOWN(efl_object)
{
   eina_log_domain_unregister(_eo_log_dom);
   ck_assert_int_eq(efl_object_shutdown(), 0);
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
                                           "Eo", etc, SUITE_INIT_FN(efl_object), SUITE_SHUTDOWN_FN(efl_object));

   return (failed_count == 0) ? 0 : 255;
}
