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

static Ecore_Timer    *timer = NULL;
static int             animators_delete_me = 0;
static Ecore_Animator *animators = NULL;
static double          animators_frametime = 1.0 / 30.0;

/**
 * @addtogroup Ecore_Group Ecore - Main Loop and Job Functions.
 *
 * @{
 */

/**
 * @addtogroup Ecore_Animator_Group Ecore Animator functions
 *
 * @{
 */

/**
 * Add a animator to tick off at every animaton tick during main loop execution.
 * @param func The function to call when it ticks off
 * @param data The data to pass to the function
 * @return A handle to the new animator
 *
 * This function adds a animator and returns its handle on success and NULL on
 * failure. The function @p func will be called every N seconds where N is the
 * frametime interval set by ecore_animator_frametime_set(). The function will
 * be passed the @p data pointer as its parameter.
 *
 * When the animator @p func is called, it must return a value of either 1 or 0.
 * If it returns 1 (or ECORE_CALLBACK_RENEW), it will be called again at the
 * next tick, or if it returns 0 (or ECORE_CALLBACK_CANCEL) it will be deleted
 * automatically making any references/handles for it invalid.
 */
EAPI Ecore_Animator *
ecore_animator_add(Ecore_Task_Cb func, const void *data)
{
   Ecore_Animator *animator;

   if (!func) return NULL;
   animator = calloc(1, sizeof(Ecore_Animator));
   if (!animator) return NULL;
   ECORE_MAGIC_SET(animator, ECORE_MAGIC_ANIMATOR);
   animator->func = func;
   animator->data = (void *)data;
   animators = (Ecore_Animator *)eina_inlist_append(EINA_INLIST_GET(animators), EINA_INLIST_GET(animator));
   if (!timer)
     {
        double t_loop = ecore_loop_time_get();
        double sync_0 = 0.0;
        double d = -fmod(t_loop - sync_0, animators_frametime);

        timer = ecore_timer_loop_add(animators_frametime, _ecore_animator, NULL);
        ecore_timer_delay(timer, d);
     }
   return animator;
}

/**
 * Add a animator that runs for a limited time
 * @param runtime The time to run in seconds
 * @param func The function to call when it ticks off
 * @param data The data to pass to the function
 * @return A handle to the new animator
 *
 * This function is just like ecore_animator_add() except the animator only
 * runs for a limited time specified in seconds by @p runtime. Once the runtime
 * the animator has elapsed (animator finished) it will automatically be
 * deleted. The callback function @p func can return ECORE_CALLBACK_RENEW to
 * keep the animator running or ECORE_CALLBACK_CANCEL ro stop it and have
 * it be deleted automatically at any time.
 *
 * The @p func will ALSO be passed a position parameter that will be in value
 * from 0.0 to 1.0 to indicate where along the timeline (0.0 start, 1.0 end)
 * the animator run is at. If the callback wishes not to have a linear
 * transition it can "map" this value to one of several curves and mappings
 * via ecore_animator_pos_map().
 *
 * @since 1.1.0
 */
EAPI Ecore_Animator *
ecore_animator_timeline_add(double runtime, Ecore_Timeline_Cb func, const void *data)
{
   Ecore_Animator *animator;

   if (runtime <= 0.0) runtime = 0.0;
   animator = ecore_animator_add(_ecore_animator_run, NULL);
   animator->data = animator;
   animator->run_func = func;
   animator->run_data = (void *)data;
   animator->start = ecore_loop_time_get();
   animator->run = runtime;
   return animator;
}

static double
_pos_map_accel_factor(double pos, double v1)
{
   int i, fact = (int)v1;
   double p, o1 = pos, o2 = pos, v;
   p = 1.0 - sin((M_PI / 2.0) + ((pos * M_PI) / 2.0));
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
   return sin((M_PI / 2.0) + (p2 * len)) * decay;
}

/**
 * Maps an input position from 0.0 to 1.0 along a timeline to another position
 *
 * Takes an input position (0.0 to 1.0) and maps to a new position (normally
 * between 0.0 and 1.0, but it may go above/below 0.0 or 1.0 to show that it
 * has "overshot" the mark) using some interpolation (mapping) algorithm.
 *
 * You might normally use this like:
 * @code
 * double pos; // input position in a timeline from 0.0 to 1.0
 * double out; // output position after mapping
 * int x1, y1, x2, y2; // x1 & y1 are start position, x2 & y2 are end position
 * int x, y; // x & y are the calculated position
 *
 * out = ecore_animator_pos_map(pos, ECORE_POS_MAP_BOUNCE, 1.8, 7);
 * x = (x1 * out) + (x2 * (1.0 - out));
 * y = (y1 * out) + (y2 * (1.0 - out));
 * move_my_object_to(myobject, x, y);
 * @endcode
 *
 * @param pos The input position to map
 * @param map The mapping to use
 * @param v1 A parameter use by the mapping (pass 0.0 if not used)
 * @param v2 A parameter use by the mapping (pass 0.0 if not used)
 * @return The mapped value
 *
 * @since 1.1.0
 */
