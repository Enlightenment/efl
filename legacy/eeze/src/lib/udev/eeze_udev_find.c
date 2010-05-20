#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "eeze_udev_private.h"
#include <Eeze_Udev.h>


/**
 * Returns a list of all syspaths that are (or should be) the same
 * device as the device pointed at by @p syspath.
 *
 * @param syspath The syspath of the device to find matches for
 * @return All devices which are the same as the one passed
 *
 * @ingroup udev
 */
EAPI Eina_List *
eeze_udev_find_similar_from_syspath(const char *syspath)
{
   struct udev *udev;
   struct udev_device *device;
   struct udev_list_entry *devs, *cur;
   struct udev_enumerate *en;
   Eina_List *l, *ret = NULL;
   Eina_Strbuf *sbuf;
   const char *vendor, *model, *revision, *devname, *dev;

   if (!syspath) return NULL;
   udev = udev_new();
   if (!udev) return NULL;
   en = udev_enumerate_new(udev);
   if (!en)
     {
        udev_unref(udev);
        return NULL;
     }
      
   sbuf = eina_strbuf_new();
   if (!strstr(syspath, "/sys/"))
     eina_strbuf_append(sbuf, "/sys/");
   eina_strbuf_append(sbuf, syspath);

   device = udev_device_new_from_syspath(udev, syspath);
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
        _get_unlisted_parents(ret, device);

        udev_device_unref(device);
    }

   udev_enumerate_unref(en);
   udev_unref(udev);
   eina_strbuf_free(sbuf);

   return ret;
}

/**
 * Updates a list of all syspaths that are (or should be) the same
 * device.
 *
 * @param syspath The syspath of the device to find matches for.
 *
 * This function will update @p list to include all devices matching
 * devices with syspaths currently stored in @p list.
 *
 * @ingroup udev
 */
EAPI void
eeze_udev_find_unlisted_similar(Eina_List *list)
{
   struct udev *udev;
   struct udev_device *device;
   struct udev_list_entry *devs, *cur;
   struct udev_enumerate *en;
   Eina_List *l;
   const char *vendor, *model, *revision, *devname, *dev;

   if (!list) return;
   udev = udev_new();
   if (!udev) return;

   EINA_LIST_FOREACH(list, l, dev)
     {
         en = udev_enumerate_new(udev);
         if (!en)
           {
              udev_unref(udev);
              return;
           }
        device = udev_device_new_from_syspath(udev, dev);
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
             device = udev_device_new_from_syspath(udev, devname);

             /* only device roots have this sysattr,
              * and we only need to check parents of the roots
              */
             if (udev_device_get_sysattr_value(device, "idVendor"))
               _get_unlisted_parents(list, device);

             udev_device_unref(device);
         }

        udev_enumerate_unref(en);
     }
   udev_unref(udev);
   return;
}

/**
 * Find devices using a EEZE_UDEV_TYPE_* and/or a name.
 *
 * @param type A Eeze_Udev_Type or 0
 * @param name A filter for the device name or NULL
 * @return A Eina_List* of matched devices or NULL on failure
 * 
 * Return a list of syspaths (/sys/$syspath) for matching udev devices.
 *
 * @ingroup udev
 */
