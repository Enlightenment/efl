#include "efl_canvas_rotate_animation_private.h"

#define MY_CLASS EFL_CANVAS_ROTATE_ANIMATION_CLASS

EOLIAN static void
_efl_canvas_rotate_animation_rotate_set(Eo *eo_obj EINA_UNUSED,
                                 Efl_Canvas_Rotate_Animation_Data *pd,
                                 double from_degree,
                                 double to_degree,
                                 Efl_Canvas_Object *pivot,
                                 Eina_Vector2 center_point)
{
   pd->from.degree = from_degree;
   pd->to.degree = to_degree;

   //TODO: check whether ref for pivot should be added.
   pd->rel_pivot.obj = pivot;
   pd->rel_pivot.pos = center_point;
   pd->use_rel_pivot = EINA_TRUE;
}

EOLIAN static void
_efl_canvas_rotate_animation_rotate_get(const Eo *eo_obj EINA_UNUSED,
                                 Efl_Canvas_Rotate_Animation_Data *pd,
                                 double *from_degree,
                                 double *to_degree,
                                 Efl_Canvas_Object **pivot,
                                 Eina_Vector2 *center_point)
{
   if (!pd->use_rel_pivot)
     {
        ERR("Animation is done in absolute value.");
        return;
     }

   if (from_degree)
     *from_degree = pd->from.degree;

   if (to_degree)
     *to_degree = pd->to.degree;

   if (pivot)
     *pivot = pd->rel_pivot.obj;

   if (center_point)
     *center_point = pd->rel_pivot.pos;
}

EOLIAN static void
_efl_canvas_rotate_animation_rotate_absolute_set(Eo *eo_obj EINA_UNUSED,
                                          Efl_Canvas_Rotate_Animation_Data *pd,
                                          double from_degree,
                                          double to_degree,
                                          Eina_Position2D abs)
{
   pd->from.degree = from_degree;
   pd->to.degree = to_degree;

   pd->abs_pivot = abs;
   pd->use_rel_pivot = EINA_FALSE;
}

EOLIAN static void
_efl_canvas_rotate_animation_rotate_absolute_get(const Eo *eo_obj EINA_UNUSED,
                                          Efl_Canvas_Rotate_Animation_Data *pd,
                                          double *from_degree,
                                          double *to_degree,
                                          Eina_Position2D *abs)
{
   if (pd->use_rel_pivot)
     {
        ERR("Animation is done in relative value.");
        return;
     }

   if (from_degree)
     *from_degree = pd->from.degree;

   if (to_degree)
     *to_degree = pd->to.degree;

   if (abs)
     *abs = pd->abs_pivot;
}

EOLIAN static double
_efl_canvas_rotate_animation_efl_canvas_animation_animation_apply(Eo *eo_obj,
                                                    Efl_Canvas_Rotate_Animation_Data *pd,
                                                    double progress,
                                                    Efl_Canvas_Object *target)
{
   double new_degree;

   progress = efl_animation_apply(efl_super(eo_obj, MY_CLASS), progress, target);
   if (!target) return progress;

   new_degree = GET_STATUS(pd->from.degree, pd->to.degree, progress);

   if (pd->use_rel_pivot)
     {
        efl_gfx_mapping_rotate(target,
                               new_degree,
                               (pd->rel_pivot.obj) ? pd->rel_pivot.obj : target,
                               pd->rel_pivot.pos.x, pd->rel_pivot.pos.y);
     }
   else
     {
        efl_gfx_mapping_rotate_absolute(target,
                                        new_degree,
                                        pd->abs_pivot.x, pd->abs_pivot.y);
     }

   return progress;
}

EOLIAN static Efl_Object *
_efl_canvas_rotate_animation_efl_object_constructor(Eo *eo_obj,
                                             Efl_Canvas_Rotate_Animation_Data *pd)
{
   eo_obj = efl_constructor(efl_super(eo_obj, MY_CLASS));

   pd->from.degree = 0.0;
   pd->to.degree = 0.0;

   pd->rel_pivot.obj = NULL;
   pd->rel_pivot.pos.x = 0.5;
   pd->rel_pivot.pos.y = 0.5;

   pd->abs_pivot.x = 0;
   pd->abs_pivot.y = 0;

   pd->use_rel_pivot = EINA_TRUE;

   return eo_obj;
}

#include "efl_canvas_rotate_animation.eo.c"
