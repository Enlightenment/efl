#include "elput_private.h"
#include <libudev.h>

void
_elput_input_window_update(Elput_Manager *manager)
{
   Eina_List *l, *ll;
   Elput_Seat *seat;
   Elput_Device *device;

   if (manager->input.thread) return;
   EINA_LIST_FOREACH(manager->input.seats, l, seat)
     EINA_LIST_FOREACH(seat->devices, ll, device)
       device->window = manager->window;
}

void
_elput_input_pointer_max_update(Elput_Manager *manager)
{
   Eina_List *l;
   Elput_Seat *eseat;

   if (manager->input.thread) return;
   EINA_LIST_FOREACH(manager->input.seats, l, eseat)
     {
        if (!eseat->ptr) continue;

        eseat->ptr->maxw = manager->input.pointer_w;
        eseat->ptr->maxh = manager->input.pointer_h;
     }
}

static int
_cb_open_restricted(const char *path, int flags, void *data)
{
   Elput_Manager *em = data;
   int ret = -1;
   Elput_Async_Open *ao;
   int p[2];

   if (!em->input.thread)
     return em->interface->open(em, path, flags);
   if (!em->interface->open_async) return ret;
   ao = calloc(1, sizeof(Elput_Async_Open));
   if (!ao) return ret;
   if (pipe2(p, O_CLOEXEC) < 0)
     {
        free(ao);
        return ret;
     }
   ao->manager = em;
   ao->path = strdup(path);
   ao->flags = flags;
   em->input.pipe = p[1];
   ecore_thread_feedback(em->input.thread, ao);
   while (!ecore_thread_check(em->input.thread))
     {
        int avail, fd;
        fd_set rfds, wfds, exfds;
        struct timeval tv, *t;

        FD_ZERO(&rfds);
        FD_ZERO(&wfds);
        FD_ZERO(&exfds);
        FD_SET(p[0], &rfds);
        tv.tv_sec = 0;
        tv.tv_usec = 300;
        t = &tv;
        avail = select(p[0] + 1, &rfds, &wfds, &exfds, t);
        if (avail > 0)
          {
             read(p[0], &fd, sizeof(int));
             ret = fd;
             break;
          }
        if (avail < 0) break;
     }
   close(p[0]);
   return ret;
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

static Elput_Seat *
_udev_seat_create(Elput_Manager *em, const char *name)
{
   Elput_Seat *eseat;

   eseat = calloc(1, sizeof(Elput_Seat));
   if (!eseat) return NULL;

   eseat->name = eina_stringshare_add(name);
   em->input.seats = eina_list_append(em->input.seats, eseat);

   return eseat;
}

static void
_udev_seat_destroy(Elput_Seat *eseat)
{
   Elput_Device *edev;

   EINA_LIST_FREE(eseat->devices, edev)
     _evdev_device_destroy(edev);

   if (eseat->kbd) _evdev_keyboard_destroy(eseat->kbd);
   if (eseat->ptr) _evdev_pointer_destroy(eseat->ptr);
   if (eseat->touch) _evdev_touch_destroy(eseat->touch);

   eina_stringshare_del(eseat->name);
   free(eseat);
}

static Elput_Seat *
_udev_seat_named_get(Elput_Manager *em, const char *name)
{
   Elput_Seat *eseat;
   Eina_List *l;

   EINA_LIST_FOREACH(em->input.seats, l, eseat)
     if (!strcmp(eseat->name, name)) return eseat;

   return _udev_seat_create(em, name);
}

static Elput_Seat *
_udev_seat_get(Elput_Manager *em, struct libinput_device *device)
{
   struct libinput_seat *lseat;
   const char *name;

   lseat = libinput_device_get_seat(device);
   name = libinput_seat_get_logical_name(lseat);

   return _udev_seat_named_get(em, name);
}

static void
_device_event_cb_free(void *data EINA_UNUSED, void *event)
{
   Elput_Event_Device_Change *ev;

   ev = event;

   if (ev->type == ELPUT_DEVICE_REMOVED)
     {
        Elput_Seat *seat;

        seat = ev->device->seat;
        if (seat)
          seat->devices = eina_list_remove(seat->devices, ev->device);

        _evdev_device_destroy(ev->device);
     }

   free(ev);
}

static void
_device_event_send(Elput_Device *edev, Elput_Device_Change_Type type)
{
   Elput_Event_Device_Change *ev;

   ev = calloc(1, sizeof(Elput_Event_Device_Change));
   if (!ev) return;

   ev->device = edev;
   ev->type = type;

   ecore_event_add(ELPUT_EVENT_DEVICE_CHANGE, ev, _device_event_cb_free, NULL);
}

static void
_device_add(Elput_Manager *em, struct libinput_device *dev)
{
   Elput_Seat *eseat;
   Elput_Device *edev;
   const char *oname;

   eseat = _udev_seat_get(em, dev);
   if (!eseat) return;

   edev = _evdev_device_create(eseat, dev);
   if (!edev) return;

   oname = libinput_device_get_output_name(dev);
   eina_stringshare_replace(&edev->output_name, oname);

   eseat->devices = eina_list_append(eseat->devices, edev);

   _device_event_send(edev, ELPUT_DEVICE_ADDED);
}

static void
_device_remove(Elput_Manager *em EINA_UNUSED, struct libinput_device *device)
{
   Elput_Device *edev;

   edev = libinput_device_get_user_data(device);
   if (!edev) return;

   _device_event_send(edev, ELPUT_DEVICE_REMOVED);
}

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
        _device_add(em, dev);
        break;
      case LIBINPUT_EVENT_DEVICE_REMOVED:
        DBG("Input Device Removed: %s", libinput_device_get_name(dev));
        _device_remove(em, dev);
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
   if (_evdev_event_process(event)) return;
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

static void
_elput_input_init_cancel(void *data, Ecore_Thread *eth EINA_UNUSED)
{
   Elput_Manager *manager = data;

   manager->input.thread = NULL;
   if (manager->input.current_pending)
     {
        eldbus_pending_cancel(manager->input.current_pending);
        if (manager->input.pipe >= 0)
          close(manager->input.pipe);
     }
   if (manager->del)
     elput_manager_disconnect(manager);
}

static void
_elput_input_init_end(void *data, Ecore_Thread *eth EINA_UNUSED)
{
   Elput_Manager *manager = data;

   manager->input.thread = NULL;
   if (!manager->input.lib) return;
   manager->input.hdlr =
     ecore_main_fd_handler_add(libinput_get_fd(manager->input.lib), ECORE_FD_READ,
       _cb_input_dispatch, &manager->input, NULL, NULL);

   if (manager->input.hdlr)
     {
        _process_events(&manager->input);
        _elput_input_window_update(manager);
        _elput_input_pointer_max_update(manager);
     }
   else
     {
        ERR("Could not create input fd handler");
        libinput_unref(manager->input.lib);
        manager->input.lib = NULL;
     }
}

static void
_elput_input_init_notify(void *data EINA_UNUSED, Ecore_Thread *eth EINA_UNUSED, void *msg_data)
{
   Elput_Async_Open *ao = msg_data;

   ao->manager->interface->open_async(ao->manager, ao->path, ao->flags);
   free(ao->path);
   free(ao);
}

static void
_elput_input_init_thread(void *data, Ecore_Thread *eth EINA_UNUSED)
{
   Elput_Manager *manager = data;
   struct udev *udev = udev_new();

   manager->input.lib =
     libinput_udev_create_context(&_input_interface, manager, udev);
   if (!manager->input.lib)
     {
        ERR("libinput could not create udev context");
        return;
     }
   udev_unref(udev);

   if (libinput_udev_assign_seat(manager->input.lib, manager->seat))
     {
        ERR("libinput could not assign udev seat");
        libinput_unref(manager->input.lib);
        manager->input.lib = NULL;
     }
}

EAPI Eina_Bool
elput_input_init(Elput_Manager *manager)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(manager, EINA_FALSE);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(!!manager->input.hdlr, EINA_TRUE);

   memset(&manager->input, 0, sizeof(Elput_Input));
   manager->input.thread =
     ecore_thread_feedback_run(_elput_input_init_thread, _elput_input_init_notify,
     _elput_input_init_end, _elput_input_init_cancel, manager, 1);
   return !!manager->input.thread;
}

