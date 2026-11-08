#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <SDL2/SDL.h>

#include <Eina.h>
#include <Ecore.h>
#include <ecore_private.h>
#include <Ecore_Input.h>
#include "Ecore_Sdl.h"
#include "Ecore_Sdl_Keys.h"
#include "ecore_sdl_private.h"

int _ecore_sdl_log_dom = -1;

typedef struct _Ecore_SDL_Pressed Ecore_SDL_Pressed;
struct _Ecore_SDL_Pressed
{
   EINA_RBTREE;

   SDL_Keycode key;
};

EAPI int ECORE_SDL_EVENT_GOT_FOCUS = 0;
EAPI int ECORE_SDL_EVENT_LOST_FOCUS = 0;
EAPI int ECORE_SDL_EVENT_RESIZE = 0;
EAPI int ECORE_SDL_EVENT_EXPOSE = 0;

static int _ecore_sdl_init_count = 0;
static Eina_Rbtree *repeat = NULL;
static Eina_Bool _ecore_sdl_ralt_is_alt = EINA_FALSE;

/* SDL scancode -> Linux evdev key code. Ecore reports evdev code + 8 as
 * the keycode, as the X11 and DRM backends do. */
static const unsigned short _ecore_sdl_evdev_codes[SDL_NUM_SCANCODES] =
{
   [SDL_SCANCODE_ESCAPE] = 1,
   [SDL_SCANCODE_1] = 2,
   [SDL_SCANCODE_2] = 3,
   [SDL_SCANCODE_3] = 4,
   [SDL_SCANCODE_4] = 5,
   [SDL_SCANCODE_5] = 6,
   [SDL_SCANCODE_6] = 7,
   [SDL_SCANCODE_7] = 8,
   [SDL_SCANCODE_8] = 9,
   [SDL_SCANCODE_9] = 10,
   [SDL_SCANCODE_0] = 11,
   [SDL_SCANCODE_MINUS] = 12,
   [SDL_SCANCODE_EQUALS] = 13,
   [SDL_SCANCODE_BACKSPACE] = 14,
   [SDL_SCANCODE_TAB] = 15,
   [SDL_SCANCODE_Q] = 16,
   [SDL_SCANCODE_W] = 17,
   [SDL_SCANCODE_E] = 18,
   [SDL_SCANCODE_R] = 19,
   [SDL_SCANCODE_T] = 20,
   [SDL_SCANCODE_Y] = 21,
   [SDL_SCANCODE_U] = 22,
   [SDL_SCANCODE_I] = 23,
   [SDL_SCANCODE_O] = 24,
   [SDL_SCANCODE_P] = 25,
   [SDL_SCANCODE_LEFTBRACKET] = 26,
   [SDL_SCANCODE_RIGHTBRACKET] = 27,
   [SDL_SCANCODE_RETURN] = 28,
   [SDL_SCANCODE_LCTRL] = 29,
   [SDL_SCANCODE_A] = 30,
   [SDL_SCANCODE_S] = 31,
   [SDL_SCANCODE_D] = 32,
   [SDL_SCANCODE_F] = 33,
   [SDL_SCANCODE_G] = 34,
   [SDL_SCANCODE_H] = 35,
   [SDL_SCANCODE_J] = 36,
   [SDL_SCANCODE_K] = 37,
   [SDL_SCANCODE_L] = 38,
   [SDL_SCANCODE_SEMICOLON] = 39,
   [SDL_SCANCODE_APOSTROPHE] = 40,
   [SDL_SCANCODE_GRAVE] = 41,
   [SDL_SCANCODE_LSHIFT] = 42,
   [SDL_SCANCODE_BACKSLASH] = 43,
   [SDL_SCANCODE_Z] = 44,
   [SDL_SCANCODE_X] = 45,
   [SDL_SCANCODE_C] = 46,
   [SDL_SCANCODE_V] = 47,
   [SDL_SCANCODE_B] = 48,
   [SDL_SCANCODE_N] = 49,
   [SDL_SCANCODE_M] = 50,
   [SDL_SCANCODE_COMMA] = 51,
   [SDL_SCANCODE_PERIOD] = 52,
   [SDL_SCANCODE_SLASH] = 53,
   [SDL_SCANCODE_RSHIFT] = 54,
   [SDL_SCANCODE_KP_MULTIPLY] = 55,
   [SDL_SCANCODE_LALT] = 56,
   [SDL_SCANCODE_SPACE] = 57,
   [SDL_SCANCODE_CAPSLOCK] = 58,
   [SDL_SCANCODE_F1] = 59,
   [SDL_SCANCODE_F2] = 60,
   [SDL_SCANCODE_F3] = 61,
   [SDL_SCANCODE_F4] = 62,
   [SDL_SCANCODE_F5] = 63,
   [SDL_SCANCODE_F6] = 64,
   [SDL_SCANCODE_F7] = 65,
   [SDL_SCANCODE_F8] = 66,
   [SDL_SCANCODE_F9] = 67,
   [SDL_SCANCODE_F10] = 68,
   [SDL_SCANCODE_NUMLOCKCLEAR] = 69,
   [SDL_SCANCODE_SCROLLLOCK] = 70,
   [SDL_SCANCODE_KP_7] = 71,
   [SDL_SCANCODE_KP_8] = 72,
   [SDL_SCANCODE_KP_9] = 73,
   [SDL_SCANCODE_KP_MINUS] = 74,
   [SDL_SCANCODE_KP_4] = 75,
   [SDL_SCANCODE_KP_5] = 76,
   [SDL_SCANCODE_KP_6] = 77,
   [SDL_SCANCODE_KP_PLUS] = 78,
   [SDL_SCANCODE_KP_1] = 79,
   [SDL_SCANCODE_KP_2] = 80,
   [SDL_SCANCODE_KP_3] = 81,
   [SDL_SCANCODE_KP_0] = 82,
   [SDL_SCANCODE_KP_PERIOD] = 83,
   [SDL_SCANCODE_LANG5] = 85,
   [SDL_SCANCODE_NONUSBACKSLASH] = 86,
   [SDL_SCANCODE_F11] = 87,
   [SDL_SCANCODE_F12] = 88,
   [SDL_SCANCODE_INTERNATIONAL1] = 89,
   [SDL_SCANCODE_LANG3] = 90,
   [SDL_SCANCODE_LANG4] = 91,
   [SDL_SCANCODE_INTERNATIONAL4] = 92,
   [SDL_SCANCODE_INTERNATIONAL2] = 93,
   [SDL_SCANCODE_INTERNATIONAL5] = 94,
   [SDL_SCANCODE_KP_ENTER] = 96,
   [SDL_SCANCODE_RCTRL] = 97,
   [SDL_SCANCODE_KP_DIVIDE] = 98,
   [SDL_SCANCODE_SYSREQ] = 99,
   [SDL_SCANCODE_RALT] = 100,
   [SDL_SCANCODE_HOME] = 102,
   [SDL_SCANCODE_UP] = 103,
   [SDL_SCANCODE_PAGEUP] = 104,
   [SDL_SCANCODE_LEFT] = 105,
   [SDL_SCANCODE_RIGHT] = 106,
   [SDL_SCANCODE_END] = 107,
   [SDL_SCANCODE_DOWN] = 108,
   [SDL_SCANCODE_PAGEDOWN] = 109,
   [SDL_SCANCODE_INSERT] = 110,
   [SDL_SCANCODE_DELETE] = 111,
   [SDL_SCANCODE_MUTE] = 113,
   [SDL_SCANCODE_VOLUMEDOWN] = 114,
   [SDL_SCANCODE_VOLUMEUP] = 115,
   [SDL_SCANCODE_POWER] = 116,
   [SDL_SCANCODE_KP_EQUALS] = 117,
   [SDL_SCANCODE_KP_PLUSMINUS] = 118,
   [SDL_SCANCODE_PAUSE] = 119,
   [SDL_SCANCODE_KP_COMMA] = 121,
   [SDL_SCANCODE_LANG1] = 122,
   [SDL_SCANCODE_LANG2] = 123,
   [SDL_SCANCODE_INTERNATIONAL3] = 124,
   [SDL_SCANCODE_LGUI] = 125,
   [SDL_SCANCODE_RGUI] = 126,
   [SDL_SCANCODE_APPLICATION] = 127,
   [SDL_SCANCODE_STOP] = 128,
   [SDL_SCANCODE_AGAIN] = 129,
   [SDL_SCANCODE_UNDO] = 131,
   [SDL_SCANCODE_COPY] = 133,
   [SDL_SCANCODE_PASTE] = 135,
   [SDL_SCANCODE_FIND] = 136,
   [SDL_SCANCODE_CUT] = 137,
   [SDL_SCANCODE_HELP] = 138,
   [SDL_SCANCODE_MENU] = 139,
   [SDL_SCANCODE_CALCULATOR] = 140,
   [SDL_SCANCODE_SLEEP] = 142,
   [SDL_SCANCODE_APP1] = 148,
   [SDL_SCANCODE_APP2] = 149,
   [SDL_SCANCODE_WWW] = 150,
   [SDL_SCANCODE_MAIL] = 155,
   [SDL_SCANCODE_AC_BOOKMARKS] = 156,
   [SDL_SCANCODE_COMPUTER] = 157,
   [SDL_SCANCODE_AC_BACK] = 158,
   [SDL_SCANCODE_AC_FORWARD] = 159,
   [SDL_SCANCODE_EJECT] = 161,
   [SDL_SCANCODE_AUDIONEXT] = 163,
   [SDL_SCANCODE_AUDIOPLAY] = 164,
   [SDL_SCANCODE_AUDIOPREV] = 165,
   [SDL_SCANCODE_AUDIOSTOP] = 166,
   [SDL_SCANCODE_AUDIOREWIND] = 168,
   [SDL_SCANCODE_AC_HOME] = 172,
   [SDL_SCANCODE_AC_REFRESH] = 173,
   [SDL_SCANCODE_KP_LEFTPAREN] = 179,
   [SDL_SCANCODE_KP_RIGHTPAREN] = 180,
   [SDL_SCANCODE_F13] = 183,
   [SDL_SCANCODE_F14] = 184,
   [SDL_SCANCODE_F15] = 185,
   [SDL_SCANCODE_F16] = 186,
   [SDL_SCANCODE_F17] = 187,
   [SDL_SCANCODE_F18] = 188,
   [SDL_SCANCODE_F19] = 189,
   [SDL_SCANCODE_F20] = 190,
   [SDL_SCANCODE_F21] = 191,
   [SDL_SCANCODE_F22] = 192,
   [SDL_SCANCODE_F23] = 193,
   [SDL_SCANCODE_F24] = 194,
   [SDL_SCANCODE_AUDIOFASTFORWARD] = 208,
   [SDL_SCANCODE_PRINTSCREEN] = 210,
   [SDL_SCANCODE_AC_SEARCH] = 217,
   [SDL_SCANCODE_ALTERASE] = 222,
   [SDL_SCANCODE_CANCEL] = 223,
   [SDL_SCANCODE_BRIGHTNESSDOWN] = 224,
   [SDL_SCANCODE_BRIGHTNESSUP] = 225,
   [SDL_SCANCODE_MEDIASELECT] = 226,
   [SDL_SCANCODE_DISPLAYSWITCH] = 227,
   [SDL_SCANCODE_KBDILLUMTOGGLE] = 228,
   [SDL_SCANCODE_KBDILLUMDOWN] = 229,
   [SDL_SCANCODE_KBDILLUMUP] = 230,
};

