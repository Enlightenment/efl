#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <check.h>
#define EFL_NOLEGACY_API_SUPPORT
#include <Efl_Ui.h>
#include "../efl_check.h"
#include "efl_ui_suite.h"
#include "suite_helpers.h"

static const Efl_Test_Case etc[] = {
  //{ "elm_focus", elm_test_focus},
  //{ "elm_focus_sub", elm_test_focus_sub},
  //{ "elm_widget_focus", elm_test_widget_focus},
  { "efl_ui_atspi", efl_ui_test_atspi},
  { "efl_ui_grid", efl_ui_test_grid},
  { NULL, NULL }
};

int
main(int argc, char **argv)
{
   int failed_count;

   if (!_efl_test_option_disp(argc, argv, etc))
     return 0;

   failed_count = suite_setup(EINA_FALSE);

   failed_count += _efl_suite_build_and_run(argc - 1, (const char **)argv + 1,
                                           "Efl_Ui", etc, SUITE_INIT_FN(elm2), SUITE_SHUTDOWN_FN(elm));

   return (failed_count == 0) ? 0 : 255;
}
