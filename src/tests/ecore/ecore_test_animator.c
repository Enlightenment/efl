#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <Ecore.h>

#include "ecore_suite.h"
#include <math.h>

static double prev = 0;
static Eina_Bool _anim_cb(void *data EINA_UNUSED, double pos)
{

  fail_if(prev > pos);
  prev = pos;

  if (pos == 1.0)
    ecore_main_loop_quit();

  return EINA_TRUE;
}

START_TEST(ecore_test_animators)
{
   Eo *animator;
   double interval1 = 0.02;
   double interval2 = 0.01;

   fail_if(!ecore_init(), "ERROR: Cannot init Ecore!\n");

   ecore_animator_frametime_set(interval1);
   animator = eo_add_custom(ECORE_ANIMATOR_CLASS, NULL, ecore_animator_timeline_constructor(1, _anim_cb, &interval1));

   fail_if(!animator);

   ecore_main_loop_begin();

   ecore_animator_frametime_set(interval2);
   prev = 0;
   animator = eo_add_custom(ECORE_ANIMATOR_CLASS, NULL, ecore_animator_timeline_constructor(1, _anim_cb, &interval2));
   fail_if(!animator);

   ecore_main_loop_begin();

   ecore_shutdown();

}
END_TEST

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

START_TEST(ecore_test_pos_map)
{
  fail_if(!test_pos(ECORE_POS_MAP_LINEAR, 0, 0, _linear));
  fail_if(!test_pos(ECORE_POS_MAP_ACCELERATE, 0, 0, _accel));
  fail_if(!test_pos(ECORE_POS_MAP_DECELERATE, 0, 0, _decel));
  fail_if(!test_pos(ECORE_POS_MAP_SINUSOIDAL, 0, 0, _sinusoidal));
}
END_TEST

void ecore_test_animator(TCase *tc)
{
  tcase_add_test(tc, ecore_test_animators);
  tcase_add_test(tc, ecore_test_pos_map);
}