static Eina_Rbtree_Direction
_ecore_sdl_pressed_key(const Ecore_SDL_Pressed *left,
                       const Ecore_SDL_Pressed *right,
                       EINA_UNUSED void *data)
{
   return left->key < right->key ? EINA_RBTREE_LEFT : EINA_RBTREE_RIGHT;
}

static int
_ecore_sdl_pressed_node(const Ecore_SDL_Pressed *node,
                        const SDL_Keycode *key,
                        EINA_UNUSED int length,
                        EINA_UNUSED void *data)
{
   return node->key - *key;
}

/**
 * @defgroup Ecore_Sdl_Library_Group SDL Library Functions
 * @ingroup Ecore
 *
 * Functions used to set up and shut down the Ecore_Sdl functions.
 */

/**
 * Sets up the Ecore_Sdl library.
 * @param   name device target name
 * @return  @c 0 on failure.  Otherwise, the number of times the library has
 *          been initialised without being shut down.
 * @ingroup Ecore_SDL_Library_Group
 */
EAPI int
ecore_sdl_init(const char *name EINA_UNUSED)
{
   if(++_ecore_sdl_init_count != 1)
     return _ecore_sdl_init_count;
   _ecore_sdl_log_dom = eina_log_domain_register
     ("ecore_sdl", ECORE_SDL_DEFAULT_LOG_COLOR);
   if(_ecore_sdl_log_dom < 0)
     {
       EINA_LOG_ERR("Impossible to create a log domain for the Ecore SDL module.");
       return --_ecore_sdl_init_count;
     }
   if (!ecore_event_init())
     return --_ecore_sdl_init_count;

   SDL_Init(SDL_INIT_EVENTS);
   _ecore_sdl_ralt_is_alt = !!getenv("ECORE_SDL_RALT_IS_ALT");

   ECORE_SDL_EVENT_GOT_FOCUS  = ecore_event_type_new();
   ECORE_SDL_EVENT_LOST_FOCUS = ecore_event_type_new();
   ECORE_SDL_EVENT_RESIZE     = ecore_event_type_new();
   ECORE_SDL_EVENT_EXPOSE     = ecore_event_type_new();

   return _ecore_sdl_init_count;
}

