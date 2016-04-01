#include "elput_private.h"

static int
_cb_open_restricted(const char *path, int flags, void *data)
{
   Elput_Manager *em;

   em = data;
   return elput_manager_open(em, path, flags);
}

static void
_cb_close_restricted(int fd, void *data)
{
   Elput_Manager *em;

   em = data;
   elput_manager_close(em, fd);
}

const struct libinput_interface _input_interface =
{
   _cb_open_restricted,
   _cb_close_restricted,
};

static int
_udev_process_event(struct libinput_event *event)
{
   Elput_Manager *em;
   struct libinput *lib;
   struct libinput_device *dev;
   int ret = 1;

   lib = libinput_event_get_context(event);
   dev = libinput_event_get_device(event);
   em = libinput_get_user_data(lib);

   switch (libinput_event_get_type(event))
     {
      case LIBINPUT_EVENT_DEVICE_ADDED:
        DBG("Input Device Added: %s", libinput_device_get_name(dev));
        /* TODO: add device */
        break;
      case LIBINPUT_EVENT_DEVICE_REMOVED:
        DBG("Input Device Removed: %s", libinput_device_get_name(dev));
        /* TODO: remove device */
        break;
      default:
        ret = 0;
        break;
     }

   return ret;
}

static void
_process_event(struct libinput_event *event)
{
   if (_udev_process_event(event)) return;
   /* TODO: evdev processing */
}

static void
_process_events(Elput_Input *ei)
{
   struct libinput_event *event;

   while ((event = libinput_get_event(ei->lib)))
     {
        _process_event(event);
        libinput_event_destroy(event);
     }
}

static Eina_Bool
_cb_input_dispatch(void *data, Ecore_Fd_Handler *hdlr EINA_UNUSED)
{
   Elput_Input *ei;

   ei = data;

   if (libinput_dispatch(ei->lib) != 0)
     WRN("libinput failed to dispatch events");

   _process_events(ei);

   return EINA_TRUE;
}

EAPI Eina_Bool
elput_input_init(Elput_Manager *manager, const char *seat)
{
   int fd;

   EINA_SAFETY_ON_NULL_RETURN_VAL(manager, EINA_FALSE);

   memset(&manager->input, 0, sizeof(Elput_Input));

   manager->input.lib =
     libinput_udev_create_context(&_input_interface, manager, eeze_udev_get());
   if (!manager->input.lib)
     {
        ERR("libinput could not create udev context");
        goto udev_err;
     }

   if (libinput_udev_assign_seat(manager->input.lib, seat) != 0)
     {
        ERR("libinput could not assign udev seat");
        goto seat_err;
     }

   _process_events(&manager->input);

   fd = libinput_get_fd(manager->input.lib);

   manager->input.hdlr =
     ecore_main_fd_handler_add(fd, ECORE_FD_READ, _cb_input_dispatch,
                               &manager->input, NULL, NULL);
   if (!manager->input.hdlr)
     {
        ERR("Could not create input fd handler");
        goto hdlr_err;
     }

   return EINA_TRUE;

hdlr_err:
seat_err:
   libinput_unref(manager->input.lib);
udev_err:
   return EINA_FALSE;
}

EAPI void
elput_input_shutdown(Elput_Manager *manager)
{
   EINA_SAFETY_ON_NULL_RETURN(manager);
   EINA_SAFETY_ON_NULL_RETURN(&manager->input);

   if (manager->input.hdlr) ecore_main_fd_handler_del(manager->input.hdlr);

   /* TODO */

   libinput_unref(manager->input.lib);
}
