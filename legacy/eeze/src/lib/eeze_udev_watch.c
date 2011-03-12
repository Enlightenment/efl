#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <Ecore.h>
#include <Eeze.h>
#include "eeze_udev_private.h"

/* opaque */
struct Eeze_Udev_Watch
{
   _udev_monitor    *mon;
   Ecore_Fd_Handler *handler;
   Eeze_Udev_Type    type;
   void             *data;
};

/* private */
struct _store_data
{
   void             (*func)(const char *,
                            Eeze_Udev_Event,
                            void *,
                            Eeze_Udev_Watch *);
   void            *data;
   Eeze_Udev_Event  event;
   _udev_monitor   *mon;
   Eeze_Udev_Type   type;
   Eeze_Udev_Watch *watch;
};

/**
 * @addtogroup watch Watch
 *
 * @brief These are functions which monitor udev for events.
 *
 * Eeze watches are simple: you specify a type of device to watch (or all devices), some events (or all) to watch for, a callback,
 * and some data, and then udev watches those device types for events of the type you specified.  Your callback is called with a
 * syspath of the triggering device and the event that happened to the device, along with the data you associated with the watch and
 * the watch object itself in case you want to stop the watch easily in a callback.
 *
 * @ingroup udev
 *
 * @{
 */

/* private function to further filter watch results based on Eeze_Udev_Type
 * specified; helpful for new udev versions, but absolutely required for
 * old udev, which does not implement filtering in device monitors.
 */
