#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif


#define EFL_ACCESS_OBJECT_BETA
#define EFL_ACCESS_OBJECT_PROTECTED

#include <Efl_Ui.h>
#include <Elementary.h>
#include "elm_priv.h"
#include "efl_ui_suite.h"

static Evas_Object *g_win, *g_btn, *g_bg;

void generate_app(void)
{
   g_win = win_add(NULL, "Title", ELM_WIN_BASIC);
   efl_gfx_entity_position_set(g_win, EINA_POSITION2D(100, 100));
   efl_gfx_entity_size_set(g_win, EINA_SIZE2D(100, 100));

   g_bg = efl_add(EFL_UI_BG_CLASS, g_win);

   g_btn = efl_add(EFL_UI_BUTTON_CLASS, g_win);

   efl_gfx_entity_visible_set(g_btn, EINA_TRUE);
   efl_gfx_entity_visible_set(g_bg, EINA_TRUE);
   efl_gfx_entity_visible_set(g_win, EINA_TRUE);
}

EFL_START_TEST(test_efl_access_app_obj_name_get)
{
   Eo* root = efl_add_ref(ELM_ATSPI_APP_OBJECT_CLASS, NULL);

   ck_assert(root != NULL);

   const char *ret = NULL;

   elm_app_name_set("Test name");

   ret = efl_access_object_i18n_name_get(root);

   ck_assert_str_eq(ret, "Test name");

   efl_unref(root);
}
EFL_END_TEST

EFL_START_TEST(test_efl_access_object_i18n_name_get)
{
   generate_app();

   const char *name;

   name = efl_access_object_i18n_name_get(g_btn);

   if (name && name[0]) {
      ck_assert(0);
   }

   // Set name with additional text tags
   efl_text_set(g_btn, "Some<br>text");

   name = efl_access_object_i18n_name_get(g_btn);

   // Accessible name should have cleared tags
   ck_assert(name != NULL);
   ck_assert_str_eq(name, "Some\ntext");

}
EFL_END_TEST

EFL_START_TEST(test_efl_access_object_i18n_name_set)
{
   generate_app();

   const char *name;

   efl_text_set(g_btn, "Other text");
   efl_access_object_i18n_name_set(g_btn, "Test name");

   name = efl_access_object_i18n_name_get(g_btn);

   ck_assert(name != NULL);
   ck_assert_str_eq(name, "Test name");

   efl_access_object_i18n_name_set(g_btn, NULL);
   name = efl_access_object_i18n_name_get(g_btn);

   ck_assert(name != NULL);
   ck_assert_str_eq(name, "Other text");

}
EFL_END_TEST

EFL_START_TEST(test_efl_access_object_role_get)
{
   Eo* root = efl_add_ref(ELM_ATSPI_APP_OBJECT_CLASS, NULL);

   Efl_Access_Role role;

   role = efl_access_object_role_get(root);

   ck_assert(role == EFL_ACCESS_ROLE_APPLICATION);

   efl_unref(root);
}
EFL_END_TEST

EFL_START_TEST(test_efl_access_object_role_set)
{
   generate_app();
   Efl_Access_Role role;

   efl_access_object_role_set(g_btn, EFL_ACCESS_ROLE_ACCELERATOR_LABEL);
   role = efl_access_object_role_get(g_btn);

   if (role != EFL_ACCESS_ROLE_ACCELERATOR_LABEL)
      ck_assert(0);

   efl_access_object_role_set(g_btn, EFL_ACCESS_ROLE_ENTRY);
   role = efl_access_object_role_get(g_btn);

   if (role != EFL_ACCESS_ROLE_ENTRY)
      ck_assert(0);

}
EFL_END_TEST

EFL_START_TEST(test_efl_access_object_role_name_get)
{
   Eo* root = efl_add_ref(ELM_ATSPI_APP_OBJECT_CLASS, NULL);

   const char *ret = NULL;

   ret = efl_access_object_role_name_get(root);

   ck_assert(ret != NULL);

   efl_unref(root);
}
EFL_END_TEST

