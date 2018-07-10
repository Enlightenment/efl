#include "elput_private.h"

static void
_seat_event_free(void *d, void *ev)
{
   _udev_seat_destroy(d);
   free(ev);
}

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
   seat->refs++;

   ecore_event_add(ELPUT_EVENT_SEAT_CAPS, ev, _seat_event_free, seat);
}

static void
_seat_frame_send(Elput_Seat *seat)
{
   Elput_Event_Seat_Frame *ev;

   ev = calloc(1, sizeof(Elput_Event_Seat_Frame));
   if (!ev) return;

   ev->seat = seat;
   seat->refs++;
   ecore_event_add(ELPUT_EVENT_SEAT_FRAME, ev, _seat_event_free, seat);
}

static void
_evdev_leds_update(Elput_Device *edev, Elput_Leds leds)
{
   enum libinput_led input_leds = 0;

   if (leds & ELPUT_LED_NUM)
     input_leds |= LIBINPUT_LED_NUM_LOCK;
   if (leds & ELPUT_LED_CAPS)
     input_leds |= LIBINPUT_LED_CAPS_LOCK;
   if (leds & ELPUT_LED_SCROLL)
     input_leds |= LIBINPUT_LED_SCROLL_LOCK;

   libinput_device_led_update(edev->device, input_leds);
}

static void
_keyboard_modifiers_update(Elput_Keyboard *kbd, Elput_Seat *seat)
{
   xkb_mod_mask_t mask;
   Elput_Leds leds = 0;

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
   if (mask & kbd->info->mods.caps)
     seat->modifiers |= ECORE_EVENT_MODIFIER_CAPS;
   if (mask & kbd->info->mods.num)
     seat->modifiers |= ECORE_EVENT_MODIFIER_NUM;

   if (kbd->mods.locked & kbd->info->mods.caps)
     seat->modifiers |= ECORE_EVENT_LOCK_CAPS;
   if (kbd->mods.locked & kbd->info->mods.num)
     seat->modifiers |= ECORE_EVENT_LOCK_NUM;

   if (xkb_state_led_index_is_active(kbd->state, kbd->info->leds.num))
     leds |= ELPUT_LED_NUM;

   if (xkb_state_led_index_is_active(kbd->state, kbd->info->leds.caps))
     leds |= ELPUT_LED_CAPS;

   if (xkb_state_led_index_is_active(kbd->state, kbd->info->leds.scroll))
     leds |= ELPUT_LED_SCROLL;

   if (kbd->leds != leds)
     {
        Eina_List *l;
        Elput_Device *edev;

        EINA_LIST_FOREACH(seat->devices, l, edev)
          _evdev_leds_update(edev, leds);

        kbd->leds = leds;
     }
}

static Elput_Keyboard_Info *
_keyboard_info_create(struct xkb_keymap *keymap)
{
   Elput_Keyboard_Info *info;

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
   info->mods.num =
     1 << xkb_keymap_mod_get_index(info->keymap.map, "Mod2");
   info->mods.ctrl =
     1 << xkb_keymap_mod_get_index(info->keymap.map, XKB_MOD_NAME_CTRL);
   info->mods.alt =
     1 << xkb_keymap_mod_get_index(info->keymap.map, XKB_MOD_NAME_ALT);
   info->mods.altgr =
     1 << xkb_keymap_mod_get_index(info->keymap.map, "ISO_Level3_Shift");

   info->leds.num =
     xkb_keymap_led_get_index(info->keymap.map, XKB_LED_NAME_NUM);
   info->leds.caps =
     xkb_keymap_led_get_index(info->keymap.map, XKB_LED_NAME_CAPS);
   info->leds.scroll =
     xkb_keymap_led_get_index(info->keymap.map, XKB_LED_NAME_SCROLL);

   return info;
}

static void
_keyboard_info_destroy(Elput_Keyboard_Info *info)
{
   if (--info->refs > 0) return;

   xkb_keymap_unref(info->keymap.map);

   free(info);
}

/**
 * @brief Creates the XKB context and keymap.
 *
 * @param kbd The keyboard config to be set up for XKB.
 * @return EINA_TRUE if build successful, EINA_FALSE otherwise.
 *
 * Sets default settings for the keyboard if not set by the system.
 * Assumes evdev rules, with a pc105 model keyboard and US key layout.
 */
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

   kbd->info = _keyboard_info_create(keymap);
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

static void
_keyboard_compose_init(Elput_Keyboard *kbd)
{
   const char *locale;

   if (!(locale = getenv("LC_ALL")))
     if (!(locale = getenv("LC_CTYPE")))
       if (!(locale = getenv("LANG")))
         locale = "C";

   if (kbd->compose_table) xkb_compose_table_unref(kbd->compose_table);
   kbd->compose_table = xkb_compose_table_new_from_locale(kbd->context, locale,
     XKB_COMPOSE_COMPILE_NO_FLAGS);
   if (kbd->compose_state) xkb_compose_state_unref(kbd->compose_state);
   if (kbd->compose_table)
     {
        kbd->compose_state =
          xkb_compose_state_new(kbd->compose_table, XKB_COMPOSE_STATE_NO_FLAGS);
     }
   else
     kbd->compose_state = NULL;
}

/**
 * Create a new keyboard object for the seat, initialized
 * to the given keymap.
 */
