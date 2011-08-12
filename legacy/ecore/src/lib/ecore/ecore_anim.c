#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>
#include <math.h>

#include "Ecore.h"
#include "ecore_private.h"


struct _Ecore_Animator
{
   EINA_INLIST;
   ECORE_MAGIC;

   Ecore_Task_Cb func;
   void          *data;

   double             start, run;
   Ecore_Timeline_Cb  run_func;
   void              *run_data;

   Eina_Bool     delete_me : 1;
   Eina_Bool     suspended : 1;
};


static Eina_Bool _ecore_animator_run(void *data);
static Eina_Bool _ecore_animator(void *data);

static int                    animators_delete_me = 0;
static Ecore_Animator        *animators = NULL;
static double                 animators_frametime = 1.0 / 30.0;

static Ecore_Animator_Source  src = ECORE_ANIMATOR_SOURCE_TIMER;
static Ecore_Timer           *timer = NULL;
static int                    ticking = 0;
static Ecore_Cb               begin_tick_cb = NULL;
static const void            *begin_tick_data = NULL;
static Ecore_Cb               end_tick_cb = NULL;
static const void            *end_tick_data = NULL;

static void
_begin_tick(void)
{
   if (ticking) return;
   ticking = 1;
   switch (src)
     {
      case ECORE_ANIMATOR_SOURCE_TIMER:
        if (!timer)
          {
             double t_loop = ecore_loop_time_get();
             double sync_0 = 0.0;
             double d = -fmod(t_loop - sync_0, animators_frametime);

             timer = _ecore_timer_loop_add(animators_frametime,
                                                    _ecore_animator, NULL);
             _ecore_timer_delay(timer, d);
          }
        break;
      case ECORE_ANIMATOR_SOURCE_CUSTOM:
        if (begin_tick_cb) begin_tick_cb((void *)begin_tick_data);
        break;
      default:
        break;
     }
}

static void
_end_tick(void)
{
   if (!ticking) return;
   ticking = 0;
   switch (src)
     {
      case ECORE_ANIMATOR_SOURCE_TIMER:
        if (timer)
          {
             _ecore_timer_del(timer);
             timer = NULL;
          }
        break;
      case ECORE_ANIMATOR_SOURCE_CUSTOM:
        if (end_tick_cb) end_tick_cb((void *)end_tick_data);
        break;
      default:
        break;
     }
}

static Eina_Bool
_do_tick(void)
{
   Ecore_Animator *animator;

   EINA_INLIST_FOREACH(animators, animator)
     {
        if (!animator->delete_me && !animator->suspended)
          {
             if (!_ecore_call_task_cb(animator->func, animator->data))
               {
                  animator->delete_me = EINA_TRUE;
                  animators_delete_me++;
               }
          }
     }
   if (animators_delete_me)
     {
        Ecore_Animator *l;
        for (l = animators; l;)
          {
             animator = l;
             l = (Ecore_Animator *) EINA_INLIST_GET(l)->next;
             if (animator->delete_me)
               {
                  animators = (Ecore_Animator *)
                     eina_inlist_remove(EINA_INLIST_GET(animators), 
                                        EINA_INLIST_GET(animator));
                  ECORE_MAGIC_SET(animator, ECORE_MAGIC_NONE);
                  free(animator);
                  animators_delete_me--;
                  if (animators_delete_me == 0) break;
               }
          }
     }
   if (!animators)
     {
        _end_tick();
        return ECORE_CALLBACK_CANCEL;
     }
   return ECORE_CALLBACK_RENEW;
}

static Ecore_Animator *
_ecore_animator_add(Ecore_Task_Cb func, const void *data)
{
   Ecore_Animator *animator = NULL;

   if (!func) return animator;
   animator = calloc(1, sizeof(Ecore_Animator));
   if (!animator) return animator;
   ECORE_MAGIC_SET(animator, ECORE_MAGIC_ANIMATOR);
   animator->func = func;
   animator->data = (void *)data;
   animators = (Ecore_Animator *)eina_inlist_append(EINA_INLIST_GET(animators), EINA_INLIST_GET(animator));
   _begin_tick();
   return animator;
}

EAPI Ecore_Animator *
ecore_animator_add(Ecore_Task_Cb func, const void *data)
{
   Ecore_Animator *animator;

   _ecore_lock();
   animator = _ecore_animator_add(func, data);
   _ecore_unlock();

   return animator;
}

