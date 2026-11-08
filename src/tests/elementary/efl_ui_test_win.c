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
   efl_gfx_entity_geometry_set(*win, EINA_RECT(0, 0, 200, 200));
   efl_gfx_entity_geometry_set(*rect, EINA_RECT(0, 0, 200, 200));
   get_me_to_those_events(evas_object_evas_get(*win));
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
   ck_assert_str_eq(efl_input_key_sym_get(ev->info), "ö");
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

static void
position_eq(Eina_Position2D a, Eina_Position2D b)
{
   ck_assert_int_eq(a.x, b.x);
   ck_assert_int_eq(a.y, b.y);
}

static void
_check_ptr_move_event(void *data, const Efl_Event *ev)
{
   Eina_Bool *called = data;
   ck_assert_int_eq(efl_input_timestamp_get(ev->info), TIMESTAMP);
   ck_assert_int_eq(efl_input_pointer_wheel_delta_get(ev->info), 0);
   ck_assert_int_eq(efl_input_pointer_wheel_horizontal_get(ev->info), 0);
   ck_assert_int_eq(efl_input_pointer_double_click_get(ev->info), 0);
   ck_assert_int_eq(efl_input_pointer_triple_click_get(ev->info), 0);
   ck_assert_int_eq(efl_input_pointer_button_flags_get(ev->info), 0);
   ck_assert_int_eq(efl_input_pointer_touch_id_get(ev->info), 0);
   position_eq(efl_input_pointer_previous_position_get(ev->info), EINA_POSITION2D(20, 20));
   position_eq(efl_input_pointer_position_get(ev->info), EINA_POSITION2D(25, 25));
   ck_assert_int_eq(efl_input_pointer_button_get(ev->info), 0);
   ck_assert_int_eq(efl_input_pointer_action_get(ev->info), EFL_POINTER_ACTION_MOVE);
   *called = EINA_TRUE;
}

EFL_START_TEST(efl_ui_win_test_efl_input_interface_pointer_move)
{
   Efl_Ui_Win *win;
   Eina_Bool called = EINA_FALSE;
   Efl_Canvas_Object *rect;
   create_environment(&win, &rect);

   evas_event_feed_mouse_move(evas_object_evas_get(win), 20, 20, TIMESTAMP - 1, NULL);
   efl_event_callback_add(rect, EFL_EVENT_POINTER_MOVE , _check_ptr_move_event, &called);
   evas_event_feed_mouse_move(evas_object_evas_get(win), 25, 25, TIMESTAMP, NULL);
   ck_assert_int_eq(called, EINA_TRUE);
}
EFL_END_TEST

static void
_check_ptr_down_event(void *data, const Efl_Event *ev)
{
   Eina_Bool *called = data;
   ck_assert_int_eq(efl_input_timestamp_get(ev->info), TIMESTAMP);
   ck_assert_int_eq(efl_input_pointer_wheel_delta_get(ev->info), 0);
   ck_assert_int_eq(efl_input_pointer_wheel_horizontal_get(ev->info), 0);
   ck_assert_int_eq(efl_input_pointer_double_click_get(ev->info), 0);
   ck_assert_int_eq(efl_input_pointer_triple_click_get(ev->info), EINA_TRUE);
   ck_assert_int_eq(efl_input_pointer_button_flags_get(ev->info), EVAS_BUTTON_TRIPLE_CLICK);
   ck_assert_int_eq(efl_input_pointer_touch_id_get(ev->info), 0);
   position_eq(efl_input_pointer_position_get(ev->info), EINA_POSITION2D(20, 20));
   ck_assert_int_eq(efl_input_pointer_button_get(ev->info), 1);
   ck_assert_int_eq(efl_input_pointer_action_get(ev->info), EFL_POINTER_ACTION_DOWN);
   *called = EINA_TRUE;
}

