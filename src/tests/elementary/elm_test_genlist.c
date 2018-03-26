#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_BETA
#define EFL_ACCESS_PROTECTED
#include <Elementary.h>
#include "elm_suite.h"

static Evas_Object *win, *genlist;
static Elm_Gen_Item_Class itc;
static Eo *current;
static int counter;
static Efl_Access_Event_Children_Changed_Data ev_data;
Evas_Object *content;

static void
verify_group_api(Elm_Object_Item *git)
{
   Elm_Object_Item *it;
   int i;

   /* the last item is the group item
    * this is not consistent with the visual layout but this is consistent with behavior since 1.0
    */
   it = elm_genlist_last_item_get(genlist);
   ck_assert_ptr_ne(it, NULL);
   ck_assert_ptr_eq(it, git);

   /* assert that the last item is actually the last item */
   it = elm_genlist_item_next_get(git);
   ck_assert_ptr_eq(it, NULL);

   /* assert that the other items added to the list exist */
   it = elm_genlist_item_prev_get(git);
   ck_assert_ptr_ne(it, NULL);

   /* assert that the parent of this item is the group item */
   ck_assert_ptr_eq(elm_genlist_item_parent_get(it), git);

   /* check that the first item in the list is a normal item */
   it = elm_genlist_first_item_get(genlist);
   ck_assert_ptr_eq(elm_genlist_item_parent_get(it), git);

   /* verify list consistency */
   for (i = 0; i < 9; i++)
     {
        it = elm_genlist_item_next_get(it);
        ck_assert_ptr_eq(elm_genlist_item_parent_get(it), git);
     }
   it = elm_genlist_item_next_get(it);
   /* verify once again that we have arrived at the group item */
   ck_assert_ptr_eq(it, git);
}

EFL_START_TEST (elm_genlist_group)
{
   const char *type;
   Elm_Object_Item *git;
   int i;

   char *args[] = { "exe" };
   elm_init(1, args);
   win = elm_win_add(NULL, "genlist", ELM_WIN_BASIC);

   genlist = elm_genlist_add(win);

   git = elm_genlist_item_append(genlist, &itc, NULL, NULL, ELM_GENLIST_ITEM_GROUP, NULL, NULL);
   for (i = 0; i < 10; i++)
     elm_genlist_item_append(genlist, &itc, NULL, git, 0, NULL, NULL);

   verify_group_api(git);

   elm_shutdown();
}
EFL_END_TEST

EFL_START_TEST (elm_genlist_legacy_type_check)
{
   const char *type;

   char *args[] = { "exe" };
   elm_init(1, args);
   win = elm_win_add(NULL, "genlist", ELM_WIN_BASIC);

   genlist = elm_genlist_add(win);

   type = elm_object_widget_type_get(genlist);
   ck_assert(type != NULL);
   ck_assert(!strcmp(type, "Elm_Genlist"));

   type = evas_object_type_get(genlist);
   ck_assert(type != NULL);
   ck_assert(!strcmp(type, "elm_genlist"));

   elm_shutdown();
}
EFL_END_TEST

void test_init(void)
{
   char *args[] = { "exe" };
   elm_init(1, args);
   elm_config_atspi_mode_set(EINA_TRUE);
   win = elm_win_add(NULL, "genlist", ELM_WIN_BASIC);
   genlist = elm_genlist_add(win);
}

EFL_START_TEST (elm_atspi_role_get)
{
   test_init();

   Efl_Access_Role role;

   role = efl_access_role_get(genlist);

   ck_assert(role == EFL_ACCESS_ROLE_LIST);

   elm_shutdown();
}
EFL_END_TEST

