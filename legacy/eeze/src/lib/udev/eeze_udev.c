#include "eeze_udev_private.h"
#include <Eeze_Udev.h>

/* from watch.c */
Eina_Bool _walk_parents_for_attr(struct udev_device *device, const char *sysattr, const char* value);

/**
 * @defgroup udev udev
 *
 * These are functions which interact directly with udev.
 */

/**
 * Find the root device of a device from its syspath.
 *
 * @param syspath The syspath of a device, with or without "/sys/"
 * @return The syspath of the parent device
 * 
 * Return a syspath (/sys/$syspath) for the parent device.
 *
 * @ingroup udev
 */
EAPI const char *
eeze_udev_syspath_rootdev_get(const char *syspath)
{
   struct udev *udev;
   struct udev_device *device, *parent;
   const char *ret;

   if (!syspath) return NULL;

   udev = udev_new();
   if (!udev) return NULL;

   device = udev_device_new_from_syspath(udev, syspath);
   parent = udev_device_get_parent(device);
   ret = eina_stringshare_add(udev_device_get_property_value(parent, "DEVPATH"));

   udev_device_unref(device);
   udev_unref(udev);

   return ret;
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
   struct udev_device *device;
   const char *devname;
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

        if (name)
             if (!strstr(devname,name))
               goto out;

        ret = eina_list_append(ret, eina_stringshare_add(udev_device_get_property_value(device, "DEVPATH")));

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

        ret = eina_list_append(ret, eina_stringshare_add(udev_device_get_property_value(device, "DEVPATH")));

out:
        udev_device_unref(device);
    }
    udev_enumerate_unref(en);
    udev_unref(udev);

    return ret;
}


/**
 * Get the /dev/ path from the /sys/ path.
 *
 * @param syspath The /sys/ path with or without the /sys/
 * @return A const char* with the /dev/ path or NULL on failure
 * 
 * Takes /sys/$PATH and turns it into the corresponding "/dev/x/y".
 * 
 * @ingroup udev
 */
EAPI const char *
eeze_udev_syspath_get_devpath(const char *syspath)
{
      struct udev *udev;
      struct udev_device *device;
      const char *name;
      Eina_Strbuf *sbuf;

      if (!syspath) return NULL;
      udev = udev_new();
      if (!udev) return NULL;
      
      sbuf = eina_strbuf_new();
      if (!strstr(syspath, "/sys/"))
        eina_strbuf_append(sbuf, "/sys/");
      eina_strbuf_append(sbuf, syspath);

      device = udev_device_new_from_syspath(udev, eina_strbuf_string_get(sbuf));
      name = eina_stringshare_add(udev_device_get_property_value(device, "DEVNAME"));

      udev_device_unref(device);
      udev_unref(udev);
      eina_strbuf_free(sbuf);

      return name;
}

/**
 * Get the subsystem of a device from the /sys/ path.
 *
 * @param syspath The /sys/ path with or without the /sys/
 * @return A const char* with the subsystem of the device or NULL on failure
 * 
 * Takes /sys/$PATH and returns the corresponding device subsystem,
 * such as "input" for keyboards/mice.
 * 
 * @ingroup udev
 */
EAPI const char *
eeze_udev_syspath_get_subsystem(const char *syspath)
{
      struct udev *udev;
      struct udev_device *device;
      const char *subsystem;
      Eina_Strbuf *sbuf;

      if (!syspath) return NULL;
      udev = udev_new();
      if (!udev) return NULL;
      
      sbuf = eina_strbuf_new();
      if (!strstr(syspath, "/sys/"))
        eina_strbuf_append(sbuf, "/sys/");
      eina_strbuf_append(sbuf, syspath);

      device = udev_device_new_from_syspath(udev, eina_strbuf_string_get(sbuf));
      subsystem = eina_stringshare_add(udev_device_get_property_value(device, "SUBSYSTEM"));

      udev_device_unref(device);
      udev_unref(udev);
      eina_strbuf_free(sbuf);

      return subsystem;
}

