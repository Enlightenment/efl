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
   int c = 0;

   Eina_Array *arr_selected;
   efl_ui_select_mode_set(widget, EFL_UI_SELECT_MODE_MULTI);
   efl_event_callback_add(widget, EFL_UI_SINGLE_SELECTABLE_EVENT_SELECTION_CHANGED, (Efl_Event_Cb) event_callback_single_call_int_data, &c);
   efl_event_callback_add(widget, EFL_UI_SINGLE_SELECTABLE_EVENT_SELECTION_CHANGED, event_callback_that_quits_the_main_loop_when_called, NULL);

   efl_ui_selectable_selected_set(efl_pack_content_get(widget, 0), EINA_TRUE);
   if (c == 0) efl_loop_begin(efl_main_loop_get());
   ck_assert_int_eq(c, 1);
   c = 0;

   efl_ui_selectable_selected_set(efl_pack_content_get(widget, 2), EINA_TRUE);
   if (c == 0) efl_loop_begin(efl_main_loop_get());
   ck_assert_int_eq(c, 1);
   c = 0;
   ck_assert_int_eq(efl_ui_selectable_selected_get(efl_pack_content_get(widget, 0)), EINA_TRUE);
   ck_assert_int_eq(efl_ui_selectable_selected_get(efl_pack_content_get(widget, 2)), EINA_TRUE);
   ck_assert_ptr_eq(efl_ui_single_selectable_last_selected_get(widget), efl_pack_content_get(widget, 2));
   _iterator_to_array(&arr_selected, efl_ui_selected_items_get(widget));
   ck_assert_int_eq(eina_array_count(arr_selected), 2);
   ck_assert_ptr_eq(eina_array_data_get(arr_selected, 0), efl_pack_content_get(widget, 0));
   ck_assert_ptr_eq(eina_array_data_get(arr_selected, 1), efl_pack_content_get(widget, 2));

   efl_ui_selectable_selected_set(efl_pack_content_get(widget, 2), EINA_TRUE);
   ck_assert_int_eq(c, 0);

   efl_event_callback_del(widget, EFL_UI_SINGLE_SELECTABLE_EVENT_SELECTION_CHANGED, (Efl_Event_Cb) event_callback_single_call_int_data, &c);
   efl_event_callback_del(widget, EFL_UI_SINGLE_SELECTABLE_EVENT_SELECTION_CHANGED, event_callback_that_quits_the_main_loop_when_called, NULL);
}
EFL_END_TEST

EFL_START_TEST(test_multi_select_removal)
{
   int c = 0;
   Eina_Array *arr_selected;
   efl_ui_select_mode_set(widget, EFL_UI_SELECT_MODE_MULTI);
   efl_event_callback_add(widget, EFL_UI_SINGLE_SELECTABLE_EVENT_SELECTION_CHANGED, (Efl_Event_Cb) event_callback_single_call_int_data, &c);
   efl_event_callback_add(widget, EFL_UI_SINGLE_SELECTABLE_EVENT_SELECTION_CHANGED, event_callback_that_quits_the_main_loop_when_called, NULL);

   efl_ui_selectable_selected_set(efl_pack_content_get(widget, 0), EINA_TRUE);
   c = 0;
   efl_ui_selectable_selected_set(efl_pack_content_get(widget, 2), EINA_TRUE);
   c = 0;
   efl_ui_selectable_selected_set(efl_pack_content_get(widget, 0), EINA_FALSE);
   if (c == 0) efl_loop_begin(efl_main_loop_get());
   ck_assert_int_eq(c, 1);
   c = 0;
   efl_ui_selectable_selected_set(efl_pack_content_get(widget, 2), EINA_FALSE);
   if (c == 0) efl_loop_begin(efl_main_loop_get());
   ck_assert_int_eq(c, 1);
   c = 0;

   ck_assert_ptr_eq(efl_ui_single_selectable_last_selected_get(widget), NULL);
   _iterator_to_array(&arr_selected, efl_ui_selected_items_get(widget));
   ck_assert_int_eq(eina_array_count(arr_selected), 0);
   efl_event_callback_del(widget, EFL_UI_SINGLE_SELECTABLE_EVENT_SELECTION_CHANGED, (Efl_Event_Cb) event_callback_single_call_int_data, &c);
   efl_event_callback_del(widget, EFL_UI_SINGLE_SELECTABLE_EVENT_SELECTION_CHANGED, event_callback_that_quits_the_main_loop_when_called, NULL);
}
EFL_END_TEST

