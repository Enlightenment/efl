#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>

#define EFL_INTERNAL_UNSTABLE
#include <Evas.h>
#include <Evas_Internal.h>
#include <Ecore_Evas.h>

#include "evas_suite.h"
#include "evas_tests_helpers.h"

static void
_mouse_down_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   int *val = data;
   ck_assert_int_eq(*val, 0);
   *val = 1;
}

EFL_START_TEST(evas_test_events_frozen_mouse_up)
{
   Evas *evas;
   Evas_Object *rect;
   static int callback_called = 0;
   evas = EVAS_TEST_INIT_EVAS();
   rect = evas_object_rectangle_add(evas);
   evas_object_event_callback_add(rect, EVAS_CALLBACK_MOUSE_DOWN, _mouse_down_cb, &callback_called);
   evas_object_event_callback_add(rect, EVAS_CALLBACK_MOUSE_UP, _mouse_down_cb, &callback_called);
   evas_object_resize(rect, 500, 500);
   evas_object_show(rect);

   evas_event_feed_mouse_in(evas, 0, NULL);
   evas_event_feed_mouse_move(evas, 100, 100, 0, NULL);
   evas_event_feed_mouse_down(evas, 1, 0, 0, NULL);
   ck_assert_int_eq(callback_called, 1);
   callback_called = 0;
   evas_object_freeze_events_set(rect, 1);
   evas_event_feed_mouse_up(evas, 1, 0, 0, NULL);
   ck_assert_int_eq(callback_called, 0);
}
EFL_END_TEST

static void _touch_feed(Evas *evas, Efl_Input_Device *dev, Efl_Pointer_Action action, int id, double x, double y);

/* A seat with a mouse of its own, so that it has pointer data to feed. */
typedef struct
{
   Efl_Input_Device *seat;
   Efl_Input_Device *mouse;
} Test_Seat;

static void
_test_seat_add(Evas *evas, Test_Seat *ts, const char *name)
{
   ts->seat = evas_device_add_full(evas, name, "a test seat", NULL, NULL,
                                   EVAS_DEVICE_CLASS_SEAT,
                                   EVAS_DEVICE_SUBCLASS_NONE);
   ck_assert_ptr_ne(ts->seat, NULL);
   ts->mouse = evas_device_add_full(evas, name, "a test mouse", ts->seat, NULL,
                                    EVAS_DEVICE_CLASS_MOUSE,
                                    EVAS_DEVICE_SUBCLASS_NONE);
   ck_assert_ptr_ne(ts->mouse, NULL);
   /* a pointer outside the canvas gets no move events */
   _touch_feed(evas, ts->mouse, EFL_POINTER_ACTION_IN, 0, 0, 0);
}

static void
_touch_feed(Evas *evas, Efl_Input_Device *dev, Efl_Pointer_Action action,
            int id, double x, double y)
{
   const Efl_Event_Description *desc;
   Efl_Input_Pointer *ptr;

   switch (action)
     {
      case EFL_POINTER_ACTION_DOWN: desc = EFL_EVENT_POINTER_DOWN; break;
      case EFL_POINTER_ACTION_IN: desc = EFL_EVENT_POINTER_IN; break;
      case EFL_POINTER_ACTION_UP: desc = EFL_EVENT_POINTER_UP; break;
      case EFL_POINTER_ACTION_CANCEL: desc = EFL_EVENT_POINTER_CANCEL; break;
      default: desc = EFL_EVENT_POINTER_MOVE; break;
     }

   ptr = efl_add(EFL_INPUT_POINTER_CLASS, evas);
   efl_input_device_set(ptr, dev);
   efl_input_pointer_touch_id_set(ptr, id);
   efl_input_pointer_position_set(ptr, EINA_POSITION2D(x, y));
   efl_input_pointer_action_set(ptr, action);
   efl_event_callback_call(evas, desc, ptr);
   efl_del(ptr);
}

static void
_touch_check(Evas *evas, Efl_Input_Device *seat, unsigned int n, int id,
             double x, double y)
{
   Evas_Touch_Point_State state;
   double px, py;
   int pid;

   ck_assert(evas_canvas_seat_touch_point_nth_get(evas, seat, n, &pid, &state,
                                                  &px, &py));
   ck_assert_int_eq(pid, id);
   ck_assert_msg(px == x, "n=%u id=%d px=%f x=%f", n, id, px, x);
   ck_assert(py == y);
}

/* The same touch id on two seats is two fingers, and each one is found,
 * moved and removed by its own seat only. */
EFL_START_TEST(evas_test_events_touch_seats_distinct)
{
   Evas *evas = EVAS_TEST_INIT_EVAS();
   Test_Seat a, b;

   _test_seat_add(evas, &a, "seat-a");
   _test_seat_add(evas, &b, "seat-b");

   _touch_feed(evas, a.mouse, EFL_POINTER_ACTION_DOWN, 1, 10, 10);
   _touch_feed(evas, b.mouse, EFL_POINTER_ACTION_DOWN, 1, 50, 50);
   _touch_feed(evas, b.mouse, EFL_POINTER_ACTION_DOWN, 2, 80, 80);

   /* the legacy list spans every seat */
   ck_assert_int_eq(evas_touch_point_list_count(evas), 3);
   ck_assert_int_eq(evas_canvas_seat_touch_point_count(evas, a.seat), 1);
   ck_assert_int_eq(evas_canvas_seat_touch_point_count(evas, b.seat), 2);
   _touch_check(evas, a.seat, 0, 1, 10, 10);
   _touch_check(evas, b.seat, 0, 1, 50, 50);
   _touch_check(evas, b.seat, 1, 2, 80, 80);

   /* moving one of them leaves the other seat's finger alone */
   _touch_feed(evas, a.mouse, EFL_POINTER_ACTION_MOVE, 1, 20, 30);
   _touch_check(evas, a.seat, 0, 1, 20, 30);
   _touch_check(evas, b.seat, 0, 1, 50, 50);

   /* releasing it removes that finger and no other */
   _touch_feed(evas, a.mouse, EFL_POINTER_ACTION_UP, 1, 20, 30);
   ck_assert_int_eq(evas_touch_point_list_count(evas), 2);
   ck_assert_int_eq(evas_canvas_seat_touch_point_count(evas, a.seat), 0);
   ck_assert_int_eq(evas_canvas_seat_touch_point_count(evas, b.seat), 2);
   _touch_check(evas, b.seat, 0, 1, 50, 50);

   _touch_feed(evas, b.mouse, EFL_POINTER_ACTION_UP, 1, 50, 50);
   _touch_feed(evas, b.mouse, EFL_POINTER_ACTION_UP, 2, 80, 80);
   ck_assert_int_eq(evas_touch_point_list_count(evas), 0);

   ecore_evas_free(ecore_evas_ecore_evas_get(evas));
}
EFL_END_TEST

