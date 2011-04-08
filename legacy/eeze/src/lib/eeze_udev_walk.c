#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <Eeze.h>
#include "eeze_udev_private.h"

EAPI Eina_Bool
eeze_udev_walk_check_sysattr(const char *syspath,
                             const char *sysattr,
                             const char *value)
{
   _udev_device *device, *child, *parent;
   Eina_Bool ret = EINA_FALSE;
   const char *test = NULL;

   if (!udev)
     return EINA_FALSE;

   if (!(device = _new_device(syspath)))
     return EINA_FALSE;

   for (parent = device; parent;
        child = parent, parent = udev_device_get_parent(child))
     {
        if (!(test = udev_device_get_sysattr_value(parent, sysattr)))
          continue;
        if ((value && (!strcmp(test, value))) || (!value))
          {
             ret = EINA_TRUE;
             break;
          }
     }

   udev_device_unref(device);
   return ret;
}

EAPI const char *
eeze_udev_walk_get_sysattr(const char *syspath,
                           const char *sysattr)
{
   _udev_device *device, *child, *parent;
   const char *test = NULL;

   if (!syspath)
     return NULL;

   if (!(device = _new_device(syspath)))
     return NULL;

   for (parent = device; parent;
        child = parent, parent = udev_device_get_parent(child))
     {
        if ((test = udev_device_get_sysattr_value(parent, sysattr)))
          {
             test = eina_stringshare_add(test);
             udev_device_unref(device);
             return test;
          }
     }

   udev_device_unref(device);
   return NULL;
}
