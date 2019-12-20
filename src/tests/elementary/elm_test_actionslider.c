#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_OBJECT_BETA
#include <Elementary.h>
#include "elm_suite.h"

EFL_START_TEST(elm_actionslider_legacy_type_check)
{
   Evas_Object *win, *actionslider;
   const char *type;

   win = win_add(NULL, "actionslider", ELM_WIN_BASIC);

   actionslider = elm_actionslider_add(win);

   type = elm_object_widget_type_get(actionslider);
   ck_assert(type != NULL);
   ck_assert(!strcmp(type, "Elm_Actionslider"));

   type = evas_object_type_get(actionslider);
   ck_assert(type != NULL);
   ck_assert(!strcmp(type, "elm_actionslider"));

}
EFL_END_TEST

EFL_START_TEST(elm_atspi_role_get)
{
   Evas_Object *win, *actionslider;
   Efl_Access_Role role;

   win = win_add(NULL, "actionslider", ELM_WIN_BASIC);

   actionslider = elm_actionslider_add(win);
   role = efl_access_object_role_get(actionslider);

   ck_assert(role == EFL_ACCESS_ROLE_SLIDER);

}
EFL_END_TEST

static const char *test_val;

static void
test_selected_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   /* if the test crashes with a crazy stack trace, this assert failed */
   ck_assert_str_eq(event_info, test_val);
   ecore_main_loop_quit();
}

EFL_START_TEST(elm_actionslider_test_callbacks)
{
   Evas_Object *win, *as;
   int called = 0;

   win = win_add(NULL, "as", ELM_WIN_BASIC);

   as = elm_actionslider_add(win);
   evas_object_resize(as, 200, 200);
   evas_object_resize(win, 200, 200);

   evas_object_show(win);
   evas_object_show(as);
   elm_actionslider_indicator_pos_set(as, ELM_ACTIONSLIDER_RIGHT);
   elm_actionslider_magnet_pos_set(as, ELM_ACTIONSLIDER_RIGHT);
   elm_object_part_text_set(as, "left", "test1");
   elm_object_part_text_set(as, "center", "test2");
   elm_object_part_text_set(as, "right", "test3");
   elm_actionslider_enabled_pos_set(as, ELM_ACTIONSLIDER_LEFT |
                                    ELM_ACTIONSLIDER_CENTER | ELM_ACTIONSLIDER_RIGHT);
   evas_object_smart_callback_add(as, "pos_changed",
                                  event_callback_single_call_int_data, &called);
   evas_object_smart_callback_add(as, "selected", test_selected_cb, &called);

   get_me_to_those_events(as);

   test_val = "test2";
   called = 0;
   click_part(as, "elm.text.left");
   edje_object_message_signal_process(as);
   ecore_main_loop_begin();
   ck_assert_int_eq(called, 1);

   test_val = "test1";
   called = 0;
   click_part(as, "elm.text.left");
   edje_object_message_signal_process(as);
   ecore_main_loop_begin();
   ck_assert_int_eq(called, 1);

   test_val = "test2";
   called = 0;
   click_part(as, "elm.text.center");
   edje_object_message_signal_process(as);
   ecore_main_loop_begin();
   ck_assert_int_eq(called, 1);

   test_val = "test3";
   called = 0;
   click_part(as, "elm.text.right");
   edje_object_message_signal_process(as);
   ecore_main_loop_begin();
   ck_assert_int_eq(called, 1);
}
EFL_END_TEST

EFL_START_TEST(elm_actionslider_test_parts)
{
   Evas_Object *win, *as;

   win = win_add(NULL, "as", ELM_WIN_BASIC);

   as = elm_actionslider_add(win);

   elm_actionslider_indicator_pos_set(as, ELM_ACTIONSLIDER_RIGHT);
   ck_assert_int_eq(elm_actionslider_indicator_pos_get(as), ELM_ACTIONSLIDER_RIGHT);
   elm_actionslider_magnet_pos_set(as, ELM_ACTIONSLIDER_RIGHT);
   ck_assert_int_eq(elm_actionslider_magnet_pos_get(as), ELM_ACTIONSLIDER_RIGHT);
   elm_object_part_text_set(as, "left", "Snooze");
   ck_assert_str_eq(elm_object_part_text_get(as, "left"), "Snooze");
   elm_object_part_text_set(as, "center", NULL);
   ck_assert_ptr_eq(elm_object_part_text_get(as, "center"), NULL);
   elm_object_part_text_set(as, "right", "Stop");
   ck_assert_str_eq(elm_object_part_text_get(as, "right"), "Stop");
   elm_actionslider_enabled_pos_set(as, ELM_ACTIONSLIDER_LEFT | ELM_ACTIONSLIDER_RIGHT);
   ck_assert_int_eq(elm_actionslider_enabled_pos_get(as), ELM_ACTIONSLIDER_LEFT | ELM_ACTIONSLIDER_RIGHT);
}
EFL_END_TEST

void elm_test_actionslider(TCase *tc)
{
   tcase_add_test(tc, elm_actionslider_legacy_type_check);
   tcase_add_test(tc, elm_atspi_role_get);
   tcase_add_test(tc, elm_actionslider_test_callbacks);
   tcase_add_test(tc, elm_actionslider_test_parts);
}