/* An event without a device is the default seat's. */
EFL_START_TEST(evas_test_events_touch_default_seat)
{
   Evas *evas = EVAS_TEST_INIT_EVAS();
   Test_Seat a;

   _test_seat_add(evas, &a, "seat-a");

   _touch_feed(evas, NULL, EFL_POINTER_ACTION_DOWN, 1, 10, 10);
   _touch_feed(evas, a.mouse, EFL_POINTER_ACTION_DOWN, 1, 50, 50);

   ck_assert_int_eq(evas_touch_point_list_count(evas), 2);
   ck_assert_int_eq(evas_canvas_seat_touch_point_count(evas, NULL), 1);
   ck_assert_int_eq(evas_canvas_seat_touch_point_count(evas, efl_canvas_scene_seat_default_get(evas)), 1);
   _touch_check(evas, NULL, 0, 1, 10, 10);
   _touch_check(evas, a.seat, 0, 1, 50, 50);

   _touch_feed(evas, NULL, EFL_POINTER_ACTION_UP, 1, 10, 10);
   _touch_feed(evas, a.mouse, EFL_POINTER_ACTION_UP, 1, 50, 50);
   ck_assert_int_eq(evas_touch_point_list_count(evas), 0);

   ecore_evas_free(ecore_evas_ecore_evas_get(evas));
}
EFL_END_TEST

/* A seat that goes away while it still has fingers down takes them with it,
 * and the other seats keep theirs. */
EFL_START_TEST(evas_test_events_touch_seat_removed)
{
   Evas *evas = EVAS_TEST_INIT_EVAS();
   Test_Seat a, b;

   _test_seat_add(evas, &a, "seat-a");
   _test_seat_add(evas, &b, "seat-b");

   _touch_feed(evas, a.mouse, EFL_POINTER_ACTION_DOWN, 1, 10, 10);
   _touch_feed(evas, b.mouse, EFL_POINTER_ACTION_DOWN, 1, 50, 50);
   _touch_feed(evas, b.mouse, EFL_POINTER_ACTION_DOWN, 2, 80, 80);
   ck_assert_int_eq(evas_touch_point_list_count(evas), 3);

   evas_device_del(b.mouse);
   evas_device_del(b.seat);
   ck_assert_int_eq(evas_touch_point_list_count(evas), 1);
   ck_assert_int_eq(evas_touch_point_list_nth_id_get(evas, 0), 1);
   _touch_check(evas, a.seat, 0, 1, 10, 10);

   _touch_feed(evas, a.mouse, EFL_POINTER_ACTION_UP, 1, 10, 10);
   ck_assert_int_eq(evas_touch_point_list_count(evas), 0);

   ecore_evas_free(ecore_evas_ecore_evas_get(evas));
}
EFL_END_TEST

/* Cancelling the pointer of one seat releases the fingers of that seat only. */
EFL_START_TEST(evas_test_events_touch_cancel_one_seat)
{
   Evas *evas = EVAS_TEST_INIT_EVAS();
   Test_Seat a, b;

   _test_seat_add(evas, &a, "seat-a");
   _test_seat_add(evas, &b, "seat-b");

   /* the other seat's finger comes first in the list, so a cancel that walks
    * every point meets it before the fingers of the seat being cancelled */
   _touch_feed(evas, b.mouse, EFL_POINTER_ACTION_DOWN, 3, 80, 80);
   _touch_feed(evas, a.mouse, EFL_POINTER_ACTION_DOWN, 1, 10, 10);
   _touch_feed(evas, a.mouse, EFL_POINTER_ACTION_DOWN, 2, 50, 50);

   _touch_feed(evas, a.mouse, EFL_POINTER_ACTION_CANCEL, 0, 0, 0);
   ck_assert_int_eq(evas_canvas_seat_touch_point_count(evas, a.seat), 0);
   ck_assert_int_eq(evas_canvas_seat_touch_point_count(evas, b.seat), 1);
   ck_assert_int_eq(evas_touch_point_list_count(evas), 1);
   _touch_check(evas, b.seat, 0, 3, 80, 80);

   ecore_evas_free(ecore_evas_ecore_evas_get(evas));
}
EFL_END_TEST

void evas_test_events(TCase *tc)
{
   tcase_add_test(tc, evas_test_events_frozen_mouse_up);
   tcase_add_test(tc, evas_test_events_touch_seats_distinct);
   tcase_add_test(tc, evas_test_events_touch_default_seat);
   tcase_add_test(tc, evas_test_events_touch_seat_removed);
   tcase_add_test(tc, evas_test_events_touch_cancel_one_seat);
}
