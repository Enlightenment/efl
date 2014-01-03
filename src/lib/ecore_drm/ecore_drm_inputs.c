#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ecore_drm_private.h"

/* local functions */
static void 
_ecore_drm_inputs_seat_devices_remove(Ecore_Drm_Seat *seat)
{
   Ecore_Drm_Evdev *edev;

   EINA_LIST_FREE(seat->devices, edev)
     _ecore_drm_evdev_device_destroy(edev);
}

static Ecore_Drm_Seat *
_ecore_drm_inputs_seat_add(Ecore_Drm_Input *input EINA_UNUSED, struct libinput_seat *lseat)
{
   Ecore_Drm_Seat *eseat;
   const char *name;

   if (!(eseat = calloc(1, sizeof(Ecore_Drm_Seat))))
     return NULL;

   name = libinput_seat_get_name(lseat);
   eseat->name = eina_stringshare_add(name);

   eseat->seat = lseat;

   libinput_seat_ref(lseat);
   libinput_seat_set_user_data(lseat, eseat);

   return eseat;
}

static void 
_ecore_drm_inputs_seat_del(Ecore_Drm_Seat *seat)
{
   _ecore_drm_inputs_seat_devices_remove(seat);
   libinput_seat_ref(seat->seat);
   if (seat->name) eina_stringshare_del(seat->name);
   free(seat);
}

static Eina_Bool 
_ecore_drm_inputs_device_add(Ecore_Drm_Input *input, struct libinput_device *ldevice)
{
   struct libinput *linput;
   struct libinput_seat *lseat;
   Ecore_Drm_Evdev *edev;
   Ecore_Drm_Seat *eseat;

   linput = input->linput;

   if (!(edev = _ecore_drm_evdev_device_create(linput, ldevice)))
     return EINA_FALSE;

   lseat = libinput_device_get_seat(ldevice);
   eseat = libinput_seat_get_user_data(lseat);

   eseat->devices = eina_list_append(eseat->devices, edev);

   /* TODO: output and seat repick ?? */

   return EINA_TRUE;
}

static void 
_ecore_drm_inputs_device_del(Ecore_Drm_Input *input)
{
   if (!input) return;

   /* remove fd listener */
   if (input->input_hdlr) ecore_main_fd_handler_del(input->input_hdlr);

   /* destroy libinput reference */
   if (input->linput) libinput_destroy(input->linput);

   /* free structure */
   free(input);
}

static int 
_ecore_drm_inputs_restricted_open(const char *path, int flags EINA_UNUSED, void *data EINA_UNUSED)
{
   char p[PATH_MAX];
   int fd = -1;

   strcpy(p, path);
   _ecore_drm_message_send(ECORE_DRM_OP_DEVICE_OPEN, -1, p, strlen(p));
   _ecore_drm_message_receive(ECORE_DRM_OP_DEVICE_OPEN, &fd, NULL, 0);

   DBG("Open Restricted: %s %d", path, fd);
   return fd;
}

static void 
_ecore_drm_inputs_restricted_close(int fd, void *data EINA_UNUSED)
{
   int dfd = -1;

   DBG("Close Restricted: %d", fd);
   _ecore_drm_message_send(ECORE_DRM_OP_DEVICE_CLOSE, fd, NULL, 0);
   _ecore_drm_message_receive(ECORE_DRM_OP_DEVICE_CLOSE, &dfd, NULL, 0);
}

static void 
_ecore_drm_inputs_screen_size_get(struct libinput_device *input_device EINA_UNUSED, int *w EINA_UNUSED, int *h EINA_UNUSED, void *data EINA_UNUSED)
{
   /* Ecore_Drm_Input *input; */

   /* input = data; */
   DBG("FIXME: Screen Size Get");
}

static const struct libinput_interface _input_interface = 
{
   _ecore_drm_inputs_restricted_open,
   _ecore_drm_inputs_restricted_close,
   _ecore_drm_inputs_screen_size_get,
};

static Eina_Bool 
_ecore_drm_inputs_udev_event_process(struct libinput_event *event)
{
   struct libinput *linput;
   struct libinput_device *ldevice;
   Ecore_Drm_Input *input;
   Eina_Bool ret = EINA_FALSE;

   if (!(linput = libinput_event_get_target(event).libinput))
     return EINA_FALSE;
   if (!(input = libinput_get_user_data(linput))) return EINA_FALSE;

   switch (libinput_event_get_type(event))
     {
      case LIBINPUT_EVENT_ADDED_SEAT:
          {
             struct libinput_event_added_seat *ev;
             struct libinput_seat *lseat;
             Ecore_Drm_Seat *seat;

             DBG("Seat Added");

             ev = (struct libinput_event_added_seat *)event;
             lseat = libinput_event_added_seat_get_seat(ev);
             if ((seat = _ecore_drm_inputs_seat_add(input, lseat)))
               ret = EINA_TRUE;
          }
        break;
      case LIBINPUT_EVENT_REMOVED_SEAT:
          {
             struct libinput_event_removed_seat *ev;
             struct libinput_seat *lseat;
             Ecore_Drm_Seat *seat;

             DBG("Seat Removed");

             ev = (struct libinput_event_removed_seat *)event;
             lseat = libinput_event_removed_seat_get_seat(ev);
             seat = libinput_seat_get_user_data(lseat);
             libinput_seat_unref(lseat);
             _ecore_drm_inputs_seat_del(seat);

             ret = EINA_TRUE;
          }
        break;
      case LIBINPUT_EVENT_ADDED_DEVICE:
          {
             struct libinput_event_added_device *ev;

             DBG("Device Added");

             ev = (struct libinput_event_added_device *)event;
             ldevice = libinput_event_added_device_get_device(ev);

             ret = _ecore_drm_inputs_device_add(input, ldevice);
          }
        break;
      case LIBINPUT_EVENT_REMOVED_DEVICE:
          {
             struct libinput_event_removed_device *ev;
             Ecore_Drm_Evdev *edev;

             DBG("Device Removed");

             ev = (struct libinput_event_removed_device *)event;
             ldevice = libinput_event_removed_device_get_device(ev);

             edev = libinput_device_get_user_data(ldevice);
             _ecore_drm_evdev_device_destroy(edev);

             ret = EINA_TRUE;
          }
        break;
      default:
        DBG("Unhandled Event");
        ret = EINA_FALSE;
        break;
     }

   return ret;
}

