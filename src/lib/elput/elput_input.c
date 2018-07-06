/* this file contains code copied from weston; the copyright notice is below */
/*
 * Copyright © 2013 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "elput_private.h"
#include <libudev.h>

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
   if (ecore_thread_check(em->input.thread)) return ret;
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
             if (read(p[0], &fd, sizeof(int)) < 1)
               ret = -1;
             else
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

   eseat->manager = em;
   eseat->refs = 1;

   eseat->name = eina_stringshare_add(name);
   em->input.seats = eina_list_append(em->input.seats, eseat);

   return eseat;
}

void
_udev_seat_destroy(Elput_Seat *eseat)
{
   Elput_Device *edev;

   eseat->refs--;
   if (eseat->refs) return;

   EINA_LIST_FREE(eseat->devices, edev)
     _evdev_device_destroy(edev);

   if (eseat->kbd) _evdev_keyboard_destroy(eseat->kbd);
   eseat->kbd = NULL;
   if (eseat->ptr) _evdev_pointer_destroy(eseat->ptr);
   eseat->ptr = NULL;
   if (eseat->touch) _evdev_touch_destroy(eseat->touch);
   eseat->touch = NULL;
   if (eseat->manager->input.seats)
     eseat->manager->input.seats = eina_list_remove(eseat->manager->input.seats, eseat);
   if (eseat->refs) return;

   eina_stringshare_del(eseat->name);
   free(eseat);
}

static Elput_Seat *
_udev_seat_named_get(Elput_Manager *em, const char *name)
{
   Elput_Seat *eseat;
   Eina_List *l;

   if (!name) name = "seat0";

   EINA_LIST_FOREACH(em->input.seats, l, eseat)
     if (!strcmp(eseat->name, name)) return eseat;

   eseat = _udev_seat_create(em, name);
   if (!eseat) return NULL;

   return eseat;
}

static Elput_Seat *
_udev_seat_get(Elput_Manager *em, struct libinput_device *device)
{
   struct libinput_seat *lseat;
   const char *name;

   lseat = libinput_device_get_seat(device);
   name = libinput_seat_get_physical_name(lseat);

   return _udev_seat_named_get(em, name);
}

static void
_device_event_cb_free(void *data EINA_UNUSED, void *event)
{
   Elput_Event_Device_Change *ev;

   ev = event;

   ev->device->refs--;
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
   edev->refs++;

   ecore_event_add(ELPUT_EVENT_DEVICE_CHANGE, ev, _device_event_cb_free, NULL);
}

static void
_device_add(Elput_Manager *em, struct libinput_device *dev)
{
   Elput_Seat *eseat;
   Elput_Device *edev;

   eseat = _udev_seat_get(em, dev);
   if (!eseat) return;

   edev = _evdev_device_create(eseat, dev);
   if (!edev) return;

   eseat->devices = eina_list_append(eseat->devices, edev);

   DBG("Input Device Added: %s", libinput_device_get_name(dev));

   if (edev->caps & ELPUT_DEVICE_CAPS_KEYBOARD)
     DBG("\tDevice added as Keyboard device");

   if (edev->caps & ELPUT_DEVICE_CAPS_POINTER)
     {
        DBG("\tDevice added as Pointer device");
        switch (em->input.rotation)
          {
           case 0:
             edev->swap = EINA_FALSE;
             edev->invert_x = EINA_FALSE;
             edev->invert_y = EINA_FALSE;
             break;
           case 90:
             edev->swap = EINA_TRUE;
             edev->invert_x = EINA_FALSE;
             edev->invert_y = EINA_TRUE;
             break;
           case 180:
             edev->swap = EINA_FALSE;
             edev->invert_x = EINA_TRUE;
             edev->invert_y = EINA_TRUE;
             break;
           case 270:
             edev->swap = EINA_TRUE;
             edev->invert_x = EINA_TRUE;
             edev->invert_y = EINA_FALSE;
             break;
           default:
             break;
          }
     }

   if (edev->caps & ELPUT_DEVICE_CAPS_TOUCH)
     {
        DBG("\tDevice added as Touch device");
     }

   _device_event_send(edev, ELPUT_DEVICE_ADDED);
}

static void
_device_remove(Elput_Manager *em EINA_UNUSED, struct libinput_device *device)
{
   Elput_Device *edev;

   edev = libinput_device_get_user_data(device);
   if (!edev) return;

   DBG("Input Device Removed: %s", libinput_device_get_name(device));

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
        _device_add(em, dev);
        break;
      case LIBINPUT_EVENT_DEVICE_REMOVED:
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

   while ((ei->lib) && (event = libinput_get_event(ei->lib)))
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

   if ((ei->lib) && (libinput_dispatch(ei->lib) != 0))
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
     ecore_main_fd_handler_add(libinput_get_fd(manager->input.lib),
                               ECORE_FD_READ, _cb_input_dispatch,
                               &manager->input, NULL, NULL);

   if (manager->input.hdlr)
      _process_events(&manager->input);
   else
     {
        ERR("Could not create input fd handler");
        libinput_unref(manager->input.lib);
        manager->input.lib = NULL;
     }

   if ((manager->pending_ptr_x) || (manager->pending_ptr_y))
     {
        elput_input_pointer_xy_set(manager, NULL, manager->pending_ptr_x,
                                   manager->pending_ptr_y);
        manager->pending_ptr_x = 0;
        manager->pending_ptr_y = 0;
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
   struct udev *udev;

   udev = udev_new();

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

void
_elput_input_enable(Elput_Manager *manager)
{
   if (!manager->input.hdlr)
     {
        manager->input.hdlr =
          ecore_main_fd_handler_add(libinput_get_fd(manager->input.lib),
                                    ECORE_FD_READ, _cb_input_dispatch,
                                    &manager->input, NULL, NULL);
     }

   if (manager->input.suspended)
     {
        if (libinput_resume(manager->input.lib) != 0) return;
        manager->input.suspended = EINA_FALSE;
        _process_events(&manager->input);
     }
}

void
_elput_input_disable(Elput_Manager *manager)
{
   Elput_Seat *seat;
   Eina_List *l;

   EINA_LIST_FOREACH(manager->input.seats, l, seat)
     seat->pending_motion = 1;
   if (manager->input.lib) libinput_suspend(manager->input.lib);
   _process_events(&manager->input);
   manager->input.suspended = EINA_TRUE;
}

EAPI Eina_Bool
elput_input_init(Elput_Manager *manager)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(manager, EINA_FALSE);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(!!manager->input.hdlr, EINA_TRUE);

   memset(&manager->input, 0, sizeof(Elput_Input));
   manager->input.thread =
     ecore_thread_feedback_run(_elput_input_init_thread,
                               _elput_input_init_notify,
                               _elput_input_init_end,
                               _elput_input_init_cancel, manager, 1);
   return !!manager->input.thread;
}

EAPI void
elput_input_shutdown(Elput_Manager *manager)
{
   Elput_Seat *seat;
   Eina_List *l, *ll;

   EINA_SAFETY_ON_NULL_RETURN(manager);

   ecore_main_fd_handler_del(manager->input.hdlr);

   EINA_LIST_FOREACH_SAFE(manager->input.seats, l, ll, seat)
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
        if (!eina_streq(eseat->name, seat)) continue;
        if (x) *x = eseat->pointer.x;
        if (y) *y = eseat->pointer.y;
        return;
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

   if (eina_list_count(manager->input.seats) < 1)
     {
        manager->pending_ptr_x = x;
        manager->pending_ptr_y = y;
        return;
     }

   EINA_LIST_FOREACH(manager->input.seats, l, eseat)
     {
        if (!eseat->ptr) continue;
        if ((eseat->name) && (strcmp(eseat->name, seat)))
          continue;

        eseat->pointer.x = x;
        eseat->pointer.y = y;
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

EAPI void
elput_input_pointer_max_set(Elput_Manager *manager, int maxw, int maxh)
{
   EINA_SAFETY_ON_NULL_RETURN(manager);
   manager->input.pointer_w = maxw;
   manager->input.pointer_h = maxh;
}

EAPI Eina_Bool
elput_input_pointer_rotation_set(Elput_Manager *manager, int rotation)
{
   Elput_Seat *eseat;
   Elput_Device *edev;
   Eina_List *l, *ll;

   EINA_SAFETY_ON_NULL_RETURN_VAL(manager, EINA_FALSE);

   if ((rotation % 90 != 0) || (rotation / 90 > 3) || (rotation < 0))
     return EINA_FALSE;

   manager->input.rotation = rotation;

   EINA_LIST_FOREACH(manager->input.seats, l, eseat)
     {
        EINA_LIST_FOREACH(eseat->devices, ll, edev)
          {
             if (!(edev->caps & ELPUT_DEVICE_CAPS_POINTER)) continue;

             switch (rotation)
               {
                case 0:
                  edev->swap = EINA_FALSE;
                  edev->invert_x = EINA_FALSE;
                  edev->invert_y = EINA_FALSE;
                  break;
                case 90:
                  edev->swap = EINA_TRUE;
                  edev->invert_x = EINA_FALSE;
                  edev->invert_y = EINA_TRUE;
                  break;
                case 180:
                  edev->swap = EINA_FALSE;
                  edev->invert_x = EINA_TRUE;
                  edev->invert_y = EINA_TRUE;
                  break;
                case 270:
                  edev->swap = EINA_TRUE;
                  edev->invert_x = EINA_TRUE;
                  edev->invert_y = EINA_FALSE;
                  break;
                default:
                  break;
               }
          }
     }

   return EINA_TRUE;
}

EAPI void
elput_input_devices_calibrate(Elput_Manager *manager, int w, int h)
{
   Elput_Seat *eseat;
   Elput_Device *edev;
   Eina_List *l, *ll;

   EINA_SAFETY_ON_NULL_RETURN(manager);

   manager->output_w = w;
   manager->output_h = h;

   EINA_LIST_FOREACH(manager->input.seats, l, eseat)
     {
        EINA_LIST_FOREACH(eseat->devices, ll, edev)
          {
             edev->ow = w;
             edev->oh = h;
             _evdev_device_calibrate(edev);
          }
     }
}

EAPI Eina_Bool
elput_input_key_remap_enable(Elput_Manager *manager, Eina_Bool enable)
{
   Elput_Seat *eseat;
   Elput_Device *edev;
   Eina_List *l, *ll;

   EINA_SAFETY_ON_NULL_RETURN_VAL(manager, EINA_FALSE);

   EINA_LIST_FOREACH(manager->input.seats, l, eseat)
     {
        EINA_LIST_FOREACH(eseat->devices, ll, edev)
          {
             if (!(edev->caps & ELPUT_DEVICE_CAPS_KEYBOARD)) continue;

             edev->key_remap = enable;
             if ((!enable) && (edev->key_remap_hash))
               {
                  eina_hash_free(edev->key_remap_hash);
                  edev->key_remap_hash = NULL;
               }
          }
     }

   return EINA_TRUE;
}

EAPI Eina_Bool
elput_input_key_remap_set(Elput_Manager *manager, int *from_keys, int *to_keys, int num)
{
   Elput_Seat *eseat;
   Elput_Device *edev;
   Eina_List *l, *ll;
   int i = 0;

   EINA_SAFETY_ON_NULL_RETURN_VAL(manager, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(from_keys, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(to_keys, EINA_FALSE);
   EINA_SAFETY_ON_TRUE_RETURN_VAL((num <= 0), EINA_FALSE);

   EINA_LIST_FOREACH(manager->input.seats, l, eseat)
     {
        EINA_LIST_FOREACH(eseat->devices, ll, edev)
          {
             if (!(edev->caps & ELPUT_DEVICE_CAPS_KEYBOARD)) continue;

             if (!edev->key_remap) continue;
             if (!edev->key_remap_hash)
               edev->key_remap_hash = eina_hash_int32_new(NULL);
             if (!edev->key_remap_hash) continue;

             for (i = 0; i < num; i++)
               {
                  if ((!from_keys[i]) || (!to_keys[i]))
                    continue;
               }

             for (i = 0; i < num; i++)
               eina_hash_add(edev->key_remap_hash, &from_keys[i],
                             (void *)(intptr_t)to_keys[i]);
          }
     }

   return EINA_TRUE;
}

EAPI void
elput_input_keyboard_info_set(Elput_Manager *manager, void *context, void *keymap, int group)
{
   Eina_List *l;
   Elput_Seat *seat;

   EINA_SAFETY_ON_NULL_RETURN(manager);
   EINA_SAFETY_ON_FALSE_RETURN((!!context) == (!!keymap));

   if ((manager->cached.context == context) &&
       (manager->cached.keymap == keymap))
     return;
   if (context) xkb_context_ref(context);
   if (keymap) xkb_keymap_ref(keymap);
   if (manager->cached.context) xkb_context_unref(manager->cached.context);
   if (manager->cached.keymap) xkb_keymap_unref(manager->cached.keymap);
   manager->cached.context = context;
   manager->cached.keymap = keymap;
   manager->cached.group = group;
   EINA_LIST_FOREACH(manager->input.seats, l, seat)
     _keyboard_keymap_update(seat);
}

EAPI void
elput_input_keyboard_group_set(Elput_Manager *manager, int group)
{
   Eina_List *l;
   Elput_Seat *seat;
   EINA_SAFETY_ON_NULL_RETURN(manager);

   if (manager->cached.group == group) return;
   manager->cached.group = group;
   EINA_LIST_FOREACH(manager->input.seats, l, seat)
     _keyboard_group_update(seat);
}

EAPI void
elput_input_pointer_accel_profile_set(Elput_Manager *manager, const char *seat, uint32_t profile)
{
   Elput_Seat *eseat;
   Elput_Device *edev;
   Eina_List *l, *ll;

   EINA_SAFETY_ON_NULL_RETURN(manager);

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

             if (libinput_device_config_accel_set_profile(edev->device,
                                                          profile) !=
                 LIBINPUT_CONFIG_STATUS_SUCCESS)
               {
                  WRN("Failed to set acceleration profile for device: %s",
                      libinput_device_get_name(edev->device));
                  continue;
               }
          }
     }
}

EAPI void
elput_input_pointer_accel_speed_set(Elput_Manager *manager, const char *seat, double speed)
{
   Elput_Seat *eseat;
   Elput_Device *edev;
   Eina_List *l, *ll;

   EINA_SAFETY_ON_NULL_RETURN(manager);

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

             if (libinput_device_config_accel_set_speed(edev->device,
                                                        speed) !=
                 LIBINPUT_CONFIG_STATUS_SUCCESS)
               {
                  WRN("Failed to set acceleration speed for device: %s",
                      libinput_device_get_name(edev->device));
                  continue;
               }
          }
     }
}

EAPI Elput_Seat *
elput_device_seat_get(const Elput_Device *dev)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(dev, NULL);
   return dev->seat;
}

EAPI Elput_Device_Caps
elput_device_caps_get(const Elput_Device *dev)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(dev, 0);
   return dev->caps;
}

EAPI Eina_Stringshare *
elput_device_output_name_get(Elput_Device *device)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(device, NULL);

   return device->output_name;
}

EAPI const Eina_List *
elput_seat_devices_get(const Elput_Seat *seat)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(seat, NULL);
   return seat->devices;
}

EAPI Eina_Stringshare *
elput_seat_name_get(const Elput_Seat *seat)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(seat, NULL);
   return seat->name;
}

EAPI Elput_Manager *
elput_seat_manager_get(const Elput_Seat *seat)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(seat, NULL);
   return seat->manager;
}
