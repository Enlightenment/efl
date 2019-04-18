#include "efl_canvas_animation_rotate_private.h"

#define MY_CLASS EFL_CANVAS_ANIMATION_ROTATE_CLASS

static double
_rotation_get(Eo *target)
{
   double x1, x2, y1, y2;
   double theta;

   efl_gfx_mapping_coord_absolute_get(target, 0, &x1, &y1, NULL);
   efl_gfx_mapping_coord_absolute_get(target, 1, &x2, &y2, NULL);
   theta = atan((y2 - y1) / (x2 - x1));

   return theta * 180 / M_PI;
}

EOLIAN static void
_efl_canvas_animation_rotate_rotate_set(Eo *eo_obj EINA_UNUSED,
                                 Efl_Canvas_Animation_Rotate_Data *pd,
                                 double from_degree,
                                 double to_degree,
                                 Efl_Canvas_Object *pivot,
                                 double cx,
                                 double cy)
{
   pd->from.degree = from_degree;
   pd->to.degree = to_degree;

   //TODO: check whether ref for pivot should be added.
   pd->rel_pivot.obj = pivot;
   pd->rel_pivot.cx = cx;
   pd->rel_pivot.cy = cy;
   pd->use_rel_pivot = EINA_TRUE;
}

EOLIAN static void
_efl_canvas_animation_rotate_rotate_get(const Eo *eo_obj EINA_UNUSED,
                                 Efl_Canvas_Animation_Rotate_Data *pd,
                                 double *from_degree,
                                 double *to_degree,
                                 Efl_Canvas_Object **pivot,
                                 double *cx,
                                 double *cy)
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

   if (cx)
     *cx = pd->rel_pivot.cx;

   if (cy)
     *cy = pd->rel_pivot.cy;
}

EOLIAN static void
_efl_canvas_animation_rotate_rotate_absolute_set(Eo *eo_obj EINA_UNUSED,
                                          Efl_Canvas_Animation_Rotate_Data *pd,
                                          double from_degree,
                                          double to_degree,
                                          Evas_Coord cx,
                                          Evas_Coord cy)
{
   pd->from.degree = from_degree;
   pd->to.degree = to_degree;

   pd->abs_pivot.cx = cx;
   pd->abs_pivot.cy = cy;
   pd->use_rel_pivot = EINA_FALSE;
}

EOLIAN static void
_efl_canvas_animation_rotate_rotate_absolute_get(const Eo *eo_obj EINA_UNUSED,
                                          Efl_Canvas_Animation_Rotate_Data *pd,
                                          double *from_degree,
                                          double *to_degree,
                                          Evas_Coord *cx,
                                          Evas_Coord *cy)
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

   if (cx)
     *cx = pd->abs_pivot.cx;

   if (cy)
     *cy = pd->abs_pivot.cy;
}

EOLIAN static double
_efl_canvas_animation_rotate_efl_canvas_animation_animation_apply(Eo *eo_obj,
                                                    Efl_Canvas_Animation_Rotate_Data *pd,
                                                    double progress,
                                                    Efl_Canvas_Object *target)
{
   double new_degree;
   double prev_degree;

   progress = efl_animation_apply(efl_super(eo_obj, MY_CLASS), progress, target);
   if (!target) return progress;

   prev_degree = _rotation_get(target);
   new_degree = GET_STATUS(pd->from.degree, pd->to.degree, progress);

   if (pd->use_rel_pivot)
     {
        efl_gfx_mapping_rotate(target,
                           new_degree - prev_degree,
                           (pd->rel_pivot.obj) ? pd->rel_pivot.obj : target,
                           pd->rel_pivot.cx, pd->rel_pivot.cy);
     }
   else
     {
        efl_gfx_mapping_rotate_absolute(target,
                                    new_degree - prev_degree,
                                    pd->abs_pivot.cx, pd->abs_pivot.cy);
     }

   return progress;
}

EOLIAN static Efl_Object *
_efl_canvas_animation_rotate_efl_object_constructor(Eo *eo_obj,
                                             Efl_Canvas_Animation_Rotate_Data *pd)
{
   eo_obj = efl_constructor(efl_super(eo_obj, MY_CLASS));

   pd->from.degree = 0.0;
   pd->to.degree = 0.0;

   pd->rel_pivot.obj = NULL;
   pd->rel_pivot.cx = 0.5;
   pd->rel_pivot.cy = 0.5;

   pd->abs_pivot.cx = 0;
   pd->abs_pivot.cy = 0;

   pd->use_rel_pivot = EINA_TRUE;

   return eo_obj;
}

#include "efl_canvas_animation_rotate.eo.c"
