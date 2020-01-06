#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Efl_Ui.h>
#include "efl_ui_suite.h"

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
   LONG_TAP,
   DOUBLE_TAP,
   TRIPLE_TAP,
   MOMENTUM,
   FLICK,
   ZOOM,
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
   efl_gfx_entity_size_set(win, EINA_SIZE2D(100, 100));

   rect = efl_add(EFL_CANVAS_RECTANGLE_CLASS, win);
   efl_content_set(win, rect);

#define WATCH(type) \
   efl_event_callback_add(rect, EFL_EVENT_GESTURE_##type, gesture_cb, &count[(type)])
   WATCH(TAP);
   WATCH(LONG_TAP);
   WATCH(DOUBLE_TAP);
   WATCH(TRIPLE_TAP);
   WATCH(MOMENTUM);
   WATCH(FLICK);
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
   CHECK_ALL(LONG_TAP, 1, 0, 0, 1);
   CHECK_ALL(DOUBLE_TAP, 1, 1, 0, 0);
   CHECK_ALL(TRIPLE_TAP, 1, 1, 0, 0);
   CHECK_ZERO(MOMENTUM);
   CHECK_ZERO(FLICK);
   CHECK_ZERO(ZOOM);

   RESET;

   /* add a second tap */
   click_object(rect);
   CHECK_ALL(TAP, 1, 0, 1, 0);
   CHECK_ALL(LONG_TAP, 1, 0, 0, 1);
   /* UPDATE -> FINISH */
   CHECK_ALL(DOUBLE_TAP, 0, 1, 1, 0);
   CHECK_ALL(TRIPLE_TAP, 0, 2, 0, 0);
   CHECK_ZERO(MOMENTUM);
   CHECK_ZERO(FLICK);
   CHECK_ZERO(ZOOM);

   RESET;

   /* add a third tap */
   click_object(rect);
   CHECK_ALL(TAP, 1, 0, 1, 0);
   CHECK_ALL(LONG_TAP, 1, 0, 0, 1);
   /* UPDATE -> FINISH */
   CHECK_ALL(DOUBLE_TAP, 1, 1, 0, 0);
   CHECK_ALL(TRIPLE_TAP, 0, 1, 1, 0);
   CHECK_ZERO(MOMENTUM);
   CHECK_ZERO(FLICK);
   CHECK_ZERO(ZOOM);
}
EFL_END_TEST

EFL_START_TEST(test_efl_ui_gesture_flick)
{
   Eo *rect = setup();

   /* basic flick */
   drag_object(rect, 0, 0, 75, 0, EINA_FALSE);
   /* canceled */
   CHECK_ALL(TAP, 1, 0, 0, 1);
   /* canceled */
   CHECK_ALL(LONG_TAP, 1, 0, 0, 1);
   /* canceled */
   CHECK_ALL(DOUBLE_TAP, 1, 0, 0, 1);
   /* canceled */
   CHECK_ALL(TRIPLE_TAP, 1, 0, 0, 1);
   /* updated but canceled */
   CHECK_ALL(MOMENTUM, 1, DRAG_OBJECT_NUM_MOVES - 1, 0, 1);
   /* triggered */
   CHECK_ALL(FLICK, 1, DRAG_OBJECT_NUM_MOVES - 1, 1, 0);
   CHECK_ZERO(ZOOM);

   RESET;
}
EFL_END_TEST

void efl_ui_test_gesture(TCase *tc)
{
   tcase_add_test(tc, test_efl_ui_gesture_taps);
   tcase_add_test(tc, test_efl_ui_gesture_flick);
}