/**
 * Shuts down the Ecore_Sdl library.
 * @return  @c The number of times the system has been initialised without
 *             being shut down.
 * @ingroup Ecore_SDL_Library_Group
 */
EAPI int
ecore_sdl_shutdown(void)
{
   if (--_ecore_sdl_init_count != 0)
     return _ecore_sdl_init_count;

   SDL_Quit();

   ecore_event_type_flush(ECORE_SDL_EVENT_GOT_FOCUS,
                          ECORE_SDL_EVENT_LOST_FOCUS,
                          ECORE_SDL_EVENT_RESIZE,
                          ECORE_SDL_EVENT_EXPOSE);

   ecore_event_shutdown();
   eina_log_domain_unregister(_ecore_sdl_log_dom);
   _ecore_sdl_log_dom = -1;
   return _ecore_sdl_init_count;
}

static unsigned int
_ecore_sdl_event_modifiers(Uint16 mod)
{
   unsigned int modifiers = 0;

   if (mod & KMOD_SHIFT) modifiers |= ECORE_EVENT_MODIFIER_SHIFT;
   if (mod & KMOD_CTRL) modifiers |= ECORE_EVENT_MODIFIER_CTRL;
   if (mod & KMOD_LALT) modifiers |= ECORE_EVENT_MODIFIER_ALT;
   /* SDL reports AltGr (ISO level 3 shift) as the right Alt key. On most
    * non-US layouts that is what it is, so report it as AltGr unless
    * ECORE_SDL_RALT_IS_ALT says the right Alt key is a plain Alt key. */
   if (mod & KMOD_RALT)
     modifiers |= _ecore_sdl_ralt_is_alt ?
       ECORE_EVENT_MODIFIER_ALT : ECORE_EVENT_MODIFIER_ALTGR;
   if (mod & KMOD_MODE) modifiers |= ECORE_EVENT_MODIFIER_ALTGR;
   if (mod & KMOD_GUI) modifiers |= ECORE_EVENT_MODIFIER_WIN;
   if (mod & KMOD_NUM) modifiers |= ECORE_EVENT_LOCK_NUM;
   if (mod & KMOD_CAPS) modifiers |= ECORE_EVENT_LOCK_CAPS;
#if SDL_VERSION_ATLEAST(2, 0, 18)
   if (mod & KMOD_SCROLL) modifiers |= ECORE_EVENT_LOCK_SCROLL;
#endif

   return modifiers;
}