/**
 * Get the property value of a device from the /sys/ path.
 *
 * @param syspath The /sys/ path with or without the /sys/
 * @param property The property to get; full list of these is a FIXME
 * @return A const char* with the subsystem of the device or NULL on failure
 * 
 * Takes /sys/$PATH and returns the corresponding device subsystem,
 * such as "input" for keyboards/mice.
 * 
 * @ingroup udev
 */
EAPI const char *
eeze_udev_syspath_get_property(const char *syspath, const char *property)
{
      struct udev *udev;
      struct udev_device *device;
      const char *value;
      Eina_Strbuf *sbuf;

      if (!syspath) return NULL;
      udev = udev_new();
      if (!udev) return NULL;

      sbuf = eina_strbuf_new();
      if (!strstr(syspath, "/sys/"))
        eina_strbuf_append(sbuf, "/sys/");
      eina_strbuf_append(sbuf, syspath);

      device = udev_device_new_from_syspath(udev, eina_strbuf_string_get(sbuf));
      value = eina_stringshare_add(udev_device_get_property_value(device, property));

      udev_device_unref(device);
      udev_unref(udev);
      eina_strbuf_free(sbuf);

      return value;
}

/**
 * Get the syspath of a device from the /dev/ path.
 *
 * @param devpath The /dev/ path of the device
 * @return A const char* which corresponds to the /sys/ path of the device or NULL on failure
 * 
 * Takes "/dev/path" and returns the corresponding /sys/ path (without the "/sys/")
 * 
 * @ingroup udev
 */
EAPI const char *
eeze_udev_devpath_get_syspath(const char *devpath)
{
   struct udev *udev;
   struct udev_enumerate *en;
   struct udev_list_entry *devs, *cur;
   struct udev_device *device;
   const char *name, *ret;

   if (!devpath) return NULL;
   udev = udev_new();
   if (!udev) return NULL;
   en = udev_enumerate_new(udev);
   if (!en) return NULL;

   udev_enumerate_add_match_property(en, "DEVNAME", devpath);
   udev_enumerate_scan_devices(en);
   devs = udev_enumerate_get_list_entry(en);

   udev_list_entry_foreach(cur, devs)
     {
        name = udev_list_entry_get_name(cur);
        device = udev_device_new_from_syspath(udev, name);
        ret = eina_stringshare_add(udev_device_get_sysname(device));
 
        udev_device_unref(device);
        break; /*just in case there's more than one somehow*/
     }
    udev_enumerate_unref(en);
    udev_unref(udev);

    return ret;
}

/**
 * Get the subsystem of a device from the /dev/ path.
 *
 * @param devpath The /dev/ path of the device
 * @return A const char* with the subsystem of the device or NULL on failure
 * 
 * Takes "/dev/path" and returns the subsystem of the device.
 * 
 * @ingroup udev
 */
EAPI const char *
eeze_udev_devpath_get_subsystem(const char *devpath)
{
   struct udev *udev;
   struct udev_enumerate *en;
   struct udev_list_entry *devs, *cur;
   struct udev_device *device;
   const char *name, *ret;

   if (!devpath) return NULL;
   udev = udev_new();
   if (!udev) return NULL;
   en = udev_enumerate_new(udev);
   if (!en) return NULL;

   udev_enumerate_add_match_property(en, "DEVNAME", devpath);
   udev_enumerate_scan_devices(en);
   devs = udev_enumerate_get_list_entry(en);

   udev_list_entry_foreach(cur, devs)
     {
        name = udev_list_entry_get_name(cur);
        device = udev_device_new_from_syspath(udev, name);
        ret = eina_stringshare_add(udev_device_get_subsystem(device));
 
        udev_device_unref(device);
        break; /*just in case there's more than one somehow*/
     }
    udev_enumerate_unref(en);
    udev_unref(udev);

    return ret;
}

