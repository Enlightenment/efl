#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_OBJECT_BETA
#include <Elementary.h>
#include "elm_suite.h"

EFL_START_TEST(elm_naviframe_test_legacy_type_check)
{
   Evas_Object *win, *naviframe;
   const char *type;

   win = win_add(NULL, "naviframe", ELM_WIN_BASIC);

   naviframe = elm_naviframe_add(win);

   type = elm_object_widget_type_get(naviframe);
   ck_assert(type != NULL);
   ck_assert(!strcmp(type, "Elm_Naviframe"));

   type = evas_object_type_get(naviframe);
   ck_assert(type != NULL);
   ck_assert(!strcmp(type, "elm_naviframe"));

}
EFL_END_TEST

EFL_START_TEST(elm_naviframe_test_atspi_role_get)
{
   Evas_Object *win, *naviframe;
   Efl_Access_Role role;

   win = win_add(NULL, "naviframe", ELM_WIN_BASIC);

   naviframe = elm_naviframe_add(win);
   role = efl_access_object_role_get(naviframe);

   ck_assert(role == EFL_ACCESS_ROLE_PAGE_TAB_LIST);

}
EFL_END_TEST

static Evas_Object *preserve_on_pop_button;
static Evas_Object *button;
static Elm_Object_Item *popped_it;

static void
_test_fail(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   ck_abort_msg("Object deleted!");
}

void
_elm_naviframe_test_content_preserve_on_pop_button_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   elm_naviframe_item_push(data, "Next Page", NULL, NULL, preserve_on_pop_button, NULL);
}

static void
_transition_finished(void *data EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   static int count;
   if (popped_it)
     {
        if (count++)
          {
             evas_object_event_callback_del(preserve_on_pop_button, EVAS_CALLBACK_DEL, _test_fail);
             ecore_main_loop_quit();
          }
        else
          elm_object_signal_emit(button, "elm,action,click", "elm");
     }
   else
     popped_it = elm_naviframe_item_pop(obj);
}

EFL_START_TEST(elm_naviframe_test_content_preserve_on_pop)
{
   Evas_Object *win, *naviframe;

   win = win_add(NULL, "naviframe", ELM_WIN_BASIC);

   naviframe = elm_naviframe_add(win);
   elm_naviframe_content_preserve_on_pop_set(naviframe, EINA_TRUE);

   evas_object_size_hint_weight_set(naviframe, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, naviframe);
   evas_object_show(naviframe);

   button = elm_button_add(naviframe);
   elm_object_text_set(button, "Go to Second Page.");
   evas_object_smart_callback_add(button, "clicked", _elm_naviframe_test_content_preserve_on_pop_button_cb, naviframe);

   preserve_on_pop_button = elm_button_add(naviframe);
   evas_object_event_callback_add(preserve_on_pop_button, EVAS_CALLBACK_DEL, _test_fail, NULL);
   elm_object_text_set(preserve_on_pop_button, "Go to Next Page.");
   evas_object_smart_callback_add(preserve_on_pop_button, "clicked", _elm_naviframe_test_content_preserve_on_pop_button_cb, naviframe);

   elm_naviframe_item_push(naviframe, "First Page", NULL, NULL, button, NULL);

   evas_object_smart_callback_add(naviframe, "transition,finished", _transition_finished, NULL);

   evas_object_resize(win, 400, 400);
   evas_object_show(win);
   /* trigger next page push */
   elm_object_signal_emit(button, "elm,action,click", "elm");

   ecore_main_loop_begin();
}
EFL_END_TEST

static Eo *
create_content(Eo *parent)
{
   Eo *content = elm_layout_add(parent);

   evas_object_size_hint_min_set(content, 50, 50);

   return content;
}

#define NUM_ITEMS 10
EFL_START_TEST(elm_naviframe_test_behavior)
{
   Evas_Object *win, *naviframe;
   //int title_transition = 0;
   int title_clicked = 0;
   Elm_Object_Item *active_it = NULL;
   Elm_Object_Item *it[NUM_ITEMS];
   int i;

   win = win_add(NULL, "naviframe", ELM_WIN_BASIC);
   evas_object_resize(win, 500, 500);

   naviframe = elm_naviframe_add(win);
   //this seems broken beyond space and time
   //evas_object_smart_callback_add(naviframe, "title,transition,finished", event_callback_single_call_int_data, &title_transition);
   evas_object_smart_callback_add(naviframe, "title,clicked", event_callback_single_call_int_data, &title_clicked);
   evas_object_smart_callback_add(naviframe, "transition,finished", event_callback_that_quits_the_main_loop_when_called, NULL);
   evas_object_smart_callback_add(naviframe, "item,activated", smart_callback_that_stores_event_info_to_data, &active_it);
   evas_object_resize(naviframe, 500, 500);
   evas_object_show(naviframe);

   for (i = 0; i < NUM_ITEMS; i++)
     {
        char buf[12];
        snprintf(buf, sizeof(buf), "page%d", i);
        it[i] = elm_naviframe_item_push(naviframe, buf, NULL, NULL, create_content(naviframe), NULL);
        elm_naviframe_item_title_enabled_set(it[i], EINA_TRUE, EINA_TRUE);
     }
   ck_assert_ptr_eq(it[0], active_it);

   evas_object_show(win);
   get_me_to_those_events(win);

   ecore_main_loop_begin();

   ck_assert_ptr_eq(it[9], active_it);
   click_part(naviframe, "elm.text.title");
   get_me_to_those_events(win);
   ck_assert_int_eq(title_clicked, 1);

   click_part(naviframe, "elm.swallow.prev_btn");
   ecore_main_loop_begin();
   /* reaching this point verifies that the back button works */
}
EFL_END_TEST

void elm_test_naviframe(TCase *tc)
{
   tcase_add_test(tc, elm_naviframe_test_legacy_type_check);
   tcase_add_test(tc, elm_naviframe_test_atspi_role_get);

   tcase_add_test(tc, elm_naviframe_test_content_preserve_on_pop);
   tcase_add_test(tc, elm_naviframe_test_behavior);
}
