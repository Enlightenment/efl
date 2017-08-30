#include "efl_animation_rotate_private.h"

EOLIAN static void
_efl_animation_rotate_rotate_set(Eo *eo_obj,
                                 Efl_Animation_Rotate_Data *pd,
                                 double from_degree,
                                 double to_degree,
                                 Efl_Canvas_Object *pivot,
                                 double cx,
                                 double cy)
{
   EFL_ANIMATION_ROTATE_CHECK_OR_RETURN(eo_obj);

   pd->from.degree = from_degree;
   pd->to.degree = to_degree;

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
_efl_animation_rotate_rotate_get(Eo *eo_obj,
                                 Efl_Animation_Rotate_Data *pd,
                                 double *from_degree,
                                 double *to_degree,
                                 Efl_Canvas_Object **pivot,
                                 double *cx,
                                 double *cy)
{
   EFL_ANIMATION_ROTATE_CHECK_OR_RETURN(eo_obj);

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
_efl_animation_rotate_rotate_absolute_set(Eo *eo_obj,
                                          Efl_Animation_Rotate_Data *pd,
                                          double from_degree,
                                          double to_degree,
                                          Evas_Coord cx,
                                          Evas_Coord cy)
{
   EFL_ANIMATION_ROTATE_CHECK_OR_RETURN(eo_obj);

   pd->from.degree = from_degree;
   pd->to.degree = to_degree;

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
_efl_animation_rotate_rotate_absolute_get(Eo *eo_obj,
                                          Efl_Animation_Rotate_Data *pd,
                                          double *from_degree,
                                          double *to_degree,
                                          Evas_Coord *cx,
                                          Evas_Coord *cy)
{
   EFL_ANIMATION_ROTATE_CHECK_OR_RETURN(eo_obj);

   //Update relative pivot based on absolute pivot
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

   if (from_degree)
     *from_degree = pd->from.degree;

   if (to_degree)
     *to_degree = pd->to.degree;

   if (cx)
     *cx = pd->abs_pivot.cx;

   if (cy)
     *cy = pd->abs_pivot.cy;
}

EOLIAN static Efl_Animation_Object *
_efl_animation_rotate_efl_animation_object_create(Eo *eo_obj,
                                                  Efl_Animation_Rotate_Data *pd)
{
   EFL_ANIMATION_ROTATE_CHECK_OR_RETURN(eo_obj, NULL);

   Efl_Animation_Object_Rotate *anim_obj
      = efl_add(EFL_ANIMATION_OBJECT_ROTATE_CLASS, NULL);

   Efl_Canvas_Object *target = efl_animation_target_get(eo_obj);
   efl_animation_object_target_set(anim_obj, target);

   Eina_Bool state_keep = efl_animation_final_state_keep_get(eo_obj);
   efl_animation_object_final_state_keep_set(anim_obj, state_keep);

   double duration = efl_animation_duration_get(eo_obj);
   efl_animation_object_duration_set(anim_obj, duration);

   double total_duration = efl_animation_total_duration_get(eo_obj);
   efl_animation_object_total_duration_set(anim_obj, total_duration);

   int repeat_count = efl_animation_repeat_count_get(eo_obj);
   efl_animation_object_repeat_count_set(anim_obj, repeat_count);

   if (pd->use_rel_pivot)
     {
        efl_animation_object_rotate_set(anim_obj,
                                        pd->from.degree,
                                        pd->to.degree,
                                        pd->rel_pivot.obj,
                                        pd->rel_pivot.cx,
                                        pd->rel_pivot.cy);
     }
   else
     {
        efl_animation_object_rotate_absolute_set(anim_obj,
                                                 pd->from.degree,
                                                 pd->to.degree,
                                                 pd->abs_pivot.cx,
                                                 pd->abs_pivot.cy);
     }

   return anim_obj;
}

EOLIAN static Efl_Object *
_efl_animation_rotate_efl_object_constructor(Eo *eo_obj,
                                             Efl_Animation_Rotate_Data *pd)
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

#include "efl_animation_rotate.eo.c"
