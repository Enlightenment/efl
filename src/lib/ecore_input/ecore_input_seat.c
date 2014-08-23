#include "ecore_input_private.h"

#ifdef HAVE_LIBINPUT
static struct libinput *_libinput;
static Ecore_Fd_Handler *_libinput_hdlr;
Eina_List *_ecore_input_devices;
#endif

#ifdef HAVE_LIBINPUT
static void 
_ecore_input_device_added(struct libinput_device *dev)
{
   Ecore_Input_Device *edev;
   struct libinput_seat *seat;
   const char *sname;

   /* allocate space for new input device */
   if (!(edev = calloc(1, sizeof(Ecore_Input_Device))))
     {
        ERR("Failed to allocate space for input device");
        return;
     }

   seat = libinput_device_get_seat(dev);
   sname = libinput_seat_get_logical_name(seat);

   edev->seat = eina_stringshare_add(sname);
   edev->name = eina_stringshare_add(libinput_device_get_sysname(dev));
   edev->output = eina_stringshare_add(libinput_device_get_output_name(dev));

   DBG("Device Added: %s", edev->name);

   /* check input device capabilities */
   if (libinput_device_has_capability(dev, LIBINPUT_DEVICE_CAP_KEYBOARD))
     edev->type |= ECORE_INPUT_DEVICE_TYPE_KEYBOARD;

   if (libinput_device_has_capability(dev, LIBINPUT_DEVICE_CAP_POINTER))
     edev->type |= ECORE_INPUT_DEVICE_TYPE_POINTER;

   if (libinput_device_has_capability(dev, LIBINPUT_DEVICE_CAP_TOUCH))
     edev->type |= ECORE_INPUT_DEVICE_TYPE_TOUCH;

   libinput_device_set_user_data(dev, edev);
   libinput_device_ref(dev);

   /* append this device to internal list */
   _ecore_input_devices = eina_list_append(_ecore_input_devices, edev);

   /* FIXME: Raise an Ecore_Input_Event here for device added and pass it the 
    * Ecore_Input_Device */
}

static void 
_ecore_input_device_removed(struct libinput_device *dev)
{
   Ecore_Input_Device *edev;

   /* try to get the ecore_input device from this libinput device */
   if (!(edev = libinput_device_get_user_data(dev)))
     {
        ERR("Could not get Ecore_Input_Device from libinput device: %m");
        return;
     }

   DBG("Device Removed: %s", edev->name);

   /* FIXME: Raise an Ecore_Input_Event here for device removed */

   /* remove this device from our internal list */
   _ecore_input_devices = eina_list_remove(_ecore_input_devices, edev);

   /* unreference the libinput device */
   libinput_device_unref(dev);

   /* free our input device structure */
   eina_stringshare_del(edev->output);
   eina_stringshare_del(edev->name);
   eina_stringshare_del(edev->seat);
   free(edev);
}

static int 
_ecore_input_restricted_cb_open(const char *path, int flags EINA_UNUSED, void *data EINA_UNUSED)
{
   DBG("Open Restricted: %s", path);
   return _ecore_input_dbus_device_open(path);
}

static void 
_ecore_input_restricted_cb_close(int fd, void *data EINA_UNUSED)
{
   DBG("Close Restricted: %d", fd);
   _ecore_input_dbus_device_close(fd);
}

const struct libinput_interface _interface = 
{
   _ecore_input_restricted_cb_open,
   _ecore_input_restricted_cb_close,
};

static Eina_Bool 
_ecore_input_event_process(struct libinput_event *ev)
{
   Eina_Bool ret = EINA_TRUE;
   struct libinput_device *dev;

   dev = libinput_event_get_device(ev);

   /* TODO: handle other events */
   switch (libinput_event_get_type(ev))
     {
      case LIBINPUT_EVENT_DEVICE_ADDED:
        _ecore_input_device_added(dev);
        break;
      case LIBINPUT_EVENT_DEVICE_REMOVED:
        _ecore_input_device_removed(dev);
        break;
      case LIBINPUT_EVENT_KEYBOARD_KEY:
        break;
      case LIBINPUT_EVENT_POINTER_MOTION:
        break;
      case LIBINPUT_EVENT_POINTER_MOTION_ABSOLUTE:
        break;
      case LIBINPUT_EVENT_POINTER_BUTTON:
        break;
      case LIBINPUT_EVENT_POINTER_AXIS:
        break;
      case LIBINPUT_EVENT_TOUCH_DOWN:
        break;
      case LIBINPUT_EVENT_TOUCH_MOTION:
        break;
      case LIBINPUT_EVENT_TOUCH_UP:
        break;
      case LIBINPUT_EVENT_TOUCH_FRAME:
        break;
      default:
        ret = EINA_FALSE;
        break;
     }

   return ret;
}

