#include "efl_animation_private.h"

static void
_target_del_cb(void *data, const Efl_Event *event EINA_UNUSED)
{
   Eo *eo_obj = data;

   EFL_ANIMATION_DATA_GET(eo_obj, pd);

   pd->target = NULL;
}

EOLIAN static void
_efl_animation_target_set(Eo *eo_obj,
                          Efl_Animation_Data *pd,
                          Efl_Canvas_Object *target)
{
   efl_event_callback_add(target, EFL_EVENT_DEL, _target_del_cb, eo_obj);

   pd->target = target;
}

EOLIAN static Efl_Canvas_Object *
_efl_animation_target_get(Eo *eo_obj EINA_UNUSED, Efl_Animation_Data *pd)
{
   return pd->target;
}

EOLIAN static void
_efl_animation_duration_set(Eo *eo_obj,
                            Efl_Animation_Data *pd,
                            double duration)
{
   efl_animation_total_duration_set(eo_obj, duration);

   pd->duration = duration;
}

EOLIAN static double
_efl_animation_duration_get(Eo *eo_obj EINA_UNUSED, Efl_Animation_Data *pd)
{
   return pd->duration;
}

EOLIAN static void
_efl_animation_duration_only_set(Eo *eo_obj EINA_UNUSED,
                                 Efl_Animation_Data *pd,
                                 double duration)
{
   pd->duration = duration;
}

EOLIAN static void
_efl_animation_total_duration_set(Eo *eo_obj EINA_UNUSED,
                                  Efl_Animation_Data *pd,
                                  double total_duration)
{
   pd->total_duration = total_duration;
}

EOLIAN static double
_efl_animation_total_duration_get(Eo *eo_obj EINA_UNUSED,
                                  Efl_Animation_Data *pd)
{
   return pd->total_duration;
}

EOLIAN static void
_efl_animation_final_state_keep_set(Eo *eo_obj EINA_UNUSED,
                                    Efl_Animation_Data *pd,
                                    Eina_Bool keep_final_state)
{
   if (pd->keep_final_state == keep_final_state) return;

   pd->keep_final_state = !!keep_final_state;
}

EOLIAN static Eina_Bool
_efl_animation_final_state_keep_get(Eo *eo_obj EINA_UNUSED,
                                    Efl_Animation_Data *pd)
{
   return pd->keep_final_state;
}

EOLIAN static Efl_Animation_Object *
_efl_animation_object_create(Eo *eo_obj, Efl_Animation_Data *pd EINA_UNUSED)
{
   Efl_Animation_Object *anim_obj
      = efl_add(EFL_ANIMATION_OBJECT_CLASS, NULL);

   Efl_Canvas_Object *target = efl_animation_target_get(eo_obj);
   efl_animation_object_target_set(anim_obj, target);

   Eina_Bool state_keep = efl_animation_final_state_keep_get(eo_obj);
   efl_animation_object_final_state_keep_set(anim_obj, state_keep);

   double duration = efl_animation_duration_get(eo_obj);
   efl_animation_object_duration_set(anim_obj, duration);

   double total_duration = efl_animation_total_duration_get(eo_obj);
   efl_animation_object_total_duration_set(anim_obj, total_duration);

   int repeat_count = efl_animation_repeat_count_get(eo_obj);
   efl_animation_object_repeat_count_set(anim_obj, repeat_count);

   return anim_obj;
}

EOLIAN static void
_efl_animation_repeat_mode_set(Eo *eo_obj EINA_UNUSED,
                               Efl_Animation_Data *pd,
                               Efl_Animation_Repeat_Mode mode)
{
   if ((mode == EFL_ANIMATION_REPEAT_MODE_RESTART) ||
       (mode == EFL_ANIMATION_REPEAT_MODE_REVERSE))
     pd->repeat_mode = mode;
}

EOLIAN static Efl_Animation_Repeat_Mode
_efl_animation_repeat_mode_get(Eo *eo_obj EINA_UNUSED, Efl_Animation_Data *pd)
{
   return pd->repeat_mode;
}

EOLIAN static void
_efl_animation_repeat_count_set(Eo *eo_obj EINA_UNUSED,
                                Efl_Animation_Data *pd,
                                int count)
{
   //EFL_ANIMATION_REPEAT_INFINITE repeats animation infinitely
   if ((count < 0) && (count != EFL_ANIMATION_REPEAT_INFINITE)) return;

   pd->repeat_count = count;
}

EOLIAN static int
_efl_animation_repeat_count_get(Eo *eo_obj EINA_UNUSED, Efl_Animation_Data *pd)
{
   return pd->repeat_count;
}

EOLIAN static void
_efl_animation_start_delay_set(Eo *eo_obj EINA_UNUSED,
                               Efl_Animation_Data *pd,
                               double delay_time)
{
   if (delay_time < 0.0) return;

   pd->start_delay_time = delay_time;
}

EOLIAN static double
_efl_animation_start_delay_get(Eo *eo_obj EINA_UNUSED,
                               Efl_Animation_Data *pd)
{
   return pd->start_delay_time;
}

EOLIAN static void
_efl_animation_interpolator_set(Eo *eo_obj EINA_UNUSED,
                                Efl_Animation_Data *pd,
                                Efl_Interpolator *interpolator)
{
   pd->interpolator = interpolator;
}

EOLIAN static Efl_Interpolator *
_efl_animation_interpolator_get(Eo *eo_obj EINA_UNUSED,
                                Efl_Animation_Data *pd)
{
   return pd->interpolator;
}

EOLIAN static Efl_Object *
_efl_animation_efl_object_constructor(Eo *eo_obj,
                                      Efl_Animation_Data *pd)
{
   eo_obj = efl_constructor(efl_super(eo_obj, MY_CLASS));

   pd->target = NULL;

   pd->duration = 0.0;

   pd->start_delay_time = 0.0;

   pd->repeat_count = 0;

   pd->interpolator = NULL;

   pd->keep_final_state = EINA_FALSE;

   return eo_obj;
}

EOLIAN static void
_efl_animation_efl_object_destructor(Eo *eo_obj, Efl_Animation_Data *pd)
{
   if (pd->target)
     efl_event_callback_del(pd->target, EFL_EVENT_DEL, _target_del_cb, eo_obj);

   efl_destructor(efl_super(eo_obj, MY_CLASS));
}

#include "efl_animation.eo.c"
