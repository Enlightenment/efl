#include <string.h>

#include "config.h"
#include "Ecore.h"
#include "ecore_private.h"
#include "Ecore_Input.h"
#include "Evas.h"

#include <stdio.h>

typedef struct _Ecore_Input_Window Ecore_Input_Window;
struct _Ecore_Input_Window
{
   Evas *evas;
   void *window;
   Ecore_Event_Mouse_Move_Cb move_mouse;
   int ignore_event;
};

EAPI int ECORE_EVENT_KEY_DOWN = 0;
EAPI int ECORE_EVENT_KEY_UP = 0;
EAPI int ECORE_EVENT_MOUSE_BUTTON_DOWN = 0;
EAPI int ECORE_EVENT_MOUSE_BUTTON_UP = 0;
EAPI int ECORE_EVENT_MOUSE_MOVE = 0;
EAPI int ECORE_EVENT_MOUSE_WHEEL = 0;
EAPI int ECORE_EVENT_MOUSE_IN = 0;
EAPI int ECORE_EVENT_MOUSE_OUT = 0;

static int _ecore_event_init_count = 0;

#ifdef BUILD_ECORE_EVAS
#include "ecore_evas_private.h"

static Ecore_Event_Handler *ecore_event_evas_handlers[8];
static Eina_Hash *_window_hash = NULL;

static int _ecore_event_evas_init_count = 0;

EAPI void
ecore_event_evas_modifier_lock_update(Evas *e, unsigned int modifiers)
{
   if (modifiers & ECORE_EVENT_MODIFIER_SHIFT)
     evas_key_modifier_on(e, "Shift");
   else evas_key_modifier_off(e, "Shift");

   if (modifiers & ECORE_EVENT_MODIFIER_CTRL)
     evas_key_modifier_on(e, "Control");
   else evas_key_modifier_off(e, "Control");

   if (modifiers & ECORE_EVENT_MODIFIER_ALT)
     evas_key_modifier_on(e, "Alt");
   else evas_key_modifier_off(e, "Alt");

   if (modifiers & ECORE_EVENT_MODIFIER_WIN)
     {
	evas_key_modifier_on(e, "Super");
	evas_key_modifier_on(e, "Hyper");
     }
   else
     {
	evas_key_modifier_off(e, "Super");
	evas_key_modifier_off(e, "Hyper");
     }

   if (modifiers & ECORE_EVENT_LOCK_SCROLL)
     evas_key_lock_on(e, "Scroll_Lock");
   else evas_key_lock_off(e, "Scroll_Lock");

   if (modifiers & ECORE_EVENT_LOCK_NUM)
     evas_key_lock_on(e, "Num_Lock");
   else evas_key_lock_off(e, "Num_Lock");

   if (modifiers & ECORE_EVENT_LOCK_CAPS)
     evas_key_lock_on(e, "Caps_Lock");
   else evas_key_lock_off(e, "Caps_Lock");
}

EAPI void
ecore_event_window_register(Ecore_Window id, void *window, Evas *evas, Ecore_Event_Mouse_Move_Cb move_mouse)
{
   Ecore_Input_Window *new;

   new = malloc(sizeof (Ecore_Input_Window));
   if (!new) return ;

   new->evas = evas;
   new->window = window;
   new->move_mouse = move_mouse;
   new->ignore_event = 0;

   eina_hash_add(_window_hash, &id, new);

   evas_key_modifier_add(evas, "Shift");
   evas_key_modifier_add(evas, "Control");
   evas_key_modifier_add(evas, "Alt");
   evas_key_modifier_add(evas, "Meta");
   evas_key_modifier_add(evas, "Hyper");
   evas_key_modifier_add(evas, "Super");
   evas_key_lock_add(evas, "Caps_Lock");
   evas_key_lock_add(evas, "Num_Lock");
   evas_key_lock_add(evas, "Scroll_Lock");
}

EAPI void
ecore_event_window_unregister(Ecore_Window id)
{
   eina_hash_del(_window_hash, &id, NULL);
}

EAPI void*
ecore_event_window_match(Ecore_Window id)
{
   Ecore_Input_Window *lookup;

   lookup = eina_hash_find(_window_hash, &id);
   if (lookup) return lookup->window;
   return NULL;
}

