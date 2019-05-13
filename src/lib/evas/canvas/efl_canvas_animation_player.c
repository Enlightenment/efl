#include "efl_canvas_animation_player_private.h"

static void
_target_del_cb(void *data, const Efl_Event *event EINA_UNUSED)
{
   Efl_Canvas_Animation_Player_Data *pd = data;
   pd->target = NULL;
}

EOLIAN static void
_efl_canvas_animation_player_target_set(Eo *eo_obj EINA_UNUSED,
                                 Efl_Canvas_Animation_Player_Data *pd,
                                 Efl_Canvas_Object *target)
{
   efl_event_callback_add(target, EFL_EVENT_DEL, _target_del_cb, pd);

   pd->target = target;
}

EOLIAN static Efl_Canvas_Object *
_efl_canvas_animation_player_target_get(const Eo *eo_obj EINA_UNUSED, Efl_Canvas_Animation_Player_Data *pd)
{
   return pd->target;
}

EOLIAN static void
_efl_canvas_animation_player_auto_del_set(Eo *eo_obj EINA_UNUSED,
                                   Efl_Canvas_Animation_Player_Data *pd,
                                   Eina_Bool auto_del)
{
   pd->auto_del = auto_del;
}

EOLIAN static Eina_Bool
_efl_canvas_animation_player_auto_del_get(const Eo *eo_obj EINA_UNUSED,
                                   Efl_Canvas_Animation_Player_Data *pd)
{
   return pd->auto_del;
}

EOLIAN static void
_efl_canvas_animation_player_animation_set(Eo *eo_obj,
                                    Efl_Canvas_Animation_Player_Data *pd,
                                    Efl_Canvas_Animation *anim)
{
   if (anim == pd->animation)
     return;

   if (!efl_isa(anim, EFL_CANVAS_ANIMATION_CLASS))
     {
        ERR("Passed argument [%p]:[%s] is not an Efl.Animation",
             anim, efl_class_name_get(efl_class_get(anim)));
        return;
     }

   if (pd->animation)
     {
        efl_player_stop(eo_obj);
        efl_unref(pd->animation);
     }
   pd->animation = anim;
   efl_ref(pd->animation);
}

EOLIAN static Efl_Canvas_Animation *
_efl_canvas_animation_player_animation_get(const Eo *eo_obj EINA_UNUSED,
                                    Efl_Canvas_Animation_Player_Data *pd)
{
   return pd->animation;
}

static Eina_Bool
_animator_cb(void *data)
{
   Eo *eo_obj = data;
   EFL_ANIMATION_PLAYER_DATA_GET(eo_obj, pd);
   EFL_ANIMATION_PLAYER_ANIMATION_GET(eo_obj, anim);
   double duration, elapsed_time, vector;

   if (efl_player_seekable_get(eo_obj))
     {
        pd->time.current = ecore_loop_time_get();

        duration = efl_animation_duration_get(anim);
        elapsed_time = pd->time.current - pd->time.prev;
        vector = elapsed_time / duration;

        /* When animation player starts, _animator_cb() is called immediately so
         * both elapsed time and progress are 0.0.
         * Since it is the beginning of the animation if progress is 0.0, the
         * following codes for animation should be executed. */
        if ((vector <= DBL_EPSILON) && (pd->progress != 0.0))
          return ECORE_CALLBACK_RENEW; // There is no update.

        //TODO: check negative play_speed.
        if (!pd->is_direction_forward)
          vector *= -1;
        pd->progress += vector;

        if (pd->progress > 1.0)
          pd->progress = 1.0;
        else if (pd->progress < 0.0)
          pd->progress = 0.0;
     }
   else
     {
        pd->progress = (double)(pd->is_direction_forward);
     }

   efl_animation_apply(anim, pd->progress, efl_animation_player_target_get(eo_obj));

   Efl_Canvas_Animation_Player_Event_Running event_running;
   event_running.progress = pd->progress;
   efl_event_callback_call(eo_obj, EFL_ANIMATION_PLAYER_EVENT_RUNNING,
                           &event_running);
   pd->time.prev = pd->time.current;

   //Not end. Keep going.
   if (fabs((!!(pd->is_direction_forward)) - pd->progress) > DBL_EPSILON)
     return ECORE_CALLBACK_RENEW;

   //Repeat animation
   if ((efl_animation_repeat_count_get(anim) == EFL_ANIMATION_REPEAT_INFINITE) ||
       (pd->remaining_repeat_count > 0))
     {
        if (pd->remaining_repeat_count > 0)
          pd->remaining_repeat_count--;

        if (efl_animation_repeat_mode_get(anim) == EFL_CANVAS_ANIMATION_REPEAT_MODE_REVERSE)
          {
             pd->is_direction_forward = !pd->is_direction_forward;
          }
        else
          {
             pd->progress = 0.0;
          }

        return ECORE_CALLBACK_RENEW;
     }
   efl_player_stop(eo_obj);

   return ECORE_CALLBACK_CANCEL;
}