static Eina_Bool
_keyboard_init(Elput_Seat *seat, struct xkb_keymap *keymap)
{
   Elput_Keyboard *kbd;

   if (seat->kbd)
     {
        seat->count.kbd += 1;
        if (seat->count.kbd == 1)
          _seat_caps_update(seat);
        return EINA_TRUE;
     }

   kbd = _keyboard_create(seat);
   if (!kbd) return EINA_FALSE;

   if (keymap)
     {
        if (seat->manager->cached.keymap == keymap)
          kbd->context = xkb_context_ref(seat->manager->cached.context);
        kbd->info = _keyboard_info_create(keymap);
        if (!kbd->info) goto err;
     }
   else
     {
        if (!_keyboard_global_build(kbd)) goto err;
        kbd->info->refs++;
     }

   kbd->state = xkb_state_new(kbd->info->keymap.map);
   if (!kbd->state) goto err;
   kbd->maskless_state = xkb_state_new(kbd->info->keymap.map);

   _keyboard_compose_init(kbd);

   seat->kbd = kbd;
   seat->count.kbd = 1;

   _seat_caps_update(seat);

   return EINA_TRUE;

err:
   if (kbd->info) _keyboard_info_destroy(kbd->info);
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
_event_free(void *dev, void *ev)
{
   if (dev) efl_unref(dev);
   free(ev);
}

/**
 * Creates an event key object and generates a corresponding event.
 *
 * An ECORE_EVENT_KEY_DOWN event is generated on state
 * LIBINPUT_KEY_STATE_PRESSED, or ECORE_EVENT_KEY_DOWN otherwise.
 */
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
   ev->dev = dev->evas_device;
   if (ev->dev) efl_ref(ev->dev);

   ev->window = dev->seat->manager->window;
   ev->event_window = dev->seat->manager->window;
   ev->root_window = dev->seat->manager->window;

   if (state == LIBINPUT_KEY_STATE_PRESSED)
     ecore_event_add(ECORE_EVENT_KEY_DOWN, ev, _event_free, ev->dev);
   else
     ecore_event_add(ECORE_EVENT_KEY_UP, ev, _event_free, ev->dev);
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

static Eina_Bool
_keyboard_state_update(Elput_Keyboard *kbd, struct xkb_keymap *map, xkb_mod_mask_t *latched, xkb_mod_mask_t *locked)
{
   struct xkb_state *state, *maskless_state;

   state = xkb_state_new(map);
   if (!state) return EINA_FALSE;
   maskless_state = xkb_state_new(map);
   if (!maskless_state)
     {
        xkb_state_unref(state);
        return EINA_FALSE;
     }

   *latched = xkb_state_serialize_mods(kbd->state, XKB_STATE_MODS_LATCHED);
   *locked = xkb_state_serialize_mods(kbd->state, XKB_STATE_MODS_LOCKED);
   xkb_state_update_mask(state, 0, *latched, *locked,
                         kbd->seat->manager->cached.group, 0, 0);

   xkb_state_unref(kbd->state);
   kbd->state = state;
   xkb_state_unref(kbd->maskless_state);
   kbd->maskless_state = maskless_state;
   return EINA_TRUE;
}

void
_keyboard_keymap_update(Elput_Seat *seat)
{
   Elput_Keyboard *kbd;
   Elput_Keyboard_Info *info = NULL;
   xkb_mod_mask_t latched, locked;
   Eina_Bool state = EINA_TRUE;

   kbd = _evdev_keyboard_get(seat);
   if (!kbd) return;
   kbd->pending_keymap = 1;
   if (kbd->key_count) return;

   if (kbd->seat->manager->cached.keymap)
     {
        if (kbd->context) xkb_context_unref(kbd->context);
        kbd->context = xkb_context_ref(kbd->seat->manager->cached.context);
        info = _keyboard_info_create(kbd->seat->manager->cached.keymap);
        if (!info) return;
        state = _keyboard_state_update(kbd, info->keymap.map,
                                       &latched, &locked);
     }
   else if (!_keyboard_global_build(kbd))
     return;
   else
     state = _keyboard_state_update(kbd, kbd->info->keymap.map,
                                    &latched, &locked);

   kbd->pending_keymap = 0;
   if (!state)
     {
        if (info) _keyboard_info_destroy(info);
        return;
     }

   if (info)
     {
        _keyboard_info_destroy(kbd->info);
        kbd->info = info;
     }

   _keyboard_compose_init(kbd);

   _keyboard_modifiers_update(kbd, seat);

   if ((!latched) && (!locked)) return;

   _keyboard_modifiers_send(kbd);
}

void
_keyboard_group_update(Elput_Seat *seat)
{
   Elput_Keyboard *kbd;
   xkb_mod_mask_t latched, locked;
   Eina_Bool state;

   kbd = _evdev_keyboard_get(seat);
   if (!kbd) return;

   state = _keyboard_state_update(kbd, kbd->info->keymap.map, &latched, &locked);
   if (!state) return;
   _keyboard_compose_init(kbd);

   _keyboard_modifiers_update(kbd, seat);

   if ((!latched) && (!locked)) return;

   _keyboard_modifiers_send(kbd);
}

static int
_keyboard_remapped_key_get(Elput_Device *edev, int code)
{
   void *ret = NULL;

   if (!edev) return code;
   if (!edev->key_remap) return code;
   if (!edev->key_remap_hash) return code;

   ret = eina_hash_find(edev->key_remap_hash, &code);
   if (ret) code = (int)(intptr_t)ret;
   return code;
}

static int
_keyboard_keysym_translate(xkb_keysym_t keysym, unsigned int modifiers, char *buffer, int bytes)
{
/* this function is copied, with slight changes in variable names, from KeyBind.c in libX11
 * the license from that file can be found below:
 */
/*

Copyright 1985, 1987, 1998  The Open Group

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from The Open Group.

*/
   if (!keysym) return 0;

   /* check for possible control codes */
   if (modifiers & ECORE_EVENT_MODIFIER_CTRL)
     {
        Eina_Bool valid_control_code = EINA_TRUE;
        unsigned long hbytes = 0;
        unsigned char c;

        hbytes = (keysym >> 8);
        if (!(bytes &&
        ((hbytes == 0) ||
        ((hbytes == 0xFF) &&
        (((keysym >= XKB_KEY_BackSpace) && (keysym <= XKB_KEY_Clear)) ||
        (keysym == XKB_KEY_Return) ||
        (keysym == XKB_KEY_Escape) ||
        (keysym == XKB_KEY_KP_Space) ||
        (keysym == XKB_KEY_KP_Tab) ||
        (keysym == XKB_KEY_KP_Enter) ||
        ((keysym >= XKB_KEY_KP_Multiply) && (keysym <= XKB_KEY_KP_9)) ||
        (keysym == XKB_KEY_KP_Equal) ||
        (keysym == XKB_KEY_Delete))))))
          return 0;

        if (keysym == XKB_KEY_KP_Space)
          c = (XKB_KEY_space & 0x7F);
        else if (hbytes == 0xFF)
          c = (keysym & 0x7F);
        else
          c = (keysym & 0xFF);

        /* We are building here a control code
           for more details, read:
           https://en.wikipedia.org/wiki/C0_and_C1_control_codes#C0_.28ASCII_and_derivatives.29
         */

        if (((c >= '@') && (c <= '_')) || /* those are the one defined in C0 with capital letters */
             ((c >= 'a') && (c <= 'z')) ||  /* the lowercase symbols (not part of the standard, but useful) */
              c == ' ')
          c &= 0x1F;
        else if (c == '\x7f')
          c = '\177';
        /* following codes are alternatives, they are longer here, i dont want to change them */
        else if (c == '2')
          c = '\000'; /* 0 code */
        else if ((c >= '3') && (c <= '7'))
          c -= ('3' - '\033'); /* from escape to unitseperator code*/
        else if (c == '8')
          c = '\177'; /* delete code */
        else if (c == '/')
          c = '_' & 0x1F; /* unit seperator code */
        else
          valid_control_code = EINA_FALSE;

        if (valid_control_code)
          buffer[0] = c;
        else
          return 0;
     }
   else
     {
        /* if its not a control code, try to produce useful output */
        if (!xkb_keysym_to_utf8(keysym, buffer, bytes))
          return 0;
     }

   return 1;
}

/* from weston/clients/window.c */
/* Translate symbols appropriately if a compose sequence is being entered */
static xkb_keysym_t
process_key_press(xkb_keysym_t sym, Elput_Keyboard *kbd)
{
   if (!kbd->compose_state)
     return sym;
   if (sym == XKB_KEY_NoSymbol)
     return sym;
   if (xkb_compose_state_feed(kbd->compose_state, sym) != XKB_COMPOSE_FEED_ACCEPTED)
     return sym;

   switch (xkb_compose_state_get_status(kbd->compose_state))
     {
      case XKB_COMPOSE_COMPOSING:
        return XKB_KEY_NoSymbol;
      case XKB_COMPOSE_COMPOSED:
        return xkb_compose_state_get_one_sym(kbd->compose_state);
      case XKB_COMPOSE_CANCELLED:
        return XKB_KEY_NoSymbol;
      case XKB_COMPOSE_NOTHING:
      default: break;
     }
   return sym;
}

/**
 * Retrieve the string name ('q', 'bracketleft', etc.) for a given key symbol.
 */
static void
_elput_symbol_rep_find(xkb_keysym_t keysym, char *buffer, int size, unsigned int code)
{
    if (xkb_keysym_get_name(keysym, buffer, size) != 0)
      return;

    snprintf(buffer, size, "Keycode-%u", code);
}

/**
 * Handle keyboard events emitted by libinput.
 *
 * Processes a single key pressed / released event from libinput.  The
 * key code will be remapped to another code if one has been registered
 * (see elput_input_key_remap_enable()), and then XKB notified of the
 * keyboard status change.  XKB translates the key code into the
 * appropriate symbol for the current keyboard layout.  Compose
 * translation is performed, if appropriate.  An up or down event is
 * then generated for the processed key via _keyboard_key_send().
 */
static void
_keyboard_key(struct libinput_device *idevice, struct libinput_event_keyboard *event)
{
   Elput_Device *dev;
   Elput_Keyboard *kbd;
   enum libinput_key_state state;
   xkb_keysym_t sym_name, sym = XKB_KEY_NoSymbol;
   const xkb_keysym_t *syms;
   unsigned int code = 0;
   unsigned int nsyms;
   unsigned int timestamp;
   char key[256] = {0}, keyname[256] = {0}, compose[256] = {0};
   int count;

   dev = libinput_device_get_user_data(idevice);
   if (!dev) return;

   kbd = _evdev_keyboard_get(dev->seat);
   if (!kbd) return;

   /* Retrieve details about the event from libinput */
   state = libinput_event_keyboard_get_key_state(event);
   kbd->key_count = count = libinput_event_keyboard_get_seat_key_count(event);

   /* Ignore key events that are not seat wide state changes. */
   if (((state == LIBINPUT_KEY_STATE_PRESSED) && (count != 1)) ||
       ((state == LIBINPUT_KEY_STATE_RELEASED) && (count != 0)))
     return;

   /* Retrieve the code and remap it if a remap for it has been registered */
   code = libinput_event_keyboard_get_key(event);
   code = _keyboard_remapped_key_get(dev, code) + 8;

   timestamp = libinput_event_keyboard_get_time(event);

   /* Update the XKB keyboard state for the key that was pressed or released */
   if (state == LIBINPUT_KEY_STATE_PRESSED)
     xkb_state_update_key(kbd->state, code, XKB_KEY_DOWN);
   else
     xkb_state_update_key(kbd->state, code, XKB_KEY_UP);

   /* Apply the current keyboard state to translate the code for the key
    * that was struck into its effective symbol (after applying
    * modifiers like CAPSLOCK and so on).  We also use the maskless
    * keyboard state to lookup the underlying symbol name (i.e. without
    * applying modifiers).
    */
   nsyms = xkb_key_get_syms(kbd->state, code, &syms);
   if (nsyms == 1) sym = syms[0];
   sym_name = xkb_state_key_get_one_sym(kbd->maskless_state, code);

   if (state == LIBINPUT_KEY_STATE_PRESSED)
     sym = process_key_press(sym, kbd);

   /* Lookup the textual name ('q', 'space', 'bracketleft', etc.) of the
    * symbol and of the symbol name.
    */
   _elput_symbol_rep_find(sym, key, sizeof(key), code);
   _elput_symbol_rep_find(sym_name, keyname, sizeof(keyname), code);

   /* If no keyname was found, name it "Keycode-NNN" */
   if (keyname[0] == '\0')
     snprintf(keyname, sizeof(keyname), "Keycode-%u", code);

   /* If Shift key is active, downcase the keyname's first letter */
   if (xkb_state_mod_index_is_active(kbd->state, kbd->info->mods.shift,
                                     XKB_STATE_MODS_EFFECTIVE))
     {
        if (keyname[0] != '\0')
          keyname[0] = tolower(keyname[0]);
     }

   /* Update the seat's modifiers to match what's active in the kbd */
   _keyboard_modifiers_update(kbd, dev->seat);

   /* Translate the key symbol into a printable character in Unicode (UTF-8) format */
   _keyboard_keysym_translate(sym, dev->seat->modifiers, compose, sizeof(compose));

   /* Issue the appropriate key up or down event with all related key data */
   _keyboard_key_send(dev, state, keyname, key, compose, code, timestamp);

   if ((kbd->pending_keymap) && (count == 0))
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
   ptr->mouse.threshold = 250;

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
          _seat_caps_update(seat);
        return EINA_TRUE;
     }

   ptr = _pointer_create(seat);
   if (!ptr) return EINA_FALSE;

   seat->ptr = ptr;
   seat->count.ptr = 1;
   ptr->pressure = 1.0;

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
   touch->pressure = 1.0;

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
          _seat_caps_update(seat);
        return EINA_TRUE;
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
   edev->seat->pending_motion = 0;

   x = ptr->seat->pointer.x;
   y = ptr->seat->pointer.y;

   if (x < ptr->minx)
     x = ptr->minx;
   else if (x >= ptr->minx + ptr->seat->manager->input.pointer_w)
     x = ptr->minx + ptr->seat->manager->input.pointer_w - 1;

   if (y < ptr->miny)
     y = ptr->miny;
   else if (y >= ptr->miny + ptr->seat->manager->input.pointer_h)
     y = ptr->miny + ptr->seat->manager->input.pointer_h - 1;

   ptr->seat->pointer.x = x;
   ptr->seat->pointer.y = y;

   ev->window = edev->seat->manager->window;
   ev->event_window = edev->seat->manager->window;
   ev->root_window = edev->seat->manager->window;
   ev->timestamp = ptr->timestamp;
   ev->same_screen = 1;
   ev->dev = edev->evas_device;
   if (ev->dev) efl_ref(ev->dev);

   ev->x = ptr->seat->pointer.x;
   ev->y = ptr->seat->pointer.y;
   ev->root.x = ptr->seat->pointer.x;
   ev->root.y = ptr->seat->pointer.y;

   kbd = _evdev_keyboard_get(edev->seat);
   if (kbd) _keyboard_modifiers_update(kbd, edev->seat);

   ev->modifiers = edev->seat->modifiers;

   touch = _evdev_touch_get(edev->seat);
   if (touch) ev->multi.device = touch->slot;

   ev->multi.radius = 1;
   ev->multi.radius_x = 1;
   ev->multi.radius_y = 1;
   ev->multi.pressure = ptr->pressure;
   ev->multi.angle = 0.0;
   ev->multi.x = ptr->seat->pointer.x;
   ev->multi.y = ptr->seat->pointer.y;
   ev->multi.root.x = ptr->seat->pointer.x;
   ev->multi.root.y = ptr->seat->pointer.y;

   ecore_event_add(ECORE_EVENT_MOUSE_MOVE, ev, _event_free, ev->dev);
}