EFL_START_TEST(test_efl_access_object_localized_role_name_get)
{
   Eo* root = efl_add_ref(ELM_ATSPI_APP_OBJECT_CLASS, NULL);

   const char *ret = NULL;

   ret = efl_access_object_localized_role_name_get(root);

   ck_assert(ret != NULL);

   efl_unref(root);
}
EFL_END_TEST

EFL_START_TEST(test_efl_access_object_description_set)
{
   Eo* root = efl_add_ref(ELM_ATSPI_APP_OBJECT_CLASS, NULL);

   const char *ret = NULL;

   const char *desc = "Test description";

   ret = efl_access_object_description_get(root);

   ck_assert(ret == NULL);

   efl_access_object_description_set(root, desc);
   ret = efl_access_object_description_get(root);

   ck_assert(ret != NULL);
   ck_assert_str_eq(ret, "Test description");

   efl_access_object_description_set(root, NULL);
   ret = efl_access_object_description_get(root);

   ck_assert(ret == NULL);

   efl_unref(root);
}
EFL_END_TEST

/* Test if initial description value is NULL */
EFL_START_TEST(test_efl_access_object_description_get)
{
   generate_app();

   const char *descr;
   descr = efl_access_object_description_get(g_bg);

   ck_assert(descr == NULL);

}
EFL_END_TEST

EFL_START_TEST(test_efl_access_children_and_parent)
{
   generate_app();
   Eo* root = efl_add_ref(ELM_ATSPI_APP_OBJECT_CLASS, NULL);

   Eina_List *child_list = NULL;

   child_list = efl_access_object_access_children_get(root);

   //bg_child_list = efl_access_object_access_children_get(eina_list_nth(child_list, 0));

   ck_assert_int_eq(eina_list_count(child_list), 1);

   Eo *win = NULL;

   win = eina_list_nth(child_list, 0);

   ck_assert_ptr_ne(win, NULL);
   ck_assert_ptr_eq(win, g_win);

   efl_unref(root);
}
EFL_END_TEST

EFL_START_TEST(test_efl_access_children_and_parent2)
{
   generate_app();
   Eo* root = efl_add_ref(ELM_ATSPI_APP_OBJECT_CLASS, NULL);

   Eo *win = NULL;

   Eina_List *root_children;
   root_children = efl_access_object_access_children_get(root);
   win = eina_list_nth(root_children, 0);

   Eina_List *win_children;
   win_children = efl_access_object_access_children_get(win);

   ck_assert(eina_list_count(win_children) == 1);

   Eo *btn = NULL;

   btn = eina_list_nth(win_children, 0);
   ck_assert(btn != NULL);
   ck_assert(btn == g_btn);

   efl_unref(root);
}
EFL_END_TEST

/* Initial value of translation domain should be NULL */
EFL_START_TEST(test_efl_access_object_translation_domain_get)
{
   generate_app();

   const char *domain;

   domain = efl_access_object_translation_domain_get(g_btn);

   ck_assert(domain == NULL);

}
EFL_END_TEST

EFL_START_TEST(test_efl_access_object_translation_domain_set)
{
   generate_app();

   const char *domain;

   efl_access_object_translation_domain_set(g_btn, "Test translation_domain");
   domain = efl_access_object_translation_domain_get(g_btn);

   ck_assert(domain != NULL);
   ck_assert_str_eq(domain, "Test translation_domain");

   efl_access_object_translation_domain_set(g_btn, NULL);
   domain = efl_access_object_translation_domain_get(g_btn);

   ck_assert(domain == NULL);

}
EFL_END_TEST

