#include <E_Udev.h>
#include "e_udev_private.h"

/* opaque */
struct Eudev_Watch
{
   struct udev_monitor *mon;
   Ecore_Fd_Handler *handler;
   Eudev_Type type;
};

/* private */
struct _store_data
{
   void(*func)(const char *, void *, Eudev_Watch *);
   void *data;
   struct udev_monitor *mon;
   Eudev_Type type;
   Eudev_Watch *watch;
};

/* private function to further filter watch results based on Eudev_Type
 * specified
 */
static int
_get_syspath_from_watch(void *data, Ecore_Fd_Handler *fd_handler)
{
   struct _store_data *store = data;
   struct udev_device *device;
   const char *ret, *test;
   void(*func)(const char *, void *, Eudev_Watch *) = store->func;
   void *sdata = store->data;
   Eudev_Watch *watch = store->watch;
   int cap = 0;

   if (!ecore_main_fd_handler_active_get(fd_handler, ECORE_FD_READ))
     return 0;
   device = udev_monitor_receive_device(store->mon);
   if (!device) return 0;

			switch (store->type)
					{
						  case EUDEV_TYPE_KEYBOARD:
										if (!udev_device_get_property_value(device, "ID_INPUT_KEYBOARD"))
												goto error;
										break;
   			  case EUDEV_TYPE_MOUSE:
										if (!udev_device_get_property_value(device, "ID_INPUT_MOUSE"))
												goto error;
										break;
   			  case EUDEV_TYPE_TOUCHPAD:
										if (!udev_device_get_property_value(device, "ID_INPUT_TOUCHPAD"))
												goto error;
   			  		break;
   			  case EUDEV_TYPE_DRIVE_MOUNTABLE:
   			    test = udev_device_get_sysattr_value(device, "capability");
   			    if (test) cap = atoi(test);
										if (!(test = (udev_device_get_property_value(device, "ID_FS_USAGE"))) ||
												strcmp("filesystem", test) || cap == 52)
												goto error;
   			    break;
   			  case EUDEV_TYPE_DRIVE_INTERNAL:
										if (!(test = udev_device_get_property_value(device, "ID_BUS")) || strcmp("ata", test) ||
										    !(test = udev_device_get_sysattr_value(device, "removable")) || atoi(test))
												goto error;
   			  		break;
        case EUDEV_TYPE_DRIVE_REMOVABLE:
										if ((!(test = udev_device_get_sysattr_value(device, "removable")) || !atoi(test)) &&
														(!(test = udev_device_get_sysattr_value(device, "capability")) || atoi(test) != 10))
												goto error;
   			  		break;
   			  case EUDEV_TYPE_DRIVE_CDROM:
										if (!udev_device_get_property_value(device, "ID_CDROM"))
												goto error;
   			  		break;
   			  case EUDEV_TYPE_POWER_AC:
										if (!(test = (udev_device_get_property_value(device, "POWER_SUPPLY_TYPE"))) ||
													strcmp("Mains", test))
   			  		break;
   			  case EUDEV_TYPE_POWER_BAT:
										if (!(test = (udev_device_get_property_value(device, "POWER_SUPPLY_TYPE"))) ||
													strcmp("Battery", test))
   			  		break;
/*   			  		
   			  case EUDEV_TYPE_ANDROID:
										udev_monitor_filter_add_match_subsystem_devtype(mon, "input", "usb_interface");
   			  		break;
*/
        default:
          break;
     }

   ret = eina_stringshare_add(udev_device_get_syspath(device));
			if (!ret) return 0;

   udev_device_unref(device);

   (*func)(ret, sdata, watch);

   return 1;

error:
			udev_device_unref(device);
			return 0;
}
/**
 * Add a watch in a subsystem for a device type
 *
 * @param subsystem The subsystem type. See @ref Subsystem_Types
 * @param device_type The device type. See @ref Device_Types
 * @param func The function to call when the watch receives data
 * @param user_data Data to pass to the callback function
 *
 * @return A watch struct for the watch type specified, or NULL on failure
 * 
 * @ingroup udev
 */
EAPI Eudev_Watch *
e_udev_watch_add(Eudev_Type type, void(*func)(const char *, void *, Eudev_Watch *), void *user_data)
{
   struct udev *udev;
   struct udev_monitor *mon;
   int fd;
   Ecore_Fd_Handler *handler;
   Eudev_Watch *watch;
   struct _store_data *store;
   const char *subsystem, *device_type;

   if (!(store = malloc(sizeof(struct _store_data)))) return NULL;
   if (!(watch = malloc(sizeof(Eudev_Watch))))
					goto error;

   if (!(udev = udev_new()))
					goto error;
   if (!(mon = udev_monitor_new_from_netlink(udev, "udev")))
					goto error;
   switch (type)
     {
						  case EUDEV_TYPE_KEYBOARD:
										udev_monitor_filter_add_match_subsystem_devtype(mon, "input", "usb_interface");
						    break;
   			  case EUDEV_TYPE_MOUSE:
										udev_monitor_filter_add_match_subsystem_devtype(mon, "input", "usb_interface");
										break;
   			  case EUDEV_TYPE_TOUCHPAD:
										udev_monitor_filter_add_match_subsystem_devtype(mon, "input", "usb_interface");
   			  		break;
   			  case EUDEV_TYPE_DRIVE_MOUNTABLE:
   			    udev_monitor_filter_add_match_subsystem_devtype(mon, "block", "disk");
   			    break;
   			  case EUDEV_TYPE_DRIVE_INTERNAL:
										udev_monitor_filter_add_match_subsystem_devtype(mon, "block", "disk");
   			  		break;
        case EUDEV_TYPE_DRIVE_REMOVABLE:
										udev_monitor_filter_add_match_subsystem_devtype(mon, "block", "disk");
   			  		break;
   			  case EUDEV_TYPE_DRIVE_CDROM:
										udev_monitor_filter_add_match_subsystem_devtype(mon, "block", "disk");
   			  		break;
   			  case EUDEV_TYPE_POWER_AC:
										udev_monitor_filter_add_match_subsystem_devtype(mon, "power_supply", "*");
   			  		break;
   			  case EUDEV_TYPE_POWER_BAT:
										udev_monitor_filter_add_match_subsystem_devtype(mon, "power_supply", "*");
   			  		break;
/*   			  		
   			  case EUDEV_TYPE_ANDROID:
										udev_monitor_filter_add_match_subsystem_devtype(mon, "input", "usb_interface");
   			  		break;
*/
        default:
          break;
     }
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
 * @param watch An Eudev_Watch object
 * @return The data originally associated with the watch, or NULL
 *
 * Deletes a watch, closing file descriptors and freeing related udev memory.
 *
 * @ingroup udev
 */
EAPI void *
e_udev_watch_del(Eudev_Watch *watch)
{
   struct udev *udev;
   struct udev_monitor *mon = watch->mon;
   Ecore_Fd_Handler *handler = watch->handler;
			struct _store_data *sdata;
   void *ret;

   if (!watch || !mon || !handler) return NULL;

   udev = udev_monitor_get_udev(mon);
   udev_monitor_unref(mon);
   udev_unref(udev);
   sdata = ecore_main_fd_handler_del(handler);
   ret = sdata->data;
   free(sdata);
   free(watch);

   return ret;
}
