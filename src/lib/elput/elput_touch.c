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

EAPI Eina_Bool
elput_touch_dwt_enabled_set(Elput_Device *device, Eina_Bool enabled)
{
   Eina_Bool ret = EINA_FALSE;

   EINA_SAFETY_ON_NULL_RETURN_VAL(device, EINA_FALSE);

   if (!libinput_device_config_dwt_is_available(device->device))
     return EINA_FALSE;

   if (enabled)
     {
        ret =
          libinput_device_config_dwt_set_enabled(device->device,
                                                 LIBINPUT_CONFIG_DWT_ENABLED);
     }
   else
     {
        ret =
          libinput_device_config_dwt_set_enabled(device->device,
                                                 LIBINPUT_CONFIG_DWT_DISABLED);
     }

   return ret;
}

EAPI Eina_Bool
elput_touch_dwt_enabled_get(Elput_Device *device)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(device, EINA_FALSE);

   if (!libinput_device_config_dwt_is_available(device->device))
     return EINA_FALSE;

   return libinput_device_config_dwt_get_enabled(device->device);
}

EAPI Eina_Bool
elput_touch_scroll_method_set(Elput_Device *device, int method)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(device, EINA_FALSE);

   if (libinput_device_config_scroll_set_method(device->device, method) ==
       LIBINPUT_CONFIG_STATUS_SUCCESS)
     return EINA_TRUE;

   return EINA_FALSE;
}

EAPI int
elput_touch_scroll_method_get(Elput_Device *device)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(device, -1);

   return libinput_device_config_scroll_get_method(device->device);
}
