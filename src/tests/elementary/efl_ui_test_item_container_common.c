#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Efl_Ui.h>
#include "efl_ui_suite.h"
#include "efl_ui_test_item_container_common.h"

Eo *item_container;

void
fill_items(const Efl_Class *klass)
{
  for (int i = 0; i < 3; ++i)
    {
       char buf[PATH_MAX];
       Eo *it = efl_add(klass, item_container);

       snprintf(buf, sizeof(buf), "%d - Test %d", i, i%13);
       efl_text_set(it, buf);
       efl_pack_end(item_container, it);
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
   efl_ui_select_mode_set(item_container, EFL_UI_SELECT_MODE_MULTI);
   efl_event_callback_add(item_container, EFL_UI_EVENT_ITEM_SELECTED, _set_pointer_quit, &selected);
   efl_event_callback_add(item_container, EFL_UI_EVENT_ITEM_UNSELECTED, _set_pointer_quit, &unselected);
   fill_items(EFL_UI_LIST_DEFAULT_ITEM_CLASS);

   efl_ui_item_selected_set(efl_pack_content_get(item_container, 0), EINA_TRUE);
   ck_assert_ptr_eq(selected, efl_pack_content_get(item_container, 0));
   ck_assert_ptr_eq(unselected, NULL);
   selected = NULL;
   unselected = NULL;
   efl_ui_item_selected_set(efl_pack_content_get(item_container, 2), EINA_TRUE);
   ck_assert_ptr_eq(selected, efl_pack_content_get(item_container, 2));
   ck_assert_ptr_eq(unselected, NULL);
   selected = NULL;
   unselected = NULL;
   ck_assert_int_eq(efl_ui_item_selected_get(efl_pack_content_get(item_container, 0)), EINA_TRUE);
   ck_assert_int_eq(efl_ui_item_selected_get(efl_pack_content_get(item_container, 2)), EINA_TRUE);
   ck_assert_ptr_eq(efl_ui_item_container_last_selected_item_get(item_container), efl_pack_content_get(item_container, 2));
   _iterator_to_array(&arr_selected, efl_ui_item_container_selected_items_get(item_container));
   ck_assert_int_eq(eina_array_count(arr_selected), 2);
   ck_assert_ptr_eq(eina_array_data_get(arr_selected, 0), efl_pack_content_get(item_container, 0));
   ck_assert_ptr_eq(eina_array_data_get(arr_selected, 1), efl_pack_content_get(item_container, 2));

   efl_ui_item_selected_set(efl_pack_content_get(item_container, 2), EINA_TRUE);
   ck_assert_ptr_eq(selected, NULL);
   ck_assert_ptr_eq(unselected, NULL);
   selected = NULL;
   unselected = NULL;
   efl_event_callback_del(item_container, EFL_UI_EVENT_ITEM_SELECTED, _set_pointer_quit, &selected);
   efl_event_callback_del(item_container, EFL_UI_EVENT_ITEM_UNSELECTED, _set_pointer_quit, &unselected);
}
EFL_END_TEST

EFL_START_TEST(test_multi_select_removal)
{
   Efl_Ui_Item *selected = NULL;
   Efl_Ui_Item *unselected = NULL;
   Eina_Array *arr_selected;
   efl_ui_select_mode_set(item_container, EFL_UI_SELECT_MODE_MULTI);
   efl_event_callback_add(item_container, EFL_UI_EVENT_ITEM_SELECTED, _set_pointer_quit, &selected);
   efl_event_callback_add(item_container, EFL_UI_EVENT_ITEM_UNSELECTED, _set_pointer_quit, &unselected);
   fill_items(EFL_UI_LIST_DEFAULT_ITEM_CLASS);

   efl_ui_item_selected_set(efl_pack_content_get(item_container, 0), EINA_TRUE);
   selected = NULL;//No need to ckeck the flag, we asserted in the tcase before
   efl_ui_item_selected_set(efl_pack_content_get(item_container, 2), EINA_TRUE);
   selected = NULL;//No need to ckeck the flag, we asserted in the tcase before
   unselected = NULL;
   efl_ui_item_selected_set(efl_pack_content_get(item_container, 0), EINA_FALSE);
   ck_assert_ptr_eq(selected, NULL);
   ck_assert_ptr_eq(unselected, efl_pack_content_get(item_container, 0));
   selected = NULL;
   unselected = NULL;
   efl_ui_item_selected_set(efl_pack_content_get(item_container, 2), EINA_FALSE);
   ck_assert_ptr_eq(selected, NULL);
   ck_assert_ptr_eq(unselected, efl_pack_content_get(item_container, 2));
   selected = NULL;
   unselected = NULL;

   ck_assert_ptr_eq(efl_ui_item_container_last_selected_item_get(item_container), NULL);
   _iterator_to_array(&arr_selected, efl_ui_item_container_selected_items_get(item_container));
   ck_assert_int_eq(eina_array_count(arr_selected), 0);
   efl_event_callback_del(item_container, EFL_UI_EVENT_ITEM_SELECTED, _set_pointer_quit, &selected);
   efl_event_callback_del(item_container, EFL_UI_EVENT_ITEM_UNSELECTED, _set_pointer_quit, &unselected);
}
EFL_END_TEST

EFL_START_TEST(test_single_select)
{
   Efl_Ui_Item *selected = NULL;
   Efl_Ui_Item *unselected = NULL;
   Eina_Array *arr_selected;

   efl_ui_select_mode_set(item_container, EFL_UI_SELECT_MODE_SINGLE);
   efl_event_callback_add(item_container, EFL_UI_EVENT_ITEM_SELECTED, _set_pointer_quit, &selected);
   efl_event_callback_add(item_container, EFL_UI_EVENT_ITEM_UNSELECTED, _set_pointer_quit, &unselected);
   fill_items(EFL_UI_LIST_DEFAULT_ITEM_CLASS);

   efl_ui_item_selected_set(efl_pack_content_get(item_container, 0), EINA_TRUE);
   ck_assert_ptr_eq(selected, efl_pack_content_get(item_container, 0));
   ck_assert_ptr_eq(unselected, NULL);
   selected = NULL;
   unselected = NULL;
   efl_ui_item_selected_set(efl_pack_content_get(item_container, 2), EINA_TRUE);
   ck_assert_ptr_eq(selected, efl_pack_content_get(item_container, 2));
   ck_assert_ptr_eq(unselected, efl_pack_content_get(item_container, 0));
   selected = NULL;
   unselected = NULL;
   ck_assert_int_eq(efl_ui_item_selected_get(efl_pack_content_get(item_container, 0)), EINA_FALSE);
   ck_assert_int_eq(efl_ui_item_selected_get(efl_pack_content_get(item_container, 2)), EINA_TRUE);
   ck_assert_ptr_eq(efl_ui_item_container_last_selected_item_get(item_container), efl_pack_content_get(item_container, 2));
   _iterator_to_array(&arr_selected, efl_ui_item_container_selected_items_get(item_container));
   ck_assert_int_eq(eina_array_count(arr_selected), 1);
   ck_assert_ptr_eq(eina_array_data_get(arr_selected, 0), efl_pack_content_get(item_container, 2));

   efl_ui_item_selected_set(efl_pack_content_get(item_container, 2), EINA_TRUE);
   ck_assert_ptr_eq(selected, NULL);
   ck_assert_ptr_eq(unselected, NULL);
   selected = NULL;
   unselected = NULL;
   efl_event_callback_del(item_container, EFL_UI_EVENT_ITEM_SELECTED, _set_pointer_quit, &selected);
   efl_event_callback_del(item_container, EFL_UI_EVENT_ITEM_UNSELECTED, _set_pointer_quit, &unselected);
}
EFL_END_TEST

EFL_START_TEST(test_single_select_always)
{
   Efl_Ui_Item *selected = NULL;
   Efl_Ui_Item *unselected = NULL;
   Eina_Array *arr_selected;

   efl_ui_select_mode_set(item_container, EFL_UI_SELECT_MODE_SINGLE_ALWAYS);
   efl_event_callback_add(item_container, EFL_UI_EVENT_ITEM_SELECTED, _set_pointer_quit, &selected);
   efl_event_callback_add(item_container, EFL_UI_EVENT_ITEM_UNSELECTED, _set_pointer_quit, &unselected);
   fill_items(EFL_UI_LIST_DEFAULT_ITEM_CLASS);

   efl_ui_item_selected_set(efl_pack_content_get(item_container, 0), EINA_TRUE);
   ck_assert_ptr_eq(selected, efl_pack_content_get(item_container, 0));
   ck_assert_ptr_eq(unselected, NULL);
   selected = NULL;
   unselected = NULL;
   efl_ui_item_selected_set(efl_pack_content_get(item_container, 2), EINA_TRUE);
   ck_assert_ptr_eq(selected, efl_pack_content_get(item_container, 2));
   ck_assert_ptr_eq(unselected, efl_pack_content_get(item_container, 0));
   selected = NULL;
   unselected = NULL;
   efl_ui_item_selected_set(efl_pack_content_get(item_container, 2), EINA_TRUE);
   ck_assert_ptr_eq(selected, efl_pack_content_get(item_container, 2));
   ck_assert_ptr_eq(unselected, NULL);
   selected = NULL;
   unselected = NULL;
   ck_assert_int_eq(efl_ui_item_selected_get(efl_pack_content_get(item_container, 0)), EINA_FALSE);
   ck_assert_int_eq(efl_ui_item_selected_get(efl_pack_content_get(item_container, 2)), EINA_TRUE);
   ck_assert_ptr_eq(efl_ui_item_container_last_selected_item_get(item_container), efl_pack_content_get(item_container, 2));
   _iterator_to_array(&arr_selected, efl_ui_item_container_selected_items_get(item_container));
   ck_assert_int_eq(eina_array_count(arr_selected), 1);
   ck_assert_ptr_eq(eina_array_data_get(arr_selected, 0), efl_pack_content_get(item_container, 2));
   efl_event_callback_del(item_container, EFL_UI_EVENT_ITEM_SELECTED, _set_pointer_quit, &selected);
   efl_event_callback_del(item_container, EFL_UI_EVENT_ITEM_UNSELECTED, _set_pointer_quit, &unselected);
}
EFL_END_TEST

EFL_START_TEST(test_none_select)
{
   Efl_Ui_Item *selected = NULL;
   Efl_Ui_Item *unselected = NULL;
   Eina_Array *arr_selected;

   efl_ui_select_mode_set(item_container, EFL_UI_SELECT_MODE_NONE);
   efl_event_callback_add(item_container, EFL_UI_EVENT_ITEM_SELECTED, _set_pointer_quit, &selected);
   efl_event_callback_add(item_container, EFL_UI_EVENT_ITEM_UNSELECTED, _set_pointer_quit, &unselected);
   fill_items(EFL_UI_LIST_DEFAULT_ITEM_CLASS);

   efl_ui_item_selected_set(efl_pack_content_get(item_container, 0), EINA_TRUE);
   ck_assert_ptr_eq(selected, NULL);
   ck_assert_ptr_eq(unselected, NULL);
   selected = NULL;
   unselected = NULL;
   efl_ui_item_selected_set(efl_pack_content_get(item_container, 2), EINA_TRUE);
   ck_assert_ptr_eq(selected, NULL);
   ck_assert_ptr_eq(unselected, NULL);
   selected = NULL;
   unselected = NULL;
   ck_assert_int_eq(efl_ui_item_selected_get(efl_pack_content_get(item_container, 0)), EINA_FALSE);
   ck_assert_int_eq(efl_ui_item_selected_get(efl_pack_content_get(item_container, 2)), EINA_FALSE);
   ck_assert_ptr_eq(efl_ui_item_container_last_selected_item_get(item_container), NULL);
   _iterator_to_array(&arr_selected, efl_ui_item_container_selected_items_get(item_container));
   ck_assert_int_eq(eina_array_count(arr_selected), 0);
   efl_event_callback_del(item_container, EFL_UI_EVENT_ITEM_SELECTED, _set_pointer_quit, &selected);
   efl_event_callback_del(item_container, EFL_UI_EVENT_ITEM_UNSELECTED, _set_pointer_quit, &unselected);
}
EFL_END_TEST

void efl_ui_test_item_container_common_add(TCase *tc)
{
   tcase_add_test(tc, test_multi_select);
   tcase_add_test(tc, test_multi_select_removal);
   tcase_add_test(tc, test_single_select);
   tcase_add_test(tc, test_none_select);
   tcase_add_test(tc, test_single_select_always);
}
