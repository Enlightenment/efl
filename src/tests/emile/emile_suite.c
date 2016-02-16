#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "emile_suite.h"
#include "../efl_check.h"

static const Efl_Test_Case etc[] = {
  { "Emile_Base", emile_test_base },
  { "Emile_Base64", emile_test_base64 },
  { NULL, NULL }
};

int
main(int argc, char *argv[])
{
   int failed_count;

   if (!_efl_test_option_disp(argc, argv, etc))
     return 0;

   putenv("EFL_RUN_IN_TREE=1");

   failed_count = _efl_suite_build_and_run(argc - 1, (const char **)argv + 1,
                                           "Emile", etc);

   return (failed_count == 0) ? 0 : 255;
}
