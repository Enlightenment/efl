/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <SDL/SDL.h>

#include "Eina.h"
#include "Ecore_Sdl.h"
#include "ecore_private.h"
#include "Ecore_Input.h"
#include "Ecore.h"
#include "Ecore_Data.h"
#include "Ecore_Sdl_Keys.h"

#include <eina_rbtree.h>

typedef struct _Ecore_SDL_Pressed Ecore_SDL_Pressed;
struct _Ecore_SDL_Pressed
{
   EINA_RBTREE;

   SDLKey key;
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
		       __UNUSED__ void *data)
{
   return left->key < right->key ? EINA_RBTREE_LEFT : EINA_RBTREE_RIGHT;
}

static int
_ecore_sdl_pressed_node(const Ecore_SDL_Pressed *node,
			const SDLKey *key,
			__UNUSED__ int length,
			__UNUSED__ void *data)
{
   return node->key - *key;
}

/**
 * @defgroup Ecore_Sdl_Library_Group Framebuffer Library Functions
 *
 * Functions used to set up and shut down the Ecore_Framebuffer functions.
 */

/**
 * Sets up the Ecore_Sdl library.
 * @param   name device target name
 * @return  @c 0 on failure.  Otherwise, the number of times the library has
 *          been initialised without being shut down.
 * @ingroup Ecore_SDL_Library_Group
 */
EAPI int
ecore_sdl_init(const char *name __UNUSED__)
{
   if(!_ecore_sdl_init_count)
     {
	ECORE_SDL_EVENT_GOT_FOCUS         = ecore_event_type_new();
	ECORE_SDL_EVENT_LOST_FOCUS        = ecore_event_type_new();
	ECORE_SDL_EVENT_RESIZE            = ecore_event_type_new();
	ECORE_SDL_EVENT_EXPOSE            = ecore_event_type_new();

	SDL_EnableKeyRepeat(200, 100);
     }
   ecore_event_init();
   return ++_ecore_sdl_init_count;
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
   _ecore_sdl_init_count--;
   ecore_event_shutdown();
   return _ecore_sdl_init_count;
}

static Ecore_Event_Key*
_ecore_sdl_event_key(SDL_Event *event, double time)
{
   Ecore_Event_Key *ev;
   unsigned int i;

   ev = malloc(sizeof(Ecore_Event_Key));
   if (!ev) return NULL;

   ev->timestamp = time;
   ev->window = 0;
   ev->modifiers = 0; /* FIXME: keep modifier around. */
   ev->key = NULL;
   ev->compose = NULL;

   for (i = 0; i < sizeof(keystable) / sizeof(struct _ecore_sdl_keys_s); ++i)
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
   unsigned int time;

   while(SDL_PollEvent(&event))
     {
        time = (unsigned int)((unsigned long long)(ecore_time_get() * 1000.0) & 0xffffffff);
        switch(event.type)
          {
          case SDL_MOUSEMOTION:
          {
	     Ecore_Event_Mouse_Move *ev;

	     ev = malloc(sizeof(Ecore_Event_Mouse_Move));
	     if (!ev) return ;

	     ev->timestamp = time;
	     ev->window = 0;
	     ev->modifiers = 0; /* FIXME: keep modifier around. */
             ev->x = event.motion.x;
             ev->y = event.motion.y;
	     ev->root.x = ev->x;
	     ev->root.y = ev->y;

             ecore_event_add(ECORE_EVENT_MOUSE_MOVE, ev, NULL, NULL);
             break;
          }
          case SDL_MOUSEBUTTONDOWN:
          {
             if (event.button.button == SDL_BUTTON_WHEELUP ||
                 event.button.button == SDL_BUTTON_WHEELDOWN)
               {
                  Ecore_Event_Mouse_Wheel *ev;

                  ev = malloc(sizeof(Ecore_Event_Mouse_Wheel));
		  if (!ev) return ;

		  ev->timestamp = time;
		  ev->window = 0;
		  ev->modifiers = 0; /* FIXME: keep modifier around. */
                  ev->direction = 0;
                  ev->z = event.button.button == SDL_BUTTON_WHEELDOWN ? -1 : 1;

                  ecore_event_add(ECORE_EVENT_MOUSE_WHEEL, ev, NULL, NULL);
               }
             else
               {
                  Ecore_Event_Mouse_Button *ev;

                  ev = malloc(sizeof(Ecore_Event_Mouse_Button));
		  if (!ev) return ;

		  ev->timestamp = time;
		  ev->window = 0;
		  ev->modifiers = 0; /* FIXME: keep modifier around. */
                  ev->buttons = event.button.button;
                  ev->double_click = 0;
                  ev->triple_click = 0;

                  ecore_event_add(ECORE_EVENT_MOUSE_BUTTON_DOWN, ev, NULL, NULL);
               }
             break;
          }
          case SDL_MOUSEBUTTONUP:
          {
             Ecore_Event_Mouse_Button *ev;

             ev = malloc(sizeof(Ecore_Event_Mouse_Button));
	     if (!ev) return ;
	     ev->timestamp = time;
	     ev->window = 0;
	     ev->modifiers = 0; /* FIXME: keep modifier around. */
             ev->buttons = event.button.button;
             ev->double_click = 0;
             ev->triple_click = 0;

             ecore_event_add(ECORE_EVENT_MOUSE_BUTTON_UP, ev, NULL, NULL);
             break;
          }
          case SDL_VIDEORESIZE:
          {
             Ecore_Sdl_Event_Video_Resize       *ev;

             ev = malloc(sizeof (Ecore_Sdl_Event_Video_Resize));
             ev->w = event.resize.w;
             ev->h = event.resize.h;

             ecore_event_add(ECORE_SDL_EVENT_RESIZE, ev, NULL, NULL);
             break;
          }
          case SDL_VIDEOEXPOSE:
             ecore_event_add(ECORE_SDL_EVENT_EXPOSE, NULL, NULL, NULL);
             break;
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
		  ev = _ecore_sdl_event_key(&event, time);
		  if (ev) ecore_event_add(ECORE_EVENT_KEY_UP, ev, NULL, NULL);
	       }

	     ev = _ecore_sdl_event_key(&event, time);
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

	     ev = _ecore_sdl_event_key(&event, time);
	     if (ev) ecore_event_add(ECORE_EVENT_KEY_UP, ev, NULL, NULL);
             break;
          }
          case SDL_ACTIVEEVENT:
             /* FIXME: Focus gain. */
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
