#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Efl_Ui.h>
#include "efl_ui_spec_suite.h"
#include "suite_helpers.h"

/* spec-meta-start
      {"test-interface":"Efl.Ui.Range_Display_Interactive",
       "test-widgets": ["Efl.Ui.Spin", "Efl.Ui.Slider", "Efl.Ui.Spin_Button"]}
   spec-meta-end */

EFL_START_TEST(step_setting)
{
   efl_ui_range_step_set(widget, 20.0);
   ck_assert(efl_ui_range_step_get(widget) == 20.0);
   efl_ui_range_step_set(widget, 100.0);
   ck_assert(efl_ui_range_step_get(widget) == 100.0);

   EXPECT_ERROR_START;
   efl_ui_range_step_set(widget, 0.0);
   ck_assert(efl_ui_range_step_get(widget) == 100.0);
   EXPECT_ERROR_END;

   EXPECT_ERROR_START;
   efl_ui_range_step_set(widget, -20.0);
   ck_assert(efl_ui_range_step_get(widget) == 100.0);
   EXPECT_ERROR_END;
}
EFL_END_TEST

void
efl_ui_range_display_interactive_behavior_test(TCase *tc)
{
   tcase_add_test(tc, step_setting);
}

