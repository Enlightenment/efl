#include "efl_animation_object_scale_private.h"

EOLIAN static void
_efl_animation_object_scale_scale_set(Eo *eo_obj,
                                      Efl_Animation_Object_Scale_Data *pd,
                                      double from_scale_x,
                                      double from_scale_y,
                                      double to_scale_x,
                                      double to_scale_y,
                                      Efl_Canvas_Object *pivot,
                                      double cx,
                                      double cy)
{
   EFL_ANIMATION_OBJECT_SCALE_CHECK_OR_RETURN(eo_obj);

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
        Efl_Canvas_Object *target = efl_animation_object_target_get(eo_obj);
        if (target)
          evas_object_geometry_get(target, &x, &y, &w, &h);
     }

   pd->abs_pivot.cx = x + (w * cx);
   pd->abs_pivot.cy = y + (h * cy);

   pd->use_rel_pivot = EINA_TRUE;
}

EOLIAN static void
_efl_animation_object_scale_scale_get(Eo *eo_obj,
                                      Efl_Animation_Object_Scale_Data *pd,
                                      double *from_scale_x,
                                      double *from_scale_y,
                                      double *to_scale_x,
                                      double *to_scale_y,
                                      Efl_Canvas_Object **pivot,
                                      double *cx,
                                      double *cy)
{
   EFL_ANIMATION_OBJECT_SCALE_CHECK_OR_RETURN(eo_obj);

   //Update relative pivot based on absolute pivot
   if (!pd->use_rel_pivot)
     {
        Evas_Coord x = 0;
        Evas_Coord y = 0;
        Evas_Coord w = 0;
        Evas_Coord h = 0;

        Efl_Canvas_Object *target = efl_animation_object_target_get(eo_obj);
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
_efl_animation_object_scale_scale_absolute_set(Eo *eo_obj,
                                               Efl_Animation_Object_Scale_Data *pd,
                                               double from_scale_x,
                                               double from_scale_y,
                                               double to_scale_x,
                                               double to_scale_y,
                                               Evas_Coord cx,
                                               Evas_Coord cy)
{
   EFL_ANIMATION_OBJECT_SCALE_CHECK_OR_RETURN(eo_obj);

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

   Efl_Canvas_Object *target = efl_animation_object_target_get(eo_obj);
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
_efl_animation_object_scale_scale_absolute_get(Eo *eo_obj,
                                               Efl_Animation_Object_Scale_Data *pd,
                                               double *from_scale_x,
                                               double *from_scale_y,
                                               double *to_scale_x,
                                               double *to_scale_y,
                                               Evas_Coord *cx,
                                               Evas_Coord *cy)
{
   EFL_ANIMATION_OBJECT_SCALE_CHECK_OR_RETURN(eo_obj);

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
             Efl_Canvas_Object *target
                = efl_animation_object_target_get(eo_obj);
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

static void
_progress_set(Eo *eo_obj, double progress)
{
   EFL_ANIMATION_OBJECT_SCALE_DATA_GET(eo_obj, pd);

   Efl_Canvas_Object *target = efl_animation_object_target_get(eo_obj);
   if (!target) return;

   double scale_x =
      (pd->from.scale_x * (1.0 - progress)) + (pd->to.scale_x * progress);

   double scale_y =
      (pd->from.scale_y * (1.0 - progress)) + (pd->to.scale_y * progress);

   if (pd->use_rel_pivot)
     {
        efl_gfx_map_zoom(target,
                         scale_x, scale_y,
                         pd->rel_pivot.obj,
                         pd->rel_pivot.cx, pd->rel_pivot.cy);
     }
   else
     {
        efl_gfx_map_zoom_absolute(target,
                                  scale_x, scale_y,
                                  pd->abs_pivot.cx, pd->abs_pivot.cy);
     }
}

EOLIAN static void
_efl_animation_object_scale_efl_animation_object_progress_set(Eo *eo_obj,
                                                              Efl_Animation_Object_Scale_Data *pd EINA_UNUSED,
                                                              double progress)
{
   EFL_ANIMATION_OBJECT_SCALE_CHECK_OR_RETURN(eo_obj);

   if ((progress < 0.0) || (progress > 1.0)) return;

   _progress_set(eo_obj, progress);

   efl_animation_object_progress_set(efl_super(eo_obj, MY_CLASS), progress);
}

EOLIAN static Efl_Object *
_efl_animation_object_scale_efl_object_constructor(Eo *eo_obj,
                                                   Efl_Animation_Object_Scale_Data *pd)
{
   eo_obj = efl_constructor(efl_super(eo_obj, MY_CLASS));

   pd->from.scale_x = 1.0;
   pd->from.scale_y = 1.0;

   pd->rel_pivot.obj = NULL;
   pd->rel_pivot.cx = 0.5;
   pd->rel_pivot.cy = 0.5;

   pd->abs_pivot.cx = 0;
   pd->abs_pivot.cy = 0;

   pd->use_rel_pivot = EINA_TRUE;

   return eo_obj;
}

/* Internal EO APIs */

EOAPI EFL_VOID_FUNC_BODYV(efl_animation_object_scale_set, EFL_FUNC_CALL(from_scale_x, from_scale_y, to_scale_x, to_scale_y, pivot, cx, cy), double from_scale_x, double from_scale_y, double to_scale_x, double to_scale_y, Efl_Canvas_Object *pivot, double cx, double cy);
EOAPI EFL_VOID_FUNC_BODYV_CONST(efl_animation_object_scale_get, EFL_FUNC_CALL(from_scale_x, from_scale_y, to_scale_x, to_scale_y, pivot, cx, cy), double *from_scale_x, double *from_scale_y, double *to_scale_x, double *to_scale_y, Efl_Canvas_Object **pivot, double *cx, double *cy);

EOAPI EFL_VOID_FUNC_BODYV(efl_animation_object_scale_absolute_set, EFL_FUNC_CALL(from_scale_x, from_scale_y, to_scale_x, to_scale_y, cx, cy), double from_scale_x, double from_scale_y, double to_scale_x, double to_scale_y, int cx, int cy);
EOAPI EFL_VOID_FUNC_BODYV_CONST(efl_animation_object_scale_absolute_get, EFL_FUNC_CALL(from_scale_x, from_scale_y, to_scale_x, to_scale_y, cx, cy), double *from_scale_x, double *from_scale_y, double *to_scale_x, double *to_scale_y, int *cx, int *cy);

#define EFL_ANIMATION_OBJECT_SCALE_EXTRA_OPS \
   EFL_OBJECT_OP_FUNC(efl_animation_object_scale_set, _efl_animation_object_scale_scale_set), \
   EFL_OBJECT_OP_FUNC(efl_animation_object_scale_get, _efl_animation_object_scale_scale_get), \
   EFL_OBJECT_OP_FUNC(efl_animation_object_scale_absolute_set, _efl_animation_object_scale_scale_absolute_set), \
   EFL_OBJECT_OP_FUNC(efl_animation_object_scale_absolute_get, _efl_animation_object_scale_scale_absolute_get)

#include "efl_animation_object_scale.eo.c"
