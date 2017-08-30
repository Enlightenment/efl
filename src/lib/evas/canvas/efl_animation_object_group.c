#include "efl_animation_object_group_private.h"

EOLIAN static void
_efl_animation_object_group_object_add(Eo *eo_obj,
                                       Efl_Animation_Object_Group_Data *pd,
                                       Efl_Animation_Object *anim_obj)
{
   EFL_ANIMATION_OBJECT_GROUP_CHECK_OR_RETURN(eo_obj);

   if (!anim_obj) return;

   Efl_Canvas_Object *target = efl_animation_object_target_get(eo_obj);
   if (target)
     efl_animation_object_target_set(anim_obj, target);

   double duration = efl_animation_object_duration_get(eo_obj);
   /* if group animation object duration is available value, then the duration
    * is propagated to its child. */
   if (duration != EFL_ANIMATION_OBJECT_GROUP_DURATION_NONE)
     efl_animation_object_duration_set(anim_obj, duration);

   Eina_Bool state_keep = efl_animation_object_final_state_keep_get(eo_obj);
   efl_animation_object_final_state_keep_set(anim_obj, state_keep);

   pd->anim_objs = eina_list_append(pd->anim_objs, anim_obj);
}

EOLIAN static void
_efl_animation_object_group_object_del(Eo *eo_obj,
                                       Efl_Animation_Object_Group_Data *pd,
                                       Efl_Animation_Object *anim_obj)
{
   EFL_ANIMATION_OBJECT_GROUP_CHECK_OR_RETURN(eo_obj);

   if (!anim_obj) return;

   pd->anim_objs = eina_list_remove(pd->anim_objs, anim_obj);
}

EOLIAN static Eina_List *
_efl_animation_object_group_objects_get(Eo *eo_obj,
                                        Efl_Animation_Object_Group_Data *pd)
{
   EFL_ANIMATION_OBJECT_GROUP_CHECK_OR_RETURN(eo_obj, NULL);

   return pd->anim_objs;
}

EOLIAN static void
_efl_animation_object_group_efl_animation_object_target_set(Eo *eo_obj,
                                                            Efl_Animation_Object_Group_Data *pd,
                                                            Efl_Canvas_Object *target)
{
   EFL_ANIMATION_OBJECT_GROUP_CHECK_OR_RETURN(eo_obj);

   Eina_List *l;
   Efl_Animation_Object *anim_obj;

   EINA_LIST_FOREACH(pd->anim_objs, l, anim_obj)
     {
        efl_animation_object_target_set(anim_obj, target);
     }

   efl_animation_object_target_set(efl_super(eo_obj, MY_CLASS), target);
}

EOLIAN static void
_efl_animation_object_group_efl_animation_object_duration_set(Eo *eo_obj,
                                                              Efl_Animation_Object_Group_Data *pd,
                                                              double duration)
{
   EFL_ANIMATION_OBJECT_GROUP_CHECK_OR_RETURN(eo_obj);

   if (duration == EFL_ANIMATION_OBJECT_GROUP_DURATION_NONE) goto end;

   if (duration < 0.0) return;

   Eina_List *l;
   Efl_Animation_Object *anim_obj;

   EINA_LIST_FOREACH(pd->anim_objs, l, anim_obj)
     {
        efl_animation_object_duration_set(anim_obj, duration);
     }

end:
   efl_animation_object_duration_only_set(efl_super(eo_obj, MY_CLASS),
                                          duration);

   /* efl_animation_object_total_duration_get() should calculate the new total
    * duration. */
   double total_duration = efl_animation_object_total_duration_get(eo_obj);
   efl_animation_object_total_duration_set(eo_obj, total_duration);
}

EOLIAN static void
_efl_animation_object_group_efl_animation_object_final_state_keep_set(Eo *eo_obj,
                                                                      Efl_Animation_Object_Group_Data *pd,
                                                                      Eina_Bool state_keep)
{
   EFL_ANIMATION_OBJECT_GROUP_CHECK_OR_RETURN(eo_obj);

   Eina_List *l;
   Efl_Animation_Object *anim_obj;

   EINA_LIST_FOREACH(pd->anim_objs, l, anim_obj)
     {
        efl_animation_object_final_state_keep_set(anim_obj, state_keep);
     }

   efl_animation_object_final_state_keep_set(efl_super(eo_obj, MY_CLASS),
                                             state_keep);
}