EAPI Ecore_Animator *
ecore_animator_timeline_add(double runtime, Ecore_Timeline_Cb func, const void *data)
{
   Ecore_Animator *animator;

   _ecore_lock();
   if (runtime <= 0.0) runtime = 0.0;
   animator = _ecore_animator_add(_ecore_animator_run, NULL);
   animator->data = animator;
   animator->run_func = func;
   animator->run_data = (void *)data;
   animator->start = ecore_loop_time_get();
   animator->run = runtime;
   _ecore_unlock();
   return animator;
}

static double
_pos_map_sin(double in)
{
   return eina_f32p32_double_to(eina_f32p32_sin(eina_f32p32_double_from(in)));
}

static double
_pos_map_cos(double in)
{
   return eina_f32p32_double_to(eina_f32p32_cos(eina_f32p32_double_from(in)));
}

static double
_pos_map_accel_factor(double pos, double v1)
{
   int i, fact = (int)v1;
   double p, o1 = pos, o2 = pos, v;
   p = 1.0 - _pos_map_sin((M_PI / 2.0) + ((pos * M_PI) / 2.0));
   o2 = p;
   for (i = 0; i < fact; i++)
     {
        o1 = o2;
        o2 = o2 * p;
     }
   v = v1 - (double)fact;
   pos = (v * o2) + ((1.0 - v) * o1);
   return pos;
}

static double
_pos_map_pow(double pos, double divis, int p)
{
   double v = 1.0;
   int i;
   for (i = 0; i < p; i++) v *= pos;
   return ((pos * divis) * (1.0 - v)) + (pos * v);
}

static double
_pos_map_spring(double pos, int bounces, double decfac)
{
   int segnum, segpos, b1, b2;
   double len, decay, decpos, p2;
   if (bounces < 0) bounces = 0;
   p2 = _pos_map_pow(pos, 0.5, 3);
   len = (M_PI / 2.0) + ((double)bounces * M_PI);
   segnum = (bounces * 2) + 1;
   segpos = 2 * (((int)(p2 * segnum) + 1) / 2);
   b1 = segpos;
   b2 = segnum + 1;
   if (b1 < 0) b1 = 0;
   decpos = (double)b1 / (double)b2;
   decay = _pos_map_accel_factor(1.0 - decpos, decfac);
   return _pos_map_sin((M_PI / 2.0) + (p2 * len)) * decay;
}

EAPI double
ecore_animator_pos_map(double pos, Ecore_Pos_Map map, double v1, double v2)
{
   /* purely functional - locking not required */
   if (pos > 1.0) pos = 1.0;
   else if (pos < 0.0) pos = 0.0;
   switch (map)
     {
      case ECORE_POS_MAP_LINEAR:
        return pos;
      case ECORE_POS_MAP_ACCELERATE:
        pos = 1.0 - _pos_map_sin((M_PI / 2.0) + ((pos * M_PI) / 2.0));
        return pos;
      case ECORE_POS_MAP_DECELERATE:
        pos = _pos_map_sin((pos * M_PI) / 2.0);
        return pos;
      case ECORE_POS_MAP_SINUSOIDAL:
        pos = (1.0 - _pos_map_cos(pos * M_PI)) / 2.0;
        return pos;
      case ECORE_POS_MAP_ACCELERATE_FACTOR:
        pos = _pos_map_accel_factor(pos, v1);
        return pos;
      case ECORE_POS_MAP_DECELERATE_FACTOR:
        pos = 1.0 - _pos_map_accel_factor(1.0 - pos, v1);
        return pos;
      case ECORE_POS_MAP_SINUSOIDAL_FACTOR:
        if (pos < 0.5) pos = _pos_map_accel_factor(pos * 2.0, v1) / 2.0;
        else pos = 1.0 - (_pos_map_accel_factor((1.0 - pos) * 2.0, v1) / 2.0);
        return pos;
      case ECORE_POS_MAP_DIVISOR_INTERP:
        pos = _pos_map_pow(pos, v1, (int)v2);
        return pos;
      case ECORE_POS_MAP_BOUNCE:
        pos = _pos_map_spring(pos, (int)v2, v1);
        if (pos < 0.0) pos = -pos;
        pos = 1.0 - pos;
        return pos;
      case ECORE_POS_MAP_SPRING:
        pos = 1.0 - _pos_map_spring(pos, (int)v2, v1);
        return pos;
      default:
        return pos;
     }
   return pos;
   v2 = 0.0;
}

