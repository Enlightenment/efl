#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <Eeze_Udev.h>
#include "eeze_udev_private.h"

/**
 * private function to simulate udevadm info -a
 * walks up the device tree checking each node for sysattr
 * with value $value
 */
Eina_Bool
_walk_parents_test_attr(struct udev_device *device, const char *sysattr, const char* value)
{
   struct udev_device *parent, *child = device;
   const char *test;

   if ((test = udev_device_get_sysattr_value(device, sysattr)))
     return 1;
   parent = udev_device_get_parent(child);
   for (; parent; child = parent, parent = udev_device_get_parent(child))
     {
        if (!(test = udev_device_get_sysattr_value(parent, sysattr)))
          continue;
        if (!value)
          return 1;
        else if (!strcmp(test, value))
          return 1;
     }

   return 0;
}

/**
 * check a list for all parents of a device,
 * stringshare adding all devices that are not in the list
 */
void
_get_unlisted_parents(Eina_List *list, struct udev_device *device)
{
    struct udev_device *parent, *child = device;
    const char *test, *devname, *vendor, *vendor2, *model, *model2;
    Eina_List *l;
    Eina_Bool found;
   
   if (!(vendor = udev_device_get_property_value(child, "ID_VENDOR_ID")))
     vendor = udev_device_get_property_value(child, "ID_VENDOR");
   if (!(model = udev_device_get_property_value(child, "ID_MODEL_ID")))
     model = udev_device_get_property_value(child, "ID_MODEL");
   parent = udev_device_get_parent(child);
   for (; parent; child = parent, parent = udev_device_get_parent(child))
     {
        found = 0;
        if (!(vendor2 = udev_device_get_property_value(child, "ID_VENDOR_ID")))
          vendor2 = udev_device_get_property_value(child, "ID_VENDOR");
        if (!(model2 = udev_device_get_property_value(child, "ID_MODEL_ID")))
          model2 = udev_device_get_property_value(child, "ID_MODEL");
        if ((!model2 && model) || (model2 && !model) || (!vendor2 && vendor) || (vendor2 && !vendor))
            break;
        else if (((model && model2) && (strcmp(model, model2))) ||
            ((vendor && vendor2) && (strcmp(vendor, vendor2))))
          break;
        devname = udev_device_get_syspath(parent);
        EINA_LIST_FOREACH(list, l, test)
          {
             if (!strcmp(test, devname))
               {
                  found = 1;
                  break;
               }
          }
        if (!found)
          list = eina_list_prepend(list, eina_stringshare_add(devname));
     }
   return;
}
