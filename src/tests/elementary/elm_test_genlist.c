#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_OBJECT_BETA
#define EFL_ACCESS_OBJECT_PROTECTED
#include <Elementary.h>
#include "elm_suite.h"

static Evas_Object *win, *genlist;
static Elm_Gen_Item_Class itc;
static Eo *current;
static int counter;
static Efl_Access_Event_Children_Changed_Data ev_data;
Evas_Object *content;

static void
verify_item_iteration_api(Elm_Object_Item *parent)
{
   Elm_Object_Item *it, *first_child;
   Eina_List *children, *l;
   int i;

   /* verify first_item_get() */
   it = elm_genlist_first_item_get(genlist);
   ck_assert_ptr_eq(it, parent);
   ck_assert_ptr_eq(elm_object_item_data_get(it), NULL);
   ck_assert_ptr_eq(elm_genlist_item_prev_get(it), NULL);

   /* verify last_item_get() */
   it = elm_genlist_last_item_get(genlist);
   ck_assert_ptr_eq(elm_object_item_data_get(it), (void*)(uintptr_t)10);
   ck_assert_ptr_eq(elm_genlist_item_next_get(it), NULL);

   /* verify next item of parent item is it's first child */
   first_child = elm_genlist_item_next_get(parent);
   ck_assert_ptr_eq(elm_object_item_data_get(first_child), (void*)(uintptr_t)1);
   ck_assert_ptr_eq(elm_genlist_item_parent_get(first_child), parent);

   /* verify subitems_count() */
   ck_assert_int_eq(elm_genlist_item_subitems_count(parent), 10);
   ck_assert_int_eq(elm_genlist_item_subitems_count(first_child), 0);

   /* verify list consistency */
   it = first_child;
   for (i = 1; i <= 9; i++)
     {
        ck_assert_ptr_eq(elm_object_item_data_get(it), (void*)(uintptr_t)i);
        ck_assert_ptr_eq(elm_genlist_item_parent_get(it), parent);
        it = elm_genlist_item_next_get(it);
     }

   /* verify children list */
   i = 1;
   children = (Eina_List *)elm_genlist_item_subitems_get(parent);
   EINA_LIST_FOREACH(children, l, it)
     {
        ck_assert_ptr_eq(elm_object_item_data_get(it), (void*)(uintptr_t)i);
        ck_assert_ptr_eq(elm_genlist_item_parent_get(it), parent);
        i++;
     }

   /* verify item_expanded_depth_get() */
   ck_assert_int_eq(elm_genlist_item_expanded_depth_get(parent), 0);
   if (elm_genlist_item_type_get(parent) == ELM_GENLIST_ITEM_GROUP)
     ck_assert_int_eq(elm_genlist_item_expanded_depth_get(first_child), 0);
   else if (elm_genlist_item_type_get(parent) == ELM_GENLIST_ITEM_TREE)
     ck_assert_int_eq(elm_genlist_item_expanded_depth_get(first_child), 1);

   /* verify nth_item_get() and item_index_get() */
   for (i = 0; i <= 11; i++) // also test the not existant item 11
     {
        it = elm_genlist_nth_item_get(genlist, i);
        if (i == 11)
          // item #11 do not exists
          ck_assert_int_eq(elm_genlist_item_index_get(it), -1);
        else
          ck_assert_int_eq(elm_genlist_item_index_get(it), i + 1);

        if ((i == 0) || (i == 11))
          // test first and item #11 (that do not exists)
          ck_assert_ptr_eq(elm_object_item_data_get(it), NULL);
        else
          ck_assert_ptr_eq(elm_object_item_data_get(it), (void*)(uintptr_t)i);
     }
}

EFL_START_TEST (elm_genlist_item_iteration)
{
   Elm_Object_Item *parent;
   int i;

   win = win_add(NULL, "genlist", ELM_WIN_BASIC);

   genlist = elm_genlist_add(win);

   // perform test using a GROUP item
   parent = elm_genlist_item_append(genlist, &itc, NULL, NULL,
                                    ELM_GENLIST_ITEM_GROUP, NULL, NULL);
   for (i = 1; i <= 10; i++)
     elm_genlist_item_append(genlist, &itc, (void*)(uintptr_t)i, parent,
                             0, NULL, NULL);

   verify_item_iteration_api(parent);

   // repeat same test with a TREE item
   elm_genlist_clear(genlist);
   parent = elm_genlist_item_append(genlist, &itc, NULL, NULL,
                                    ELM_GENLIST_ITEM_TREE, NULL, NULL);
   for (i = 1; i <= 10; i++)
     elm_genlist_item_append(genlist, &itc, (void*)(uintptr_t)i, parent,
                             0, NULL, NULL);

   verify_item_iteration_api(parent);

}
EFL_END_TEST