EAPI void *
ecore_animator_del(Ecore_Animator *animator)
{
   void *data = NULL;

   _ecore_lock();
   if (!ECORE_MAGIC_CHECK(animator, ECORE_MAGIC_ANIMATOR))
     {
        ECORE_MAGIC_FAIL(animator, ECORE_MAGIC_ANIMATOR,
                         "ecore_animator_del");
        goto unlock;
     }
   if (animator->delete_me)
     {
        data = animator->data;
        goto unlock;
     }
   animator->delete_me = EINA_TRUE;
   animators_delete_me++;
   if (animator->run_func)
     data = animator->run_data;
   else
     data = animator->data;
unlock:
   _ecore_unlock();
   return data;
}

EAPI void
ecore_animator_frametime_set(double frametime)
{
   _ecore_lock();
   if (frametime < 0.0) frametime = 0.0;
   if (animators_frametime == frametime) goto unlock;
   animators_frametime = frametime;
   _end_tick();
   if (animators) _begin_tick();
unlock:
   _ecore_unlock();
}

EAPI double
ecore_animator_frametime_get(void)
{
   return animators_frametime;
}

EAPI void
ecore_animator_freeze(Ecore_Animator *animator)
{
   _ecore_lock();
   if (!ECORE_MAGIC_CHECK(animator, ECORE_MAGIC_ANIMATOR))
     {
        ECORE_MAGIC_FAIL(animator, ECORE_MAGIC_ANIMATOR,
                         "ecore_animator_del");
        goto unlock;
     }
   if (animator->delete_me) goto unlock;
   animator->suspended = EINA_TRUE;
unlock:
   _ecore_unlock();
}

EAPI void
ecore_animator_thaw(Ecore_Animator *animator)
{
   _ecore_lock();
   if (!ECORE_MAGIC_CHECK(animator, ECORE_MAGIC_ANIMATOR))
     {
        ECORE_MAGIC_FAIL(animator, ECORE_MAGIC_ANIMATOR,
                         "ecore_animator_del");
        goto unlock;
     }
   if (animator->delete_me) goto unlock;
   animator->suspended = EINA_FALSE;
unlock:
   _ecore_unlock();
}

EAPI void
ecore_animator_source_set(Ecore_Animator_Source source)
{
   _ecore_lock();
   src = source;
   _end_tick();
   if (animators) _begin_tick();
   _ecore_unlock();
}

EAPI Ecore_Animator_Source
ecore_animator_source_get(void)
{
   return src;
}

EAPI void
ecore_animator_custom_source_tick_begin_callback_set(Ecore_Cb func, const void *data)
{
   _ecore_lock();
   begin_tick_cb = func;
   begin_tick_data = data;
   _end_tick();
   if (animators) _begin_tick();
   _ecore_unlock();
}

EAPI void
ecore_animator_custom_source_tick_end_callback_set(Ecore_Cb func, const void *data)
{
   _ecore_lock();
   end_tick_cb = func;
   end_tick_data = data;
   _end_tick();
   if (animators) _begin_tick();
   _ecore_unlock();
}

EAPI void
ecore_animator_custom_tick(void)
{
   _ecore_lock();
   if (src == ECORE_ANIMATOR_SOURCE_CUSTOM) _do_tick();
   _ecore_unlock();
}

void
_ecore_animator_shutdown(void)
{
   _end_tick();
   while (animators)
     {
        Ecore_Animator *animator;

        animator = animators;
        animators = (Ecore_Animator *) eina_inlist_remove(EINA_INLIST_GET(animators), EINA_INLIST_GET(animators));
        ECORE_MAGIC_SET(animator, ECORE_MAGIC_NONE);
        free(animator);
     }
}

static Eina_Bool
_ecore_animator_run(void *data)
{
   Ecore_Animator *animator = data;
   double pos = 0.0, t;
   Eina_Bool run_ret;

   t = ecore_loop_time_get();
   if (animator->run > 0.0)
     {
        pos = (t - animator->start) / animator->run;
        if (pos > 1.0) pos = 1.0;
        else if (pos < 0.0) pos = 0.0;
     }
   run_ret = animator->run_func(animator->run_data, pos);
   if (t >= (animator->start + animator->run)) run_ret = EINA_FALSE;
   return run_ret;
}

static Eina_Bool
_ecore_animator(void *data __UNUSED__)
{
   Eina_Bool r;
   _ecore_lock();
   r = _do_tick();
   _ecore_unlock();
   return r;
}