EFL_START_TEST(test_single_select)
{
   int c = 0;
   Eina_Array *arr_selected;

   efl_ui_select_mode_set(widget, EFL_UI_SELECT_MODE_SINGLE);
   efl_event_callback_add(widget, EFL_UI_SINGLE_SELECTABLE_EVENT_SELECTION_CHANGED, (Efl_Event_Cb) event_callback_single_call_int_data, &c);
   efl_event_callback_add(widget, EFL_UI_SINGLE_SELECTABLE_EVENT_SELECTION_CHANGED, event_callback_that_quits_the_main_loop_when_called, NULL);

   efl_ui_selectable_selected_set(efl_pack_content_get(widget, 0), EINA_TRUE);
   if (c == 0) efl_loop_begin(efl_main_loop_get());
   ck_assert_int_eq(c, 1);
   c = 0;
   efl_ui_selectable_selected_set(efl_pack_content_get(widget, 2), EINA_TRUE);
   if (c == 0) efl_loop_begin(efl_main_loop_get());
   ck_assert_int_eq(c, 1);
   c = 0;
   ck_assert_int_eq(efl_ui_selectable_selected_get(efl_pack_content_get(widget, 0)), EINA_FALSE);
   ck_assert_int_eq(efl_ui_selectable_selected_get(efl_pack_content_get(widget, 2)), EINA_TRUE);
   ck_assert_ptr_eq(efl_ui_single_selectable_last_selected_get(widget), efl_pack_content_get(widget, 2));
   _iterator_to_array(&arr_selected, efl_ui_selected_items_get(widget));
   ck_assert_int_eq(eina_array_count(arr_selected), 1);
   ck_assert_ptr_eq(eina_array_data_get(arr_selected, 0), efl_pack_content_get(widget, 2));

   efl_ui_selectable_selected_set(efl_pack_content_get(widget, 2), EINA_TRUE);
   ck_assert_int_eq(c, 0);

   efl_event_callback_del(widget, EFL_UI_SINGLE_SELECTABLE_EVENT_SELECTION_CHANGED, (Efl_Event_Cb) event_callback_single_call_int_data, &c);
   efl_event_callback_del(widget, EFL_UI_SINGLE_SELECTABLE_EVENT_SELECTION_CHANGED, event_callback_that_quits_the_main_loop_when_called, NULL);
}
EFL_END_TEST

