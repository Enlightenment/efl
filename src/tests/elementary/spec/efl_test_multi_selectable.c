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
   efl_ui_multi_selectable_select_mode_set(widget, EFL_UI_SELECT_MODE_MULTI);
   efl_event_callback_add(widget, EFL_UI_SELECTABLE_EVENT_SELECTION_CHANGED, (void*) event_callback_single_call_int_data, &c);
   efl_event_callback_add(widget, EFL_UI_SELECTABLE_EVENT_SELECTION_CHANGED, event_callback_that_quits_the_main_loop_when_called, NULL);

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
   ck_assert_ptr_eq(efl_ui_selectable_last_selected_get(widget), efl_pack_content_get(widget, 2));
   _iterator_to_array(&arr_selected, efl_ui_multi_selectable_selected_iterator_new(widget));
   ck_assert_int_eq(eina_array_count(arr_selected), 2);
   ck_assert_ptr_eq(eina_array_data_get(arr_selected, 0), efl_pack_content_get(widget, 0));
   ck_assert_ptr_eq(eina_array_data_get(arr_selected, 1), efl_pack_content_get(widget, 2));

   efl_ui_selectable_selected_set(efl_pack_content_get(widget, 2), EINA_TRUE);
   ck_assert_int_eq(c, 0);

   efl_event_callback_del(widget, EFL_UI_SELECTABLE_EVENT_SELECTION_CHANGED, (void*) event_callback_single_call_int_data, &c);
   efl_event_callback_del(widget, EFL_UI_SELECTABLE_EVENT_SELECTION_CHANGED, event_callback_that_quits_the_main_loop_when_called, NULL);
   eina_array_free(arr_selected);
}
EFL_END_TEST

EFL_START_TEST(test_multi_select_removal)
{
   int c = 0;
   Eina_Array *arr_selected;
   efl_ui_multi_selectable_select_mode_set(widget, EFL_UI_SELECT_MODE_MULTI);
   efl_event_callback_add(widget, EFL_UI_SELECTABLE_EVENT_SELECTION_CHANGED, (void*) event_callback_single_call_int_data, &c);
   efl_event_callback_add(widget, EFL_UI_SELECTABLE_EVENT_SELECTION_CHANGED, event_callback_that_quits_the_main_loop_when_called, NULL);

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

   ck_assert_ptr_eq(efl_ui_selectable_last_selected_get(widget), NULL);
   _iterator_to_array(&arr_selected, efl_ui_multi_selectable_selected_iterator_new(widget));
   ck_assert_int_eq(eina_array_count(arr_selected), 0);
   efl_event_callback_del(widget, EFL_UI_SELECTABLE_EVENT_SELECTION_CHANGED, (void*) event_callback_single_call_int_data, &c);
   efl_event_callback_del(widget, EFL_UI_SELECTABLE_EVENT_SELECTION_CHANGED, event_callback_that_quits_the_main_loop_when_called, NULL);
   eina_array_free(arr_selected);
}
EFL_END_TEST

EFL_START_TEST(test_single_select)
{
   int c = 0;
   Eina_Array *arr_selected;

   efl_ui_multi_selectable_select_mode_set(widget, EFL_UI_SELECT_MODE_SINGLE);
   efl_event_callback_add(widget, EFL_UI_SELECTABLE_EVENT_SELECTION_CHANGED, (void*) event_callback_single_call_int_data, &c);
   efl_event_callback_add(widget, EFL_UI_SELECTABLE_EVENT_SELECTION_CHANGED, event_callback_that_quits_the_main_loop_when_called, NULL);

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
   ck_assert_ptr_eq(efl_ui_selectable_last_selected_get(widget), efl_pack_content_get(widget, 2));
   _iterator_to_array(&arr_selected, efl_ui_multi_selectable_selected_iterator_new(widget));
   ck_assert_int_eq(eina_array_count(arr_selected), 1);
   ck_assert_ptr_eq(eina_array_data_get(arr_selected, 0), efl_pack_content_get(widget, 2));

   efl_ui_selectable_selected_set(efl_pack_content_get(widget, 2), EINA_TRUE);
   ck_assert_int_eq(c, 0);

   efl_event_callback_del(widget, EFL_UI_SELECTABLE_EVENT_SELECTION_CHANGED, (void*) event_callback_single_call_int_data, &c);
   efl_event_callback_del(widget, EFL_UI_SELECTABLE_EVENT_SELECTION_CHANGED, event_callback_that_quits_the_main_loop_when_called, NULL);
   eina_array_free(arr_selected);
}
EFL_END_TEST

