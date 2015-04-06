#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define ELM_INTERFACE_ATSPI_ACCESSIBLE_PROTECTED
#include <Elementary.h>
#include "elm_suite.h"

static Evas_Object *win, *genlist;
static Elm_Gen_Item_Class itc;
static Eo *current;
static int counter;
static Elm_Atspi_Event_Children_Changed_Data ev_data;

void test_init(void)
{
   elm_init(1, NULL);
   elm_config_atspi_mode_set(EINA_TRUE);
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
   it[1] = elm_genlist_item_append(genlist, &itc, NULL, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
   it[2] = elm_genlist_item_append(genlist, &itc, NULL, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);

   eo_do(genlist, children = elm_interface_atspi_accessible_children_get());
   ck_assert(eina_list_count(children) == 3);
   ck_assert(eina_list_nth(children, 0) == it[0]);
   ck_assert(eina_list_nth(children, 1) == it[1]);
   ck_assert(eina_list_nth(children, 2) == it[2]);

   eina_list_free(children);

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

static Eina_Bool
_children_changed_cb(void *data EINA_UNUSED, Eo *obj EINA_UNUSED,
                     const Eo_Event_Description *desc EINA_UNUSED, void *event_info EINA_UNUSED)
{
   ev_data = *(Elm_Atspi_Event_Children_Changed_Data*)event_info;
   current = obj;
   counter++;

   return EINA_TRUE;
}

START_TEST(elm_atspi_children_events_add)
{
   test_init();

   current = NULL;
   counter = 0;

   Elm_Object_Item *it[3];

   eo_do(genlist, eo_event_callback_add(ELM_INTERFACE_ATSPI_ACCESSIBLE_EVENT_CHILDREN_CHANGED, _children_changed_cb, NULL));

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
END_TEST

START_TEST(elm_atspi_children_events_del1)
{
   test_init();

   current = NULL;
   counter = 0;

   Elm_Object_Item *it[3];

   it[0] = elm_genlist_item_append(genlist, &itc, NULL, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
   it[1] = elm_genlist_item_prepend(genlist, &itc, NULL, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
   it[2] = elm_genlist_item_append(genlist, &itc, NULL, NULL, ELM_GENLIST_ITEM_TREE, NULL, NULL);

   eo_do(genlist, eo_event_callback_add(ELM_INTERFACE_ATSPI_ACCESSIBLE_EVENT_CHILDREN_CHANGED, _children_changed_cb, NULL));

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
END_TEST

START_TEST(elm_atspi_children_events_del2)
{
   test_init();

   Elm_Object_Item *it;
   current = NULL;
   counter = 0;

   it = elm_genlist_item_append(genlist, &itc, NULL, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);

   eo_do(genlist, eo_event_callback_add(ELM_INTERFACE_ATSPI_ACCESSIBLE_EVENT_CHILDREN_CHANGED, _children_changed_cb, NULL));
   elm_genlist_clear(genlist);

   ck_assert(genlist == current);
   ck_assert(counter == 1);
   ck_assert(ev_data.is_added == EINA_FALSE);
   ck_assert(ev_data.child == it);

   elm_shutdown();
}
END_TEST

void elm_test_genlist(TCase *tc)
{
   tcase_add_test(tc, elm_atspi_role_get);
   tcase_add_test(tc, elm_atspi_children_get1);
   tcase_add_test(tc, elm_atspi_children_get2);
   tcase_add_test(tc, elm_atspi_children_events_add);
   tcase_add_test(tc, elm_atspi_children_events_del1);
   tcase_add_test(tc, elm_atspi_children_events_del2);
}
