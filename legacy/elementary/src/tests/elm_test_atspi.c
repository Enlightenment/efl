#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define ELM_INTERNAL_API_ARGESFSDFEFC

#include <Elementary.h>
#include "elm_priv.h"
#include "elm_suite.h"

#include "elm_atspi_app_object.eo.h"

#define ELM_INTERFACE_ATSPI_ACCESSIBLE_PROTECTED
#include "elm_interface_atspi_accessible.h"
#include "elm_interface_atspi_accessible.eo.h"

static Evas_Object *g_win, *g_btn, *g_bg;

void generate_app(void)
{
   g_win = elm_win_add(NULL, "Title", ELM_WIN_BASIC);
   evas_object_geometry_set(g_win, 100, 100, 100, 100);

   g_bg = elm_bg_add(g_win);

   g_btn = elm_button_add(g_win);

   evas_object_show(g_btn);
   evas_object_show(g_bg);
   evas_object_show(g_win);
}

START_TEST (elm_atspi_name_get)
{
   elm_init(0, NULL);

   Eo* root = eo_add(ELM_ATSPI_APP_OBJECT_CLASS, NULL);

   ck_assert(root != NULL);

   const char *ret = NULL;

   elm_app_name_set("Test name");

   eo_do(root, ret = elm_interface_atspi_accessible_name_get());

   ck_assert_str_eq(ret, "Test name");

   eo_unref(root);
   elm_shutdown();
}
END_TEST

START_TEST (elm_atspi_role_get)
{
   elm_init(0, NULL);

   Eo* root = eo_add(ELM_ATSPI_APP_OBJECT_CLASS, NULL);

   Elm_Atspi_Role role;

   eo_do(root, role = elm_interface_atspi_accessible_role_get());

   ck_assert(role == ELM_ATSPI_ROLE_APPLICATION);

   eo_unref(root);
   elm_shutdown();
}
END_TEST


START_TEST (elm_atspi_role_name_get)
{
   elm_init(0, NULL);

   Eo* root = eo_add(ELM_ATSPI_APP_OBJECT_CLASS, NULL);

   const char *ret = NULL;

   eo_do(root, ret = elm_interface_atspi_accessible_role_name_get());

   ck_assert(ret != NULL);

   eo_unref(root);
   elm_shutdown();
}
END_TEST

START_TEST (elm_atspi_localized_role_name_get)
{
   elm_init(0, NULL);

   Eo* root = eo_add(ELM_ATSPI_APP_OBJECT_CLASS, NULL);

   const char *ret = NULL;

   eo_do(root, ret = elm_interface_atspi_accessible_localized_role_name_get());

   ck_assert(ret != NULL);

   eo_unref(root);
   elm_shutdown();
}
END_TEST

START_TEST (elm_atspi_description_set)
{
   elm_init(0, NULL);

   Eo* root = eo_add(ELM_ATSPI_APP_OBJECT_CLASS, NULL);

   const char *ret = NULL;

   const char *desc = "Test description";

   eo_do(root, ret = elm_interface_atspi_accessible_description_get());

   ck_assert(ret == NULL);

   eo_do(root, elm_interface_atspi_accessible_description_set(desc));

   eo_do(root, ret = elm_interface_atspi_accessible_description_get());

   ck_assert(ret != NULL);
   ck_assert_str_eq(ret, "Test description");

   eo_unref(root);
   elm_shutdown();
}
END_TEST

START_TEST (elm_atspi_children_and_parent)
{
   elm_init(0, NULL);

   generate_app();
   Eo* root = eo_add(ELM_ATSPI_APP_OBJECT_CLASS, NULL);

   Eina_List *child_list = NULL;

   eo_do(root, child_list = elm_interface_atspi_accessible_children_get());

   //eo_do(eina_list_nth(child_list, 0), bg_child_list = elm_interface_atspi_accessible_children_get());

   ck_assert(eina_list_count(child_list) == 1);

   Eo *win = NULL;

   win = eina_list_nth(child_list, 0);

   ck_assert(win != NULL);
   ck_assert(win == g_win);

   eo_unref(root);
   elm_shutdown();
}
END_TEST

START_TEST (elm_atspi_children_and_parent2)
{
   elm_init(0, NULL);

   generate_app();
   Eo* root = eo_add(ELM_ATSPI_APP_OBJECT_CLASS, NULL);

   Eo *win = NULL;

   Eina_List *root_children;
   eo_do(root, root_children = elm_interface_atspi_accessible_children_get());
   win = eina_list_nth(root_children, 0);

   Eina_List *win_children;
   eo_do(win, win_children = elm_interface_atspi_accessible_children_get());

   ck_assert(eina_list_count(win_children) == 2);

   Eo *btn = NULL;

   btn = eina_list_nth(win_children, 0);
   ck_assert(btn != NULL);
   ck_assert(btn == g_bg);

   btn = eina_list_nth(win_children, 1);
   ck_assert(btn != NULL);
   ck_assert(btn == g_btn);

   eo_unref(root);
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
   tcase_add_test(tc, elm_atspi_children_and_parent2);
}
