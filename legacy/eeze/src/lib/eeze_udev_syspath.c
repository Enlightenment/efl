#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <Eeze.h>
#include "eeze_udev_private.h"

/**
 * @addtogroup syspath Syspath
 *
 * These are functions which interact with the syspath (/sys/$PATH) of
 * a device.
 *
 * @ingroup udev
 *
 * @{
 */

/**
 * Find the root device of a device from its syspath.
 *
 * @param syspath The syspath of a device, with or without "/sys/"
 * @return The syspath of the parent device
 *
 * Return a stringshared syspath (/sys/$syspath) for the parent device.
 */
EAPI const char *
eeze_udev_syspath_get_parent(const char *syspath)
{
   _udev_device *device, *parent;
   const char *ret;

   if (!syspath)
     return NULL;

   if (!(device = _new_device(syspath)))
     return NULL;
   parent = udev_device_get_parent(device);
   ret = eina_stringshare_add(udev_device_get_syspath(parent));
   udev_device_unref(device);
   return ret;
}

/**
 * Returns a list of all parent device syspaths for @p syspath.
 *
 * @param syspath The device to find parents of
 * @return A stringshared list of the parent devices of @p syspath
 */
EAPI Eina_List *
eeze_udev_syspath_get_parents(const char *syspath)
{
   _udev_device *child, *parent, *device;
   const char *path;
   Eina_List *devlist = NULL;

   if (!syspath)
     return NULL;

   if (!(device = _new_device(syspath)))
     return NULL;

   if (!(parent = udev_device_get_parent(device)))
     return NULL;

   for (; parent; child = parent, parent = udev_device_get_parent(child))
     {
        path = udev_device_get_syspath(parent);
        devlist = eina_list_append(devlist, eina_stringshare_add(path));
     }

   udev_device_unref(device);
   return devlist;
}

/**
 * Get the /dev/ path from the /sys/ path.
 *
 * @param syspath The /sys/ path with or without the /sys/
 * @return A stringshared char* with the /dev/ path or NULL on failure
 *
 * Takes /sys/$PATH and turns it into the corresponding "/dev/x/y".
 */
EAPI const char *
eeze_udev_syspath_get_devpath(const char *syspath)
{
   _udev_device *device;
   const char *name = NULL;

   if (!syspath)
     return NULL;

   if (!(device = _new_device(syspath)))
     return NULL;

   if (!(name = udev_device_get_devnode(device)))
     return NULL;

   name = eina_stringshare_add(name);
   udev_device_unref(device);
   return name;
}

/**
 * Get the /dev/ name from the /sys/ path.
 *
 * @param syspath The /sys/ path with or without the /sys/
 * @return A stringshared char* of the device name without the /dev/ path, or NULL on failure
 *
 * Takes /sys/$PATH and turns it into the corresponding /dev/x/"y".
 */
EAPI const char *
eeze_udev_syspath_get_devname(const char *syspath)
{
   _udev_device *device;
   const char *name = NULL;

   if (!syspath)
     return NULL;

   if (!(device = _new_device(syspath)))
     return NULL;

   if (!(name = udev_device_get_sysname(device)))
     return NULL;

   name = eina_stringshare_add(name);
   udev_device_unref(device);
   return name;
}

/**
 * Get the subsystem of a device from the /sys/ path.
 *
 * @param syspath The /sys/ path with or without the /sys/
 * @return A stringshared char* with the subsystem of the device or NULL on failure
 *
 * Takes /sys/$PATH and returns the corresponding device subsystem,
 * such as "input" for keyboards/mice.
 */
EAPI const char *
eeze_udev_syspath_get_subsystem(const char *syspath)
{
   _udev_device *device;
   const char *subsystem;

   if (!syspath)
     return NULL;

   if (!(device = _new_device(syspath)))
     return NULL;
   subsystem = eina_stringshare_add(udev_device_get_property_value(device, "SUBSYSTEM"));
   udev_device_unref(device);
   return subsystem;
}

/**
 * Get the property value of a device from the /sys/ path.
 *
 * @param syspath The /sys/ path with or without the /sys/
 * @param property The property to get; full list of these is a FIXME
 * @return A stringshared char* with the property or NULL on failure
 */
EAPI const char *
eeze_udev_syspath_get_property(const char *syspath,
                               const char *property)
{
   _udev_device *device;
   const char *value = NULL, *test;

   if (!syspath || !property)
     return NULL;

   if (!(device = _new_device(syspath)))
     return NULL;
   if ((test = udev_device_get_property_value(device, property)))
     value = eina_stringshare_add(test);

   udev_device_unref(device);
   return value;
}

