#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_SELECTION_PROTECTED
#define EFL_ACCESS_OBJECT_BETA
#define EFL_ACCESS_OBJECT_PROTECTED
#include <Elementary.h>
#include "elm_suite.h"

EFL_START_TEST(elm_list_legacy_type_check)
{
   Evas_Object *win, *list;
   const char *type;

   win = win_add(NULL, "list", ELM_WIN_BASIC);

   list = elm_list_add(win);

   type = elm_object_widget_type_get(list);
   ck_assert(type != NULL);
   ck_assert(!strcmp(type, "Elm_List"));

   type = evas_object_type_get(list);
   ck_assert(type != NULL);
   ck_assert(!strcmp(type, "elm_list"));

}
EFL_END_TEST

#if 0
// Skip following tests until Elm_Widget_Item will migrate to Eo infrastructure
// and following features can be implemented

EFL_START_TEST(elm_list_atspi_selection_selected_children_count_get)
{
 Evas_Object *win, *list;
 Elm_Object_Item *item;
 int val;


 win = win_add(NULL, "list", ELM_WIN_BASIC);

 list = elm_list_add(win);
 item = elm_list_item_append(list, "First Element", NULL, NULL, NULL, NULL);

 val = elm_interface_atspi_selection_selected_children_count_get(list);
 ck_assert(val == 0);

 elm_list_item_selected_set(item, EINA_TRUE);
 val = elm_interface_atspi_selection_selected_children_count_get(list);
 ck_assert(val == 1);

 elm_list_item_selected_set(item, EINA_FALSE);
 val = elm_interface_atspi_selection_selected_children_count_get(list);
 ck_assert(val == 0);

}
EFL_END_TEST

EFL_START_TEST(elm_list_atspi_selection_child_select)
{
 Evas_Object *win, *list;
 Elm_Object_Item *item;
 Eina_Bool val;


 win = win_add(NULL, "list", ELM_WIN_BASIC);

 list = elm_list_add(win);
 item = elm_list_item_append(list, "First Element", NULL, NULL, NULL, NULL);

 val = elm_interface_atspi_selection_child_select(list, 0);
 ck_assert(val == EINA_TRUE);
 ck_assert(EINA_TRUE == elm_list_item_selected_get(item));

}
EFL_END_TEST

EFL_START_TEST(elm_list_atspi_selection_selected_child_deselect)
{
 Evas_Object *win, *list;
 Elm_Object_Item *item;
 Eina_Bool val;


 win = win_add(NULL, "list", ELM_WIN_BASIC);

 list = elm_list_add(win);
 elm_list_multi_select_set(list, EINA_TRUE);
 elm_list_item_append(list, "First Element", NULL, NULL, NULL, NULL);
 item = elm_list_item_append(list, "Second Element", NULL, NULL, NULL, NULL);

 elm_list_item_selected_set(item, EINA_TRUE);
 val = elm_interface_atspi_selection_selected_child_deselect(list, 0);
 ck_assert(val == EINA_TRUE);
 ck_assert(EINA_FALSE == elm_list_item_selected_get(item));

}
EFL_END_TEST

EFL_START_TEST(elm_list_atspi_selection_is_child_selected)
{
 Evas_Object *win, *list;
 Elm_Object_Item *item;
 Eina_Bool val;


 win = win_add(NULL, "list", ELM_WIN_BASIC);

 list = elm_list_add(win);
 item = elm_list_item_append(list, "First Element", NULL, NULL, NULL, NULL);

 val = elm_interface_atspi_selection_is_child_selected(list, 0);
 ck_assert(val == EINA_FALSE);

 elm_list_item_selected_set(item, EINA_TRUE);
 val = elm_interface_atspi_selection_is_child_selected(list, 0);
 ck_assert(val == EINA_TRUE);

}
EFL_END_TEST

