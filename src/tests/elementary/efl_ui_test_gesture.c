#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Efl_Ui.h>
#include "efl_ui_suite.h"
/* mouse feeding */
#include <Evas_Legacy.h>
#include <evas_canvas_eo.h>

#include "custom_gesture.eo.h"
#include "custom_recognizer.eo.h"
#include "custom_recognizer2.eo.h"

/*
typedef enum
{
  EFL_GESTURE_STATE_NONE = 0,
  EFL_GESTURE_STATE_STARTED = 1,
  EFL_GESTURE_STATE_UPDATED,
  EFL_GESTURE_STATE_FINISHED,
  EFL_GESTURE_STATE_CANCELED
} Efl_Canvas_Gesture_State;
*/

enum
{
   TAP,
   LONG_PRESS,
   DOUBLE_TAP,
   TRIPLE_TAP,
   MOMENTUM,
   FLICK,
   ROTATE,
   ZOOM,
   CUSTOM,
   CUSTOM2,
   LAST
};

static int count[LAST][4] = {0};

/* macros to simplify checking gesture counts */
#define CHECK_START(type, val) \
   ck_assert_int_eq(count[(type)][EFL_GESTURE_STATE_STARTED - 1], (val))
#define CHECK_UPDATE(type, val) \
   ck_assert_int_eq(count[(type)][EFL_GESTURE_STATE_UPDATED - 1], (val))
#define CHECK_FINISH(type, val) \
   ck_assert_int_eq(count[(type)][EFL_GESTURE_STATE_FINISHED - 1], (val))
#define CHECK_CANCEL(type, val) \
   ck_assert_int_eq(count[(type)][EFL_GESTURE_STATE_CANCELED - 1], (val))
#define CHECK_ALL(type, ...) \
  do {\
    int state_vals[] = {__VA_ARGS__}; \
    for (int i = 0; i < 4; i++) \
      ck_assert_int_eq(count[(type)][i], state_vals[i]); \
  } while (0)
#define CHECK_NONZERO(type) \
  do {\
    for (int i = 0; i < 4; i++) \
      ck_assert_int_ne(count[(type)][i], 0); \
  } while (0)
#define CHECK_ZERO(type) CHECK_ALL((type), 0, 0, 0, 0)
#define RESET memset(count, 0, sizeof(count))

static void
gesture_cb(void *data , const Efl_Event *ev)
{
   Efl_Canvas_Gesture *g = ev->info;
   int *count = data;
   /* increment counter for event state which has been processed */
   count[efl_gesture_state_get(g) - 1]++;
}

