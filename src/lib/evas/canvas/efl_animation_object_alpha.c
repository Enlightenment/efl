#include "efl_animation_object_alpha_private.h"

EOLIAN static void
_efl_animation_object_alpha_alpha_set(Eo *eo_obj,
                                      Efl_Animation_Object_Alpha_Data *pd,
                                      double from_alpha,
                                      double to_alpha)
{
   EFL_ANIMATION_OBJECT_ALPHA_CHECK_OR_RETURN(eo_obj);

   pd->from.alpha = from_alpha;
   pd->to.alpha = to_alpha;
}

EOLIAN static void
_efl_animation_object_alpha_alpha_get(Eo *eo_obj EINA_UNUSED,
                                      Efl_Animation_Object_Alpha_Data *pd,
                                      double *from_alpha,
                                      double *to_alpha)
{
   EFL_ANIMATION_OBJECT_ALPHA_CHECK_OR_RETURN(eo_obj);

   if (from_alpha)
     *from_alpha = pd->from.alpha;
   if (to_alpha)
     *to_alpha = pd->to.alpha;
}

static void
_progress_set(Eo *eo_obj, double progress)
{
   EFL_ANIMATION_OBJECT_ALPHA_DATA_GET(eo_obj, pd);

   Efl_Canvas_Object *target = efl_animation_object_target_get(eo_obj);
   if (!target) return;

   double alpha
      = (pd->from.alpha * (1.0 - progress)) + (pd->to.alpha * progress);

   int r[4], g[4], b[4], a[4];
   int i;
   for (i = 0; i < 4; i++)
     efl_gfx_map_color_get(target, i, &r[i], &g[i], &b[i], &a[i]);

   for (i = 0; i < 4; i++)
     {
        r[i] = (int)(r[i] * alpha);
        g[i] = (int)(g[i] * alpha);
        b[i] = (int)(b[i] * alpha);
        a[i] = (int)(a[i] * alpha);
        efl_gfx_map_color_set(target, i, r[i], g[i], b[i], a[i]);
     }
}

EOLIAN static void
_efl_animation_object_alpha_efl_animation_object_progress_set(Eo *eo_obj,
                                                              Efl_Animation_Object_Alpha_Data *pd EINA_UNUSED,
                                                              double progress)
{
   EFL_ANIMATION_OBJECT_ALPHA_CHECK_OR_RETURN(eo_obj);

   if ((progress < 0.0) || (progress > 1.0)) return;

   _progress_set(eo_obj, progress);

   efl_animation_object_progress_set(efl_super(eo_obj, MY_CLASS), progress);
}

EOLIAN static Efl_Object *
_efl_animation_object_alpha_efl_object_constructor(Eo *eo_obj,
                                                   Efl_Animation_Object_Alpha_Data *pd)
{
   eo_obj = efl_constructor(efl_super(eo_obj, MY_CLASS));

   pd->from.alpha = 1.0;
   pd->to.alpha = 1.0;

   return eo_obj;
}

/* Internal EO APIs */

EOAPI EFL_VOID_FUNC_BODYV(efl_animation_object_alpha_set, EFL_FUNC_CALL(from_alpha, to_alpha), double from_alpha, double to_alpha);
EOAPI EFL_VOID_FUNC_BODYV_CONST(efl_animation_object_alpha_get, EFL_FUNC_CALL(from_alpha, to_alpha), double *from_alpha, double *to_alpha);

#define EFL_ANIMATION_OBJECT_ALPHA_EXTRA_OPS \
   EFL_OBJECT_OP_FUNC(efl_animation_object_alpha_set, _efl_animation_object_alpha_alpha_set), \
   EFL_OBJECT_OP_FUNC(efl_animation_object_alpha_get, _efl_animation_object_alpha_alpha_get)

#include "efl_animation_object_alpha.eo.c"