EOLIAN static Efl_Object *
_efl_animation_object_group_efl_object_constructor(Eo *eo_obj,
                                                   Efl_Animation_Object_Group_Data *pd)
{
   eo_obj = efl_constructor(efl_super(eo_obj, MY_CLASS));

   pd->anim_objs = NULL;

   //group animation object does not affect its child duration by default.
   efl_animation_object_duration_only_set(efl_super(eo_obj, MY_CLASS),
                                          EFL_ANIMATION_OBJECT_GROUP_DURATION_NONE);

   return eo_obj;
}

EOLIAN static void
_efl_animation_object_group_efl_object_destructor(Eo *eo_obj,
                                                  Efl_Animation_Object_Group_Data *pd)
{
   Efl_Animation_Object *anim_obj;

   EINA_LIST_FREE(pd->anim_objs, anim_obj)
      efl_del(anim_obj);

   efl_destructor(efl_super(eo_obj, MY_CLASS));
}

EOLIAN static void
_efl_animation_object_group_efl_animation_object_target_state_save(Eo *eo_obj,
                                                                   Efl_Animation_Object_Group_Data *pd)
{
   EFL_ANIMATION_OBJECT_GROUP_CHECK_OR_RETURN(eo_obj);

   Eina_List *l;
   Efl_Animation_Object *anim_obj;

   EINA_LIST_FOREACH(pd->anim_objs, l, anim_obj)
     {
        efl_animation_object_target_state_save(anim_obj);
     }
}

EOLIAN static void
_efl_animation_object_group_efl_animation_object_target_state_reset(Eo *eo_obj,
                                                                    Efl_Animation_Object_Group_Data *pd)
{
   EFL_ANIMATION_OBJECT_GROUP_CHECK_OR_RETURN(eo_obj);

   Eina_List *l;
   Efl_Animation_Object *anim_obj;

   EINA_LIST_FOREACH(pd->anim_objs, l, anim_obj)
     {
        efl_animation_object_target_state_reset(anim_obj);
     }
}

EOLIAN static void
_efl_animation_object_group_efl_animation_object_target_map_reset(Eo *eo_obj,
                                                                  Efl_Animation_Object_Group_Data *pd)
{
   EFL_ANIMATION_OBJECT_GROUP_CHECK_OR_RETURN(eo_obj);

   Eina_List *l;
   Efl_Animation_Object *anim_obj;

   EINA_LIST_FOREACH(pd->anim_objs, l, anim_obj)
     {
        efl_animation_object_target_map_reset(anim_obj);
     }
}

/* Internal EO APIs */

EOAPI EFL_VOID_FUNC_BODYV(efl_animation_object_group_object_add, EFL_FUNC_CALL(anim_obj), Efl_Animation_Object *anim_obj);

EOAPI EFL_VOID_FUNC_BODYV(efl_animation_object_group_object_del, EFL_FUNC_CALL(anim_obj), Efl_Animation_Object *anim_obj);

EOAPI EFL_FUNC_BODY(efl_animation_object_group_objects_get, Eina_List *, NULL);

#define EFL_ANIMATION_OBJECT_GROUP_EXTRA_OPS \
   EFL_OBJECT_OP_FUNC(efl_animation_object_group_object_add, _efl_animation_object_group_object_add), \
   EFL_OBJECT_OP_FUNC(efl_animation_object_group_object_del, _efl_animation_object_group_object_del), \
   EFL_OBJECT_OP_FUNC(efl_animation_object_group_objects_get, _efl_animation_object_group_objects_get), \
   EFL_OBJECT_OP_FUNC(efl_animation_object_target_set, _efl_animation_object_group_efl_animation_object_target_set), \
   EFL_OBJECT_OP_FUNC(efl_animation_object_duration_set, _efl_animation_object_group_efl_animation_object_duration_set), \
   EFL_OBJECT_OP_FUNC(efl_animation_object_final_state_keep_set, _efl_animation_object_group_efl_animation_object_final_state_keep_set)

#include "efl_animation_object_group.eo.c"