static Eo *
setup(void)
{
   Eo *win, *rect;

   RESET;

   win = win_add();
   efl_gfx_entity_size_set(win, EINA_SIZE2D(1000, 1000));

   rect = efl_add(EFL_CANVAS_RECTANGLE_CLASS, win);
   efl_content_set(win, rect);

#define WATCH(type) \
   efl_event_callback_add(rect, EFL_EVENT_GESTURE_##type, gesture_cb, &count[(type)])
   WATCH(TAP);
   WATCH(LONG_PRESS);
   WATCH(DOUBLE_TAP);
   WATCH(TRIPLE_TAP);
   WATCH(MOMENTUM);
   WATCH(FLICK);
   WATCH(ROTATE);
   WATCH(ZOOM);

   get_me_to_those_events(win);
   return rect;
}

EFL_START_TEST(test_efl_ui_gesture_taps)
{
   Eo *rect = setup();

   /* basic tap */
   click_object(rect);
   CHECK_ALL(TAP, 1, 0, 1, 0);
   CHECK_ALL(LONG_PRESS, 1, 0, 0, 1);
   CHECK_ALL(DOUBLE_TAP, 1, 1, 0, 0);
   CHECK_ALL(TRIPLE_TAP, 1, 1, 0, 0);
   CHECK_ZERO(MOMENTUM);
   CHECK_ZERO(FLICK);
   CHECK_ZERO(ROTATE);
   CHECK_ZERO(ZOOM);

   RESET;

   /* add a second tap */
   click_object(rect);
   CHECK_ALL(TAP, 1, 0, 1, 0);
   CHECK_ALL(LONG_PRESS, 1, 0, 0, 1);
   /* UPDATE -> FINISH */
   CHECK_ALL(DOUBLE_TAP, 0, 1, 1, 0);
   CHECK_ALL(TRIPLE_TAP, 0, 2, 0, 0);
   CHECK_ZERO(MOMENTUM);
   CHECK_ZERO(FLICK);
   CHECK_ZERO(ROTATE);
   CHECK_ZERO(ZOOM);

   RESET;

   /* add a third tap */
   click_object(rect);
   CHECK_ALL(TAP, 1, 0, 1, 0);
   CHECK_ALL(LONG_PRESS, 1, 0, 0, 1);
   /* UPDATE -> FINISH */
   CHECK_ALL(DOUBLE_TAP, 1, 1, 0, 0);
   CHECK_ALL(TRIPLE_TAP, 0, 1, 1, 0);
   CHECK_ZERO(MOMENTUM);
   CHECK_ZERO(FLICK);
   CHECK_ZERO(ROTATE);
   CHECK_ZERO(ZOOM);

   /* clear states */
   wait_timer(0.4);
   RESET;

   /* verify finger size */
   click_object_at(rect, 500, 500);
   click_object_at(rect, 505, 505);
   CHECK_ALL(TAP, 2, 0, 2, 0);
   CHECK_ALL(LONG_PRESS, 2, 0, 0, 2);
   /* UPDATE -> FINISH */
   CHECK_ALL(DOUBLE_TAP, 1, 2, 1, 0);
   CHECK_ALL(TRIPLE_TAP, 1, 3, 0, 0);
   CHECK_ZERO(MOMENTUM);
   CHECK_ZERO(FLICK);
   CHECK_ZERO(ROTATE);
   CHECK_ZERO(ZOOM);

   /* clear states */
   wait_timer(0.4);
   RESET;

   /* verify multiple simultaneous presses treated as same press */
   multi_click_object(rect, 2);
   CHECK_ALL(TAP, 1, 0, 1, 0);
   CHECK_ALL(LONG_PRESS, 1, 0, 0, 1);
   CHECK_ALL(DOUBLE_TAP, 1, 1, 0, 0);
   CHECK_ALL(TRIPLE_TAP, 1, 1, 0, 0);
   CHECK_ZERO(MOMENTUM);
   CHECK_ZERO(FLICK);
   /* this is two fingers, so we have a rotate start */
   CHECK_ALL(ROTATE, 1, 0, 0, 1);
   /* this is two fingers, so we have a zoom start */
   CHECK_ALL(ZOOM, 1, 0, 0, 1);

   RESET;

   multi_click_object(rect, 2);
   CHECK_ALL(TAP, 1, 0, 1, 0);
   CHECK_ALL(LONG_PRESS, 1, 0, 0, 1);
   /* UPDATE -> FINISH */
   CHECK_ALL(DOUBLE_TAP, 0, 1, 1, 0);
   CHECK_ALL(TRIPLE_TAP, 0, 2, 0, 0);
   CHECK_ZERO(MOMENTUM);
   CHECK_ZERO(FLICK);
   /* this is two fingers, so we have a rotate start */
   CHECK_ALL(ROTATE, 1, 0, 0, 1);
   /* this is two fingers, so we have a zoom start */
   CHECK_ALL(ZOOM, 1, 0, 0, 1);

   RESET;

   multi_click_object(rect, 2);
   CHECK_ALL(TAP, 1, 0, 1, 0);
   CHECK_ALL(LONG_PRESS, 1, 0, 0, 1);
   /* UPDATE -> FINISH */
   CHECK_ALL(DOUBLE_TAP, 1, 1, 0, 0);
   CHECK_ALL(TRIPLE_TAP, 0, 1, 1, 0);
   CHECK_ZERO(MOMENTUM);
   CHECK_ZERO(FLICK);
   /* this is two fingers, so we have a rotate start */
   CHECK_ALL(ROTATE, 1, 0, 0, 1);
   /* this is two fingers, so we have a zoom start */
   CHECK_ALL(ZOOM, 1, 0, 0, 1);
   /* clear states */
   wait_timer(0.4);
   RESET;

   multi_click_object(rect, 10);
   CHECK_ALL(TAP, 1, 0, 1, 0);
   CHECK_ALL(LONG_PRESS, 1, 0, 0, 1);
   CHECK_ALL(DOUBLE_TAP, 1, 0, 0, 1);
   CHECK_ALL(TRIPLE_TAP, 1, 0, 0, 1);
   CHECK_ZERO(MOMENTUM);
   CHECK_ZERO(FLICK);
   /* this is two fingers, so we have a rotate start */
   CHECK_ALL(ROTATE, 1, 0, 0, 1);
   /* this is two fingers, so we have a zoom start */
   CHECK_ALL(ZOOM, 1, 0, 0, 1);
   RESET;

}
EFL_END_TEST

EFL_START_TEST(test_efl_ui_gesture_long_press)
{
   Eo *rect = setup();
   double timeout = 1.2;
   Eina_Value *val;
   Eo *e = efl_provider_find(rect, EVAS_CANVAS_CLASS);

   val = efl_config_get(efl_provider_find(rect, EFL_CONFIG_INTERFACE), "glayer_long_tap_start_timeout");
   eina_value_get(val, &timeout);

   /* press */
   press_object(rect);
   CHECK_ALL(TAP, 1, 0, 0, 0);
   CHECK_ALL(LONG_PRESS, 1, 0, 0, 0);
   CHECK_ALL(DOUBLE_TAP, 1, 0, 0, 0);
   CHECK_ALL(TRIPLE_TAP, 1, 0, 0, 0);
   CHECK_ZERO(MOMENTUM);
   CHECK_ZERO(FLICK);
   CHECK_ZERO(ROTATE);
   CHECK_ZERO(ZOOM);

   RESET;

   wait_timer(timeout + 0.01);

   /* verify longpress */
   CHECK_ALL(TAP, 0, 0, 0, 1);
   CHECK_ALL(LONG_PRESS, 0, 1, 0, 0);
   CHECK_ALL(DOUBLE_TAP, 0, 0, 0, 1);
   CHECK_ALL(TRIPLE_TAP, 0, 0, 0, 1);
   CHECK_ZERO(MOMENTUM);
   CHECK_ZERO(FLICK);
   CHECK_ZERO(ROTATE);
   CHECK_ZERO(ZOOM);

   RESET;
   evas_event_feed_mouse_up(e, 1, 0, 2, NULL);

   CHECK_ZERO(TAP);
   CHECK_ALL(LONG_PRESS, 0, 0, 1, 0);
   CHECK_ZERO(DOUBLE_TAP);
   CHECK_ZERO(TRIPLE_TAP);
   CHECK_ZERO(MOMENTUM);
   CHECK_ZERO(FLICK);
   CHECK_ZERO(ROTATE);
   CHECK_ZERO(ZOOM);

   RESET;

   press_object_at(rect, 0, 0);
   RESET;

   /* move off-canvas */
   evas_event_feed_mouse_move(e, -1, 0, 2, NULL);
   wait_timer(timeout + 0.01);

   /* verify longpress */
   CHECK_ALL(TAP, 0, 1, 0, 0);
   CHECK_ALL(LONG_PRESS, 0, 1, 0, 0);
   CHECK_ALL(DOUBLE_TAP, 0, 0, 0, 1);
   CHECK_ALL(TRIPLE_TAP, 0, 0, 0, 1);
   CHECK_ALL(MOMENTUM, 1, 0, 0, 0);
   CHECK_ALL(FLICK, 1, 0, 0, 0);
   CHECK_ZERO(ROTATE);
   CHECK_ZERO(ZOOM);

   RESET;
   evas_event_feed_mouse_up(e, 1, 0, 3, NULL);
}
EFL_END_TEST

EFL_START_TEST(test_efl_ui_gesture_flick)
{
   int moves, i, single = 0;
   Eo *rect = setup();

   /* add extra random cb to verify that we get exactly 1 event */
   efl_event_callback_add(rect, EFL_EVENT_GESTURE_MOMENTUM, (void*)event_callback_that_increments_an_int_when_called, &single);

   /* basic flick */
   drag_object(rect, 0, 0, 75, 0, EINA_FALSE);
   /* canceled */
   CHECK_ALL(TAP, 1, 0, 0, 1);
   /* canceled */
   CHECK_ALL(LONG_PRESS, 1, 0, 0, 1);
   /* canceled */
   CHECK_ALL(DOUBLE_TAP, 1, 0, 0, 1);
   /* canceled */
   CHECK_ALL(TRIPLE_TAP, 1, 0, 0, 1);
   /* updated but canceled */
   CHECK_ALL(MOMENTUM, 1, DRAG_OBJECT_NUM_MOVES - 1, 0, 1);
   /* triggered */
   CHECK_ALL(FLICK, 1, DRAG_OBJECT_NUM_MOVES - 1, 1, 0);
   CHECK_ZERO(ROTATE);
   CHECK_ZERO(ZOOM);

   RESET;

   efl_event_callback_del(rect, EFL_EVENT_GESTURE_MOMENTUM, (void*)event_callback_that_increments_an_int_when_called, &single);
   ck_assert_int_ge(single, 0);

   /* reverse flick */
   drag_object(rect, 75, 0, -75, 0, EINA_FALSE);
   /* canceled */
   CHECK_ALL(TAP, 1, 0, 0, 1);
   /* canceled */
   CHECK_ALL(LONG_PRESS, 1, 0, 0, 1);
   /* canceled */
   CHECK_ALL(DOUBLE_TAP, 1, 0, 0, 1);
   /* canceled */
   CHECK_ALL(TRIPLE_TAP, 1, 0, 0, 1);
   /* updated but canceled */
   CHECK_ALL(MOMENTUM, 1, DRAG_OBJECT_NUM_MOVES - 1, 0, 1);
   /* triggered */
   CHECK_ALL(FLICK, 1, DRAG_OBJECT_NUM_MOVES - 1, 1, 0);
   CHECK_ZERO(ROTATE);
   CHECK_ZERO(ZOOM);

   RESET;

   /* vertical flick */
   drag_object(rect, 0, 0, 0, 75, EINA_FALSE);
   /* canceled */
   CHECK_ALL(TAP, 1, 0, 0, 1);
   /* canceled */
   CHECK_ALL(LONG_PRESS, 1, 0, 0, 1);
   /* canceled */
   CHECK_ALL(DOUBLE_TAP, 1, 0, 0, 1);
   /* canceled */
   CHECK_ALL(TRIPLE_TAP, 1, 0, 0, 1);
   /* updated but canceled */
   CHECK_ALL(MOMENTUM, 1, DRAG_OBJECT_NUM_MOVES - 1, 0, 1);
   /* triggered */
   CHECK_ALL(FLICK, 1, DRAG_OBJECT_NUM_MOVES - 1, 1, 0);
   CHECK_ZERO(ROTATE);
   CHECK_ZERO(ZOOM);

   RESET;

   /* reverse vertical flick */
   drag_object(rect, 0, 75, 0, -75, EINA_FALSE);
   /* canceled */
   CHECK_ALL(TAP, 1, 0, 0, 1);
   /* canceled */
   CHECK_ALL(LONG_PRESS, 1, 0, 0, 1);
   /* canceled */
   CHECK_ALL(DOUBLE_TAP, 1, 0, 0, 1);
   /* canceled */
   CHECK_ALL(TRIPLE_TAP, 1, 0, 0, 1);
   /* updated but canceled */
   CHECK_ALL(MOMENTUM, 1, DRAG_OBJECT_NUM_MOVES - 1, 0, 1);
   /* triggered */
   CHECK_ALL(FLICK, 1, DRAG_OBJECT_NUM_MOVES - 1, 1, 0);
   CHECK_ZERO(ROTATE);
   CHECK_ZERO(ZOOM);

   RESET;


   /* diagonal flick */
   drag_object(rect, 0, 0, 75, 75, EINA_FALSE);
   /* canceled */
   CHECK_ALL(TAP, 1, 0, 0, 1);
   /* canceled */
   CHECK_ALL(LONG_PRESS, 1, 0, 0, 1);
   /* canceled */
   CHECK_ALL(DOUBLE_TAP, 1, 0, 0, 1);
   /* canceled */
   CHECK_ALL(TRIPLE_TAP, 1, 0, 0, 1);
   /* updated but canceled */
   CHECK_ALL(MOMENTUM, 1, DRAG_OBJECT_NUM_MOVES - 1, 0, 1);
   /* triggered */
   CHECK_ALL(FLICK, 1, DRAG_OBJECT_NUM_MOVES - 1, 1, 0);
   CHECK_ZERO(ROTATE);
   CHECK_ZERO(ZOOM);

   RESET;

   /* off-canvas flick */
   drag_object(rect, 999, 0, 50, 0, EINA_FALSE);
   /* canceled */
   CHECK_ALL(TAP, 1, 0, 0, 1);
   /* canceled */
   CHECK_ALL(LONG_PRESS, 1, 0, 0, 1);
   /* canceled */
   CHECK_ALL(DOUBLE_TAP, 1, 0, 0, 1);
   /* canceled */
   CHECK_ALL(TRIPLE_TAP, 1, 0, 0, 1);
   CHECK_START(MOMENTUM, 1);
   CHECK_FINISH(MOMENTUM, 0);
   CHECK_CANCEL(MOMENTUM, 1);
   CHECK_START(FLICK, 1);
   CHECK_FINISH(FLICK, 1);
   CHECK_CANCEL(FLICK, 0);
   CHECK_ZERO(ROTATE);
   CHECK_ZERO(ZOOM);

   RESET;

   /* definitely not a flick */
   moves = drag_object_around(rect, 500, 500, 450, 180);
   for (i = 0; i <= TRIPLE_TAP; i++)
     {
        /* canceled */
        CHECK_START(TAP, 1);
        CHECK_CANCEL(TAP, 1);
     }
   /* completed: a momentum gesture is any completed motion */
   CHECK_ALL(MOMENTUM, 1, moves - 2, 1, 0);
   /* NOT triggered; this is going to have some crazy number of update events since it ignores a bunch */
   CHECK_FINISH(FLICK, 0);
   CHECK_ZERO(ROTATE);
   CHECK_ZERO(ZOOM);

   RESET;

   /* definitely not a flick, also outside canvas */
   moves = drag_object_around(rect, 25, 50, 50, 180);
   for (i = 0; i <= TRIPLE_TAP; i++)
     {
        /* canceled */
        CHECK_START(TAP, 1);
        CHECK_CANCEL(TAP, 1);
     }
   /* momentum should only begin at the initial press or if canceled due to timeout */
   CHECK_START(MOMENTUM, 1);
   CHECK_FINISH(MOMENTUM, 1);
   /* canceled: the motion ends outside the canvas, so there is no momentum */
   CHECK_CANCEL(MOMENTUM, 0);

   /* flick checks a tolerance value for straight lines, so "start" will be >= 1 */
   ck_assert_int_ge(count[FLICK][EFL_GESTURE_STATE_STARTED - 1], 1);
   CHECK_FINISH(FLICK, 0);
   /* flick checks a tolerance value for straight lines, so "start" will be >= 1 */
   ck_assert_int_ge(count[FLICK][EFL_GESTURE_STATE_CANCELED - 1], 1);
   CHECK_ZERO(ROTATE);
   CHECK_ZERO(ZOOM);

   RESET;

   /* definitely not a flick, test re-entering canvas */
   moves = drag_object_around(rect, 500, 750, 400, 180);
   for (i = 0; i <= TRIPLE_TAP; i++)
     {
        /* canceled */
        CHECK_START(TAP, 1);
        CHECK_CANCEL(TAP, 1);
     }
   /* momentum should only begin at the initial press or if canceled due to timeout */
   CHECK_START(MOMENTUM, 1);
   /* finished: the motion ends outside the canvas, but we still count it */
   CHECK_FINISH(MOMENTUM, 1);
   CHECK_CANCEL(MOMENTUM, 0);

   /* flick checks a tolerance value for straight lines, so "start" will be >= 1 */
   ck_assert_int_ge(count[FLICK][EFL_GESTURE_STATE_STARTED - 1], 1);
   CHECK_FINISH(FLICK, 0);
   /* flick checks a tolerance value for straight lines, so "start" will be >= 1 */
   ck_assert_int_ge(count[FLICK][EFL_GESTURE_STATE_CANCELED - 1], 1);
   CHECK_ZERO(ROTATE);
   CHECK_ZERO(ZOOM);

   RESET;
}
EFL_END_TEST

EFL_START_TEST(test_efl_ui_gesture_zoom)
{
   Eo *rect = setup();
   int moves;

   moves = pinch_object(rect, 500, 500, 501, 501, -250, -250, 250, 250);
   /* canceled */
   CHECK_ALL(TAP, 1, 0, 0, 1);
   /* canceled */
   CHECK_ALL(LONG_PRESS, 1, 0, 0, 1);
   /* canceled */
   CHECK_ALL(DOUBLE_TAP, 1, 0, 0, 1);
   /* canceled */
   CHECK_ALL(TRIPLE_TAP, 1, 0, 0, 1);

   CHECK_START(MOMENTUM, 1);
   CHECK_UPDATE(MOMENTUM, 0);
   CHECK_FINISH(MOMENTUM, 0);
   CHECK_CANCEL(MOMENTUM, 1);

   /* only finish is verifiable */
   CHECK_FINISH(FLICK, 0);
   /* started then canceled */
   CHECK_ALL(ROTATE, 1, 0, 0, 1);
   /* started 1x */
   CHECK_START(ZOOM, 1);
   /* 2 touch points tracked, so this will be roughly (2 * moves) but probably less */
   ck_assert_int_ge(count[ZOOM][EFL_GESTURE_STATE_UPDATED - 1], moves);
   /* finished 1x */
   CHECK_FINISH(ZOOM, 1);
   CHECK_CANCEL(ZOOM, 0);

   RESET;


   moves = pinch_object(rect, 250, 250, 750, 750, 250, 250, -250, -250);
   /* canceled */
   CHECK_ALL(TAP, 1, 0, 0, 1);
   /* canceled */
   CHECK_ALL(LONG_PRESS, 1, 0, 0, 1);
   /* canceled */
   CHECK_ALL(DOUBLE_TAP, 1, 0, 0, 1);
   /* canceled */
   CHECK_ALL(TRIPLE_TAP, 1, 0, 0, 1);

   CHECK_START(MOMENTUM, 1);
   CHECK_UPDATE(MOMENTUM, 0);
   CHECK_FINISH(MOMENTUM, 0);
   CHECK_CANCEL(MOMENTUM, 1);

   /* only finish is verifiable */
   CHECK_FINISH(FLICK, 0);
      /* started then canceled */
   CHECK_ALL(ROTATE, 1, 0, 0, 1);
   /* started 1x */
   CHECK_START(ZOOM, 1);
   /* 2 touch points tracked, so this will be roughly (2 * moves) but probably less */
   ck_assert_int_ge(count[ZOOM][EFL_GESTURE_STATE_UPDATED - 1], moves);
   /* finished 1x */
   CHECK_FINISH(ZOOM, 1);
   CHECK_CANCEL(ZOOM, 0);

   RESET;

}
EFL_END_TEST

EFL_START_TEST(test_efl_ui_gesture_rotate)
{
   Eo *rect = setup();
   int moves, momentum_moves;

   multi_press_object(rect, 1);
   CHECK_ALL(TAP, 1, 0, 0, 0);
   CHECK_ALL(LONG_PRESS, 1, 0, 0, 0);
   CHECK_ALL(DOUBLE_TAP, 1, 0, 0, 0);
   CHECK_ALL(TRIPLE_TAP, 1, 0, 0, 0);

   CHECK_ZERO(MOMENTUM);
   CHECK_ZERO(FLICK);
   CHECK_ZERO(ROTATE);
   CHECK_ZERO(ZOOM);

   RESET;


   moves = multi_drag_object_around(rect, 1, 500, 500, 250, 180);
   CHECK_ALL(TAP, 0, 0, 0, 1);
   CHECK_ALL(LONG_PRESS, 0, 0, 0, 1);
   CHECK_ALL(DOUBLE_TAP, 0, 0, 0, 1);
   CHECK_ALL(TRIPLE_TAP, 0, 0, 0, 1);

   CHECK_START(MOMENTUM, 1);
   momentum_moves = count[MOMENTUM][EFL_GESTURE_STATE_UPDATED - 1];
   ck_assert_int_ge(count[MOMENTUM][EFL_GESTURE_STATE_UPDATED - 1], moves - 5);
   CHECK_FINISH(MOMENTUM, 1);
   CHECK_CANCEL(MOMENTUM, 0);

   /* flick is just going to do flick stuff here, so don't even bother checking much */
   CHECK_FINISH(FLICK, 0);

   CHECK_ALL(ROTATE, 1, moves - 1, 1, 0);
   CHECK_ALL(ZOOM, 1, 0, 0, 1);

   RESET;

   /* verify identical motion in reverse */
   moves = multi_drag_object_around(rect, 1, 500, 500, 250, -180);
   /* already occurred, first finger still down */
   CHECK_ZERO(TAP);
   /* already canceled, first finger still down */
   CHECK_ZERO(LONG_PRESS);
   CHECK_ZERO(DOUBLE_TAP);
   CHECK_ZERO(TRIPLE_TAP);

   /* continuing gesture, counts as already started */
   CHECK_START(MOMENTUM, 0);
   /* should be exactly 1 more than previous time */
   CHECK_UPDATE(MOMENTUM, momentum_moves + 1);
   CHECK_FINISH(MOMENTUM, 1);
   CHECK_CANCEL(MOMENTUM, 0);

   /* flick is just going to do flick stuff here, so don't even bother checking much */
   CHECK_FINISH(FLICK, 0);

   /* continuing gesture, counts as already started, increment update counter */
   CHECK_ALL(ROTATE, 0, (moves - 1) + 1, 1, 0);
   CHECK_ALL(ZOOM, 0, 1, 0, 1);

}
EFL_END_TEST

static void
custom_cb(void *data EINA_UNUSED , const Efl_Event *ev)
{
   Efl_Canvas_Gesture *g = ev->info;

   int *count = data;
   if (!eina_streq(efl_gesture_custom_gesture_name_get(g), "custom_gesture")) return;
   /* increment counter for event state which has been processed */
   count[efl_gesture_state_get(g) - 1]++;
}

static void
custom_cb2(void *data EINA_UNUSED , const Efl_Event *ev)
{
   Efl_Canvas_Gesture *g = ev->info;

   int *count = data;
   if (!eina_streq(efl_gesture_custom_gesture_name_get(g), "custom_gesture2")) return;
   /* increment counter for event state which has been processed */
   count[efl_gesture_state_get(g) - 1]++;
}

static void
custom_gesture_cb(void *data EINA_UNUSED , const Efl_Event *ev)
{
   Efl_Canvas_Gesture *g = ev->info;

   Eina_Position2D *delta = data;
   if (!eina_streq(efl_gesture_custom_gesture_name_get(g), "custom_gesture")) return;
   delta->x = custom_gesture_x_delta_get(g);
   delta->y = custom_gesture_y_delta_get(g);
}

EFL_START_TEST(test_efl_ui_gesture_custom)
{
   Eo *rect = setup();
   Eo *manager = efl_provider_find(rect, EFL_CANVAS_GESTURE_MANAGER_CLASS);
   Eo *recognizer = efl_add(CUSTOM_RECOGNIZER_CLASS, manager);
   Eo *recognizer2 = efl_add(CUSTOM_RECOGNIZER2_CLASS, manager);
   Eina_Position2D delta = {0};

   efl_gesture_manager_recognizer_register(manager, recognizer);
   efl_gesture_manager_recognizer_register(manager, recognizer2);
   efl_event_callback_add(rect, EFL_EVENT_GESTURE_CUSTOM, custom_cb, &count[CUSTOM]);
   efl_event_callback_add(rect, EFL_EVENT_GESTURE_CUSTOM, custom_cb2, &count[CUSTOM2]);

   /* verify that we're processing */
   click_object(rect);
   CHECK_ALL(CUSTOM, 1, 0, 1, 0);
   CHECK_ALL(CUSTOM2, 1, 0, 0, 1);

   RESET;

   /* verify gesture properties */
   efl_event_callback_add(rect, EFL_EVENT_GESTURE_CUSTOM, custom_gesture_cb, &delta);
   drag_object(rect, 0, 0, 75, 30, EINA_FALSE);
   ck_assert_int_eq(delta.x, 75);
   ck_assert_int_eq(delta.y, 30);
   efl_event_callback_del(rect, EFL_EVENT_GESTURE_CUSTOM, custom_gesture_cb, &delta);

   RESET;

   /* verify that we aren't still processing */
   efl_gesture_manager_recognizer_unregister(manager, recognizer);
   efl_gesture_manager_recognizer_unregister(manager, recognizer2);
   click_object(rect);
   CHECK_ZERO(CUSTOM);
   CHECK_ZERO(CUSTOM2);

   RESET;

   /* verify re-register + early finish from custom2 */
   efl_gesture_manager_recognizer_register(manager, recognizer);
   efl_gesture_manager_recognizer_register(manager, recognizer2);
   drag_object(rect, 500, 500, 1, 0, EINA_FALSE);
   CHECK_ALL(CUSTOM, 1, 1, 1, 0);
   CHECK_ALL(CUSTOM2, 1, 0, 1, 0);

   efl_event_callback_del(rect, EFL_EVENT_GESTURE_CUSTOM, custom_cb, &count[CUSTOM]);
   efl_event_callback_del(rect, EFL_EVENT_GESTURE_CUSTOM, custom_cb2, &count[CUSTOM2]);

   RESET;

   /* verify we don't have anything totally weird going on */
   click_object(rect);
   CHECK_ZERO(CUSTOM);
   CHECK_ZERO(CUSTOM2);

   efl_gesture_manager_recognizer_unregister(manager, recognizer);
   efl_gesture_manager_recognizer_unregister(manager, recognizer2);
}
EFL_END_TEST


EFL_START_TEST(test_efl_ui_gesture_sequence)
{
   Eo *rect = setup();
   int moves;

   multi_click_object(rect, 1);
   CHECK_ALL(TAP, 1, 0, 1, 0);

   wait_timer(0.4);
   RESET;

   moves = pinch_object(rect, 500, 500, 501, 501, -250, 0, 250, 0);
   /* canceled */
   CHECK_ALL(TAP, 1, 0, 0, 1);
   /* canceled */
   CHECK_ALL(LONG_PRESS, 1, 0, 0, 1);
   /* canceled */
   CHECK_ALL(DOUBLE_TAP, 1, 0, 0, 1);
   /* canceled */
   CHECK_ALL(TRIPLE_TAP, 1, 0, 0, 1);


   CHECK_START(ZOOM, 1);
   /* 2 touch points tracked, so this will be roughly (2 * moves) but probably less */
   ck_assert_int_ge(count[ZOOM][EFL_GESTURE_STATE_UPDATED - 1], moves);
   /* finished 1x */
   CHECK_FINISH(ZOOM, 1);
   CHECK_CANCEL(ZOOM, 0);

   wait_timer(0.4);
   RESET;

   multi_click_object(rect, 1);
   CHECK_ALL(TAP, 1, 0, 1, 0);

   RESET;
}
EFL_END_TEST

void efl_ui_test_gesture(TCase *tc)
{
   tcase_add_test(tc, test_efl_ui_gesture_taps);
   tcase_add_test(tc, test_efl_ui_gesture_long_press);
   tcase_add_test(tc, test_efl_ui_gesture_flick);
   tcase_add_test(tc, test_efl_ui_gesture_zoom);
   tcase_add_test(tc, test_efl_ui_gesture_rotate);
   tcase_add_test(tc, test_efl_ui_gesture_custom);
   tcase_add_test(tc, test_efl_ui_gesture_sequence);
}
