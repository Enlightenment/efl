#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "elput_suite.h"
#include "../efl_check.h"

static const Efl_Test_Case etc[] =
{
#if HAVE_ELPUT
   { "Elput", elput_test_elput },
#endif
   { NULL, NULL }
};

int
main(int argc, char **argv)
{
   int count;

   if (!_efl_test_option_disp(argc, argv, etc)) return 0;

#ifdef NEED_RUN_IN_TREE
   putenv("EFL_RUN_IN_TREE=1");
#endif

   count =
     _efl_suite_build_and_run(argc - 1, (const char **)argv + 1, "Elput", etc);

   return (count == 0) ? 0 : 255;
}
