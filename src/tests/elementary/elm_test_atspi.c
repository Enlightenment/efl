#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define ELM_INTERNAL_API_ARGESFSDFEFC
#define EFL_ACCESS_BETA
#define EFL_ACCESS_PROTECTED

#include <Elementary.h>
#include "elm_priv.h"
#include "elm_suite.h"

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

START_TEST (test_efl_access_app_obj_name_get)
{
   char *args[] = { "exe" };
   elm_init(1, args);

   Eo* root = efl_add_ref(ELM_ATSPI_APP_OBJECT_CLASS, NULL);

   ck_assert(root != NULL);

   const char *ret = NULL;

   elm_app_name_set("Test name");

   ret = efl_access_i18n_name_get(root);

   ck_assert_str_eq(ret, "Test name");

   efl_unref(root);
   elm_shutdown();
}
END_TEST

START_TEST (test_efl_access_i18n_name_get)
{
   char *args[] = { "exe" };
   elm_init(1, args);
   generate_app();

   const char *name;

   name = efl_access_i18n_name_get(g_btn);

   if (name && name[0]) {
      ck_assert(0);
   }

   // Set name with additional text tags
   elm_object_text_set(g_btn, "Some<br>text");

   name = efl_access_i18n_name_get(g_btn);

   // Accessible name should have cleared tags
   ck_assert(name != NULL);
   ck_assert_str_eq(name, "Some\ntext");

   elm_shutdown();
}
END_TEST

START_TEST (test_efl_access_i18n_name_set)
{
   char *args[] = { "exe" };
   elm_init(1, args);
   generate_app();

   const char *name;

   elm_object_text_set(g_btn, "Other text");
   efl_access_i18n_name_set(g_btn, "Test name");

   name = efl_access_i18n_name_get(g_btn);

   ck_assert(name != NULL);
   ck_assert_str_eq(name, "Test name");

   efl_access_i18n_name_set(g_btn, NULL);
   name = efl_access_i18n_name_get(g_btn);

   ck_assert(name != NULL);
   ck_assert_str_eq(name, "Other text");

   elm_shutdown();
}
END_TEST

START_TEST (test_efl_access_role_get)
{
   char *args[] = { "exe" };
   elm_init(1, args);

   Eo* root = efl_add_ref(ELM_ATSPI_APP_OBJECT_CLASS, NULL);

   Efl_Access_Role role;

   role = efl_access_role_get(root);

   ck_assert(role == EFL_ACCESS_ROLE_APPLICATION);

   efl_unref(root);
   elm_shutdown();
}
END_TEST

START_TEST (test_efl_access_role_set)
{
   char *args[] = { "exe" };
   elm_init(1, args);
   generate_app();
   Efl_Access_Role role;

   efl_access_role_set(g_btn, EFL_ACCESS_ROLE_ACCELERATOR_LABEL);
   role = efl_access_role_get(g_btn);

   if (role != EFL_ACCESS_ROLE_ACCELERATOR_LABEL)
      ck_assert(0);

   efl_access_role_set(g_btn, EFL_ACCESS_ROLE_ENTRY);
   role = efl_access_role_get(g_btn);

   if (role != EFL_ACCESS_ROLE_ENTRY)
      ck_assert(0);

   elm_shutdown();
}
END_TEST

START_TEST (test_efl_access_role_name_get)
{
   char *args[] = { "exe" };
   elm_init(1, args);

   Eo* root = efl_add_ref(ELM_ATSPI_APP_OBJECT_CLASS, NULL);

   const char *ret = NULL;

   ret = efl_access_role_name_get(root);

   ck_assert(ret != NULL);

   efl_unref(root);
   elm_shutdown();
}
END_TEST

START_TEST (test_efl_access_localized_role_name_get)
{
   char *args[] = { "exe" };
   elm_init(1, args);

   Eo* root = efl_add_ref(ELM_ATSPI_APP_OBJECT_CLASS, NULL);

   const char *ret = NULL;

   ret = efl_access_localized_role_name_get(root);

   ck_assert(ret != NULL);

   efl_unref(root);
   elm_shutdown();
}
END_TEST