EFL_START_TEST(efl_ui_win_test_efl_input_interface_pointer_down)
{
   Efl_Ui_Win *win;
   Eina_Bool called = EINA_FALSE;
   Efl_Canvas_Object *rect;
   create_environment(&win, &rect);

   evas_event_feed_mouse_move(evas_object_evas_get(win), 20, 20, TIMESTAMP - 1, NULL);
   efl_event_callback_add(rect, EFL_EVENT_POINTER_DOWN , _check_ptr_down_event, &called);
   evas_event_feed_mouse_down(evas_object_evas_get(win), 1, EVAS_BUTTON_TRIPLE_CLICK, TIMESTAMP, NULL);
   ck_assert_int_eq(called, EINA_TRUE);
}
EFL_END_TEST

static void
_check_ptr_up_event(void *data, const Efl_Event *ev)
{
   Eina_Bool *called = data;
   ck_assert_int_eq(efl_input_timestamp_get(ev->info), TIMESTAMP);
   ck_assert_int_eq(efl_input_pointer_wheel_delta_get(ev->info), 0);
   ck_assert_int_eq(efl_input_pointer_wheel_horizontal_get(ev->info), 0);
   ck_assert_int_eq(efl_input_pointer_double_click_get(ev->info), 0);
   ck_assert_int_eq(efl_input_pointer_triple_click_get(ev->info), EINA_TRUE);
   ck_assert_int_eq(efl_input_pointer_button_flags_get(ev->info), EVAS_BUTTON_TRIPLE_CLICK);
   ck_assert_int_eq(efl_input_pointer_touch_id_get(ev->info), 0);
   position_eq(efl_input_pointer_position_get(ev->info), EINA_POSITION2D(20, 20));
   ck_assert_int_eq(efl_input_pointer_button_get(ev->info), 1);
   ck_assert_int_eq(efl_input_pointer_action_get(ev->info), EFL_POINTER_ACTION_UP);
   *called = EINA_TRUE;
}

EFL_START_TEST(efl_ui_win_test_efl_input_interface_pointer_up)
{
   Efl_Ui_Win *win;
   Eina_Bool called = EINA_FALSE;
   Efl_Canvas_Object *rect;
   create_environment(&win, &rect);

   evas_event_feed_mouse_move(evas_object_evas_get(win), 20, 20, TIMESTAMP - 1, NULL);
   evas_event_feed_mouse_down(evas_object_evas_get(win), 1, EVAS_BUTTON_TRIPLE_CLICK, TIMESTAMP, NULL);
   efl_event_callback_add(rect, EFL_EVENT_POINTER_UP , _check_ptr_up_event, &called);
   evas_event_feed_mouse_up(evas_object_evas_get(win), 1, EVAS_BUTTON_TRIPLE_CLICK, TIMESTAMP, NULL);
   ck_assert_int_eq(called, EINA_TRUE);
}
EFL_END_TEST

static void
_check_pointer_in_cb(void *data, const Efl_Event *ev)
{
   Eina_Bool *called = data;
   ck_assert_int_eq(efl_input_timestamp_get(ev->info), TIMESTAMP);
   ck_assert_int_eq(efl_input_pointer_wheel_delta_get(ev->info), 0);
   ck_assert_int_eq(efl_input_pointer_wheel_horizontal_get(ev->info), 0);
   ck_assert_int_eq(efl_input_pointer_double_click_get(ev->info), 0);
   ck_assert_int_eq(efl_input_pointer_triple_click_get(ev->info), 0);
   ck_assert_int_eq(efl_input_pointer_button_flags_get(ev->info), 0);
   ck_assert_int_eq(efl_input_pointer_touch_id_get(ev->info), 0);
   position_eq(efl_input_pointer_position_get(ev->info), EINA_POSITION2D(20, 20));
   ck_assert_int_eq(efl_input_pointer_button_get(ev->info), 0);
   ck_assert_int_eq(efl_input_pointer_action_get(ev->info), EFL_POINTER_ACTION_IN );
   *called = EINA_TRUE;
}

