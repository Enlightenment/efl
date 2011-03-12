#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <Eeze.h>
#include "eeze_udev_private.h"

/**
 * @addtogroup find Find
 *
 * These are functions which find/supplement lists of devices.
 *
 * @ingroup udev
 *
 * @{
 */

/**
 * Returns a stringshared list of all syspaths that are (or should be) the same
 * device as the device pointed at by @p syspath.
 *
 * @param syspath The syspath of the device to find matches for
 * @return All devices which are the same as the one passed
 */
EAPI Eina_List *
eeze_udev_find_similar_from_syspath(const char *syspath)
{
   _udev_device *device;
   _udev_list_entry *devs, *cur;
   _udev_enumerate *en;
   Eina_List *l, *ret = NULL;
   const char *vendor, *model, *revision, *devname, *dev;

   if (!syspath)
     return NULL;

   en = udev_enumerate_new((udev));

   if (!en)
     return NULL;

   if (!(device = _new_device(syspath)))
     return NULL;

   vendor = udev_device_get_property_value(device, "ID_VENDOR_ID");

   if (vendor)
     udev_enumerate_add_match_property(en, "ID_VENDOR_ID", vendor);

   model = udev_device_get_property_value(device, "ID_MODEL_ID");

   if (model)
     udev_enumerate_add_match_property(en, "ID_MODEL_ID", model);

   revision = udev_device_get_property_value(device, "ID_REVISION");

   if (revision)
     udev_enumerate_add_match_property(en, "ID_REVISION", revision);

   udev_enumerate_scan_devices(en);
   udev_device_unref(device);
   devs = udev_enumerate_get_list_entry(en);
   udev_list_entry_foreach(cur, devs)
     {
        devname = udev_list_entry_get_name(cur);
        /* verify unlisted device */

        EINA_LIST_FOREACH(ret, l, dev)
          if (!strcmp(dev, devname))
            continue;

        ret = eina_list_prepend(ret, eina_stringshare_add(devname));
        device = udev_device_new_from_syspath(udev, devname);

        /* only device roots have this sysattr,
         * and we only need to check parents of the roots
         */
        if (udev_device_get_sysattr_value(device, "idVendor"))
          ret = _get_unlisted_parents(ret, device);

        udev_device_unref(device);
     }
   udev_enumerate_unref(en);
   return ret;
}

/**
 * Updates a list of all syspaths that are (or should be) the same
 * device.
 *
 * @param list The list of devices to update
 * @return The updated list
 *
 * This function will update @p list to include all devices matching
 * devices with syspaths currently stored in @p list.  All strings are
 * stringshared.
 */
EAPI Eina_List *
eeze_udev_find_unlisted_similar(Eina_List *list)
{
   _udev_device *device;
   _udev_list_entry *devs, *cur;
   _udev_enumerate *en;
   Eina_List *l;
   const char *vendor, *model, *revision, *devname, *dev;

   if (!list)
     return NULL;

   EINA_LIST_FOREACH(list, l, dev)
     {
        en = udev_enumerate_new((udev));

        if (!en)
          return NULL;

        device = _new_device(dev);
        if (!device) continue;

        if ((vendor = udev_device_get_property_value(device, "ID_VENDOR_ID")))
          udev_enumerate_add_match_property(en, "ID_VENDOR_ID", vendor);
        else
        if ((vendor = udev_device_get_property_value(device, "ID_VENDOR")))
          udev_enumerate_add_match_property(en, "ID_VENDOR", vendor);

        if ((model = udev_device_get_property_value(device, "ID_MODEL_ID")))
          udev_enumerate_add_match_property(en, "ID_MODEL_ID", model);
        else
        if ((model = udev_device_get_property_value(device, "ID_MODEL")))
          udev_enumerate_add_match_property(en, "ID_MODEL", model);

        if ((revision = udev_device_get_property_value(device, "ID_REVISION")))
          udev_enumerate_add_match_property(en, "ID_REVISION", revision);

        udev_enumerate_scan_devices(en);
        udev_device_unref(device);
        devs = udev_enumerate_get_list_entry(en);
        udev_list_entry_foreach(cur, devs)
          {
             devname = udev_list_entry_get_name(cur);
             device = udev_device_new_from_syspath(udev, devname);

             /* only device roots have this sysattr,
              * and we only need to check parents of the roots
              */
             if (udev_device_get_sysattr_value(device, "idVendor"))
               list = _get_unlisted_parents(list, device);

             udev_device_unref(device);
          }
        udev_enumerate_unref(en);
     }
   return list;
}

