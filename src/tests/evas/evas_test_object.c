#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>

#include <Evas.h>
#define EFL_LOOP_PROTECTED //needed to set the loop time, we need that to simulate time passing for animation,tick
#include <Ecore.h>

#include "evas_suite.h"
#include "evas_tests_helpers.h"

static int called_changed;
static Efl_Canvas_Animation *animation_changed_ev;
static int called_running;
static double animation_running_position;

EFL_START_TEST(evas_object_various)
{
   Evas *evas = EVAS_TEST_INIT_EVAS();

   Evas_Object *obj = evas_object_rectangle_add(evas);

   evas_object_ref(obj);
   evas_free(evas);
   evas_object_unref(obj);

   evas = EVAS_TEST_INIT_EVAS();
   obj = evas_object_rectangle_add(evas);
   efl_ref(obj);
   evas_free(evas);
   efl_unref(obj);


   /* Twice because EVAS_TEST_INIT_EVAS inits it twice. */
}
EFL_END_TEST

EFL_START_TEST(evas_object_freeze_events)
{
   Evas *evas = EVAS_TEST_INIT_EVAS();
   Evas_Object *obj = evas_object_rectangle_add(evas);

   evas_object_freeze_events_set(obj, EINA_TRUE);
   ck_assert_int_eq(evas_object_freeze_events_get(obj), EINA_TRUE);
   evas_object_freeze_events_set(obj, EINA_FALSE);
   ck_assert_int_eq(evas_object_freeze_events_get(obj), EINA_FALSE);

   evas_object_freeze_events_set(obj, EINA_TRUE);
   evas_object_freeze_events_set(obj, EINA_TRUE);
   ck_assert_int_eq(evas_object_freeze_events_get(obj), EINA_TRUE);
   evas_object_freeze_events_set(obj, EINA_FALSE);
   ck_assert_int_eq(evas_object_freeze_events_get(obj), EINA_FALSE);
   evas_object_freeze_events_set(obj, EINA_FALSE);
   evas_object_freeze_events_set(obj, EINA_TRUE);
   ck_assert_int_eq(evas_object_freeze_events_get(obj), EINA_TRUE);
}
EFL_END_TEST

EFL_START_TEST(evas_object_animation_simple)
{
   Evas *evas = EVAS_TEST_INIT_EVAS();
   Evas_Object *obj = evas_object_rectangle_add(evas);
   Efl_Canvas_Animation *animation = efl_add(EFL_CANVAS_ANIMATION_CLASS, evas);

   ck_assert_ptr_eq(efl_canvas_object_animation_get(obj) , NULL);
   ck_assert(EINA_DBL_EQ(efl_canvas_object_animation_progress_get(obj), -1.0));

   efl_canvas_object_animation_start(obj, animation, 1.0, 0.0);
   ck_assert_ptr_eq(efl_canvas_object_animation_get(obj) , animation);
   ck_assert(EINA_DBL_EQ(efl_canvas_object_animation_progress_get(obj), 0.0));

   efl_canvas_object_animation_stop(obj);
   ck_assert_ptr_eq(efl_canvas_object_animation_get(obj) , NULL);
   ck_assert(EINA_DBL_EQ(efl_canvas_object_animation_progress_get(obj), -1.0));

   efl_canvas_object_animation_start(obj, animation, 1.0, 0.0);
   efl_canvas_object_animation_stop(obj);

   efl_canvas_object_animation_start(obj, animation, -1.0, 1.0);
   efl_canvas_object_animation_stop(obj);
   efl_canvas_object_animation_stop(obj);
}
EFL_END_TEST

EFL_START_TEST(evas_object_animation_progress)
{
   Evas *evas = EVAS_TEST_INIT_EVAS();
   Evas_Object *obj = evas_object_rectangle_add(evas);
   Efl_Canvas_Animation *animation = efl_add(EFL_CANVAS_ANIMATION_CLASS, evas, efl_animation_duration_set(efl_added, 1.0));

   efl_canvas_object_animation_start(obj, animation, 1.0, 0.0);
   efl_loop_time_set(efl_main_loop_get(), efl_loop_time_get(efl_main_loop_get()) + 0.5);
   efl_event_callback_call(obj, EFL_CANVAS_OBJECT_EVENT_ANIMATOR_TICK, NULL);
   ck_assert(EINA_DBL_EQ(efl_canvas_object_animation_progress_get(obj), 0.5));
   efl_canvas_object_animation_stop(obj);
}
EFL_END_TEST

static inline void
_simulate_time_passing(Eo *obj, double start, double jump)
{
   efl_loop_time_set(efl_main_loop_get(), start + jump);
   efl_event_callback_call(obj, EFL_CANVAS_OBJECT_EVENT_ANIMATOR_TICK, NULL);
}