EAPI void
ecore_event_window_ignore_events(Ecore_Window id, int ignore_event)
{
   Ecore_Input_Window *lookup;

   lookup = eina_hash_find(_window_hash, &id);
   if (!lookup) return ;
   lookup->ignore_event = ignore_event;
}

static Ecore_Input_Window*
_ecore_event_window_match(Ecore_Window id)
{
   Ecore_Input_Window *lookup;

   lookup = eina_hash_find(_window_hash, &id);
   if (!lookup) return NULL;
   if (lookup->ignore_event) return NULL; /* Pass on event. */
   return lookup;
}

static int
_ecore_event_evas_key(Ecore_Event_Key *e, Ecore_Event_Press press)
{
   Ecore_Input_Window *lookup;

   lookup = _ecore_event_window_match(e->window);
   if (!lookup) return 1;
   ecore_event_evas_modifier_lock_update(lookup->evas, e->modifiers);
   if (press == ECORE_DOWN)
     evas_event_feed_key_down(lookup->evas, e->keyname, e->key, e->string, e->compose, e->timestamp, NULL);
   else
     evas_event_feed_key_up(lookup->evas, e->keyname, e->key, e->string, e->compose, e->timestamp, NULL);
   return 1;
}

static int
_ecore_event_evas_mouse_button(Ecore_Event_Mouse_Button *e, Ecore_Event_Press press)
{
   Ecore_Input_Window *lookup;
   Evas_Button_Flags flags = EVAS_BUTTON_NONE;

   lookup = _ecore_event_window_match(e->window);
   if (!lookup) return 1;
   ecore_event_evas_modifier_lock_update(lookup->evas, e->modifiers);
   if (e->double_click) flags |= EVAS_BUTTON_DOUBLE_CLICK;
   if (e->triple_click) flags |= EVAS_BUTTON_TRIPLE_CLICK;
   if (press == ECORE_DOWN)
     evas_event_feed_mouse_down(lookup->evas, e->buttons, flags, e->timestamp, NULL);
   else
     evas_event_feed_mouse_up(lookup->evas, e->buttons, flags, e->timestamp, NULL);
   return 1;
}

static int
_ecore_event_evas_mouse_io(Ecore_Event_Mouse_IO *e, Ecore_Event_IO io)
{
   Ecore_Input_Window *lookup;

   lookup = _ecore_event_window_match(e->window);
   if (!lookup) return 1;
   ecore_event_evas_modifier_lock_update(lookup->evas, e->modifiers);
   switch (io)
     {
      case ECORE_IN:
	 evas_event_feed_mouse_in(lookup->evas, e->timestamp, NULL);
	 break;
      case ECORE_OUT:
	 evas_event_feed_mouse_out(lookup->evas, e->timestamp, NULL);
	 break;
      default:
	 break;
     }

   lookup->move_mouse(lookup->window, e->x, e->y, e->timestamp);
   return 1;
}
#endif

EAPI int
ecore_event_evas_key_down(void *data __UNUSED__, int type __UNUSED__, void *event)
{
#ifdef BUILD_ECORE_EVAS
   return _ecore_event_evas_key((Ecore_Event_Key*) event, ECORE_DOWN);
#else
   return 0;
#endif
}

EAPI int
ecore_event_evas_key_up(void *data __UNUSED__, int type __UNUSED__, void *event)
{
#ifdef BUILD_ECORE_EVAS
   return _ecore_event_evas_key((Ecore_Event_Key*) event, ECORE_UP);
#else
   return 0;
#endif
}

EAPI int
ecore_event_evas_mouse_button_down(void *data __UNUSED__, int type __UNUSED__, void *event)
{
#ifdef BUILD_ECORE_EVAS
   return _ecore_event_evas_mouse_button((Ecore_Event_Mouse_Button*) event, ECORE_DOWN);
#else
   return 0;
#endif
}

EAPI int
ecore_event_evas_mouse_button_up(void *data __UNUSED__, int type __UNUSED__, void *event)
{
#ifdef BUILD_ECORE_EVAS
   return _ecore_event_evas_mouse_button((Ecore_Event_Mouse_Button*) event, ECORE_UP);
#else
   return 0;
#endif
}