EAPI void
elput_input_shutdown(Elput_Manager *manager)
{
   Elput_Seat *seat;

   EINA_SAFETY_ON_NULL_RETURN(manager);

   ecore_main_fd_handler_del(manager->input.hdlr);

   EINA_LIST_FREE(manager->input.seats, seat)
     _udev_seat_destroy(seat);
   if (manager->input.thread)
     ecore_thread_cancel(manager->input.thread);
   else
     {
        libinput_unref(manager->input.lib);
        manager->input.lib = NULL;
     }
}

EAPI void
elput_input_pointer_xy_get(Elput_Manager *manager, const char *seat, int *x, int *y)
{
   Elput_Seat *eseat;
   Eina_List *l;

   if (x) *x = 0;
   if (y) *y = 0;

   EINA_SAFETY_ON_NULL_RETURN(manager);

   /* if no seat name is passed in, just use default seat name */
   if (!seat) seat = "seat0";

   EINA_LIST_FOREACH(manager->input.seats, l, eseat)
     {
        if (!eseat->ptr) continue;
        if ((eseat->name) && (strcmp(eseat->name, seat)))
          continue;
        if (x) *x = eseat->ptr->x;
        if (y) *y = eseat->ptr->y;
        break;
     }
}

EAPI void
elput_input_pointer_xy_set(Elput_Manager *manager, const char *seat, int x, int y)
{
   Elput_Seat *eseat;
   Elput_Device *edev;
   Eina_List *l, *ll;

   EINA_SAFETY_ON_NULL_RETURN(manager);

   /* if no seat name is passed in, just use default seat name */
   if (!seat) seat = "seat0";

   EINA_LIST_FOREACH(manager->input.seats, l, eseat)
     {
        if (!eseat->ptr) continue;
        if ((eseat->name) && (strcmp(eseat->name, seat)))
          continue;

        eseat->ptr->x = x;
        eseat->ptr->y = y;
        eseat->ptr->timestamp = ecore_loop_time_get();

        EINA_LIST_FOREACH(eseat->devices, ll, edev)
          {
             if (!libinput_device_has_capability(edev->device,
                                                 LIBINPUT_DEVICE_CAP_POINTER))
               continue;

             _evdev_pointer_motion_send(edev);
             break;
          }
     }
}