static void
_pointer_motion_relative_fake(struct libinput_event_pointer *event, double dx, double dy)
{
   Elput_Event_Pointer_Motion *ev;

   ev = calloc(1, sizeof(Elput_Event_Pointer_Motion));
   EINA_SAFETY_ON_NULL_RETURN(ev);

   ev->time_usec = libinput_event_pointer_get_time_usec(event);
   ev->dx = dx;
   ev->dy = dy;
   ev->dx_unaccel = dx;
   ev->dy_unaccel = dy;

   ecore_event_add(ELPUT_EVENT_POINTER_MOTION, ev, NULL, NULL);
}

static void
_pointer_motion_relative(struct libinput_event_pointer *event)
{
   Elput_Event_Pointer_Motion *ev;

   ev = calloc(1, sizeof(Elput_Event_Pointer_Motion));
   EINA_SAFETY_ON_NULL_RETURN(ev);

   ev->time_usec = libinput_event_pointer_get_time_usec(event);
   ev->dx = libinput_event_pointer_get_dx(event);
   ev->dy = libinput_event_pointer_get_dy(event);
   ev->dx_unaccel = libinput_event_pointer_get_dx_unaccelerated(event);
   ev->dy_unaccel = libinput_event_pointer_get_dy_unaccelerated(event);

   ecore_event_add(ELPUT_EVENT_POINTER_MOTION, ev, NULL, NULL);
}