static void
_check_pointer_out_cb(void *data, const Efl_Event *ev)
{
   Eina_Bool *called = data;
   ck_assert_int_eq(efl_input_timestamp_get(ev->info), TIMESTAMP);
   ck_assert_int_eq(efl_input_pointer_wheel_delta_get(ev->info), 0);
   ck_assert_int_eq(efl_input_pointer_wheel_horizontal_get(ev->info), 0);
   ck_assert_int_eq(efl_input_pointer_double_click_get(ev->info), 0);
   ck_assert_int_eq(efl_input_pointer_triple_click_get(ev->info), 0);
   ck_assert_int_eq(efl_input_pointer_button_flags_get(ev->info), 0);
   ck_assert_int_eq(efl_input_pointer_touch_id_get(ev->info), 0);
   position_eq(efl_input_pointer_position_get(ev->info), EINA_POSITION2D(5, 5));
   ck_assert_int_eq(efl_input_pointer_button_get(ev->info), 0);
   ck_assert_int_eq(efl_input_pointer_action_get(ev->info), EFL_POINTER_ACTION_OUT );
   *called = EINA_TRUE;
}

EFL_START_TEST(efl_ui_win_test_efl_input_interface_pointer_in_out)
{
   Efl_Ui_Win *win;
   Eina_Bool pointer_in = EINA_FALSE, pointer_out = EINA_FALSE;
   Efl_Canvas_Object *rect;

   create_environment(&win, &rect);
   evas_event_feed_mouse_move(evas_object_evas_get(win), 5, 5, TIMESTAMP - 1, NULL);
   evas_object_geometry_set(rect, 10, 10, 20, 20);

   evas_event_feed_mouse_move(evas_object_evas_get(win), 5, 5, TIMESTAMP - 1, NULL);
   efl_event_callback_add(rect, EFL_EVENT_POINTER_IN, _check_pointer_in_cb, &pointer_in);
   efl_event_callback_add(rect, EFL_EVENT_POINTER_OUT, _check_pointer_out_cb, &pointer_out);

   evas_event_feed_mouse_move(evas_object_evas_get(win), 20, 20, TIMESTAMP, NULL);
   ck_assert_int_eq(pointer_in, EINA_TRUE);
   ck_assert_int_eq(pointer_out, EINA_FALSE);
   pointer_in = EINA_FALSE;
   pointer_out = EINA_FALSE;

   evas_event_feed_mouse_move(evas_object_evas_get(win), 5, 5, TIMESTAMP, NULL);
   ck_assert_int_eq(pointer_in, EINA_FALSE);
   ck_assert_int_eq(pointer_out, EINA_TRUE);
}
EFL_END_TEST

static void
_check_pointer_wheel_cb(void *data, const Efl_Event *ev)
{
   Eina_Bool *called = data;
   ck_assert_int_eq(efl_input_timestamp_get(ev->info), TIMESTAMP);
   ck_assert_int_eq(efl_input_pointer_wheel_delta_get(ev->info), 2);
   ck_assert_int_eq(efl_input_pointer_wheel_horizontal_get(ev->info), 1);
   ck_assert_int_eq(efl_input_pointer_double_click_get(ev->info), 0);
   ck_assert_int_eq(efl_input_pointer_triple_click_get(ev->info), 0);
   ck_assert_int_eq(efl_input_pointer_button_flags_get(ev->info), 0);
   ck_assert_int_eq(efl_input_pointer_touch_id_get(ev->info), 0);
   position_eq(efl_input_pointer_position_get(ev->info), EINA_POSITION2D(20, 20));
   ck_assert_int_eq(efl_input_pointer_button_get(ev->info), 0);
   ck_assert_int_eq(efl_input_pointer_action_get(ev->info), EFL_POINTER_ACTION_WHEEL );
   *called = EINA_TRUE;
}

EFL_START_TEST(efl_ui_win_test_efl_input_interface_pointer_wheel)
{
   Efl_Ui_Win *win;
   Eina_Bool pointer_wheel = EINA_FALSE;
   Efl_Canvas_Object *rect;

   create_environment(&win, &rect);

   efl_event_callback_add(rect, EFL_EVENT_POINTER_WHEEL, _check_pointer_wheel_cb, &pointer_wheel);

   evas_event_feed_mouse_move(evas_object_evas_get(win), 20, 20, TIMESTAMP, NULL);
   evas_event_feed_mouse_wheel(evas_object_evas_get(win), 1, 2, TIMESTAMP, NULL);
   ck_assert_int_eq(pointer_wheel, EINA_TRUE);
}
EFL_END_TEST