/**
 * Find devices using an #Eeze_Udev_Type and/or a name.
 *
 * @param etype An #Eeze_Udev_Type or 0
 * @param name A filter for the device name or #NULL
 * @return A stringshared Eina_List of matched devices or #NULL on failure
 *
 * Return a list of syspaths (/sys/$syspath) for matching udev devices.
 */
EAPI Eina_List *
eeze_udev_find_by_type(Eeze_Udev_Type etype,
                       const char    *name)
{
   _udev_enumerate *en;
   _udev_list_entry *devs, *cur;
   _udev_device *device, *parent;
   const char *devname, *test;
   Eina_List *ret = NULL;

   if ((!etype) && (!name))
     return NULL;

   en = udev_enumerate_new((udev));

   if (!en)
     return NULL;

   switch (etype)
     {
      case EEZE_UDEV_TYPE_NONE:
        break;

      case EEZE_UDEV_TYPE_KEYBOARD:
        udev_enumerate_add_match_subsystem(en, "input");
#ifndef OLD_UDEV_RRRRRRRRRRRRRR
        udev_enumerate_add_match_property(en, "ID_INPUT_KEYBOARD", "1");
#else
        udev_enumerate_add_match_property(en, "ID_CLASS", "kbd");
#endif
        break;

      case EEZE_UDEV_TYPE_MOUSE:
        udev_enumerate_add_match_subsystem(en, "input");
#ifndef OLD_UDEV_RRRRRRRRRRRRRR
        udev_enumerate_add_match_property(en, "ID_INPUT_MOUSE", "1");
#else
        udev_enumerate_add_match_property(en, "ID_CLASS", "mouse");
#endif
        break;

      case EEZE_UDEV_TYPE_TOUCHPAD:
        udev_enumerate_add_match_subsystem(en, "input");
#ifndef OLD_UDEV_RRRRRRRRRRRRRR
        udev_enumerate_add_match_property(en, "ID_INPUT_TOUCHPAD", "1");
#endif
        break;

      case EEZE_UDEV_TYPE_DRIVE_MOUNTABLE:
        udev_enumerate_add_match_subsystem(en, "block");
        udev_enumerate_add_match_property(en, "ID_FS_USAGE", "filesystem");
        /* parent node */
        udev_enumerate_add_nomatch_sysattr(en, "capability", "50");
        break;

      case EEZE_UDEV_TYPE_DRIVE_INTERNAL:
        udev_enumerate_add_match_subsystem(en, "block");
        udev_enumerate_add_match_property(en, "ID_TYPE", "disk");
        udev_enumerate_add_match_property(en, "ID_BUS", "ata");
        udev_enumerate_add_match_sysattr(en, "removable", "0");
        break;

      case EEZE_UDEV_TYPE_DRIVE_REMOVABLE:
        udev_enumerate_add_match_subsystem(en, "block");
        udev_enumerate_add_match_property(en, "ID_TYPE", "disk");
        break;

      case EEZE_UDEV_TYPE_DRIVE_CDROM:
        udev_enumerate_add_match_subsystem(en, "block");
        udev_enumerate_add_match_property(en, "ID_CDROM", "1");
        break;

      case EEZE_UDEV_TYPE_POWER_AC:
        udev_enumerate_add_match_subsystem(en, "power_supply");
        udev_enumerate_add_match_sysattr(en, "type", "Mains");
        break;

      case EEZE_UDEV_TYPE_POWER_BAT:
        udev_enumerate_add_match_subsystem(en, "power_supply");
        udev_enumerate_add_match_sysattr(en, "type", "Battery");
        break;

      case EEZE_UDEV_TYPE_NET:
        udev_enumerate_add_match_subsystem(en, "net");
        break;

      case EEZE_UDEV_TYPE_IS_IT_HOT_OR_IS_IT_COLD_SENSOR:
        udev_enumerate_add_match_subsystem(en, "hwmon");
        break;

      /*
              case EEZE_UDEV_TYPE_ANDROID:
                udev_enumerate_add_match_subsystem(en, "block");
                udev_enumerate_add_match_property(en, "ID_MODEL", "Android_*");
                break;
       */
      default:
        break;
     }

   udev_enumerate_scan_devices(en);
   devs = udev_enumerate_get_list_entry(en);
   udev_list_entry_foreach(cur, devs)
     {
        devname = udev_list_entry_get_name(cur);
        device = udev_device_new_from_syspath(udev, devname);

        if (etype == EEZE_UDEV_TYPE_IS_IT_HOT_OR_IS_IT_COLD_SENSOR) /* ensure that temp input exists somewhere in this device chain */
          {
             if (!_walk_parents_test_attr(device, "temp1_input", NULL))
               goto out;

             /* if device is not the one which has the temp input, we must go up the chain */
             if (!udev_device_get_sysattr_value(device, "temp1_input"))
               {
                  devname = NULL;

                  for (parent = udev_device_get_parent(device); parent; parent = udev_device_get_parent(parent)) /*check for parent */
                    if ((udev_device_get_sysattr_value(parent, "temp1_input")))
                      {
                         devname = udev_device_get_syspath(parent);
                         break;
                      }

                  if (!devname)
                    goto out;
               }
          }
        else if (etype == EEZE_UDEV_TYPE_DRIVE_INTERNAL)
          {
             if (udev_device_get_property_value(device, "ID_USB_DRIVER"))
               goto out;
          }
        else if (etype == EEZE_UDEV_TYPE_DRIVE_REMOVABLE)
          {
             if (!(test = udev_device_get_property_value(device, "ID_BUS")) || (!strncmp(test, "ata", 3)))
               goto out;
          }
        else if (etype == EEZE_UDEV_TYPE_DRIVE_MOUNTABLE)
          {
             int devcheck;

             devcheck = open(udev_device_get_devnode(device), O_RDONLY | O_EXCL);
             if (devcheck < 0) goto out;
             close(devcheck);
          }

        if (name && (!strstr(devname, name)))
          goto out;

        ret = eina_list_append(ret, eina_stringshare_add(devname));
out:
        udev_device_unref(device);
     }
   udev_enumerate_unref(en);
   return ret;
}

