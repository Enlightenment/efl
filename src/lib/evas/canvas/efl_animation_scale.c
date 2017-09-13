#include "efl_animation_scale_private.h"

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
   EFL_ANIMATION_SCALE_CHECK_OR_RETURN(eo_obj);

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
   EFL_ANIMATION_SCALE_CHECK_OR_RETURN(eo_obj);

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
   EFL_ANIMATION_SCALE_CHECK_OR_RETURN(eo_obj);

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
   EFL_ANIMATION_SCALE_CHECK_OR_RETURN(eo_obj);

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

EOLIAN static Efl_Animation_Object *
_efl_animation_scale_efl_animation_object_create(Eo *eo_obj,
                                                 Efl_Animation_Scale_Data *pd)
{
   EFL_ANIMATION_SCALE_CHECK_OR_RETURN(eo_obj, NULL);

   Efl_Animation_Object_Scale *anim_obj
      = efl_add(EFL_ANIMATION_OBJECT_SCALE_CLASS, NULL);

   Efl_Canvas_Object *target = efl_animation_target_get(eo_obj);
   efl_animation_object_target_set(anim_obj, target);

   Eina_Bool state_keep = efl_animation_final_state_keep_get(eo_obj);
   efl_animation_object_final_state_keep_set(anim_obj, state_keep);

   double duration = efl_animation_duration_get(eo_obj);
   efl_animation_object_duration_set(anim_obj, duration);

   double total_duration = efl_animation_total_duration_get(eo_obj);
   efl_animation_object_total_duration_set(anim_obj, total_duration);

   double start_delay_time = efl_animation_start_delay_get(eo_obj);
   efl_animation_object_start_delay_set(anim_obj, start_delay_time);

   Efl_Animation_Object_Repeat_Mode repeat_mode =
      (Efl_Animation_Object_Repeat_Mode)efl_animation_repeat_mode_get(eo_obj);
   efl_animation_object_repeat_mode_set(anim_obj, repeat_mode);

   int repeat_count = efl_animation_repeat_count_get(eo_obj);
   efl_animation_object_repeat_count_set(anim_obj, repeat_count);

   Efl_Interpolator *interpolator = efl_animation_interpolator_get(eo_obj);
   efl_animation_object_interpolator_set(anim_obj, interpolator);

   if (pd->use_rel_pivot)
     {
        efl_animation_object_scale_set(anim_obj,
                                       pd->from.scale_x, pd->from.scale_y,
                                       pd->to.scale_x, pd->to.scale_y,
                                       pd->rel_pivot.obj,
                                       pd->rel_pivot.cx, pd->rel_pivot.cy);
     }
   else
     {
        efl_animation_object_scale_absolute_set(anim_obj,
                                                pd->from.scale_x, pd->from.scale_y,
                                                pd->to.scale_x, pd->to.scale_y,
                                                pd->abs_pivot.cx, pd->abs_pivot.cy);
     }

   return anim_obj;
}

EOLIAN static Efl_Object *
_efl_animation_scale_efl_object_constructor(Eo *eo_obj,
                                            Efl_Animation_Scale_Data *pd)
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

#include "efl_animation_scale.eo.c"