EAPI int
ecore_event_evas_mouse_wheel(void *data __UNUSED__, int type __UNUSED__, void *event)
{
#ifdef BUILD_ECORE_EVAS
   Ecore_Event_Mouse_Wheel *e;
   Ecore_Input_Window *lookup;

   e = event;
   lookup = _ecore_event_window_match(e->window);
   if (!lookup) return 1;
   ecore_event_evas_modifier_lock_update(lookup->evas, e->modifiers);
   evas_event_feed_mouse_wheel(lookup->evas, e->direction, e->z, e->timestamp, NULL);

   return 1;
#else
   return 0;
#endif
}

EAPI int
ecore_event_evas_mouse_move(void *data __UNUSED__, int type __UNUSED__, void *event)
{
#ifdef BUILD_ECORE_EVAS
   Ecore_Event_Mouse_Move *e;
   Ecore_Input_Window *lookup;

   e = event;
   lookup = _ecore_event_window_match(e->window);
   if (!lookup) return 1;
   ecore_event_evas_modifier_lock_update(lookup->evas, e->modifiers);
   lookup->move_mouse(lookup->window, e->x, e->y, e->timestamp);
   return 1;
#else
   return 0;
#endif
}

EAPI int
ecore_event_evas_mouse_in(void *data __UNUSED__, int type __UNUSED__, void *event)
{
#ifdef BUILD_ECORE_EVAS
   return _ecore_event_evas_mouse_io((Ecore_Event_Mouse_IO*) event, ECORE_IN);
#else
   return 0;
#endif
}

EAPI int
ecore_event_evas_mouse_out(void *data __UNUSED__, int type __UNUSED__, void *event)
{
#ifdef BUILD_ECORE_EVAS
   return _ecore_event_evas_mouse_io((Ecore_Event_Mouse_IO*) event, ECORE_OUT);
#else
   return 0;
#endif
}

EAPI int
ecore_event_evas_init(void)
{
#ifdef BUILD_ECORE_EVAS
   if (!_ecore_event_evas_init_count)
     {
	ecore_init();
	ecore_event_init();

	ecore_event_evas_handlers[0] = ecore_event_handler_add(ECORE_EVENT_KEY_DOWN,
							       ecore_event_evas_key_down,
							       NULL);
	ecore_event_evas_handlers[1] = ecore_event_handler_add(ECORE_EVENT_KEY_UP,
							       ecore_event_evas_key_up,
							       NULL);
	ecore_event_evas_handlers[2] = ecore_event_handler_add(ECORE_EVENT_MOUSE_BUTTON_DOWN,
							       ecore_event_evas_mouse_button_down,
							       NULL);
	ecore_event_evas_handlers[3] = ecore_event_handler_add(ECORE_EVENT_MOUSE_BUTTON_UP,
							       ecore_event_evas_mouse_button_up,
							       NULL);
	ecore_event_evas_handlers[4] = ecore_event_handler_add(ECORE_EVENT_MOUSE_MOVE,
							       ecore_event_evas_mouse_move,
							       NULL);
	ecore_event_evas_handlers[5] = ecore_event_handler_add(ECORE_EVENT_MOUSE_WHEEL,
							       ecore_event_evas_mouse_wheel,
							       NULL);
	ecore_event_evas_handlers[6] = ecore_event_handler_add(ECORE_EVENT_MOUSE_IN,
							       ecore_event_evas_mouse_in,
							       NULL);
	ecore_event_evas_handlers[7] = ecore_event_handler_add(ECORE_EVENT_MOUSE_OUT,
							       ecore_event_evas_mouse_out,
							       NULL);

	_window_hash = eina_hash_pointer_new(free);
     }
   return ++_ecore_event_evas_init_count;
#else
   return 0;
#endif
}