static Eina_Bool
_pointer_motion(struct libinput_device *idev, struct libinput_event_pointer *event)
{
   Elput_Device *edev;
   Elput_Pointer *ptr;
   double dx, dy, tmp;

   edev = libinput_device_get_user_data(idev);
   if (!edev) return EINA_FALSE;

   ptr = _evdev_pointer_get(edev->seat);
   if (!ptr) return EINA_FALSE;

   dx = libinput_event_pointer_get_dx(event);
   dy = libinput_event_pointer_get_dy(event);

   if (edev->swap)
     {
        tmp = dx;
        dx = dy;
        dy = tmp;
     }
   if (edev->invert_x) dx *= -1;
   if (edev->invert_y) dy *= -1;

   ptr->seat->pointer.x += dx;
   ptr->seat->pointer.y += dy;
   ptr->timestamp = libinput_event_pointer_get_time(event);

   _pointer_motion_send(edev);
   _pointer_motion_relative(event);

   return EINA_TRUE;
}

static Eina_Bool
_pointer_motion_abs(struct libinput_device *idev, struct libinput_event_pointer *event)
{
   Elput_Device *edev;
   Elput_Pointer *ptr;
   double x, y;

   edev = libinput_device_get_user_data(idev);
   if (!edev) return EINA_FALSE;

   ptr = _evdev_pointer_get(edev->seat);
   if (!ptr) return EINA_FALSE;

   x = edev->absx;
   edev->absx =
     libinput_event_pointer_get_absolute_x_transformed(event, edev->ow);
   ptr->seat->pointer.x = edev->absx;

   y = edev->absy;
   edev->absy =
     libinput_event_pointer_get_absolute_y_transformed(event, edev->oh);
   ptr->seat->pointer.y = edev->absy;

   ptr->timestamp = libinput_event_pointer_get_time(event);

   /* TODO: these needs to run a matrix transform based on output */

   _pointer_motion_send(edev);
   _pointer_motion_relative_fake(event, edev->absx - x, edev->absy - y);

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

   ev->window = edev->seat->manager->window;
   ev->event_window = edev->seat->manager->window;
   ev->root_window = edev->seat->manager->window;
   ev->timestamp = ptr->timestamp;
   ev->same_screen = 1;
   ev->dev = edev->evas_device;
   if (ev->dev) efl_ref(ev->dev);

   ev->x = ptr->seat->pointer.x;
   ev->y = ptr->seat->pointer.y;
   ev->root.x = ptr->seat->pointer.x;
   ev->root.y = ptr->seat->pointer.y;

   touch = _evdev_touch_get(edev->seat);
   if (touch) ev->multi.device = touch->slot;
   ev->multi.radius = 1;
   ev->multi.radius_x = 1;
   ev->multi.radius_y = 1;
   ev->multi.pressure = ptr->pressure;
   ev->multi.angle = 0.0;
   ev->multi.x = ptr->seat->pointer.x;
   ev->multi.y = ptr->seat->pointer.y;
   ev->multi.root.x = ptr->seat->pointer.x;
   ev->multi.root.y = ptr->seat->pointer.y;

   ev->buttons = ptr->buttons;

   ev->double_click = ptr->mouse.double_click;
   ev->triple_click = ptr->mouse.triple_click;

   kbd = _evdev_keyboard_get(edev->seat);
   if (kbd)
     _keyboard_modifiers_update(kbd, edev->seat);
   ev->modifiers = edev->seat->modifiers;

   if (state)
     ecore_event_add(ECORE_EVENT_MOUSE_BUTTON_DOWN, ev, _event_free, ev->dev);
   else
     ecore_event_add(ECORE_EVENT_MOUSE_BUTTON_UP, ev, _event_free, ev->dev);
}

