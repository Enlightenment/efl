#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Efl_Ui.h>
#include "efl_ui_spec_suite.h"
#include "suite_helpers.h"

/* spec-meta-start
      {"test-interface":"Efl.Ui.Single_Selectable",
       "test-widgets": ["Efl.Ui.Grid", "Efl.Ui.List"],
       "custom-mapping" : {
          "Efl.Ui.Grid" : "EFL_UI_GRID_DEFAULT_ITEM_CLASS",
          "Efl.Ui.List" : "EFL_UI_LIST_DEFAULT_ITEM_CLASS"
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
   Eo *c1 = efl_pack_content_get(widget, 0);
   Eo *c2 = efl_pack_content_get(widget, 2);

   efl_ui_selectable_selected_set(c1, EINA_TRUE);
   ck_assert_ptr_eq(efl_ui_single_selectable_last_selected_get(widget), c1);

   efl_ui_selectable_selected_set(c2, EINA_TRUE);
   ck_assert_ptr_eq(efl_ui_single_selectable_last_selected_get(widget), c2);
   efl_ui_selectable_selected_set(c1, EINA_FALSE);
   ck_assert_ptr_eq(efl_ui_single_selectable_last_selected_get(widget), c2);
   efl_ui_selectable_selected_set(c2, EINA_FALSE);
   ck_assert_ptr_eq(efl_ui_single_selectable_last_selected_get(widget), NULL);
}
EFL_END_TEST

void
efl_ui_single_selectable_behavior_test(TCase *tc)
{
   tcase_add_checked_fixture(tc, _setup, NULL);
   tcase_add_test(tc, last_selectable_check);
}
