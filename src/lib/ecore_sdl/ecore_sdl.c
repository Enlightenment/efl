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

   ecore_event_shutdown();
   eina_log_domain_unregister(_ecore_sdl_log_dom);
   _ecore_sdl_log_dom = -1;
   return _ecore_sdl_init_count;
}

static unsigned int
_ecore_sdl_event_modifiers(int mod)
{
   unsigned int        modifiers = 0;

   if(mod & KMOD_LSHIFT) modifiers |= ECORE_EVENT_MODIFIER_SHIFT;
   if(mod & KMOD_RSHIFT) modifiers |= ECORE_EVENT_MODIFIER_SHIFT;
   if(mod & KMOD_LCTRL) modifiers |= ECORE_EVENT_MODIFIER_CTRL;
   if(mod & KMOD_RCTRL) modifiers |= ECORE_EVENT_MODIFIER_CTRL;
   if(mod & KMOD_LALT) modifiers |= ECORE_EVENT_MODIFIER_ALT;
   if(mod & KMOD_RALT) modifiers |= ECORE_EVENT_MODIFIER_ALT;
   if(mod & KMOD_NUM) modifiers |= ECORE_EVENT_LOCK_NUM;
   if(mod & KMOD_CAPS) modifiers |= ECORE_EVENT_LOCK_CAPS;

   return modifiers;
}

static Ecore_Event_Key*
_ecore_sdl_event_key(SDL_Event *event, double timestamp)
{
   Ecore_Event_Key *ev;
   unsigned int i;

   ev = malloc(sizeof(Ecore_Event_Key));
   if (!ev) return NULL;

   ev->timestamp = timestamp;
   ev->window = event->key.windowID;
   ev->event_window = 0;
   ev->modifiers = _ecore_sdl_event_modifiers(SDL_GetModState());
   ev->key = NULL;
   ev->compose = NULL;

   for (i = 0; i < EINA_C_ARRAY_LENGTH(keystable); ++i)
     if (keystable[i].code == event->key.keysym.sym)
       {
          ev->keyname = keystable[i].name;
          ev->string = keystable[i].compose;

          return ev;
       }

   free(ev);
   return NULL;
}

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

             ev = malloc(sizeof(Ecore_Event_Mouse_Move));
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

             ev = malloc(sizeof(Ecore_Event_Mouse_Button));
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

             ev = malloc(sizeof(Ecore_Event_Mouse_Wheel));
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

             ev = malloc(sizeof(Ecore_Event_Mouse_Button));
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

             entry = (Ecore_SDL_Pressed*) eina_rbtree_inline_lookup(repeat, &event.key.keysym.sym, sizeof (event.key.keysym.sym),
                                                                    EINA_RBTREE_CMP_KEY_CB(_ecore_sdl_pressed_node), NULL);
             if (entry)
               {
                  ev = _ecore_sdl_event_key(&event, timestamp);
                  if (ev) ecore_event_add(ECORE_EVENT_KEY_UP, ev, NULL, NULL);
               }

             ev = _ecore_sdl_event_key(&event, timestamp);
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

             ev = _ecore_sdl_event_key(&event, timestamp);
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
