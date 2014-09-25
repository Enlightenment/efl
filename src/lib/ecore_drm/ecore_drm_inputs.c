#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ecore_drm_private.h"

typedef struct _Ecore_Drm_Device_Open_Data Ecore_Drm_Device_Open_Data;
struct _Ecore_Drm_Device_Open_Data
{
   Ecore_Drm_Seat *seat;
   const char *node;
};

/* local functions */
static int 
_device_flags_set(int fd)
{
   int ret, fl;
   /* char name[256] = "unknown"; */

   if (fd < 0)
     {
        ERR("Failed to take device");
        return -1;
     }

   if ((fl = fcntl(fd, F_GETFL)) < 0)
     {
        ERR("Failed to get file flags: %m");
        goto flag_err;
     }

   fl = (O_RDWR | O_NONBLOCK);

   if ((ret = fcntl(fd, F_SETFL, fl)) < 0)
     {
        ERR("Failed to set file flags: %m");
        goto flag_err;
     }

   if ((fl = fcntl(fd, F_GETFD)) < 0)
     {
        ERR("Failed to get file fd: %m");
        goto flag_err;
     }

   fl &= ~FD_CLOEXEC;

   if ((ret = fcntl(fd, F_SETFD, fl)) < 0)
     {
        ERR("Failed to set file fds: %m");
        goto flag_err;
     }

   /* if (ioctl(fd, EVIOCGNAME(sizeof(name)), name) < 0) */
   /*   { */
   /*      ERR("Could not get device name: %m"); */
   /*      goto flag_err; */
   /*   } */
   /* else */
   /*   { */
   /*      name[sizeof(name) - 1] = '\0'; */
   /*      DBG("%s Opened", name); */
   /*   } */

   return fd;

flag_err:
   close(fd);
   return -1;
}

static void 
_cb_device_opened(void *data, const Eldbus_Message *msg, Eldbus_Pending *pending EINA_UNUSED)
{
   Ecore_Drm_Device_Open_Data *d;
   Ecore_Drm_Evdev *edev;
   Eina_Bool b = EINA_FALSE;
   const char *errname, *errmsg;
   int fd = -1;

   if (eldbus_message_error_get(msg, &errname, &errmsg))
     {
        ERR("Eldbus Message Error: %s %s", errname, errmsg);
        return;
     }

   if (!(d = data)) return;

   /* DBG("Input Device Opened: %s", d->node); */

   /* DBUS_TYPE_UNIX_FD == 'h' */
   if (!eldbus_message_arguments_get(msg, "hb", &fd, &b))
     {
        ERR("\tCould not get UNIX_FD from eldbus message: %d %d", fd, b);
        goto cleanup;
     }

   if (!(fd = _device_flags_set(fd)))
     {
        ERR("\tCould not set fd flags");
        goto release;
     }

   if (!(edev = _ecore_drm_evdev_device_create(d->seat, d->node, fd)))
     {
        ERR("\tCould not create evdev device: %s", d->node);
        goto release;
     }

   d->seat->devices = eina_list_append(d->seat->devices, edev);

   goto cleanup;

release:
   _ecore_drm_dbus_device_close(d->node);
cleanup:
   eina_stringshare_del(d->node);
   free(d);
}

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
_device_add(Ecore_Drm_Input *input, const char *device)
{
   Ecore_Drm_Seat *seat;
   Ecore_Drm_Device_Open_Data *data;
   const char *devseat, *wlseat;

   DBG("Add Input Device: %s", device);

   if (!(devseat = eeze_udev_syspath_get_property(device, "ID_SEAT")))
     devseat = eina_stringshare_add("seat0");

   if (strcmp(devseat, input->seat)) goto seat_err;

   if (!(wlseat = eeze_udev_syspath_get_property(device, "WL_SEAT")))
     wlseat = eina_stringshare_add("seat0");

   if (!(seat = _seat_get(input, wlseat))) 
     {
        ERR("\tCould not get matching seat");
        goto seat_get_err;
     }

   if (!(data = calloc(1, sizeof(Ecore_Drm_Device_Open_Data))))
     goto seat_get_err;

   data->seat = seat;
   if (!(data->node = eeze_udev_syspath_get_devpath(device)))
     goto dev_err;

   _ecore_drm_dbus_device_open(data->node, _cb_device_opened, data);

   return EINA_TRUE;

dev_err:
   free(data);
seat_get_err:
   eina_stringshare_del(wlseat);
seat_err:
   eina_stringshare_del(devseat);
   return EINA_FALSE;
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

static void 
_cb_input_event(const char *device, Eeze_Udev_Event event, void *data, Eeze_Udev_Watch *watch EINA_UNUSED)
{
   Ecore_Drm_Input *input;

   if (!(input = data)) return;

   switch (event)
     {
      case EEZE_UDEV_EVENT_ADD:
        _device_add(input, device);
        break;
      case EEZE_UDEV_EVENT_REMOVE:
          {
             const char *node;

             node = eeze_udev_syspath_get_devpath(device);
             _device_remove(input, node);
             eina_stringshare_del(node);
          }
        break;
      default:
        break;
     }
}

static Eina_Bool 
_devices_add(Ecore_Drm_Input *input)
{
   Eina_List *devices;
   Eina_Bool found = EINA_FALSE;
   const char *device;

   /* NB: This really sucks !! We cannot 'OR' diferent device types 
    * together for eeze_udev_find_by_type
    * 
    * For now, just find by 'NONE" and we'll filter for input devices by 
    * running tests */
   devices = eeze_udev_find_by_type(EEZE_UDEV_TYPE_NONE, NULL);
   EINA_LIST_FREE(devices, device)
     {
        if ((eeze_udev_syspath_is_mouse(device)) || 
            (eeze_udev_syspath_is_kbd(device)) || 
            (eeze_udev_syspath_is_touchpad(device)) || 
            (eeze_udev_syspath_is_joystick(device)))
          {
             if (!_device_add(input, device))
               ERR("\tFailed to add device");
             else
               found = EINA_TRUE;
          }

        eina_stringshare_del(device);
     }

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
   if (!dev) return EINA_FALSE;

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
          {
             _ecore_drm_dbus_device_close(edev->path);
             _ecore_drm_evdev_device_destroy(edev);
          }
     }
}

EAPI Eina_Bool 
ecore_drm_inputs_enable(Ecore_Drm_Input *input)
{
   /* check for valid input */
   if (!input) return EINA_FALSE;

   if (!input->watch)
     {
        int events = 0;

        events = (EEZE_UDEV_EVENT_ADD | EEZE_UDEV_EVENT_REMOVE);
        /* NB: This really sucks !! We cannot 'OR' diferent device types 
         * together for eeze_udev_watch_add :(
         * 
         * For now, just put a 'watch' on mouse type as this (in effect) does 
         * what we need by internally by adding a subsystem match for 'input' */
        if (!(input->watch = 
              eeze_udev_watch_add(EEZE_UDEV_TYPE_MOUSE, events, 
                                  _cb_input_event, input)))
          {
             ERR("Could not create Eeze_Udev_Watch for input");
             return EINA_FALSE;
          }
     }

   /* try to add devices */
   if (!_devices_add(input))
     {
        ERR("Could not add input devices");
        eeze_udev_watch_del(input->watch);
        input->watch = NULL;
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

   if (input->watch) eeze_udev_watch_del(input->watch);
   input->watch = NULL;

   input->enabled = EINA_FALSE;
   input->suspended = EINA_TRUE;

   ecore_drm_inputs_destroy(input->dev);
}