EFL_START_TEST(test_none_select)
{
   Eina_Array *arr_selected;
   int c = 0;

   efl_ui_multi_selectable_select_mode_set(widget, EFL_UI_SELECT_MODE_NONE);
   efl_event_callback_add(widget, EFL_UI_SELECTABLE_EVENT_SELECTION_CHANGED, (void*) event_callback_single_call_int_data, &c);
   efl_event_callback_add(widget, EFL_UI_SELECTABLE_EVENT_SELECTION_CHANGED, event_callback_that_quits_the_main_loop_when_called, NULL);

   efl_ui_selectable_selected_set(efl_pack_content_get(widget, 0), EINA_TRUE);
   get_me_to_those_events(widget);
   ck_assert_int_eq(c, 0);
   efl_ui_selectable_selected_set(efl_pack_content_get(widget, 2), EINA_TRUE);
   get_me_to_those_events(widget);
   ck_assert_int_eq(c, 0);
   ck_assert_int_eq(efl_ui_selectable_selected_get(efl_pack_content_get(widget, 0)), EINA_FALSE);
   ck_assert_int_eq(efl_ui_selectable_selected_get(efl_pack_content_get(widget, 2)), EINA_FALSE);
   ck_assert_ptr_eq(efl_ui_selectable_last_selected_get(widget), NULL);
   _iterator_to_array(&arr_selected, efl_ui_multi_selectable_selected_iterator_new(widget));
   ck_assert_int_eq(eina_array_count(arr_selected), 0);
   efl_event_callback_del(widget, EFL_UI_SELECTABLE_EVENT_SELECTION_CHANGED, (void*) event_callback_single_call_int_data, &c);
   efl_event_callback_del(widget, EFL_UI_SELECTABLE_EVENT_SELECTION_CHANGED, event_callback_that_quits_the_main_loop_when_called, NULL);
   eina_array_free(arr_selected);
}
EFL_END_TEST

EFL_START_TEST(all_select_api)
{
   Eina_Array *arr_selected;

   efl_ui_multi_selectable_select_mode_set(widget, EFL_UI_SELECT_MODE_MULTI);
   efl_ui_multi_selectable_all_select(widget);
   _iterator_to_array(&arr_selected, efl_ui_multi_selectable_selected_iterator_new(widget));

   ck_assert_int_eq(eina_array_count(arr_selected), 3);

   ck_assert_ptr_eq(eina_array_data_get(arr_selected, 0), efl_pack_content_get(widget, 0));
   ck_assert_ptr_eq(eina_array_data_get(arr_selected, 1), efl_pack_content_get(widget, 1));
   ck_assert_ptr_eq(eina_array_data_get(arr_selected, 2), efl_pack_content_get(widget, 2));
   eina_array_free(arr_selected);
}
EFL_END_TEST

EFL_START_TEST(all_unselect_api)
{
   Eina_Array *arr_selected;

   efl_ui_multi_selectable_select_mode_set(widget, EFL_UI_SELECT_MODE_MULTI);
   efl_ui_selectable_selected_set(efl_pack_content_get(widget, 0), EINA_TRUE);

   efl_ui_multi_selectable_all_unselect(widget);
   _iterator_to_array(&arr_selected, efl_ui_multi_selectable_selected_iterator_new(widget));

   ck_assert_int_eq(eina_array_count(arr_selected), 0);
   ck_assert_int_eq(efl_ui_selectable_selected_get(efl_pack_content_get(widget, 0)), EINA_FALSE);
   eina_array_free(arr_selected);
}
EFL_END_TEST

EFL_START_TEST(range_unselect)
{
   Eina_Array *arr_selected;

   efl_ui_multi_selectable_select_mode_set(widget, EFL_UI_SELECT_MODE_MULTI);
   efl_ui_multi_selectable_all_select(widget);

   efl_ui_multi_selectable_range_unselect(widget, efl_pack_content_get(widget, 1), efl_pack_content_get(widget, 2));
   _iterator_to_array(&arr_selected, efl_ui_multi_selectable_selected_iterator_new(widget));
   ck_assert_int_eq(eina_array_count(arr_selected), 1);
   ck_assert_ptr_eq(eina_array_data_get(arr_selected, 0), efl_pack_content_get(widget, 0));
   eina_array_free(arr_selected);
}
EFL_END_TEST

EFL_START_TEST(range_unselect2)
{
   Eina_Array *arr_selected;

   efl_ui_multi_selectable_select_mode_set(widget, EFL_UI_SELECT_MODE_MULTI);
   efl_ui_multi_selectable_all_select(widget);

   efl_ui_multi_selectable_range_unselect(widget, efl_pack_content_get(widget, 2), efl_pack_content_get(widget, 1));
   _iterator_to_array(&arr_selected, efl_ui_multi_selectable_selected_iterator_new(widget));
   ck_assert_int_eq(eina_array_count(arr_selected), 1);
   ck_assert_ptr_eq(eina_array_data_get(arr_selected, 0), efl_pack_content_get(widget, 0));
   eina_array_free(arr_selected);
}
EFL_END_TEST