static void
_pointer_click_update(Elput_Pointer *ptr, unsigned int btn)
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

   if (state) _pointer_click_update(ptr, btn);
   if (edev->seat->pending_motion) _pointer_motion_send(edev);

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

   ev->window = dev->seat->manager->window;
   ev->event_window = dev->seat->manager->window;
   ev->root_window = dev->seat->manager->window;
   ev->timestamp = ptr->timestamp;
   ev->same_screen = 1;
   ev->dev = dev->evas_device;
   if (ev->dev) efl_ref(ev->dev);

   ev->x = ptr->seat->pointer.x;
   ev->y = ptr->seat->pointer.y;
   ev->root.x = ptr->seat->pointer.x;
   ev->root.y = ptr->seat->pointer.y;

   ev->z = value;
   ev->direction = direction;

   kbd = _evdev_keyboard_get(dev->seat);
   if (kbd) _keyboard_modifiers_update(kbd, dev->seat);

   ev->modifiers = dev->seat->modifiers;

   ecore_event_add(ECORE_EVENT_MOUSE_WHEEL, ev, _event_free, ev->dev);
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
        val = libinput_event_pointer_get_axis_value_discrete(event, axis);
        break;
      case LIBINPUT_POINTER_AXIS_SOURCE_FINGER:
      case LIBINPUT_POINTER_AXIS_SOURCE_CONTINUOUS:
        val = libinput_event_pointer_get_axis_value(event, axis);
        break;
      default:
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

   ptr->timestamp = libinput_event_pointer_get_time(event);

   _pointer_axis_send(dev, dir, val);

   return EINA_TRUE;
}

