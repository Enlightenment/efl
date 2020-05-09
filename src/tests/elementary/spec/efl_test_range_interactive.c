#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Efl_Ui.h>
#include <Elementary.h>
#include "efl_ui_spec_suite.h"
#include "efl_ui_suite.h"
#include "suite_helpers.h"

/* spec-meta-start
      {"test-interface":"Efl.Ui.Range_Display_Interactive",
       "test-widgets": ["Efl.Ui.Slider", "Efl.Ui.Spin_Button"]}
   spec-meta-end */

EFL_START_TEST(step_setting)
{
   efl_ui_range_step_set(widget, 20.0);
   ck_assert(EINA_DBL_EQ(efl_ui_range_step_get(widget), 20.0));
   efl_ui_range_step_set(widget, 100.0);
   ck_assert(EINA_DBL_EQ(efl_ui_range_step_get(widget), 100.0));

   EXPECT_ERROR_START;
   efl_ui_range_step_set(widget, 0.0);
   ck_assert(EINA_DBL_EQ(efl_ui_range_step_get(widget), 100.0));
   EXPECT_ERROR_END;

   EXPECT_ERROR_START;
   efl_ui_range_step_set(widget, -20.0);
   ck_assert(EINA_DBL_EQ(efl_ui_range_step_get(widget), 100.0));
   EXPECT_ERROR_END;
}
EFL_END_TEST

static void
_steady_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   int *count = data;
   *count = *count + 1;
   efl_loop_quit(efl_main_loop_get(), EINA_VALUE_EMPTY);
}

static Eina_Value
_quit_ml(void *data EINA_UNUSED, const Eina_Value v EINA_UNUSED,  const Eina_Future *dead_future EINA_UNUSED)
{
   efl_loop_quit(efl_main_loop_get(), EINA_VALUE_EMPTY);
   return EINA_VALUE_EMPTY;
}

EFL_START_TEST(steady_event)
{
   Evas *e;
   int count = 0;

   efl_event_callback_add(widget, EFL_UI_RANGE_EVENT_STEADY, _steady_cb, &count),
   efl_ui_range_limits_set(widget, 0, 100);
   efl_ui_range_step_set(widget, 10);
   efl_ui_range_value_set(widget, 20);

   e = evas_object_evas_get(win);

   efl_layout_signal_process(widget, EINA_TRUE);
   get_me_to_those_events(widget);

   int x, y, w, h;

   evas_object_geometry_get(widget, &x, &y, &w, &h);
   evas_event_feed_mouse_in(e, 0, NULL);
   evas_event_feed_mouse_move(e, x + (w / 2), y + (h / 2), 0, NULL);
   evas_event_feed_mouse_wheel(e, -1, 1, 0, NULL);
   evas_event_feed_mouse_up(e, 1, 0, 0, NULL);
   efl_layout_signal_process(widget, EINA_TRUE);
   ck_assert_int_eq(efl_ui_range_value_get(widget), 10);

   Eina_Future *f = efl_loop_timeout(efl_main_loop_get(), 10.0);
   eina_future_then(f, _quit_ml);

   efl_loop_begin(efl_main_loop_get());
   ck_assert_int_eq(count, 1);
}
EFL_END_TEST

void
efl_ui_range_display_interactive_behavior_test(TCase *tc)
{
   tcase_add_test(tc, step_setting);
   tcase_add_test(tc, steady_event);
}

