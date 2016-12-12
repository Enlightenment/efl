#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ecore_con_suite.h"
#include "../efl_check.h"

static const Efl_Test_Case etc[] = {
  { "Ecore_Con", ecore_con_test_ecore_con },
  { "Ecore_Con_Url", ecore_con_test_ecore_con_url },
  { "Ecore_Con_Eet", ecore_con_test_ecore_con_eet },
  { "Efl_Net_Ip_Address", ecore_con_test_efl_net_ip_address },
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
                                           "Ecore_Con", etc);

   return (failed_count == 0) ? 0 : 255;
}
