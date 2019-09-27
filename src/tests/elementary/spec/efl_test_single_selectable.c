#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Efl_Ui.h>
#include "efl_ui_spec_suite.h"
#include "suite_helpers.h"

/* spec-meta-start
      {"test-interface":"Efl.Ui.Single_Selectable",
       "test-widgets": ["Efl.Ui.Grid", "Efl.Ui.List", "Efl.Ui.Radio_Box", "Efl.Ui.Tab_Bar"],
       "custom-mapping" : {
          "Efl.Ui.Grid" : "EFL_UI_GRID_DEFAULT_ITEM_CLASS",
          "Efl.Ui.List" : "EFL_UI_LIST_DEFAULT_ITEM_CLASS",
          "Efl.Ui.Radio_Box" : "EFL_UI_RADIO_CLASS",
          "Efl.Ui.Tab_Bar" : "EFL_UI_TAB_BAR_DEFAULT_ITEM_CLASS"
        }
      }

   spec-meta-end */

static void
_setup(void)
{
   Eo *c[3];

   for (int i = 0; i < 3; ++i)
     {
        c[i] = create_test_widget();
        if (efl_isa(c[i], EFL_UI_RADIO_CLASS))
          efl_ui_radio_state_value_set(c[i], i+1);
        efl_pack_end(widget, c[i]);
     }
}

EFL_START_TEST(last_selectable_check)
{
   int c = 0;

   efl_event_callback_add(widget, EFL_UI_SELECTABLE_EVENT_SELECTION_CHANGED, (void*) event_callback_single_call_int_data, &c);
   efl_event_callback_add(widget, EFL_UI_SELECTABLE_EVENT_SELECTION_CHANGED, event_callback_that_quits_the_main_loop_when_called, NULL);

   Eo *c1 = efl_pack_content_get(widget, 0);
   Eo *c2 = efl_pack_content_get(widget, 2);

   efl_ui_selectable_selected_set(c1, EINA_TRUE);
   ck_assert_ptr_eq(efl_ui_selectable_last_selected_get(widget), c1);
   if (c == 0) efl_loop_begin(efl_main_loop_get());
   ck_assert_int_eq(c, 1);
   c = 0;

   efl_ui_selectable_selected_set(c2, EINA_TRUE);
   ck_assert_ptr_eq(efl_ui_selectable_last_selected_get(widget), c2);
   if (c == 0) efl_loop_begin(efl_main_loop_get());
   ck_assert_int_eq(c, 1);
   c = 0;

   efl_ui_selectable_selected_set(c1, EINA_FALSE);
   ck_assert_ptr_eq(efl_ui_selectable_last_selected_get(widget), c2);
   efl_ui_selectable_selected_set(c2, EINA_FALSE);
   if (c == 0) efl_loop_begin(efl_main_loop_get());
   ck_assert_int_eq(c, 1);
   c = 0;

   ck_assert_ptr_eq(efl_ui_selectable_last_selected_get(widget), NULL);
}
EFL_END_TEST

EFL_START_TEST(fallback_selection)
{
   Eo *c1 = efl_pack_content_get(widget, 0);
   Eo *c2 = efl_pack_content_get(widget, 2);

   efl_ui_selectable_fallback_selection_set(widget, c2);
   ck_assert_int_eq(efl_ui_selectable_selected_get(c2), EINA_TRUE);
   efl_ui_selectable_selected_set(c1, EINA_TRUE);
   ck_assert_int_eq(efl_ui_selectable_selected_get(c2), EINA_FALSE);
   ck_assert_int_eq(efl_ui_selectable_selected_get(c1), EINA_TRUE);
   efl_ui_selectable_selected_set(c1, EINA_FALSE);
   ck_assert_int_eq(efl_ui_selectable_selected_get(c2), EINA_TRUE);
   ck_assert_int_eq(efl_ui_selectable_selected_get(c1), EINA_FALSE);
}
EFL_END_TEST

void
efl_ui_single_selectable_behavior_test(TCase *tc)
{
   tcase_add_checked_fixture(tc, _setup, NULL);
   tcase_add_test(tc, last_selectable_check);
   tcase_add_test(tc, fallback_selection);
}
