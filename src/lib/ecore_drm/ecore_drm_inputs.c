#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ecore_drm_private.h"

/* local functions */
static Ecore_Drm_Seat *
_seat_get(Ecore_Drm_Input *input, const char *seat)
{
   Ecore_Drm_Seat *s;
   Eina_List *l;

   EINA_LIST_FOREACH(input->dev->seats, l, s)
     {
        if (!strcmp(s->name, seat)) return s;
     }

   if (!(s = calloc(1, sizeof(Ecore_Drm_Seat))))
     return NULL;

   s->input = input;
   s->name = eina_stringshare_add(seat);

   input->dev->seats = eina_list_append(input->dev->seats, s);

   return s;
}

static Eina_Bool 
_device_add(Ecore_Drm_Input *input, struct udev_device *device)
{
   Ecore_Drm_Evdev *edev;
   Ecore_Drm_Seat *seat;
   const char *dev_seat, *wl_seat;
   const char *node;
   char n[PATH_MAX];
   int fd = -1;

   if (!(dev_seat = udev_device_get_property_value(device, "ID_SEAT")))
     dev_seat = "seat0";

   if (strcmp(dev_seat, input->seat)) return EINA_FALSE;

   if (!(wl_seat = udev_device_get_property_value(device, "WL_SEAT")))
     wl_seat = "seat0";

   if (!(seat = _seat_get(input, wl_seat)))
     return EINA_FALSE;

   node = udev_device_get_devnode(device);
   strcpy(n, node);

   fd = _ecore_drm_dbus_device_open(n);
   if (fd < 0)
     {
        ERR("FAILED TO OPEN %s: %m", n);
        return EINA_FALSE;
     }

   /* DBG("Opened Restricted Input: %s %d", node, fd); */

   if (!(edev = _ecore_drm_evdev_device_create(seat, node, fd)))
     return EINA_FALSE;

   seat->devices = eina_list_append(seat->devices, edev);

   /* TODO: finish */

   return EINA_TRUE;
}

static void 
_device_remove(Ecore_Drm_Input *input, const char *device)
{
   Ecore_Drm_Seat *seat;
   Eina_List *l;

   if (!input) return;

   EINA_LIST_FOREACH(input->dev->seats, l, seat)
     {
        Ecore_Drm_Evdev *edev;
        Eina_List *ll;

        EINA_LIST_FOREACH(seat->devices, ll, edev)
          {
             if (!strcmp(edev->path, device))
               {
                  seat->devices = eina_list_remove(seat->devices, edev);
                  _ecore_drm_evdev_device_destroy(edev);
                  break;
               }
          }
     }
}

static Eina_Bool 
_cb_input_event(void *data, Ecore_Fd_Handler *hdlr EINA_UNUSED)
{
   Ecore_Drm_Input *input;
   struct udev_device *udevice;
   const char *act;

   DBG("Input Event");

   if (!(input = data)) return EINA_FALSE;

   if (!(udevice = udev_monitor_receive_device(input->monitor)))
     return EINA_TRUE;

   if (!(act = udev_device_get_action(udevice))) return EINA_TRUE;

   if (strncmp("event", udev_device_get_sysname(udevice), 5) != 0)
     goto err;

   if (!strcmp(act, "add"))
     {
        DBG("\tDevice Added");
        _device_add(input, udevice);
     }
   else if (!strcmp(act, "remove"))
     {
        const char *node;

        node = udev_device_get_devnode(udevice);

        DBG("\tDevice Removed: %s", node);
        _device_remove(input, node);
     }

   return EINA_TRUE;

err:
   if (udevice) udev_device_unref(udevice);
   return EINA_TRUE;
}