EAPI Eina_Bool
elput_input_pointer_left_handed_set(Elput_Manager *manager, const char *seat, Eina_Bool left)
{
   Elput_Seat *eseat;
   Elput_Device *edev;
   Eina_List *l, *ll;

   EINA_SAFETY_ON_NULL_RETURN_VAL(manager, EINA_FALSE);

   /* if no seat name is passed in, just use default seat name */
   if (!seat) seat = "seat0";

   EINA_LIST_FOREACH(manager->input.seats, l, eseat)
     {
        if ((eseat->name) && (strcmp(eseat->name, seat)))
          continue;

        EINA_LIST_FOREACH(eseat->devices, ll, edev)
          {
             if (!libinput_device_has_capability(edev->device,
                                                 LIBINPUT_DEVICE_CAP_POINTER))
               continue;

             if (edev->left_handed == left) continue;

             if (libinput_device_config_left_handed_set(edev->device,
                                                        (int)left) !=
                 LIBINPUT_CONFIG_STATUS_SUCCESS)
               {
                  WRN("Failed to set left handed mode for device: %s",
                      libinput_device_get_name(edev->device));
                  continue;
               }
             else
               edev->left_handed = !!left;
          }
     }

   return EINA_TRUE;
}

EAPI const Eina_List *
elput_input_devices_get(Elput_Seat *seat)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(seat, NULL);
   return seat->devices;
}

EAPI void
elput_input_pointer_max_set(Elput_Manager *manager, int maxw, int maxh)
{
   Eina_List *l;
   Elput_Seat *eseat;

   EINA_SAFETY_ON_NULL_RETURN(manager);
   manager->input.pointer_w = maxw;
   manager->input.pointer_h = maxh;

   _elput_input_pointer_max_update(manager);
}
