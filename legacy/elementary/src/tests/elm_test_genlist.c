#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define ELM_INTERFACE_ATSPI_ACCESSIBLE_PROTECTED
#include <Elementary.h>
#include "elm_suite.h"

static Evas_Object *win, *genlist;
static Elm_Gen_Item_Class itc;

void test_init(void)
{
   elm_init(1, NULL);
   win = elm_win_add(NULL, "genlist", ELM_WIN_BASIC);
   genlist = elm_genlist_add(win);
}

START_TEST (elm_atspi_role_get)
{
   test_init();

   Elm_Atspi_Role role;

   eo_do(genlist, role = elm_interface_atspi_accessible_role_get());

   ck_assert(role == ELM_ATSPI_ROLE_LIST);

   elm_shutdown();
}
END_TEST

START_TEST(elm_atspi_children_get1)
{
   test_init();
   Eina_List *children;
   Elm_Object_Item *it[3];

   eo_do(genlist, children = elm_interface_atspi_accessible_children_get());
   ck_assert(children == NULL);

   it[0] = elm_genlist_item_append(genlist, &itc, NULL, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
   it[1] = elm_genlist_item_append(genlist, &itc, NULL, NULL, ELM_GENLIST_ITEM_GROUP, NULL, NULL);
   it[2] = elm_genlist_item_append(genlist, &itc, NULL, NULL, ELM_GENLIST_ITEM_TREE, NULL, NULL);

   eo_do(genlist, children = elm_interface_atspi_accessible_children_get());
   ck_assert(eina_list_count(children) == 3);
   ck_assert(eina_list_nth(children, 0) == it[0]);
   ck_assert(eina_list_nth(children, 1) == it[1]);
   ck_assert(eina_list_nth(children, 2) == it[2]);

   eina_list_free(children);

   elm_genlist_item_append(genlist, &itc, NULL, it[0], ELM_GENLIST_ITEM_NONE, NULL, NULL);
   elm_genlist_item_append(genlist, &itc, NULL, it[1], ELM_GENLIST_ITEM_NONE, NULL, NULL);
   elm_genlist_item_append(genlist, &itc, NULL, it[2], ELM_GENLIST_ITEM_NONE, NULL, NULL);

   eo_do(genlist, children = elm_interface_atspi_accessible_children_get());
   ck_assert(eina_list_count(children) == 3);
   ck_assert(eina_list_nth(children, 0) == it[0]);
   ck_assert(eina_list_nth(children, 1) == it[1]);
   ck_assert(eina_list_nth(children, 2) == it[2]);

   elm_shutdown();
}
END_TEST

START_TEST(elm_atspi_children_get2)
{
   test_init();
   Eina_List *children;
   Elm_Object_Item *it[3];

   it[0] = elm_genlist_item_append(genlist, &itc, NULL, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
   it[1] = elm_genlist_item_prepend(genlist, &itc, NULL, NULL, ELM_GENLIST_ITEM_GROUP, NULL, NULL);
   it[2] = elm_genlist_item_append(genlist, &itc, NULL, NULL, ELM_GENLIST_ITEM_TREE, NULL, NULL);

   eo_do(genlist, children = elm_interface_atspi_accessible_children_get());
   ck_assert(eina_list_nth(children, 1) == it[0]);
   ck_assert(eina_list_nth(children, 0) == it[1]);
   ck_assert(eina_list_nth(children, 2) == it[2]);

   elm_shutdown();
}
END_TEST

void elm_test_genlist(TCase *tc)
{
   tcase_add_test(tc, elm_atspi_role_get);
   tcase_add_test(tc, elm_atspi_children_get1);
   tcase_add_test(tc, elm_atspi_children_get2);
}