static void
_start(Eo *eo_obj, Efl_Canvas_Animation_Player_Data *pd)
{
   EFL_ANIMATION_PLAYER_ANIMATION_GET(eo_obj, anim);

   pd->is_direction_forward = EINA_TRUE;

   pd->remaining_repeat_count = efl_animation_repeat_count_get(anim);

   ecore_animator_del(pd->animator);
   pd->animator = NULL;
   pd->time.prev = ecore_loop_time_get();

   //pre started event is supported within class only (protected event)
   efl_event_callback_call(eo_obj, EFL_ANIMATION_PLAYER_EVENT_PRE_STARTED,
                           NULL);
   efl_event_callback_call(eo_obj, EFL_ANIMATION_PLAYER_EVENT_STARTED, NULL);

   pd->animator = ecore_evas_animator_add(pd->target, _animator_cb, eo_obj);

   _animator_cb(eo_obj);
}

static Eina_Bool
_start_delay_timer_cb(void *data)
{
   Eo *eo_obj = data;
   EFL_ANIMATION_PLAYER_DATA_GET(eo_obj, pd);

   pd->start_delay_timer = NULL;

   _start(eo_obj, pd);

   return ECORE_CALLBACK_CANCEL;
}

EOLIAN static void
_efl_canvas_animation_player_efl_player_start(Eo *eo_obj,
                                       Efl_Canvas_Animation_Player_Data *pd)
{
   double start_delay;
   EFL_ANIMATION_PLAYER_ANIMATION_GET(eo_obj, anim);

   if (!efl_player_playable_get(eo_obj)) return;
   pd->is_play = EINA_TRUE;
   //TODO: check this case is correct
   if (pd->start_delay_timer) return;

   pd->progress = 0.0;
   start_delay = efl_animation_start_delay_get(anim);
   if (start_delay > 0.0)
     {
        pd->start_delay_timer = ecore_timer_add(start_delay,
                                                _start_delay_timer_cb, eo_obj);
        return;
     }

   _start(eo_obj, pd);
}

EOLIAN static void
_efl_canvas_animation_player_efl_player_stop(Eo *eo_obj,
                                      Efl_Canvas_Animation_Player_Data *pd)
{
   EFL_ANIMATION_PLAYER_ANIMATION_GET(eo_obj, anim);
   Eina_Bool play = efl_player_play_get(eo_obj);
   if (play)
     {
        efl_player_play_set(eo_obj, EINA_FALSE);
        //Reset the state of the target to the initial state
        if ((efl_animation_final_state_keep_get(anim)) &&
            (efl_animation_repeat_mode_get(anim) != EFL_CANVAS_ANIMATION_REPEAT_MODE_REVERSE) &&
            (!(efl_animation_repeat_count_get(anim) & 1)))
          {
               pd->progress = 1.0;
               efl_animation_apply(anim, pd->progress,
                                   efl_animation_player_target_get(eo_obj));
          }
        else
          {
             pd->progress = 0.0;
             efl_gfx_mapping_reset(efl_animation_player_target_get(eo_obj));
          }
        efl_event_callback_call(eo_obj, EFL_ANIMATION_PLAYER_EVENT_ENDED, NULL);
     }
   else
     {
         pd->progress = 0.0;
         efl_gfx_mapping_reset(efl_animation_player_target_get(eo_obj));
     }

   if (pd->auto_del) efl_del(eo_obj);
}

EOLIAN static void
_efl_canvas_animation_player_efl_player_play_set(Eo *eo_obj,
                                          Efl_Canvas_Animation_Player_Data *pd,
                                          Eina_Bool play)
{
   if (efl_player_play_get(eo_obj) == !!play)
     return;

   pd->is_play = play;
   if (play)
     {
        //TODO: check this case is correct.
        if (pd->start_delay_timer) return;

        pd->time.prev = ecore_loop_time_get();
        pd->animator = ecore_evas_animator_add(pd->target, _animator_cb, eo_obj);

        _animator_cb(eo_obj);
     }
   else
     {
        ecore_timer_del(pd->start_delay_timer);
        pd->start_delay_timer = NULL;
        ecore_animator_del(pd->animator);
        pd->animator = NULL;
     }
}

