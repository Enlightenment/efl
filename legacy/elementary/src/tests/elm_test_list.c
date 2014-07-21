#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_suite.h"

#define ELM_INTERFACE_ATSPI_SELECTION_PROTECTED
#include "elm_interface_atspi_selection.eo.h"

#define ELM_INTERFACE_ATSPI_ACCESSIBLE_PROTECTED
#include "elm_interface_atspi_accessible.h"
#include "elm_interface_atspi_accessible.eo.h"

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

 eo_do(list, val = elm_interface_atspi_selection_selected_children_count_get());
 ck_assert(val == 0);

 elm_list_item_selected_set(item, EINA_TRUE);
 eo_do(list, val = elm_interface_atspi_selection_selected_children_count_get());
 ck_assert(val == 1);

 elm_list_item_selected_set(item, EINA_FALSE);
 eo_do(list, val = elm_interface_atspi_selection_selected_children_count_get());
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

 eo_do(list, val = elm_interface_atspi_selection_child_select(0));
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
 eo_do(list, val = elm_interface_atspi_selection_selected_child_deselect(0));
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

 eo_do(list, val = elm_interface_atspi_selection_is_child_selected(0));
 ck_assert(val == EINA_FALSE);

 elm_list_item_selected_set(item, EINA_TRUE);
 eo_do(list, val = elm_interface_atspi_selection_is_child_selected(0));
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

 eo_do(list, val = elm_interface_atspi_selection_all_children_select());
 ck_assert(val == EINA_FALSE);

 elm_list_multi_select_set(list, EINA_TRUE);
 eo_do(list, val = elm_interface_atspi_selection_all_children_select());
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


 eo_do(list, val = elm_interface_atspi_selection_clear());
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
 eo_do(list, val = elm_interface_atspi_selection_selected_child_deselect(1));
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
 eo_do(list, role = elm_interface_atspi_accessible_role_get());

 ck_assert(role == ELM_ATSPI_ROLE_LIST);

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
}