static void
_check_ptr_cancel_down_event(void *data, const Efl_Event *ev)
{
   Eina_Bool *called = data;
   ck_assert_int_eq(efl_input_timestamp_get(ev->info), TIMESTAMP);
   ck_assert_int_eq(efl_input_pointer_wheel_delta_get(ev->info), 0);
   ck_assert_int_eq(efl_input_pointer_wheel_horizontal_get(ev->info), 0);
   ck_assert_int_eq(efl_input_pointer_double_click_get(ev->info), 0);
   ck_assert_int_eq(efl_input_pointer_triple_click_get(ev->info), 0);
   ck_assert_int_eq(efl_input_pointer_button_flags_get(ev->info), 0);
   ck_assert_int_eq(efl_input_pointer_touch_id_get(ev->info), 0);
   position_eq(efl_input_pointer_position_get(ev->info), EINA_POSITION2D(20, 20));
   ck_assert_int_eq(efl_input_pointer_button_get(ev->info), 1);
   ck_assert_int_eq(efl_input_pointer_action_get(ev->info), EFL_POINTER_ACTION_DOWN);
   *called = EINA_TRUE;
}

static void
_check_ptr_cancel_event(void *data, const Efl_Event *ev)
{
   int *called = data;
   ck_assert_int_eq(efl_input_timestamp_get(ev->info), TIMESTAMP);
   ck_assert_int_eq(efl_input_pointer_wheel_delta_get(ev->info), 0);
   ck_assert_int_eq(efl_input_pointer_wheel_horizontal_get(ev->info), 0);
   ck_assert_int_eq(efl_input_pointer_double_click_get(ev->info), 0);
   ck_assert_int_eq(efl_input_pointer_triple_click_get(ev->info), 0);
   ck_assert_int_eq(efl_input_pointer_button_flags_get(ev->info), 0);
   ck_assert_int_eq(efl_input_pointer_touch_id_get(ev->info), 0);
   position_eq(efl_input_pointer_position_get(ev->info), EINA_POSITION2D(20, 20));
   ck_assert_int_eq(efl_input_pointer_button_get(ev->info), 1);
   ck_assert_int_eq(efl_input_pointer_action_get(ev->info), EFL_POINTER_ACTION_CANCEL );
   ck_assert_int_eq(*called, 0);
   *called = 1;
}

static void
_check_ptr_cancel_up_event(void *data, const Efl_Event *ev)
{
   Eina_Bool *called = data;
   ck_assert_int_eq(efl_input_timestamp_get(ev->info), TIMESTAMP);
   ck_assert_int_eq(efl_input_pointer_wheel_delta_get(ev->info), 0);
   ck_assert_int_eq(efl_input_pointer_wheel_horizontal_get(ev->info), 0);
   ck_assert_int_eq(efl_input_pointer_double_click_get(ev->info), 0);
   ck_assert_int_eq(efl_input_pointer_triple_click_get(ev->info), 0);
   ck_assert_int_eq(efl_input_pointer_button_flags_get(ev->info), 0);
   ck_assert_int_eq(efl_input_pointer_touch_id_get(ev->info), 0);
   position_eq(efl_input_pointer_position_get(ev->info), EINA_POSITION2D(20, 20));
   ck_assert_int_eq(efl_input_pointer_button_get(ev->info), 1);
   ck_assert_int_eq(efl_input_pointer_action_get(ev->info), EFL_POINTER_ACTION_UP);
   ck_assert_int_eq(*called, 1);
   *called = 2;
}

EFL_START_TEST(efl_ui_win_test_efl_input_interface_pointer_cancel)
{
   Efl_Ui_Win *win;
   Eina_Bool called_down = EINA_FALSE;
   int called;
   Efl_Canvas_Object *rect;
   create_environment(&win, &rect);

   evas_event_feed_mouse_move(evas_object_evas_get(win), 20, 20, TIMESTAMP - 1, NULL);
   efl_event_callback_add(rect, EFL_EVENT_POINTER_DOWN, _check_ptr_cancel_down_event, &called_down);
   evas_event_feed_mouse_down(evas_object_evas_get(win), 1, 0, TIMESTAMP, NULL);
   ck_assert_int_eq(called_down, EINA_TRUE);
   called = EINA_FALSE;

   efl_event_callback_add(rect, EFL_EVENT_POINTER_CANCEL, _check_ptr_cancel_event, &called);
   efl_event_callback_add(rect, EFL_EVENT_POINTER_UP, _check_ptr_cancel_up_event, &called);
   evas_event_feed_mouse_cancel(evas_object_evas_get(win), TIMESTAMP, NULL);
   ck_assert_int_eq(called, 2);
}
EFL_END_TEST

