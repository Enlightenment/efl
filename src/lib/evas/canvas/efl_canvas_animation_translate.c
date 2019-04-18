#include "efl_canvas_animation_translate_private.h"

#define MY_CLASS EFL_CANVAS_ANIMATION_TRANSLATE_CLASS

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

   geometry = efl_gfx_entity_geometry_get(target);

   efl_gfx_mapping_coord_absolute_get(target, 0, &x1, &y1, NULL);
   efl_gfx_mapping_coord_absolute_get(target, 2, &x2, &y2, NULL);
   translate.x = ((x1 + x2) / 2.0) - (geometry.x + (geometry.w / 2.0));
   translate.y = ((y1 + y2) / 2.0) - (geometry.y + (geometry.h / 2.0));

   return translate;
}

EOLIAN static void
_efl_canvas_animation_translate_translate_set(Eo *eo_obj EINA_UNUSED,
                                       Efl_Canvas_Animation_Translate_Data *pd,
                                       Evas_Coord from_x,
                                       Evas_Coord from_y,
                                       Evas_Coord to_x,
                                       Evas_Coord to_y)
{
   pd->from.move_x = from_x;
   pd->from.move_y = from_y;

   pd->to.move_x = to_x;
   pd->to.move_y = to_y;

   pd->use_rel_move = EINA_TRUE;
}

EOLIAN static void
_efl_canvas_animation_translate_translate_get(const Eo *eo_obj EINA_UNUSED,
                                       Efl_Canvas_Animation_Translate_Data *pd,
                                       Evas_Coord *from_x,
                                       Evas_Coord *from_y,
                                       Evas_Coord *to_x,
                                       Evas_Coord *to_y)
{
   if (!pd->use_rel_move)
     {
        ERR("Animation is done in absolute value.");
        return;
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
_efl_canvas_animation_translate_translate_absolute_set(Eo *eo_obj EINA_UNUSED,
                                                Efl_Canvas_Animation_Translate_Data *pd,
                                                Evas_Coord from_x,
                                                Evas_Coord from_y,
                                                Evas_Coord to_x,
                                                Evas_Coord to_y)
{
   pd->from.x = from_x;
   pd->from.y = from_y;

   pd->to.x = to_x;
   pd->to.y = to_y;

   pd->use_rel_move = EINA_FALSE;
}

EOLIAN static void
_efl_canvas_animation_translate_translate_absolute_get(const Eo *eo_obj EINA_UNUSED,
                                                Efl_Canvas_Animation_Translate_Data *pd,
                                                Evas_Coord *from_x,
                                                Evas_Coord *from_y,
                                                Evas_Coord *to_x,
                                                Evas_Coord *to_y)
{
   if (pd->use_rel_move)
     {
        ERR("Animation is done in absolute value.");
        return;
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

EOLIAN static double
_efl_canvas_animation_translate_efl_canvas_animation_animation_apply(Eo *eo_obj,
                                                       Efl_Canvas_Animation_Translate_Data *pd,
                                                       double progress,
                                                       Efl_Canvas_Object *target)
{
   _Translate_Property_Double prev;
   _Translate_Property_Double new;

   progress = efl_animation_apply(efl_super(eo_obj, MY_CLASS), progress, target);
   if (!target) return progress;

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

   efl_gfx_mapping_translate(target, new.x - prev.x, new.y - prev.y, 0.0);

   return progress;
}

EOLIAN static Efl_Object *
_efl_canvas_animation_translate_efl_object_constructor(Eo *eo_obj,
                                                Efl_Canvas_Animation_Translate_Data *pd)
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

#include "efl_canvas_animation_translate.eo.c"
