#include "efl_animation_translate_private.h"

#define MY_CLASS EFL_ANIMATION_TRANSLATE_CLASS

typedef struct __Translate_Property_Double
{
   double x;
   double y;
} _Translate_Property_Double;

static _Translate_Property_Double
_translation_get(Eo *target)
{
   double x1, x2, y1, y2;
   _Translate_Property_Double translate;
   Eina_Rect geometry;

   geometry = efl_gfx_geometry_get(target);

   efl_gfx_map_coord_absolute_get(target, 0, &x1, &y1, NULL);
   efl_gfx_map_coord_absolute_get(target, 2, &x2, &y2, NULL);
   translate.x = ((x1 + x2) / 2.0) - (geometry.x + (geometry.w / 2.0));
   translate.y = ((y1 + y2) / 2.0) - (geometry.y + (geometry.h / 2.0));

   return translate;
}

EOLIAN static void
_efl_animation_translate_translate_set(Eo *eo_obj,
                                       Efl_Animation_Translate_Data *pd,
                                       Evas_Coord from_x,
                                       Evas_Coord from_y,
                                       Evas_Coord to_x,
                                       Evas_Coord to_y)
{
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

EOLIAN static void
_efl_animation_translate_efl_playable_progress_set(Eo *eo_obj,
                                                   Efl_Animation_Translate_Data *pd,
                                                   double progress)
{
   _Translate_Property_Double prev;
   _Translate_Property_Double new;

   efl_playable_progress_set(efl_super(eo_obj, MY_CLASS), progress);
   progress = efl_playable_progress_get(eo_obj);

   Efl_Canvas_Object *target = efl_animation_target_get(eo_obj);
   if (!target) return;

   prev = _translation_get(target);
   if (pd->use_rel_move)
     {
        new.x = GET_STATUS(pd->from.move_x, pd->to.move_x, progress);
        new.y = GET_STATUS(pd->from.move_y, pd->to.move_y, progress);
     }
   else
     {
        new.x = GET_STATUS(pd->from.x, pd->to.x, progress);
        new.y = GET_STATUS(pd->from.y, pd->to.y, progress);
     }

   efl_gfx_map_translate(target, new.x - prev.x, new.y - prev.y, 0.0);
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
