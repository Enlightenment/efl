#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <Eeze.h>
#include "eeze_udev_private.h"

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

EAPI const char *
eeze_udev_devpath_get_syspath(const char *devpath)
{
   _udev_enumerate *en;
   _udev_list_entry *devs, *cur;
   const char *ret = NULL;

   if (!devpath)
     return NULL;

   en = udev_enumerate_new(udev);

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