EAPI double
ecore_animator_pos_map(double pos, Ecore_Pos_Map map, double v1, double v2)
{
   if (pos > 1.0) pos = 1.0;
   else if (pos < 0.0) pos = 0.0;
   switch (map)
     {
      case ECORE_POS_MAP_LINEAR:
        return pos;
      case ECORE_POS_MAP_ACCELERATE:
        pos = 1.0 - sin((M_PI / 2.0) + ((pos * M_PI) / 2.0));
        return pos;
      case ECORE_POS_MAP_DECELERATE:
        pos = sin((pos * M_PI) / 2.0);
        return pos;
      case ECORE_POS_MAP_SINUSOIDAL:
        pos = (1.0 - cos(pos * M_PI)) / 2.0;
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

/**
 * Delete the specified animator from the animator list.
 * @param animator The animator to delete
 * @return The data pointer set for the animator on add
 *
 * Delete the specified @p aqnimator from the set of animators that are executed
 * during main loop execution. This function returns the data parameter that
 * was being passed to the callback on success, or NULL on failure. After this
 * call returns the specified animator object @p animator is invalid and should not
 * be used again. It will not get called again after deletion.
 */
EAPI void *
ecore_animator_del(Ecore_Animator *animator)
{
   if (!ECORE_MAGIC_CHECK(animator, ECORE_MAGIC_ANIMATOR))
     {
        ECORE_MAGIC_FAIL(animator, ECORE_MAGIC_ANIMATOR,
                         "ecore_animator_del");
        return NULL;
     }
   if (animator->delete_me) return animator->data;
   animator->delete_me = EINA_TRUE;
   animators_delete_me++;
   if (animator->run_func) return animator->run_data;
   return animator->data;
}

/**
 * Set the animator call interval in seconds.
 * @param frametime The time in seconds in between animator ticks.
 *
 * This function sets the time interval (in seconds) between animator ticks.
 */
EAPI void
ecore_animator_frametime_set(double frametime)
{
   if (frametime < 0.0) frametime = 0.0;
   if (animators_frametime == frametime) return;
   animators_frametime = frametime;
   if (timer)
     {
        ecore_timer_del(timer);
        timer = NULL;
     }
   if (animators)
     timer = ecore_timer_add(animators_frametime, _ecore_animator, NULL);
}

/**
 * Get the animator call interval in seconds.
 * @return The time in second in between animator ticks.
 *
 * this function retrieves the time between animator ticks, in seconds.
 */
EAPI double
ecore_animator_frametime_get(void)
{
   return animators_frametime;
}

/**
 * Suspend the specified animator.
 * @param animator The animator to delete
 *
 * The specified @p animator will be temporarly removed from the set of animators
 * that are executed during main loop execution.
 */
EAPI void
ecore_animator_freeze(Ecore_Animator *animator)
{
   if (!ECORE_MAGIC_CHECK(animator, ECORE_MAGIC_ANIMATOR))
     {
        ECORE_MAGIC_FAIL(animator, ECORE_MAGIC_ANIMATOR,
                         "ecore_animator_del");
        return;
     }
   if (animator->delete_me) return;
   animator->suspended = EINA_TRUE;
}

/**
 * Restore execution of the specified animator.
 * @param animator The animator to delete
 *
 * The specified @p animator will be put back in the set of animators
 * that are executed during main loop execution.
 */
EAPI void
ecore_animator_thaw(Ecore_Animator *animator)
{
   if (!ECORE_MAGIC_CHECK(animator, ECORE_MAGIC_ANIMATOR))
     {
        ECORE_MAGIC_FAIL(animator, ECORE_MAGIC_ANIMATOR,
                         "ecore_animator_del");
        return;
     }
   if (animator->delete_me) return;
   animator->suspended = EINA_FALSE;
}

void
_ecore_animator_shutdown(void)
{
   if (timer)
     {
        ecore_timer_del(timer);
        timer = NULL;
     }
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
   Ecore_Animator *animator;

   EINA_INLIST_FOREACH(animators, animator)
     {
        if (!animator->delete_me && !animator->suspended)
          {
             if (!animator->func(animator->data))
               {
                  animator->delete_me = EINA_TRUE;
                  animators_delete_me++;
               }
          }
     }
   if (animators_delete_me)
     {
        Ecore_Animator *l;
        for(l = animators; l;)
          {
             animator = l;
             l = (Ecore_Animator *) EINA_INLIST_GET(l)->next;
             if (animator->delete_me)
               {
                  animators = (Ecore_Animator *) eina_inlist_remove(EINA_INLIST_GET(animators), EINA_INLIST_GET(animator));
                  ECORE_MAGIC_SET(animator, ECORE_MAGIC_NONE);
                  free(animator);
                  animators_delete_me--;
                  if (animators_delete_me == 0) break;
               }
          }
     }
   if (!animators)
     {
        timer = NULL;
        return ECORE_CALLBACK_CANCEL;
     }
   return ECORE_CALLBACK_RENEW;
}

/**
 * @}
 */

/**
 * @}
 */