static Eina_Bool
_get_syspath_from_watch(void             *data,
                        Ecore_Fd_Handler *fd_handler)
{
   struct _store_data *store = data;
   _udev_device *device = NULL, *parent, *tmpdev;
   const char *ret, *test;
   Eeze_Udev_Watch_Cb func = store->func;
   void *sdata = store->data;
   Eeze_Udev_Watch *watch = store->watch;
   int cap = 0, event = 0;

   if (!ecore_main_fd_handler_active_get(fd_handler, ECORE_FD_READ))
     return EINA_TRUE;

   device = udev_monitor_receive_device(store->mon);

   if (!device)
     return EINA_TRUE;

   if ((!(test = udev_device_get_action(device)))
       || (!(ret = udev_device_get_syspath(device))))
     goto error;

   if (store->event)
     {
        if (!strcmp(test, "add"))
          {
             if ((store->event & EEZE_UDEV_EVENT_ADD) != EEZE_UDEV_EVENT_ADD)
               goto error;

             event |= EEZE_UDEV_EVENT_ADD;
          }
        else if (!strcmp(test, "remove"))
          {
             if ((store->event & EEZE_UDEV_EVENT_REMOVE) !=
                 EEZE_UDEV_EVENT_REMOVE)
               goto error;

             event |= EEZE_UDEV_EVENT_REMOVE;
          }
        else if (!strcmp(test, "change"))
          {
             if ((store->event & EEZE_UDEV_EVENT_CHANGE) !=
                 EEZE_UDEV_EVENT_CHANGE)
               goto error;

             event |= EEZE_UDEV_EVENT_CHANGE;
          }
        else if (!strcmp(test, "online"))
          {
             if ((store->event & EEZE_UDEV_EVENT_ONLINE) !=
                 EEZE_UDEV_EVENT_ONLINE)
               goto error;

             event |= EEZE_UDEV_EVENT_ONLINE;
          }
        else
          {
             if ((store->event & EEZE_UDEV_EVENT_OFFLINE) !=
                 EEZE_UDEV_EVENT_OFFLINE)
               goto error;

             event |= EEZE_UDEV_EVENT_OFFLINE;
          }
     }

   if ((event & EEZE_UDEV_EVENT_OFFLINE) || (event & EEZE_UDEV_EVENT_REMOVE))
     goto out;
   switch (store->type)
     {
      case EEZE_UDEV_TYPE_KEYBOARD:
#ifdef OLD_UDEV_RRRRRRRRRRRRRR
        if ((!(test = udev_device_get_subsystem(device)))
            || (strcmp(test, "input")))
          goto error;

        test = udev_device_get_property_value(device, "ID_CLASS");

        if ((_walk_parents_test_attr(device, "bInterfaceProtocol", "01"))
            || ((test) && (!strcmp(test, "kbd"))))
          break;

        goto error;
#endif
        if ((!udev_device_get_property_value(device, "ID_INPUT_KEYBOARD")) &&
            (!udev_device_get_property_value(device, "ID_INPUT_KEY")))
          goto error;

        break;

      case EEZE_UDEV_TYPE_MOUSE:
#ifdef OLD_UDEV_RRRRRRRRRRRRRR
        if ((!(test = udev_device_get_subsystem(device)))
            || (strcmp(test, "input")))
          goto error;

        test = udev_device_get_property_value(device, "ID_CLASS");

        if ((_walk_parents_test_attr(device, "bInterfaceProtocol", "02"))
            || ((test) && (!strcmp(test, "mouse"))))
          break;

        goto error;
#endif

        if (!udev_device_get_property_value(device, "ID_INPUT_MOUSE"))
          goto error;

        break;

      case EEZE_UDEV_TYPE_TOUCHPAD:
#ifdef OLD_UDEV_RRRRRRRRRRRRRR
        if ((!(test = udev_device_get_subsystem(device)))
            || (strcmp(test, "input")))
          goto error;

        if (_walk_parents_test_attr(device, "resolution", NULL))
          break;

        goto error;
#endif
        if (!udev_device_get_property_value(device, "ID_INPUT_TOUCHPAD"))
          goto error;

        break;

      case EEZE_UDEV_TYPE_DRIVE_MOUNTABLE:
#ifdef OLD_UDEV_RRRRRRRRRRRRRR
        if ((!(test = udev_device_get_subsystem(device)))
            || (strcmp(test, "block")))
          goto error;
#endif
        test = udev_device_get_sysattr_value(device, "capability");

        if (test)
          cap = strtol(test, NULL, 10);

        if (!(test = (udev_device_get_property_value(device, "ID_FS_USAGE"))) ||
            (strcmp("filesystem", test)) || (cap == 50))
          goto error;
        {
           int devcheck;

           devcheck = open(udev_device_get_devnode(device), O_RDONLY | O_EXCL);
           if (devcheck < 0) goto error;
           close(devcheck);
        }

        break;

      case EEZE_UDEV_TYPE_DRIVE_INTERNAL:
#ifdef OLD_UDEV_RRRRRRRRRRRRRR
        if ((!(test = udev_device_get_subsystem(device)))
            || (strcmp(test, "block")))
          goto error;
#endif
        if (!(test = udev_device_get_property_value(device, "ID_BUS"))
            || (strcmp("ata", test))
            || !(test = udev_device_get_sysattr_value(device, "removable"))
            || (strtol(test, NULL, 10)))
          goto error;

        break;

      case EEZE_UDEV_TYPE_DRIVE_REMOVABLE:
#ifdef OLD_UDEV_RRRRRRRRRRRRRR
        if ((!(test = udev_device_get_subsystem(device)))
            || (strcmp(test, "block")))
          goto error;
#endif
        if ((!(test = udev_device_get_sysattr_value(device, "removable"))
             || (!strtol(test, NULL, 10)))
            && (!(test = udev_device_get_sysattr_value(device, "capability"))
                || (strtol(test, NULL, 10) != 10)))
          goto error;

        break;

      case EEZE_UDEV_TYPE_DRIVE_CDROM:
#ifdef OLD_UDEV_RRRRRRRRRRRRRR
        if ((!(test = udev_device_get_subsystem(device)))
            || (strcmp(test, "block")))
          goto error;
#endif
        if (!udev_device_get_property_value(device, "ID_CDROM"))
          goto error;

        break;

      case EEZE_UDEV_TYPE_POWER_AC:
#ifdef OLD_UDEV_RRRRRRRRRRRRRR
        if ((!(test = udev_device_get_subsystem(device)))
            || (strcmp(test, "power_supply")))
          goto error;
#endif
        if (!(test = udev_device_get_property_value(device, "POWER_SUPPLY_TYPE"))
            || (strcmp("Mains", test)))
          goto error;
        break;

      case EEZE_UDEV_TYPE_POWER_BAT:
#ifdef OLD_UDEV_RRRRRRRRRRRRRR
        if ((!(test = udev_device_get_subsystem(device)))
            || (strcmp(test, "power_supply")))
          goto error;
#endif
        if (!(test = udev_device_get_property_value(device, "POWER_SUPPLY_TYPE"))
            || (strcmp("Battery", test)))
          goto error;
        break;

      case EEZE_UDEV_TYPE_NET:
#ifdef OLD_UDEV_RRRRRRRRRRRRRR
        if ((!(test = udev_device_get_subsystem(device)))
            || (strcmp(test, "net")))
          goto error;
#endif
        break;

      case EEZE_UDEV_TYPE_IS_IT_HOT_OR_IS_IT_COLD_SENSOR:
#ifdef OLD_UDEV_RRRRRRRRRRRRRR
        if ((!(test = udev_device_get_subsystem(device)))
            || (strcmp(test, "hwmon")))
          goto error;
#endif /* have to do stuff up here since we need info from the parent */
        if (!_walk_parents_test_attr(device, "temp1_input", NULL))
          goto error;

        /* if device is not the one which has the temp input, we must go up the chain */
        if (!udev_device_get_sysattr_value(device, "temp1_input"))
          {
             for (parent = udev_device_get_parent(device); parent; parent = udev_device_get_parent(parent)) /*check for parent */
               if (udev_device_get_sysattr_value(parent, "temp1_input"))
                 {
                    tmpdev = device;

                    if (!(device = _copy_device(parent)))
                      goto error;

                    udev_device_unref(tmpdev);
                    break;
                 }
          }

        break;

      default:
        break;
     }
out:
   (*func)(eina_stringshare_add(ret), event, sdata, watch);
error:
   if (device)
     udev_device_unref(device);
   return EINA_TRUE;
}