EFL_START_TEST(range_select)
{
   Eina_Array *arr_selected;

   efl_ui_multi_selectable_select_mode_set(widget, EFL_UI_SELECT_MODE_MULTI);
   efl_ui_multi_selectable_range_select(widget, efl_pack_content_get(widget, 1), efl_pack_content_get(widget, 2));
   _iterator_to_array(&arr_selected, efl_ui_multi_selectable_selected_iterator_new(widget));
   ck_assert_int_eq(eina_array_count(arr_selected), 2);
   ck_assert_ptr_eq(eina_array_data_get(arr_selected, 0), efl_pack_content_get(widget, 1));
   ck_assert_ptr_eq(eina_array_data_get(arr_selected, 1), efl_pack_content_get(widget, 2));
   eina_array_free(arr_selected);
}
EFL_END_TEST

EFL_START_TEST(range_select2)
{
   Eina_Array *arr_selected;

   efl_ui_multi_selectable_select_mode_set(widget, EFL_UI_SELECT_MODE_MULTI);
   efl_ui_multi_selectable_range_select(widget, efl_pack_content_get(widget, 2), efl_pack_content_get(widget, 1));
   _iterator_to_array(&arr_selected, efl_ui_multi_selectable_selected_iterator_new(widget));
   ck_assert_int_eq(eina_array_count(arr_selected), 2);
   ck_assert_ptr_eq(eina_array_data_get(arr_selected, 0), efl_pack_content_get(widget, 1));
   ck_assert_ptr_eq(eina_array_data_get(arr_selected, 1), efl_pack_content_get(widget, 2));
   eina_array_free(arr_selected);
}
EFL_END_TEST

EFL_START_TEST(change_mode_from_multi_to_single)
{
   int sel = 0;

   efl_ui_multi_selectable_select_mode_set(widget, EFL_UI_SELECT_MODE_MULTI);
   efl_ui_multi_selectable_all_select(widget);
   efl_ui_multi_selectable_select_mode_set(widget, EFL_UI_SELECT_MODE_SINGLE);
   for (int i = 0; i < efl_content_count(widget); ++i)
     {
        if (efl_ui_selectable_selected_get(efl_pack_content_get(widget, i)))
          sel ++;
     }
   ck_assert_int_eq(sel, 1);
}
EFL_END_TEST


EFL_START_TEST(change_mode_from_multi_to_none)
{
   int sel = 0;

   efl_ui_multi_selectable_select_mode_set(widget, EFL_UI_SELECT_MODE_MULTI);
   efl_ui_multi_selectable_all_select(widget);
   efl_ui_multi_selectable_select_mode_set(widget, EFL_UI_SELECT_MODE_NONE);
   for (int i = 0; i < efl_content_count(widget); ++i)
     {
        if (efl_ui_selectable_selected_get(efl_pack_content_get(widget, i)))
          sel ++;
     }
   ck_assert_int_eq(sel, 0);
}
EFL_END_TEST

EFL_START_TEST(change_mode_from_single_to_none)
{
   int sel = 0;

   efl_ui_multi_selectable_select_mode_set(widget, EFL_UI_SELECT_MODE_SINGLE);
   efl_ui_multi_selectable_all_select(widget);
   efl_ui_multi_selectable_select_mode_set(widget, EFL_UI_SELECT_MODE_NONE);
   for (int i = 0; i < efl_content_count(widget); ++i)
     {
        if (efl_ui_selectable_selected_get(efl_pack_content_get(widget, i)))
          sel ++;
     }
   ck_assert_int_eq(sel, 0);
}
EFL_END_TEST

void
efl_ui_multi_selectable_behavior_test(TCase *tc)
{
   //Items are getting added by the fixture added in efl_ui_selectable_behavior_test
   tcase_add_test(tc, test_multi_select);
   tcase_add_test(tc, test_multi_select_removal);
   tcase_add_test(tc, test_single_select);
   tcase_add_test(tc, test_none_select);
   tcase_add_test(tc, all_select_api);
   tcase_add_test(tc, all_unselect_api);
   tcase_add_test(tc, range_unselect);
   tcase_add_test(tc, range_unselect2);
   tcase_add_test(tc, range_select);
   tcase_add_test(tc, range_select2);
   tcase_add_test(tc, change_mode_from_multi_to_single);
   tcase_add_test(tc, change_mode_from_multi_to_none);
   tcase_add_test(tc, change_mode_from_single_to_none);
   efl_ui_single_selectable_behavior_test(tc);
}