START_TEST (test_efl_access_description_set)
{
   char *args[] = { "exe" };
   elm_init(1, args);

   Eo* root = efl_add_ref(ELM_ATSPI_APP_OBJECT_CLASS, NULL);

   const char *ret = NULL;

   const char *desc = "Test description";

   ret = efl_access_description_get(root);

   ck_assert(ret == NULL);

   efl_access_description_set(root, desc);
   ret = efl_access_description_get(root);

   ck_assert(ret != NULL);
   ck_assert_str_eq(ret, "Test description");

   efl_access_description_set(root, NULL);
   ret = efl_access_description_get(root);

   ck_assert(ret == NULL);

   efl_unref(root);
   elm_shutdown();
}
END_TEST

/* Test if initial description value is NULL */
START_TEST (test_efl_access_description_get)
{
   char *args[] = { "exe" };
   elm_init(1, args);
   generate_app();

   const char *descr;
   descr = efl_access_description_get(g_bg);

   ck_assert(descr == NULL);

   elm_shutdown();
}
END_TEST

START_TEST (test_efl_access_children_and_parent)
{
   char *args[] = { "exe" };
   elm_init(1, args);

   generate_app();
   Eo* root = efl_add_ref(ELM_ATSPI_APP_OBJECT_CLASS, NULL);

   Eina_List *child_list = NULL;

   child_list = efl_access_children_get(root);

   //bg_child_list = efl_access_children_get(eina_list_nth(child_list, 0));

   ck_assert(eina_list_count(child_list) == 1);

   Eo *win = NULL;

   win = eina_list_nth(child_list, 0);

   ck_assert(win != NULL);
   ck_assert(win == g_win);

   efl_unref(root);
   elm_shutdown();
}
END_TEST

START_TEST (test_efl_access_children_and_parent2)
{
   char *args[] = { "exe" };
   elm_init(1, args);

   generate_app();
   Eo* root = efl_add_ref(ELM_ATSPI_APP_OBJECT_CLASS, NULL);

   Eo *win = NULL;

   Eina_List *root_children;
   root_children = efl_access_children_get(root);
   win = eina_list_nth(root_children, 0);

   Eina_List *win_children;
   win_children = efl_access_children_get(win);

   ck_assert(eina_list_count(win_children) == 1);

   Eo *btn = NULL;

   btn = eina_list_nth(win_children, 0);
   ck_assert(btn != NULL);
   ck_assert(btn == g_btn);

   efl_unref(root);
   elm_shutdown();
}
END_TEST

/* Initial value of translation domain should be NULL */
START_TEST (test_efl_access_translation_domain_get)
{
   char *args[] = { "exe" };
   elm_init(1, args);
   generate_app();

   const char *domain;

   domain = efl_access_translation_domain_get(g_btn);

   ck_assert(domain == NULL);

   elm_shutdown();
}
END_TEST

START_TEST (test_efl_access_translation_domain_set)
{
   char *args[] = { "exe" };
   elm_init(1, args);
   generate_app();

   const char *domain;

   efl_access_translation_domain_set(g_btn, "Test translation_domain");
   domain = efl_access_translation_domain_get(g_btn);

   ck_assert(domain != NULL);
   ck_assert_str_eq(domain, "Test translation_domain");

   efl_access_translation_domain_set(g_btn, NULL);
   domain = efl_access_translation_domain_get(g_btn);

   ck_assert(domain == NULL);

   elm_shutdown();
}
END_TEST

