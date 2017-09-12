#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Ecore.h"
#include "ecore_private.h"

#define MY_CLASS EFL_INTERPOLATOR_LINEAR_CLASS

typedef struct _Efl_Interpolator_Linear_Data Efl_Interpolator_Linear_Data;

struct _Efl_Interpolator_Linear_Data
{
};

EOLIAN static double
_efl_interpolator_linear_efl_interpolator_interpolate(Eo *eo_obj EINA_UNUSED,
                                                      Efl_Interpolator_Linear_Data *pd EINA_UNUSED,
                                                      double progress)
{
   if ((progress < 0.0) || (progress > 1.0))
     return progress;

   return ecore_animator_pos_map(progress, ECORE_POS_MAP_LINEAR, 0, 0);
}

#include "efl_interpolator_linear.eo.c"