EAPI int
ecore_event_evas_shutdown(void)
{
#ifdef BUILD_ECORE_EVAS
   if (_ecore_event_evas_init_count == 1)
     {
	int i;

	for (i = 0; i < sizeof(ecore_event_evas_handlers)/sizeof(Ecore_Event_Handler*); ++i)
	  {
	     ecore_event_handler_del(ecore_event_evas_handlers[i]);
	     ecore_event_evas_handlers[i] = NULL;
	  }

	ecore_event_shutdown();
	ecore_shutdown();
     }
   return --_ecore_event_evas_init_count;
#else
   return 0;
#endif
}

typedef struct _Ecore_Event_Modifier_Match Ecore_Event_Modifier_Match;
struct _Ecore_Event_Modifier_Match
{
   const char *key;
   Ecore_Event_Modifier modifier;
   unsigned int event_modifier;
};

static const Ecore_Event_Modifier_Match matchs[] = {
  { "Shift_L", ECORE_SHIFT, ECORE_EVENT_MODIFIER_SHIFT },
  { "Shift_R", ECORE_SHIFT, ECORE_EVENT_MODIFIER_SHIFT },
  { "Alt_L", ECORE_ALT, ECORE_EVENT_MODIFIER_ALT },
  { "Alt_R", ECORE_ALT, ECORE_EVENT_MODIFIER_ALT },
  { "Control_L", ECORE_CTRL, ECORE_EVENT_MODIFIER_CTRL },
  { "Control_R", ECORE_CTRL, ECORE_EVENT_MODIFIER_CTRL },
  { "Caps_Lock", ECORE_CAPS, ECORE_EVENT_MODIFIER_CAPS },
  { "Super_L", ECORE_WIN, ECORE_EVENT_MODIFIER_WIN },
  { "Super_R", ECORE_WIN, ECORE_EVENT_MODIFIER_WIN },
  { "Scroll_Lock", ECORE_SCROLL, ECORE_EVENT_MODIFIER_SCROLL }
};

EAPI unsigned int
ecore_event_modifier_mask(Ecore_Event_Modifier modifier)
{
   int i;

   for (i = 0; i < sizeof (matchs) / sizeof (Ecore_Event_Modifier_Match); i++)
     if (matchs[i].modifier == modifier)
       return matchs[i].event_modifier;

   return 0;
}

EAPI Ecore_Event_Modifier
ecore_event_update_modifier(const char *key, Ecore_Event_Modifiers *modifiers, int inc)
{
   int i;

   for (i = 0; i < sizeof (matchs) / sizeof (Ecore_Event_Modifier_Match); i++)
     if (strcmp(matchs[i].key, key) == 0)
       {
	  if (modifiers && matchs[i].modifier < modifiers->size)
	    modifiers->array[matchs[i].modifier] += inc;
	  return matchs[i].modifier;
       }

   return ECORE_NONE;
}

EAPI int
ecore_event_init(void)
{
   if (!_ecore_event_init_count)
     {
	ECORE_EVENT_KEY_DOWN = ecore_event_type_new();
	ECORE_EVENT_KEY_UP = ecore_event_type_new();
	ECORE_EVENT_MOUSE_BUTTON_DOWN = ecore_event_type_new();
	ECORE_EVENT_MOUSE_BUTTON_UP = ecore_event_type_new();
	ECORE_EVENT_MOUSE_MOVE = ecore_event_type_new();
	ECORE_EVENT_MOUSE_WHEEL = ecore_event_type_new();
	ECORE_EVENT_MOUSE_IN = ecore_event_type_new();
	ECORE_EVENT_MOUSE_OUT = ecore_event_type_new();
     }
   return ++_ecore_event_init_count;
}

EAPI int
ecore_event_shutdown(void)
{
   if (_ecore_event_init_count == 1)
     {
	ECORE_EVENT_KEY_DOWN = 0;
	ECORE_EVENT_KEY_UP = 0;
	ECORE_EVENT_MOUSE_BUTTON_DOWN = 0;
	ECORE_EVENT_MOUSE_BUTTON_UP = 0;
	ECORE_EVENT_MOUSE_MOVE = 0;
	ECORE_EVENT_MOUSE_WHEEL = 0;
	ECORE_EVENT_MOUSE_IN = 0;
	ECORE_EVENT_MOUSE_OUT = 0;
     }
   return ++_ecore_event_init_count;
}
