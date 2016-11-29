#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "eo_suite.h"
#include "../../efl_check.h"

static const Efl_Test_Case etc[] = {
  { "Eo init", eo_test_init },
  { "Eo general", eo_test_general },
  { "Eo class errors", eo_test_class_errors },
  { "Eo class behaviour errors", eo_test_class_behaviour_errors },
  { "Eo call errors", eo_test_call_errors },
  { "Eo eina value", eo_test_value },
  { "Eo threaded eo calls", eo_test_threaded_calls },
  { "Eo event calls", eo_test_event},
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
                                           "Eo", etc);

   return (failed_count == 0) ? 0 : 255;
}
