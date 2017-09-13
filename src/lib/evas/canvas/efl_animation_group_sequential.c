#include "efl_animation_group_sequential_private.h"

EOLIAN static void
_efl_animation_group_sequential_efl_animation_group_animation_add(Eo *eo_obj,
                                                                  Efl_Animation_Group_Sequential_Data *pd EINA_UNUSED,
                                                                  Efl_Animation *animation)
{
   EFL_ANIMATION_GROUP_SEQUENTIAL_CHECK_OR_RETURN(eo_obj);

   if (!animation) return;

   efl_animation_group_animation_add(efl_super(eo_obj, MY_CLASS), animation);

   /* Total duration is calculated in efl_animation_total_duration_get() based
    * on the current group animation list.
    * Therefore, the calculated total duration should be set to update total
    * duration. */
   double total_duration = efl_animation_total_duration_get(eo_obj);
   efl_animation_total_duration_set(eo_obj, total_duration);
}

EOLIAN static void
_efl_animation_group_sequential_efl_animation_group_animation_del(Eo *eo_obj,
                                                                  Efl_Animation_Group_Sequential_Data *pd EINA_UNUSED,
                                                                  Efl_Animation *animation)
{
   EFL_ANIMATION_GROUP_SEQUENTIAL_CHECK_OR_RETURN(eo_obj);

   if (!animation) return;

   efl_animation_group_animation_del(efl_super(eo_obj, MY_CLASS), animation);

   /* Total duration is calculated in efl_animation_total_duration_get() based
    * on the current group animation list.
    * Therefore, the calculated total duration should be set to update total
    * duration. */
   double total_duration = efl_animation_total_duration_get(eo_obj);
   efl_animation_total_duration_set(eo_obj, total_duration);
}

EOLIAN static double
_efl_animation_group_sequential_efl_animation_total_duration_get(Eo *eo_obj,
                                                                 Efl_Animation_Group_Sequential_Data *pd EINA_UNUSED)
{
   EFL_ANIMATION_GROUP_SEQUENTIAL_CHECK_OR_RETURN(eo_obj, 0.0);

   Eina_List *animations = efl_animation_group_animations_get(eo_obj);
   if (!animations) return 0.0;

   double total_duration = 0.0;
   Eina_List *l;
   Efl_Animation *anim;
   EINA_LIST_FOREACH(animations, l, anim)
     {
        double child_total_duration = efl_animation_total_duration_get(anim);

        double start_delay = efl_animation_start_delay_get(anim);
        if (start_delay > 0.0)
          child_total_duration += start_delay;

        int child_repeat_count = efl_animation_repeat_count_get(anim);
        if (child_repeat_count > 0)
          child_total_duration *= (child_repeat_count + 1);

        total_duration += child_total_duration;
     }
   return total_duration;
}

EOLIAN static Efl_Animation_Object *
_efl_animation_group_sequential_efl_animation_object_create(Eo *eo_obj,
                                                            Efl_Animation_Group_Sequential_Data *pd EINA_UNUSED)
{
   EFL_ANIMATION_GROUP_SEQUENTIAL_CHECK_OR_RETURN(eo_obj, NULL);

   Efl_Animation_Object_Group_Sequential *group_anim_obj
      = efl_add(EFL_ANIMATION_OBJECT_GROUP_SEQUENTIAL_CLASS, NULL);

   Eina_List *animations = efl_animation_group_animations_get(eo_obj);
   Eina_List *l;
   Efl_Animation *child_anim;
   Efl_Animation_Object *child_anim_obj;

   EINA_LIST_FOREACH(animations, l, child_anim)
     {
        child_anim_obj = efl_animation_object_create(child_anim);
        efl_animation_object_group_object_add(group_anim_obj, child_anim_obj);
     }

   Efl_Canvas_Object *target = efl_animation_target_get(eo_obj);
   if (target)
     efl_animation_object_target_set(group_anim_obj, target);

   Eina_Bool state_keep = efl_animation_final_state_keep_get(eo_obj);
   efl_animation_object_final_state_keep_set(group_anim_obj, state_keep);

   double duration = efl_animation_duration_get(eo_obj);
   efl_animation_object_duration_set(group_anim_obj, duration);

   double total_duration = efl_animation_total_duration_get(eo_obj);
   efl_animation_object_total_duration_set(group_anim_obj, total_duration);

   double start_delay_time = efl_animation_start_delay_get(eo_obj);
   efl_animation_object_start_delay_set(group_anim_obj, start_delay_time);

   Efl_Animation_Object_Repeat_Mode repeat_mode =
      (Efl_Animation_Object_Repeat_Mode)efl_animation_repeat_mode_get(eo_obj);
   efl_animation_object_repeat_mode_set(group_anim_obj, repeat_mode);

   int repeat_count = efl_animation_repeat_count_get(eo_obj);
   efl_animation_object_repeat_count_set(group_anim_obj, repeat_count);

   Efl_Interpolator *interpolator = efl_animation_interpolator_get(eo_obj);
   efl_animation_object_interpolator_set(group_anim_obj, interpolator);

   return group_anim_obj;
}

#include "efl_animation_group_sequential.eo.c"
