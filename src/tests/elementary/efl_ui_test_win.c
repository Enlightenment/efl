#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "efl_ui_suite.h"

static void
_boolean_flag_set(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Eina_Bool *flag = data;
   *flag = EINA_TRUE;

   efl_loop_quit(efl_main_loop_get(), EINA_VALUE_EMPTY);
}

EFL_START_TEST(efl_ui_win_test_scene_focus)
{
   Efl_Ui_Win *win1;
   Ecore_Evas *ee;

   Eina_Bool win1_focus_in = EINA_FALSE;
   Eina_Bool win1_focus_out = EINA_FALSE;

   win1 = efl_new(EFL_UI_WIN_CLASS);

   //we want to test here the correct propagation of events from ecore_evas to the win object
   //in order to ensure that we are resetting ee focus first here.
   ee = ecore_evas_ecore_evas_get(evas_object_evas_get(win1));
   ecore_evas_focus_set(ee, EINA_TRUE);
   ecore_evas_focus_set(ee, EINA_FALSE);
   efl_event_callback_add(win1, EFL_CANVAS_SCENE_EVENT_SCENE_FOCUS_IN, _boolean_flag_set, &win1_focus_in);
   efl_event_callback_add(win1, EFL_CANVAS_SCENE_EVENT_SCENE_FOCUS_OUT, _boolean_flag_set, &win1_focus_out);

   //focus in check
   ecore_evas_focus_set(ee, EINA_TRUE);
   if (!win1_focus_in)
     efl_loop_begin(efl_main_loop_get());
   ck_assert_int_eq(win1_focus_in, EINA_TRUE);
   ck_assert_int_eq(win1_focus_out, EINA_FALSE);
   win1_focus_in = EINA_FALSE;

   //focus out check
   ecore_evas_focus_set(ee, EINA_FALSE);
   if (!win1_focus_out)
     efl_loop_begin(efl_main_loop_get());

   ck_assert_int_eq(win1_focus_out, EINA_TRUE);
   ck_assert_int_eq(win1_focus_in, EINA_FALSE);

   efl_unref(win1);
}
EFL_END_TEST

static void
_check_focus_event(void *data, const Efl_Event *ev)
{
   void **tmp = data;

   *tmp = efl_input_focus_object_get(ev->info);
}

EFL_START_TEST(efl_ui_win_test_object_focus)
{
   Efl_Ui_Win *win1;

   void *win1_focus_in = NULL;
   void *win1_focus_out = NULL;

   win1 = efl_new(EFL_UI_WIN_CLASS);

   efl_event_callback_add(win1, EFL_CANVAS_SCENE_EVENT_OBJECT_FOCUS_IN, _check_focus_event, &win1_focus_in);
   efl_event_callback_add(win1, EFL_CANVAS_SCENE_EVENT_OBJECT_FOCUS_OUT, _check_focus_event, &win1_focus_out);

   Eo *r1 = efl_add(EFL_CANVAS_RECTANGLE_CLASS, evas_object_evas_get(win1));
   efl_canvas_object_seat_focus_add(r1, NULL);
   ck_assert_ptr_eq(win1_focus_in, r1);
   ck_assert_ptr_eq(win1_focus_out, NULL);
   win1_focus_in = NULL;
   win1_focus_out = NULL;

   Eo *r2 = efl_add(EFL_CANVAS_RECTANGLE_CLASS, evas_object_evas_get(win1));
   efl_canvas_object_seat_focus_add(r2, NULL);
   ck_assert_ptr_eq(win1_focus_in, r2);
   ck_assert_ptr_eq(win1_focus_out, r1);
   win1_focus_in = NULL;
   win1_focus_out = NULL;

   efl_canvas_object_seat_focus_del(r2, NULL);
   ck_assert_ptr_eq(win1_focus_in, NULL);
   ck_assert_ptr_eq(win1_focus_out, r2);

   efl_unref(win1);
}
EFL_END_TEST

static void
create_environment(Eo **win, Eo **rect)
{
   *win = efl_new(EFL_UI_WIN_CLASS);
   *rect = efl_add(EFL_CANVAS_RECTANGLE_CLASS, evas_object_evas_get(*win));
   efl_canvas_object_seat_focus_add(*rect, NULL);
}

EFL_START_TEST(efl_ui_win_test_efl_input_interface_focus)
{
   Efl_Ui_Win *win;
   Efl_Canvas_Object *rect, *focus_in = NULL, *focus_out = NULL;
   create_environment(&win, &rect);
   efl_canvas_object_seat_focus_del(rect, NULL);

   efl_event_callback_add(rect, EFL_EVENT_FOCUS_IN , _check_focus_event, &focus_in);
   efl_event_callback_add(rect, EFL_EVENT_FOCUS_OUT, _check_focus_event, &focus_out);

   efl_canvas_object_seat_focus_add(rect, NULL);
   ck_assert_ptr_eq(focus_out, NULL);
   ck_assert_ptr_eq(focus_in, rect);
   focus_out = NULL;
   focus_in = NULL;

   efl_canvas_object_seat_focus_del(rect, NULL);
   ck_assert_ptr_eq(focus_out, rect);
   ck_assert_ptr_eq(focus_in, NULL);
   focus_out = NULL;
   focus_in = NULL;
}
EFL_END_TEST

#define TIMESTAMP 1337

static void
_check_key_event(void *data, const Efl_Event *ev)
{
   Eina_Bool *pressed = data;

   ck_assert_int_eq(efl_input_timestamp_get(ev->info), TIMESTAMP);
   ck_assert_str_eq(efl_input_key_name_get(ev->info), "odiaeresis");
   ck_assert_str_eq(efl_input_key_get(ev->info), "ö");
   ck_assert_str_eq(efl_input_key_string_get(ev->info), "Ö");
   ck_assert_str_eq(efl_input_key_compose_string_get(ev->info), "Ö");
   ck_assert_int_eq(efl_input_key_code_get(ev->info), 0xffe1);
   *pressed = efl_input_key_pressed_get(ev->info);
}

EFL_START_TEST(efl_ui_win_test_efl_input_interface_key_down)
{
   Efl_Ui_Win *win;
   Eina_Bool pressed = EINA_FALSE;
   Efl_Canvas_Object *rect;
   create_environment(&win, &rect);

   efl_event_callback_add(rect, EFL_EVENT_KEY_DOWN , _check_key_event, &pressed);
   efl_event_callback_add(rect, EFL_EVENT_KEY_UP, _check_key_event, &pressed);

   evas_event_feed_key_down_with_keycode(evas_object_evas_get(win), "odiaeresis", "ö", "Ö", "Ö", TIMESTAMP, NULL, 0xffe1);
   ck_assert_int_eq(pressed, EINA_TRUE);

   evas_event_feed_key_up_with_keycode(evas_object_evas_get(win), "odiaeresis", "ö", "Ö", "Ö", TIMESTAMP, NULL, 0xffe1);
   ck_assert_int_eq(pressed, EINA_FALSE);
}
EFL_END_TEST

void
efl_ui_test_win(TCase *tc)
{
   tcase_add_test(tc, efl_ui_win_test_scene_focus);
   tcase_add_test(tc, efl_ui_win_test_object_focus);
   tcase_add_test(tc, efl_ui_win_test_object_focus);
   tcase_add_test(tc, efl_ui_win_test_efl_input_interface_focus);
   tcase_add_test(tc, efl_ui_win_test_efl_input_interface_key_down);
}
