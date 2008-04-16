#include <SDL/SDL.h>

#include "Ecore_Sdl.h"
#include "ecore_private.h"
#include "Ecore.h"
#include "Ecore_Data.h"
#include "Ecore_Sdl_Keys.h"

EAPI int ECORE_SDL_EVENT_KEY_DOWN = 0;
EAPI int ECORE_SDL_EVENT_KEY_UP = 0;
EAPI int ECORE_SDL_EVENT_MOUSE_BUTTON_DOWN = 0;
EAPI int ECORE_SDL_EVENT_MOUSE_BUTTON_UP = 0;
EAPI int ECORE_SDL_EVENT_MOUSE_MOVE = 0;
EAPI int ECORE_SDL_EVENT_MOUSE_WHEEL = 0;
EAPI int ECORE_SDL_EVENT_GOT_FOCUS = 0;
EAPI int ECORE_SDL_EVENT_LOST_FOCUS = 0;
EAPI int ECORE_SDL_EVENT_RESIZE = 0;
EAPI int ECORE_SDL_EVENT_EXPOSE = 0;

static int _ecore_sdl_init_count = 0;

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
		ECORE_SDL_EVENT_KEY_DOWN          = ecore_event_type_new();
		ECORE_SDL_EVENT_KEY_UP            = ecore_event_type_new();
		ECORE_SDL_EVENT_MOUSE_BUTTON_DOWN = ecore_event_type_new();
		ECORE_SDL_EVENT_MOUSE_BUTTON_UP   = ecore_event_type_new();
		ECORE_SDL_EVENT_MOUSE_MOVE        = ecore_event_type_new();
		ECORE_SDL_EVENT_MOUSE_WHEEL       = ecore_event_type_new();
                ECORE_SDL_EVENT_GOT_FOCUS         = ecore_event_type_new();
                ECORE_SDL_EVENT_LOST_FOCUS        = ecore_event_type_new();
                ECORE_SDL_EVENT_RESIZE            = ecore_event_type_new();
                ECORE_SDL_EVENT_EXPOSE            = ecore_event_type_new();
	}
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
	return _ecore_sdl_init_count;
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
             Ecore_Sdl_Event_Mouse_Move *ev;

             ev = malloc(sizeof(Ecore_Sdl_Event_Mouse_Move));
             ev->x = event.motion.x;
             ev->y = event.motion.y;
             ev->time = time;

             ecore_event_add(ECORE_SDL_EVENT_MOUSE_MOVE, ev, NULL, NULL);
             break;
          }
          case SDL_MOUSEBUTTONDOWN:
          {
             if (event.button.button == SDL_BUTTON_WHEELUP ||
                 event.button.button == SDL_BUTTON_WHEELDOWN)
               {
                  Ecore_Sdl_Event_Mouse_Wheel   *ev;

                  ev = malloc(sizeof (Ecore_Sdl_Event_Mouse_Wheel));
                  ev->x = event.button.x;
                  ev->y = event.button.y;
                  ev->direction = 0;
                  ev->wheel = event.button.button == SDL_BUTTON_WHEELDOWN ? -1 : 1;
                  ev->time = time;

                  ecore_event_add(ECORE_SDL_EVENT_MOUSE_WHEEL, ev, NULL, NULL);
               }
             else
               {
                  Ecore_Sdl_Event_Mouse_Button_Down  *ev;

                  ev = malloc(sizeof (Ecore_Sdl_Event_Mouse_Button_Down));
                  ev->x = event.button.x;
                  ev->y = event.button.y;
                  ev->button = event.button.button;
                  ev->double_click = 0;
                  ev->triple_click = 0;
                  ev->time = time;

                  ecore_event_add(ECORE_SDL_EVENT_MOUSE_BUTTON_DOWN, ev, NULL, NULL);
               }
             break;
          }
          case SDL_MOUSEBUTTONUP:
          {
             Ecore_Sdl_Event_Mouse_Button_Up  *ev;

             ev = malloc(sizeof (Ecore_Sdl_Event_Mouse_Button_Up));
             ev->x = event.button.x;
             ev->y = event.button.y;
             ev->button = event.button.button;
             ev->double_click = 0;
             ev->triple_click = 0;
             ev->time = time;

             ecore_event_add(ECORE_SDL_EVENT_MOUSE_BUTTON_UP, ev, NULL, NULL);
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
             Ecore_Sdl_Event_Key_Down   *ev;
             unsigned int               i;

             ev = malloc(sizeof (Ecore_Sdl_Event_Key_Down));
             ev->time = time;

             for (i = 0; i < sizeof (keystable) / sizeof (struct _ecore_sdl_keys_s); ++i)
               if (keystable[i].code == event.key.keysym.sym)
                 {
                    ev->keyname = keystable[i].name;
                    ev->keycompose = keystable[i].compose;

                    ecore_event_add(ECORE_SDL_EVENT_KEY_DOWN, ev, NULL, NULL);
                    return ;
                 }

             free(ev);
             break;
          }
          case SDL_KEYUP:
          {
             Ecore_Sdl_Event_Key_Up     *ev;
             unsigned int               i;

             ev = malloc(sizeof (Ecore_Sdl_Event_Key_Up));
             ev->time = time;

             for (i = 0; i < sizeof (keystable) / sizeof (struct _ecore_sdl_keys_s); ++i)
               if (keystable[i].code == event.key.keysym.sym)
                 {
                    ev->keyname = keystable[i].name;
                    ev->keycompose = keystable[i].compose;

                    ecore_event_add(ECORE_SDL_EVENT_KEY_UP, ev, NULL, NULL);
                    return ;
                 }

             free(ev);
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
