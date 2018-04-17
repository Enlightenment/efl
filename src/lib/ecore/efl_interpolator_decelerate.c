#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Ecore.h"
#include "ecore_private.h"

#define MY_CLASS EFL_INTERPOLATOR_DECELERATE_CLASS

typedef struct _Efl_Interpolator_Decelerate_Data Efl_Interpolator_Decelerate_Data;

struct _Efl_Interpolator_Decelerate_Data
{
   double factor;
};

EOLIAN static double
_efl_interpolator_decelerate_efl_interpolator_interpolate(Eo *eo_obj EINA_UNUSED,
                                                          Efl_Interpolator_Decelerate_Data *pd EINA_UNUSED,
                                                          double progress)
{
   if ((progress < 0.0) || (progress > 1.0))
     return progress;

   return ecore_animator_pos_map(progress, ECORE_POS_MAP_DECELERATE_FACTOR,
                                 pd->factor, 0);
}

EOLIAN static void
_efl_interpolator_decelerate_factor_set(Eo *eo_obj EINA_UNUSED,
                                        Efl_Interpolator_Decelerate_Data *pd,
                                        double factor)
{
   pd->factor = factor;
}

EOLIAN static double
_efl_interpolator_decelerate_factor_get(const Eo *eo_obj EINA_UNUSED,
                                        Efl_Interpolator_Decelerate_Data *pd EINA_UNUSED)
{
   return pd->factor;
}

EOLIAN static Efl_Object *
_efl_interpolator_decelerate_efl_object_constructor(Eo *eo_obj,
                                                    Efl_Interpolator_Decelerate_Data *pd)
{
   eo_obj = efl_constructor(efl_super(eo_obj, MY_CLASS));

   pd->factor = 1.0;

   return eo_obj;
}

#include "efl_interpolator_decelerate.eo.c"
