#include "efl_animation_translate_private.h"

EOLIAN static void
_efl_animation_translate_translate_set(Eo *eo_obj,
                                       Efl_Animation_Translate_Data *pd,
                                       Evas_Coord from_x,
                                       Evas_Coord from_y,
                                       Evas_Coord to_x,
                                       Evas_Coord to_y)
{
   EFL_ANIMATION_TRANSLATE_CHECK_OR_RETURN(eo_obj);

   pd->from.move_x = from_x;
   pd->from.move_y = from_y;

   pd->to.move_x = to_x;
   pd->to.move_y = to_y;

   //Update absolute coordinate based on relative move
   Evas_Coord x = 0;
   Evas_Coord y = 0;

   Efl_Canvas_Object *target = efl_animation_target_get(eo_obj);
   if (target)
     evas_object_geometry_get(target, &x, &y, NULL, NULL);

   pd->from.x = pd->from.move_x + x;
   pd->from.y = pd->from.move_y + y;

   pd->to.x = pd->to.move_x + x;
   pd->to.y = pd->to.move_y + y;

   pd->use_rel_move = EINA_TRUE;
}

EOLIAN static void
_efl_animation_translate_translate_get(Eo *eo_obj,
                                       Efl_Animation_Translate_Data *pd,
                                       Evas_Coord *from_x,
                                       Evas_Coord *from_y,
                                       Evas_Coord *to_x,
                                       Evas_Coord *to_y)
{
   EFL_ANIMATION_TRANSLATE_CHECK_OR_RETURN(eo_obj);

   //Update relative move based on absolute coordinate
   if (!pd->use_rel_move)
     {
        Evas_Coord x = 0;
        Evas_Coord y = 0;

        Efl_Canvas_Object *target = efl_animation_target_get(eo_obj);
        if (target)
          evas_object_geometry_get(target, &x, &y, NULL, NULL);

        pd->from.move_x = pd->from.x - x;
        pd->from.move_y = pd->from.y - y;

        pd->to.move_x = pd->to.x - x;
        pd->to.move_y = pd->to.y - y;
     }

   if (from_x)
     *from_x = pd->from.move_x;
   if (from_y)
     *from_y = pd->from.move_y;

   if (to_x)
     *to_x = pd->to.move_x;
   if (to_y)
     *to_y = pd->to.move_y;
}

EOLIAN static void
_efl_animation_translate_translate_absolute_set(Eo *eo_obj,
                                                Efl_Animation_Translate_Data *pd,
                                                Evas_Coord from_x,
                                                Evas_Coord from_y,
                                                Evas_Coord to_x,
                                                Evas_Coord to_y)
{
   EFL_ANIMATION_TRANSLATE_CHECK_OR_RETURN(eo_obj);

   pd->from.x = from_x;
   pd->from.y = from_y;

   pd->to.x = to_x;
   pd->to.y = to_y;

   //Update relative move based on absolute coordinate
   Evas_Coord x = 0;
   Evas_Coord y = 0;

   Efl_Canvas_Object *target = efl_animation_target_get(eo_obj);
   if (target)
     evas_object_geometry_get(target, &x, &y, NULL, NULL);

   pd->from.move_x = pd->from.x - x;
   pd->from.move_y = pd->from.y - y;

   pd->to.move_x = pd->to.x - x;
   pd->to.move_y = pd->to.y - y;

   pd->use_rel_move = EINA_FALSE;
}

EOLIAN static void
_efl_animation_translate_translate_absolute_get(Eo *eo_obj,
                                                Efl_Animation_Translate_Data *pd,
                                                Evas_Coord *from_x,
                                                Evas_Coord *from_y,
                                                Evas_Coord *to_x,
                                                Evas_Coord *to_y)
{
   EFL_ANIMATION_TRANSLATE_CHECK_OR_RETURN(eo_obj);

   //Update absolute coordinate based on relative move
   if (pd->use_rel_move)
     {
        Evas_Coord x = 0;
        Evas_Coord y = 0;

        Efl_Canvas_Object *target = efl_animation_target_get(eo_obj);
        if (target)
          evas_object_geometry_get(target, &x, &y, NULL, NULL);

        pd->from.x = pd->from.move_x + x;
        pd->from.y = pd->from.move_y + y;

        pd->to.x = pd->to.move_x + x;
        pd->to.y = pd->to.move_y + y;
     }

   if (from_x)
     *from_x = pd->from.x;
   if (from_y)
     *from_y = pd->from.y;

   if (to_x)
     *to_x = pd->to.x;
   if (to_y)
     *to_y = pd->to.y;
}

EOLIAN static Efl_Animation_Object *
_efl_animation_translate_efl_animation_object_create(Eo *eo_obj,
                                                     Efl_Animation_Translate_Data *pd)
{
   EFL_ANIMATION_TRANSLATE_CHECK_OR_RETURN(eo_obj, NULL);

   Efl_Animation_Object_Translate *anim_obj
      = efl_add(EFL_ANIMATION_OBJECT_TRANSLATE_CLASS, NULL);

   Efl_Canvas_Object *target = efl_animation_target_get(eo_obj);
   efl_animation_object_target_set(anim_obj, target);

   Eina_Bool state_keep = efl_animation_final_state_keep_get(eo_obj);
   efl_animation_object_final_state_keep_set(anim_obj, state_keep);

   double duration = efl_animation_duration_get(eo_obj);
   efl_animation_object_duration_set(anim_obj, duration);

   double total_duration = efl_animation_total_duration_get(eo_obj);
   efl_animation_object_total_duration_set(anim_obj, total_duration);

   if (pd->use_rel_move)
     {
        efl_animation_object_translate_set(anim_obj,
                                           pd->from.move_x,
                                           pd->from.move_y,
                                           pd->to.move_x,
                                           pd->to.move_y);
     }
   else
     {
        efl_animation_object_translate_absolute_set(anim_obj,
                                                    pd->from.x,
                                                    pd->from.y,
                                                    pd->to.x,
                                                    pd->to.y);
     }

   return anim_obj;
}

EOLIAN static Efl_Object *
_efl_animation_translate_efl_object_constructor(Eo *eo_obj,
                                                Efl_Animation_Translate_Data *pd)
{
   eo_obj = efl_constructor(efl_super(eo_obj, MY_CLASS));

   pd->from.move_x = 0;
   pd->from.move_y = 0;
   pd->from.x = 0;
   pd->from.y = 0;

   pd->to.move_x = 0;
   pd->to.move_y = 0;
   pd->to.x = 0;
   pd->to.y = 0;

   pd->use_rel_move = EINA_TRUE;

   return eo_obj;
}

#include "efl_animation_translate.eo.c"
