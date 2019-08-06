#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Efl_Ui.h>
#include "efl_ui_spec_suite.h"
#include "suite_helpers.h"

/* spec-meta-start
      {"test-interface":"Efl.Ui.Multi_Selectable",
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
        efl_pack_end(widget, c[i]);
     }
}

static void
_set_pointer_quit(void *data, const Efl_Event *ev)
{
   Efl_Ui_Item **b = data;

   ck_assert_ptr_eq(*b, NULL);
   *b = ev->info;
}

static void
_iterator_to_array(Eina_Array **arr, Eina_Iterator *iter)
{
   Efl_Ui_Widget *widget;

   *arr = eina_array_new(10);

   EINA_ITERATOR_FOREACH(iter, widget)
     {
        eina_array_push(*arr, widget);
     }
   eina_iterator_free(iter);
}

EFL_START_TEST(test_multi_select)
{
   Efl_Ui_Item *selected = NULL;
   Efl_Ui_Item *unselected = NULL;
   Eina_Array *arr_selected;
   efl_ui_select_mode_set(widget, EFL_UI_SELECT_MODE_MULTI);
   efl_event_callback_add(widget, EFL_UI_EVENT_ITEM_SELECTED, _set_pointer_quit, &selected);
   efl_event_callback_add(widget, EFL_UI_EVENT_ITEM_UNSELECTED, _set_pointer_quit, &unselected);

   efl_ui_selectable_selected_set(efl_pack_content_get(widget, 0), EINA_TRUE);
   ck_assert_ptr_eq(selected, efl_pack_content_get(widget, 0));
   ck_assert_ptr_eq(unselected, NULL);
   selected = NULL;
   unselected = NULL;
   efl_ui_selectable_selected_set(efl_pack_content_get(widget, 2), EINA_TRUE);
   ck_assert_ptr_eq(selected, efl_pack_content_get(widget, 2));
   ck_assert_ptr_eq(unselected, NULL);
   selected = NULL;
   unselected = NULL;
   ck_assert_int_eq(efl_ui_selectable_selected_get(efl_pack_content_get(widget, 0)), EINA_TRUE);
   ck_assert_int_eq(efl_ui_selectable_selected_get(efl_pack_content_get(widget, 2)), EINA_TRUE);
   ck_assert_ptr_eq(efl_ui_single_selectable_last_selected_get(widget), efl_pack_content_get(widget, 2));
   _iterator_to_array(&arr_selected, efl_ui_selected_items_get(widget));
   ck_assert_int_eq(eina_array_count(arr_selected), 2);
   ck_assert_ptr_eq(eina_array_data_get(arr_selected, 0), efl_pack_content_get(widget, 0));
   ck_assert_ptr_eq(eina_array_data_get(arr_selected, 1), efl_pack_content_get(widget, 2));

   efl_ui_selectable_selected_set(efl_pack_content_get(widget, 2), EINA_TRUE);
   ck_assert_ptr_eq(selected, NULL);
   ck_assert_ptr_eq(unselected, NULL);
   selected = NULL;
   unselected = NULL;
   efl_event_callback_del(widget, EFL_UI_EVENT_ITEM_SELECTED, _set_pointer_quit, &selected);
   efl_event_callback_del(widget, EFL_UI_EVENT_ITEM_UNSELECTED, _set_pointer_quit, &unselected);
}
EFL_END_TEST

EFL_START_TEST(test_multi_select_removal)
{
   Efl_Ui_Item *selected = NULL;
   Efl_Ui_Item *unselected = NULL;
   Eina_Array *arr_selected;
   efl_ui_select_mode_set(widget, EFL_UI_SELECT_MODE_MULTI);
   efl_event_callback_add(widget, EFL_UI_EVENT_ITEM_SELECTED, _set_pointer_quit, &selected);
   efl_event_callback_add(widget, EFL_UI_EVENT_ITEM_UNSELECTED, _set_pointer_quit, &unselected);

   efl_ui_selectable_selected_set(efl_pack_content_get(widget, 0), EINA_TRUE);
   selected = NULL;//No need to ckeck the flag, we asserted in the tcase before
   efl_ui_selectable_selected_set(efl_pack_content_get(widget, 2), EINA_TRUE);
   selected = NULL;//No need to ckeck the flag, we asserted in the tcase before
   unselected = NULL;
   efl_ui_selectable_selected_set(efl_pack_content_get(widget, 0), EINA_FALSE);
   ck_assert_ptr_eq(selected, NULL);
   ck_assert_ptr_eq(unselected, efl_pack_content_get(widget, 0));
   selected = NULL;
   unselected = NULL;
   efl_ui_selectable_selected_set(efl_pack_content_get(widget, 2), EINA_FALSE);
   ck_assert_ptr_eq(selected, NULL);
   ck_assert_ptr_eq(unselected, efl_pack_content_get(widget, 2));
   selected = NULL;
   unselected = NULL;

   ck_assert_ptr_eq(efl_ui_single_selectable_last_selected_get(widget), NULL);
   _iterator_to_array(&arr_selected, efl_ui_selected_items_get(widget));
   ck_assert_int_eq(eina_array_count(arr_selected), 0);
   efl_event_callback_del(widget, EFL_UI_EVENT_ITEM_SELECTED, _set_pointer_quit, &selected);
   efl_event_callback_del(widget, EFL_UI_EVENT_ITEM_UNSELECTED, _set_pointer_quit, &unselected);
}
EFL_END_TEST

EFL_START_TEST(test_single_select)
{
   Efl_Ui_Item *selected = NULL;
   Efl_Ui_Item *unselected = NULL;
   Eina_Array *arr_selected;

   efl_ui_select_mode_set(widget, EFL_UI_SELECT_MODE_SINGLE);
   efl_event_callback_add(widget, EFL_UI_EVENT_ITEM_SELECTED, _set_pointer_quit, &selected);
   efl_event_callback_add(widget, EFL_UI_EVENT_ITEM_UNSELECTED, _set_pointer_quit, &unselected);

   efl_ui_selectable_selected_set(efl_pack_content_get(widget, 0), EINA_TRUE);
   ck_assert_ptr_eq(selected, efl_pack_content_get(widget, 0));
   ck_assert_ptr_eq(unselected, NULL);
   selected = NULL;
   unselected = NULL;
   efl_ui_selectable_selected_set(efl_pack_content_get(widget, 2), EINA_TRUE);
   ck_assert_ptr_eq(selected, efl_pack_content_get(widget, 2));
   ck_assert_ptr_eq(unselected, efl_pack_content_get(widget, 0));
   selected = NULL;
   unselected = NULL;
   ck_assert_int_eq(efl_ui_selectable_selected_get(efl_pack_content_get(widget, 0)), EINA_FALSE);
   ck_assert_int_eq(efl_ui_selectable_selected_get(efl_pack_content_get(widget, 2)), EINA_TRUE);
   ck_assert_ptr_eq(efl_ui_single_selectable_last_selected_get(widget), efl_pack_content_get(widget, 2));
   _iterator_to_array(&arr_selected, efl_ui_selected_items_get(widget));
   ck_assert_int_eq(eina_array_count(arr_selected), 1);
   ck_assert_ptr_eq(eina_array_data_get(arr_selected, 0), efl_pack_content_get(widget, 2));

   efl_ui_selectable_selected_set(efl_pack_content_get(widget, 2), EINA_TRUE);
   ck_assert_ptr_eq(selected, NULL);
   ck_assert_ptr_eq(unselected, NULL);
   selected = NULL;
   unselected = NULL;
   efl_event_callback_del(widget, EFL_UI_EVENT_ITEM_SELECTED, _set_pointer_quit, &selected);
   efl_event_callback_del(widget, EFL_UI_EVENT_ITEM_UNSELECTED, _set_pointer_quit, &unselected);
}
EFL_END_TEST

EFL_START_TEST(test_single_select_always)
{
   Efl_Ui_Item *selected = NULL;
   Efl_Ui_Item *unselected = NULL;
   Eina_Array *arr_selected;

   efl_ui_select_mode_set(widget, EFL_UI_SELECT_MODE_SINGLE_ALWAYS);
   efl_event_callback_add(widget, EFL_UI_EVENT_ITEM_SELECTED, _set_pointer_quit, &selected);
   efl_event_callback_add(widget, EFL_UI_EVENT_ITEM_UNSELECTED, _set_pointer_quit, &unselected);

   efl_ui_selectable_selected_set(efl_pack_content_get(widget, 0), EINA_TRUE);
   ck_assert_ptr_eq(selected, efl_pack_content_get(widget, 0));
   ck_assert_ptr_eq(unselected, NULL);
   selected = NULL;
   unselected = NULL;
   efl_ui_selectable_selected_set(efl_pack_content_get(widget, 2), EINA_TRUE);
   ck_assert_ptr_eq(selected, efl_pack_content_get(widget, 2));
   ck_assert_ptr_eq(unselected, efl_pack_content_get(widget, 0));
   selected = NULL;
   unselected = NULL;
   efl_ui_selectable_selected_set(efl_pack_content_get(widget, 2), EINA_TRUE);
   ck_assert_ptr_eq(selected, efl_pack_content_get(widget, 2));
   ck_assert_ptr_eq(unselected, NULL);
   selected = NULL;
   unselected = NULL;
   ck_assert_int_eq(efl_ui_selectable_selected_get(efl_pack_content_get(widget, 0)), EINA_FALSE);
   ck_assert_int_eq(efl_ui_selectable_selected_get(efl_pack_content_get(widget, 2)), EINA_TRUE);
   ck_assert_ptr_eq(efl_ui_single_selectable_last_selected_get(widget), efl_pack_content_get(widget, 2));
   _iterator_to_array(&arr_selected, efl_ui_selected_items_get(widget));
   ck_assert_int_eq(eina_array_count(arr_selected), 1);
   ck_assert_ptr_eq(eina_array_data_get(arr_selected, 0), efl_pack_content_get(widget, 2));
   efl_event_callback_del(widget, EFL_UI_EVENT_ITEM_SELECTED, _set_pointer_quit, &selected);
   efl_event_callback_del(widget, EFL_UI_EVENT_ITEM_UNSELECTED, _set_pointer_quit, &unselected);
}
EFL_END_TEST

EFL_START_TEST(test_none_select)
{
   Efl_Ui_Item *selected = NULL;
   Efl_Ui_Item *unselected = NULL;
   Eina_Array *arr_selected;

   efl_ui_select_mode_set(widget, EFL_UI_SELECT_MODE_NONE);
   efl_event_callback_add(widget, EFL_UI_EVENT_ITEM_SELECTED, _set_pointer_quit, &selected);
   efl_event_callback_add(widget, EFL_UI_EVENT_ITEM_UNSELECTED, _set_pointer_quit, &unselected);

   efl_ui_selectable_selected_set(efl_pack_content_get(widget, 0), EINA_TRUE);
   ck_assert_ptr_eq(selected, NULL);
   ck_assert_ptr_eq(unselected, NULL);
   selected = NULL;
   unselected = NULL;
   efl_ui_selectable_selected_set(efl_pack_content_get(widget, 2), EINA_TRUE);
   ck_assert_ptr_eq(selected, NULL);
   ck_assert_ptr_eq(unselected, NULL);
   selected = NULL;
   unselected = NULL;
   ck_assert_int_eq(efl_ui_selectable_selected_get(efl_pack_content_get(widget, 0)), EINA_FALSE);
   ck_assert_int_eq(efl_ui_selectable_selected_get(efl_pack_content_get(widget, 2)), EINA_FALSE);
   ck_assert_ptr_eq(efl_ui_single_selectable_last_selected_get(widget), NULL);
   _iterator_to_array(&arr_selected, efl_ui_selected_items_get(widget));
   ck_assert_int_eq(eina_array_count(arr_selected), 0);
   efl_event_callback_del(widget, EFL_UI_EVENT_ITEM_SELECTED, _set_pointer_quit, &selected);
   efl_event_callback_del(widget, EFL_UI_EVENT_ITEM_UNSELECTED, _set_pointer_quit, &unselected);
}
EFL_END_TEST

void
efl_ui_multi_selectable_behavior_test(TCase *tc)
{
   tcase_add_checked_fixture(tc, _setup, NULL);
   tcase_add_test(tc, test_multi_select);
   tcase_add_test(tc, test_multi_select_removal);
   tcase_add_test(tc, test_single_select);
   tcase_add_test(tc, test_none_select);
   tcase_add_test(tc, test_single_select_always);
   efl_ui_single_selectable_behavior_test(tc);
}
