#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "evas_common_private.h"
#include "evas_private.h"
#include "efl_canvas_object_animation.eo.h"
#include <Ecore.h>

#define MY_CLASS EFL_CANVAS_OBJECT_ANIMATION_MIXIN


typedef struct
{
   Efl_Canvas_Animation *animation;
   double speed;
   double progress;
   double run_start_time;
   double start_pos;
   int remaining_repeats;
   Efl_Loop_Timer *timer;
   Eina_Bool pause_state : 1;
} Efl_Canvas_Object_Animation_Indirect_Data;

typedef struct
{
   Efl_Canvas_Object_Animation_Indirect_Data *in;
} Efl_Canvas_Object_Animation_Data;

static void _end(Efl_Canvas_Object_Animation *obj, Efl_Canvas_Object_Animation_Data *pd);


static void
_animator_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Eo *obj = data;
   Efl_Canvas_Object_Animation_Data *pd = efl_data_scope_get(obj, MY_CLASS);
   double duration, elapsed_time, vector, current;

   EINA_SAFETY_ON_NULL_RETURN(pd->in);
   current = ecore_loop_time_get();
   EINA_SAFETY_ON_FALSE_RETURN(pd->in->run_start_time <= current);

   duration = efl_animation_duration_get(pd->in->animation) / pd->in->speed;
   elapsed_time = current - pd->in->run_start_time;
   if (EINA_DBL_EQ(duration, 0))
     {
        if (pd->in->speed < 0.0)
          vector = -1.0;
        else
          vector = 1.0;
     }
   else
     vector = elapsed_time / duration;

   /* When animation player starts, _animator_cb() is called immediately so
    * both elapsed time and progress are 0.0.
    * Since it is the beginning of the animation if progress is 0.0, the
    * following codes for animation should be executed. */
   if (pd->in->speed < 0.0)
     vector += 1.0;
   pd->in->progress = CLAMP(0.0, vector, 1.0);

   /* The previously applied map effect should be reset before applying the
    * current map effect. Otherwise, the incrementally added map effects
    * increase numerical error. */
   efl_gfx_mapping_reset(obj);
   efl_animation_apply(pd->in->animation, pd->in->progress, obj);

   double progress = pd->in->progress;
   efl_event_callback_call(obj, EFL_CANVAS_OBJECT_ANIMATION_EVENT_ANIMATION_PROGRESS_UPDATED, &progress);

   //Check if animation stopped in animation_progress,updated callback.
   if (!pd->in) return;

   //Not end. Keep going.
   if ((pd->in->speed < 0 && EINA_DBL_EQ(pd->in->progress, 0)) ||
       (pd->in->speed > 0 && EINA_DBL_EQ(pd->in->progress, 1.0)))
     {
        //Repeat animation
        if ((efl_animation_play_count_get(pd->in->animation) == 0) ||
            (pd->in->remaining_repeats > 0))
          {
             pd->in->remaining_repeats--;

             if (efl_animation_repeat_mode_get(pd->in->animation) == EFL_CANVAS_ANIMATION_REPEAT_MODE_REVERSE)
               pd->in->speed *= -1;

             pd->in->run_start_time = current;
          }
        else
          {
             efl_canvas_object_animation_stop(obj);
          }
     }
}

static void
_end(Efl_Canvas_Object_Animation *obj, Efl_Canvas_Object_Animation_Data *pd)
{
   EINA_SAFETY_ON_NULL_RETURN(pd->in);
   efl_event_callback_del(obj, EFL_CANVAS_OBJECT_EVENT_ANIMATOR_TICK, _animator_cb, obj);
}

static void
_start(Efl_Canvas_Object_Animation *obj, Efl_Canvas_Object_Animation_Data *pd, double delay)
{
   EINA_SAFETY_ON_NULL_RETURN(pd->in);
   pd->in->run_start_time = ecore_loop_time_get() - efl_animation_duration_get(pd->in->animation)*delay;
   efl_event_callback_add(obj, EFL_CANVAS_OBJECT_EVENT_ANIMATOR_TICK, _animator_cb, obj);
   _animator_cb(obj, NULL);
}

