#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_OBJECT_BETA
#define EFL_ACCESS_OBJECT_PROTECTED
#include <Elementary.h>
#include "elm_suite.h"
#include "suite_helpers.h"

static Evas_Object *win, *genlist;
static Elm_Gen_Item_Class itc = { .version = ELM_GENLIST_ITEM_CLASS_VERSION };
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
          DISABLE_ABORT_ON_CRITICAL_START;
          // item #11 do not exists
          ck_assert_int_eq(elm_genlist_item_index_get(it), -1);
          DISABLE_ABORT_ON_CRITICAL_END;
        else
          ck_assert_int_eq(elm_genlist_item_index_get(it), i + 1);

        if ((i == 0) || (i == 11))
          DISABLE_ABORT_ON_CRITICAL_START;
          // test first and item #11 (that do not exists)
          ck_assert_ptr_eq(elm_object_item_data_get(it), NULL);
          DISABLE_ABORT_ON_CRITICAL_END;
        else
          ck_assert_ptr_eq(elm_object_item_data_get(it), (void*)(uintptr_t)i);
     }
}

EFL_START_TEST(elm_genlist_test_item_iteration)
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

static void
_genlist_item_content_test_realize(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   ecore_main_loop_quit();
}

static Evas_Object *
_item_content_get(void *data EINA_UNUSED, Evas_Object *obj, const char *part EINA_UNUSED)
{
   Evas_Object *ic = elm_button_add(obj);
   return ic;
}

EFL_START_TEST(elm_genlist_test_item_content)
{
   Elm_Genlist_Item_Class *gtc;
   Evas_Object *end, *parent;
   Elm_Object_Item *it;

   gtc = elm_genlist_item_class_new();
   gtc->item_style = "default";
   gtc->func.content_get = _item_content_get;
   gtc->func.state_get = NULL;
   gtc->func.del = NULL;

   win = win_add(NULL, "genlist", ELM_WIN_BASIC);

   genlist = elm_genlist_add(win);
   evas_object_smart_callback_add(genlist, "realized", _genlist_item_content_test_realize, NULL);

   it = elm_genlist_item_append(genlist, gtc, NULL, NULL,
                                ELM_GENLIST_ITEM_NONE, NULL, NULL);

   evas_object_resize(genlist, 100, 100);
   evas_object_resize(win, 150, 150);
   evas_object_show(genlist);
   evas_object_show(win);

   ecore_main_loop_begin();

   end = elm_object_item_part_content_get(it, "elm.swallow.end");
   parent = elm_object_parent_widget_get(end);
   ck_assert_ptr_eq(parent, genlist);

   elm_genlist_item_all_contents_unset(it, NULL);
   parent = elm_object_parent_widget_get(end);
   ck_assert_ptr_eq(parent, win);
}
EFL_END_TEST

EFL_START_TEST(elm_genlist_test_legacy_type_check)
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

EFL_START_TEST(elm_genlist_test_atspi_role_get)
{
   test_init();

   Efl_Access_Role role;

   role = efl_access_object_role_get(genlist);

   ck_assert(role == EFL_ACCESS_ROLE_LIST);

}
EFL_END_TEST

EFL_START_TEST(elm_genlist_test_atspi_children_get1)
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

EFL_START_TEST(elm_genlist_test_atspi_children_get2)
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

