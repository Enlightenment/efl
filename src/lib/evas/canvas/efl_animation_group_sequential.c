#include "efl_animation_group_sequential_private.h"

#define MY_CLASS EFL_ANIMATION_GROUP_SEQUENTIAL_CLASS

EOLIAN static double
_efl_animation_group_sequential_efl_animation_animation_apply(Eo *eo_obj,
                                                              void *_pd EINA_UNUSED,
                                                              double progress,
                                                              Efl_Canvas_Object *target)
{
   double group_length, group_elapsed_time;
   double anim_length, anim_duration, anim_start_delay, anim_progress, anim_play_time, anim_position;
   double total_anim_elapsed_time = 0.0;
   double temp;
   int anim_repeated_count;

   progress = efl_animation_apply(efl_super(eo_obj, MY_CLASS), progress, target);
   Eina_List *group_anim = efl_animation_group_animations_get(eo_obj);
   if (!group_anim) return progress;

   group_length = efl_playable_length_get(eo_obj);
   group_elapsed_time = group_length * progress;

   Eina_List *l;
   Efl_Animation *anim;
   EINA_LIST_FOREACH(group_anim, l, anim)
     {
        anim_start_delay = efl_animation_start_delay_get(anim);
        anim_length = efl_playable_length_get(anim) + anim_start_delay;
        anim_duration = efl_animation_duration_get(anim);

        //Check whether this animation is playing.
        temp = total_anim_elapsed_time + anim_length + anim_start_delay;
        if (temp <= group_elapsed_time)
          {
             if (efl_animation_final_state_keep_get(anim) && (!FINAL_STATE_IS_REVERSE(anim)))
               anim_progress = 1.0;
             else
               anim_progress = 0.0;
             efl_animation_apply(anim, anim_progress, target);
             total_anim_elapsed_time = temp;
             continue;
          }

        anim_play_time = group_elapsed_time - total_anim_elapsed_time - anim_start_delay;
        //TODO: check infinite repeat
        anim_repeated_count = (int)(anim_play_time / anim_length);
        anim_position = MAX(((anim_play_time - anim_duration * anim_repeated_count)), 0.0);
        anim_progress = MIN((anim_position / anim_duration), 1.0);
        if (FINAL_STATE_IS_REVERSE(anim))
          anim_progress = 1.0 - anim_progress;
        efl_animation_apply(anim, anim_progress, target);

        break;
     }

   return progress;
}

EOLIAN static double
_efl_animation_group_sequential_efl_animation_duration_get(Eo *eo_obj, void *_pd EINA_UNUSED)
{
   double total_duration = 0.0;
   double child_total_duration;

   Eina_List *animations = efl_animation_group_animations_get(eo_obj);
   if (!animations) return 0.0;

   Eina_List *l;
   Efl_Animation *anim;
   EINA_LIST_FOREACH(animations, l, anim)
     {
        child_total_duration = efl_playable_length_get(anim);
        child_total_duration += efl_animation_start_delay_get(anim);
        total_duration += child_total_duration;
     }

   return total_duration;
}

#include "efl_animation_group_sequential.eo.c"