static void
_count_event(void *data, const Efl_Event *ev EINA_UNUSED)
{
   int *count = data;

   (*count)++;
}

/* The window is a scene, so it says when a device comes and goes on its
 * canvas, as it already did for device,changed. */
EFL_START_TEST(efl_ui_win_test_device_added_removed)
{
   Efl_Ui_Win *win;
   Evas_Device *dev;
   int added = 0, removed = 0;

   win = efl_new(EFL_UI_WIN_CLASS);
   efl_event_callback_add(win, EFL_CANVAS_SCENE_EVENT_DEVICE_ADDED, _count_event, &added);
   efl_event_callback_add(win, EFL_CANVAS_SCENE_EVENT_DEVICE_REMOVED, _count_event, &removed);

   dev = evas_device_add_full(evas_object_evas_get(win), "test-seat", "a seat",
                              NULL, NULL, EVAS_DEVICE_CLASS_SEAT,
                              EVAS_DEVICE_SUBCLASS_NONE);
   ck_assert_ptr_ne(dev, NULL);
   ck_assert_int_eq(added, 1);

   evas_device_del(dev);
   ck_assert_int_eq(removed, 1);

   efl_event_callback_del(win, EFL_CANVAS_SCENE_EVENT_DEVICE_ADDED, _count_event, &added);
   efl_event_callback_del(win, EFL_CANVAS_SCENE_EVENT_DEVICE_REMOVED, _count_event, &removed);
   efl_unref(win);
}
EFL_END_TEST

/* Any angle is taken, and kept as the same angle between 0 and 359. */
EFL_START_TEST(efl_ui_win_test_rotation_normalized)
{
   Efl_Ui_Win *win;

   win = efl_new(EFL_UI_WIN_CLASS);
   efl_ui_win_rotation_set(win, 90);
   ck_assert_int_eq(efl_ui_win_rotation_get(win), 90);
   efl_ui_win_rotation_set(win, 360);
   ck_assert_int_eq(efl_ui_win_rotation_get(win), 0);
   efl_ui_win_rotation_set(win, -90);
   ck_assert_int_eq(efl_ui_win_rotation_get(win), 270);
   efl_ui_win_rotation_set(win, 450);
   ck_assert_int_eq(efl_ui_win_rotation_get(win), 90);
   efl_unref(win);
}
EFL_END_TEST

/* Loading an image as the window's background says it worked when it did. */
EFL_START_TEST(efl_ui_win_test_background_load)
{
   Efl_Ui_Win *win;

   win = efl_new(EFL_UI_WIN_CLASS);
   ck_assert(efl_file_simple_load(efl_part(win, "background"), ELM_IMAGE_DATA_DIR"/images/logo.png", NULL));
   ck_assert(efl_isa(efl_content_get(efl_part(win, "background")), EFL_GFX_IMAGE_INTERFACE));
   efl_unref(win);
}
EFL_END_TEST

EFL_START_TEST(efl_ui_win_test_type)
{
   Efl_Ui_Win *win;

   win = win_add(NULL, "win", EFL_UI_WIN_TYPE_BASIC);

   ck_assert_int_eq(efl_ui_win_type_get(win), EFL_UI_WIN_TYPE_BASIC);
}
EFL_END_TEST

EFL_START_TEST(efl_ui_win_test_content_replaced)
{
   Efl_Ui_Win *win;
   Eo *a, *b;

   win = win_add(NULL, "win", EFL_UI_WIN_TYPE_BASIC);
   a = efl_add(EFL_UI_BUTTON_CLASS, win);
   b = efl_add(EFL_UI_BUTTON_CLASS, win);

   efl_content_set(win, a);
   ck_assert_ptr_eq(efl_content_get(win), a);
   efl_content_set(win, b);
   ck_assert_ptr_eq(efl_content_get(win), b);

   efl_del(a);
   ck_assert_ptr_eq(efl_content_get(win), b);

   efl_content_set(win, NULL);
   ck_assert_ptr_eq(efl_content_get(win), NULL);
}
EFL_END_TEST