EFL_START_TEST (elm_genlist_legacy_type_check)
{
   const char *type;

   win = win_add(NULL, "genlist", ELM_WIN_BASIC);

   genlist = elm_genlist_add(win);

   type = elm_object_widget_type_get(genlist);
   ck_assert(type != NULL);
   ck_assert(!strcmp(type, "Elm_Genlist"));

   type = evas_object_type_get(genlist);
   ck_assert(type != NULL);
   ck_assert(!strcmp(type, "elm_genlist"));

}
EFL_END_TEST

void test_init(void)
{
   elm_config_atspi_mode_set(EINA_TRUE);
   win = win_add(NULL, "genlist", ELM_WIN_BASIC);
   genlist = elm_genlist_add(win);
}

EFL_START_TEST (elm_atspi_role_get)
{
   test_init();

   Efl_Access_Role role;

   role = efl_access_object_role_get(genlist);

   ck_assert(role == EFL_ACCESS_ROLE_LIST);

}
EFL_END_TEST

EFL_START_TEST(elm_atspi_children_get1)
{
   test_init();
   Eina_List *children;
   Elm_Object_Item *it[3];

   children = efl_access_object_access_children_get(genlist);
   ck_assert(children == NULL);

   it[0] = elm_genlist_item_append(genlist, &itc, NULL, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
   it[1] = elm_genlist_item_append(genlist, &itc, NULL, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
   it[2] = elm_genlist_item_append(genlist, &itc, NULL, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);

   children = efl_access_object_access_children_get(genlist);
   ck_assert(eina_list_count(children) == 3);
   ck_assert(eina_list_nth(children, 0) == it[0]);
   ck_assert(eina_list_nth(children, 1) == it[1]);
   ck_assert(eina_list_nth(children, 2) == it[2]);

   eina_list_free(children);

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

   children = efl_access_object_access_children_get(genlist);
   ck_assert(eina_list_nth(children, 1) == it[0]);
   ck_assert(eina_list_nth(children, 0) == it[1]);
   ck_assert(eina_list_nth(children, 2) == it[2]);

}
EFL_END_TEST

static void
_children_changed_cb(void *data EINA_UNUSED, const Efl_Event *event)
{
   if (event->desc != EFL_ACCESS_OBJECT_EVENT_CHILDREN_CHANGED)
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

   efl_access_object_event_handler_add(EFL_ACCESS_OBJECT_MIXIN, _children_changed_cb, NULL);

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

   efl_access_object_event_handler_add(EFL_ACCESS_OBJECT_MIXIN, _children_changed_cb, NULL);

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

}
EFL_END_TEST

EFL_START_TEST(elm_atspi_children_events_del2)
{
   test_init();

   Elm_Object_Item *it;
   current = NULL;
   counter = 0;

   it = elm_genlist_item_append(genlist, &itc, NULL, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);

   efl_access_object_event_handler_add(EFL_ACCESS_OBJECT_MIXIN, _children_changed_cb, NULL);
   elm_genlist_clear(genlist);

   ck_assert(genlist == current);
   ck_assert(counter == 1);
   ck_assert(ev_data.is_added == EINA_FALSE);
   ck_assert(ev_data.child == it);

}
EFL_END_TEST

static int it_del = 0;

static void
_gl_destroy()
{
   ck_assert_int_eq(it_del, 2);
}

static void
_it_destroy()
{
   it_del++;
}

static void
_it_del()
{
   it_del = 1;
}

EFL_START_TEST(elm_genlist_item_destroy)
{
   Elm_Object_Item *git;

   win = win_add(NULL, "genlist", ELM_WIN_BASIC);

   genlist = elm_genlist_add(win);
   efl_event_callback_add(genlist, EFL_EVENT_DESTRUCT, _gl_destroy, NULL);

   git = elm_genlist_item_append(genlist, &itc, NULL, NULL, 0, NULL, NULL);
   efl_event_callback_add(git, EFL_EVENT_DEL, _it_del, NULL);
   efl_event_callback_add(git, EFL_EVENT_DESTRUCT, _it_destroy, NULL);

}
EFL_END_TEST

void elm_test_genlist(TCase *tc)
{
   tcase_add_test(tc, elm_genlist_legacy_type_check);
   tcase_add_test(tc, elm_genlist_item_destroy);
   tcase_add_test(tc, elm_genlist_item_iteration);
   tcase_add_test(tc, elm_atspi_role_get);
   tcase_add_test(tc, elm_atspi_children_get1);
   tcase_add_test(tc, elm_atspi_children_get2);
   tcase_add_test(tc, elm_atspi_children_events_add);
   tcase_add_test(tc, elm_atspi_children_events_del1);
   tcase_add_test(tc, elm_atspi_children_events_del2);
}
