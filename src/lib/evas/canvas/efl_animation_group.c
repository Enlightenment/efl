#include "efl_animation_group_private.h"

EOLIAN static void
_efl_animation_group_animation_add(Eo *eo_obj,
                                   Efl_Animation_Group_Data *pd,
                                   Efl_Animation *animation)
{
   EFL_ANIMATION_GROUP_CHECK_OR_RETURN(eo_obj);

   if (!animation) return;

   Efl_Canvas_Object *target = efl_animation_target_get(eo_obj);
   if (target)
     efl_animation_target_set(animation, target);

   double duration = efl_animation_duration_get(eo_obj);
   /* if group animation duration is available value, then the duration is
    * propagated to its child. */
   if (duration != EFL_ANIMATION_GROUP_DURATION_NONE)
     efl_animation_duration_set(animation, duration);

   Eina_Bool keep_final_state = efl_animation_final_state_keep_get(eo_obj);
   efl_animation_final_state_keep_set(animation, keep_final_state);

   pd->animations = eina_list_append(pd->animations, animation);
}

EOLIAN static void
_efl_animation_group_animation_del(Eo *eo_obj,
                                   Efl_Animation_Group_Data *pd,
                                   Efl_Animation*animation)
{
   EFL_ANIMATION_GROUP_CHECK_OR_RETURN(eo_obj);

   if (!animation) return;

   pd->animations = eina_list_remove(pd->animations, animation);
}

EOLIAN static Eina_List *
_efl_animation_group_animations_get(Eo *eo_obj,
                                    Efl_Animation_Group_Data *pd)
{
   EFL_ANIMATION_GROUP_CHECK_OR_RETURN(eo_obj, NULL);

   return pd->animations;
}

EOLIAN static void
_efl_animation_group_efl_animation_target_set(Eo *eo_obj,
                                              Efl_Animation_Group_Data *pd,
                                              Efl_Canvas_Object *target)
{
   EFL_ANIMATION_GROUP_CHECK_OR_RETURN(eo_obj);

   Eina_List *l;
   Efl_Animation *anim;
   EINA_LIST_FOREACH(pd->animations, l, anim)
     {
        efl_animation_target_set(anim, target);
     }

   efl_animation_target_set(efl_super(eo_obj, MY_CLASS), target);
}

EOLIAN static void
_efl_animation_group_efl_animation_duration_set(Eo *eo_obj,
                                                Efl_Animation_Group_Data *pd,
                                                double duration)
{
   EFL_ANIMATION_GROUP_CHECK_OR_RETURN(eo_obj);

   if (duration == EFL_ANIMATION_GROUP_DURATION_NONE) goto end;

   if (duration < 0.0) return;

   Eina_List *l;
   Efl_Animation *anim;
   EINA_LIST_FOREACH(pd->animations, l, anim)
     {
        efl_animation_duration_set(anim, duration);
     }

end:
   efl_animation_duration_only_set(efl_super(eo_obj, MY_CLASS), duration);

   //efl_animation_total_duration_get() should calculate the new total duration.
   double total_duration = efl_animation_total_duration_get(eo_obj);
   efl_animation_total_duration_set(eo_obj, total_duration);
}

EOLIAN static void
_efl_animation_group_efl_animation_final_state_keep_set(Eo *eo_obj,
                                                        Efl_Animation_Group_Data *pd,
                                                        Eina_Bool keep_final_state)
{
   EFL_ANIMATION_GROUP_CHECK_OR_RETURN(eo_obj);

   Eina_List *l;
   Efl_Animation *anim;
   EINA_LIST_FOREACH(pd->animations, l, anim)
     {
        efl_animation_final_state_keep_set(anim, keep_final_state);
     }

   efl_animation_final_state_keep_set(efl_super(eo_obj, MY_CLASS), keep_final_state);
}

EOLIAN static void
_efl_animation_group_efl_animation_interpolator_set(Eo *eo_obj,
                                                    Efl_Animation_Group_Data *pd,
                                                    Efl_Interpolator *interpolator)
{
   EFL_ANIMATION_GROUP_CHECK_OR_RETURN(eo_obj);

   Eina_List *l;
   Efl_Animation *anim;
   EINA_LIST_FOREACH(pd->animations, l, anim)
     {
        efl_animation_interpolator_set(anim, interpolator);
     }

   efl_animation_interpolator_set(efl_super(eo_obj, MY_CLASS), interpolator);
}

EOLIAN static Efl_Object *
_efl_animation_group_efl_object_constructor(Eo *eo_obj,
                                            Efl_Animation_Group_Data *pd)
{
   eo_obj = efl_constructor(efl_super(eo_obj, MY_CLASS));

   pd->animations = NULL;

   //group animation does not affect its child duration by default.
   efl_animation_duration_only_set(efl_super(eo_obj, MY_CLASS),
                                   EFL_ANIMATION_GROUP_DURATION_NONE);

   return eo_obj;
}

EOLIAN static void
_efl_animation_group_efl_object_destructor(Eo *eo_obj,
                                           Efl_Animation_Group_Data *pd)
{
   Efl_Animation *anim;

   EINA_LIST_FREE(pd->animations, anim)
      efl_del(anim);

   efl_destructor(efl_super(eo_obj, MY_CLASS));
}

#include "efl_animation_group.eo.c"
