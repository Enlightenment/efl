#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "efreet_suite.h"
#include "../efl_check.h"

static const Efl_Test_Case etc[] = {
  { "Efreet", efreet_test_efreet },
  { "Efreet Cache", efreet_test_efreet_cache },
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
                                           "Efreet", etc);

   return (failed_count == 0) ? 0 : 255;
}