static void
_count_inc(void *data, const Efl_Event *ev EINA_UNUSED)
{
   int *count = data;
   (*count)++;
}

static void
_feed_render(Efl_Ui_Win *win)
{
   Evas *e = evas_object_evas_get(win);
   Eo *r = efl_add(EFL_CANVAS_RECTANGLE_CLASS, e);

   efl_gfx_entity_size_set(r, EINA_SIZE2D(10, 10));
   efl_gfx_entity_visible_set(r, EINA_TRUE);
   evas_render(e);
   efl_del(r);
   evas_render(e);
}

static void
_canvas_pointer_emit(Evas *e, const Efl_Event_Description *desc,
                     Efl_Pointer_Action action, int touch_id)
{
   Eo *p = efl_add_ref(EFL_INPUT_POINTER_CLASS, e);

   efl_input_pointer_action_set(p, action);
   efl_input_pointer_touch_id_set(p, touch_id);
   efl_input_pointer_position_set(p, EINA_POSITION2D(50, 50));
   efl_input_pointer_button_set(p, 1);
   efl_event_callback_call(e, desc, p);
   efl_unref(p);
}

static void
_canvas_key_emit(Evas *e, const Efl_Event_Description *desc, Eina_Bool down)
{
   Eo *k = efl_add_ref(EFL_INPUT_KEY_CLASS, e);

   efl_input_key_pressed_set(k, down);
   efl_input_key_name_set(k, "a");
   efl_input_key_sym_set(k, "a");
   efl_input_key_string_set(k, "a");
   efl_input_key_code_set(k, 0x61);
   efl_event_callback_call(e, desc, k);
   efl_unref(k);
}

EFL_START_TEST(efl_ui_win_test_event_added_at_construction)
{
   int render = 0, focus = 0, key = 0, pdown = 0, fdown = 0;
   Efl_Ui_Win *win;
   Ecore_Evas *ee;
   Evas *e;

   win = efl_add(EFL_UI_WIN_CLASS, efl_main_loop_get(),
                 efl_ui_win_type_set(efl_added, EFL_UI_WIN_TYPE_BASIC),
                 efl_event_callback_add(efl_added, EFL_CANVAS_SCENE_EVENT_RENDER_PRE, _count_inc, &render),
                 efl_event_callback_add(efl_added, EFL_CANVAS_SCENE_EVENT_SCENE_FOCUS_IN, _count_inc, &focus),
                 efl_event_callback_add(efl_added, EFL_EVENT_POINTER_DOWN, _count_inc, &pdown),
                 efl_event_callback_add(efl_added, EFL_EVENT_FINGER_DOWN, _count_inc, &fdown),
                 efl_event_callback_add(efl_added, EFL_EVENT_KEY_DOWN, _count_inc, &key));
   ck_assert_ptr_ne(win, NULL);
   efl_gfx_entity_size_set(win, EINA_SIZE2D(100, 100));
   e = evas_object_evas_get(win);
   ee = ecore_evas_ecore_evas_get(e);

   _feed_render(win);
   ck_assert_int_gt(render, 0);

   ecore_evas_focus_set(ee, EINA_FALSE);
   ecore_evas_focus_set(ee, EINA_TRUE);
   ck_assert_int_eq(focus, 1);

   _canvas_pointer_emit(e, EFL_EVENT_POINTER_DOWN, EFL_POINTER_ACTION_DOWN, 0);
   ck_assert_int_eq(pdown, 1);
   _canvas_pointer_emit(e, EFL_EVENT_FINGER_DOWN, EFL_POINTER_ACTION_DOWN, 1);
   ck_assert_int_eq(fdown, 1);

   /* emitted on the canvas, as ecore_evas does, with the window not focused */
   evas_object_focus_set(win, EINA_FALSE);
   _canvas_key_emit(e, EFL_EVENT_KEY_DOWN, EINA_TRUE);
   ck_assert_int_eq(key, 1);
}
EFL_END_TEST

