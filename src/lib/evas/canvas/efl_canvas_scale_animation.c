#include "efl_canvas_scale_animation_private.h"

#define MY_CLASS EFL_CANVAS_SCALE_ANIMATION_CLASS

EOLIAN static void
_efl_canvas_scale_animation_scale_set(Eo *eo_obj EINA_UNUSED,
                               Efl_Canvas_Scale_Animation_Data *pd,
                               Eina_Vector2 from_scale,
                               Eina_Vector2 to_scale,
                               Efl_Canvas_Object *pivot,
                               Eina_Vector2 rel_pivot_pos)
{
   pd->from = from_scale;

   pd->to = to_scale;

   pd->rel_pivot.obj = pivot;
   pd->rel_pivot.pos = rel_pivot_pos;

   pd->use_rel_pivot = EINA_TRUE;
}

EOLIAN static void
_efl_canvas_scale_animation_scale_get(const Eo *obj EINA_UNUSED,
                              Efl_Canvas_Scale_Animation_Data *pd,
                              Eina_Vector2 *from_scale, Eina_Vector2 *to_scale,
                              Efl_Canvas_Object **pivot, Eina_Vector2 *pivot_pos)
{
   if (!pd->use_rel_pivot)
     {
        ERR("Animation is done in absolute value.");
        return;
     }

   if (from_scale)
     *from_scale = pd->from;

   if (to_scale)
     *to_scale = pd->to;

   if (pivot)
     *pivot = pd->rel_pivot.obj;

   if (pivot_pos)
     *pivot_pos = pd->rel_pivot.pos;
}

EOLIAN static void
_efl_canvas_scale_animation_scale_absolute_set(Eo *obj EINA_UNUSED,
                            Efl_Canvas_Scale_Animation_Data *pd,
                            Eina_Vector2 from_scale, Eina_Vector2 to_scale,
                            Eina_Position2D pos)
{
   pd->from = from_scale;

   pd->to = to_scale;

   pd->pos = pos;

   pd->use_rel_pivot = EINA_FALSE;
}

EOLIAN static void
_efl_canvas_scale_animation_scale_absolute_get(const Eo *obj EINA_UNUSED,
                            Efl_Canvas_Scale_Animation_Data *pd,
                            Eina_Vector2 *from_scale, Eina_Vector2 *to_scale,
                            Eina_Position2D *pos)
{
   if (pd->use_rel_pivot)
     {
        ERR("Animation is done in relative value.");
        return;
     }

   if (from_scale)
     *from_scale = pd->from;

   if (to_scale)
     *to_scale = pd->to;

   if (pos)
     *pos = pd->pos;
}

EOLIAN static double
_efl_canvas_scale_animation_efl_canvas_animation_animation_apply(Eo *eo_obj,
                                                   Efl_Canvas_Scale_Animation_Data *pd,
                                                   double progress,
                                                   Efl_Canvas_Object *target)
{
   Eina_Vector2 new_scale;

   progress = efl_animation_apply(efl_super(eo_obj, MY_CLASS), progress, target);
   if (!target) return progress;

   new_scale.x = GET_STATUS(pd->from.x, pd->to.x, progress);
   new_scale.y = GET_STATUS(pd->from.y, pd->to.y, progress);

   if (pd->use_rel_pivot)
     {
        efl_gfx_mapping_zoom(target,
                             new_scale.x, new_scale.y,
                             (pd->rel_pivot.obj) ? pd->rel_pivot.obj : target,
                             pd->rel_pivot.pos.x , pd->rel_pivot.pos.y);
     }
   else
     {
        efl_gfx_mapping_zoom_absolute(target,
                                      new_scale.x, new_scale.y,
                                      pd->pos.x, pd->pos.y);
     }

   return progress;
}

EOLIAN static Efl_Object *
_efl_canvas_scale_animation_efl_object_constructor(Eo *eo_obj,
                                            Efl_Canvas_Scale_Animation_Data *pd)
{
   eo_obj = efl_constructor(efl_super(eo_obj, MY_CLASS));

   pd->from = EINA_VECTOR2(1.0, 1.0);
   pd->to = EINA_VECTOR2(1.0, 1.0);
   pd->rel_pivot.pos = EINA_VECTOR2(0.5, 0.5);
   pd->rel_pivot.obj = NULL;
   pd->pos = EINA_POSITION2D(0, 0);

   pd->use_rel_pivot = EINA_TRUE;

   return eo_obj;
}

#include "efl_canvas_scale_animation.eo.c"
