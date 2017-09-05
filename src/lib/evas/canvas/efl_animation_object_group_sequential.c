#include "efl_animation_object_group_sequential_private.h"

/* Add member object data and append the data to the member object data list.
 * The member object data contains the repeated count of the member object.
 */
static void
_member_anim_obj_data_add(Efl_Animation_Object_Group_Sequential_Data *pd,
                          Efl_Animation_Object *anim_obj,
                          int repeated_count)
{
   Member_Object_Data *member_anim_obj_data =
      calloc(1, sizeof(Member_Object_Data));

   if (!member_anim_obj_data) return;

   member_anim_obj_data->anim_obj = anim_obj;
   member_anim_obj_data->repeated_count = repeated_count;

   pd->member_anim_obj_data_list =
      eina_list_append(pd->member_anim_obj_data_list, member_anim_obj_data);
}

/* Find the member object data which contains the repeated count of the member
 * object. */
static Member_Object_Data *
_member_anim_obj_data_find(Efl_Animation_Object_Group_Sequential_Data *pd,
                           Efl_Animation_Object *anim_obj)
{
   Eina_List *l;
   Member_Object_Data *member_anim_obj_data = NULL;
   EINA_LIST_FOREACH(pd->member_anim_obj_data_list, l, member_anim_obj_data)
     {
        if (member_anim_obj_data->anim_obj == anim_obj)
          break;
     }

   return member_anim_obj_data;
}

/* Delete member object data and remove the data from the member object data
 * list.
 * The member object data contains the repeated count of the member object.
 */
static void
_member_anim_obj_data_del(Efl_Animation_Object_Group_Sequential_Data *pd,
                          Efl_Animation_Object *anim_obj)
{
   Member_Object_Data *member_anim_obj_data =
      _member_anim_obj_data_find(pd, anim_obj);
   if (member_anim_obj_data)
     {
        pd->member_anim_obj_data_list =
           eina_list_remove(pd->member_anim_obj_data_list,
                            member_anim_obj_data);
        free(member_anim_obj_data);
     }
}

EOLIAN static void
_efl_animation_object_group_sequential_efl_animation_object_group_object_add(Eo *eo_obj,
                                                                             Efl_Animation_Object_Group_Sequential_Data *pd EINA_UNUSED,
                                                                             Efl_Animation_Object *anim_obj)
{
   EFL_ANIMATION_OBJECT_GROUP_SEQUENTIAL_CHECK_OR_RETURN(eo_obj);

   if (!anim_obj) return;

   efl_animation_object_group_object_add(efl_super(eo_obj, MY_CLASS), anim_obj);

   /* Add member object data and append the data to the member object data
    * list. */
   _member_anim_obj_data_add(pd, anim_obj, 0);

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

   /* Delete member object data and remove the data from the member object
    * data list. */
   _member_anim_obj_data_del(pd, anim_obj);

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
        double child_total_duration =
           efl_animation_object_total_duration_get(anim_obj);

        double start_delay = efl_animation_object_start_delay_get(anim_obj);
        if (start_delay > 0.0)
          child_total_duration += start_delay;

        int child_repeat_count =
           efl_animation_object_repeat_count_get(anim_obj);
        if (child_repeat_count > 0)
          child_total_duration *= (child_repeat_count + 1);

        total_duration += child_total_duration;
     }
   return total_duration;
}

//Set how many times the given object has been repeated.
static void
_repeated_count_set(Efl_Animation_Object_Group_Sequential_Data *pd,
                    Efl_Animation_Object *anim_obj,
                    int repeated_count)
{

   Member_Object_Data *member_anim_obj_data = _member_anim_obj_data_find(pd, anim_obj);
   if (!member_anim_obj_data) return;

   member_anim_obj_data->repeated_count = repeated_count;
}

//Get how many times the given object has been repeated.
static int
_repeated_count_get(Efl_Animation_Object_Group_Sequential_Data *pd,
                    Efl_Animation_Object *anim_obj)
{
   Member_Object_Data *member_anim_obj_data =
      _member_anim_obj_data_find(pd, anim_obj);
   if (!member_anim_obj_data) return 0;

   return member_anim_obj_data->repeated_count;
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
        double start_delay = efl_animation_object_start_delay_get(anim_obj);
        double anim_obj_progress;

        Eina_Bool start_repeat = EINA_FALSE;

        if (total_duration == 0.0)
          anim_obj_progress = 1.0;
        else
          {
             //If object is repeated, then recalculate progress.
             int repeated_count = _repeated_count_get(pd, anim_obj);
             if (repeated_count > 0)
               sum_prev_total_duration +=
                  ((total_duration + start_delay) * repeated_count);

             double elapsed_time_without_delay =
                elapsed_time - sum_prev_total_duration - start_delay;

             //Object should not start to wait for start delay time.
             if (elapsed_time_without_delay < 0.0) break;

             anim_obj_progress = elapsed_time_without_delay / total_duration;

             if (anim_obj_progress > 1.0)
               anim_obj_progress = 1.0;

             //Animation has been finished.
             if (anim_obj_progress == 1.0)
               {
                  /* If object is finished and it should be repeated, then
                   * increate the repeated count to recalculate progress. */
                  int repeat_count =
                     efl_animation_object_repeat_count_get(anim_obj);
                  if (repeat_count > 0)
                    {
                       int repeated_count = _repeated_count_get(pd, anim_obj);
                       if (repeated_count < repeat_count)
                         {
                            repeated_count++;
                            _repeated_count_set(pd, anim_obj, repeated_count);

                            start_repeat = EINA_TRUE;
                         }
                    }
               }
          }

        /* Update the sum of the previous objects' total durations and start
         * delays */
        sum_prev_total_duration += (total_duration + start_delay);

        if ((anim_obj_progress == 1.0) && (!start_repeat) &&
            (!efl_animation_object_final_state_keep_get(anim_obj)))
          continue;

        /* If object is repeated with reverse mode, then the progress value
         * should be modified as (1.0 - progress). */
        Efl_Animation_Object_Repeat_Mode repeat_mode
           = efl_animation_object_repeat_mode_get(anim_obj);
        if (repeat_mode == EFL_ANIMATION_OBJECT_REPEAT_MODE_REVERSE)
          {
             int repeated_count = _repeated_count_get(pd, anim_obj);
             if (repeated_count > 0)
               {
                  if ((((repeated_count % 2) == 1) && (!start_repeat)) ||
                      (((repeated_count % 2) == 0) && (start_repeat)))
                    anim_obj_progress = 1.0 - anim_obj_progress;
               }
          }

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