EFL_START_TEST(elm_list_atspi_selection_all_children_select)
{
 Evas_Object *win, *list;
 Elm_Object_Item *item1, *item2;
 Eina_Bool val;


 win = win_add(NULL, "list", ELM_WIN_BASIC);

 list = elm_list_add(win);

 item1 = elm_list_item_append(list, "First Element", NULL, NULL, NULL, NULL);
 item2 = elm_list_item_append(list, "Second Element", NULL, NULL, NULL, NULL);

 val = elm_interface_atspi_selection_all_children_select(list);
 ck_assert(val == EINA_FALSE);

 elm_list_multi_select_set(list, EINA_TRUE);
 val = elm_interface_atspi_selection_all_children_select(list);
 ck_assert(val == EINA_TRUE);
 ck_assert(EINA_TRUE == elm_list_item_selected_get(item1));
 ck_assert(EINA_TRUE == elm_list_item_selected_get(item2));

}
EFL_END_TEST


EFL_START_TEST(elm_list_atspi_selection_clear)
{
 Evas_Object *win, *list;
 Elm_Object_Item *item1, *item2;
 Eina_Bool val;


 win = win_add(NULL, "list", ELM_WIN_BASIC);

 list = elm_list_add(win);
 elm_list_multi_select_set(list, EINA_TRUE);
 item1 = elm_list_item_append(list, "First Element", NULL, NULL, NULL, NULL);
 item2 = elm_list_item_append(list, "Second Element", NULL, NULL, NULL, NULL);
 elm_list_item_selected_set(item1, EINA_TRUE);
 elm_list_item_selected_set(item2, EINA_TRUE);


 val = elm_interface_atspi_selection_clear(list);
 ck_assert(val == EINA_TRUE);
 ck_assert(EINA_FALSE == elm_list_item_selected_get(item1));
 ck_assert(EINA_FALSE == elm_list_item_selected_get(item2));

}
EFL_END_TEST

EFL_START_TEST(elm_list_atspi_selection_child_deselect)
{
 Evas_Object *win, *list;
 Elm_Object_Item *item;
 Eina_Bool val;


 win = win_add(NULL, "list", ELM_WIN_BASIC);

 list = elm_list_add(win);
 elm_list_multi_select_set(list, EINA_TRUE);
 elm_list_item_append(list, "First Element", NULL, NULL, NULL, NULL);
 item = elm_list_item_append(list, "Second Element", NULL, NULL, NULL, NULL);

 elm_list_item_selected_set(item, EINA_TRUE);
 val = elm_interface_atspi_selection_selected_child_deselect(list, 1);
 ck_assert(val == EINA_TRUE);
 ck_assert(EINA_FALSE == elm_list_item_selected_get(item));

}
EFL_END_TEST

#endif

EFL_START_TEST(elm_atspi_role_get)
{
 Evas_Object *win, *list;
 Efl_Access_Role role;


 win = win_add(NULL, "list", ELM_WIN_BASIC);

 list = elm_list_add(win);
 role = efl_access_object_role_get(list);

 ck_assert(role == EFL_ACCESS_ROLE_LIST);

}
EFL_END_TEST


/**
 * Validate if genlist implementation properly reset parent to Elm_Genlist_Item
 * from Elm_Genlist
 */
EFL_START_TEST(elm_atspi_children_parent)
{
   Efl_Access_Object *parent;
   Elm_Object_Item *it;

   Evas_Object *win = win_add(NULL, "list", ELM_WIN_BASIC);

   Evas_Object *icon = elm_icon_add(win);
   Evas_Object *end = elm_icon_add(win);

   Evas_Object *list = elm_list_add(win);
   it = elm_list_item_append(list, "First Element", icon, end, NULL, NULL);

   evas_object_show(list);

   parent = efl_provider_find(efl_parent_get(icon), EFL_ACCESS_OBJECT_MIXIN);
   ck_assert(it == parent);

   parent = efl_provider_find(efl_parent_get(end), EFL_ACCESS_OBJECT_MIXIN);
   ck_assert(it == parent);

}
EFL_END_TEST