START_TEST (test_efl_access_relationship_append)
{
   char *args[] = { "exe" };
   elm_init(1, args);
   generate_app();

   Efl_Access_Relation_Set set;
   Efl_Access_Relation *rel, *rel_to, *rel_from;
   Eina_List *l;

   efl_access_relationship_append(g_btn, EFL_ACCESS_RELATION_FLOWS_TO, g_bg);
   efl_access_relationship_append(g_btn, EFL_ACCESS_RELATION_FLOWS_FROM, g_win);
   set = efl_access_relation_set_get(g_btn);

   ck_assert(set != NULL);
   ck_assert(eina_list_count(set) >= 2);

   rel_to = rel_from = NULL;
   EINA_LIST_FOREACH(set, l, rel)
   {
      if (rel->type == EFL_ACCESS_RELATION_FLOWS_TO)
        rel_to = rel;
      if (rel->type == EFL_ACCESS_RELATION_FLOWS_FROM)
        rel_from = rel;
   }

   ck_assert(rel_to != NULL);
   ck_assert(eina_list_data_find(rel_to->objects, g_bg) != NULL);

   ck_assert(rel_from != NULL);
   ck_assert(eina_list_data_find(rel_from->objects, g_win) != NULL);

   efl_access_relation_set_free(set);

   /* Check if append do not procude duplicated relations */
   efl_access_relationship_append(g_btn, EFL_ACCESS_RELATION_FLOWS_TO, g_bg);
   efl_access_relationship_append(g_btn, EFL_ACCESS_RELATION_FLOWS_FROM, g_win);
   set = efl_access_relation_set_get(g_btn);

   rel_to = rel_from = NULL;
   EINA_LIST_FOREACH(set, l, rel)
   {
      if (rel->type == EFL_ACCESS_RELATION_FLOWS_TO)
        rel_to = rel;
      if (rel->type == EFL_ACCESS_RELATION_FLOWS_FROM)
        rel_from = rel;
   }

   ck_assert(rel_to != NULL);
   ck_assert(rel_to->objects != NULL);
   rel_to->objects = eina_list_remove(rel_to->objects, g_bg);
   ck_assert(eina_list_data_find(rel_to->objects, g_bg) == NULL);

   ck_assert(rel_from != NULL);
   ck_assert(rel_from->objects != NULL);
   rel_from->objects = eina_list_remove(rel_from->objects, g_win);
   ck_assert(eina_list_data_find(rel_from->objects, g_win) == NULL);

   efl_access_relation_set_free(set);

   elm_shutdown();
}
END_TEST

START_TEST (test_efl_access_relationship_remove)
{
   char *args[] = { "exe" };
   elm_init(1, args);
   generate_app();

   Efl_Access_Relation_Set set;
   Efl_Access_Relation *rel, *rel_to, *rel_from;
   Eina_List *l;

   /* Test if removal of single relationship works */
   efl_access_relationship_append(g_btn, EFL_ACCESS_RELATION_FLOWS_TO, g_bg);
   efl_access_relationship_append(g_btn, EFL_ACCESS_RELATION_FLOWS_FROM, g_win);
   efl_access_relationship_remove(g_btn, EFL_ACCESS_RELATION_FLOWS_TO, g_bg);
   set = efl_access_relation_set_get(g_btn);

   ck_assert(set != NULL);
   ck_assert(eina_list_count(set) >= 1);

   rel_to = rel_from = NULL;
   EINA_LIST_FOREACH(set, l, rel)
   {
      if (rel->type == EFL_ACCESS_RELATION_FLOWS_TO)
        rel_to = rel;
      if (rel->type == EFL_ACCESS_RELATION_FLOWS_FROM)
        rel_from = rel;
   }

   if (rel_to) ck_assert(eina_list_data_find(rel_to->objects, g_bg) == NULL);
   ck_assert(rel_from != NULL);
   ck_assert(eina_list_data_find(rel_from->objects, g_win) != NULL);

   efl_access_relation_set_free(set);

   /* Test if removal of type relationship works */
   efl_access_relationship_append(g_btn, EFL_ACCESS_RELATION_FLOWS_TO, g_bg);
   efl_access_relationship_append(g_btn, EFL_ACCESS_RELATION_FLOWS_TO, g_win);
   efl_access_relationship_remove(g_btn, EFL_ACCESS_RELATION_FLOWS_TO, NULL);
   set = efl_access_relation_set_get(g_btn);

   rel_to = rel_from = NULL;
   EINA_LIST_FOREACH(set, l, rel)
   {
      if (rel->type == EFL_ACCESS_RELATION_FLOWS_TO)
        rel_to = rel;
      if (rel->type == EFL_ACCESS_RELATION_FLOWS_FROM)
        rel_from = rel;
   }

   ck_assert(rel_to == NULL);
   ck_assert(rel_from != NULL);
   ck_assert(eina_list_data_find(rel_from->objects, g_win) != NULL);

   efl_access_relation_set_free(set);

   /* Test if relationship is implicity removed when object is deleted */
   efl_access_relationship_append(g_btn, EFL_ACCESS_RELATION_FLOWS_TO, g_bg);
   efl_access_relationship_append(g_btn, EFL_ACCESS_RELATION_FLOWS_FROM, g_bg);
   efl_del(g_bg);
   set = efl_access_relation_set_get(g_btn);

   rel_to = rel_from = NULL;
   EINA_LIST_FOREACH(set, l, rel)
   {
      if (rel->type == EFL_ACCESS_RELATION_FLOWS_TO)
        rel_to = rel;
      if (rel->type == EFL_ACCESS_RELATION_FLOWS_FROM)
        rel_from = rel;
   }

   if (rel_to) ck_assert(eina_list_data_find(rel_to->objects, g_bg) == NULL);
   if (rel_from) ck_assert(eina_list_data_find(rel_from->objects, g_bg) == NULL);

   efl_access_relation_set_free(set);
   elm_shutdown();
}
END_TEST

