#include "eeze_udev_private.h"
#include <Eeze_Udev.h>

/* opaque */
struct Eeze_Udev_Watch
{
   struct udev_monitor *mon;
   Ecore_Fd_Handler *handler;
   Eeze_Udev_Type type;
};

/* private */
struct _store_data
{
   void(*func)(const char *, const char *, void *, Eeze_Udev_Watch *);
   void *data;
   struct udev_monitor *mon;
   Eeze_Udev_Type type;
   Eeze_Udev_Watch *watch;
};


/* private function to simulate udevadm info -a
 * walks up the device tree checking each node for sysattr
 * with value value
 */
static Eina_Bool
_walk_parents_for_attr(struct udev_device *device, const char *sysattr, const char* value)
{
   struct udev_device *parent, *child = device;
   const char *test;

   while ((parent = udev_device_get_parent(child)))
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

/* private function to further filter watch results based on Eeze_Udev_Type
 * specified
 */
static int
_get_syspath_from_watch(void *data, Ecore_Fd_Handler *fd_handler)
{
   struct _store_data *store = data;
   struct udev_device *device;
   const char *ret, *test;
   void(*func)(const char *, const char *, void *, Eeze_Udev_Watch *) = store->func;
   void *sdata = store->data;
   Eeze_Udev_Watch *watch = store->watch;
   int cap = 0;

   if (!ecore_main_fd_handler_active_get(fd_handler, ECORE_FD_READ))
     return 1;
   device = udev_monitor_receive_device(store->mon);
   if (!device) return 1;

   switch (store->type)
     {
        case EEZE_UDEV_TYPE_KEYBOARD:
#ifdef OLD_UDEV_RRRRRRRRRRRRRR
          if ((!(test = udev_device_get_subsystem(device))) || (strcmp(test, "input")))
            goto error;
          if (_walk_parents_for_attr(device, "bInterfaceProtocol", "01"))
            break;
          goto error;
#endif
          if (!udev_device_get_property_value(device, "ID_INPUT_KEYBOARD"))
            goto error;
          break;
        case EEZE_UDEV_TYPE_MOUSE:
#ifdef OLD_UDEV_RRRRRRRRRRRRRR
          if ((!(test = udev_device_get_subsystem(device))) || (strcmp(test, "input")))
            goto error;
          if (_walk_parents_for_attr(device, "bInterfaceProtocol", "02"))
            break;
          goto error;
#endif
          if (!udev_device_get_property_value(device, "ID_INPUT_MOUSE"))
            goto error;
          break;
        case EEZE_UDEV_TYPE_TOUCHPAD:
#ifdef OLD_UDEV_RRRRRRRRRRRRRR
          if ((!(test = udev_device_get_subsystem(device))) || (strcmp(test, "input")))
            goto error;
          if (_walk_parents_for_attr(device, "resolution", NULL))
            break;
          goto error;
#endif
          if (!udev_device_get_property_value(device, "ID_INPUT_TOUCHPAD"))
            goto error;
          break;
        case EEZE_UDEV_TYPE_DRIVE_MOUNTABLE:
#ifdef OLD_UDEV_RRRRRRRRRRRRRR
          if ((!(test = udev_device_get_subsystem(device))) || (strcmp(test, "block")))
            goto error;
#endif
          test = udev_device_get_sysattr_value(device, "capability");
          if (test) cap = atoi(test);
          if (!(test = (udev_device_get_property_value(device, "ID_FS_USAGE"))) ||
            (strcmp("filesystem", test)) || (cap == 52))
            goto error;
          break;
        case EEZE_UDEV_TYPE_DRIVE_INTERNAL:
#ifdef OLD_UDEV_RRRRRRRRRRRRRR
          if ((!(test = udev_device_get_subsystem(device))) || (strcmp(test, "block")))
            goto error;
#endif
          if (!(test = udev_device_get_property_value(device, "ID_BUS")) || (strcmp("ata", test)) ||
              !(test = udev_device_get_sysattr_value(device, "removable")) || (atoi(test)))
            goto error;
          break;
        case EEZE_UDEV_TYPE_DRIVE_REMOVABLE:
#ifdef OLD_UDEV_RRRRRRRRRRRRRR
          if ((!(test = udev_device_get_subsystem(device))) || (strcmp(test, "block")))
            goto error;
#endif
          if ((!(test = udev_device_get_sysattr_value(device, "removable")) || (!atoi(test))) &&
              (!(test = udev_device_get_sysattr_value(device, "capability")) || (atoi(test) != 10)))
            goto error;
          break;
        case EEZE_UDEV_TYPE_DRIVE_CDROM:
#ifdef OLD_UDEV_RRRRRRRRRRRRRR
          if ((!(test = udev_device_get_subsystem(device))) || (strcmp(test, "block")))
            goto error;
#endif
          if (!udev_device_get_property_value(device, "ID_CDROM"))
            goto error;
          break;
        case EEZE_UDEV_TYPE_POWER_AC:
#ifdef OLD_UDEV_RRRRRRRRRRRRRR
          if ((!(test = udev_device_get_subsystem(device))) || (strcmp(test, "power_supply")))
            goto error;
#endif
          if (!(test = (udev_device_get_property_value(device, "POWER_SUPPLY_TYPE"))) ||
             (strcmp("Mains", test)))
          break;
        case EEZE_UDEV_TYPE_POWER_BAT:
#ifdef OLD_UDEV_RRRRRRRRRRRRRR
          if ((!(test = udev_device_get_subsystem(device))) || (strcmp(test, "power_supply")))
            goto error;
#endif
          if (!(test = (udev_device_get_property_value(device, "POWER_SUPPLY_TYPE"))) ||
             (strcmp("Battery", test)))
          break;
/*          
        case EEZE_UDEV_TYPE_ANDROID:
          udev_monitor_filter_add_match_subsystem_devtype(mon, "input", "usb_interface");
          break;
*/
        default:
          break;
     }
   test = eina_stringshare_add(udev_device_get_action(device));
   if (!test)
     {
        udev_device_unref(device);
        return 1;
     }
   ret = eina_stringshare_add(udev_device_get_syspath(device));
   if (!ret)
     {
        udev_device_unref(device);
        eina_stringshare_del(test);
        return 1;
     }

   udev_device_unref(device);

   (*func)(ret, test, sdata, watch);
   eina_stringshare_del(test);
   eina_stringshare_del(ret);

   return 1;

error:
   udev_device_unref(device);
   return 1;
}
/**
 * Add a watch in a subsystem for a device type
 *
 * @param subsystem The subsystem type. See @ref Subsystem_Types
 * @param device_type The device type. See @ref Device_Types
 * @param func The function to call when the watch receives data;
 * must take (const char *device, const char *event_type, void *data, Eeze_Udev_Watch *watch)
 * @param user_data Data to pass to the callback function
 *
 * @return A watch struct for the watch type specified, or NULL on failure
 * 
 * @ingroup udev
 */
EAPI Eeze_Udev_Watch *
eeze_udev_watch_add(Eeze_Udev_Type type, void(*func)(const char *, const char *, void *, Eeze_Udev_Watch *), void *user_data)
{
   struct udev *udev;
   struct udev_monitor *mon;
   int fd;
   Ecore_Fd_Handler *handler;
   Eeze_Udev_Watch *watch;
   struct _store_data *store;

   if (!(store = malloc(sizeof(struct _store_data)))) return NULL;
   if (!(watch = malloc(sizeof(Eeze_Udev_Watch))))
     goto error;

   if (!(udev = udev_new()))
     goto error;
   if (!(mon = udev_monitor_new_from_netlink(udev, "udev")))
     goto error;
#ifndef OLD_UDEV_RRRRRRRRRRRRRR
   switch (type)
     {
        case EEZE_UDEV_TYPE_KEYBOARD:
          udev_monitor_filter_add_match_subsystem_devtype(mon, "input", NULL);
          break;
        case EEZE_UDEV_TYPE_MOUSE:
          udev_monitor_filter_add_match_subsystem_devtype(mon, "input", NULL);
          break;
        case EEZE_UDEV_TYPE_TOUCHPAD:
          udev_monitor_filter_add_match_subsystem_devtype(mon, "input", NULL);
          break;
        case EEZE_UDEV_TYPE_DRIVE_MOUNTABLE:
          udev_monitor_filter_add_match_subsystem_devtype(mon, "block", NULL);
          break;
        case EEZE_UDEV_TYPE_DRIVE_INTERNAL:
          udev_monitor_filter_add_match_subsystem_devtype(mon, "block", NULL);
          break;
        case EEZE_UDEV_TYPE_DRIVE_REMOVABLE:
          udev_monitor_filter_add_match_subsystem_devtype(mon, "block", NULL);
          break;
        case EEZE_UDEV_TYPE_DRIVE_CDROM:
          udev_monitor_filter_add_match_subsystem_devtype(mon, "block", NULL);
          break;
        case EEZE_UDEV_TYPE_POWER_AC:
          udev_monitor_filter_add_match_subsystem_devtype(mon, "power_supply", NULL);
          break;
        case EEZE_UDEV_TYPE_POWER_BAT:
          udev_monitor_filter_add_match_subsystem_devtype(mon, "power_supply", NULL);
          break;
/*          
        case EEZE_UDEV_TYPE_ANDROID:
          udev_monitor_filter_add_match_subsystem_devtype(mon, "input", "usb_interface");
          break;
*/
        default:
          break;
     }
#endif
   if (udev_monitor_enable_receiving(mon))
     goto error;
   
   fd = udev_monitor_get_fd(mon);
   store->func = func;
   store->data = user_data;
   store->mon = mon;
   store->type = type;
   store->watch = watch;
   if (!(handler = ecore_main_fd_handler_add(fd, ECORE_FD_READ, _get_syspath_from_watch, store, NULL, NULL)))
     goto error;
   watch->mon = mon;
   watch->handler = handler;

   return watch;

error:
        free(store);
        free(watch);
        udev_monitor_unref(mon);
        udev_unref(udev);
        return NULL;
}

/**
 * Deletes a watch.
 *
 * @param watch An Eeze_Udev_Watch object
 * @return The data originally associated with the watch, or NULL
 *
 * Deletes a watch, closing file descriptors and freeing related udev memory.
 *
 * @ingroup udev
 */
EAPI void *
eeze_udev_watch_del(Eeze_Udev_Watch *watch)
{
   struct udev *udev;
   struct udev_monitor *mon = watch->mon;
   Ecore_Fd_Handler *handler = watch->handler;
   struct _store_data *sdata;
   void *ret;

   if ((!watch) || (!mon) || (!handler)) return NULL;

   udev = udev_monitor_get_udev(mon);
   udev_monitor_unref(mon);
   udev_unref(udev);
   sdata = ecore_main_fd_handler_del(handler);
   if (sdata)
     {
        ret = sdata->data;
        free(sdata);
     }
   free(watch);

   return ret;
}
