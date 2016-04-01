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

static void
_process_event(struct libinput_event *event)
{
   /* TODO */
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

EAPI Eina_Bool
elput_input_init(Elput_Manager *manager, const char *seat)
{
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

   /* TODO */

   return EINA_TRUE;

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

   /* TODO */

   libinput_unref(manager->input.lib);
}