/* whether a key table string is a printable character, i.e. text that
 * depends on the keyboard layout, Shift, AltGr... and that SDL gives us
 * in SDL_TEXTINPUT, as opposed to a control code like "\r" or "\010" */
static Eina_Bool
_ecore_sdl_key_string_is_text(const char *s)
{
   return (s) && (s[0]) && (!s[1]) &&
     ((unsigned char)s[0] >= 0x20) && ((unsigned char)s[0] < 0x7f);
}

/* SDL sends the text a key press produces, with the layout, Shift, AltGr
 * and dead keys applied, as an SDL_TEXTINPUT event queued right after the
 * SDL_KEYDOWN. Take it, so that the key event carries the real text. */
static Eina_Bool
_ecore_sdl_key_text_take(Uint32 windowID, char *buf, size_t size)
{
   SDL_Event next;

   if (SDL_PeepEvents(&next, 1, SDL_PEEKEVENT,
                      SDL_FIRSTEVENT, SDL_LASTEVENT) != 1)
     return EINA_FALSE;
   if ((next.type != SDL_TEXTINPUT) || (next.text.windowID != windowID))
     return EINA_FALSE;
   if (SDL_PeepEvents(&next, 1, SDL_GETEVENT,
                      SDL_TEXTINPUT, SDL_TEXTINPUT) != 1)
     return EINA_FALSE;
   eina_strlcpy(buf, next.text.text, size);
   return EINA_TRUE;
}