/**
 * A more advanced find, allows finds using udev properties.
 *
 * @param subsystem The udev subsystem to filter by, or NULL
 * @param type "ID_INPUT_KEY", "ID_INPUT_MOUSE", "ID_INPUT_TOUCHPAD", NULL, etc
 * @param name A filter for the device name, or NULL
 * @return A stringshared Eina_List* of matched devices or NULL on failure
 *
 * Return a list of syspaths (/sys/$syspath) for matching udev devices.
 * Requires at least one filter.
 */
EAPI Eina_List *
eeze_udev_find_by_filter(const char *subsystem,
                         const char *type,
                         const char *name)
{
   _udev_enumerate *en;
   _udev_list_entry *devs, *cur;
   _udev_device *device;
   const char *devname;
   Eina_List *ret = NULL;

   if ((!subsystem) && (!type) && (!name))
     return NULL;

   en = udev_enumerate_new((udev));

   if (!en)
     return NULL;

   if (subsystem)
     udev_enumerate_add_match_subsystem(en, subsystem);

   udev_enumerate_add_match_property(en, type, "1");
   udev_enumerate_scan_devices(en);
   devs = udev_enumerate_get_list_entry(en);
   udev_list_entry_foreach(cur, devs)
     {
        devname = udev_list_entry_get_name(cur);
        device = udev_device_new_from_syspath(udev, devname);

        if (name)
          if (!strstr(devname, name))
            goto out;

        ret = eina_list_append(ret, eina_stringshare_add(devname));
out:
        udev_device_unref(device);
     }
   udev_enumerate_unref(en);
   return ret;
}

/**
 * Find a list of devices by a sysattr (and, optionally, a value of that sysattr).
 *
 * @param sysattr The attribute to find
 * @param value Optional: the value that the attribute should have
 *
 * @return A stringshared list of the devices found with the attribute
 *
 * @ingroup find
 */
EAPI Eina_List *
eeze_udev_find_by_sysattr(const char *sysattr,
                          const char *value)
{
   _udev_enumerate *en;
   _udev_list_entry *devs, *cur;
   _udev_device *device;
   const char *devname;
   Eina_List *ret = NULL;

   if (!sysattr)
     return NULL;

   en = udev_enumerate_new((udev));

   if (!en)
     return NULL;

   udev_enumerate_add_match_sysattr(en, sysattr, value);
   udev_enumerate_scan_devices(en);
   devs = udev_enumerate_get_list_entry(en);
   udev_list_entry_foreach(cur, devs)
     {
        devname = udev_list_entry_get_name(cur);
        device = udev_device_new_from_syspath(udev, devname);
        ret = eina_list_append(ret, eina_stringshare_add(devname));
        udev_device_unref(device);
     }
   udev_enumerate_unref(en);
   return ret;
}

/** @} */