/**
 * Add a watch for a device type
 *
 * @param type The #Eeze_Udev_Type to watch
 * @param event The events to watch; an OR list of #Eeze_Udev_Event (ie (#EEZE_UDEV_EVENT_ADD | #EEZE_UDEV_EVENT_REMOVE)), or 0 for all events
 * @param cb The function to call when the watch receives data of type #Eeze_Udev_Watch_Cb
 * @param user_data Data to pass to the callback function
 *
 * @return A watch struct for the watch type specified, or NULL on failure
 *
 * Eeze watches will monitor udev for changes of type(s) @p event to devices of type @p type.  When these changes occur, the stringshared
 * syspath of the device will be sent to function @p func, along with the bitmask of the event type which can be detected through
 * binary &.
 */
EAPI Eeze_Udev_Watch *
eeze_udev_watch_add(Eeze_Udev_Type     type,
                    int                event,
                    Eeze_Udev_Watch_Cb cb,
                    void              *user_data)
{
   _udev_monitor *mon = NULL;
   int fd;
   Ecore_Fd_Handler *handler;
   Eeze_Udev_Watch *watch = NULL;
   struct _store_data *store = NULL;

   if (!(store = calloc(1, sizeof(struct _store_data))))
     return NULL;

   if (!(watch = malloc(sizeof(Eeze_Udev_Watch))))
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
      case EEZE_UDEV_TYPE_POWER_BAT:
        udev_monitor_filter_add_match_subsystem_devtype(mon, "power_supply",
                                                        NULL);
        break;

      case EEZE_UDEV_TYPE_NET:
        udev_monitor_filter_add_match_subsystem_devtype(mon, "net", NULL);
        break;

      case EEZE_UDEV_TYPE_IS_IT_HOT_OR_IS_IT_COLD_SENSOR:
        udev_monitor_filter_add_match_subsystem_devtype(mon, "hwmon", NULL);
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
   store->func = cb;
   store->data = user_data;
   store->mon = mon;
   store->type = type;
   store->watch = watch;
   store->event = event;

   if (!(handler = ecore_main_fd_handler_add(fd, ECORE_FD_READ,
                                             _get_syspath_from_watch, store, NULL, NULL)))
     goto error;

   watch->mon = mon;
   watch->handler = handler;
   return watch;
error:
   if (store)
     free(store);
   if (watch)
     free(watch);
   if (mon)
     udev_monitor_unref(mon);
   ERR("Could not create watch!");
   return NULL;
}

/**
 * Deletes a watch.
 *
 * @param watch An Eeze_Udev_Watch object
 * @return The data originally associated with the watch, or NULL
 *
 * Deletes a watch, closing file descriptors and freeing related udev memory.
 */
EAPI void *
eeze_udev_watch_del(Eeze_Udev_Watch *watch)
{
   struct _store_data *sdata;
   void *ret = NULL;

   if ((!watch) || (!watch->mon) || (!watch->handler))
     return NULL;

   sdata = ecore_main_fd_handler_del(watch->handler);
   udev_monitor_unref(watch->mon);

   if (sdata)
     {
        ret = sdata->data;
        free(sdata);
     }

   free(watch);
   return ret;
}

/** @} */