static void
_touch_event_send(Elput_Device *dev, int type)
{
   Elput_Touch *touch;
   Ecore_Event_Mouse_Button *ev;
   unsigned int btn = 0;

   touch = _evdev_touch_get(dev->seat);
   if (!touch) return;

   ev = calloc(1, sizeof(Ecore_Event_Mouse_Button));
   if (!ev) return;

   ev->window = dev->seat->manager->window;
   ev->event_window = dev->seat->manager->window;
   ev->root_window = dev->seat->manager->window;
   ev->timestamp = touch->timestamp;
   ev->same_screen = 1;

   ev->x = touch->seat->pointer.x;
   ev->y = touch->seat->pointer.y;
   ev->root.x = touch->seat->pointer.x;
   ev->root.y = touch->seat->pointer.y;

   ev->modifiers = dev->seat->modifiers;

   ev->multi.device = touch->slot;
   ev->multi.radius = 1;
   ev->multi.radius_x = 1;
   ev->multi.radius_y = 1;
   ev->multi.pressure = touch->pressure;
   ev->multi.angle = 0.0;
   ev->multi.x = ev->x;
   ev->multi.y = ev->y;
   ev->multi.root.x = ev->x;
   ev->multi.root.y = ev->y;

   btn = ((btn & 0x00F) + 1);
// XXX: this code is useless. above btn is set to 0 at declaration time, then
// no code changes it until the above like effectively makes it 1. it can
// only ever be 1 so the below lines are pointless. this is probably a bug
// lurking...
//   if (btn == 3) btn = 2;
//   else if (btn == 2) btn = 3;
   ev->buttons = btn;

   ecore_event_add(type, ev, NULL, NULL);
}

static void
_touch_motion_send(Elput_Device *dev)
{
   Elput_Touch *touch;
   Ecore_Event_Mouse_Move *ev;

   touch = _evdev_touch_get(dev->seat);
   if (!touch) return;

   ev = calloc(1, sizeof(Ecore_Event_Mouse_Move));
   if (!ev) return;

   ev->window = dev->seat->manager->window;
   ev->event_window = dev->seat->manager->window;
   ev->root_window = dev->seat->manager->window;
   ev->timestamp = touch->timestamp;
   ev->same_screen = 1;
   ev->dev = dev->evas_device;
   if (ev->dev) efl_ref(ev->dev);

   ev->x = lround(touch->seat->pointer.x);
   ev->y = lround(touch->seat->pointer.y);
   ev->root.x = ev->x;
   ev->root.y = ev->y;

   ev->modifiers = dev->seat->modifiers;

   ev->multi.device = touch->slot;
   ev->multi.radius = 1;
   ev->multi.radius_x = 1;
   ev->multi.radius_y = 1;
   ev->multi.pressure = touch->pressure;
   ev->multi.angle = 0.0;
   ev->multi.x = touch->seat->pointer.x;
   ev->multi.y = touch->seat->pointer.y;
   ev->multi.root.x = touch->seat->pointer.x;
   ev->multi.root.y = touch->seat->pointer.y;

   ecore_event_add(ECORE_EVENT_MOUSE_MOVE, ev, _event_free, ev->dev);
}

static void
_touch_down(struct libinput_device *idevice, struct libinput_event_touch *event)
{
   Elput_Device *dev;
   Elput_Touch *touch;

   dev = libinput_device_get_user_data(idevice);
   if (!dev) return;

   touch = _evdev_touch_get(dev->seat);
   if (!touch) return;

   touch->slot = libinput_event_touch_get_slot(event);
   touch->timestamp = libinput_event_touch_get_time(event);

   touch->seat->pointer.x =
     libinput_event_touch_get_x_transformed(event, dev->ow);
   touch->seat->pointer.y =
     libinput_event_touch_get_y_transformed(event, dev->oh);

   /* TODO: these needs to run a matrix transform based on output */
   /* _ecore_drm2_output_coordinate_transform(dev->output, */
   /*                                         touch->seat->pointer.x, touch->seat->pointer.y, */
   /*                                         &touch->seat->pointer.x, &touch->seat->pointer.y); */

   if (touch->slot == touch->grab.id)
     {
        touch->grab.x = touch->seat->pointer.x;
        touch->grab.y = touch->seat->pointer.y;
     }

   touch->points++;

   _touch_motion_send(dev);
   _touch_event_send(dev, ECORE_EVENT_MOUSE_BUTTON_DOWN);

   if (touch->points == 1)
     {
        touch->grab.id = touch->slot;
        touch->grab.x = touch->seat->pointer.x;
        touch->grab.y = touch->seat->pointer.y;
        touch->grab.timestamp = touch->timestamp;
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
   touch->slot = libinput_event_touch_get_slot(event);
   touch->timestamp = libinput_event_touch_get_time(event);

   _touch_motion_send(dev);
   _touch_event_send(dev, ECORE_EVENT_MOUSE_BUTTON_UP);
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

   touch->seat->pointer.x =
     libinput_event_touch_get_x_transformed(event, dev->ow);
   touch->seat->pointer.y =
     libinput_event_touch_get_y_transformed(event, dev->oh);

   /* TODO: these needs to run a matrix transform based on output */
   /* _ecore_drm2_output_coordinate_transform(dev->output, */
   /*                                         touch->seat->pointer.x, touch->seat->pointer.y, */
   /*                                         &touch->seat->pointer.x, &touch->seat->pointer.y); */

   touch->slot = libinput_event_touch_get_slot(event);
   touch->timestamp = libinput_event_touch_get_time(event);

   _touch_motion_send(dev);
}

void
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
       (libinput_device_config_calibration_get_default_matrix(dev->device, cal) != 0))
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