EOLIAN static Eina_Bool
_efl_canvas_animation_player_efl_player_play_get(const Eo *eo_obj EINA_UNUSED,
                                          Efl_Canvas_Animation_Player_Data *pd)
{
   return pd->is_play;
}

EOLIAN static Eina_Bool
_efl_canvas_animation_player_efl_player_playable_get(const Eo *eo_obj,
                                              Efl_Canvas_Animation_Player_Data *pd EINA_UNUSED)
{
   Efl_Canvas_Animation *anim = efl_animation_player_animation_get(eo_obj);

   return efl_playable_get(anim);
}

EOLIAN static double
_efl_canvas_animation_player_efl_player_pos_get(const Eo *eo_obj,
                                              Efl_Canvas_Animation_Player_Data *pd EINA_UNUSED)
{
   //TODO: this is not correct
   Efl_Canvas_Animation *anim = efl_animation_player_animation_get(eo_obj);
   double length = efl_animation_duration_get(anim);

   return length * efl_player_progress_get(eo_obj);
}

EOLIAN static void
_efl_canvas_animation_player_efl_player_pos_set(Eo *eo_obj,
                                              Efl_Canvas_Animation_Player_Data *pd EINA_UNUSED,
                                              double sec)
{
   //TODO: this is not correct
   if (!efl_player_seekable_get(eo_obj))
     return;

   EFL_ANIMATION_PLAYER_ANIMATION_GET(eo_obj, anim);
   double length = efl_animation_duration_get(anim);
   pd->progress = sec / length;
   efl_animation_apply(anim, pd->progress, efl_animation_player_target_get(eo_obj));
}

EOLIAN static double
_efl_canvas_animation_player_efl_player_progress_get(const Eo *eo_obj EINA_UNUSED,
                                              Efl_Canvas_Animation_Player_Data *pd)
{
   return pd->progress;
}

EOLIAN static void
_efl_canvas_animation_player_efl_player_play_speed_set(Eo *eo_obj EINA_UNUSED,
                                                Efl_Canvas_Animation_Player_Data *pd,
                                                double play_speed)
{
   //TODO: check reverse play case.
   if (play_speed < 0)
     return;
   pd->play_speed = play_speed;
}

EOLIAN static double
_efl_canvas_animation_player_efl_player_play_speed_get(const Eo *eo_obj EINA_UNUSED,
                                                Efl_Canvas_Animation_Player_Data *pd)
{
   return pd->play_speed;
}

EOLIAN static double
_efl_canvas_animation_player_efl_player_length_get(const Eo *eo_obj,
                                              Efl_Canvas_Animation_Player_Data *pd EINA_UNUSED)
{
   EFL_ANIMATION_PLAYER_ANIMATION_GET(eo_obj, anim);
   return efl_playable_length_get(anim);
}

EOLIAN static Eina_Bool
_efl_canvas_animation_player_efl_player_seekable_get(const Eo *eo_obj EINA_UNUSED,
                                              Efl_Canvas_Animation_Player_Data *pd EINA_UNUSED)
{
   EFL_ANIMATION_PLAYER_ANIMATION_GET(eo_obj, anim);
   return efl_playable_seekable_get(anim);
}

EOLIAN static Efl_Object *
_efl_canvas_animation_player_efl_object_constructor(Eo *eo_obj,
                                             Efl_Canvas_Animation_Player_Data *pd)
{
   eo_obj = efl_constructor(efl_super(eo_obj, MY_CLASS));

   pd->time.begin = 0.0;
   pd->time.current = 0.0;

   pd->animation = NULL;

   pd->progress = 0.0;

   //pd->auto_del = EINA_TRUE;

   return eo_obj;
}

EOLIAN static void
_efl_canvas_animation_player_efl_object_destructor(Eo *eo_obj,
                                            Efl_Canvas_Animation_Player_Data *pd)
{
   if (pd->animator)
     {
        ecore_animator_del(pd->animator);
        pd->animator = NULL;

        //Reset the state of the target to the initial state
        efl_player_stop(eo_obj);

        efl_event_callback_call(eo_obj, EFL_ANIMATION_PLAYER_EVENT_ENDED, NULL);
     }
   efl_unref(pd->animation);

   efl_destructor(efl_super(eo_obj, MY_CLASS));
}

EWAPI const Efl_Event_Description _EFL_ANIMATION_PLAYER_EVENT_PRE_STARTED =
   EFL_EVENT_DESCRIPTION("pre_started");

#include "efl_canvas_animation_player.eo.c"
