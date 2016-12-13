#include "elput_private.h"

EAPI Eina_Bool
elput_touch_drag_enabled_set(Elput_Device *device, Eina_Bool enabled)
{
   Eina_Bool ret = EINA_FALSE;

   EINA_SAFETY_ON_NULL_RETURN_VAL(device, EINA_FALSE);

   if (enabled)
     {
        ret =
          libinput_device_config_tap_set_drag_enabled(device->device,
                                                      LIBINPUT_CONFIG_DRAG_ENABLED);
     }
   else
     {
        ret =
          libinput_device_config_tap_set_drag_enabled(device->device,
                                                      LIBINPUT_CONFIG_DRAG_DISABLED);
     }

   return ret;
}

EAPI Eina_Bool
elput_touch_drag_enabled_get(Elput_Device *device)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(device, EINA_FALSE);

   return libinput_device_config_tap_get_drag_enabled(device->device);
}

EAPI Eina_Bool
elput_touch_drag_lock_enabled_set(Elput_Device *device, Eina_Bool enabled)
{
   Eina_Bool ret = EINA_FALSE;

   EINA_SAFETY_ON_NULL_RETURN_VAL(device, EINA_FALSE);

   if (enabled)
     {
        ret =
          libinput_device_config_tap_set_drag_lock_enabled(device->device,
                                                           LIBINPUT_CONFIG_DRAG_LOCK_ENABLED);
     }
   else
     {
        ret =
          libinput_device_config_tap_set_drag_lock_enabled(device->device,
                                                           LIBINPUT_CONFIG_DRAG_LOCK_DISABLED);
     }

   return ret;
}

EAPI Eina_Bool
elput_touch_drag_lock_enabled_get(Elput_Device *device)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(device, EINA_FALSE);

   return libinput_device_config_tap_get_drag_lock_enabled(device->device);
}
