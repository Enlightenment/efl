#include "ecore_input_private.h"

#ifdef HAVE_LIBINPUT
static struct libinput *_libinput;
static Ecore_Fd_Handler *_libinput_hdlr;
#endif

#ifdef HAVE_LIBINPUT
static void 
_ecore_input_device_added(struct libinput_device *dev EINA_UNUSED)
{
   DBG("Device Added");
}

static void 
_ecore_input_device_removed(struct libinput_device *dev EINA_UNUSED)
{
   DBG("Device Removed");
}

static int 
_ecore_input_restricted_cb_open(const char *path, int flags EINA_UNUSED, void *data EINA_UNUSED)
{
   DBG("Open Restricted: %s", path);
   return 1;
}

static void 
_ecore_input_restricted_cb_close(int fd EINA_UNUSED, void *data EINA_UNUSED)
{
   DBG("Close Restricted");
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

   switch (libinput_event_get_type(ev))
     {
      case LIBINPUT_EVENT_DEVICE_ADDED:
        _ecore_input_device_added(dev);
        break;
      case LIBINPUT_EVENT_DEVICE_REMOVED:
        _ecore_input_device_removed(dev);
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

   /* try to create a seat */
   _libinput = 
     libinput_udev_create_for_seat(&_interface, NULL, 
                                   _ecore_input_udev, seat);
   if (!_libinput)
     {
        ERR("libinput failed to create %s for udev: %m", seat);
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
   if (_libinput) libinput_destroy(_libinput);
#endif
}
