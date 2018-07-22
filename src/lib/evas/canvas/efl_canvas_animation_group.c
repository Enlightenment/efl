#include "efl_canvas_animation_group_private.h"

EOLIAN static void
_efl_canvas_animation_group_animation_add(Eo *eo_obj,
                                   Efl_Canvas_Animation_Group_Data *pd,
                                   Efl_Canvas_Animation *animation)
{
   if (!animation) return;

   double duration = efl_animation_duration_get(efl_super(eo_obj, MY_CLASS));
   /* if group animation duration is available value, then the duration is
    * propagated to its child. */
   if (duration != EFL_ANIMATION_GROUP_DURATION_NONE)
     efl_animation_duration_set(animation, duration);

   Eina_Bool keep_final_state = efl_animation_final_state_keep_get(eo_obj);
   efl_animation_final_state_keep_set(animation, keep_final_state);

   pd->animations = eina_list_append(pd->animations, animation);
   efl_ref(animation);
}

EOLIAN static void
_efl_canvas_animation_group_animation_del(Eo *eo_obj EINA_UNUSED,
                                   Efl_Canvas_Animation_Group_Data *pd,
                                   Efl_Canvas_Animation *animation)
{
   Eina_List *list;
   if (!animation) return;

   list = eina_list_data_find_list(pd->animations, animation);
   if (list)
     {
        pd->animations = eina_list_remove_list(pd->animations, list);
        efl_unref(animation);
     }
   else
     {
        ERR("Animation(%s@%p) is not in the group animation.",
            efl_class_name_get(animation), animation);
     }
}

EOLIAN static Eina_List *
_efl_canvas_animation_group_animations_get(const Eo *eo_obj EINA_UNUSED,
                                    Efl_Canvas_Animation_Group_Data *pd)
{
   return pd->animations;
}

EOLIAN static void
_efl_canvas_animation_group_efl_canvas_animation_duration_set(Eo *eo_obj,
                                                Efl_Canvas_Animation_Group_Data *pd,
                                                double duration)
{
   if (duration == EFL_ANIMATION_GROUP_DURATION_NONE) goto end;

   if (duration < 0.0) return;

   Eina_List *l;
   Efl_Canvas_Animation *anim;
   EINA_LIST_FOREACH(pd->animations, l, anim)
     {
        efl_animation_duration_set(anim, duration);
     }

end:
   efl_animation_duration_set(efl_super(eo_obj, MY_CLASS), duration);
}

EOLIAN static void
_efl_canvas_animation_group_efl_canvas_animation_final_state_keep_set(Eo *eo_obj,
                                                        Efl_Canvas_Animation_Group_Data *pd,
                                                        Eina_Bool keep_final_state)
{
   Eina_List *l;
   Efl_Canvas_Animation *anim;
   EINA_LIST_FOREACH(pd->animations, l, anim)
     {
        efl_animation_final_state_keep_set(anim, keep_final_state);
     }

   efl_animation_final_state_keep_set(efl_super(eo_obj, MY_CLASS), keep_final_state);
}

EOLIAN static void
_efl_canvas_animation_group_efl_canvas_animation_interpolator_set(Eo *eo_obj,
                                                    Efl_Canvas_Animation_Group_Data *pd,
                                                    Efl_Interpolator *interpolator)
{
   Eina_List *l;
   Efl_Canvas_Animation *anim;
   EINA_LIST_FOREACH(pd->animations, l, anim)
     {
        efl_animation_interpolator_set(anim, interpolator);
     }

   efl_animation_interpolator_set(efl_super(eo_obj, MY_CLASS), interpolator);
}

EOLIAN static Efl_Object *
_efl_canvas_animation_group_efl_object_constructor(Eo *eo_obj,
                                            Efl_Canvas_Animation_Group_Data *pd)
{
   eo_obj = efl_constructor(efl_super(eo_obj, MY_CLASS));

   pd->animations = NULL;

   //group animation does not affect its child duration by default.
   efl_animation_duration_set(eo_obj, EFL_ANIMATION_GROUP_DURATION_NONE);

   return eo_obj;
}

EOLIAN static void
_efl_canvas_animation_group_efl_object_destructor(Eo *eo_obj,
                                           Efl_Canvas_Animation_Group_Data *pd)
{
   Efl_Canvas_Animation *anim;

   EINA_LIST_FREE(pd->animations, anim)
     efl_unref(anim);

   efl_destructor(efl_super(eo_obj, MY_CLASS));
}

#include "efl_canvas_animation_group.eo.c"
