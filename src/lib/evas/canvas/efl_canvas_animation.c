#include "efl_canvas_animation_private.h"
#include <math.h>
#define MY_CLASS EFL_CANVAS_ANIMATION_CLASS

static double _default_animation_time = 0.2; //in seconds

EOLIAN static void
_efl_canvas_animation_duration_set(Eo *eo_obj EINA_UNUSED,
                            Efl_Canvas_Animation_Data *pd,
                            double sec)
{
   EINA_SAFETY_ON_FALSE_RETURN(sec >= 0.0);
   pd->duration = sec;
}

EOLIAN static double
_efl_canvas_animation_duration_get(const Eo *eo_obj EINA_UNUSED, Efl_Canvas_Animation_Data *pd)
{
   return pd->duration;
}

EOLIAN static void
_efl_canvas_animation_final_state_keep_set(Eo *eo_obj EINA_UNUSED,
                                    Efl_Canvas_Animation_Data *pd,
                                    Eina_Bool keep)
{
   if (pd->keep_final_state == keep) return;

   pd->keep_final_state = !!keep;
}

EOLIAN static Eina_Bool
_efl_canvas_animation_final_state_keep_get(const Eo *eo_obj EINA_UNUSED,
                                    Efl_Canvas_Animation_Data *pd)
{
   return pd->keep_final_state;
}

EOLIAN static void
_efl_canvas_animation_repeat_mode_set(Eo *eo_obj EINA_UNUSED,
                               Efl_Canvas_Animation_Data *pd,
                               Efl_Canvas_Animation_Repeat_Mode mode)
{
   EINA_SAFETY_ON_FALSE_RETURN(mode >= 0 && mode < EFL_CANVAS_ANIMATION_REPEAT_MODE_LAST);
   pd->repeat_mode = mode;
}

EOLIAN static Efl_Canvas_Animation_Repeat_Mode
_efl_canvas_animation_repeat_mode_get(const Eo *eo_obj EINA_UNUSED, Efl_Canvas_Animation_Data *pd)
{
   return pd->repeat_mode;
}

EOLIAN static void
_efl_canvas_animation_play_count_set(Eo *eo_obj EINA_UNUSED,
                                Efl_Canvas_Animation_Data *pd,
                                int count)
{
   EINA_SAFETY_ON_FALSE_RETURN(count >= 0);

   pd->play_count = count;
}

EOLIAN static int
_efl_canvas_animation_play_count_get(const Eo *eo_obj EINA_UNUSED, Efl_Canvas_Animation_Data *pd)
{
   return pd->play_count;
}

EOLIAN static void
_efl_canvas_animation_start_delay_set(Eo *eo_obj EINA_UNUSED,
                               Efl_Canvas_Animation_Data *pd,
                               double sec)
{
   EINA_SAFETY_ON_FALSE_RETURN(sec >= 0.0);

   pd->start_delay_time = sec;
}

EOLIAN static double
_efl_canvas_animation_start_delay_get(const Eo *eo_obj EINA_UNUSED,
                               Efl_Canvas_Animation_Data *pd)
{
   return pd->start_delay_time;
}

EOLIAN static void
_efl_canvas_animation_interpolator_set(Eo *eo_obj EINA_UNUSED,
                                Efl_Canvas_Animation_Data *pd,
                                Efl_Interpolator *interpolator)
{
   pd->interpolator = interpolator;
}

EOLIAN static Efl_Interpolator *
_efl_canvas_animation_interpolator_get(const Eo *eo_obj EINA_UNUSED,
                                Efl_Canvas_Animation_Data *pd)
{
   return pd->interpolator;
}

EOLIAN static double
_efl_canvas_animation_animation_apply(Eo *eo_obj,
                               Efl_Canvas_Animation_Data *pd EINA_UNUSED,
                               double progress,
                               Efl_Canvas_Object *target EINA_UNUSED)
{
   Efl_Interpolator *interpolator = efl_animation_interpolator_get(eo_obj);
   if (interpolator)
     progress = efl_interpolator_interpolate(interpolator, progress);

   return progress;
}

EOLIAN static double
_efl_canvas_animation_efl_playable_length_get(const Eo *eo_obj, Efl_Canvas_Animation_Data *pd EINA_UNUSED)
{
   if (efl_animation_play_count_get(eo_obj) == 0)
     {
        return INFINITY;
     }

   return (efl_animation_duration_get(eo_obj) * efl_animation_play_count_get(eo_obj));
}

EOLIAN static Eina_Bool
_efl_canvas_animation_efl_playable_playable_get(const Eo *eo_obj EINA_UNUSED, Efl_Canvas_Animation_Data *pd EINA_UNUSED)
{
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_canvas_animation_efl_playable_seekable_get(const Eo *eo_obj EINA_UNUSED, Efl_Canvas_Animation_Data *pd EINA_UNUSED)
{
   return EINA_TRUE;
}

EOLIAN static Efl_Object*
_efl_canvas_animation_efl_object_constructor(Eo *obj, Efl_Canvas_Animation_Data *pd)
{
   pd->duration = _default_animation_time;
   pd->play_count = 1;
   return efl_constructor(efl_super(obj, MY_CLASS));
}

EOLIAN static void
_efl_canvas_animation_default_duration_set(double animation_time)
{
   EINA_SAFETY_ON_FALSE_RETURN(animation_time >= 0.0);
   _default_animation_time = animation_time;
}

EOLIAN static double
_efl_canvas_animation_default_duration_get(void)
{
   return _default_animation_time;
}

#include "efl_canvas_animation.eo.c"
