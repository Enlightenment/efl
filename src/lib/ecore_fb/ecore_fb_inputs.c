#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ecore_fb_private.h"

EAPI int ECORE_FB_EVENT_SEAT_ADD = -1;

/* local functions */

int
_ecore_fb_launcher_device_open_no_pending(const char *device, int flags)
{
   int fd = -1;
   struct stat s;

   fd = open(device, flags | O_CLOEXEC);
   if (fd < 0) return fd;
   if (fstat(fd, &s) == -1)
     {
        close(fd);
        return -1;
     }

   return fd;
}

void
_ecore_fb_launcher_device_close(const char *device EINA_UNUSED, int fd)
{
   close(fd);
}

static int
_cb_open_restricted(const char *path, int flags, void *data)
{
   Ecore_Fb_Input *input;
   Ecore_Fb_Seat *seat;
   Ecore_Fb_Evdev *edev;
   Eina_List *l, *ll;
   int fd = -1;

   if (!(input = data)) return -1;

   /* try to open the device */
   fd = _ecore_fb_launcher_device_open_no_pending(path, flags);
   if (fd < 0) ERR("Could not open device");

   EINA_LIST_FOREACH(input->dev->seats, l, seat)
     {
        EINA_LIST_FOREACH(seat->devices, ll, edev)
          {
             if (strstr(path, edev->path))
               {
                  edev->fd = fd;
                  return fd;
               }
          }
     }

   return fd;
}

static void
_cb_close_restricted(int fd, void *data)
{
   Ecore_Fb_Input *input;
   Ecore_Fb_Seat *seat;
   Ecore_Fb_Evdev *edev;
   Eina_List *l, *ll;

   if (!(input = data)) return;

   EINA_LIST_FOREACH(input->dev->seats, l, seat)
     {
        EINA_LIST_FOREACH(seat->devices, ll, edev)
          {
             if (edev->fd == fd)
               {
                  _ecore_fb_launcher_device_close(edev->path, fd);
                  return;
               }
          }
     }
}

static Ecore_Fb_Seat *
_seat_create(Ecore_Fb_Input *input, const char *seat)
{
   Ecore_Fb_Seat *s;

   /* try to allocate space for new seat */
   if (!(s = calloc(1, sizeof(Ecore_Fb_Seat))))
     return NULL;

   s->input = input;
   s->name = eina_stringshare_add(seat);

   /* add this new seat to list */
   input->dev->seats = eina_list_append(input->dev->seats, s);

   ecore_event_add(ECORE_FB_EVENT_SEAT_ADD, NULL, NULL, NULL);

   return s;
}

static Ecore_Fb_Seat *
_seat_get(Ecore_Fb_Input *input, const char *seat)
{
   Ecore_Fb_Seat *s;
   Eina_List *l;

   /* search for this name in existing seats */
   EINA_LIST_FOREACH(input->dev->seats, l, s)
     if (!strcmp(s->name, seat)) return s;

   return _seat_create(input, seat);
}

static void
_device_added(Ecore_Fb_Input *input, struct libinput_device *device)
{
   struct libinput_seat *libinput_seat;
   const char *seat_name;
   Ecore_Fb_Seat *seat;
   Ecore_Fb_Evdev *edev;

   libinput_seat = libinput_device_get_seat(device);
   seat_name = libinput_seat_get_logical_name(libinput_seat);

   /* try to get a seat */
   if (!(seat = _seat_get(input, seat_name)))
     {
        ERR("Could not get matching seat: %s", seat_name);
        return;
     }

   /* try to create a new evdev device */
   if (!(edev = _ecore_fb_evdev_device_create(seat, device)))
     {
        ERR("Failed to create new evdev device");
        return;
     }

   /* append this device to the seat */
   seat->devices = eina_list_append(seat->devices, edev);
}

static void
_device_removed(Ecore_Fb_Input *input EINA_UNUSED, struct libinput_device *device)
{
   Ecore_Fb_Evdev *edev;

   /* try to get the evdev structure */
   if (!(edev = libinput_device_get_user_data(device)))
     return;

   /* remove this evdev from the seat's list of devices */
   edev->seat->devices = eina_list_remove(edev->seat->devices, edev);

   /* tell launcher to release device */
   _ecore_fb_launcher_device_close(edev->path, edev->fd);

   /* destroy this evdev */
   _ecore_fb_evdev_device_destroy(edev);
}

static int
_udev_event_process(struct libinput_event *event)
{
   struct libinput *libinput;
   struct libinput_device *device;
   Ecore_Fb_Input *input;
   Eina_Bool ret = EINA_TRUE;

   libinput = libinput_event_get_context(event);
   input = libinput_get_user_data(libinput);
   device = libinput_event_get_device(event);

   switch (libinput_event_get_type(event))
     {
      case LIBINPUT_EVENT_DEVICE_ADDED:
        _device_added(input, device);
        break;
      case LIBINPUT_EVENT_DEVICE_REMOVED:
        _device_removed(input, device);
        break;
      default:
        ret = EINA_FALSE;
     }

   return ret;
}

