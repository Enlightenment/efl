#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define ELM_INTERNAL_API_ARGESFSDFEFC

#include <Elementary.h>
#include "elm_priv.h"
#include "elm_suite.h"

#define ELM_INTERFACE_ATSPI_ACCESSIBLE_PROTECTED
#include "elm_interface_atspi_accessible.h"
#include "elm_interface_atspi_accessible.eo.h"

#define ELM_INTERFACE_ATSPI_COMPONENT_PROTECTED
#include "elm_interface_atspi_component.eo.h"

static Evas_Object *g_win, *g_btn, *g_bg;

Eo* generate_app(void)
{
   g_win = elm_win_add(NULL, "Title", ELM_WIN_BASIC);
   evas_object_geometry_set(g_win, 100, 100, 100, 100);

   g_bg = elm_bg_add(g_win);

   g_btn = elm_button_add(g_win);

   evas_object_show(g_btn);
   evas_object_show(g_bg);
   evas_object_show(g_win);

   return _elm_atspi_bridge_root_get();
}

START_TEST (elm_atspi_name_get)
{
   elm_init(0, NULL);

   Eo* obj = generate_app();

   const char *ret = NULL;

   elm_app_name_set("Test name");

   eo_do(obj, ret = elm_interface_atspi_accessible_name_get());

   ck_assert_str_eq(ret, "Test name");

   elm_shutdown();
}
END_TEST

START_TEST (elm_atspi_role_get)
{
   elm_init(0, NULL);

   Eo *obj = generate_app();

   Elm_Atspi_Role role;

   eo_do(obj, role = elm_interface_atspi_accessible_role_get());

   ck_assert(role == ELM_ATSPI_ROLE_APPLICATION);

   elm_shutdown();
}
END_TEST


START_TEST (elm_atspi_role_name_get)
{
   elm_init(0, NULL);

   Eo *obj = generate_app();

   const char *ret = NULL;

   eo_do(obj, ret = elm_interface_atspi_accessible_role_name_get());

   ck_assert(ret != NULL);

   elm_shutdown();
}
END_TEST

START_TEST (elm_atspi_localized_role_name_get)
{
   elm_init(0, NULL);

   Eo *obj = generate_app();

   const char *ret = NULL;

   eo_do(obj, ret = elm_interface_atspi_accessible_localized_role_name_get());

   ck_assert(ret != NULL);

   elm_shutdown();
}
END_TEST

START_TEST (elm_atspi_description_set)
{
   elm_init(0, NULL);

   Eo *obj = generate_app();

   const char *ret = NULL;

   const char *desc = "Test description";

   eo_do(obj, ret = elm_interface_atspi_accessible_description_get());

   ck_assert(ret == NULL);

   eo_do(obj, elm_interface_atspi_accessible_description_set(desc));

   eo_do(obj, ret = elm_interface_atspi_accessible_description_get());

   ck_assert(ret != NULL);
   ck_assert_str_eq(ret, "Test description");

   elm_shutdown();
}
END_TEST

START_TEST (elm_atspi_children_and_parent)
{
   elm_init(0, NULL);

   Eo *root = generate_app();

   Eina_List *child_list = NULL;

   eo_do(root, child_list = elm_interface_atspi_accessible_children_get());

   //eo_do(eina_list_nth(child_list, 0), bg_child_list = elm_interface_atspi_accessible_children_get());

   ck_assert(eina_list_count(child_list) == 1);

   Eo *win = NULL;

   win = eina_list_nth(child_list, 0);

   ck_assert(win != NULL);
   ck_assert(win == g_win);

   Eo *win_parent = NULL;

   eo_do(win, win_parent = elm_interface_atspi_accessible_parent_get());

   ck_assert(root == win_parent);

   elm_shutdown();
}
END_TEST

START_TEST (elm_atspi_component_position)
{
   Eina_List *child_list = NULL;
   elm_init(0, NULL);

   Eo *root = generate_app();

   Eo *win = NULL;

   eo_do(root, child_list = elm_interface_atspi_accessible_children_get());

   win = eina_list_nth(child_list, 0);

   Eina_Bool ret = EINA_FALSE;

   eo_do(win, ret = elm_interface_atspi_component_position_set(EINA_TRUE, 100, 100));

   ck_assert(ret == EINA_TRUE);

   int x, y;

   eo_do(win, elm_interface_atspi_component_position_get(EINA_TRUE, &x, &y));

   ck_assert((x == 100) && (y == 100));

   elm_shutdown();
}
END_TEST

START_TEST (elm_atspi_component_size)
{
   Eina_List *child_list = NULL;
   elm_init(0, NULL);

   Eo *root = generate_app();

   Eo *win = NULL;

   eo_do(root, child_list = elm_interface_atspi_accessible_children_get());

   win = eina_list_nth(child_list, 0);

   Eina_Bool ret = EINA_FALSE;

   eo_do(win, ret = elm_interface_atspi_component_size_set(100, 100));

   ck_assert(ret == EINA_TRUE);

   int w, h;

   eo_do(win, elm_interface_atspi_component_size_get(&w, &h));

   ck_assert((w == 100) && (h == 100));

   elm_shutdown();
}
END_TEST