EFL_START_TEST(elm_genlist_test_atspi_children_events_add)
{
   test_init();

   current = NULL;
   counter = 0;

   Elm_Object_Item *it[3];

   efl_access_object_event_handler_add(_children_changed_cb, NULL);

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

EFL_START_TEST(elm_genlist_test_atspi_children_events_del1)
{
   test_init();

   current = NULL;
   counter = 0;

   Elm_Object_Item *it[3];

   it[0] = elm_genlist_item_append(genlist, &itc, NULL, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
   it[1] = elm_genlist_item_prepend(genlist, &itc, NULL, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
   it[2] = elm_genlist_item_append(genlist, &itc, NULL, NULL, ELM_GENLIST_ITEM_TREE, NULL, NULL);

   efl_access_object_event_handler_add(_children_changed_cb, NULL);

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

EFL_START_TEST(elm_genlist_test_atspi_children_events_del2)
{
   test_init();

   Elm_Object_Item *it;
   current = NULL;
   counter = 0;

   it = elm_genlist_item_append(genlist, &itc, NULL, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);

   efl_access_object_event_handler_add(_children_changed_cb, NULL);
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

EFL_START_TEST(elm_genlist_test_item_destroy)
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

#define NUM_TREE_ITEMS 10

static Evas_Object *
genlist_tree_test_content_get(void *data EINA_UNUSED, Evas_Object *obj, const char *part)
{
   if (!strcmp(part, "elm.swallow.icon"))
     {
        Evas_Object *ic = elm_icon_add(obj);
        elm_image_file_set(ic, ELM_IMAGE_DATA_DIR "/images/logo_small.png", NULL);
        evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
        evas_object_show(ic);
        return ic;
     }
   else if (!strcmp(part, "elm.swallow.end"))
     {
        Evas_Object *ck;
        ck = elm_check_add(obj);
        evas_object_propagate_events_set(ck, EINA_FALSE);
        evas_object_show(ck);
        return ck;
     }
   return NULL;
}

static void
_focus_set(void *data)
{
   elm_genlist_item_selected_set(data, 1);
   elm_object_item_focus_set(data, 1);
}

static void
genlist_tree_test_realize(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   evas_object_smart_callback_del(genlist, "realized", genlist_tree_test_realize);
   ecore_job_add(_focus_set, event_info);
   elm_object_item_signal_emit(event_info, "elm,action,expand,toggle", "elm");
}

static void
genlist_tree_test_realize2(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   static unsigned int count;

   if (!elm_genlist_item_parent_get(event_info)) return;
   if (++count != NUM_TREE_ITEMS) return;
   evas_object_smart_callback_del(genlist, "realized", genlist_tree_test_realize2);
   elm_object_item_signal_emit(elm_genlist_first_item_get(genlist), "elm,action,expand,toggle", "elm");
}

static void
genlist_tree_test_expand(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Elm_Object_Item *glit = event_info;
   Evas_Object *gl = elm_object_item_widget_get(glit);
   int i = 0;

   evas_object_smart_callback_add(genlist, "realized", genlist_tree_test_realize2, NULL);
   for (i = 0; i < NUM_TREE_ITEMS; i++)
     {
        elm_genlist_item_append(gl, &itc,
                                NULL/* item data */,
                                glit/* parent */,
                                ELM_GENLIST_ITEM_TREE, NULL/* func */,
                                NULL/* func data */);
     }
}

static void
_do_quit()
{
   itc.func.content_get = NULL;
   fail_on_errors_teardown();
   ecore_main_loop_quit();
}

static void
genlist_tree_test_contract(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Elm_Object_Item *glit = event_info;
   ck_assert_int_eq(elm_genlist_item_subitems_count(glit), NUM_TREE_ITEMS);
   elm_genlist_item_subitems_clear(glit);
   ecore_job_add(_do_quit, NULL);
}

static void
genlist_tree_test_expand_request(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Elm_Object_Item *glit = event_info;
   elm_genlist_item_expanded_set(glit, EINA_TRUE);
}

static void
genlist_tree_test_contract_request(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Elm_Object_Item *glit = event_info;
   elm_genlist_item_expanded_set(glit, EINA_FALSE);
}

EFL_START_TEST(elm_genlist_test_tree_expand)
{
   int i;
   win = win_add(NULL, "genlist", ELM_WIN_BASIC);

   itc.func.content_get = genlist_tree_test_content_get;

   genlist = elm_genlist_add(win);
   elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);
   elm_genlist_multi_select_set(genlist, EINA_TRUE);

   evas_object_smart_callback_add(genlist, "expand,request", genlist_tree_test_expand_request, NULL);
   evas_object_smart_callback_add(genlist, "contract,request", genlist_tree_test_contract_request, NULL);
   evas_object_smart_callback_add(genlist, "expanded", genlist_tree_test_expand, NULL);
   evas_object_smart_callback_add(genlist, "contracted", genlist_tree_test_contract, NULL);
   evas_object_smart_callback_add(genlist, "realized", genlist_tree_test_realize, NULL);

   evas_object_size_hint_align_set(genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_genlist_item_append(genlist, &itc,
                           NULL/* item data */, NULL/* parent */,
                           ELM_GENLIST_ITEM_TREE, NULL/* func */,
                           NULL/* func data */);
   for (i = 0; i < 30; i++)
     elm_genlist_item_append(genlist, &itc,
                             NULL/* item data */, NULL/* parent */,
                             0, NULL/* func */,
                             NULL/* func data */);

   evas_object_show(genlist);
   evas_object_resize(genlist, 100, 10 + 10 * NUM_TREE_ITEMS);
   evas_object_show(win);
   evas_object_resize(win, 100, 10 + 10 * NUM_TREE_ITEMS);
   fail_on_errors_setup();
   ecore_main_loop_begin();
}
EFL_END_TEST

EFL_START_TEST(elm_genlist_test_focus_state)
{
   Elm_Object_Item *it;
   Evas_Object *btn;

   win = win_add_focused(NULL, "genlist", ELM_WIN_BASIC);

   evas_object_show(win);

   btn = elm_button_add(win);
   evas_object_show(btn);
   elm_object_focus_set(btn, EINA_TRUE);

   genlist = elm_genlist_add(win);

   it = elm_genlist_item_append(genlist, &itc, NULL, NULL,
                                ELM_GENLIST_ITEM_NONE, NULL, NULL);
   evas_object_show(genlist);

   elm_object_focus_set(genlist, EINA_TRUE);
   elm_object_item_focus_set(it, EINA_TRUE);
   ck_assert_ptr_ne(elm_object_focused_object_get(win), btn);
   ck_assert_ptr_eq(elm_object_focused_object_get(win), genlist);
   ck_assert_int_eq(elm_object_focus_get(btn), EINA_FALSE);
   ck_assert_int_eq(elm_object_focus_get(genlist), EINA_TRUE);

   elm_object_focus_set(genlist, EINA_FALSE);
   ck_assert_ptr_ne(elm_object_focused_object_get(win), genlist);
   ck_assert_ptr_eq(elm_object_focused_object_get(win), btn);
   ck_assert_int_eq(elm_object_focus_get(btn), EINA_TRUE);
   ck_assert_int_eq(elm_object_focus_get(genlist), EINA_FALSE);

   elm_object_focus_set(btn, EINA_TRUE);
   elm_object_focus_set(genlist, EINA_TRUE);
   elm_object_item_focus_set(it, EINA_TRUE);
   evas_object_hide(genlist);
   ck_assert_ptr_ne(elm_object_focused_object_get(win), genlist);
   ck_assert_ptr_eq(elm_object_focused_object_get(win), btn);
   ck_assert_int_eq(elm_object_focus_get(btn), EINA_TRUE);
   ck_assert_int_eq(elm_object_focus_get(genlist), EINA_FALSE);
}
EFL_END_TEST

void elm_test_genlist(TCase *tc)
{
   tcase_add_test(tc, elm_genlist_test_legacy_type_check);
   tcase_add_test(tc, elm_genlist_test_item_destroy);
   tcase_add_test(tc, elm_genlist_test_item_iteration);
   tcase_add_test(tc, elm_genlist_test_item_content);
   tcase_add_test(tc, elm_genlist_test_atspi_role_get);
   tcase_add_test(tc, elm_genlist_test_atspi_children_get1);
   tcase_add_test(tc, elm_genlist_test_atspi_children_get2);
   tcase_add_test(tc, elm_genlist_test_atspi_children_events_add);
   tcase_add_test(tc, elm_genlist_test_atspi_children_events_del1);
   tcase_add_test(tc, elm_genlist_test_atspi_children_events_del2);

   tcase_add_test(tc, elm_genlist_test_focus_state);
   tcase_add_test(tc, elm_genlist_test_tree_expand);
}