static void 
_ecore_input_events_process(void)
{
   struct libinput_event *ev;

   /* loop any libinput events and process them */
   while ((ev = libinput_get_event(_libinput)))
     {
        _ecore_input_event_process(ev);
        libinput_event_destroy(ev);
     }
}

static Eina_Bool 
_ecore_input_cb_dispatch(void *data EINA_UNUSED, Ecore_Fd_Handler *hdlr EINA_UNUSED)
{
   /* try to let libinput dispatch events */
   if (libinput_dispatch(_libinput) != 0)
     {
        ERR("libinput Failed to dispatch events: %m");
        return ECORE_CALLBACK_RENEW;
     }

   /* process any libinput events */
   _ecore_input_events_process();

   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool 
_ecore_input_enable(void)
{
   int fd = -1;

   if (_libinput_hdlr) return EINA_TRUE;

   /* get the fd from libinput */
   if ((fd = libinput_get_fd(_libinput)) < 0)
     {
        ERR("Could not get fd from libinput: %m");
        return EINA_FALSE;
     }

   /* setup handler to listen for libinput events */
   _libinput_hdlr = 
     ecore_main_fd_handler_add(fd, ECORE_FD_READ, 
                               _ecore_input_cb_dispatch, NULL, NULL, NULL);
   if (!_libinput_hdlr)
     {
        ERR("Could not create fd handler");
        return EINA_FALSE;
     }

   return EINA_TRUE;
}
#endif

EAPI Eina_Bool 
ecore_input_seat_init(const char *seat)
{
#ifdef HAVE_LIBINPUT
   if (!_ecore_input_udev) return EINA_FALSE;

   /* try to create a reference to libinput */
   _libinput = 
     libinput_udev_create_context(&_interface, NULL, _ecore_input_udev);
   if (!_libinput)
     {
        ERR("libinput failed to create %s for udev: %m", seat);
        return EINA_FALSE;
     }

   /* get existing eina log level for this domain and set libinput log level */
   switch (eina_log_domain_level_get("ecore_input"))
     {
      case EINA_LOG_LEVEL_DBG:
        libinput_log_set_priority(_libinput, LIBINPUT_LOG_PRIORITY_DEBUG);
        break;
      case EINA_LOG_LEVEL_WARN:
      case EINA_LOG_LEVEL_ERR:
      case EINA_LOG_LEVEL_CRITICAL:
        libinput_log_set_priority(_libinput, LIBINPUT_LOG_PRIORITY_ERROR);
        break;
      case EINA_LOG_LEVEL_INFO:
      default:
        libinput_log_set_priority(_libinput, LIBINPUT_LOG_PRIORITY_INFO);
        break;
     }

   /* try to assign this seat to libinput */
   if (libinput_udev_assign_seat(_libinput, seat) != 0)
     {
        ERR("libinput failed to assign seat %s: %m", seat);
        libinput_unref(_libinput);
        return EINA_FALSE;
     }

   /* process any pending libinput events */
   _ecore_input_events_process();

   return _ecore_input_enable();
#endif

   return EINA_TRUE;
}

EAPI void 
ecore_input_seat_shutdown(const char *seat EINA_UNUSED)
{
#ifdef HAVE_LIBINPUT
   if (_libinput_hdlr) ecore_main_fd_handler_del(_libinput_hdlr);
   _libinput_hdlr = NULL;

   /* destroy the libinput reference */
   if (_libinput) libinput_unref(_libinput);
#endif
}