START_TEST (elm_atspi_component_focus)
{
   Eina_List *child_list = NULL;
   elm_init(0, NULL);

   Eo *root = generate_app();

   Eo *win = NULL;

   eo_do(root, child_list = elm_interface_atspi_accessible_children_get());

   win = eina_list_nth(child_list, 0);

   Eina_Bool ret = EINA_FALSE;

   eo_do(win, ret = elm_interface_atspi_component_focus_grab());

   ck_assert(ret == EINA_TRUE);

   elm_shutdown();
}
END_TEST

START_TEST (elm_test_atspi_object_children_get)
{
   elm_init(0, NULL);

   Eo *root = _elm_atspi_bridge_root_get();

   Eina_List *children = NULL;
   eo_do(root, children = elm_interface_atspi_accessible_children_get());

   ck_assert(children == NULL);

   eina_list_free(children);

   elm_shutdown();
}
END_TEST

START_TEST (elm_test_atspi_obj_index_in_parent_get)
{
   elm_init(0, NULL);

   Eo *root = generate_app();

   int ret = 0;

   Eina_List *children = NULL;
   eo_do(root, children = elm_interface_atspi_accessible_children_get());

   Evas_Object *win = eina_list_nth(children, 0);
   ck_assert(win != NULL);

   eo_do(win, ret = elm_interface_atspi_accessible_index_in_parent_get());

   ck_assert(ret == 0);

   elm_shutdown();
}
END_TEST

START_TEST (elm_atspi_component_z_order)
{
   elm_init(0, NULL);

   Eo *root = generate_app();

   Eo *win = NULL;

   Eina_List *children;
   eo_do(root, children = elm_interface_atspi_accessible_children_get());

   win = eina_list_nth(children, 0);
   int z_order = -1;

   eo_do(win, z_order = elm_interface_atspi_component_z_order_get());

   ck_assert(z_order != -1);

   elm_shutdown();
}
END_TEST

START_TEST (elm_atspi_component_layer)
{
   elm_init(0, NULL);

   Eo *root = generate_app();

   Eo *win = NULL;

   Eina_List *children;
   eo_do(root, children = elm_interface_atspi_accessible_children_get());

   win = eina_list_nth(children, 0);

   int layer = -1;

   eo_do(win, layer = elm_interface_atspi_component_layer_get());

   ck_assert(layer != -1);

   elm_shutdown();
}
END_TEST

START_TEST (elm_atspi_component_alpha)
{
   elm_init(0, NULL);

   Eo *root = generate_app();

   Eo *win = NULL;

   Eina_List *children;
   eo_do(root, children = elm_interface_atspi_accessible_children_get());
   win = eina_list_nth(children, 0);

   double alpha = -1.0;

   eo_do(win, alpha = elm_interface_atspi_component_alpha_get());

   ck_assert(alpha >= 0.0);
   ck_assert(alpha <= 1.0);

   elm_shutdown();
}
END_TEST

START_TEST (elm_atspi_children_and_parent2)
{
   elm_init(0, NULL);

   Eo *root = generate_app();

   Eo *win = NULL;

   Eina_List *root_children;
   eo_do(root, root_children = elm_interface_atspi_accessible_children_get());
   win = eina_list_nth(root_children, 0);

   Eina_List *win_children;
   eo_do(win, win_children = elm_interface_atspi_accessible_children_get());

   ck_assert(eina_list_count(win_children) == 2);

   Eo *btn = NULL;

   btn = eina_list_nth(win_children, 1);
   ck_assert(btn != NULL);
   ck_assert(btn == g_btn);

   elm_shutdown();
}
END_TEST

void elm_test_atspi(TCase *tc)
{
   tcase_add_test(tc, elm_atspi_name_get);
   tcase_add_test(tc, elm_atspi_role_get);
   tcase_add_test(tc, elm_atspi_role_name_get);
   tcase_add_test(tc, elm_atspi_localized_role_name_get);
   tcase_add_test(tc, elm_atspi_description_set);
   tcase_add_test(tc, elm_atspi_children_and_parent);
   tcase_add_test(tc, elm_atspi_component_position);
   tcase_add_test(tc, elm_atspi_component_size);
   tcase_add_test(tc, elm_atspi_component_focus);
   tcase_add_test(tc, elm_test_atspi_object_children_get);
   tcase_add_test(tc, elm_test_atspi_obj_index_in_parent_get);
   tcase_add_test(tc, elm_atspi_component_z_order);
   tcase_add_test(tc, elm_atspi_component_layer);
   tcase_add_test(tc, elm_atspi_component_alpha);
   tcase_add_test(tc, elm_atspi_children_and_parent2);
}

/*
 *    TO DO
 *    elm_interface_atspi_accessible_relation_set_get
 *    elm_interface_atspi_accessible_relation_set_set
 *    elm_interface_atspi_accessible_state_get
 *    elm_interface_atspi_accessible_attributes_get
 *    elm_interface_atspi_component_contains
*/