EFL_START_TEST(efl_ui_win_test_event_add_del_before_finalize)
{
   int first = 0, second = 0;
   Efl_Ui_Win *win;
   Ecore_Evas *ee;

   win = efl_add(EFL_UI_WIN_CLASS, efl_main_loop_get(),
                 efl_ui_win_type_set(efl_added, EFL_UI_WIN_TYPE_BASIC),
                 efl_event_callback_add(efl_added, EFL_CANVAS_SCENE_EVENT_SCENE_FOCUS_IN, _count_inc, &first));
   ck_assert_ptr_ne(win, NULL);
   ee = ecore_evas_ecore_evas_get(evas_object_evas_get(win));

   /* removing a construction-time callback must keep the counter sane */
   efl_event_callback_del(win, EFL_CANVAS_SCENE_EVENT_SCENE_FOCUS_IN, _count_inc, &first);
   efl_event_callback_add(win, EFL_CANVAS_SCENE_EVENT_SCENE_FOCUS_IN, _count_inc, &second);

   ecore_evas_focus_set(ee, EINA_FALSE);
   ecore_evas_focus_set(ee, EINA_TRUE);
   ck_assert_int_eq(first, 0);
   ck_assert_int_eq(second, 1);
}
EFL_END_TEST

EFL_START_TEST(efl_ui_win_test_event_forward_fires_once)
{
   int pout = 0, fdown = 0, fup = 0, fmove = 0;
   Efl_Ui_Win *win;
   Evas *e;

   win = efl_add(EFL_UI_WIN_CLASS, efl_main_loop_get(),
                 efl_ui_win_type_set(efl_added, EFL_UI_WIN_TYPE_BASIC));
   e = evas_object_evas_get(win);

   efl_event_callback_add(win, EFL_EVENT_POINTER_OUT, _count_inc, &pout);
   efl_event_callback_add(win, EFL_EVENT_FINGER_DOWN, _count_inc, &fdown);
   efl_event_callback_add(win, EFL_EVENT_FINGER_UP, _count_inc, &fup);
   efl_event_callback_add(win, EFL_EVENT_FINGER_MOVE, _count_inc, &fmove);

   _canvas_pointer_emit(e, EFL_EVENT_POINTER_OUT, EFL_POINTER_ACTION_OUT, 0);
   _canvas_pointer_emit(e, EFL_EVENT_FINGER_DOWN, EFL_POINTER_ACTION_DOWN, 1);
   _canvas_pointer_emit(e, EFL_EVENT_FINGER_MOVE, EFL_POINTER_ACTION_MOVE, 1);
   _canvas_pointer_emit(e, EFL_EVENT_FINGER_UP, EFL_POINTER_ACTION_UP, 1);
   ck_assert_int_eq(pout, 1);
   ck_assert_int_eq(fdown, 1);
   ck_assert_int_eq(fmove, 1);
   ck_assert_int_eq(fup, 1);
}
EFL_END_TEST

EFL_START_TEST(efl_ui_win_test_event_forward_no_leak)
{
   static const Efl_Event_Description *descs[4];
   unsigned int before[4], i;
   Efl_Ui_Win *win;
   Evas *e;

   descs[0] = EFL_EVENT_POINTER_DOWN;
   descs[1] = EFL_EVENT_POINTER_MOVE;
   descs[2] = EFL_EVENT_FINGER_DOWN;
   descs[3] = EFL_EVENT_KEY_DOWN;

   win = efl_add_ref(EFL_UI_WIN_CLASS, efl_main_loop_get(),
                     efl_ui_win_type_set(efl_added, EFL_UI_WIN_TYPE_BASIC));
   e = evas_object_evas_get(win);
   efl_ref(e);

   /* nobody listens on the window: the canvas must not be forwarding.
    * Compare with a window nobody touched after creation. */
   for (i = 0; i < 4; i++)
     before[i] = efl_event_callback_count(e, descs[i]);
   {
      int n = 0;
      efl_event_callback_add(win, descs[0], _count_inc, &n);
      efl_event_callback_add(win, descs[2], _count_inc, &n);
      /* one forwarder for the user's handler, whatever the window itself
       * registers for */
      ck_assert_int_eq(efl_event_callback_count(e, descs[2]), before[2] + 1);
      efl_event_callback_del(win, descs[0], _count_inc, &n);
      efl_event_callback_del(win, descs[2], _count_inc, &n);
   }
   for (i = 0; i < 4; i++)
     ck_assert_int_eq(efl_event_callback_count(e, descs[i]), before[i]);

   efl_unref(e);
   efl_unref(win);
}
EFL_END_TEST

