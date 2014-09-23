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
   int event = 0;

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
             if ((store->event != EEZE_UDEV_EVENT_NONE) &&
                 ((store->event & EEZE_UDEV_EVENT_ADD) != EEZE_UDEV_EVENT_ADD))
               goto error;

             event |= EEZE_UDEV_EVENT_ADD;
          }
        else if (!strcmp(test, "remove"))
          {
             if ((store->event != EEZE_UDEV_EVENT_NONE) &&
                 ((store->event & EEZE_UDEV_EVENT_REMOVE) != EEZE_UDEV_EVENT_REMOVE))
               goto error;

             event |= EEZE_UDEV_EVENT_REMOVE;
          }
        else if (!strcmp(test, "change"))
          {
             if ((store->event != EEZE_UDEV_EVENT_NONE) &&
                 ((store->event & EEZE_UDEV_EVENT_CHANGE) != EEZE_UDEV_EVENT_CHANGE))
               goto error;

             event |= EEZE_UDEV_EVENT_CHANGE;
          }
        else if (!strcmp(test, "online"))
          {
             if ((store->event != EEZE_UDEV_EVENT_NONE) &&
                 ((store->event & EEZE_UDEV_EVENT_ONLINE) != EEZE_UDEV_EVENT_ONLINE))
               goto error;

             event |= EEZE_UDEV_EVENT_ONLINE;
          }
        else
          {
             if ((store->event != EEZE_UDEV_EVENT_NONE) &&
                 ((store->event & EEZE_UDEV_EVENT_OFFLINE) != EEZE_UDEV_EVENT_OFFLINE))
               goto error;

             event |= EEZE_UDEV_EVENT_OFFLINE;
          }
     }

   if ((event & EEZE_UDEV_EVENT_OFFLINE) || (event & EEZE_UDEV_EVENT_REMOVE))
     goto out;
   switch (store->type)
     {
      case EEZE_UDEV_TYPE_KEYBOARD:
        if ((!udev_device_get_property_value(device, "ID_INPUT_KEYBOARD")) &&
            (!udev_device_get_property_value(device, "ID_INPUT_KEY")))
          goto error;

        break;

      case EEZE_UDEV_TYPE_MOUSE:
        if (!udev_device_get_property_value(device, "ID_INPUT_MOUSE"))
          goto error;

        break;

      case EEZE_UDEV_TYPE_TOUCHPAD:
        if (!udev_device_get_property_value(device, "ID_INPUT_TOUCHPAD"))
          goto error;

        break;

      case EEZE_UDEV_TYPE_JOYSTICK:
        if (!udev_device_get_property_value(device, "ID_INPUT_JOYSTICK"))
          goto error;

        break;

      case EEZE_UDEV_TYPE_DRIVE_MOUNTABLE:
        if (!(test = (udev_device_get_property_value(device, "ID_FS_USAGE"))) ||
            (strcmp("filesystem", test)))
        {
           if (event & EEZE_UDEV_EVENT_CHANGE)
             {
                test = udev_device_get_sysname(device);
                if (!test) goto error;
                if (!strncmp(test, "loop", 4)) break;
             }
           goto error;
        }
        {
           int devcheck;

           devcheck = open(udev_device_get_devnode(device), O_RDONLY);
           if (devcheck < 0) goto error;
           close(devcheck);
        }

        break;

      case EEZE_UDEV_TYPE_DRIVE_INTERNAL:
        if (udev_device_get_property_value(device, "ID_FS_USAGE")) goto error;
        test = udev_device_get_sysattr_value(device, "removable");
        if (test && test[0] == '1') goto error;
        test = udev_device_get_property_value(device, "ID_BUS");
        if ((!test) || strcmp(test, "ata")) goto error;
        test = udev_device_get_property_value(device, "ID_TYPE");
        if (!(event & EEZE_UDEV_EVENT_CHANGE) && ((!test) || strcmp(test, "disk"))) goto error;
        break;

      case EEZE_UDEV_TYPE_DRIVE_REMOVABLE:
        if (udev_device_get_sysattr_value(device, "partition")) goto error;
        test = udev_device_get_sysattr_value(device, "removable");
        if ((!test) || (test[0] == '0')) goto error;
        test = udev_device_get_property_value(device, "ID_TYPE");
        if ((!test) || strcmp(test, "disk")) goto error;

        break;

      case EEZE_UDEV_TYPE_DRIVE_CDROM:
        if (!udev_device_get_property_value(device, "ID_CDROM"))
          goto error;

        break;

      case EEZE_UDEV_TYPE_POWER_AC:
        test = udev_device_get_property_value(device, "POWER_SUPPLY_ONLINE");
        if (!test) goto error;
        break;

      case EEZE_UDEV_TYPE_POWER_BAT:
        test = udev_device_get_property_value(device, "POWER_SUPPLY_PRESENT");
        if ((!test) || (strcmp(test, "1"))) goto error;
        break;

      case EEZE_UDEV_TYPE_NET:
        break;

      case EEZE_UDEV_TYPE_IS_IT_HOT_OR_IS_IT_COLD_SENSOR:
      {
        Eina_Bool one, two;
        const char *t;

        one = _walk_parents_test_attr(device, "temp1_input", NULL);
        two = _walk_parents_test_attr(device, "temp2_input", NULL);
        if ((!one) && (!two)) goto error;

        t = one ? "temp1_input" : "temp2_input";
        /* if device is not the one which has the temp input, we must go up the chain */
        if (!udev_device_get_sysattr_value(device, t))
          {
             for (parent = udev_device_get_parent(device); parent; parent = udev_device_get_parent(parent)) /*check for parent */
               if (udev_device_get_sysattr_value(parent, t))
                 {
                    tmpdev = device;

                    if (!(device = _copy_device(parent)))
                      goto error;

                    udev_device_unref(tmpdev);
                    break;
                 }
          }

        break;
      }
      case EEZE_UDEV_TYPE_V4L:
        if ((!(test = udev_device_get_subsystem(device)))
            || (strcmp(test, "video4linux")))
          goto error;
        break;

      case EEZE_UDEV_TYPE_BLUETOOTH:
        if ((!(test = udev_device_get_subsystem(device)))
            || (strcmp(test, "bluetooth")))
          goto error;
        break;

      case EEZE_UDEV_TYPE_DRM:
        if ((!(test = udev_device_get_subsystem(device)))
            || (strcmp(test, "drm")))
          goto error;
        break;

      case EEZE_UDEV_TYPE_BACKLIGHT:
        if ((!(test = udev_device_get_subsystem(device)))
            || (strcmp(test, "backlight")))
          goto error;
        break;

      case EEZE_UDEV_TYPE_LEDS:
        if ((!(test = udev_device_get_subsystem(device)))
            || (strcmp(test, "leds")))
          goto error;
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

   switch (type)
     {
      case EEZE_UDEV_TYPE_JOYSTICK:
      case EEZE_UDEV_TYPE_KEYBOARD:
      case EEZE_UDEV_TYPE_MOUSE:
      case EEZE_UDEV_TYPE_TOUCHPAD:
        udev_monitor_filter_add_match_subsystem_devtype(mon, "input", NULL);
        break;

      case EEZE_UDEV_TYPE_DRIVE_MOUNTABLE:
      case EEZE_UDEV_TYPE_DRIVE_INTERNAL:
        udev_monitor_filter_add_match_subsystem_devtype(mon, "block", NULL);
        break;

      case EEZE_UDEV_TYPE_DRIVE_REMOVABLE:
      case EEZE_UDEV_TYPE_DRIVE_CDROM:
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

      case EEZE_UDEV_TYPE_V4L:
        udev_monitor_filter_add_match_subsystem_devtype(mon, "video4linux",
                                                        NULL);
        break;

      case EEZE_UDEV_TYPE_BLUETOOTH:
        udev_monitor_filter_add_match_subsystem_devtype(mon, "bluetooth",
                                                        NULL);
        break;

      case EEZE_UDEV_TYPE_DRM:
        udev_monitor_filter_add_match_subsystem_devtype(mon, "drm_minor",
                                                        NULL);
        break;

      default:
        break;
     }

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