EFL_START_TEST(test_efl_access_object_relationship_append)
{
   generate_app();

   Efl_Access_Relation *rel, *rel_to, *rel_from;
   Eina_Iterator *it;
   unsigned int i = 0;

   efl_access_object_relationship_append(g_btn, EFL_ACCESS_RELATION_TYPE_FLOWS_TO, g_bg);
   efl_access_object_relationship_append(g_btn, EFL_ACCESS_RELATION_TYPE_FLOWS_FROM, g_win);
   it = efl_access_object_relations_get(g_btn);

   ck_assert(it != NULL);

   rel_to = rel_from = NULL;
   EINA_ITERATOR_FOREACH(it, rel)
   {
      i++;

      if (rel->type == EFL_ACCESS_RELATION_TYPE_FLOWS_TO)
        rel_to = rel;
      if (rel->type == EFL_ACCESS_RELATION_TYPE_FLOWS_FROM)
        rel_from = rel;
   }

   ck_assert(i >= 2);
   ck_assert(rel_to != NULL);
   ck_assert(eina_list_data_find(rel_to->objects, g_bg) != NULL);

   ck_assert(rel_from != NULL);
   ck_assert(eina_list_data_find(rel_from->objects, g_win) != NULL);

   eina_iterator_free(it);

   /* Check if append do not procude duplicated relations */
   efl_access_object_relationship_append(g_btn, EFL_ACCESS_RELATION_TYPE_FLOWS_TO, g_bg);
   efl_access_object_relationship_append(g_btn, EFL_ACCESS_RELATION_TYPE_FLOWS_FROM, g_win);
   it = efl_access_object_relations_get(g_btn); i = 0;

   rel_to = rel_from = NULL;
   EINA_ITERATOR_FOREACH(it, rel)
   {
      i++;

      if (rel->type == EFL_ACCESS_RELATION_TYPE_FLOWS_TO)
        rel_to = rel;
      if (rel->type == EFL_ACCESS_RELATION_TYPE_FLOWS_FROM)
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

   eina_iterator_free(it);
}
EFL_END_TEST

EFL_START_TEST(test_efl_access_object_relationship_remove)
{
   generate_app();

   Efl_Access_Relation *rel, *rel_to, *rel_from;
   Eina_Iterator *it;
   unsigned int i = 0;

   /* Test if removal of single relationship works */
   efl_access_object_relationship_append(g_btn, EFL_ACCESS_RELATION_TYPE_FLOWS_TO, g_bg);
   efl_access_object_relationship_append(g_btn, EFL_ACCESS_RELATION_TYPE_FLOWS_FROM, g_win);
   efl_access_object_relationship_remove(g_btn, EFL_ACCESS_RELATION_TYPE_FLOWS_TO, g_bg);
   it = efl_access_object_relations_get(g_btn);

   ck_assert(it != NULL);

   rel_to = rel_from = NULL;
   EINA_ITERATOR_FOREACH(it, rel)
   {
      i++;

      if (rel->type == EFL_ACCESS_RELATION_TYPE_FLOWS_TO)
        rel_to = rel;
      if (rel->type == EFL_ACCESS_RELATION_TYPE_FLOWS_FROM)
        rel_from = rel;
   }

   ck_assert(i >= 1);

   if (rel_to) ck_assert(eina_list_data_find(rel_to->objects, g_bg) == NULL);
   ck_assert(rel_from != NULL);
   ck_assert(eina_list_data_find(rel_from->objects, g_win) != NULL);

   eina_iterator_free(it);

   /* Test if removal of type relationship works */
   efl_access_object_relationship_append(g_btn, EFL_ACCESS_RELATION_TYPE_FLOWS_TO, g_bg);
   efl_access_object_relationship_append(g_btn, EFL_ACCESS_RELATION_TYPE_FLOWS_TO, g_win);
   efl_access_object_relationship_remove(g_btn, EFL_ACCESS_RELATION_TYPE_FLOWS_TO, NULL);
   it = efl_access_object_relations_get(g_btn); i = 0;

   rel_to = rel_from = NULL;
   EINA_ITERATOR_FOREACH(it, rel)
   {
      i++;

      if (rel->type == EFL_ACCESS_RELATION_TYPE_FLOWS_TO)
        rel_to = rel;
      if (rel->type == EFL_ACCESS_RELATION_TYPE_FLOWS_FROM)
        rel_from = rel;
   }

   ck_assert(rel_to == NULL);
   ck_assert(rel_from != NULL);
   ck_assert(eina_list_data_find(rel_from->objects, g_win) != NULL);

   eina_iterator_free(it);

   /* Test if relationship is implicity removed when object is deleted */
   efl_access_object_relationship_append(g_btn, EFL_ACCESS_RELATION_TYPE_FLOWS_TO, g_bg);
   efl_access_object_relationship_append(g_btn, EFL_ACCESS_RELATION_TYPE_FLOWS_FROM, g_bg);
   efl_del(g_bg);
   it = efl_access_object_relations_get(g_btn); i = 0;

   rel_to = rel_from = NULL;
   EINA_ITERATOR_FOREACH(it, rel)
   {
      i++;

      if (rel->type == EFL_ACCESS_RELATION_TYPE_FLOWS_TO)
        rel_to = rel;
      if (rel->type == EFL_ACCESS_RELATION_TYPE_FLOWS_FROM)
        rel_from = rel;
   }

   if (rel_to) ck_assert(eina_list_data_find(rel_to->objects, g_bg) == NULL);
   if (rel_from) ck_assert(eina_list_data_find(rel_from->objects, g_bg) == NULL);

   eina_iterator_free(it);
}
EFL_END_TEST

EFL_START_TEST(test_efl_access_object_relationships_clear)
{
   Efl_Access_Relation *rel;
   Eina_Iterator *it;

   generate_app();

   efl_access_object_relationship_append(g_btn, EFL_ACCESS_RELATION_TYPE_FLOWS_TO, g_bg);
   efl_access_object_relationship_append(g_btn, EFL_ACCESS_RELATION_TYPE_FLOWS_FROM, g_bg);
   efl_access_object_relationship_append(g_btn, EFL_ACCESS_RELATION_TYPE_FLOWS_TO, g_win);
   efl_access_object_relationship_append(g_btn, EFL_ACCESS_RELATION_TYPE_FLOWS_FROM, g_win);

   efl_access_object_relationships_clear(g_btn);

   it = efl_access_object_relations_get(g_btn);
   EINA_ITERATOR_FOREACH(it, rel)
   {
      ck_assert(!((rel->type == EFL_ACCESS_RELATION_TYPE_FLOWS_TO) && eina_list_data_find(rel->objects, g_bg)));
      ck_assert(!((rel->type == EFL_ACCESS_RELATION_TYPE_FLOWS_FROM) && eina_list_data_find(rel->objects, g_bg)));
      ck_assert(!((rel->type == EFL_ACCESS_RELATION_TYPE_FLOWS_TO) && eina_list_data_find(rel->objects, g_win)));
      ck_assert(!((rel->type == EFL_ACCESS_RELATION_TYPE_FLOWS_FROM) && eina_list_data_find(rel->objects, g_win)));
   }
   eina_iterator_free(it);
}
EFL_END_TEST

EFL_START_TEST(test_efl_access_object_attribute_append)
{
   Eina_List *attr_list = NULL, *l = NULL;
   Efl_Access_Attribute *attr = NULL;
   generate_app();
   efl_access_object_attribute_append(g_btn, "color1", "red");
   efl_access_object_attribute_append(g_btn, "color2", "blue");
   efl_access_object_attribute_append(g_btn, "color3", "green");
   attr_list = efl_access_object_attributes_get(g_btn);

   ck_assert(attr_list != NULL);
   EINA_LIST_FOREACH(attr_list, l, attr)
     {
        if (!strcmp(attr->key, "color1"))
          ck_assert_str_eq(attr->value, "red");
        else if (!strcmp(attr->key, "color2"))
          ck_assert_str_eq(attr->value, "blue");
        else if (!strcmp(attr->key, "color3"))
          ck_assert_str_eq(attr->value, "green");
     }
   EINA_LIST_FREE(attr_list, attr)
     {
        eina_stringshare_del(attr->key);
        eina_stringshare_del(attr->value);
        free(attr);
     }
}
EFL_END_TEST

EFL_START_TEST(test_efl_access_object_attributes_get)
{
   Eina_List *attr_list = NULL, *l = NULL;
   Efl_Access_Attribute *attr = NULL;
   generate_app();
   efl_access_object_attribute_append(g_btn, "color1", "red");
   efl_access_object_attribute_append(g_btn, "color2", "blue");
   efl_access_object_attribute_append(g_btn, "color3", "green");
   attr_list = efl_access_object_attributes_get(g_btn);

   ck_assert(attr_list != NULL);
   EINA_LIST_FOREACH(attr_list, l, attr)
     {
        if (!strcmp(attr->key, "color1"))
          ck_assert_str_eq(attr->value, "red");
        else if (!strcmp(attr->key, "color2"))
          ck_assert_str_eq(attr->value, "blue");
        else if (!strcmp(attr->key, "color3"))
          ck_assert_str_eq(attr->value, "green");
     }
   EINA_LIST_FREE(attr_list, attr)
     {
        eina_stringshare_del(attr->key);
        eina_stringshare_del(attr->value);
        free(attr);
     }
}
EFL_END_TEST

EFL_START_TEST(test_efl_access_object_attribute_del)
{
   Eina_List *attr_list = NULL;
   Efl_Access_Attribute *attr = NULL;
   int count1 = 0;
   int count2 = 0;
   generate_app();
   efl_access_object_attribute_append(g_btn, "color1", "red");
   efl_access_object_attribute_append(g_btn, "color2", "blue");
   efl_access_object_attribute_append(g_btn, "color3", "green");
   attr_list = efl_access_object_attributes_get(g_btn);//default attributes are added again
   ck_assert(attr_list != NULL);
   count1 = eina_list_count(attr_list);
   EINA_LIST_FREE(attr_list, attr)
     {
        eina_stringshare_del(attr->key);
        eina_stringshare_del(attr->value);
        free(attr);
     }
   attr_list = NULL;
   efl_access_object_attribute_del(g_btn, "color4");//non existent key deletion
   efl_access_object_attribute_del(g_btn, "color3");//existing key deletion
   attr_list = efl_access_object_attributes_get(g_btn);
   ck_assert(attr_list != NULL);
   count2 = eina_list_count(attr_list);
   ck_assert(count1 == (count2+1));
   EINA_LIST_FREE(attr_list, attr)
     {
        eina_stringshare_del(attr->key);
        eina_stringshare_del(attr->value);
        free(attr);
     }
}
EFL_END_TEST

EFL_START_TEST(test_efl_access_object_attributes_clear)
{
   Eina_List *attr_list = NULL;
   Efl_Access_Attribute *attr = NULL;
   generate_app();
   efl_access_object_attribute_append(g_btn, "color1", "red");
   efl_access_object_attribute_append(g_btn, "color2", "blue");
   efl_access_object_attribute_append(g_btn, "color3", "green");
   efl_access_object_attributes_clear(g_btn);
   attr_list = efl_access_object_attributes_get(g_btn);//default attributes are added again
   ck_assert(attr_list != NULL);
   ck_assert(eina_list_count(attr_list) <= 2);
   EINA_LIST_FREE(attr_list, attr)
     {
        eina_stringshare_del(attr->key);
        eina_stringshare_del(attr->value);
        free(attr);
     }
}
EFL_END_TEST

void efl_ui_test_atspi(TCase *tc)
{
   tcase_add_test(tc, test_efl_access_app_obj_name_get);
   tcase_add_test(tc, test_efl_access_object_i18n_name_get);
   tcase_add_test(tc, test_efl_access_object_i18n_name_set);
   tcase_add_test(tc, test_efl_access_object_role_get);
   tcase_add_test(tc, test_efl_access_object_role_set);
   tcase_add_test(tc, test_efl_access_object_role_name_get);
   tcase_add_test(tc, test_efl_access_object_localized_role_name_get);
   tcase_add_test(tc, test_efl_access_object_description_set);
   tcase_add_test(tc, test_efl_access_object_description_get);
   tcase_add_test(tc, test_efl_access_children_and_parent);
   tcase_add_test(tc, test_efl_access_children_and_parent2);
   tcase_add_test(tc, test_efl_access_object_translation_domain_get);
   tcase_add_test(tc, test_efl_access_object_translation_domain_set);
   tcase_add_test(tc, test_efl_access_object_relationship_append);
   tcase_add_test(tc, test_efl_access_object_relationship_remove);
   tcase_add_test(tc, test_efl_access_object_relationships_clear);
   tcase_add_test(tc, test_efl_access_object_attribute_append);
   tcase_add_test(tc, test_efl_access_object_attributes_get);
   tcase_add_test(tc, test_efl_access_object_attribute_del);
   tcase_add_test(tc, test_efl_access_object_attributes_clear);
}
