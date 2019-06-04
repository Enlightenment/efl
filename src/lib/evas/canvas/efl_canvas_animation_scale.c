#include "efl_canvas_animation_scale_private.h"

#define MY_CLASS EFL_CANVAS_ANIMATION_SCALE_CLASS

EOLIAN static void
_efl_canvas_animation_scale_scale_set(Eo *eo_obj EINA_UNUSED,
                               Efl_Canvas_Animation_Scale_Data *pd,
                               double from_scale_x,
                               double from_scale_y,
                               double to_scale_x,
                               double to_scale_y,
                               Efl_Canvas_Object *pivot,
                               double cx,
                               double cy)
{
   pd->from.scale_x = from_scale_x;
   pd->from.scale_y = from_scale_y;

   pd->to.scale_x = to_scale_x;
   pd->to.scale_y = to_scale_y;

   pd->rel_pivot.obj = pivot;
   pd->rel_pivot.cx = cx;
   pd->rel_pivot.cy = cy;

   pd->use_rel_pivot = EINA_TRUE;
}

EOLIAN static void
_efl_canvas_animation_scale_scale_get(const Eo *eo_obj EINA_UNUSED,
                               Efl_Canvas_Animation_Scale_Data *pd,
                               double *from_scale_x,
                               double *from_scale_y,
                               double *to_scale_x,
                               double *to_scale_y,
                               Efl_Canvas_Object **pivot,
                               double *cx,
                               double *cy)
{
   if (!pd->use_rel_pivot)
     {
        ERR("Animation is done in absolute value.");
        return;
     }

   if (from_scale_x)
     *from_scale_x = pd->from.scale_x;

   if (from_scale_y)
     *from_scale_y = pd->from.scale_y;

   if (to_scale_x)
     *to_scale_x = pd->to.scale_x;

   if (to_scale_y)
     *to_scale_y = pd->to.scale_y;

   if (pivot)
     *pivot = pd->rel_pivot.obj;

   if (cx)
     *cx = pd->rel_pivot.cx;

   if (cy)
     *cy = pd->rel_pivot.cy;
}

EOLIAN static void
_efl_canvas_animation_scale_scale_absolute_set(Eo *eo_obj EINA_UNUSED,
                                        Efl_Canvas_Animation_Scale_Data *pd,
                                        double from_scale_x,
                                        double from_scale_y,
                                        double to_scale_x,
                                        double to_scale_y,
                                        Evas_Coord cx,
                                        Evas_Coord cy)
{
   pd->from.scale_x = from_scale_x;
   pd->from.scale_y = from_scale_y;

   pd->to.scale_x = to_scale_x;
   pd->to.scale_y = to_scale_y;

   pd->abs_pivot.cx = cx;
   pd->abs_pivot.cy = cy;

   pd->use_rel_pivot = EINA_FALSE;
}

EOLIAN static void
_efl_canvas_animation_scale_scale_absolute_get(const Eo *eo_obj EINA_UNUSED,
                                        Efl_Canvas_Animation_Scale_Data *pd,
                                        double *from_scale_x,
                                        double *from_scale_y,
                                        double *to_scale_x,
                                        double *to_scale_y,
                                        Evas_Coord *cx,
                                        Evas_Coord *cy)
{
   if (pd->use_rel_pivot)
     {
        ERR("Animation is done in relative value.");
        return;
     }

   if (from_scale_x)
     *from_scale_x = pd->from.scale_x;

   if (from_scale_y)
     *from_scale_y = pd->from.scale_y;

   if (to_scale_x)
     *to_scale_x = pd->to.scale_x;

   if (to_scale_y)
     *to_scale_y = pd->to.scale_y;

   if (cx)
     *cx = pd->abs_pivot.cx;

   if (cy)
     *cy = pd->abs_pivot.cy;
}

EOLIAN static double
_efl_canvas_animation_scale_efl_canvas_animation_animation_apply(Eo *eo_obj,
                                                   Efl_Canvas_Animation_Scale_Data *pd,
                                                   double progress,
                                                   Efl_Canvas_Object *target)
{
   Efl_Canvas_Animation_Scale_Property new_scale;

   progress = efl_animation_apply(efl_super(eo_obj, MY_CLASS), progress, target);
   if (!target) return progress;

   new_scale.scale_x = GET_STATUS(pd->from.scale_x, pd->to.scale_x, progress);
   new_scale.scale_y = GET_STATUS(pd->from.scale_y, pd->to.scale_y, progress);

   if (pd->use_rel_pivot)
     {
        efl_gfx_mapping_zoom(target,
                             new_scale.scale_x, new_scale.scale_y,
                             (pd->rel_pivot.obj) ? pd->rel_pivot.obj : target,
                             pd->rel_pivot.cx, pd->rel_pivot.cy);
     }
   else
     {
        efl_gfx_mapping_zoom_absolute(target,
                                      new_scale.scale_x, new_scale.scale_y,
                                      pd->abs_pivot.cx, pd->abs_pivot.cy);
     }

   return progress;
}

EOLIAN static Efl_Object *
_efl_canvas_animation_scale_efl_object_constructor(Eo *eo_obj,
                                            Efl_Canvas_Animation_Scale_Data *pd)
{
   eo_obj = efl_constructor(efl_super(eo_obj, MY_CLASS));

   pd->from.scale_x = 1.0;
   pd->from.scale_y = 1.0;
   pd->to.scale_x = 1.0;
   pd->to.scale_y = 1.0;

   pd->rel_pivot.obj = NULL;
   pd->rel_pivot.cx = 0.5;
   pd->rel_pivot.cy = 0.5;

   pd->abs_pivot.cx = 0;
   pd->abs_pivot.cy = 0;

   pd->use_rel_pivot = EINA_TRUE;

   return eo_obj;
}

#include "efl_canvas_animation_scale.eo.c"
