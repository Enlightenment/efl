#include "elput_private.h"

static void
_seat_caps_update(Elput_Seat *seat)
{
   /* TODO: raise event for seat caps */
}

static int
_keyboard_fd_get(off_t size)
{
   int fd = 0, blen = 0, len = 0;
   const char *path;
   char tmp[PATH_MAX];
   long flags;

   blen = sizeof(tmp) - 1;

   if (!(path = getenv("XDG_RUNTIME_DIR")))
     return -1;

   len = strlen(path);
   if (len < blen)
     {
        strcpy(tmp, path);
        strcat(tmp, "/elput-keymap-XXXXXX");
     }
   else
     return -1;

   if ((fd = mkstemp(tmp)) < 0) return -1;

   flags = fcntl(fd, F_GETFD);
   if (flags < 0)
     {
        close(fd);
        return -1;
     }

   if (fcntl(fd, F_SETFD, (flags | FD_CLOEXEC)) == -1)
     {
        close(fd);
        return -1;
     }

   if (ftruncate(fd, size) < 0)
     {
        close(fd);
        return -1;
     }

   unlink(tmp);
   return fd;
}

static Elput_Keyboard_Info *
_keyboard_info_create(struct xkb_keymap *keymap, Eina_Bool external)
{
   Elput_Keyboard_Info *info;
   char *str;

   info = calloc(1, sizeof(Elput_Keyboard_Info));
   if (!info) return NULL;

   info->keymap.map = xkb_keymap_ref(keymap);
   info->refs = 1;

   info->mods.super =
     1 << xkb_keymap_mod_get_index(info->keymap.map, XKB_MOD_NAME_LOGO);
   info->mods.shift =
     1 << xkb_keymap_mod_get_index(info->keymap.map, XKB_MOD_NAME_SHIFT);
   info->mods.caps =
     1 << xkb_keymap_mod_get_index(info->keymap.map, XKB_MOD_NAME_CAPS);
   info->mods.ctrl =
     1 << xkb_keymap_mod_get_index(info->keymap.map, XKB_MOD_NAME_CTRL);
   info->mods.alt =
     1 << xkb_keymap_mod_get_index(info->keymap.map, XKB_MOD_NAME_ALT);
   info->mods.altgr =
     1 << xkb_keymap_mod_get_index(info->keymap.map, "ISO_Level3_Shift");

   /* if we are using an external keymap then we do not need go further */
   if (external) return info;

   str = xkb_keymap_get_as_string(info->keymap.map, XKB_KEYMAP_FORMAT_TEXT_V1);
   if (!str) goto err;

   info->keymap.size = strlen(str) + 1;

   info->keymap.fd = _keyboard_fd_get(info->keymap.size);
   if (info->keymap.fd < 0) goto err_fd;

   info->keymap.area =
     mmap(NULL, info->keymap.size, PROT_READ | PROT_WRITE,
          MAP_SHARED, info->keymap.fd, 0);
   if (info->keymap.area == MAP_FAILED) goto err_map;

   strcpy(info->keymap.area, str);
   free(str);

   return info;

err_map:
   close(info->keymap.fd);
err_fd:
   free(str);
err:
   xkb_keymap_unref(info->keymap.map);
   free(info);
   return NULL;
}

static void
_keyboard_info_destroy(Elput_Keyboard_Info *info, Eina_Bool external)
{
   if (--info->refs > 0) return;

   if (!external)
     {
        xkb_keymap_unref(info->keymap.map);
        if (info->keymap.area) munmap(info->keymap.area, info->keymap.size);
        if (info->keymap.fd >= 0) close(info->keymap.fd);
     }

   free(info);
}