EFL_START_TEST(test_single_select_always)
{
   int c = 0;
   Eina_Array *arr_selected;

   efl_ui_select_mode_set(widget, EFL_UI_SELECT_MODE_SINGLE_ALWAYS);
   efl_event_callback_add(widget, EFL_UI_SINGLE_SELECTABLE_EVENT_SELECTION_CHANGED, (Efl_Event_Cb) event_callback_single_call_int_data, &c);
   efl_event_callback_add(widget, EFL_UI_SINGLE_SELECTABLE_EVENT_SELECTION_CHANGED, event_callback_that_quits_the_main_loop_when_called, NULL);

   efl_ui_selectable_selected_set(efl_pack_content_get(widget, 0), EINA_TRUE);
   if (c == 0) efl_loop_begin(efl_main_loop_get());
   ck_assert_int_eq(c, 1);
   c = 0;
   efl_ui_selectable_selected_set(efl_pack_content_get(widget, 2), EINA_TRUE);
   if (c == 0) efl_loop_begin(efl_main_loop_get());
   ck_assert_int_eq(c, 1);
   c = 0;
   efl_ui_selectable_selected_set(efl_pack_content_get(widget, 2), EINA_TRUE);
   if (c == 0) efl_loop_begin(efl_main_loop_get());
   ck_assert_int_eq(c, 1);
   c = 0;
   ck_assert_int_eq(efl_ui_selectable_selected_get(efl_pack_content_get(widget, 0)), EINA_FALSE);
   ck_assert_int_eq(efl_ui_selectable_selected_get(efl_pack_content_get(widget, 2)), EINA_TRUE);
   ck_assert_ptr_eq(efl_ui_single_selectable_last_selected_get(widget), efl_pack_content_get(widget, 2));
   _iterator_to_array(&arr_selected, efl_ui_selected_items_get(widget));
   ck_assert_int_eq(eina_array_count(arr_selected), 1);
   ck_assert_ptr_eq(eina_array_data_get(arr_selected, 0), efl_pack_content_get(widget, 2));
   efl_event_callback_del(widget, EFL_UI_SINGLE_SELECTABLE_EVENT_SELECTION_CHANGED, (Efl_Event_Cb) event_callback_single_call_int_data, &c);
   efl_event_callback_del(widget, EFL_UI_SINGLE_SELECTABLE_EVENT_SELECTION_CHANGED, event_callback_that_quits_the_main_loop_when_called, NULL);
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

EFL_START_TEST(select_all_api)
{
   Eina_Array *arr_selected;

   efl_ui_select_mode_set(widget, EFL_UI_SELECT_MODE_MULTI);
   efl_ui_select_all(widget);
   _iterator_to_array(&arr_selected, efl_ui_selected_items_get(widget));

   ck_assert_int_eq(eina_array_count(arr_selected), 3);

   ck_assert_ptr_eq(eina_array_data_get(arr_selected, 0), efl_pack_content_get(widget, 0));
   ck_assert_ptr_eq(eina_array_data_get(arr_selected, 1), efl_pack_content_get(widget, 1));
   ck_assert_ptr_eq(eina_array_data_get(arr_selected, 2), efl_pack_content_get(widget, 2));
}
EFL_END_TEST

EFL_START_TEST(unselect_all_api)
{
   Eina_Array *arr_selected;

   efl_ui_select_mode_set(widget, EFL_UI_SELECT_MODE_MULTI);
   efl_ui_selectable_selected_set(efl_pack_content_get(widget, 0), EINA_TRUE);

   efl_ui_unselect_all(widget);
   _iterator_to_array(&arr_selected, efl_ui_selected_items_get(widget));

   ck_assert_int_eq(eina_array_count(arr_selected), 0);
   ck_assert_int_eq(efl_ui_selectable_selected_get(efl_pack_content_get(widget, 0)), EINA_FALSE);
}
EFL_END_TEST

EFL_START_TEST(unselect_range)
{
   Eina_Array *arr_selected;

   efl_ui_select_mode_set(widget, EFL_UI_SELECT_MODE_MULTI);
   efl_ui_select_all(widget);

   efl_ui_unselect_range(widget, efl_pack_content_get(widget, 1), efl_pack_content_get(widget, 2));
   _iterator_to_array(&arr_selected, efl_ui_selected_items_get(widget));
   ck_assert_int_eq(eina_array_count(arr_selected), 1);
   ck_assert_ptr_eq(eina_array_data_get(arr_selected, 0), efl_pack_content_get(widget, 0));
}
EFL_END_TEST

EFL_START_TEST(unselect_range2)
{
   Eina_Array *arr_selected;

   efl_ui_select_mode_set(widget, EFL_UI_SELECT_MODE_MULTI);
   efl_ui_select_all(widget);

   efl_ui_unselect_range(widget, efl_pack_content_get(widget, 2), efl_pack_content_get(widget, 1));
   _iterator_to_array(&arr_selected, efl_ui_selected_items_get(widget));
   ck_assert_int_eq(eina_array_count(arr_selected), 1);
   ck_assert_ptr_eq(eina_array_data_get(arr_selected, 0), efl_pack_content_get(widget, 0));
}
EFL_END_TEST

EFL_START_TEST(select_range)
{
   Eina_Array *arr_selected;

   efl_ui_select_mode_set(widget, EFL_UI_SELECT_MODE_MULTI);
   efl_ui_select_range(widget, efl_pack_content_get(widget, 1), efl_pack_content_get(widget, 2));
   _iterator_to_array(&arr_selected, efl_ui_selected_items_get(widget));
   ck_assert_int_eq(eina_array_count(arr_selected), 2);
   ck_assert_ptr_eq(eina_array_data_get(arr_selected, 0), efl_pack_content_get(widget, 1));
   ck_assert_ptr_eq(eina_array_data_get(arr_selected, 1), efl_pack_content_get(widget, 2));
}
EFL_END_TEST

EFL_START_TEST(select_range2)
{
   Eina_Array *arr_selected;

   efl_ui_select_mode_set(widget, EFL_UI_SELECT_MODE_MULTI);
   efl_ui_select_range(widget, efl_pack_content_get(widget, 2), efl_pack_content_get(widget, 1));
   _iterator_to_array(&arr_selected, efl_ui_selected_items_get(widget));
   ck_assert_int_eq(eina_array_count(arr_selected), 2);
   ck_assert_ptr_eq(eina_array_data_get(arr_selected, 0), efl_pack_content_get(widget, 1));
   ck_assert_ptr_eq(eina_array_data_get(arr_selected, 1), efl_pack_content_get(widget, 2));
}
EFL_END_TEST

void
efl_ui_multi_selectable_behavior_test(TCase *tc)
{
   //Items are getting added by the fixture added in efl_ui_single_selectable_behavior_test
   tcase_add_test(tc, test_multi_select);
   tcase_add_test(tc, test_multi_select_removal);
   tcase_add_test(tc, test_single_select);
   tcase_add_test(tc, test_none_select);
   tcase_add_test(tc, test_single_select_always);
   tcase_add_test(tc, select_all_api);
   tcase_add_test(tc, unselect_all_api);
   tcase_add_test(tc, unselect_range);
   tcase_add_test(tc, unselect_range2);
   tcase_add_test(tc, select_range);
   tcase_add_test(tc, select_range2);
   efl_ui_single_selectable_behavior_test(tc);
}