static void
_input_event_process(struct libinput_event *event)
{
   if (_udev_event_process(event)) return;
   if (_ecore_fb_evdev_event_process(event)) return;
}

static void
_input_events_process(Ecore_Fb_Input *input)
{
   struct libinput_event *event;

   while ((event = libinput_get_event(input->libinput)))
     {
        _input_event_process(event);
        libinput_event_destroy(event);
     }
}

static Eina_Bool
_cb_input_dispatch(void *data, Ecore_Fd_Handler *hdlr EINA_UNUSED)
{
   Ecore_Fb_Input *input;

   if (!(input = data)) return EINA_TRUE;

   if (libinput_dispatch(input->libinput) != 0)
     ERR("Failed to dispatch libinput events: %m");

   /* process pending events */
   _input_events_process(input);

   return EINA_TRUE;
}

const struct libinput_interface _input_interface =
{
   _cb_open_restricted,
   _cb_close_restricted,
};

/* public functions */
EAPI Eina_Bool
ecore_fb_inputs_create(Ecore_Fb_Device *dev)
{
   Ecore_Fb_Input *input;

   /* check for valid device */
   EINA_SAFETY_ON_NULL_RETURN_VAL(dev, EINA_FALSE);

   /* try to allocate space for new input structure */
   if (!(input = calloc(1, sizeof(Ecore_Fb_Input))))
     return EINA_FALSE;

   /* set reference for parent device */
   input->dev = dev;

   /* try to create libinput context */
   input->libinput =
     libinput_udev_create_context(&_input_interface, input, eeze_udev_get());
   if (!input->libinput)
     {
        ERR("Could not create libinput context: %m");
        goto err;
     }

   /* set libinput log priority */
   libinput_log_set_priority(input->libinput, LIBINPUT_LOG_PRIORITY_INFO);

   /* assign udev seat */
   if (libinput_udev_assign_seat(input->libinput, dev->seat) != 0)
     {
        ERR("Failed to assign seat: %m");
        goto err;
     }

   /* process pending events */
   _input_events_process(input);

   /* enable this input */
   if (!ecore_fb_inputs_enable(input))
     {
        ERR("Failed to enable input");
        goto err;
     }

   /* append this input */
   dev->inputs = eina_list_append(dev->inputs, input);

   return EINA_TRUE;

err:
   if (input->libinput) libinput_unref(input->libinput);
   free(input);
   return EINA_FALSE;
}

EAPI void
ecore_fb_inputs_destroy(Ecore_Fb_Device *dev)
{
   Ecore_Fb_Input *input;
   Ecore_Fb_Seat *seat;
   Ecore_Fb_Evdev *edev;

   EINA_SAFETY_ON_NULL_RETURN(dev);
   EINA_LIST_FREE(dev->seats, seat)
     {
        EINA_LIST_FREE(seat->devices, edev)
          {
             _ecore_fb_launcher_device_close(edev->path, edev->fd);
             _ecore_fb_evdev_device_destroy(edev);
          }

        if (seat->name) eina_stringshare_del(seat->name);
        free(seat);
     }

   EINA_LIST_FREE(dev->inputs, input)
     {
        if (input->hdlr) ecore_main_fd_handler_del(input->hdlr);
        if (input->libinput) libinput_unref(input->libinput);
        free(input);
     }
}

EAPI Eina_Bool
ecore_fb_inputs_enable(Ecore_Fb_Input *input)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(input, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(input->libinput, EINA_FALSE);

   input->fd = libinput_get_fd(input->libinput);

   if (!input->hdlr)
     {
        input->hdlr =
          ecore_main_fd_handler_add(input->fd, ECORE_FD_READ,
                                    _cb_input_dispatch, input, NULL, NULL);
     }

   if (input->suspended)
     {
        if (libinput_resume(input->libinput) != 0)
          goto err;

        input->suspended = EINA_FALSE;

        /* process pending events */
        _input_events_process(input);
     }

   input->enabled = EINA_TRUE;
   input->suspended = EINA_FALSE;

   return EINA_TRUE;

err:
   input->enabled = EINA_FALSE;
   if (input->hdlr) ecore_main_fd_handler_del(input->hdlr);
   input->hdlr = NULL;
   return EINA_FALSE;
}

EAPI void
ecore_fb_inputs_disable(Ecore_Fb_Input *input)
{
   EINA_SAFETY_ON_NULL_RETURN(input);
   EINA_SAFETY_ON_TRUE_RETURN(input->suspended);

   /* suspend this input */
   libinput_suspend(input->libinput);

   /* process pending events */
   _input_events_process(input);

   input->suspended = EINA_TRUE;
}
