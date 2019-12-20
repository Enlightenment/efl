#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Ecore.h"
#include "ecore_private.h"

#define MY_CLASS EFL_BOUNCE_INTERPOLATOR_CLASS

typedef struct _Efl_Bounce_Interpolator_Data Efl_Bounce_Interpolator_Data;

struct _Efl_Bounce_Interpolator_Data
{
   double rigidness;
   int bounces;
};

EOLIAN static double
_efl_bounce_interpolator_efl_interpolator_interpolate(Eo *eo_obj EINA_UNUSED,
                                                      Efl_Bounce_Interpolator_Data *pd EINA_UNUSED,
                                                      double progress)
{
   if ((progress < 0.0) || (progress > 1.0))
     return progress;

   return ecore_animator_pos_map(progress, ECORE_POS_MAP_BOUNCE,
                                 pd->rigidness, (double)pd->bounces);
}

EOLIAN static void
_efl_bounce_interpolator_rigidness_set(Eo *eo_obj EINA_UNUSED,
                                       Efl_Bounce_Interpolator_Data *pd,
                                       double rigidness)
{
   pd->rigidness = rigidness;
}

EOLIAN static double
_efl_bounce_interpolator_rigidness_get(const Eo *eo_obj EINA_UNUSED,
                                       Efl_Bounce_Interpolator_Data *pd)
{
   return pd->rigidness;
}

EOLIAN static void
_efl_bounce_interpolator_bounces_set(Eo *eo_obj EINA_UNUSED,
                                     Efl_Bounce_Interpolator_Data *pd,
                                     int bounces)
{
   pd->bounces = bounces;
}

EOLIAN static int
_efl_bounce_interpolator_bounces_get(const Eo *eo_obj EINA_UNUSED,
                                     Efl_Bounce_Interpolator_Data *pd)
{
   return pd->bounces;
}

EOLIAN static Efl_Object *
_efl_bounce_interpolator_efl_object_constructor(Eo *eo_obj,
                                                Efl_Bounce_Interpolator_Data *pd)
{
   eo_obj = efl_constructor(efl_super(eo_obj, MY_CLASS));

   pd->rigidness = 1.0;
   pd->bounces = 1;

   return eo_obj;
}

#include "efl_bounce_interpolator.eo.c"