static void
_axis_event_free(void *d EINA_UNUSED, void *event)
{
   Ecore_Event_Axis_Update *ev = event;

   if (ev->dev) efl_unref(ev->dev);
   free(ev->axis);
   free(ev);
}

static void
_tablet_tool_axis(struct libinput_device *idev, struct libinput_event_tablet_tool *event)
{
   Elput_Pointer *ptr;
   struct libinput_tablet_tool *tool;
   Elput_Device *dev = libinput_device_get_user_data(idev);
   Ecore_Event_Axis_Update *ev;
   Ecore_Axis ax[8] = {}, *axis = NULL;
   int i, num = 0;

   ptr = _evdev_pointer_get(dev->seat);
   EINA_SAFETY_ON_NULL_RETURN(ptr);
   tool = libinput_event_tablet_tool_get_tool(event);

   ptr->seat->pointer.x =
     libinput_event_tablet_tool_get_x_transformed(event, dev->ow);
   ptr->seat->pointer.y =
     libinput_event_tablet_tool_get_y_transformed(event, dev->oh);

   if (libinput_event_tablet_tool_x_has_changed(event))
     {
        ax[num].label = ECORE_AXIS_LABEL_X;
        ax[num].value = ptr->seat->pointer.x;
        num++;
     }
   if (libinput_event_tablet_tool_y_has_changed(event))
     {
        ax[num].label = ECORE_AXIS_LABEL_Y;
        ax[num].value = ptr->seat->pointer.y;
        num++;
     }
   if (libinput_tablet_tool_has_pressure(tool))
     {
        if (libinput_event_tablet_tool_pressure_has_changed(event))
          {
             ax[num].label = ECORE_AXIS_LABEL_PRESSURE;
             ax[num].value = ptr->pressure =
               libinput_event_tablet_tool_get_pressure(event);
             num++;
          }
     }
   if (libinput_tablet_tool_has_distance(tool))
     {
        if (libinput_event_tablet_tool_distance_has_changed(event))
          {
             ax[num].label = ECORE_AXIS_LABEL_DISTANCE;
             ax[num].value = libinput_event_tablet_tool_get_distance(event);
             num++;
          }
     }
   if (libinput_tablet_tool_has_tilt(tool))
     {
        if (libinput_event_tablet_tool_tilt_x_has_changed(event) ||
            libinput_event_tablet_tool_tilt_y_has_changed(event))
          {
             double x = sin(libinput_event_tablet_tool_get_tilt_x(event));
             double y = sin(-libinput_event_tablet_tool_get_tilt_y(event));

             ax[num].label = ECORE_AXIS_LABEL_TILT;
             ax[num].value = asin(sqrt((x * x) + (y * y)));
             num++;

             /* note: the value of atan2(0,0) is implementation-defined */
             ax[num].label = ECORE_AXIS_LABEL_AZIMUTH;
             ax[num].value = atan2(y, x);
             num++;
          }
     }
   if (libinput_tablet_tool_has_rotation(tool))
     {
        if (libinput_event_tablet_tool_rotation_has_changed(event))
          {
             ax[num].label = ECORE_AXIS_LABEL_TWIST;
             ax[num].value = libinput_event_tablet_tool_get_rotation(event);
             ax[num].value *= M_PI / 180;
             num++;
          }
     }

   ptr->timestamp = libinput_event_tablet_tool_get_time(event);

   /* FIXME: other properties which efl event structs don't support:
    * slider_position
    * wheel_delta
    */

   if (libinput_event_tablet_tool_x_has_changed(event) ||
       libinput_event_tablet_tool_y_has_changed(event))
     _pointer_motion_send(dev);

   if (!num) return;
   ev = calloc(1, sizeof(Ecore_Event_Axis_Update));

   ev->window = dev->seat->manager->window;
   ev->event_window = dev->seat->manager->window;
   ev->root_window = dev->seat->manager->window;
   ev->timestamp = ptr->timestamp;
   ev->naxis = num;
   ev->dev = dev->evas_device;
   if (ev->dev) efl_ref(ev->dev);
   ev->axis = axis = calloc(num, sizeof(Ecore_Axis));
   for (i = 0; i < num; i++)
     {
        axis[i].label = ax[i].label;
        axis[i].value = ax[i].value;
     }
   ecore_event_add(ECORE_EVENT_AXIS_UPDATE, ev, _axis_event_free, NULL);
}

static void
_tablet_tool_tip(struct libinput_device *idev, struct libinput_event_tablet_tool *event)
{
   Elput_Pointer *ptr;
   Elput_Device *dev = libinput_device_get_user_data(idev);
   int state;
   int press[] =
     {
        [LIBINPUT_TABLET_TOOL_TIP_DOWN] = LIBINPUT_BUTTON_STATE_PRESSED,
        [LIBINPUT_TABLET_TOOL_TIP_UP] = LIBINPUT_BUTTON_STATE_RELEASED,
     };

   ptr = _evdev_pointer_get(dev->seat);
   EINA_SAFETY_ON_NULL_RETURN(ptr);

   state = libinput_event_tablet_tool_get_tip_state(event);
   ptr->buttons = 1;
   ptr->timestamp = libinput_event_tablet_tool_get_time(event);

   if (press[state]) _pointer_click_update(ptr, 1);

   _pointer_button_send(dev, press[state]);
}