EFL_START_TEST(elm_atspi_children_get1)
{
   test_init();
   Eina_List *children;
   Elm_Object_Item *it[3];

   children = efl_access_children_get(genlist);
   ck_assert(children == NULL);

   it[0] = elm_genlist_item_append(genlist, &itc, NULL, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
   it[1] = elm_genlist_item_append(genlist, &itc, NULL, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
   it[2] = elm_genlist_item_append(genlist, &itc, NULL, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);

   children = efl_access_children_get(genlist);
   ck_assert(eina_list_count(children) == 3);
   ck_assert(eina_list_nth(children, 0) == it[0]);
   ck_assert(eina_list_nth(children, 1) == it[1]);
   ck_assert(eina_list_nth(children, 2) == it[2]);

   eina_list_free(children);

   elm_shutdown();
}
EFL_END_TEST

EFL_START_TEST(elm_atspi_children_get2)
{
   test_init();
   Eina_List *children;
   Elm_Object_Item *it[3];

   it[0] = elm_genlist_item_append(genlist, &itc, NULL, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
   it[1] = elm_genlist_item_prepend(genlist, &itc, NULL, NULL, ELM_GENLIST_ITEM_GROUP, NULL, NULL);
   it[2] = elm_genlist_item_append(genlist, &itc, NULL, NULL, ELM_GENLIST_ITEM_TREE, NULL, NULL);

   children = efl_access_children_get(genlist);
   ck_assert(eina_list_nth(children, 1) == it[0]);
   ck_assert(eina_list_nth(children, 0) == it[1]);
   ck_assert(eina_list_nth(children, 2) == it[2]);

   elm_shutdown();
}
EFL_END_TEST

static void
_children_changed_cb(void *data EINA_UNUSED, const Efl_Event *event)
{
   if (event->desc != EFL_ACCESS_EVENT_CHILDREN_CHANGED)
     return;

   ev_data = *(Efl_Access_Event_Children_Changed_Data*)event->info;
   current = event->object;
   counter++;
}

EFL_START_TEST(elm_atspi_children_events_add)
{
   test_init();

   current = NULL;
   counter = 0;

   Elm_Object_Item *it[3];

   efl_access_event_handler_add(EFL_ACCESS_MIXIN, _children_changed_cb, NULL);

   it[0] = elm_genlist_item_append(genlist, &itc, NULL, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
   ck_assert(genlist == current);
   ck_assert(counter == 1);
   ck_assert(ev_data.is_added == EINA_TRUE);
   ck_assert(ev_data.child == it[0]);

   it[1] = elm_genlist_item_prepend(genlist, &itc, it[0], NULL, ELM_GENLIST_ITEM_GROUP, NULL, NULL);
   ck_assert(genlist == current);
   ck_assert(counter == 2);
   ck_assert(ev_data.is_added == EINA_TRUE);
   ck_assert(ev_data.child == it[1]);

   it[2] = elm_genlist_item_append(genlist, &itc, NULL, NULL, ELM_GENLIST_ITEM_TREE, NULL, NULL);
   ck_assert(genlist == current);
   ck_assert(counter == 3);
   ck_assert(ev_data.is_added == EINA_TRUE);
   ck_assert(ev_data.child == it[2]);

   elm_shutdown();
}
EFL_END_TEST

EFL_START_TEST(elm_atspi_children_events_del1)
{
   test_init();

   current = NULL;
   counter = 0;

   Elm_Object_Item *it[3];

   it[0] = elm_genlist_item_append(genlist, &itc, NULL, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
   it[1] = elm_genlist_item_prepend(genlist, &itc, NULL, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
   it[2] = elm_genlist_item_append(genlist, &itc, NULL, NULL, ELM_GENLIST_ITEM_TREE, NULL, NULL);

   efl_access_event_handler_add(EFL_ACCESS_MIXIN, _children_changed_cb, NULL);

   elm_object_item_del(it[0]);
   ck_assert(genlist == current);
   ck_assert(counter == 1);
   ck_assert(ev_data.is_added == EINA_FALSE);
   ck_assert(ev_data.child == it[0]);

   elm_object_item_del(it[2]);
   ck_assert(genlist == current);
   ck_assert(counter == 2);
   ck_assert(ev_data.is_added == EINA_FALSE);
   ck_assert(ev_data.child == it[2]);

   elm_shutdown();
}
EFL_END_TEST

EFL_START_TEST(elm_atspi_children_events_del2)
{
   test_init();

   Elm_Object_Item *it;
   current = NULL;
   counter = 0;

   it = elm_genlist_item_append(genlist, &itc, NULL, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);

   efl_access_event_handler_add(EFL_ACCESS_MIXIN, _children_changed_cb, NULL);
   elm_genlist_clear(genlist);

   ck_assert(genlist == current);
   ck_assert(counter == 1);
   ck_assert(ev_data.is_added == EINA_FALSE);
   ck_assert(ev_data.child == it);

   elm_shutdown();
}
EFL_END_TEST

void elm_test_genlist(TCase *tc)
{
   tcase_add_test(tc, elm_genlist_legacy_type_check);
   tcase_add_test(tc, elm_genlist_group);
   tcase_add_test(tc, elm_atspi_role_get);
   tcase_add_test(tc, elm_atspi_children_get1);
   tcase_add_test(tc, elm_atspi_children_get2);
   tcase_add_test(tc, elm_atspi_children_events_add);
   tcase_add_test(tc, elm_atspi_children_events_del1);
   tcase_add_test(tc, elm_atspi_children_events_del2);
}