/* build an Ecore key event; any text is copied into the same allocation,
 * as the event is released with a plain free() */
static Ecore_Event_Key *
_ecore_sdl_key_event_new(Uint32 windowID, const char *name,
                         const char *string, const char *text,
                         unsigned int modifiers, unsigned int keycode,
                         double timestamp)
{
   Ecore_Event_Key *ev;
   size_t tlen = text ? strlen(text) + 1 : 0;

   ev = calloc(1, sizeof(Ecore_Event_Key) + tlen);
   if (!ev) return NULL;

   if (text)
     {
        char *copy = (char *)(ev + 1);

        memcpy(copy, text, tlen);
        if (string == text) string = copy;
        if (name == text) name = copy;
     }

   ev->timestamp = timestamp;
   ev->window = windowID;
   /* ecore_input_evas routes key events by event_window, like mouse
    * events: with 0 they match no window and are silently dropped */
   ev->event_window = windowID;
   ev->modifiers = modifiers;
   ev->keyname = name;
   ev->key = name;
   ev->string = string;
   ev->compose = string;
   ev->keycode = keycode;

   return ev;
}

/* text is the SDL_TEXTINPUT text of a key press, or NULL */
static Ecore_Event_Key*
_ecore_sdl_event_key(const SDL_KeyboardEvent *kev, const char *text,
                     double timestamp)
{
   const char *name = NULL, *string = NULL;
   unsigned int i, keycode = 0;
   SDL_Scancode sc = kev->keysym.scancode;

   for (i = 0; i < EINA_C_ARRAY_LENGTH(keystable); ++i)
     if (keystable[i].code == kev->keysym.sym)
       {
          name = keystable[i].name;
          string = keystable[i].compose;
          break;
       }

   if (text)
     string = text;
   else if ((_ecore_sdl_key_string_is_text(string)) && (SDL_IsTextInputActive()))
     /* SDL produced no text for this key (key release, dead key, Ctrl
      * held...): do not make up the US-layout one */
     string = NULL;

   if (!name)
     {
        /* a key with no entry in the table (a layout-specific key on a
         * backend whose key codes follow the layout): name it after the
         * text it produces, or give up */
        if (!text) return NULL;
        name = text;
     }

   if ((sc > SDL_SCANCODE_UNKNOWN) && (sc < SDL_NUM_SCANCODES) &&
       (_ecore_sdl_evdev_codes[sc]))
     keycode = _ecore_sdl_evdev_codes[sc] + 8;

   return _ecore_sdl_key_event_new(kev->windowID, name, string, text,
                                   _ecore_sdl_event_modifiers(kev->keysym.mod),
                                   keycode, timestamp);
}

/**
 * Poll SDL for mouse, keyboard, and window events, and add corresponding events to ecore.
 * @ingroup Ecore_SDL_Library_Group
 */
