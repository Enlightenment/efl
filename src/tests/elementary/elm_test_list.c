#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define ELM_INTERFACE_ATSPI_SELECTION_PROTECTED
#define ELM_INTERFACE_ATSPI_ACCESSIBLE_PROTECTED
#include <Elementary.h>
#include "elm_suite.h"

#if 0
// Skip following tests until Elm_Widget_Item will migrate to Eo infrastructure
// and following features can be implemented

START_TEST (elm_list_atspi_selection_selected_children_count_get)
{
 Evas_Object *win, *list;
 Elm_Object_Item *item;
 int val;

 elm_init(1, NULL);
 win = elm_win_add(NULL, "list", ELM_WIN_BASIC);

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

 elm_shutdown();
}
END_TEST

START_TEST (elm_list_atspi_selection_child_select)
{
 Evas_Object *win, *list;
 Elm_Object_Item *item;
 Eina_Bool val;

 elm_init(1, NULL);
 win = elm_win_add(NULL, "list", ELM_WIN_BASIC);

 list = elm_list_add(win);
 item = elm_list_item_append(list, "First Element", NULL, NULL, NULL, NULL);

 val = elm_interface_atspi_selection_child_select(list, 0);
 ck_assert(val == EINA_TRUE);
 ck_assert(EINA_TRUE == elm_list_item_selected_get(item));

 elm_shutdown();
}
END_TEST

START_TEST (elm_list_atspi_selection_selected_child_deselect)
{
 Evas_Object *win, *list;
 Elm_Object_Item *item;
 Eina_Bool val;

 elm_init(1, NULL);
 win = elm_win_add(NULL, "list", ELM_WIN_BASIC);

 list = elm_list_add(win);
 elm_list_multi_select_set(list, EINA_TRUE);
 elm_list_item_append(list, "First Element", NULL, NULL, NULL, NULL);
 item = elm_list_item_append(list, "Second Element", NULL, NULL, NULL, NULL);

 elm_list_item_selected_set(item, EINA_TRUE);
 val = elm_interface_atspi_selection_selected_child_deselect(list, 0);
 ck_assert(val == EINA_TRUE);
 ck_assert(EINA_FALSE == elm_list_item_selected_get(item));

 elm_shutdown();
}
END_TEST

START_TEST (elm_list_atspi_selection_is_child_selected)
{
 Evas_Object *win, *list;
 Elm_Object_Item *item;
 Eina_Bool val;

 elm_init(1, NULL);
 win = elm_win_add(NULL, "list", ELM_WIN_BASIC);

 list = elm_list_add(win);
 item = elm_list_item_append(list, "First Element", NULL, NULL, NULL, NULL);

 val = elm_interface_atspi_selection_is_child_selected(list, 0);
 ck_assert(val == EINA_FALSE);

 elm_list_item_selected_set(item, EINA_TRUE);
 val = elm_interface_atspi_selection_is_child_selected(list, 0);
 ck_assert(val == EINA_TRUE);

 elm_shutdown();
}
END_TEST

START_TEST (elm_list_atspi_selection_all_children_select)
{
 Evas_Object *win, *list;
 Elm_Object_Item *item1, *item2;
 Eina_Bool val;

 elm_init(1, NULL);
 win = elm_win_add(NULL, "list", ELM_WIN_BASIC);

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

 elm_shutdown();
}
END_TEST


START_TEST (elm_list_atspi_selection_clear)
{
 Evas_Object *win, *list;
 Elm_Object_Item *item1, *item2;
 Eina_Bool val;

 elm_init(1, NULL);
 win = elm_win_add(NULL, "list", ELM_WIN_BASIC);

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

 elm_shutdown();
}
END_TEST

START_TEST (elm_list_atspi_selection_child_deselect)
{
 Evas_Object *win, *list;
 Elm_Object_Item *item;
 Eina_Bool val;

 elm_init(1, NULL);
 win = elm_win_add(NULL, "list", ELM_WIN_BASIC);

 list = elm_list_add(win);
 elm_list_multi_select_set(list, EINA_TRUE);
 elm_list_item_append(list, "First Element", NULL, NULL, NULL, NULL);
 item = elm_list_item_append(list, "Second Element", NULL, NULL, NULL, NULL);

 elm_list_item_selected_set(item, EINA_TRUE);
 val = elm_interface_atspi_selection_selected_child_deselect(list, 1);
 ck_assert(val == EINA_TRUE);
 ck_assert(EINA_FALSE == elm_list_item_selected_get(item));

 elm_shutdown();
}
END_TEST

#endif

START_TEST (elm_atspi_role_get)
{
 Evas_Object *win, *list;
 Elm_Atspi_Role role;

 elm_init(1, NULL);
 win = elm_win_add(NULL, "list", ELM_WIN_BASIC);

 list = elm_list_add(win);
 role = elm_interface_atspi_accessible_role_get(list);

 ck_assert(role == ELM_ATSPI_ROLE_LIST);

 elm_shutdown();
}
END_TEST


/**
 * Validate if genlist implementation properly reset parent to Elm_Genlist_Item
 * from Elm_Genlist
 */
START_TEST(elm_atspi_children_parent)
{
   Elm_Interface_Atspi_Accessible *parent;

   elm_init(1, NULL);
   Evas_Object *win = elm_win_add(NULL, "list", ELM_WIN_BASIC);

   Evas_Object *icon = elm_icon_add(win);
   Evas_Object *end = elm_icon_add(win);

   Evas_Object *list = elm_list_add(win);
   elm_list_item_append(list, "First Element", icon, end, NULL, NULL);

   evas_object_show(list);

   parent = elm_interface_atspi_accessible_parent_get(icon);
   ck_assert(list == parent);

   parent = elm_interface_atspi_accessible_parent_get(end);
   ck_assert(list == parent);

   elm_shutdown();
}
END_TEST

void elm_test_list(TCase *tc)
{
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
}