#define NUM_ITEMS 10
EFL_START_TEST(elm_list_test_callbacks)
{
   Evas_Object *win, *list;
   unsigned int i;
   int count[NUM_ITEMS] = {0};
   Elm_Object_Item *it;
   int selected = 0, unselected = 0;
   int highlighted = 0, unhighlighted = 0;

   win = win_add(NULL, "list", ELM_WIN_BASIC);
   evas_object_resize(win, 100, 100);

   list = elm_list_add(win);
   evas_object_smart_callback_add(list, "selected", (void*)event_callback_that_increments_an_int_when_called, &selected);
   evas_object_smart_callback_add(list, "unselected", (void*)event_callback_that_increments_an_int_when_called, &unselected);
   evas_object_smart_callback_add(list, "highlighted", (void*)event_callback_that_increments_an_int_when_called, &highlighted);
   evas_object_smart_callback_add(list, "unhighlighted", (void*)event_callback_that_increments_an_int_when_called, &unhighlighted);
   evas_object_resize(list, 100, 100);
   for (i = 0; i < NUM_ITEMS; i++)
     elm_list_item_append(list, "item", NULL, NULL, (void*)event_callback_single_call_int_data, &(count[i]));

   elm_list_go(list);
   evas_object_show(list);
   evas_object_show(win);
   get_me_to_those_events(win);

   for (i = 0, it = elm_list_first_item_get(list); i < NUM_ITEMS; i++, it = elm_list_item_next(it))
     {
        elm_list_item_selected_set(it, EINA_TRUE);
        ck_assert_int_eq(count[i], 1);

        ck_assert_int_eq(selected, i + 1);
        ck_assert_int_eq(unselected, i);

        ck_assert_int_eq(highlighted, i + 1);
        ck_assert_int_eq(unhighlighted, i);
     }
   ck_assert_int_eq(selected, 10);
   ck_assert_int_eq(unselected, 9);

   ck_assert_int_eq(highlighted, 10);
   ck_assert_int_eq(unhighlighted, 9);

   /* weird SIGILL in shutdown if the list isn't deleted here */
   evas_object_del(list);
}
EFL_END_TEST

static Eo *
create_content_with_size(Eo *parent, int mw, int mh)
{
   Evas *e = evas_object_evas_get(parent);
   Eo *rect = evas_object_rectangle_add(e);

   evas_object_size_hint_min_set(rect, mw, mh);
   evas_object_show(rect);

   return rect;
}

EFL_START_TEST(elm_list_test_sizing)
{
   Evas_Object *win, *list;
   unsigned int i;
   int count[NUM_ITEMS] = {0};
   Elm_Object_Item *it;

   win = win_add(NULL, "list", ELM_WIN_BASIC);
   evas_object_resize(win, 100, 100);

   list = elm_list_add(win);
   evas_object_resize(list, 100, 100);
   for (i = 0; i < NUM_ITEMS; i++)
     elm_list_item_append(list, "item", create_content_with_size(list, i * 5, i * 5), NULL, (void*)event_callback_single_call_int_data, &(count[i]));

   elm_list_go(list);
   evas_object_show(list);
   evas_object_show(win);
   get_me_to_those_events(win);

   for (i = 0, it = elm_list_first_item_get(list); i < NUM_ITEMS; i++, it = elm_list_item_next(it))
     {
        Eo *rect = elm_object_item_content_get(it);
        ck_assert(rect);
        /* list is always homogeneous, so these should all have the size of the largest min size */
        assert_object_size_eq(rect, (NUM_ITEMS - 1) * 5, (NUM_ITEMS - 1) * 5);
     }

   /* weird SIGILL in shutdown if the list isn't deleted here */
   evas_object_del(list);
}
EFL_END_TEST

void elm_test_list(TCase *tc)
{
   tcase_add_test(tc, elm_list_legacy_type_check);
   tcase_add_test(tc, elm_atspi_role_get);
#if 0
   tcase_add_test(tc, elm_list_atspi_selection_selected_children_count_get);
   tcase_add_test(tc, elm_list_atspi_selection_child_select);
   tcase_add_test(tc, elm_list_atspi_selection_selected_child_deselect);
   tcase_add_test(tc, elm_list_atspi_selection_is_child_selected);
   tcase_add_test(tc, elm_list_atspi_selection_all_children_select);
   tcase_add_test(tc, elm_list_atspi_selection_clear);
   tcase_add_test(tc, elm_list_atspi_selection_child_deselect);
#endif
   tcase_add_test(tc, elm_atspi_children_parent);
   tcase_add_test(tc, elm_list_test_callbacks);
   tcase_add_test(tc, elm_list_test_sizing);
}