/**
 * Get the sysattr value of a device from the /sys/ path.
 *
 * @param syspath The /sys/ path with or without the /sys/
 * @param sysattr The sysattr to get; full list of these is a FIXME
 * @return A stringshared char* with the sysattr or NULL on failure
 */
EAPI const char *
eeze_udev_syspath_get_sysattr(const char *syspath,
                              const char *sysattr)
{
   _udev_device *device;
   const char *value = NULL, *test;

   if (!syspath || !sysattr)
     return NULL;

   if (!(device = _new_device(syspath)))
     return NULL;

   if ((test = udev_device_get_sysattr_value(device, sysattr)))
     value = eina_stringshare_add(test);

   udev_device_unref(device);
   return value;
}

/**
 * Checks whether the device is a mouse.
 *
 * @param syspath The /sys/ path with or without the /sys/
 * @return If true, the device is a mouse
 */
EAPI Eina_Bool
eeze_udev_syspath_is_mouse(const char *syspath)
{
   _udev_device *device = NULL;
   Eina_Bool mouse = EINA_FALSE;
   const char *test = NULL;

   if (!syspath)
     return EINA_FALSE;

   if (!(device = _new_device(syspath)))
     return EINA_FALSE;
#ifdef OLD_UDEV_RRRRRRRRRRRRRR
   mouse = _walk_parents_test_attr(device, "bInterfaceProtocol", "02");

   if (!mouse)
     {
        test = udev_device_get_property_value(device, "ID_CLASS");

        if ((test) && (!strcmp(test, "mouse")))
          mouse = EINA_TRUE;
     }

#else
   test = udev_device_get_property_value(device, "ID_INPUT_MOUSE");

   if (test && (test[0] == '1'))
     mouse = EINA_TRUE;

#endif
   udev_device_unref(device);
   return mouse;
}

/**
 * Checks whether the device is a keyboard.
 *
 * @param syspath The /sys/ path with or without the /sys/
 * @return If true, the device is a keyboard
 */
EAPI Eina_Bool
eeze_udev_syspath_is_kbd(const char *syspath)
{
   _udev_device *device = NULL;
   Eina_Bool kbd = EINA_FALSE;
   const char *test = NULL;

   if (!syspath)
     return EINA_FALSE;

   if (!(device = _new_device(syspath)))
     return EINA_FALSE;
#ifdef OLD_UDEV_RRRRRRRRRRRRRR
   kbd = _walk_parents_test_attr(device, "bInterfaceProtocol", "01");

   if (!kbd)
     {
        test = udev_device_get_property_value(device, "ID_CLASS");

        if ((test) && (!strcmp(test, "kbd")))
          kbd = EINA_TRUE;
     }

#else
   test = udev_device_get_property_value(device, "ID_INPUT_KEYBOARD");

   if (test && (test[0] == '1'))
     kbd = EINA_TRUE;

#endif
   udev_device_unref(device);
   return kbd;
}

/**
 * Checks whether the device is a touchpad.
 *
 * @param syspath The /sys/ path with or without the /sys/
 * @return If true, the device is a touchpad
 */
EAPI Eina_Bool
eeze_udev_syspath_is_touchpad(const char *syspath)
{
   _udev_device *device = NULL;
   Eina_Bool touchpad = EINA_FALSE;

   if (!syspath)
     return EINA_FALSE;

   if (!(device = _new_device(syspath)))
     return EINA_FALSE;
#ifdef OLD_UDEV_RRRRRRRRRRRRRR
   touchpad = _walk_parents_test_attr(device, "resolution", NULL);
#else
   const char *test;
   test = udev_device_get_property_value(device, "ID_INPUT_TOUCHPAD");

   if (test && (test[0] == '1'))
     touchpad = EINA_TRUE;

#endif
   udev_device_unref(device);
   return touchpad;
}

/**
 * Get the syspath of a device from the /dev/ path.
 *
 * @param devpath The /dev/ path of the device
 * @return A stringshared char* which corresponds to the /sys/ path of the device or NULL on failure
 *
 * Takes "/dev/path" and returns the corresponding /sys/ path (without the "/sys/")
 */
EAPI const char *
eeze_udev_devpath_get_syspath(const char *devpath)
{
   _udev_enumerate *en;
   _udev_list_entry *devs, *cur;
   const char *ret = NULL;

   if (!devpath)
     return NULL;

   en = udev_enumerate_new((udev));

   if (!en)
     return NULL;

   udev_enumerate_add_match_property(en, "DEVNAME", devpath);
   udev_enumerate_scan_devices(en);
   devs = udev_enumerate_get_list_entry(en);
   udev_list_entry_foreach(cur, devs)
     {
        ret = eina_stringshare_add(udev_list_entry_get_name(cur));
        break; /*just in case there's more than one somehow */
     }
   udev_enumerate_unref(en);
   return ret;
}

/** @} */
