#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_OBJECT_BETA
#include <Elementary.h>
#include "elm_suite.h"

static int prev_abort;
static int prev_abort_level;

EFL_START_TEST (elm_naviframe_test_legacy_type_check)
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

EFL_START_TEST (elm_naviframe_test_atspi_role_get)
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

EFL_START_TEST (elm_naviframe_test_content_preserve_on_pop)
{
   Evas_Object *win, *naviframe;

   win = win_add(NULL, "naviframe", ELM_WIN_BASIC);

   prev_abort = eina_log_abort_on_critical_get();
   prev_abort_level = eina_log_abort_on_critical_level_get();
   eina_log_abort_on_critical_set(1);
   eina_log_abort_on_critical_level_set(1);

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
   eina_log_abort_on_critical_set(prev_abort);
   eina_log_abort_on_critical_level_set(prev_abort_level);
}
EFL_END_TEST

void elm_test_naviframe(TCase *tc)
{
   tcase_add_test(tc, elm_naviframe_test_legacy_type_check);
   tcase_add_test(tc, elm_naviframe_test_atspi_role_get);

   tcase_add_test(tc, elm_naviframe_test_content_preserve_on_pop);
}
