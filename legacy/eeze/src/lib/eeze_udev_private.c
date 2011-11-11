#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <Eeze.h>
#include "eeze_udev_private.h"

/*
 * helper function to set up a new device from a syspath
 * which may or may not include /sys at the beginning
 */
_udev_device *
_new_device(const char *syspath)
{
   _udev_device *device;

   device = udev_device_new_from_syspath(udev, syspath);
   if (!device)
     ERR("device %s does not exist!", syspath);
   return device;
}

/*
 * copies a device
 */
_udev_device *
_copy_device(_udev_device *device)
{
   return udev_device_ref(device);
}

/*
 * private function to simulate udevadm info -a
 * walks up the device tree checking each node for sysattr
 * with value $value
 */
Eina_Bool
_walk_parents_test_attr(_udev_device *device,
                        const char   *sysattr,
                        const char   *value)
{
   _udev_device *parent, *child = device;
   const char *test;

   if (udev_device_get_sysattr_value(device, sysattr))
     return EINA_TRUE;

   parent = udev_device_get_parent(child);

   for (; parent; child = parent, parent = udev_device_get_parent(child))
     {
        if (!(test = udev_device_get_sysattr_value(parent, sysattr)))
          continue;

        if (!value)
          return EINA_TRUE;
        else
        if (!strcmp(test, value))
          return EINA_TRUE;
     }

   return EINA_FALSE;
}

const char *
_walk_parents_get_attr(_udev_device *device,
                       const char   *sysattr,
                       Eina_Bool property)
{
   _udev_device *parent, *child = device;
   const char *test;

   if (property)
     test = udev_device_get_property_value(device, sysattr);
   else
     test = udev_device_get_sysattr_value(device, sysattr);
   if (test) return eina_stringshare_add(test);

   parent = udev_device_get_parent(child);

   for (; parent; child = parent, parent = udev_device_get_parent(child))
     {
        if (property)
          test = udev_device_get_property_value(parent, sysattr);
        else
          test = udev_device_get_sysattr_value(parent, sysattr);
        if (test) return eina_stringshare_add(test);
     }

   return NULL;
}

const char *
_walk_children_get_attr(const char *syspath,
                        const char *sysattr,
                        const char *subsystem,
                        Eina_Bool property)
{
   char buf[PATH_MAX];
   const char *path, *ret = NULL;
   _udev_enumerate *en;
   _udev_list_entry *devs, *cur;

   en = udev_enumerate_new(udev);
   EINA_SAFETY_ON_NULL_RETURN_VAL(en, NULL);
   path = strrchr(syspath, '/');
   if (path) path++;
   else path = syspath;
   snprintf(buf, sizeof(buf), "%s*", path);
   udev_enumerate_add_match_sysname(en, buf);
   if (subsystem) udev_enumerate_add_match_subsystem(en, subsystem);
   udev_enumerate_scan_devices(en);
   devs = udev_enumerate_get_list_entry(en);
   udev_list_entry_foreach(cur, devs)
     {
        const char *devname, *test;
        _udev_device *device;

        devname = udev_list_entry_get_name(cur);
        device = _new_device(devname);
        if (property)
          test = udev_device_get_property_value(device, sysattr);
        else
          test = udev_device_get_sysattr_value(device, sysattr);
        if (test)
          {
             ret = eina_stringshare_add(test);
             udev_device_unref(device);
             break;
          }
        udev_device_unref(device);
     }
   udev_enumerate_unref(en);
   return ret;
}

/*
 * check a list for all parents of a device,
 * stringshare adding all devices that are not in the list
 */
Eina_List *
_get_unlisted_parents(Eina_List    *list,
                      _udev_device *device)
{
   _udev_device *parent, *child = device;
   const char *test, *devname, *vendor, *vendor2, *model, *model2;
   Eina_List *l;
   Eina_Bool found;

   if (!(vendor = udev_device_get_property_value(child, "ID_VENDOR_ID")))
     vendor = udev_device_get_property_value(child, "ID_VENDOR");
   if (!vendor) vendor = udev_device_get_sysattr_value(child, "vendor");
   if (!vendor) vendor = udev_device_get_sysattr_value(child, "manufacturer");

   if (!(model = udev_device_get_property_value(child, "ID_MODEL_ID")))
     model = udev_device_get_property_value(child, "ID_MODEL");
   if (!model) model = udev_device_get_sysattr_value(child, "model");
   if (!model) model = udev_device_get_sysattr_value(child, "product");

   parent = udev_device_get_parent(child);

   for (; parent; child = parent, parent = udev_device_get_parent(child))
     {
        found = EINA_FALSE;

        if (!(vendor2 = udev_device_get_property_value(child, "ID_VENDOR_ID")))
          vendor2 = udev_device_get_property_value(child, "ID_VENDOR");
        if (!vendor2) vendor2 = udev_device_get_sysattr_value(child, "vendor");
        if (!vendor2) vendor2 = udev_device_get_sysattr_value(child, "manufacturer");

        if (!(model2 = udev_device_get_property_value(child, "ID_MODEL_ID")))
          model2 = udev_device_get_property_value(child, "ID_MODEL");
        if (!model2) model2 = udev_device_get_sysattr_value(child, "model");
        if (!model2) model2 = udev_device_get_sysattr_value(child, "product");

        if ((!model2 && model) || (model2 && !model) || (!vendor2 && vendor)
            || (vendor2 && !vendor))
          break;
        else
        if (((model && model2) && (strcmp(model, model2))) ||
            ((vendor && vendor2) && (strcmp(vendor, vendor2))))
          break;

        devname = udev_device_get_syspath(parent);
        EINA_LIST_FOREACH(list, l, test)
          {
             if (!strcmp(test, devname))
               {
                  found = EINA_TRUE;
                  break;
               }
          }

        if (!found)
          list = eina_list_prepend(list, eina_stringshare_add(devname));
     }

   return list;
}