EAPI Eina_List *
eeze_udev_find_by_type(const Eeze_Udev_Type etype, const char *name)
{
   struct udev *udev;
   struct udev_enumerate *en;
   struct udev_list_entry *devs, *cur;
   struct udev_device *device, *parent;
   const char *devname, *test;
   Eina_List *ret = NULL;

   if ((!etype) && (!name)) return NULL;

   udev = udev_new();
   if (!udev) return NULL;
   en = udev_enumerate_new(udev);
   if (!en) return NULL;

   switch (etype)
     {
        case EEZE_UDEV_TYPE_NONE:
          break;
        case EEZE_UDEV_TYPE_KEYBOARD:
          udev_enumerate_add_match_subsystem(en, "input");
          udev_enumerate_add_match_property(en, "ID_INPUT_KEYBOARD", "1");
          break;
        case EEZE_UDEV_TYPE_MOUSE:
          udev_enumerate_add_match_subsystem(en, "input");
          udev_enumerate_add_match_property(en, "ID_INPUT_MOUSE", "1");
          break;
        case EEZE_UDEV_TYPE_TOUCHPAD:
          udev_enumerate_add_match_subsystem(en, "input");
          udev_enumerate_add_match_property(en, "ID_INPUT_TOUCHPAD", "1");
          break;
        case EEZE_UDEV_TYPE_DRIVE_MOUNTABLE:
          udev_enumerate_add_match_subsystem(en, "block");
          udev_enumerate_add_match_property(en, "ID_FS_USAGE", "filesystem");
          udev_enumerate_add_nomatch_sysattr(en, "capability", "52");
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
          udev_enumerate_add_match_sysattr(en, "removable", "1");
          break;
        case EEZE_UDEV_TYPE_DRIVE_CDROM:
          udev_enumerate_add_match_subsystem(en, "block");
          udev_enumerate_add_match_property(en, "ID_CDROM", "1");
          break;
        case EEZE_UDEV_TYPE_POWER_AC:
          udev_enumerate_add_match_subsystem(en, "power_supply");
          udev_enumerate_add_match_property(en, "POWER_SUPPLY_TYPE", "Mains");
          break;
        case EEZE_UDEV_TYPE_POWER_BAT:
          udev_enumerate_add_match_subsystem(en, "power_supply");
          udev_enumerate_add_match_property(en, "POWER_SUPPLY_TYPE", "Battery");
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

        if (etype == EEZE_UDEV_TYPE_IS_IT_HOT_OR_IS_IT_COLD_SENSOR)
          {/* ensure that temp input exists somewhere in this device chain */
             if (!_walk_parents_for_attr(device, "temp1_input", NULL))
               goto out;
             /* if device is not the one which has the temp input, we must go up the chain */
             if (!(test = udev_device_get_sysattr_value(device, "temp1_input")))
               {
                  devname = NULL;
                  for (parent = udev_device_get_parent(device); parent; parent = udev_device_get_parent(parent))/*check for parent*/
                    if (((test = udev_device_get_sysattr_value(parent, "temp1_input"))))
                      {
                         devname = udev_device_get_syspath(parent);
                         break;
                      }

                  if (!devname)
                    goto out;
               }

          }
        if (name)
             if (!strstr(devname, name))
               goto out;

        ret = eina_list_append(ret, eina_stringshare_add(devname));

out:
        udev_device_unref(device);
    }
    udev_enumerate_unref(en);
    udev_unref(udev);

    return ret;
}

/**
 * A more advanced find, allows finds using udev properties.
 *
 * @param subsystem The udev subsystem to filter by, or NULL
 * @param type "ID_INPUT_KEY", "ID_INPUT_MOUSE", "ID_INPUT_TOUCHPAD", NULL, etc
 * @param name A filter for the device name, or NULL
 * @return A Eina_List* of matched devices or NULL on failure
 * 
 * Return a list of syspaths (/sys/$syspath) for matching udev devices.
 * Requires at least one filter.
 *
 * @ingroup udev
 */
EAPI Eina_List *
eeze_udev_find_by_filter(const char *subsystem, const char *type, const char *name)
{
   struct udev *udev;
   struct udev_enumerate *en;
   struct udev_list_entry *devs, *cur;
   struct udev_device *device;
   const char *devname;
   Eina_List *ret = NULL;

   if ((!subsystem) && (!type) && (!name)) return NULL;

   udev = udev_new();
   if (!udev) return NULL;
   en = udev_enumerate_new(udev);
   if (!en) return NULL;

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
             if (!strstr(devname,name))
               goto out;

        ret = eina_list_append(ret, eina_stringshare_add(devname));

out:
        udev_device_unref(device);
    }
    udev_enumerate_unref(en);
    udev_unref(udev);

    return ret;
}

/**
 * Find a list of devices by a sysattr (and, optionally, a value of that sysattr).
 * 
 * @param sysattr The attribute to find
 * @param value Optional: the value that the attribute should have
 * 
 * @return A list of the devices found with the attribute
 */
EAPI Eina_List *
eeze_udev_find_by_sysattr(const char *sysattr, const char *value)
{
   struct udev *udev;
   struct udev_enumerate *en;
   struct udev_list_entry *devs, *cur;
   struct udev_device *device;
   const char *devname;
   Eina_List *ret = NULL;

   if (!sysattr) return NULL;

   udev = udev_new();
   if (!udev) return NULL;
   en = udev_enumerate_new(udev);
   if (!en) return NULL;

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
    udev_unref(udev);

    return ret;
}
