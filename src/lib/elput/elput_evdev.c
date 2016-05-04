#include "elput_private.h"

static void
_seat_caps_update(Elput_Seat *seat)
{
   Elput_Event_Seat_Caps *ev;

   ev = calloc(1, sizeof(Elput_Event_Seat_Caps));
   if (!ev) return;

   ev->pointer_count = seat->count.ptr;
   ev->keyboard_count = seat->count.kbd;
   ev->touch_count = seat->count.touch;
   ev->seat = seat;

   ecore_event_add(ELPUT_EVENT_SEAT_CAPS, ev, NULL, NULL);
}

static void
_seat_frame_send(Elput_Seat *seat)
{
   Elput_Event_Seat_Frame *ev;

   ev = calloc(1, sizeof(Elput_Event_Seat_Frame));
   if (!ev) return;

   ev->seat = seat;
   ecore_event_add(ELPUT_EVENT_SEAT_FRAME, ev, NULL, NULL);
}

static void
_keyboard_modifiers_update(Elput_Keyboard *kbd, Elput_Seat *seat)
{
   xkb_mod_mask_t mask;

   kbd->mods.depressed =
     xkb_state_serialize_mods(kbd->state, XKB_STATE_DEPRESSED);
   kbd->mods.latched =
     xkb_state_serialize_mods(kbd->state, XKB_STATE_LATCHED);
   kbd->mods.locked =
     xkb_state_serialize_mods(kbd->state, XKB_STATE_LOCKED);
   kbd->mods.group =
     xkb_state_serialize_mods(kbd->state, XKB_STATE_EFFECTIVE);

   mask = (kbd->mods.depressed | kbd->mods.latched);

   seat->modifiers = 0;
   if (mask & kbd->info->mods.ctrl)
     seat->modifiers |= ECORE_EVENT_MODIFIER_CTRL;
   if (mask & kbd->info->mods.alt)
     seat->modifiers |= ECORE_EVENT_MODIFIER_ALT;
   if (mask & kbd->info->mods.shift)
     seat->modifiers |= ECORE_EVENT_MODIFIER_SHIFT;
   if (mask & kbd->info->mods.super)
     seat->modifiers |= ECORE_EVENT_MODIFIER_WIN;
   if (mask & kbd->info->mods.altgr)
     seat->modifiers |= ECORE_EVENT_MODIFIER_ALTGR;
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

static void
_keyboard_key_send(Elput_Device *dev, enum libinput_key_state state, const char *keyname, const char *key, const char *compose, unsigned int code, unsigned int timestamp)
{
   Ecore_Event_Key *ev;

   ev = calloc(1, sizeof(Ecore_Event_Key) + strlen(key) + strlen(keyname) + 
               ((compose[0] != '\0') ? strlen(compose) : 0) + 3);
   if (!ev) return;

   ev->keyname = (char *)(ev + 1);
   ev->key = ev->keyname + strlen(keyname) + 1;
   ev->compose = strlen(compose) ? ev->key + strlen(key) + 1 : NULL;
   ev->string = ev->compose;

   strcpy((char *)ev->keyname, keyname);
   strcpy((char *)ev->key, key);
   if (strlen(compose)) strcpy((char *)ev->compose, compose);

   ev->keycode = code;
   ev->modifiers = dev->seat->modifiers;
   ev->timestamp = timestamp;
   ev->same_screen = 1;

   ev->window = dev->window;
   ev->event_window = dev->window;
   ev->root_window = dev->window;

   if (state == LIBINPUT_KEY_STATE_PRESSED)
     ecore_event_add(ECORE_EVENT_KEY_DOWN, ev, NULL, NULL);
   else
     ecore_event_add(ECORE_EVENT_KEY_UP, ev, NULL, NULL);
}

static void
_keyboard_keymap_send(Elput_Keyboard_Info *info)
{
   Elput_Event_Keymap_Send *ev;

   ev = calloc(1, sizeof(Elput_Event_Keymap_Send));
   if (!ev) return;

   ev->fd = info->keymap.fd;
   ev->size = info->keymap.size;
   ev->format = XKB_KEYMAP_FORMAT_TEXT_V1;

   ecore_event_add(ELPUT_EVENT_KEYMAP_SEND, ev, NULL, NULL);
}

static void
_keyboard_modifiers_send(Elput_Keyboard *kbd)
{
   Elput_Event_Modifiers_Send *ev;

   ev = calloc(1, sizeof(Elput_Event_Modifiers_Send));
   if (!ev) return;

   ev->depressed = kbd->mods.depressed;
   ev->latched = kbd->mods.latched;
   ev->locked = kbd->mods.locked;
   ev->group = kbd->mods.group;

   ecore_event_add(ELPUT_EVENT_MODIFIERS_SEND, ev, NULL, NULL);
}

static void
_keyboard_keymap_update(Elput_Seat *seat)
{
   Elput_Keyboard *kbd;
   Elput_Keyboard_Info *info;
   struct xkb_state *state;
   xkb_mod_mask_t latched, locked;

   kbd = _evdev_keyboard_get(seat);
   if (!kbd) return;

   info = _keyboard_info_create(kbd->pending_map, kbd->external_map);
   xkb_keymap_unref(kbd->pending_map);
   kbd->pending_map = NULL;

   if (!info) return;

   state = xkb_state_new(info->keymap.map);
   if (!state)
     {
        _keyboard_info_destroy(info, kbd->external_map);
        return;
     }

   latched = xkb_state_serialize_mods(kbd->state, XKB_STATE_MODS_LATCHED);
   locked = xkb_state_serialize_mods(kbd->state, XKB_STATE_MODS_LOCKED);
   xkb_state_update_mask(state, 0, latched, locked, 0, 0, 0);

   _keyboard_info_destroy(kbd->info, kbd->external_map);
   kbd->info = info;

   xkb_state_unref(kbd->state);
   kbd->state = state;

   _keyboard_modifiers_update(kbd, seat);
   _keyboard_keymap_send(kbd->info);

   if ((!latched) && (!locked)) return;

   _keyboard_modifiers_send(kbd);
}

static int
_keyboard_keysym_translate(xkb_keysym_t keysym, unsigned int modifiers, char *buffer, int bytes)
{
   unsigned long hbytes = 0;
   unsigned char c;

   if (!keysym) return 0;
   hbytes = (keysym >> 8);

   if (!(bytes &&
         ((hbytes == 0) ||
          ((hbytes == 0xFF) &&
           (((keysym >= XKB_KEY_BackSpace) && (keysym <= XKB_KEY_Clear)) ||
            (keysym == XKB_KEY_Return) || (keysym == XKB_KEY_Escape) ||
            (keysym == XKB_KEY_KP_Space) || (keysym == XKB_KEY_KP_Tab) ||
            (keysym == XKB_KEY_KP_Enter) ||
            ((keysym >= XKB_KEY_KP_Multiply) && (keysym <= XKB_KEY_KP_9)) ||
            (keysym == XKB_KEY_KP_Equal) || (keysym == XKB_KEY_Delete))))))
     return 0;

   if (keysym == XKB_KEY_KP_Space)
     c = (XKB_KEY_space & 0x7F);
   else if (hbytes == 0xFF)
     c = (keysym & 0x7F);
   else
     c = (keysym & 0xFF);

   if (modifiers & ECORE_EVENT_MODIFIER_CTRL)
     {
        if (((c >= '@') && (c < '\177')) || c == ' ')
          c &= 0x1F;
        else if (c == '2')
          c = '\000';
        else if ((c >= '3') && (c <= '7'))
          c -= ('3' - '\033');
        else if (c == '8')
          c = '\177';
        else if (c == '/')
          c = '_' & 0x1F;
     }
   buffer[0] = c;
   return 1;
}

static void
_keyboard_key(struct libinput_device *idevice, struct libinput_event_keyboard *event)
{
   Elput_Device *dev;
   Elput_Keyboard *kbd;
   enum libinput_key_state state;
   xkb_keysym_t sym = XKB_KEY_NoSymbol;
   const xkb_keysym_t *syms;
   unsigned int code = 0;
   unsigned int nsyms;
   unsigned int timestamp;
   char key[256], keyname[256], buffer[256];
   char *tmp = NULL, *compose = NULL;
   int count;

   dev = libinput_device_get_user_data(idevice);
   if (!dev) return;

   kbd = _evdev_keyboard_get(dev->seat);
   if (!kbd) return;

   state = libinput_event_keyboard_get_key_state(event);
   count = libinput_event_keyboard_get_seat_key_count(event);

   /* Ignore key events that are not seat wide state changes. */
   if (((state == LIBINPUT_KEY_STATE_PRESSED) && (count != 1)) ||
       ((state == LIBINPUT_KEY_STATE_RELEASED) && (count != 0)))
     return;

   code = libinput_event_keyboard_get_key(event) + 8;
   timestamp = libinput_event_keyboard_get_time(event);

   if (state == LIBINPUT_KEY_STATE_PRESSED)
     xkb_state_update_key(kbd->state, code, XKB_KEY_DOWN);
   else
     xkb_state_update_key(kbd->state, code, XKB_KEY_UP);

   nsyms = xkb_key_get_syms(kbd->state, code, &syms);
   if (nsyms == 1) sym = syms[0];

   memset(key, 0, sizeof(key));
   xkb_keysym_get_name(sym, key, sizeof(key));

   memset(keyname, 0, sizeof(keyname));
   memcpy(keyname, key, sizeof(keyname));

   if (keyname[0] == '\0')
     snprintf(keyname, sizeof(keyname), "Keycode-%u", code);

   if (xkb_state_mod_index_is_active(kbd->state, kbd->info->mods.shift,
                                     XKB_STATE_MODS_EFFECTIVE))
     {
        if (keyname[0] != '\0')
          keyname[0] = tolower(keyname[0]);
     }

   _keyboard_modifiers_update(kbd, dev->seat);

   memset(buffer, 0, sizeof(buffer));
   if (_keyboard_keysym_translate(sym, dev->seat->modifiers, 
                                  buffer, sizeof(buffer)))
     {
        compose = eina_str_convert("ISO8859-1", "UTF-8", buffer);
        if (!compose)
          {
             ERR("Elput cannot convert input key string '%s' to UTF-8. "
                 "Is Eina built with iconv support?", buffer);
          }
        else
          tmp = compose;
     }

   if (!compose) compose = buffer;

   _keyboard_key_send(dev, state, keyname, key, compose, code, timestamp);

   if (tmp) free(tmp);

   if ((kbd->pending_map) && (count == 0))
     _keyboard_keymap_update(dev->seat);

   if (state == LIBINPUT_KEY_STATE_PRESSED)
     {
        kbd->grab.key = code;
        kbd->grab.timestamp = timestamp;
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

static void
_pointer_motion_send(Elput_Device *edev)
{
   Elput_Pointer *ptr;
   Elput_Keyboard *kbd;
   Elput_Touch *touch;
   Ecore_Event_Mouse_Move *ev;
   double x, y;

   ptr = _evdev_pointer_get(edev->seat);
   if (!ptr) return;

   ev = calloc(1, sizeof(Ecore_Event_Mouse_Move));
   if (!ev) return;

   x = ptr->x;
   y = ptr->y;

   if (x < ptr->minx)
     x = ptr->minx;
   else if (x >= ptr->minx + ptr->maxw)
     x = ptr->minx + ptr->maxw - 1;

   if (y < ptr->miny)
     y = ptr->miny;
   else if (y >= ptr->miny + ptr->maxh)
     y = ptr->miny + ptr->maxh - 1;

   ev->window = edev->window;
   ev->event_window = edev->window;
   ev->root_window = edev->window;
   ev->timestamp = ptr->timestamp;
   ev->same_screen = 1;

   ev->x = x;
   ev->y = y;
   ev->root.x = x;
   ev->root.y = y;

   kbd = _evdev_keyboard_get(edev->seat);
   if (kbd) _keyboard_modifiers_update(kbd, edev->seat);

   ev->modifiers = edev->seat->modifiers;

   touch = _evdev_touch_get(edev->seat);
   if (touch) ev->multi.device = touch->slot;

   ev->multi.radius = 1;
   ev->multi.radius_x = 1;
   ev->multi.radius_y = 1;
   ev->multi.pressure = 1.0;
   ev->multi.angle = 0.0;
   ev->multi.x = ev->x;
   ev->multi.y = ev->y;
   ev->multi.root.x = ev->x;
   ev->multi.root.y = ev->y;

   ecore_event_add(ECORE_EVENT_MOUSE_MOVE, ev, NULL, NULL);
}

static Eina_Bool
_pointer_motion(struct libinput_device *idev, struct libinput_event_pointer *event)
{
   Elput_Device *edev;
   Elput_Pointer *ptr;
   double x, y;

   edev = libinput_device_get_user_data(idev);
   if (!edev) return EINA_FALSE;

   ptr = _evdev_pointer_get(edev->seat);
   if (!ptr) return EINA_FALSE;

   x = ptr->x + libinput_event_pointer_get_dx(event);
   y = ptr->y + libinput_event_pointer_get_dy(event);

   if (x < ptr->minx)
     x = ptr->minx;
   else if (x >= ptr->minx + ptr->maxw)
     x = ptr->minx + ptr->maxw - 1;

   if (y < ptr->miny)
     y = ptr->miny;
   else if (y >= ptr->miny + ptr->maxh)
     y = ptr->miny + ptr->maxh - 1;

   ptr->x = x;
   ptr->y = y;
   ptr->timestamp = libinput_event_pointer_get_time(event);

   _pointer_motion_send(edev);

   return EINA_TRUE;
}

static Eina_Bool
_pointer_motion_abs(struct libinput_device *idev, struct libinput_event_pointer *event)
{
   Elput_Device *edev;
   Elput_Pointer *ptr;

   edev = libinput_device_get_user_data(idev);
   if (!edev) return EINA_FALSE;

   ptr = _evdev_pointer_get(edev->seat);
   if (!ptr) return EINA_FALSE;

   ptr->x = libinput_event_pointer_get_absolute_x_transformed(event, edev->ow);
   ptr->y = libinput_event_pointer_get_absolute_y_transformed(event, edev->oh);
   ptr->timestamp = libinput_event_pointer_get_time(event);

   /* TODO: these needs to run a matrix transform based on output */

   _pointer_motion_send(edev);

   return EINA_TRUE;
}

static void
_pointer_button_send(Elput_Device *edev, enum libinput_button_state state)
{
   Elput_Pointer *ptr;
   Elput_Keyboard *kbd;
   Elput_Touch *touch;
   Ecore_Event_Mouse_Button *ev;

   ptr = _evdev_pointer_get(edev->seat);
   if (!ptr) return;

   ev = calloc(1, sizeof(Ecore_Event_Mouse_Button));
   if (!ev) return;

   ev->window = edev->window;
   ev->event_window = edev->window;
   ev->root_window = edev->window;
   ev->timestamp = ptr->timestamp;
   ev->same_screen = 1;

   ev->x = ptr->x;
   ev->y = ptr->y;
   ev->root.x = ptr->x;
   ev->root.y = ptr->y;

   touch = _evdev_touch_get(edev->seat);
   if (touch) ev->multi.device = touch->slot;
   ev->multi.radius = 1;
   ev->multi.radius_x = 1;
   ev->multi.radius_y = 1;
   ev->multi.pressure = 1.0;
   ev->multi.angle = 0.0;
   ev->multi.x = ev->x;
   ev->multi.y = ev->y;
   ev->multi.root.x = ev->x;
   ev->multi.root.y = ev->y;

   ev->buttons = ptr->buttons;

   ev->double_click = ptr->mouse.double_click;
   ev->triple_click = ptr->mouse.triple_click;

   kbd = _evdev_keyboard_get(edev->seat);
   if (kbd)
     _keyboard_modifiers_update(kbd, edev->seat);
   ev->modifiers = edev->seat->modifiers;

   if (state)
     ecore_event_add(ECORE_EVENT_MOUSE_BUTTON_DOWN, ev, NULL, NULL);
   else
     ecore_event_add(ECORE_EVENT_MOUSE_BUTTON_UP, ev, NULL, NULL);
}

static Eina_Bool
_pointer_button(struct libinput_device *idev, struct libinput_event_pointer *event)
{
   Elput_Device *edev;
   Elput_Pointer *ptr;
   int count;
   enum libinput_button_state state;
   unsigned int btn;

   edev = libinput_device_get_user_data(idev);
   if (!edev) return EINA_FALSE;

   ptr = _evdev_pointer_get(edev->seat);
   if (!ptr) return EINA_FALSE;

   state = libinput_event_pointer_get_button_state(event);
   count = libinput_event_pointer_get_seat_button_count(event);

   /* Ignore button events that are not seat wide state changes. */
   if (((state == LIBINPUT_BUTTON_STATE_PRESSED) && (count != 1)) ||
       ((state == LIBINPUT_BUTTON_STATE_RELEASED) && (count != 0)))
     return EINA_FALSE;

   btn = libinput_event_pointer_get_button(event);

   btn = ((btn & 0x00F) + 1);
   if (btn == 3) btn = 2;
   else if (btn == 2) btn = 3;

   ptr->buttons = btn;
   ptr->timestamp = libinput_event_pointer_get_time(event);

   if (state)
     {
        unsigned int current;

        current = ptr->timestamp;
        ptr->mouse.double_click = EINA_FALSE;
        ptr->mouse.triple_click = EINA_FALSE;

        if (((current - ptr->mouse.prev_time) <= ptr->mouse.threshold) &&
            (btn == ptr->mouse.prev_button))
          {
             ptr->mouse.double_click = EINA_TRUE;
             if (((current - ptr->mouse.last_time) <=
                  (2 * ptr->mouse.threshold)) &&
                 (btn == ptr->mouse.last_button))
               {
                  ptr->mouse.triple_click = EINA_TRUE;
                  ptr->mouse.prev_time = 0;
                  ptr->mouse.last_time = 0;
                  current = 0;
               }
          }

        ptr->mouse.last_time = ptr->mouse.prev_time;
        ptr->mouse.prev_time = current;
        ptr->mouse.last_button = ptr->mouse.prev_button;
        ptr->mouse.prev_button = ptr->buttons;
     }

   _pointer_button_send(edev, state);

   return EINA_TRUE;
}

static void
_pointer_axis_send(Elput_Device *dev, int direction, int value)
{
   Elput_Pointer *ptr;
   Elput_Keyboard *kbd;
   Ecore_Event_Mouse_Wheel *ev;

   ptr = _evdev_pointer_get(dev->seat);
   if (!ptr) return;

   ev = calloc(1, sizeof(Ecore_Event_Mouse_Wheel));
   if (!ev) return;

   ev->window = dev->window;
   ev->event_window = dev->window;
   ev->root_window = dev->window;
   ev->timestamp = ptr->timestamp;
   ev->same_screen = 1;

   ev->x = ptr->x;
   ev->y = ptr->y;
   ev->root.x = ptr->x;
   ev->root.y = ptr->y;

   ev->z = value;
   ev->direction = direction;

   kbd = _evdev_keyboard_get(dev->seat);
   if (kbd) _keyboard_modifiers_update(kbd, dev->seat);

   ev->modifiers = dev->seat->modifiers;

   ecore_event_add(ECORE_EVENT_MOUSE_WHEEL, ev, NULL, NULL);
}

static double
_pointer_axis_value(struct libinput_event_pointer *event, enum libinput_pointer_axis axis)
{
   enum libinput_pointer_axis_source source;
   double val = 0.0;

   source = libinput_event_pointer_get_axis_source(event);
   switch (source)
     {
      case LIBINPUT_POINTER_AXIS_SOURCE_WHEEL:
        val = 10 * libinput_event_pointer_get_axis_value_discrete(event, axis);
        break;
      case LIBINPUT_POINTER_AXIS_SOURCE_FINGER:
      case LIBINPUT_POINTER_AXIS_SOURCE_CONTINUOUS:
        val = libinput_event_pointer_get_axis_value(event, axis);
        break;
     }

   return val;
}

static Eina_Bool
_pointer_axis(struct libinput_device *idevice, struct libinput_event_pointer *event)
{
   Elput_Device *dev;
   Elput_Pointer *ptr;
   enum libinput_pointer_axis axis;
   Eina_Bool vert = EINA_FALSE, horiz = EINA_FALSE;
   int dir = 0, val = 0;

   dev = libinput_device_get_user_data(idevice);
   if (!dev) return EINA_FALSE;

   ptr = _evdev_pointer_get(dev->seat);
   if (!ptr) return EINA_FALSE;

#ifdef LIBINPUT_HIGHER_08
   vert =
     libinput_event_pointer_has_axis(event,
                                     LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL);
   horiz =
     libinput_event_pointer_has_axis(event,
                                     LIBINPUT_POINTER_AXIS_SCROLL_HORIZONTAL);
   if ((!vert) && (!horiz)) return EINA_FALSE;

   if (vert)
     {
        axis = LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL;
        val = _pointer_axis_value(event, axis);
     }

   if (horiz)
     {
        axis = LIBINPUT_POINTER_AXIS_SCROLL_HORIZONTAL;
        val = _pointer_axis_value(event, axis);
        dir = 1;
     }

#else
   axis = libinput_event_pointer_get_axis(event);
   if (axis == LIBINPUT_POINTER_AXIS_SCROLL_HORIZONTAL) dir = 1;
   val = libinput_event_pointer_get_axis_value(event);
#endif

   ptr->timestamp = libinput_event_pointer_get_time(event);

   _pointer_axis_send(dev, dir, val);

   return EINA_TRUE;
}

static void
_touch_event_send(Elput_Device *dev, struct libinput_event_touch *event, int type)
{
   Elput_Touch *touch;
   Ecore_Event_Mouse_Button *ev;
   unsigned int btn = 0;

   touch = _evdev_touch_get(dev->seat);
   if (!touch) return;

   ev = calloc(1, sizeof(Ecore_Event_Mouse_Button));
   if (!ev) return;

   ev->window = dev->window;
   ev->event_window = dev->window;
   ev->root_window = dev->window;
   ev->timestamp = libinput_event_touch_get_time(event);
   ev->same_screen = 1;

   ev->x = touch->x;
   ev->y = touch->y;
   ev->root.x = touch->x;
   ev->root.y = touch->y;

   ev->modifiers = dev->seat->modifiers;

   ev->multi.device = touch->slot;
   ev->multi.radius = 1;
   ev->multi.radius_x = 1;
   ev->multi.radius_y = 1;
   ev->multi.pressure = 1.0;
   ev->multi.angle = 0.0;
   ev->multi.x = ev->x;
   ev->multi.y = ev->y;
   ev->multi.root.x = ev->x;
   ev->multi.root.y = ev->y;

   btn = ((btn & 0x00F) + 1);
   if (btn == 3) btn = 2;
   else if (btn == 2) btn = 3;
   ev->buttons = btn;

   ecore_event_add(type, ev, NULL, NULL);
}

static void
_touch_down(struct libinput_device *idevice, struct libinput_event_touch *event)
{
   Elput_Device *dev;
   Elput_Touch *touch;
   unsigned int timestamp;
   int slot;

   dev = libinput_device_get_user_data(idevice);
   if (!dev) return;

   touch = _evdev_touch_get(dev->seat);
   if (!touch) return;

   slot = libinput_event_touch_get_seat_slot(event);
   timestamp = libinput_event_touch_get_time(event);

   touch->x = libinput_event_touch_get_x_transformed(event, dev->ow);
   touch->y = libinput_event_touch_get_y_transformed(event, dev->oh);

   /* TODO: these needs to run a matrix transform based on output */
   /* _ecore_drm2_output_coordinate_transform(dev->output, */
   /*                                         touch->x, touch->y, */
   /*                                         &touch->x, &touch->y); */

   if (slot == touch->grab.id)
     {
        touch->grab.x = touch->x;
        touch->grab.y = touch->y;
     }

   touch->slot = slot;
   touch->points++;

   _touch_event_send(dev, event, ECORE_EVENT_MOUSE_BUTTON_DOWN);

   if (touch->points == 1)
     {
        touch->grab.id = slot;
        touch->grab.x = touch->x;
        touch->grab.y = touch->y;
        touch->grab.timestamp = timestamp;
     }
}

static void
_touch_up(struct libinput_device *idevice, struct libinput_event_touch *event)
{
   Elput_Device *dev;
   Elput_Touch *touch;

   dev = libinput_device_get_user_data(idevice);
   if (!dev) return;

   touch = _evdev_touch_get(dev->seat);
   if (!touch) return;

   touch->points--;
   touch->slot = libinput_event_touch_get_seat_slot(event);

   _touch_event_send(dev, event, ECORE_EVENT_MOUSE_BUTTON_UP);
}

static void
_touch_motion_send(Elput_Device *dev, struct libinput_event_touch *event)
{
   Elput_Touch *touch;
   Ecore_Event_Mouse_Move *ev;

   touch = _evdev_touch_get(dev->seat);
   if (!touch) return;

   ev = calloc(1, sizeof(Ecore_Event_Mouse_Move));
   if (!ev) return;

   ev->window = dev->window;
   ev->event_window = dev->window;
   ev->root_window = dev->window;
   ev->timestamp = libinput_event_touch_get_time(event);
   ev->same_screen = 1;

   ev->x = touch->x;
   ev->y = touch->y;
   ev->root.x = touch->x;
   ev->root.y = touch->y;

   ev->modifiers = dev->seat->modifiers;

   ev->multi.device = touch->slot;
   ev->multi.radius = 1;
   ev->multi.radius_x = 1;
   ev->multi.radius_y = 1;
   ev->multi.pressure = 1.0;
   ev->multi.angle = 0.0;
   ev->multi.x = ev->x;
   ev->multi.y = ev->y;
   ev->multi.root.x = ev->x;
   ev->multi.root.y = ev->y;

   ecore_event_add(ECORE_EVENT_MOUSE_MOVE, ev, NULL, NULL);
}

static void
_touch_motion(struct libinput_device *idevice, struct libinput_event_touch *event)
{
   Elput_Device *dev;
   Elput_Touch *touch;

   dev = libinput_device_get_user_data(idevice);
   if (!dev) return;

   touch = _evdev_touch_get(dev->seat);
   if (!touch) return;

   touch->x = libinput_event_touch_get_x_transformed(event, dev->ow);
   touch->y = libinput_event_touch_get_y_transformed(event, dev->oh);

   /* TODO: these needs to run a matrix transform based on output */
   /* _ecore_drm2_output_coordinate_transform(dev->output, */
   /*                                         touch->x, touch->y, */
   /*                                         &touch->x, &touch->y); */

   touch->slot = libinput_event_touch_get_seat_slot(event);

   _touch_motion_send(dev, event);
}

static void
_evdev_device_calibrate(Elput_Device *dev)
{
   float cal[6];
   const char *vals;
   const char *sysname;
   const char *device;
   Eina_List *devices;
   int w = 0, h = 0;
   enum libinput_config_status status;

   w = dev->ow;
   h = dev->oh;
   if ((w == 0) || (h == 0)) return;

   if ((!libinput_device_config_calibration_has_matrix(dev->device)) ||
       (libinput_device_config_calibration_get_default_matrix(dev->device, cal)))
     return;

   sysname = libinput_device_get_sysname(dev->device);

   devices = eeze_udev_find_by_subsystem_sysname("input", sysname);
   EINA_LIST_FREE(devices, device)
     {
        vals = eeze_udev_syspath_get_property(device, "WL_CALIBRATION");
        if ((!vals) ||
            (sscanf(vals, "%f %f %f %f %f %f",
                    &cal[0], &cal[1], &cal[2], &cal[3], &cal[4], &cal[5]) != 6))
          goto cont;

        cal[2] /= w;
        cal[5] /= h;

        status =
          libinput_device_config_calibration_set_matrix(dev->device, cal);
        if (status != LIBINPUT_CONFIG_STATUS_SUCCESS)
          WRN("Failed to apply device calibration");

cont:
        eina_stringshare_del(device);
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
        _keyboard_key(idev, libinput_event_get_keyboard_event(event));
        break;
      case LIBINPUT_EVENT_POINTER_MOTION:
        frame =
          _pointer_motion(idev, libinput_event_get_pointer_event(event));
        break;
      case LIBINPUT_EVENT_POINTER_MOTION_ABSOLUTE:
        frame =
          _pointer_motion_abs(idev, libinput_event_get_pointer_event(event));
        break;
      case LIBINPUT_EVENT_POINTER_BUTTON:
        frame =
          _pointer_button(idev, libinput_event_get_pointer_event(event));
        break;
      case LIBINPUT_EVENT_POINTER_AXIS:
        frame =
          _pointer_axis(idev, libinput_event_get_pointer_event(event));
        break;
      case LIBINPUT_EVENT_TOUCH_DOWN:
        _touch_down(idev, libinput_event_get_touch_event(event));
        break;
      case LIBINPUT_EVENT_TOUCH_MOTION:
        _touch_motion(idev, libinput_event_get_touch_event(event));
        break;
      case LIBINPUT_EVENT_TOUCH_UP:
        _touch_up(idev, libinput_event_get_touch_event(event));
        break;
      case LIBINPUT_EVENT_TOUCH_FRAME:
      default:
        ret = 0;
        break;
     }

   if (frame)
     {
        Elput_Device *edev;

        edev = libinput_device_get_user_data(idev);
        if (edev) _seat_frame_send(edev->seat);
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

void
_evdev_pointer_motion_send(Elput_Device *edev)
{
   _pointer_motion_send(edev);
}

Elput_Pointer *
_evdev_pointer_get(Elput_Seat *seat)
{
   if (!seat) return NULL;
   if (seat->count.ptr) return seat->ptr;
   return NULL;
}

Elput_Keyboard *
_evdev_keyboard_get(Elput_Seat *seat)
{
   if (!seat) return NULL;
   if (seat->count.kbd) return seat->kbd;
   return NULL;
}

Elput_Touch *
_evdev_touch_get(Elput_Seat *seat)
{
   if (!seat) return NULL;
   if (seat->count.touch) return seat->touch;
   return NULL;
}

EAPI void
elput_device_window_set(Elput_Device *device, unsigned int window)
{
   EINA_SAFETY_ON_NULL_RETURN(device);

   device->window = window;
}

EAPI void
elput_device_output_size_set(Elput_Device *device, int w, int h)
{
   EINA_SAFETY_ON_NULL_RETURN(device);

   device->ow = w;
   device->oh = h;

   if (libinput_device_has_capability(device->device,
                                      LIBINPUT_DEVICE_CAP_POINTER))
     {
        Elput_Pointer *ptr;

        ptr = _evdev_pointer_get(device->seat);
        if (ptr)
          {
             ptr->x = device->ow / 2;
             ptr->y = device->oh / 2;
          }
     }

   _evdev_device_calibrate(device);
}
