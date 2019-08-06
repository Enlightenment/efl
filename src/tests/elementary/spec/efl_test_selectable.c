#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Efl_Ui.h>
#include "efl_ui_spec_suite.h"
#include "suite_helpers.h"

/* spec-meta-start
      {"test-interface":"Efl.Ui.Selectable",
       "test-widgets": ["Efl.Ui.Item.Realized", "Efl.Ui.Check"]}

   spec-meta-end */

static void
_callback_assert_called_once(void *data, const Efl_Event *ev EINA_UNUSED)
{
   int *callback_called = data;
   ck_assert_int_eq(*callback_called, 0);
   *callback_called = *callback_called + 1;
}

EFL_START_TEST(selectable_events)
{
   int callback_called = 0;
   efl_event_callback_add(widget, EFL_UI_EVENT_SELECTED_CHANGED, _callback_assert_called_once, &callback_called);
   efl_ui_selectable_selected_set(widget, EINA_TRUE);
   ck_assert_int_eq(efl_ui_selectable_selected_get(widget), EINA_TRUE);
   ck_assert_int_eq(callback_called, 1);
   callback_called = 0;
   efl_ui_selectable_selected_set(widget, EINA_FALSE);
   ck_assert_int_eq(efl_ui_selectable_selected_get(widget), EINA_FALSE);
   ck_assert_int_eq(callback_called, 1);
   callback_called = 0;
   efl_ui_selectable_selected_set(widget, EINA_TRUE);
   callback_called = 0;
   efl_ui_selectable_selected_set(widget, EINA_TRUE);
   ck_assert_int_eq(callback_called, 0);
}
EFL_END_TEST

EFL_START_TEST(selectable_set)
{
   ck_assert_int_eq(efl_ui_selectable_selected_get(widget), EINA_FALSE);
   efl_ui_selectable_selected_set(widget, EINA_TRUE);
   ck_assert_int_eq(efl_ui_selectable_selected_get(widget), EINA_TRUE);
   efl_ui_selectable_selected_set(widget, EINA_FALSE);
   ck_assert_int_eq(efl_ui_selectable_selected_get(widget), EINA_FALSE);
}
EFL_END_TEST

void
efl_ui_selectable_behavior_test(TCase *tc)
{
   tcase_add_test(tc, selectable_set);
   tcase_add_test(tc, selectable_events);
}
