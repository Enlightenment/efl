#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Efl_Ui.h>
#include "efl_ui_spec_suite.h"
#include "suite_helpers.h"

/* spec-meta-start
      {"test-interface":"Efl.Ui.Range_Display",
       "test-widgets": ["Efl.Ui.Spin", "Efl.Ui.Progressbar", "Efl.Ui.Slider", "Efl.Ui.Spin_Button"]}
   spec-meta-end */

EFL_START_TEST(value_setting_limits)
{
   efl_ui_range_limits_set(widget, -20.0, 20.0);
   efl_ui_range_value_set(widget, 10.0);

   EXPECT_ERROR_START;
   efl_ui_range_value_set(widget, -25.0);
   EXPECT_ERROR_END;
   ck_assert(efl_ui_range_value_get(widget) == 10.0);

   EXPECT_ERROR_START;
   efl_ui_range_value_set(widget, 25.0);
   EXPECT_ERROR_END;
   ck_assert(efl_ui_range_value_get(widget) == 10.0);
}
EFL_END_TEST

EFL_START_TEST(limit_setting)
{
   double min, max;

   efl_ui_range_limits_set(widget, -20.0, 20.0);
   efl_ui_range_limits_get(widget, &min, &max);
   ck_assert(min == -20.0);
   ck_assert(max == 20.0);
   EXPECT_ERROR_START;
   efl_ui_range_limits_set(widget, -20.0, -20.0);
   EXPECT_ERROR_END;
   efl_ui_range_limits_get(widget, &min, &max);
   ck_assert(min == -20.0);
   ck_assert(max == 20.0);

   EXPECT_ERROR_START;
   efl_ui_range_limits_set(widget, 2.0, -20.0);
   EXPECT_ERROR_END;
   efl_ui_range_limits_get(widget, &min, &max);
   ck_assert(min == -20.0);
   ck_assert(max == 20.0);

   EXPECT_ERROR_START;
   efl_ui_range_limits_set(widget, 25.0, 20.0);
   EXPECT_ERROR_END;
   efl_ui_range_limits_get(widget, &min, &max);
   ck_assert(min == -20.0);
   ck_assert(max == 20.0);

   efl_ui_range_limits_set(widget, -25.0, -20.0);
   efl_ui_range_limits_get(widget, &min, &max);
   ck_assert(min == -25.0);
   ck_assert(max == -20.0);

   efl_ui_range_limits_set(widget, 20.0, 25.0);
   efl_ui_range_limits_get(widget, &min, &max);
   ck_assert(min == 20.0);
   ck_assert(max == 25.0);
}
EFL_END_TEST

EFL_START_TEST(value_setting)
{
   double i;
   efl_ui_range_limits_set(widget, -20.0, 20.0);
   for (i = -20.0; i <= 20.0; ++i)
     {
        efl_ui_range_value_set(widget, i);
        ck_assert(efl_ui_range_value_get(widget) == i);
     }
}
EFL_END_TEST

void
efl_ui_range_display_behavior_test(TCase *tc)
{
   tcase_add_test(tc, value_setting_limits);
   tcase_add_test(tc, limit_setting);
   tcase_add_test(tc, value_setting);
}