static Eina_Bool
_keyboard_global_build(Elput_Keyboard *kbd)
{
   struct xkb_keymap *keymap;

   kbd->context = xkb_context_new(0);
   if (!kbd->context) return EINA_FALSE;

   if (!kbd->names.rules) kbd->names.rules = strdup("evdev");
   if (!kbd->names.model) kbd->names.model = strdup("pc105");
   if (!kbd->names.layout) kbd->names.layout = strdup("us");

   keymap = xkb_keymap_new_from_names(kbd->context, &kbd->names, 0);
   if (!keymap) return EINA_FALSE;

   kbd->info = _keyboard_info_create(keymap, EINA_FALSE);
   xkb_keymap_unref(keymap);

   if (!kbd->info) return EINA_FALSE;
   return EINA_TRUE;
}

static Elput_Keyboard *
_keyboard_create(Elput_Seat *seat)
{
   Elput_Keyboard *kbd;

   kbd = calloc(1, sizeof(Elput_Keyboard));
   if (!kbd) return NULL;

   kbd->seat = seat;
   return kbd;
}

static Eina_Bool
_keyboard_init(Elput_Seat *seat, struct xkb_keymap *keymap)
{
   Elput_Keyboard *kbd;

   if (seat->kbd)
     {
        seat->count.kbd += 1;
        if (seat->count.kbd == 1)
          {
             _seat_caps_update(seat);
             return EINA_TRUE;
          }
     }

   kbd = _keyboard_create(seat);
   if (!kbd) return EINA_FALSE;

   if (keymap)
     {
        kbd->info = _keyboard_info_create(keymap, EINA_TRUE);
        if (!kbd->info) goto err;
     }
   else
     {
        if (!_keyboard_global_build(kbd)) goto err;
        kbd->info->refs++;
     }

   kbd->state = xkb_state_new(kbd->info->keymap.map);
   if (!kbd->state) goto err;

   seat->kbd = kbd;
   seat->count.kbd = 1;

   _seat_caps_update(seat);

   return EINA_TRUE;

err:
   if (kbd->info) _keyboard_info_destroy(kbd->info, kbd->external_map);
   free(kbd);
   return EINA_FALSE;
}

static void
_keyboard_state_reset(Elput_Keyboard *kbd)
{
   struct xkb_state *state;

   state = xkb_state_new(kbd->info->keymap.map);
   if (!state) return;

   xkb_state_unref(kbd->state);
   kbd->state = state;
}

static void
_keyboard_release(Elput_Seat *seat)
{
   seat->count.kbd--;
   if (seat->count.kbd == 0)
     {
        _keyboard_state_reset(seat->kbd);
        _seat_caps_update(seat);
     }
}

static Elput_Pointer *
_pointer_create(Elput_Seat *seat)
{
   Elput_Pointer *ptr;

   ptr = calloc(1, sizeof(Elput_Pointer));
   if (!ptr) return NULL;

   ptr->seat = seat;

   return ptr;
}

static Eina_Bool
_pointer_init(Elput_Seat *seat)
{
   Elput_Pointer *ptr;

   if (seat->ptr)
     {
        seat->count.ptr += 1;
        if (seat->count.ptr == 1)
          {
             _seat_caps_update(seat);
             return EINA_TRUE;
          }
     }

   ptr = _pointer_create(seat);
   if (!ptr) return EINA_FALSE;

   seat->ptr = ptr;
   seat->count.ptr = 1;

   _seat_caps_update(seat);

   return EINA_TRUE;
}

static void
_pointer_release(Elput_Seat *seat)
{
   seat->count.ptr--;
   if (seat->count.ptr == 0)
     {
        seat->ptr->buttons = 0;
        _seat_caps_update(seat);
     }
}

static Elput_Touch *
_touch_create(Elput_Seat *seat)
{
   Elput_Touch *touch;

   touch = calloc(1, sizeof(Elput_Touch));
   if (!touch) return NULL;

   touch->seat = seat;

   return touch;
}