START_TEST (test_efl_access_relationships_clear)
{
   Efl_Access_Relation_Set set;
   Efl_Access_Relation *rel;
   Eina_List *l;

   char *args[] = { "exe" };
   elm_init(1, args);
   generate_app();

   efl_access_relationship_append(g_btn, EFL_ACCESS_RELATION_FLOWS_TO, g_bg);
   efl_access_relationship_append(g_btn, EFL_ACCESS_RELATION_FLOWS_FROM, g_bg);
   efl_access_relationship_append(g_btn, EFL_ACCESS_RELATION_FLOWS_TO, g_win);
   efl_access_relationship_append(g_btn, EFL_ACCESS_RELATION_FLOWS_FROM, g_win);

   efl_access_relationships_clear(g_btn);

   set = efl_access_relation_set_get(g_btn);
   EINA_LIST_FOREACH(set, l, rel)
   {
      ck_assert(!((rel->type == EFL_ACCESS_RELATION_FLOWS_TO) && eina_list_data_find(rel->objects, g_bg)));
      ck_assert(!((rel->type == EFL_ACCESS_RELATION_FLOWS_FROM) && eina_list_data_find(rel->objects, g_bg)));
      ck_assert(!((rel->type == EFL_ACCESS_RELATION_FLOWS_TO) && eina_list_data_find(rel->objects, g_win)));
      ck_assert(!((rel->type == EFL_ACCESS_RELATION_FLOWS_FROM) && eina_list_data_find(rel->objects, g_win)));
   }

   efl_access_relation_set_free(set);
   elm_shutdown();
}
END_TEST

void elm_test_atspi(TCase *tc)
{
   tcase_add_test(tc, test_efl_access_app_obj_name_get);
   tcase_add_test(tc, test_efl_access_i18n_name_get);
   tcase_add_test(tc, test_efl_access_i18n_name_set);
   tcase_add_test(tc, test_efl_access_role_get);
   tcase_add_test(tc, test_efl_access_role_set);
   tcase_add_test(tc, test_efl_access_role_name_get);
   tcase_add_test(tc, test_efl_access_localized_role_name_get);
   tcase_add_test(tc, test_efl_access_description_set);
   tcase_add_test(tc, test_efl_access_description_get);
   tcase_add_test(tc, test_efl_access_children_and_parent);
   tcase_add_test(tc, test_efl_access_children_and_parent2);
   tcase_add_test(tc, test_efl_access_translation_domain_get);
   tcase_add_test(tc, test_efl_access_translation_domain_set);
   tcase_add_test(tc, test_efl_access_relationship_append);
   tcase_add_test(tc, test_efl_access_relationship_remove);
   tcase_add_test(tc, test_efl_access_relationships_clear);
}