// As Efl.Content says: a content that replaces another one removes it first
// (it is hidden, the slot is the new one's, and deleting it later leaves the
// slot alone), and the content that is unset is given back alive and alone,
// holding nothing of the window's (as a layout does), so that deleting it
// frees it.
EFL_START_TEST(efl_ui_win_test_content_replace_and_unset)
{
   Efl_Ui_Win *win;
   Eo *a, *b, *c, *gone = NULL, *got = NULL;

   win = efl_new(EFL_UI_WIN_CLASS);
   a = efl_add(EFL_UI_BUTTON_CLASS, win);
   b = efl_add(EFL_UI_BUTTON_CLASS, win);
   c = efl_add(EFL_UI_BUTTON_CLASS, win);
   efl_wref_add(a, &gone);
   efl_wref_add(c, &got);

   ck_assert(efl_content_set(win, a));
   ck_assert_ptr_eq(efl_content_get(win), a);
   ck_assert(efl_content_set(win, b));
   ck_assert_ptr_eq(efl_content_get(win), b);
   ck_assert_ptr_ne(gone, NULL);
   ck_assert(!efl_gfx_entity_visible_get(a));
   efl_del(a);
   ck_assert_ptr_eq(gone, NULL);
   ck_assert_ptr_eq(efl_content_get(win), b);

   ck_assert(efl_content_set(win, c));
   ck_assert_ptr_eq(efl_content_get(win), c);
   ck_assert_ptr_eq(efl_content_unset(win), c);
   ck_assert_ptr_eq(efl_content_get(win), NULL);
   // alive, and nobody has a reference but its parent
   ck_assert_ptr_ne(got, NULL);
   ck_assert_int_eq(efl_ref_count(c), 1);
   ck_assert_ptr_ne(efl_parent_get(c), win);
   efl_del(c);
   ck_assert_ptr_eq(got, NULL);

   // and a window that has none gives none
   ck_assert_ptr_eq(efl_content_unset(win), NULL);
   efl_unref(win);
}
EFL_END_TEST

void
efl_ui_test_win(TCase *tc)
{
   tcase_add_test(tc, efl_ui_win_test_event_added_at_construction);
   tcase_add_test(tc, efl_ui_win_test_event_add_del_before_finalize);
   tcase_add_test(tc, efl_ui_win_test_event_forward_fires_once);
   tcase_add_test(tc, efl_ui_win_test_event_forward_no_leak);
   tcase_add_test(tc, efl_ui_win_test_scene_focus);
   tcase_add_test(tc, efl_ui_win_test_object_focus);
   tcase_add_test(tc, efl_ui_win_test_object_focus);
   tcase_add_test(tc, efl_ui_win_test_efl_input_interface_focus);
   tcase_add_test(tc, efl_ui_win_test_efl_input_interface_key_down);
   tcase_add_test(tc, efl_ui_win_test_efl_input_interface_pointer_move);
   tcase_add_test(tc, efl_ui_win_test_efl_input_interface_pointer_down);
   tcase_add_test(tc, efl_ui_win_test_efl_input_interface_pointer_up);
   tcase_add_test(tc, efl_ui_win_test_efl_input_interface_pointer_in_out);
   tcase_add_test(tc, efl_ui_win_test_efl_input_interface_pointer_wheel);
   tcase_add_test(tc, efl_ui_win_test_efl_input_interface_pointer_cancel);
   tcase_add_test(tc, efl_ui_win_test_background_load);
   tcase_add_test(tc, efl_ui_win_test_device_added_removed);
   tcase_add_test(tc, efl_ui_win_test_rotation_normalized);
   tcase_add_test(tc, efl_ui_win_test_type);
   tcase_add_test(tc, efl_ui_win_test_content_replaced);
   tcase_add_test(tc, efl_ui_win_test_content_replace_and_unset);
}
