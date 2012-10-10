#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>
#include <math.h>

#include "Ecore.h"
#include "ecore_private.h"

#include "Eo.h"

#define MY_CLASS ECORE_ANIMATOR_CLASS

#define MY_CLASS_NAME "ecore_animator"

#define ECORE_ANIMATOR_CHECK(obj)                       \
  if (!eo_isa((obj), ECORE_ANIMATOR_CLASS)) \
    return

EAPI Eo_Op ECORE_ANIMATOR_BASE_ID = EO_NOOP;

struct _Ecore_Animator_Private_Data
{
   EINA_INLIST;
   Ecore_Animator   *obj;

   Ecore_Task_Cb     func;
   void             *data;

   double            start, run;
   Ecore_Timeline_Cb run_func;
   void             *run_data;

   Eina_Bool         delete_me : 1;
   Eina_Bool         suspended : 1;
   Eina_Bool         just_added : 1;
};

typedef struct _Ecore_Animator_Private_Data Ecore_Animator_Private_Data;

static Eina_Bool _ecore_animator_run(void *data);
static Eina_Bool _ecore_animator(void *data);

static int animators_delete_me = 0;
static Ecore_Animator_Private_Data *animators = NULL;
static double animators_frametime = 1.0 / 30.0;

static Ecore_Animator_Source src = ECORE_ANIMATOR_SOURCE_TIMER;
static Ecore_Timer *timer = NULL;
static int ticking = 0;
static Ecore_Cb begin_tick_cb = NULL;
static const void *begin_tick_data = NULL;
static Ecore_Cb end_tick_cb = NULL;
static const void *end_tick_data = NULL;

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
   Ecore_Animator_Private_Data *animator;

   EINA_INLIST_FOREACH(animators, animator)
     {
        animator->just_added = EINA_FALSE;
     }
   EINA_INLIST_FOREACH(animators, animator)
     {
        if ((!animator->delete_me) && 
            (!animator->suspended) && 
            (!animator->just_added))
          {
             if (!_ecore_call_task_cb(animator->func, animator->data))
               {
                  animator->delete_me = EINA_TRUE;
                  animators_delete_me++;
               }
          }
        else animator->just_added = EINA_FALSE;
     }
   if (animators_delete_me)
     {
        Ecore_Animator_Private_Data *l;
        for (l = animators; l; )
          {
             animator = l;
             l = (Ecore_Animator_Private_Data *)EINA_INLIST_GET(l)->next;
             if (animator->delete_me)
               {
                  animators = (Ecore_Animator_Private_Data *)
                    eina_inlist_remove(EINA_INLIST_GET(animators),
                                       EINA_INLIST_GET(animator));

                  eo_parent_set(animator->obj, NULL);
                  if (eo_destructed_is(animator->obj))
                     eo_manual_free(animator->obj);
                  else
                     eo_manual_free_set(animator->obj, EINA_FALSE);

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

static Eina_Bool
_ecore_animator_add(Ecore_Animator *obj,
                    Ecore_Animator_Private_Data *animator,
                    Ecore_Task_Cb func,
                    const void   *data)
{
    if (EINA_UNLIKELY(!eina_main_loop_is()))
      {
         eo_error_set(obj);
         EINA_MAIN_LOOP_CHECK_RETURN_VAL(EINA_FALSE);
      }

   animator->obj = obj;
   eo_do_super(obj, eo_constructor());
   eo_manual_free_set(obj, EINA_TRUE);

   if (!func)
     {
        eo_error_set(obj);
        ERR("callback function must be set up for an object of class: '%s'", MY_CLASS_NAME);
        return EINA_FALSE;
     }

   animator->func = func;
   animator->data = (void *)data;
   animator->just_added = EINA_TRUE;
   animators = (Ecore_Animator_Private_Data *)eina_inlist_append(EINA_INLIST_GET(animators), EINA_INLIST_GET(animator));
   _begin_tick();
   return EINA_TRUE;
}

static void
_constructor(Eo *obj, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   eo_error_set(obj);
   ERR("only custom constructor can be used with '%s' class", MY_CLASS_NAME);
}

EAPI Ecore_Animator *
ecore_animator_add(Ecore_Task_Cb func,
                   const void   *data)
{
   Ecore_Animator *animator = NULL;

   animator = eo_add_custom(MY_CLASS, _ecore_parent,
                            ecore_animator_constructor(func, data));
   eo_unref(animator);
   return animator;
}

static void
_animator_constructor(Eo *obj, void *_pd, va_list *list)
{
   Ecore_Task_Cb func = va_arg(*list, Ecore_Task_Cb);
   const void *data = va_arg(*list, const void *);

   _ecore_lock();
   Ecore_Animator_Private_Data *animator = _pd;
   _ecore_animator_add(obj, animator, func, data);
   _ecore_unlock();
}

EAPI Ecore_Animator *
ecore_animator_timeline_add(double            runtime,
                            Ecore_Timeline_Cb func,
                            const void       *data)
{
   Ecore_Animator *animator;
   animator = eo_add_custom(MY_CLASS, _ecore_parent,
                            ecore_animator_timeline_constructor(runtime, func, data));
   eo_unref(animator);
   return animator;
}

static void
_animator_timeline_constructor(Eo *obj, void *_pd, va_list *list)
{
   _ecore_lock();
   double runtime = va_arg(*list, double);
   if (runtime <= 0.0) runtime = 0.0;
   Ecore_Timeline_Cb func = va_arg(*list, Ecore_Timeline_Cb);
   const void *data = va_arg(*list, const void *);

   Ecore_Animator_Private_Data *animator = _pd;
   if (!_ecore_animator_add(obj, animator, _ecore_animator_run, NULL)) goto unlock;

   animator->data = obj;
   animator->run_func = func;
   animator->run_data = (void *)data;
   animator->start = ecore_loop_time_get();
   animator->run = runtime;

unlock:
   _ecore_unlock();
}

static double
_pos_map_sin(double in)
{
   return eina_f32p32_double_to(eina_f32p32_sin(eina_f32p32_double_from(in)));
}

#if 0
static double
_pos_map_cos(double in)
{
   return eina_f32p32_double_to(eina_f32p32_cos(eina_f32p32_double_from(in)));
}
#endif

static double
_pos_map_accel_factor(double pos,
                      double v1)
{
   int i, fact = (int)v1;
   double p, o1 = pos, o2, v;
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
_pos_map_pow(double pos,
             double divis,
             int    p)
{
   double v = 1.0;
   int i;
   for (i = 0; i < p; i++) v *= pos;
   return ((pos * divis) * (1.0 - v)) + (pos * v);
}

static double
_pos_map_spring(double pos,
                int    bounces,
                double decfac)
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

#define DBL_TO(Fp) eina_f32p32_double_to(Fp)
#define DBL_FROM(D) eina_f32p32_double_from(D)
#define INT_FROM(I) eina_f32p32_int_from(I)
#define SIN(Fp) eina_f32p32_sin(Fp)
#define COS(Fp) eina_f32p32_cos(Fp)
#define ADD(A, B) eina_f32p32_add(A, B)
#define SUB(A, B) eina_f32p32_sub(A, B)
#define MUL(A, B) eina_f32p32_mul(A, B)

EAPI double
ecore_animator_pos_map(double        pos,
                       Ecore_Pos_Map map,
                       double        v1,
                       double        v2)
{
   /* purely functional - locking not required */
    if (pos > 1.0) pos = 1.0;
    else if (pos < 0.0)
      pos = 0.0;
    switch (map)
      {
       case ECORE_POS_MAP_LINEAR:
         return pos;

       case ECORE_POS_MAP_ACCELERATE:
	 /* pos = 1 - sin(Pi / 2 + pos * Pi / 2); */
	 pos = DBL_TO(SUB(INT_FROM(1), SIN(ADD((EINA_F32P32_PI >> 1), MUL(DBL_FROM(pos), (EINA_F32P32_PI >> 1))))));
         return pos;

       case ECORE_POS_MAP_DECELERATE:
	 /* pos = sin(pos * Pi / 2); */
	 pos = DBL_TO(SIN(MUL(DBL_FROM(pos), (EINA_F32P32_PI >> 1))));
         return pos;

       case ECORE_POS_MAP_SINUSOIDAL:
	 /* pos = (1 - cos(pos * Pi)) / 2 */
	 pos = DBL_TO((SUB(INT_FROM(1), COS(MUL(DBL_FROM(pos), EINA_F32P32_PI)))) >> 1);
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
}

EAPI void *
ecore_animator_del(Ecore_Animator *obj)
{
   void *data = NULL;

   EINA_MAIN_LOOP_CHECK_RETURN_VAL(NULL);
   Ecore_Animator_Private_Data *animator = eo_data_get(obj, MY_CLASS);
   _ecore_lock();

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

static void
_destructor(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Ecore_Animator_Private_Data *pd = _pd;

   pd->delete_me = EINA_TRUE;
   animators_delete_me++;

   eo_do_super(obj, eo_destructor());
}

EAPI void
ecore_animator_frametime_set(double frametime)
{
   EINA_MAIN_LOOP_CHECK_RETURN;
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
   EINA_MAIN_LOOP_CHECK_RETURN_VAL(0.0);
   return animators_frametime;
}

EAPI void
ecore_animator_freeze(Ecore_Animator *animator)
{
   ECORE_ANIMATOR_CHECK(animator);
   eo_do(animator, eo_event_freeze());
}

static void
_ecore_animator_freeze(Eo *obj EINA_UNUSED, void *_pd,
                                            va_list *list EINA_UNUSED)
{
   EINA_MAIN_LOOP_CHECK_RETURN;
   _ecore_lock();
   Ecore_Animator_Private_Data *animator = _pd;

   if (animator->delete_me) goto unlock;
   animator->suspended = EINA_TRUE;
unlock:
   _ecore_unlock();
}

EAPI void
ecore_animator_thaw(Ecore_Animator *animator)
{
   ECORE_ANIMATOR_CHECK(animator);
   eo_do(animator, eo_event_thaw());
}

static void
_ecore_animator_thaw(Eo *obj EINA_UNUSED, void *_pd,
                                          va_list *list EINA_UNUSED)
{
   EINA_MAIN_LOOP_CHECK_RETURN;
   Ecore_Animator_Private_Data *animator = _pd;

   _ecore_lock();
   if (animator->delete_me) goto unlock;
   animator->suspended = EINA_FALSE;
unlock:
   _ecore_unlock();
}

EAPI void
ecore_animator_source_set(Ecore_Animator_Source source)
{
   EINA_MAIN_LOOP_CHECK_RETURN;
   _ecore_lock();
   src = source;
   _end_tick();
   if (animators) _begin_tick();
   _ecore_unlock();
}

EAPI Ecore_Animator_Source
ecore_animator_source_get(void)
{
   EINA_MAIN_LOOP_CHECK_RETURN_VAL(0);
   return src;
}

EAPI void
ecore_animator_custom_source_tick_begin_callback_set(Ecore_Cb    func,
                                                     const void *data)
{
   EINA_MAIN_LOOP_CHECK_RETURN;
   _ecore_lock();
   begin_tick_cb = func;
   begin_tick_data = data;
   _end_tick();
   if (animators) _begin_tick();
   _ecore_unlock();
}

EAPI void
ecore_animator_custom_source_tick_end_callback_set(Ecore_Cb    func,
                                                   const void *data)
{
   EINA_MAIN_LOOP_CHECK_RETURN;
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
   EINA_MAIN_LOOP_CHECK_RETURN;
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
        Ecore_Animator_Private_Data *animator;

        animator = animators;
        animators = (Ecore_Animator_Private_Data *)eina_inlist_remove(EINA_INLIST_GET(animators), EINA_INLIST_GET(animators));

        eo_parent_set(animator->obj, NULL);
        if (eo_destructed_is(animator->obj))
           eo_manual_free(animator->obj);
        else
           eo_manual_free_set(animator->obj, EINA_FALSE);
     }
}

static Eina_Bool
_ecore_animator_run(void *data)
{
   Ecore_Animator *obj = data;
   Ecore_Animator_Private_Data *animator = eo_data_get(obj, MY_CLASS);

   double pos = 0.0, t;
   Eina_Bool run_ret;

   t = ecore_loop_time_get();
   if (animator->run > 0.0)
     {
        pos = (t - animator->start) / animator->run;
        if (pos > 1.0) pos = 1.0;
        else if (pos < 0.0)
          pos = 0.0;
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

static void
_class_constructor(Eo_Class *klass)
{
   const Eo_Op_Func_Description func_desc[] = {
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_CONSTRUCTOR), _constructor),
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_DESTRUCTOR), _destructor),
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_EVENT_FREEZE), _ecore_animator_freeze),
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_EVENT_THAW), _ecore_animator_thaw),

        EO_OP_FUNC(ECORE_ANIMATOR_ID(ECORE_ANIMATOR_SUB_ID_CONSTRUCTOR), _animator_constructor),
        EO_OP_FUNC(ECORE_ANIMATOR_ID(ECORE_ANIMATOR_SUB_ID_TIMELINE_CONSTRUCTOR), _animator_timeline_constructor),
        EO_OP_FUNC_SENTINEL
   };

   eo_class_funcs_set(klass, func_desc);
}

static const Eo_Op_Description op_desc[] = {
     EO_OP_DESCRIPTION(ECORE_ANIMATOR_SUB_ID_CONSTRUCTOR, "Add an animator to call func at every animation tick during main loop execution."),
     EO_OP_DESCRIPTION(ECORE_ANIMATOR_SUB_ID_TIMELINE_CONSTRUCTOR, "Add an animator that runs for a limited time"),
     EO_OP_DESCRIPTION_SENTINEL
};

static const Eo_Class_Description class_desc = {
     EO_VERSION,
     MY_CLASS_NAME,
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(&ECORE_ANIMATOR_BASE_ID, op_desc, ECORE_ANIMATOR_SUB_ID_LAST),
     NULL,
     sizeof(Ecore_Animator_Private_Data),
     _class_constructor,
     NULL
};

EO_DEFINE_CLASS(ecore_animator_class_get, &class_desc, EO_BASE_CLASS, NULL)
