#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <Ecore.h>
#include <Eeze.h>
#include <Eeze_Disk.h>

#include "eeze_udev_private.h"
#include "eeze_disk_private.h"

EAPI const char *
eeze_disk_udev_get_property(Eeze_Disk *disk, const char *property)
{
   const char *ret;
   EINA_SAFETY_ON_NULL_RETURN_VAL(disk, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(property, NULL);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(!*property, NULL);

   ret = udev_device_get_property_value(disk->device, property);
   return eina_stringshare_add(ret);
}

EAPI const char *
eeze_disk_udev_get_sysattr(Eeze_Disk *disk, const char *sysattr)
{
   const char *ret;
   EINA_SAFETY_ON_NULL_RETURN_VAL(disk, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(sysattr, NULL);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(!*sysattr, NULL);

   ret = udev_device_get_sysattr_value(disk->device, sysattr);
   return eina_stringshare_add(ret);
}

EAPI const char *
eeze_disk_udev_get_parent(Eeze_Disk *disk)
{
   _udev_device *parent;
   EINA_SAFETY_ON_NULL_RETURN_VAL(disk, NULL);

   parent = udev_device_get_parent(disk->device);
   return eina_stringshare_add(udev_device_get_syspath(parent));
}

EAPI Eina_Bool
eeze_disk_udev_walk_check_sysattr(Eeze_Disk *disk,
                                  const char *sysattr,
                                  const char *value)
{
   _udev_device *child, *parent;
   const char *test = NULL;

   EINA_SAFETY_ON_NULL_RETURN_VAL(disk, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(sysattr, EINA_FALSE);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(!*sysattr, EINA_FALSE);

   for (parent = disk->device; parent;
        child = parent, parent = udev_device_get_parent(child))
     {
        if (!(test = udev_device_get_sysattr_value(parent, sysattr)))
          continue;
        if ((value && (!strcmp(test, value))) || (!value))
          {
             return EINA_TRUE;
             break;
          }
     }
   return EINA_FALSE;
}

EAPI const char *
eeze_disk_udev_walk_get_sysattr(Eeze_Disk *disk,
                                  const char *sysattr)
{
   _udev_device *child, *parent;
   const char *test = NULL;

   EINA_SAFETY_ON_NULL_RETURN_VAL(disk, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(sysattr, NULL);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(!*sysattr, NULL);

   for (parent = disk->device; parent;
        child = parent, parent = udev_device_get_parent(child))
     {
        test = udev_device_get_sysattr_value(parent, sysattr);
        if (test) return eina_stringshare_add(test);
     }
   return NULL;
}
