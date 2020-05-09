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
   ck_assert(EINA_DBL_EQ(efl_ui_range_value_get(widget),  10.0));

   EXPECT_ERROR_START;
   efl_ui_range_value_set(widget, 25.0);
   EXPECT_ERROR_END;
   ck_assert(EINA_DBL_EQ(efl_ui_range_value_get(widget), 10.0));
}
EFL_END_TEST

EFL_START_TEST(limit_setting)
{
   double min, max;

   efl_ui_range_limits_set(widget, -20.0, 20.0);
   efl_ui_range_limits_get(widget, &min, &max);
   ck_assert(EINA_DBL_EQ(min,  -20.0));
   ck_assert(EINA_DBL_EQ(max,  20.0));
   EXPECT_ERROR_START;
   efl_ui_range_limits_set(widget, -20.0, -20.0);
   EXPECT_ERROR_END;
   efl_ui_range_limits_get(widget, &min, &max);
   ck_assert(EINA_DBL_EQ(min,  -20.0));
   ck_assert(EINA_DBL_EQ(max,  20.0));

   EXPECT_ERROR_START;
   efl_ui_range_limits_set(widget, 2.0, -20.0);
   EXPECT_ERROR_END;
   efl_ui_range_limits_get(widget, &min, &max);
   ck_assert(EINA_DBL_EQ(min,  -20.0));
   ck_assert(EINA_DBL_EQ(max,  20.0));

   EXPECT_ERROR_START;
   efl_ui_range_limits_set(widget, 25.0, 20.0);
   EXPECT_ERROR_END;
   efl_ui_range_limits_get(widget, &min, &max);
   ck_assert(EINA_DBL_EQ(min,  -20.0));
   ck_assert(EINA_DBL_EQ(max,  20.0));

   efl_ui_range_limits_set(widget, -25.0, -20.0);
   efl_ui_range_limits_get(widget, &min, &max);
   ck_assert(EINA_DBL_EQ(min,  -25.0));
   ck_assert(EINA_DBL_EQ(max,  -20.0));

   efl_ui_range_limits_set(widget, 20.0, 25.0);
   efl_ui_range_limits_get(widget, &min, &max);
   ck_assert(EINA_DBL_EQ(min,  20.0));
   ck_assert(EINA_DBL_EQ(max,  25.0));
}
EFL_END_TEST

EFL_START_TEST(value_setting)
{
   double i;
   efl_ui_range_limits_set(widget, -20.0, 20.0);
   for (i = -20.0; i <= 20.0; ++i)
     {
        efl_ui_range_value_set(widget, i);
        ck_assert(EINA_DBL_EQ(efl_ui_range_value_get(widget), i));
     }
}
EFL_END_TEST

static void
_set_flag(void *data, const Efl_Event *ev)
{
   Eina_Bool *b = data;

   ck_assert_int_eq(*b, EINA_FALSE);
   *b = EINA_TRUE;
   ck_assert_ptr_eq(ev->info, NULL);
}

EFL_START_TEST (range_display_value_events)
{
   Eina_Bool changed = EINA_FALSE, min_reached = EINA_FALSE, max_reached = EINA_FALSE;

   efl_ui_range_limits_set(widget, -3.0, 3.0);
   efl_ui_range_value_set(widget, 0.0);
   efl_event_callback_add(widget, EFL_UI_RANGE_EVENT_CHANGED, _set_flag, &changed);
   efl_event_callback_add(widget, EFL_UI_RANGE_EVENT_MIN_REACHED, _set_flag, &min_reached);
   efl_event_callback_add(widget, EFL_UI_RANGE_EVENT_MAX_REACHED, _set_flag, &max_reached);

   efl_ui_range_value_set(widget, 1.0);
   ck_assert_int_eq(changed, EINA_TRUE);
   ck_assert_int_eq(min_reached, EINA_FALSE);
   ck_assert_int_eq(max_reached, EINA_FALSE);
   changed = EINA_FALSE;
   min_reached = EINA_FALSE;
   max_reached = EINA_FALSE;

   efl_ui_range_value_set(widget, 3.0);
   ck_assert_int_eq(changed, EINA_TRUE);
   ck_assert_int_eq(min_reached, EINA_FALSE);
   ck_assert_int_eq(max_reached, EINA_TRUE);
   changed = EINA_FALSE;
   min_reached = EINA_FALSE;
   max_reached = EINA_FALSE;

   efl_ui_range_value_set(widget, -3.0);
   ck_assert_int_eq(changed, EINA_TRUE);
   ck_assert_int_eq(min_reached, EINA_TRUE);
   ck_assert_int_eq(max_reached, EINA_FALSE);
   changed = EINA_FALSE;
   min_reached = EINA_FALSE;
   max_reached = EINA_FALSE;
   efl_event_callback_del(widget, EFL_UI_RANGE_EVENT_CHANGED, _set_flag, &changed);
   efl_event_callback_del(widget, EFL_UI_RANGE_EVENT_MIN_REACHED, _set_flag, &min_reached);
   efl_event_callback_del(widget, EFL_UI_RANGE_EVENT_MAX_REACHED, _set_flag, &max_reached);

}
EFL_END_TEST

void
efl_ui_range_display_behavior_test(TCase *tc)
{
   tcase_add_test(tc, value_setting_limits);
   tcase_add_test(tc, limit_setting);
   tcase_add_test(tc, value_setting);
   tcase_add_test(tc, range_display_value_events);
}