/**
 * Checks whether the device is a mouse.
 *
 * @param syspath The /sys/ path of the device
 * @return If true, the device is a mouse
 * 
 * @ingroup udev
 */
EAPI Eina_Bool
eeze_udev_syspath_is_mouse(const char *syspath)
{
      struct udev *udev;
      struct udev_device *device;
      Eina_Bool mouse = 0;
      Eina_Strbuf *sbuf;
      const char *test = NULL;

      if (!syspath) return 0;
      udev = udev_new();
      if (!udev) return 0;
      
      sbuf = eina_strbuf_new();
      if (!strstr(syspath, "/sys/"))
        eina_strbuf_append(sbuf, "/sys/");
      eina_strbuf_append(sbuf, syspath);

      device = udev_device_new_from_syspath(udev, eina_strbuf_string_get(sbuf));
#ifdef OLD_UDEV_RRRRRRRRRRRRRR
      mouse = _walk_parents_for_attr(device, "bInterfaceProtocol", "02");
      if (!mouse)
      {
         test = udev_device_get_property_value(device, "ID_CLASS");
         if ((test) && (!strcmp(test, "mouse")))
           mouse = 1;
      }
#else
      test = udev_device_get_property_value(device, "ID_INPUT_MOUSE");
      if (test) mouse = atoi(test);
#endif

      udev_device_unref(device);
      udev_unref(udev);

      return mouse;
}

/**
 * Checks whether the device is a keyboard.
 *
 * @param syspath The /sys/ path of the device
 * @return If true, the device is a keyboard
 * 
 * @ingroup udev
 */
EAPI Eina_Bool
eeze_udev_syspath_is_kbd(const char *syspath)
{
      struct udev *udev;
      struct udev_device *device;
      Eina_Bool kbd = 0;
      Eina_Strbuf *sbuf;
      const char *test = NULL;

      if (!syspath) return 0;
      udev = udev_new();
      if (!udev) return 0;
      
      sbuf = eina_strbuf_new();
      if (!strstr(syspath, "/sys/"))
        eina_strbuf_append(sbuf, "/sys/");
      eina_strbuf_append(sbuf, syspath);

      device = udev_device_new_from_syspath(udev, eina_strbuf_string_get(sbuf));
#ifdef OLD_UDEV_RRRRRRRRRRRRRR
      kbd = _walk_parents_for_attr(device, "bInterfaceProtocol", "01");
      if (!kbd)
      {
         test = udev_device_get_property_value(device, "ID_CLASS");
         if ((test) && (!strcmp(test, "kbd")))
           kbd = 1;
      }
#else      
      test = udev_device_get_property_value(device, "ID_INPUT_KEYBOARD");
      if (test) kbd = atoi(test);
#endif

      udev_device_unref(device);
      udev_unref(udev);

      return kbd;
}

/**
 * Checks whether the device is a touchpad.
 *
 * @param syspath The /sys/ path of the device
 * @return If true, the device is a touchpad
 * 
 * @ingroup udev
 */
EAPI Eina_Bool
eeze_udev_syspath_is_touchpad(const char *syspath)
{
      struct udev *udev;
      struct udev_device *device;
      Eina_Bool touchpad = 0;
      Eina_Strbuf *sbuf;
      const char *test = NULL;

      if (!syspath) return 0;
      udev = udev_new();
      if (!udev) return 0;
      
      sbuf = eina_strbuf_new();
      if (!strstr(syspath, "/sys/"))
        eina_strbuf_append(sbuf, "/sys/");
      eina_strbuf_append(sbuf, syspath);

      device = udev_device_new_from_syspath(udev, eina_strbuf_string_get(sbuf));
#ifdef OLD_UDEV_RRRRRRRRRRRRRR
      touchpad = _walk_parents_for_attr(device, "resolution", NULL);
#else
      test = udev_device_get_property_value(device, "ID_INPUT_TOUCHPAD");
      if (test) touchpad = atoi(test);
#endif
      udev_device_unref(device);
      udev_unref(udev);

      return touchpad;
}