static Eina_Bool
_touch_init(Elput_Seat *seat)
{
   Elput_Touch *touch;

   if (seat->touch)
     {
        seat->count.touch += 1;
        if (seat->count.touch == 1)
          {
             _seat_caps_update(seat);
             return EINA_TRUE;
          }
     }

   touch = _touch_create(seat);
   if (!touch) return EINA_FALSE;

   seat->touch = touch;
   seat->count.touch = 1;

   _seat_caps_update(seat);

   return EINA_TRUE;
}

static void
_touch_release(Elput_Seat *seat)
{
   seat->count.touch--;
   if (seat->count.touch == 0)
     {
        seat->touch->points = 0;
        _seat_caps_update(seat);
     }
}

int
_evdev_event_process(struct libinput_event *event)
{
   struct libinput_device *idev;
   int ret = 1;
   Eina_Bool frame = EINA_FALSE;

   idev = libinput_event_get_device(event);
   switch (libinput_event_get_type(event))
     {
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
      default:
        ret = 0;
        break;
     }

   if (frame)
     {

     }

   return ret;
}

Elput_Device *
_evdev_device_create(Elput_Seat *seat, struct libinput_device *device)
{
   Elput_Device *edev;

   edev = calloc(1, sizeof(Elput_Device));
   if (!edev) return NULL;

   edev->seat = seat;
   edev->device = device;

   if (libinput_device_has_capability(device, LIBINPUT_DEVICE_CAP_KEYBOARD))
     {
        _keyboard_init(seat, NULL);
        edev->caps |= EVDEV_SEAT_KEYBOARD;
     }
 
   if (libinput_device_has_capability(device, LIBINPUT_DEVICE_CAP_POINTER))
     {
        _pointer_init(seat);
        edev->caps |= EVDEV_SEAT_POINTER;
     }

   if (libinput_device_has_capability(device, LIBINPUT_DEVICE_CAP_TOUCH))
     {
        _touch_init(seat);
        edev->caps |= EVDEV_SEAT_TOUCH;
     }

   libinput_device_set_user_data(device, edev);
   libinput_device_ref(edev->device);

   if (libinput_device_config_tap_get_finger_count(edev->device) > 0)
     {
        Eina_Bool enable = EINA_FALSE;

        enable = libinput_device_config_tap_get_default_enabled(edev->device);
        libinput_device_config_tap_set_enabled(edev->device, enable);
     }

   /* FIXME: Normally we would do a device calibration set here however
    * that requires Output support. Since this is just an input library, we
    * may need to add external facing APIs to do calibration. Then a user of
    * elput would handle outputs, and make calls to calibrate */

   return edev;
}

void
_evdev_device_destroy(Elput_Device *edev)
{
   if (!edev) return;

   if (edev->caps & EVDEV_SEAT_POINTER)
     _pointer_release(edev->seat);
   if (edev->caps & EVDEV_SEAT_KEYBOARD)
     _keyboard_release(edev->seat);
   if (edev->caps & EVDEV_SEAT_TOUCH)
     _touch_release(edev->seat);

   libinput_device_unref(edev->device);
   eina_stringshare_del(edev->output_name);
   free(edev);
}

void
_evdev_keyboard_destroy(Elput_Keyboard *kbd)
{
   free((char *)kbd->names.rules);
   free((char *)kbd->names.model);
   free((char *)kbd->names.layout);
   free((char *)kbd->names.variant);
   free((char *)kbd->names.options);

   if (kbd->state) xkb_state_unref(kbd->state);
   if (kbd->info) _keyboard_info_destroy(kbd->info, kbd->external_map);

   xkb_context_unref(kbd->context);
   xkb_keymap_unref(kbd->pending_map);

   free(kbd);
}

void
_evdev_pointer_destroy(Elput_Pointer *ptr)
{
   /* FIXME: destroy any resources inside pointer structure */
   free(ptr);
}

void
_evdev_touch_destroy(Elput_Touch *touch)
{
   /* FIXME: destroy any resources inside touch structure */
   free(touch);
}