static void 
_ecore_drm_inputs_event_process(struct libinput_event *event)
{
   if (_ecore_drm_inputs_udev_event_process(event))
     return;
   if (_ecore_drm_evdev_event_process(event))
     return;
}

static void 
_ecore_drm_inputs_events_process(Ecore_Drm_Input *input)
{
   struct libinput_event *event = NULL;

   if (!input) return;

   while ((event == libinput_get_event(input->linput)))
     {
        _ecore_drm_inputs_event_process(event);
        libinput_event_destroy(event);
     }
}

static Eina_Bool 
_ecore_drm_inputs_cb_input_event(void *data, Ecore_Fd_Handler *hdlr EINA_UNUSED)
{
   Ecore_Drm_Input *input;

   if (!(input = data)) return EINA_FALSE;

   if (libinput_dispatch(input->linput) != 0)
     ERR("Failed to dispatch input events");

   _ecore_drm_inputs_events_process(input);

   return EINA_TRUE;
}

/**
 * @defgroup Ecore_Drm_Input_Group
 * 
 */

/* TODO: DOXY !! */

EAPI Eina_Bool 
ecore_drm_inputs_create(Ecore_Drm_Device *dev)
{
   Ecore_Drm_Input *input;

   if ((!dev) || (!udev)) return EINA_FALSE;

   /* allocate space for input structure */
   if (!(input = calloc(1, sizeof(Ecore_Drm_Input))))
     {
        ERR("Failed to allocate space for input structure");
        return EINA_FALSE;
     }

   /* create libinput structure */
   if (!(input->linput = 
         libinput_create_from_udev(&_input_interface, input, udev, "seat0")))
     {
        ERR("Failed to create libinput");
        free(input);
        return EINA_FALSE;
     }

   _ecore_drm_inputs_events_process(input);

   /* try to enable this input */
   if (!ecore_drm_inputs_enable(input))
     {
        ERR("Failed to enable input");
        goto enable_err;
     }

   /* add this input device */
   dev->inputs = eina_list_append(dev->inputs, input);

   return EINA_TRUE;

enable_err:
   if (input->linput) libinput_destroy(input->linput);
   free(input);
   return EINA_FALSE;
}

EAPI void 
ecore_drm_inputs_destroy(Ecore_Drm_Device *dev)
{
   Ecore_Drm_Input *input;

   if (!dev) return;

   EINA_LIST_FREE(dev->inputs, input)
     _ecore_drm_inputs_device_del(input);
}

EAPI Eina_Bool 
ecore_drm_inputs_enable(Ecore_Drm_Input *input)
{
   if (!input) return EINA_FALSE;

   /* try to get the fd */
   if (input->fd <= 0)
     input->fd = libinput_get_fd(input->linput);
   if (input->fd <= 0)
     {
        ERR("Failed to get input fd");
        return EINA_FALSE;
     }

   /* setup fd handler for reading events */
   if (!input->input_hdlr)
     {
        input->input_hdlr = 
          ecore_main_fd_handler_add(input->fd, ECORE_FD_READ, 
                                    _ecore_drm_inputs_cb_input_event, input, 
                                    NULL, NULL);
     }

   if (!input->input_hdlr) 
     {
        ERR("Failed to setup fd handler for input");
        return EINA_FALSE;
     }

   /* handle suspended input */
   if (input->suspended)
     {
        if (libinput_resume(input->linput) != 0)
          {
             if (input->input_hdlr)
               ecore_main_fd_handler_del(input->input_hdlr);
             input->input_hdlr = NULL;
             return EINA_FALSE;
          }
        input->suspended = EINA_FALSE;
        _ecore_drm_inputs_events_process(input);
     }

   /* TODO: notify keyboard focus ? */

   return EINA_TRUE;
}

EAPI void 
ecore_drm_inputs_disable(Ecore_Drm_Input *input)
{
   if (!input) return;

   libinput_suspend(input->linput);
   input->suspended = EINA_TRUE;
}