static Eina_Value
_start_fcb(Eo *o, void *data EINA_UNUSED, const Eina_Value v)
{
   Efl_Canvas_Object_Animation_Data *pd = efl_data_scope_safe_get(o, MY_CLASS);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pd, EINA_VALUE_EMPTY);
   if (!pd->in) return v; //animation was stopped before anything started
   _start(o, pd, pd->in->start_pos);
   return v;
}

EOLIAN static Efl_Canvas_Animation*
_efl_canvas_object_animation_animation_get(const Eo *obj EINA_UNUSED, Efl_Canvas_Object_Animation_Data *pd)
{
   if (!pd->in) return NULL;
   return pd->in->animation;
}

EOLIAN static double
_efl_canvas_object_animation_animation_progress_get(const Eo *obj EINA_UNUSED, Efl_Canvas_Object_Animation_Data *pd)
{
   if (pd->in && pd->in->animation)
     return (pd->in->speed < 0) ? fabs(1.0 - pd->in->progress) : pd->in->progress;
   else
     return -1.0;
}

EOLIAN static void
_efl_canvas_object_animation_animation_pause_set(Eo *obj, Efl_Canvas_Object_Animation_Data *pd, Eina_Bool pause)
{
   EINA_SAFETY_ON_NULL_RETURN(pd->in);

   if (pd->in->pause_state == pause) return;

   if (pause)
     _end(obj, pd);
   else
     _start(obj, pd,(pd->in->speed < 0) ? 1.0 - pd->in->progress : pd->in->progress);
   if (pd->in) pd->in->pause_state = pause;
}

EOLIAN static Eina_Bool
_efl_canvas_object_animation_animation_pause_get(const Eo *obj EINA_UNUSED, Efl_Canvas_Object_Animation_Data *pd)
{
   if (!pd->in) return EINA_FALSE;

   return pd->in->pause_state;
}

EOLIAN static void
_efl_canvas_object_animation_animation_start(Eo *obj, Efl_Canvas_Object_Animation_Data *pd, Efl_Canvas_Animation *animation, double speed, double start_pos)
{
   Efl_Canvas_Object_Animation_Indirect_Data *in;
   if (pd->in && pd->in->animation)
     efl_canvas_object_animation_stop(obj);
   EINA_SAFETY_ON_FALSE_RETURN(!pd->in);
   in = pd->in = calloc(1, sizeof(Efl_Canvas_Object_Animation_Indirect_Data));

   EINA_SAFETY_ON_NULL_RETURN(animation);
   EINA_SAFETY_ON_FALSE_RETURN(start_pos >= 0.0 && start_pos <= 1.0);
   EINA_SAFETY_ON_FALSE_RETURN(!EINA_DBL_EQ(speed, 0.0));
   EINA_SAFETY_ON_FALSE_RETURN(efl_playable_seekable_get(animation));

   in->pause_state = EINA_FALSE;
   in->animation = efl_ref(animation);
   in->remaining_repeats = efl_animation_play_count_get(animation) - 1; // -1 because one run is already going on
   in->speed = speed;
   in->start_pos = start_pos;
   efl_event_callback_call(obj, EFL_CANVAS_OBJECT_ANIMATION_EVENT_ANIMATION_CHANGED, in->animation);
   //You should not rely on in beeing available after calling the above event.
   in = NULL;

   if (efl_animation_start_delay_get(animation) > 0.0)
     {
        Eina_Future *f = efl_loop_timeout(efl_loop_get(obj), efl_animation_start_delay_get(animation));

        efl_future_then(obj, f, .success = _start_fcb);
     }
   else
     _start(obj, pd, start_pos);
}

EOLIAN static void
_efl_canvas_object_animation_animation_stop(Eo *obj, Efl_Canvas_Object_Animation_Data *pd)
{
   if (!pd->in) return;

   if (!efl_animation_final_state_keep_get(pd->in->animation))
     efl_gfx_mapping_reset(obj);
   _end(obj, pd);
   efl_unref(pd->in->animation);
   pd->in->animation = NULL;

   efl_event_callback_call(obj, EFL_CANVAS_OBJECT_ANIMATION_EVENT_ANIMATION_CHANGED, pd->in->animation);

   //this could be NULL if some weird callstack calls stop again while the above event is executed
   if (pd->in)
     free(pd->in);
   pd->in = NULL;
}

#include "efl_canvas_object_animation.eo.c"