static inline void
_simulate_assert_time_passing(Eo *obj, double start, double jump, double expected_position)
{
   _simulate_time_passing(obj, start, jump);
   ck_assert_int_eq((efl_canvas_object_animation_progress_get(obj)-expected_position)*10000, 0);
}

EFL_START_TEST(evas_object_animation_pause)
{
   Evas *evas = EVAS_TEST_INIT_EVAS();
   Evas_Object *obj = evas_object_rectangle_add(evas);
   double start = efl_loop_time_get(efl_main_loop_get());
   Efl_Canvas_Animation *animation = efl_add(EFL_CANVAS_ANIMATION_CLASS, evas, efl_animation_duration_set(efl_added, 1.0));

   efl_canvas_object_animation_start(obj, animation, 1.0, 0.0);

   _simulate_assert_time_passing(obj, start, 0.2, 0.2);
   efl_canvas_object_animation_pause_set(obj, EINA_TRUE);

   _simulate_assert_time_passing(obj, start, 0.5, 0.2);
   efl_canvas_object_animation_pause_set(obj, EINA_FALSE);

   _simulate_assert_time_passing(obj, start, 0.7, 0.4);
   efl_canvas_object_animation_stop(obj);
}
EFL_END_TEST

EFL_START_TEST(evas_object_animation_error)
{
   Evas *evas = EVAS_TEST_INIT_EVAS();
   Evas_Object *obj = evas_object_rectangle_add(evas);
   Efl_Canvas_Animation *animation = efl_add(EFL_CANVAS_ANIMATION_CLASS, evas);
   EXPECT_ERROR_START;
   efl_canvas_object_animation_start(obj, NULL, 1.0, 0.0);
   EXPECT_ERROR_END;

   EXPECT_ERROR_START;
   efl_canvas_object_animation_start(obj, animation, 0.0, 0.0);
   EXPECT_ERROR_END;
   efl_canvas_object_animation_stop(obj);

   EXPECT_ERROR_START;
   efl_canvas_object_animation_start(obj, animation, 1.0, 2.0);
   EXPECT_ERROR_END;
   efl_canvas_object_animation_stop(obj);

   EXPECT_ERROR_START;
   efl_canvas_object_animation_start(obj, animation, 1.0, -1.0);
   EXPECT_ERROR_END;
   efl_canvas_object_animation_stop(obj);
}
EFL_END_TEST

static void
_anim_changed_cb(void *data EINA_UNUSED, const Efl_Event *event EINA_UNUSED)
{
   animation_changed_ev = event->info;
   called_changed ++;
}

static void
_anim_running_cb(void *data EINA_UNUSED, const Efl_Event *event)
{
   animation_running_position = *((double*) event->info);
   called_running ++;
}

EFL_CALLBACKS_ARRAY_DEFINE(animation_stats_cb,
  {EFL_CANVAS_OBJECT_ANIMATION_EVENT_ANIMATION_CHANGED, _anim_changed_cb },
  {EFL_CANVAS_OBJECT_ANIMATION_EVENT_ANIMATION_PROGRESS_UPDATED, _anim_running_cb },
)

EFL_START_TEST(evas_object_animation_events)
{
   Evas *evas = EVAS_TEST_INIT_EVAS();
   Evas_Object *obj = evas_object_rectangle_add(evas);
   double start = efl_loop_time_get(efl_main_loop_get());
   Efl_Canvas_Animation *animation = efl_add(EFL_CANVAS_ANIMATION_CLASS, evas, efl_animation_duration_set(efl_added, 1.0));

   efl_event_callback_array_add(obj, animation_stats_cb(), NULL);

   efl_canvas_object_animation_start(obj, animation, 1.0, 0.0);

   ck_assert_int_eq(called_changed, 1);
   ck_assert_ptr_eq(animation_changed_ev, animation);
   ck_assert_int_eq(called_running, 1);
   ck_assert(EINA_DBL_EQ(animation_running_position, 0.0));

   _simulate_time_passing(obj, start, 1.0);

   ck_assert_int_eq(called_changed, 2);
   ck_assert_ptr_eq(animation_changed_ev, NULL);
   ck_assert_int_eq(called_running, 2);
   ck_assert(EINA_DBL_EQ(animation_running_position, 1.0));
   ck_assert_ptr_eq(efl_canvas_object_animation_get(obj), NULL);
   ck_assert(EINA_DBL_EQ(efl_canvas_object_animation_progress_get(obj), -1.0));
}
EFL_END_TEST

void evas_test_object(TCase *tc)
{
   tcase_add_test(tc, evas_object_various);
   tcase_add_test(tc, evas_object_freeze_events);
   tcase_add_test(tc, evas_object_animation_simple);
   tcase_add_test(tc, evas_object_animation_progress);
   tcase_add_test(tc, evas_object_animation_pause);
   tcase_add_test(tc, evas_object_animation_error);
   tcase_add_test(tc, evas_object_animation_events);
}
