#include "efl_animation_object_group_sequential_private.h"

EOLIAN static void
_efl_animation_object_group_sequential_efl_animation_object_group_object_add(Eo *eo_obj,
                                                                             Efl_Animation_Object_Group_Sequential_Data *pd EINA_UNUSED,
                                                                             Efl_Animation_Object *anim_obj)
{
   EFL_ANIMATION_OBJECT_GROUP_SEQUENTIAL_CHECK_OR_RETURN(eo_obj);

   if (!anim_obj) return;

   efl_animation_object_group_object_add(efl_super(eo_obj, MY_CLASS), anim_obj);

   /* Total duration is calculated in
    * efl_animation_object_total_duration_get() based on the current group
    * animation object list.
    * Therefore, the calculated total duration should be set to update total
    * duration. */
   double total_duration = efl_animation_object_total_duration_get(eo_obj);
   efl_animation_object_total_duration_set(eo_obj, total_duration);
}

EOLIAN static void
_efl_animation_object_group_sequential_efl_animation_object_group_object_del(Eo *eo_obj,
                                                                             Efl_Animation_Object_Group_Sequential_Data *pd EINA_UNUSED,
                                                                             Efl_Animation_Object *anim_obj)
{
   EFL_ANIMATION_OBJECT_GROUP_SEQUENTIAL_CHECK_OR_RETURN(eo_obj);

   if (!anim_obj) return;

   efl_animation_object_group_object_del(efl_super(eo_obj, MY_CLASS), anim_obj);

   /* Total duration is calculated in
    * efl_animation_object_total_duration_get() based on the current group
    * animation object list.
    * Therefore, the calculated total duration should be set to update total
    * duration. */
   double total_duration = efl_animation_object_total_duration_get(eo_obj);
   efl_animation_object_total_duration_set(eo_obj, total_duration);
}

EOLIAN static double
_efl_animation_object_group_sequential_efl_animation_object_total_duration_get(Eo *eo_obj,
                                                                               Efl_Animation_Object_Group_Sequential_Data *pd EINA_UNUSED)
{
   EFL_ANIMATION_OBJECT_GROUP_SEQUENTIAL_CHECK_OR_RETURN(eo_obj, 0.0);

   Eina_List *anim_objs = efl_animation_object_group_objects_get(eo_obj);
   if (!anim_objs) return 0.0;

   double total_duration = 0.0;
   Eina_List *l;
   Efl_Animation *anim_obj;
   EINA_LIST_FOREACH(anim_objs, l, anim_obj)
     {
        total_duration += efl_animation_object_total_duration_get(anim_obj);
     }
   return total_duration;
}

EOLIAN static void
_efl_animation_object_group_sequential_efl_animation_object_progress_set(Eo *eo_obj,
                                                                         Efl_Animation_Object_Group_Sequential_Data *pd EINA_UNUSED,
                                                                         double progress)
{
   if ((progress < 0.0) || (progress > 1.0)) return;

   Eina_List *anim_objs = efl_animation_object_group_objects_get(eo_obj);
   if (!anim_objs) return;

   double group_total_duration =
      efl_animation_object_total_duration_get(eo_obj);

   double elapsed_time = progress * group_total_duration;

   double sum_prev_total_duration = 0.0;

   Eina_List *l;
   Efl_Animation_Object *anim_obj;
   EINA_LIST_FOREACH(anim_objs, l, anim_obj)
     {
        //Current animation object does not start
        if (sum_prev_total_duration > elapsed_time) break;

        //Sum the current total duration
        double total_duration =
           efl_animation_object_total_duration_get(anim_obj);
        double anim_obj_progress;

        if (total_duration == 0.0)
          anim_obj_progress = 1.0;
        else
          {
             anim_obj_progress =
                (elapsed_time - sum_prev_total_duration) / total_duration;
             if (anim_obj_progress > 1.0)
               anim_obj_progress = 1.0;
          }

        //Update the sum of the previous objects' total durations
        sum_prev_total_duration += total_duration;

        if ((anim_obj_progress == 1.0) &&
            !efl_animation_object_final_state_keep_get(anim_obj))
          continue;

        efl_animation_object_progress_set(anim_obj, anim_obj_progress);
     }

   efl_animation_object_progress_set(efl_super(eo_obj, MY_CLASS), progress);
}

/* Internal EO APIs */

#define EFL_ANIMATION_OBJECT_GROUP_SEQUENTIAL_EXTRA_OPS \
   EFL_OBJECT_OP_FUNC(efl_animation_object_group_object_add, _efl_animation_object_group_sequential_efl_animation_object_group_object_add), \
   EFL_OBJECT_OP_FUNC(efl_animation_object_group_object_del, _efl_animation_object_group_sequential_efl_animation_object_group_object_del), \
   EFL_OBJECT_OP_FUNC(efl_animation_object_total_duration_get, _efl_animation_object_group_sequential_efl_animation_object_total_duration_get)

#include "efl_animation_object_group_sequential.eo.c"