static void
_switch_event_free(void *data EINA_UNUSED, void *event)
{
   Elput_Event_Switch *ev = event;

   _evdev_device_destroy(ev->device);
   free(ev);
}

static void
_switch_toggle(struct libinput_device *idev, struct libinput_event_switch *event)
{
   Elput_Event_Switch *ev;

   ev = calloc(1, sizeof(Elput_Event_Switch));
   if (!ev) return;
   ev->device = libinput_device_get_user_data(idev);
   ev->device->refs++;
   ev->time_usec = libinput_event_switch_get_time_usec(event);
   ev->type = (Elput_Switch_Type)libinput_event_switch_get_switch(event);
   ev->state = (Elput_Switch_State)libinput_event_switch_get_switch_state(event);
   ecore_event_add(ELPUT_EVENT_SWITCH, ev, _switch_event_free, NULL);
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
      case LIBINPUT_EVENT_TOUCH_FRAME: break;
      case LIBINPUT_EVENT_TABLET_TOOL_AXIS:
        _tablet_tool_axis(idev, libinput_event_get_tablet_tool_event(event));
        break;
      case LIBINPUT_EVENT_TABLET_TOOL_PROXIMITY: /* is this useful? */
        break;
      case LIBINPUT_EVENT_TABLET_TOOL_TIP: /* is this useful? */
        _tablet_tool_tip(idev, libinput_event_get_tablet_tool_event(event));
        break;
      case LIBINPUT_EVENT_SWITCH_TOGGLE:
        _switch_toggle(idev, libinput_event_get_switch_event(event));
        break;
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
   const char *oname;

   edev = calloc(1, sizeof(Elput_Device));
   if (!edev) return NULL;

   edev->refs = 1;
   edev->seat = seat;
   edev->device = device;
   edev->ow = seat->manager->output_w;
   edev->oh = seat->manager->output_h;

   oname = libinput_device_get_output_name(device);
   if (!oname)
     oname = libinput_device_get_name(device);
   eina_stringshare_replace(&edev->output_name, oname);

   if (libinput_device_has_capability(device, LIBINPUT_DEVICE_CAP_SWITCH))
     edev->caps |= ELPUT_DEVICE_CAPS_SWITCH;
   if ((libinput_device_has_capability(device, LIBINPUT_DEVICE_CAP_KEYBOARD)) &&
       (libinput_device_keyboard_has_key(device, KEY_ENTER)))
     edev->caps |= ELPUT_DEVICE_CAPS_KEYBOARD;
   if (edev->caps & (ELPUT_DEVICE_CAPS_SWITCH | ELPUT_DEVICE_CAPS_KEYBOARD))
     _keyboard_init(seat, seat->manager->cached.keymap);

   if ((libinput_device_has_capability(device, LIBINPUT_DEVICE_CAP_POINTER) &&
       (libinput_device_pointer_has_button(device, BTN_LEFT))))
     edev->caps |= ELPUT_DEVICE_CAPS_POINTER;
   if (libinput_device_has_capability(device, LIBINPUT_DEVICE_CAP_TABLET_TOOL))
     edev->caps |= ELPUT_DEVICE_CAPS_POINTER | ELPUT_DEVICE_CAPS_TABLET_TOOL;
   if (libinput_device_has_capability(device, LIBINPUT_DEVICE_CAP_TABLET_PAD))
     edev->caps |= ELPUT_DEVICE_CAPS_TABLET_PAD;
   if (edev->caps & ELPUT_DEVICE_CAPS_POINTER)
     _pointer_init(seat);

   if (libinput_device_has_capability(device, LIBINPUT_DEVICE_CAP_TOUCH))
     edev->caps |= ELPUT_DEVICE_CAPS_TOUCH;
   if (libinput_device_has_capability(device, LIBINPUT_DEVICE_CAP_GESTURE))
     edev->caps |= ELPUT_DEVICE_CAPS_TOUCH | ELPUT_DEVICE_CAPS_GESTURE;
   if (edev->caps & ELPUT_DEVICE_CAPS_TOUCH)
     _touch_init(seat);

   libinput_device_set_user_data(device, edev);
   libinput_device_ref(edev->device);

   if (libinput_device_config_tap_get_finger_count(edev->device) > 0)
     {
        Eina_Bool enable = EINA_FALSE;

        enable = libinput_device_config_tap_get_default_enabled(edev->device);
        libinput_device_config_tap_set_enabled(edev->device, enable);
     }

   return edev;
}

void
_evdev_device_destroy(Elput_Device *edev)
{
   if (!edev) return;
   edev->refs--;
   if (edev->refs) return;

   if (edev->caps & ELPUT_DEVICE_CAPS_POINTER)
     _pointer_release(edev->seat);
   if (edev->caps & ELPUT_DEVICE_CAPS_KEYBOARD)
     _keyboard_release(edev->seat);
   if (edev->caps & ELPUT_DEVICE_CAPS_TOUCH)
     _touch_release(edev->seat);

   libinput_device_unref(edev->device);
   eina_stringshare_del(edev->output_name);

   if (edev->key_remap_hash) eina_hash_free(edev->key_remap_hash);

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

   if (kbd->compose_table) xkb_compose_table_unref(kbd->compose_table);
   if (kbd->compose_state) xkb_compose_state_unref(kbd->compose_state);

   if (kbd->state) xkb_state_unref(kbd->state);
   if (kbd->maskless_state) xkb_state_unref(kbd->maskless_state);
   if (kbd->info) _keyboard_info_destroy(kbd->info);

   xkb_context_unref(kbd->context);

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
