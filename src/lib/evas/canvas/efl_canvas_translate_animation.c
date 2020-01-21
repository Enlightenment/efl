#include "efl_canvas_translate_animation_private.h"

#define MY_CLASS EFL_CANVAS_TRANSLATE_ANIMATION_CLASS

typedef struct __Translate_Property_Double
{
   double x;
   double y;
} _Translate_Property_Double;

EOLIAN static void
_efl_canvas_translate_animation_translate_set(Eo *eo_obj EINA_UNUSED,
                                       Efl_Canvas_Translate_Animation_Data *pd,
                                       Eina_Position2D from,
                                       Eina_Position2D to)
{
   pd->from = from;

   pd->to = to;

   pd->use_rel_move = EINA_TRUE;
}

EOLIAN static void
_efl_canvas_translate_animation_translate_get(const Eo *eo_obj EINA_UNUSED,
                                       Efl_Canvas_Translate_Animation_Data *pd,
                                       Eina_Position2D *from,
                                       Eina_Position2D *to)
{
   if (!pd->use_rel_move)
     {
        ERR("Animation is done in absolute value.");
        return;
     }

   if (from)
     *from = pd->from;

   if (to)
     *to = pd->to;
}

EOLIAN static void
_efl_canvas_translate_animation_translate_absolute_set(Eo *eo_obj EINA_UNUSED,
                                                Efl_Canvas_Translate_Animation_Data *pd,
                                                Eina_Position2D from,
                                                Eina_Position2D to)
{
   pd->from = from;

   pd->to = to;

   pd->use_rel_move = EINA_FALSE;
}

EOLIAN static void
_efl_canvas_translate_animation_translate_absolute_get(const Eo *eo_obj EINA_UNUSED,
                                                Efl_Canvas_Translate_Animation_Data *pd,
                                                Eina_Position2D *from,
                                                Eina_Position2D *to)
{
   if (pd->use_rel_move)
     {
        ERR("Animation is done in absolute value.");
        return;
     }

   if (from)
     *from = pd->from;

   if (to)
     *to = pd->to;
}

EOLIAN static double
_efl_canvas_translate_animation_efl_canvas_animation_animation_apply(Eo *eo_obj,
                                                       Efl_Canvas_Translate_Animation_Data *pd,
                                                       double progress,
                                                       Efl_Canvas_Object *target)
{
   _Translate_Property_Double new;
   Eina_Rect geometry;

   progress = efl_animation_apply(efl_super(eo_obj, MY_CLASS), progress, target);
   if (!target) return progress;

   if (pd->use_rel_move)
     {
        new.x = GET_STATUS(pd->from.x, pd->to.x, progress);
        new.y = GET_STATUS(pd->from.y, pd->to.y, progress);
     }
   else
     {
        geometry = efl_gfx_entity_geometry_get(target);
        new.x = GET_STATUS(pd->from.x, pd->to.x, progress) - geometry.x;
        new.y = GET_STATUS(pd->from.y, pd->to.y, progress) - geometry.y;
     }

   efl_gfx_mapping_translate(target, new.x, new.y, 0.0);

   return progress;
}

EOLIAN static Efl_Object *
_efl_canvas_translate_animation_efl_object_constructor(Eo *eo_obj,
                                                Efl_Canvas_Translate_Animation_Data *pd)
{
   eo_obj = efl_constructor(efl_super(eo_obj, MY_CLASS));

   pd->from = EINA_POSITION2D(0,0);
   pd->to = EINA_POSITION2D(0,0);

   pd->use_rel_move = EINA_TRUE;

   return eo_obj;
}

#include "efl_canvas_translate_animation.eo.c"
