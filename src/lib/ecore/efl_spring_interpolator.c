#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Ecore.h"
#include "ecore_private.h"

#define MY_CLASS EFL_SPRING_INTERPOLATOR_CLASS

typedef struct _Efl_Spring_Interpolator_Data Efl_Spring_Interpolator_Data;

struct _Efl_Spring_Interpolator_Data
{
   double decay;
   int oscillations;
};

EOLIAN static double
_efl_spring_interpolator_efl_interpolator_interpolate(Eo *obj EINA_UNUSED,
                                                      Efl_Spring_Interpolator_Data *pd,
                                                      double progress)
{
   if ((progress < 0.0) || (progress > 1.0))
     return progress;

   return ecore_animator_pos_map(progress, ECORE_POS_MAP_SPRING,
                                 pd->decay, (double)pd->oscillations);
}

EOLIAN static void
_efl_spring_interpolator_decay_set(Eo *eo_obj EINA_UNUSED,
                                   Efl_Spring_Interpolator_Data *pd,
                                   double decay)
{
   pd->decay = decay;
}

EOLIAN static double
_efl_spring_interpolator_decay_get(const Eo *eo_obj EINA_UNUSED,
                                     Efl_Spring_Interpolator_Data *pd)
{
   return pd->decay;
}

EOLIAN static void
_efl_spring_interpolator_oscillations_set(Eo *eo_obj EINA_UNUSED,
                                          Efl_Spring_Interpolator_Data *pd,
                                          int oscillations)
{
   pd->oscillations = oscillations;
}

EOLIAN static int
_efl_spring_interpolator_oscillations_get(const Eo *eo_obj EINA_UNUSED,
                                          Efl_Spring_Interpolator_Data *pd)
{
   return pd->oscillations;
}

EOLIAN static Efl_Object *
_efl_spring_interpolator_efl_object_constructor(Eo *eo_obj,
                                                Efl_Spring_Interpolator_Data *pd)
{
   eo_obj = efl_constructor(efl_super(eo_obj, MY_CLASS));

   pd->decay = 1.0;
   pd->oscillations = 1;

   return eo_obj;
}

#include "efl_spring_interpolator.eo.c"
