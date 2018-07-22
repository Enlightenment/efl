#include "efl_canvas_animation_alpha_private.h"

#define MY_CLASS EFL_CANVAS_ANIMATION_ALPHA_CLASS

EOLIAN static void
_efl_canvas_animation_alpha_alpha_set(Eo *eo_obj EINA_UNUSED,
                               Efl_Canvas_Animation_Alpha_Data *pd,
                               double from_alpha,
                               double to_alpha)
{
   pd->from.alpha = from_alpha;
   pd->to.alpha = to_alpha;
}

EOLIAN static void
_efl_canvas_animation_alpha_alpha_get(const Eo *eo_obj EINA_UNUSED,
                               Efl_Canvas_Animation_Alpha_Data *pd,
                               double *from_alpha,
                               double *to_alpha)
{
   if (from_alpha)
     *from_alpha = pd->from.alpha;
   if (to_alpha)
     *to_alpha = pd->to.alpha;
}


EOLIAN static double
_efl_canvas_animation_alpha_efl_canvas_animation_animation_apply(Eo *eo_obj,
                               Efl_Canvas_Animation_Alpha_Data *pd EINA_UNUSED,
                               double progress,
                               Efl_Canvas_Object *target)
{
   double from_alpha, to_alpha;
   int cur_alpha;
   int i;

   progress = efl_animation_apply(efl_super(eo_obj, MY_CLASS), progress, target);
   if (!target) return progress;

   efl_animation_alpha_get(eo_obj, &from_alpha, &to_alpha);
   cur_alpha = (int)(GET_STATUS(from_alpha, to_alpha, progress) * 255);

   for (i = 0; i < 4; i++)
     {
        efl_gfx_map_color_set(target, i, cur_alpha, cur_alpha, cur_alpha, cur_alpha);
     }

   return progress;
}

EOLIAN static Efl_Object *
_efl_canvas_animation_alpha_efl_object_constructor(Eo *eo_obj,
                                            Efl_Canvas_Animation_Alpha_Data *pd)
{
   eo_obj = efl_constructor(efl_super(eo_obj, MY_CLASS));

   pd->from.alpha = 1.0;
   pd->to.alpha = 1.0;

   return eo_obj;
}

#include "efl_canvas_animation_alpha.eo.c"
