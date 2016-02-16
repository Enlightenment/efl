#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "elua_suite.h"
#include "../efl_check.h"

static const Efl_Test_Case etc[] = {
  { "Elua Library", elua_lib_test},
  { NULL, NULL }
};

int
main(int argc, char **argv)
{
   int failed_count;

   setenv("CK_FORK", "no", 0);

   if (!_efl_test_option_disp(argc, argv, etc))
     return 0;

   putenv("EFL_RUN_IN_TREE=1");

   failed_count = _efl_suite_build_and_run(argc - 1, (const char **)argv + 1,
                                           "Elua", etc);

   return (failed_count == 0) ? 0 : 255;
}
