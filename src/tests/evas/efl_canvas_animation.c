#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>

#include <Evas.h>

#include "evas_suite.h"
#include "evas_tests_helpers.h"

EFL_START_TEST(efl_canvas_animation_negative_double_checking)
{
   Efl_Canvas_Animation *animation = efl_new(EFL_CANVAS_ANIMATION_CLASS);

   efl_animation_duration_set(animation, 1.0);
   ck_assert_int_eq(efl_animation_duration_get(animation), 1.0);
   efl_animation_duration_set(animation, 3.0);
   ck_assert_int_eq(efl_animation_duration_get(animation), 3.0);
   EXPECT_ERROR_START;
   efl_animation_duration_set(animation, -1.0);
   ck_assert_int_eq(efl_animation_duration_get(animation), 3.0);
   EXPECT_ERROR_END;

   efl_animation_start_delay_set(animation, 1.0);
   ck_assert(EINA_DBL_EQ(efl_animation_start_delay_get(animation), 1.0));
   efl_animation_start_delay_set(animation, 0.0);
   ck_assert(EINA_DBL_EQ(efl_animation_start_delay_get(animation), 0.0));
   EXPECT_ERROR_START;
   efl_animation_start_delay_set(animation, -1.0);
   ck_assert_int_eq(efl_animation_start_delay_get(animation), 0.0);
   EXPECT_ERROR_END;

}
EFL_END_TEST

EFL_START_TEST(efl_canvas_animation_default_value)
{
   Efl_Canvas_Animation *animation = efl_new(EFL_CANVAS_ANIMATION_CLASS);

   fail_if(EINA_DBL_EQ(efl_animation_duration_get(animation), 0.0));
}
EFL_END_TEST

static void
_duration_zero_anim_running_cb(void *data, const Efl_Event *event)
{
   double animation_speed = *((double*) data);
   double animation_running_position = *((double*) event->info);

   if (animation_speed > 0.0)
     ck_assert(EINA_DBL_EQ(animation_running_position, 1.0));
   else
     ck_assert(EINA_DBL_EQ(animation_running_position, 0.0));
}

static void
helper_inc_int(void *data, const Efl_Event *event EINA_UNUSED)
{
   int *called = (int*) data;
   *called+=1;
}

EFL_START_TEST(efl_canvas_animation_duration_zero)
{
   int running = 0;
   Evas *evas = EVAS_TEST_INIT_EVAS();
   Efl_Canvas_Rectangle *obj = efl_add(EFL_CANVAS_RECTANGLE_CLASS, evas);
   Efl_Canvas_Animation *animation = efl_add(EFL_CANVAS_ANIMATION_CLASS, evas, efl_animation_duration_set(efl_added, 0.0));

   double animation_speed = 1.0;
   efl_event_callback_add(obj, EFL_CANVAS_OBJECT_ANIMATION_EVENT_ANIMATION_PROGRESS_UPDATED, _duration_zero_anim_running_cb, &animation_speed);
   efl_event_callback_add(obj, EFL_CANVAS_OBJECT_ANIMATION_EVENT_ANIMATION_PROGRESS_UPDATED, helper_inc_int , &running);
   efl_canvas_object_animation_start(obj, animation, animation_speed, 0.0);
   ck_assert(EINA_DBL_EQ(efl_canvas_object_animation_progress_get(obj), -1.0));
   ck_assert_int_eq(running, 1);

   running = 0;
   animation_speed = -1.0;
   efl_canvas_object_animation_start(obj, animation, animation_speed, 0.0);
   ck_assert(EINA_DBL_EQ(efl_canvas_object_animation_progress_get(obj), -1.0));
   ck_assert_int_eq(running, 1);
}
EFL_END_TEST

void efl_test_canvas_animation(TCase *tc)
{
   tcase_add_test(tc, efl_canvas_animation_negative_double_checking);
   tcase_add_test(tc, efl_canvas_animation_default_value);
   tcase_add_test(tc, efl_canvas_animation_duration_zero);
}
