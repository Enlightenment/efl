#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Ecore.h"
#include "ecore_private.h"

#define MY_CLASS EFL_INTERPOLATOR_CUBIC_BEZIER_CLASS

typedef struct _Efl_Interpolator_Cubic_Bezier_Data Efl_Interpolator_Cubic_Bezier_Data;

struct _Efl_Interpolator_Cubic_Bezier_Data
{
   double factor[4];
};

EOLIAN static double
_efl_interpolator_cubic_bezier_efl_interpolator_interpolate(Eo *eo_obj EINA_UNUSED,
                                                            Efl_Interpolator_Cubic_Bezier_Data *pd EINA_UNUSED,
                                                            double progress)
{
   if ((progress < 0.0) || (progress > 1.0))
     return progress;

   return ecore_animator_pos_map_n(progress, ECORE_POS_MAP_CUBIC_BEZIER, 4,
                                   pd->factor);
}

EOLIAN static void
_efl_interpolator_cubic_bezier_factors_set(Eo *eo_obj EINA_UNUSED,
                                           Efl_Interpolator_Cubic_Bezier_Data *pd,
                                           double factor1, double factor2,
                                           double factor3, double factor4)
{
   pd->factor[0] = factor1;
   pd->factor[1] = factor2;
   pd->factor[2] = factor3;
   pd->factor[3] = factor4;
}

EOLIAN static void
_efl_interpolator_cubic_bezier_factors_get(const Eo *eo_obj EINA_UNUSED,
                                           Efl_Interpolator_Cubic_Bezier_Data *pd,
                                           double *factor1, double *factor2,
                                           double *factor3, double *factor4)
{
   if (factor1)
     *factor1 = pd->factor[0];

   if (factor2)
     *factor2 = pd->factor[1];

   if (factor3)
     *factor3 = pd->factor[2];

   if (factor4)
     *factor4 = pd->factor[3];
}

EOLIAN static Efl_Object *
_efl_interpolator_cubic_bezier_efl_object_constructor(Eo *eo_obj,
                                                      Efl_Interpolator_Cubic_Bezier_Data *pd)
{
   eo_obj = efl_constructor(efl_super(eo_obj, MY_CLASS));

   pd->factor[0] = 1.0;
   pd->factor[1] = 1.0;
   pd->factor[2] = 1.0;
   pd->factor[3] = 1.0;

   return eo_obj;
}

#include "efl_interpolator_cubic_bezier.eo.c"