EAPI void
ecore_sdl_feed_events(void)
{
   SDL_Event    event;
   unsigned int timestamp;

   while(SDL_PollEvent(&event))
     {
        timestamp = (unsigned int)((unsigned long long)(ecore_time_get() * 1000.0) & 0xffffffff);
        switch(event.type)
          {
          case SDL_MOUSEMOTION:
          {
             Ecore_Event_Mouse_Move *ev;

             ev = calloc(1, sizeof(Ecore_Event_Mouse_Move));
             if (!ev) return;

             ev->timestamp = timestamp;
             ev->window = event.motion.windowID;
             ev->event_window = event.motion.windowID;
             ev->modifiers = 0; /* FIXME: keep modifier around. */
             ev->x = event.motion.x;
             ev->y = event.motion.y;
             ev->root.x = ev->x;
             ev->root.y = ev->y;

             /* Must set multi touch device to 0 or it will get ignored */
             ev->multi.device = 0;
             ev->multi.radius = ev->multi.radius_x = ev->multi.radius_y = 0;
             ev->multi.pressure = ev->multi.angle = 0;
             ev->multi.x = ev->multi.y = ev->multi.root.x = ev->multi.root.y = 0;

             ecore_event_add(ECORE_EVENT_MOUSE_MOVE, ev, NULL, NULL);
             break;
          }
          case SDL_MOUSEBUTTONDOWN:
          {
             Ecore_Event_Mouse_Button *ev;

             ev = calloc(1, sizeof(Ecore_Event_Mouse_Button));
             if (!ev) return;

             ev->timestamp = timestamp;
             ev->window = event.button.windowID;
             ev->event_window = event.button.windowID;
             ev->modifiers = 0; /* FIXME: keep modifier around. */
             ev->buttons = event.button.button;
             ev->double_click = 0;
             ev->triple_click = 0;

             /* Must set multi touch device to 0 or it will get ignored */
             ev->multi.device = 0;
             ev->multi.radius = ev->multi.radius_x = ev->multi.radius_y = 0;
             ev->multi.pressure = ev->multi.angle = 0;
             ev->multi.x = ev->multi.y = ev->multi.root.x = ev->multi.root.y = 0;

             ecore_event_add(ECORE_EVENT_MOUSE_BUTTON_DOWN, ev, NULL, NULL);
             break;
          }
          case SDL_MOUSEWHEEL:
          {
             Ecore_Event_Mouse_Wheel *ev;

             ev = calloc(1, sizeof(Ecore_Event_Mouse_Wheel));
             if (!ev) return;

             ev->timestamp = timestamp;
             ev->window = event.wheel.windowID;
             ev->event_window = event.wheel.windowID;
             ev->modifiers = 0; /* FIXME: keep modifier around. */
             ev->direction = 0;
             ev->z = event.wheel.x != 0 ? event.wheel.x : event.wheel.y;
             ev->direction = event.wheel.x != 0 ? 0 : 1;

             ecore_event_add(ECORE_EVENT_MOUSE_WHEEL, ev, NULL, NULL);
             break;
          }
          case SDL_MOUSEBUTTONUP:
          {
             Ecore_Event_Mouse_Button *ev;

             ev = calloc(1, sizeof(Ecore_Event_Mouse_Button));
             if (!ev) return;
             ev->timestamp = timestamp;
             ev->window = event.button.windowID;
             ev->event_window = event.button.windowID;
             ev->modifiers = 0; /* FIXME: keep modifier around. */
             ev->buttons = event.button.button;
             ev->double_click = 0;
             ev->triple_click = 0;

             /* Must set multi touch device to 0 or it will get ignored */
             ev->multi.device = 0;
             ev->multi.radius = ev->multi.radius_x = ev->multi.radius_y = 0;
             ev->multi.pressure = ev->multi.angle = 0;
             ev->multi.x = ev->multi.y = ev->multi.root.x = ev->multi.root.y = 0;

             ecore_event_add(ECORE_EVENT_MOUSE_BUTTON_UP, ev, NULL, NULL);
             break;
          }
          case SDL_QUIT:
             ecore_main_loop_quit();
             break;

          case SDL_KEYDOWN:
          {
             Ecore_SDL_Pressed *entry;
             Ecore_Event_Key *ev;
             char text[SDL_TEXTINPUTEVENT_TEXT_SIZE];
             Eina_Bool has_text;

             has_text = _ecore_sdl_key_text_take(event.key.windowID,
                                                 text, sizeof(text));

             entry = (Ecore_SDL_Pressed*) eina_rbtree_inline_lookup(repeat, &event.key.keysym.sym, sizeof (event.key.keysym.sym),
                                                                    EINA_RBTREE_CMP_KEY_CB(_ecore_sdl_pressed_node), NULL);
             if (entry)
               {
                  ev = _ecore_sdl_event_key(&event.key, NULL, timestamp);
                  if (ev) ecore_event_add(ECORE_EVENT_KEY_UP, ev, NULL, NULL);
               }

             ev = _ecore_sdl_event_key(&event.key, has_text ? text : NULL,
                                       timestamp);
             if (ev) ecore_event_add(ECORE_EVENT_KEY_DOWN, ev, NULL, NULL);

             if (!entry)
               {
                  entry = malloc(sizeof (Ecore_SDL_Pressed));
                  if (!entry) break;

                  entry->key = event.key.keysym.sym;

                  repeat = eina_rbtree_inline_insert(repeat, EINA_RBTREE_GET(entry),
                                                     EINA_RBTREE_CMP_NODE_CB(_ecore_sdl_pressed_key), NULL);
               }
             break;
          }
          case SDL_KEYUP:
          {
             Ecore_Event_Key *ev;
             Ecore_SDL_Pressed *entry;

             entry = (Ecore_SDL_Pressed*) eina_rbtree_inline_lookup(repeat, &event.key.keysym.sym, sizeof (event.key.keysym.sym),
                                                                    EINA_RBTREE_CMP_KEY_CB(_ecore_sdl_pressed_node), NULL);
             if (entry)
               {
                  repeat = eina_rbtree_inline_remove(repeat, EINA_RBTREE_GET(entry),
                                                     EINA_RBTREE_CMP_NODE_CB(_ecore_sdl_pressed_key), NULL);
                  free(entry);
               }

             ev = _ecore_sdl_event_key(&event.key, NULL, timestamp);
             if (ev) ecore_event_add(ECORE_EVENT_KEY_UP, ev, NULL, NULL);
             break;
          }
          case SDL_TEXTINPUT:
          {
             /* text that does not follow a key press (from an input
              * method, for instance): deliver it as a key press of its
              * own, so that it is not lost */
             Ecore_Event_Key *ev;
             unsigned int modifiers;

             if (!event.text.text[0]) break;
             modifiers = _ecore_sdl_event_modifiers(SDL_GetModState());
             ev = _ecore_sdl_key_event_new(event.text.windowID,
                                           event.text.text, event.text.text,
                                           event.text.text, modifiers, 0,
                                           timestamp);
             if (ev) ecore_event_add(ECORE_EVENT_KEY_DOWN, ev, NULL, NULL);
             ev = _ecore_sdl_key_event_new(event.text.windowID,
                                           event.text.text, NULL,
                                           event.text.text, modifiers, 0,
                                           timestamp);
             if (ev) ecore_event_add(ECORE_EVENT_KEY_UP, ev, NULL, NULL);
             break;
          }
          case SDL_WINDOWEVENT:
             switch (event.window.event)
               {
                case SDL_WINDOWEVENT_RESIZED:
                  {
                     Ecore_Sdl_Event_Video_Resize *ev;

                     ev = calloc(1, sizeof (Ecore_Sdl_Event_Video_Resize));
                     ev->windowID = event.window.windowID;
                     ev->w = event.window.data1;
                     ev->h = event.window.data2;

                     ecore_event_add(ECORE_SDL_EVENT_RESIZE, ev, NULL, NULL);
                     break;
                  }
                case SDL_WINDOWEVENT_EXPOSED:
                  {
                     Ecore_Sdl_Event_Window *ev;

                     ev = calloc(1, sizeof (Ecore_Sdl_Event_Window));
                     ev->windowID = event.window.windowID;

                     ecore_event_add(ECORE_SDL_EVENT_EXPOSE, ev, NULL, NULL);
                     break;
                  }
                case SDL_WINDOWEVENT_ENTER:
                case SDL_WINDOWEVENT_LEAVE:
                  {
                     Ecore_Event_Mouse_IO *ev;

                     ev = calloc(1, sizeof (Ecore_Event_Mouse_IO));
                     ev->window = event.window.windowID;
                     ev->event_window = event.window.windowID;

                     ecore_event_add(event.window.event == SDL_WINDOWEVENT_ENTER ?
                                     ECORE_EVENT_MOUSE_IN : ECORE_EVENT_MOUSE_OUT,
                                     ev, NULL, NULL);
                     break;
                  }
                case SDL_WINDOWEVENT_FOCUS_GAINED:
                case SDL_WINDOWEVENT_FOCUS_LOST:
                  {
                     Ecore_Sdl_Event_Window *ev;

                     ev = calloc(1, sizeof (Ecore_Sdl_Event_Window));
                     ev->windowID = event.window.windowID;

                     ecore_event_add(event.window.event == SDL_WINDOWEVENT_FOCUS_GAINED ?
                                     ECORE_SDL_EVENT_GOT_FOCUS : ECORE_SDL_EVENT_LOST_FOCUS,
                                     ev, NULL, NULL);
                     break;
                  }
               }
             break;
          case SDL_SYSWMEVENT:
          case SDL_USEREVENT:
          case SDL_JOYAXISMOTION:
          case SDL_JOYBALLMOTION:
          case SDL_JOYHATMOTION:
          case SDL_JOYBUTTONDOWN:
          case SDL_JOYBUTTONUP:
          default:
             break;
          }
     }
}
