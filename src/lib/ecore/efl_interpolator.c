#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Ecore.h"
#include "ecore_private.h"

#define MY_CLASS EFL_INTERPOLATOR_CLASS

typedef struct _Efl_Interpolator_Data Efl_Interpolator_Data;

struct _Efl_Interpolator_Data
{
};

EOLIAN static double
_efl_interpolator_interpolate(Eo *eo_obj EINA_UNUSED,
                              Efl_Interpolator_Data *pd EINA_UNUSED,
                              double progress)
{
   return progress;
}

#include "efl_interpolator.eo.c"
