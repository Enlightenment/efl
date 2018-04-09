#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <math.h>

#include <Ecore.h>

#include "ecore_suite.h"

static double prev = 0;
static Eina_Bool _anim_cb(void *data EINA_UNUSED, double pos)
{

  fail_if(prev > pos);
  prev = pos;

  if (pos == 1.0)
    ecore_main_loop_quit();

  return EINA_TRUE;
}

EFL_START_TEST(ecore_test_animators)
{
   Ecore_Animator *animator;
   double interval1 = 0.02;
   double interval2 = 0.01;

   ecore_animator_frametime_set(interval1);
   animator = ecore_animator_timeline_add(0.1, _anim_cb, &interval1);

   fail_if(!animator);

   ecore_main_loop_begin();

   ecore_animator_frametime_set(interval2);
   prev = 0;
   animator = ecore_animator_timeline_add(0.1, _anim_cb, &interval2);
   fail_if(!animator);

   ecore_main_loop_begin();

}
EFL_END_TEST

Eina_Bool test_pos(Ecore_Pos_Map posmap, double v1, double v2, double (*testmap)(double val, double v1, double v2))
{
  double pos;
  double res1;
  double res2;

  for (pos = 0.0; pos < 1.01; pos += 0.01) {
      res1 = ecore_animator_pos_map(pos, posmap, v1, v2);
      res2 = testmap(pos, v1, v2);

      if (fabs(res1-res2) > 0.005) {
          printf("(%f): %f != %f\n", pos, res1, res2);
          return EINA_FALSE;
      }
  }
  fail_if(ecore_animator_pos_map(1.0, posmap, v1, v2) != 1.0);

  return EINA_TRUE;
}

double _linear(double val, double v1 EINA_UNUSED, double v2 EINA_UNUSED)
{
  return val;
}

double _accel(double val, double v1 EINA_UNUSED, double v2 EINA_UNUSED)
{
  return 1 - sin(M_PI_2 + val * M_PI_2);
}

double _decel(double val, double v1 EINA_UNUSED, double v2 EINA_UNUSED)
{
  return  sin(val * M_PI_2);
}

double _sinusoidal(double val, double v1 EINA_UNUSED, double v2 EINA_UNUSED)
{
  return  (1 - cos(val * M_PI)) / 2;
}

EFL_START_TEST(ecore_test_pos_map)
{
  fail_if(!test_pos(ECORE_POS_MAP_LINEAR, 0, 0, _linear));
  fail_if(!test_pos(ECORE_POS_MAP_ACCELERATE, 0, 0, _accel));
  fail_if(!test_pos(ECORE_POS_MAP_DECELERATE, 0, 0, _decel));
  fail_if(!test_pos(ECORE_POS_MAP_SINUSOIDAL, 0, 0, _sinusoidal));
}
EFL_END_TEST

static void _animator_called_cb(void *data)
{
   Eina_Bool *called = data;

   *called = EINA_TRUE;
}

static Eina_Bool _animator_cb(void *data)
{
   Eina_Bool *called = data;

   *called = EINA_TRUE;
   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool _switch_cb(void* data EINA_UNUSED)
{
   ecore_animator_custom_tick();
   return ECORE_CALLBACK_CANCEL;
}

static Eina_Bool _quit_cb(void* data EINA_UNUSED)
{
   ecore_main_loop_quit();
   return ECORE_CALLBACK_CANCEL;
}

EFL_START_TEST(ecore_test_begin_end_tick)
{
   Ecore_Timer *timer1, *timer2;
   Ecore_Animator *animator;
   Eina_Bool is_animator_cb_called = EINA_FALSE;
   Eina_Bool is_begin_cb_called = EINA_FALSE;
   Eina_Bool is_end_cb_called = EINA_FALSE;

   ecore_animator_custom_source_tick_begin_callback_set(_animator_called_cb, &is_begin_cb_called);
   ecore_animator_custom_source_tick_end_callback_set(_animator_called_cb, &is_end_cb_called);

   animator = ecore_animator_add(_animator_cb, &is_animator_cb_called);
   fail_if(!animator);

   timer1 = ecore_timer_add(0.01, _switch_cb, NULL);
   fail_if(!timer1);

   timer2 = ecore_timer_add(0.03, _quit_cb, NULL);
   fail_if(!timer2);

   ecore_animator_source_set(ECORE_ANIMATOR_SOURCE_CUSTOM);
   ecore_main_loop_begin();
   ecore_animator_del(animator);

   fail_if(!is_begin_cb_called);
   fail_if(!is_end_cb_called);
   fail_if(!is_animator_cb_called);
}
EFL_END_TEST

void ecore_test_animator(TCase *tc)
{
  tcase_add_test(tc, ecore_test_animators);
  tcase_add_test(tc, ecore_test_pos_map);
  tcase_add_test(tc, ecore_test_begin_end_tick);
}
