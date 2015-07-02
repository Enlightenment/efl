#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ecordova_vibration_private.h"

#include <haptic.h>

#define MY_CLASS ECORDOVA_VIBRATION_CLASS
#define MY_CLASS_NAME "Ecordova_Vibration"

static size_t _ref_count = 0;

static Eo_Base *
_ecordova_vibration_eo_base_constructor(Eo *obj, Ecordova_Vibration_Data *pd)
{
   DBG("(%p)", obj);

   pd->obj = obj;

   ++_ref_count;
   if (1 == _ref_count)
     {
        int ret = haptic_initialize();
        EINA_SAFETY_ON_FALSE_RETURN_VAL(HAPTIC_ERROR_NONE == ret, NULL);
     }

   return eo_do_super_ret(obj, MY_CLASS, obj, eo_constructor());
}

static void
_ecordova_vibration_constructor(Eo *obj EINA_UNUSED,
                                Ecordova_Vibration_Data *pd EINA_UNUSED)
{
   DBG("(%p)", obj);
}

static void
_ecordova_vibration_eo_base_destructor(Eo *obj,
                                       Ecordova_Vibration_Data *pd EINA_UNUSED)
{
   DBG("(%p)", obj);

   EINA_SAFETY_ON_FALSE_RETURN(_ref_count >= 1);

   --_ref_count;
   if (0 == _ref_count)
     {
        int ret = haptic_deinitialize();
        EINA_SAFETY_ON_FALSE_RETURN(HAPTIC_ERROR_NONE == ret);
     }

   eo_do_super(obj, MY_CLASS, eo_destructor());
}

static void
_ecordova_vibration_vibrate(Eo *obj,
                            Ecordova_Vibration_Data *pd EINA_UNUSED,
                            int time)
{
   DBG("(%p)", obj);
   int ret = haptic_vibrate_monotone(0, time, 100);
   EINA_SAFETY_ON_FALSE_RETURN(HAPTIC_ERROR_NONE == ret);
}

#include "ecordova_vibration.eo.c"
