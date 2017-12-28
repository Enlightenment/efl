#include "efl_animation_scale_private.h"

#define MY_CLASS EFL_ANIMATION_SCALE_CLASS

static Efl_Animation_Scale_Property
_scale_get(Eo *target)
{
   double x1, x2, x3, y1, y2, y3, w, h;
   Efl_Animation_Scale_Property scale;
   Eina_Rect geometry;

   geometry = efl_gfx_geometry_get(target);
   efl_gfx_map_coord_absolute_get(target, 0, &x1, &y1, NULL);
   efl_gfx_map_coord_absolute_get(target, 1, &x2, &y2, NULL);
   efl_gfx_map_coord_absolute_get(target, 2, &x3, &y3, NULL);

   w = sqrt(((x2 - x1) * (x2 - x1)) + ((y2 - y1) * (y2 - y1)));
   h = sqrt(((x3 - x2) * (x3 - x2)) + ((y3 - y2) * (y3 - y2)));

   scale.scale_x = w / geometry.w;
   scale.scale_y = h / geometry.h;

   return scale;
}

EOLIAN static void
_efl_animation_scale_scale_set(Eo *eo_obj,
                               Efl_Animation_Scale_Data *pd,
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

   //Update absolute pivot based on relative pivot
   Evas_Coord x = 0;
   Evas_Coord y = 0;
   Evas_Coord w = 0;
   Evas_Coord h = 0;

   if (pivot)
     evas_object_geometry_get(pivot, &x, &y, &w, &h);
   else
     {
        Efl_Canvas_Object *target = efl_animation_target_get(eo_obj);
        if (target)
          evas_object_geometry_get(target, &x, &y, &w, &h);
     }

   pd->abs_pivot.cx = x + (w * cx);
   pd->abs_pivot.cy = y + (h * cy);

   pd->use_rel_pivot = EINA_TRUE;
}

EOLIAN static void
_efl_animation_scale_scale_get(Eo *eo_obj,
                               Efl_Animation_Scale_Data *pd,
                               double *from_scale_x,
                               double *from_scale_y,
                               double *to_scale_x,
                               double *to_scale_y,
                               Efl_Canvas_Object **pivot,
                               double *cx,
                               double *cy)
{
   //Update relative pivot based on absolute pivot
   if (!pd->use_rel_pivot)
     {
        Evas_Coord x = 0;
        Evas_Coord y = 0;
        Evas_Coord w = 0;
        Evas_Coord h = 0;

        Efl_Canvas_Object *target = efl_animation_target_get(eo_obj);
        if (target)
          evas_object_geometry_get(target, &x, &y, &w, &h);

        if (w != 0)
          pd->rel_pivot.cx = (double)(pd->abs_pivot.cx - x) / w;
        else
          pd->rel_pivot.cx = 0.0;

        if (h != 0)
          pd->rel_pivot.cy = (double)(pd->abs_pivot.cy - y) / h;
        else
          pd->rel_pivot.cy = 0.0;
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
_efl_animation_scale_scale_absolute_set(Eo *eo_obj,
                                        Efl_Animation_Scale_Data *pd,
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

   //Update relative pivot based on absolute pivot
   Evas_Coord x = 0;
   Evas_Coord y = 0;
   Evas_Coord w = 0;
   Evas_Coord h = 0;

   Efl_Canvas_Object *target = efl_animation_target_get(eo_obj);
   if (target)
     evas_object_geometry_get(target, &x, &y, &w, &h);

   pd->rel_pivot.obj = NULL;

   if (w != 0)
     pd->rel_pivot.cx = (double)(cx - x) / w;
   else
     pd->rel_pivot.cx = 0.0;

   if (h != 0)
     pd->rel_pivot.cy = (double)(cy - y) / h;
   else
     pd->rel_pivot.cy = 0.0;

   pd->use_rel_pivot = EINA_FALSE;
}

EOLIAN static void
_efl_animation_scale_scale_absolute_get(Eo *eo_obj,
                                        Efl_Animation_Scale_Data *pd,
                                        double *from_scale_x,
                                        double *from_scale_y,
                                        double *to_scale_x,
                                        double *to_scale_y,
                                        Evas_Coord *cx,
                                        Evas_Coord *cy)
{
   //Update absolute pivot based on relative pivot
   if (pd->use_rel_pivot)
     {
        Evas_Coord x = 0;
        Evas_Coord y = 0;
        Evas_Coord w = 0;
        Evas_Coord h = 0;

        if (pd->rel_pivot.obj)
          evas_object_geometry_get(pd->rel_pivot.obj, &x, &y, &w, &h);
        else
          {
             Efl_Canvas_Object *target = efl_animation_target_get(eo_obj);
             if (target)
               evas_object_geometry_get(target, &x, &y, &w, &h);
          }

        pd->abs_pivot.cx = x + (w * pd->rel_pivot.cx);
        pd->abs_pivot.cy = y + (h * pd->rel_pivot.cy);
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

EOLIAN static void
_efl_animation_scale_efl_playable_progress_set(Eo *eo_obj,
                            Efl_Animation_Scale_Data *pd,
                            double progress)
{
   Efl_Animation_Scale_Property prev_scale;
   Efl_Animation_Scale_Property new_scale;

   efl_playable_progress_set(efl_super(eo_obj, MY_CLASS), progress);
   progress = efl_playable_progress_get(eo_obj);
   Efl_Canvas_Object *target = efl_animation_target_get(eo_obj);
   if (!target) return;

   prev_scale = _scale_get(target);
   new_scale.scale_x = GET_STATUS(pd->from.scale_x, pd->to.scale_x, progress);
   new_scale.scale_y = GET_STATUS(pd->from.scale_y, pd->to.scale_y, progress);

   if (pd->use_rel_pivot)
     {
        efl_gfx_map_zoom(target,
                         new_scale.scale_x / prev_scale.scale_x, new_scale.scale_y / prev_scale.scale_y,
                         pd->rel_pivot.obj,
                         pd->rel_pivot.cx, pd->rel_pivot.cy);
     }
   else
     {
        efl_gfx_map_zoom_absolute(target,
                                  new_scale.scale_x / prev_scale.scale_x, new_scale.scale_y / prev_scale.scale_y,
                                  pd->abs_pivot.cx, pd->abs_pivot.cy);
     }
}

EOLIAN static Efl_Object *
_efl_animation_scale_efl_object_constructor(Eo *eo_obj,
                                            Efl_Animation_Scale_Data *pd)
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

#include "efl_animation_scale.eo.c"
