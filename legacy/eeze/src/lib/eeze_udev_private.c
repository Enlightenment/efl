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
   Eina_Strbuf *sbuf;
   _udev_device *device;

   sbuf = eina_strbuf_new();

   if (strncmp(syspath, "/sys/", 5))
     eina_strbuf_append(sbuf, "/sys/");

   eina_strbuf_append(sbuf, syspath);
   device = udev_device_new_from_syspath(udev, eina_strbuf_string_get(sbuf));
   if (!device)
     ERR("device %s does not exist!", syspath);
   eina_strbuf_free(sbuf);
   return device;
}

/*
 * copies a device
 */
_udev_device *
_copy_device(_udev_device *device)
{
   const char *syspath;

   if (!(syspath = udev_device_get_syspath(device)))
     return NULL;

   return udev_device_new_from_syspath(udev, syspath);
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

   return EINA_TRUE;
}

const char *
_walk_parents_get_attr(_udev_device *device,
                       const char   *sysattr)
{
   _udev_device *parent, *child = device;
   const char *test;

   if ((test = udev_device_get_sysattr_value(device, sysattr)))
     return eina_stringshare_add(test);

   parent = udev_device_get_parent(child);

   for (; parent; child = parent, parent = udev_device_get_parent(child))
     {
        if ((test = udev_device_get_sysattr_value(parent, sysattr)))
          return eina_stringshare_add(test);
     }

   return NULL;
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