static Eina_Bool 
_devices_add(Ecore_Drm_Input *input)
{
   struct udev_enumerate *uenum;
   struct udev_list_entry *uentry;
   struct udev_device *udevice;
   const char *path, *name;
   Eina_Bool found = EINA_FALSE;

   uenum = udev_enumerate_new(udev);
   udev_enumerate_add_match_subsystem(uenum, "input");
   udev_enumerate_scan_devices(uenum);

   udev_list_entry_foreach(uentry, udev_enumerate_get_list_entry(uenum))
     {
        path = udev_list_entry_get_name(uentry);
        udevice = udev_device_new_from_syspath(udev, path);
        name = udev_device_get_sysname(udevice);

        if (strncmp("event", name, 5) != 0)
          {
             udev_device_unref(udevice);
             continue;
          }

        if (!_device_add(input, udevice))
          {
             udev_device_unref(udevice);
             continue;
          }

        found = EINA_TRUE;

        udev_device_unref(udevice);
     }

   udev_enumerate_unref(uenum);

   if (!found)
     {
        ERR("No Input Devices Found");
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

/* public functions */
EAPI Eina_Bool 
ecore_drm_inputs_create(Ecore_Drm_Device *dev)
{
   Ecore_Drm_Input *input;

   /* check for valid device */
   if ((!dev) || (!udev)) return EINA_FALSE;

   /* try to allocate space for input structure */
   if (!(input = calloc(1, sizeof(Ecore_Drm_Input))))
     return EINA_FALSE;

   /* FIXME: Hardcoded seat name */
   input->seat = eina_stringshare_add("seat0");
   input->dev = dev;

   /* try to enable this input */
   if (!ecore_drm_inputs_enable(input))
     {
        ERR("Could not enable input");
        if (input->seat) eina_stringshare_del(input->seat);
        free(input);
        return EINA_FALSE;
     }

   /* add this input to dev */
   dev->inputs = eina_list_append(dev->inputs, input);

   return EINA_TRUE;
}

EAPI void 
ecore_drm_inputs_destroy(Ecore_Drm_Device *dev)
{
   Ecore_Drm_Seat *seat;
   Eina_List *l;

   if (!dev) return;

   EINA_LIST_FOREACH(dev->seats, l, seat)
     {
        Ecore_Drm_Evdev *edev;

        EINA_LIST_FREE(seat->devices, edev)
          _ecore_drm_evdev_device_destroy(edev);
     }
}

EAPI Eina_Bool 
ecore_drm_inputs_enable(Ecore_Drm_Input *input)
{
   /* check for valid input */
   if (!input) return EINA_FALSE;

   if (!input->monitor)
     input->monitor = udev_monitor_new_from_netlink(udev, "udev");

   if (!input->monitor)
     {
        ERR("Could not create udev monitor: %m");
        return EINA_FALSE;
     }

   /* setup input filter */
   udev_monitor_filter_add_match_subsystem_devtype(input->monitor, 
                                                   "input", NULL);

   /* try to enable receiving udev events */
   if (udev_monitor_enable_receiving(input->monitor))
     {
        ERR("Could not bind udev monitor: %m");
        udev_monitor_unref(input->monitor);
        return EINA_FALSE;
     }

   /* save the fd */
   if ((input->fd = udev_monitor_get_fd(input->monitor)) < 0)
     {
        ERR("Input monitor has no fd: %m");
        udev_monitor_unref(input->monitor);
        return EINA_FALSE;
     }

   /* create fd handler */
   if (!input->hdlr)
     {
        input->hdlr = 
          ecore_main_fd_handler_add(input->fd, ECORE_FD_READ, 
                                    _cb_input_event, input, NULL, NULL);
     }

   if (!input->hdlr)
     {
        ERR("Failed to setup input fd handler: %m");
        udev_monitor_unref(input->monitor);
        return EINA_FALSE;
     }

   /* try to add devices */
   if (!_devices_add(input))
     {
        ERR("Could not add input devices");
        udev_monitor_unref(input->monitor);
        return EINA_FALSE;
     }

   input->enabled = EINA_TRUE;
   input->suspended = EINA_FALSE;

   return EINA_TRUE;
}

EAPI void 
ecore_drm_inputs_disable(Ecore_Drm_Input *input)
{
   if (!input) return;
}
