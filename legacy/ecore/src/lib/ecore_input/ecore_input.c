#include <string.h>

#include "config.h"
#include "Ecore.h"
#include "ecore_private.h"
#include "Ecore_Input.h"

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

static Ecore_Event_Handler *ecore_evas_event_handlers[8];
static Evas_Hash *_ecore_evases_hash = NULL;

static int _ecore_evas_event_init_count = 0;

static const char*
_ecore_evas_winid_str_get(Ecore_Window window)
{
   const char *vals = "qWeRtYuIoP5-$&<~";
   static char id[9];

#define COMPUTE_ID(Result, Index, Source, Window) \
   Result[Index] = Source[(Window >> (28 - Index * 4)) & 0xf];

   COMPUTE_ID(id, 0, vals, window);
   COMPUTE_ID(id, 1, vals, window);
   COMPUTE_ID(id, 2, vals, window);
   COMPUTE_ID(id, 3, vals, window);
   COMPUTE_ID(id, 4, vals, window);
   COMPUTE_ID(id, 5, vals, window);
   COMPUTE_ID(id, 6, vals, window);
   COMPUTE_ID(id, 7, vals, window);
   id[8] = '\0';

   return id;
}

EAPI void
ecore_evas_event_modifier_lock_update(Evas *e, unsigned int modifiers)
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
ecore_evas_register(Ecore_Evas *ee, Ecore_Window window)
{
   _ecore_evases_hash = evas_hash_add(_ecore_evases_hash, _ecore_evas_winid_str_get(window), ee);

   evas_key_modifier_add(ee->evas, "Shift");
   evas_key_modifier_add(ee->evas, "Control");
   evas_key_modifier_add(ee->evas, "Alt");
   evas_key_modifier_add(ee->evas, "Meta");
   evas_key_modifier_add(ee->evas, "Hyper");
   evas_key_modifier_add(ee->evas, "Super");
   evas_key_lock_add(ee->evas, "Caps_Lock");
   evas_key_lock_add(ee->evas, "Num_Lock");
   evas_key_lock_add(ee->evas, "Scroll_Lock");
}

EAPI void
ecore_evas_unregister(Ecore_Evas *ee, Ecore_Window window)
{
   _ecore_evases_hash = evas_hash_del(_ecore_evases_hash, _ecore_evas_winid_str_get(window), ee);
}

EAPI Ecore_Evas*
ecore_evas_window_match(Ecore_Window window)
{
   return evas_hash_find(_ecore_evases_hash, _ecore_evas_winid_str_get(window));
}

EAPI void
ecore_evas_mouse_move_process(Ecore_Evas *ee, int x, int y, unsigned int timestamp)
{
   ee->mouse.x = x;
   ee->mouse.y = y;
   if (ee->prop.cursor.object)
     {
	evas_object_show(ee->prop.cursor.object);
	if (ee->rotation == 0)
	  evas_object_move(ee->prop.cursor.object,
			   x - ee->prop.cursor.hot.x,
			   y - ee->prop.cursor.hot.y);
	else if (ee->rotation == 90)
	  evas_object_move(ee->prop.cursor.object,
			   ee->h - y - 1 - ee->prop.cursor.hot.x,
			   x - ee->prop.cursor.hot.y);
	else if (ee->rotation == 180)
	  evas_object_move(ee->prop.cursor.object,
			   ee->w - x - 1 - ee->prop.cursor.hot.x,
			   ee->h - y - 1 - ee->prop.cursor.hot.y);
	else if (ee->rotation == 270)
	  evas_object_move(ee->prop.cursor.object,
			   y - ee->prop.cursor.hot.x,
			   ee->w - x - 1 - ee->prop.cursor.hot.y);
     }
   if (ee->rotation == 0)
     evas_event_feed_mouse_move(ee->evas, x, y, timestamp, NULL);
   else if (ee->rotation == 90)
     evas_event_feed_mouse_move(ee->evas, ee->h - y - 1, x, timestamp, NULL);
   else if (ee->rotation == 180)
     evas_event_feed_mouse_move(ee->evas, ee->w - x - 1, ee->h - y - 1, timestamp, NULL);
   else if (ee->rotation == 270)
     evas_event_feed_mouse_move(ee->evas, y, ee->w - x - 1, timestamp, NULL);
}

static int
_ecore_evas_event_key(Ecore_Event_Key *e, Ecore_Event_Press press)
{
   Ecore_Evas *ee;

   ee = ecore_evas_window_match(e->window);
   if ((!ee) || (ee->ignore_events)) return 1; /* pass on event */
   ecore_evas_event_modifier_lock_update(ee->evas, e->modifiers);
   if (press == ECORE_DOWN)
     evas_event_feed_key_down(ee->evas, e->keyname, e->key, e->string, e->compose, e->timestamp, NULL);
   else
     evas_event_feed_key_up(ee->evas, e->keyname, e->key, e->string, e->compose, e->timestamp, NULL);
   return 1;
}

static int
_ecore_evas_event_mouse_button(Ecore_Event_Mouse_Button *e, Ecore_Event_Press press)
{
   Ecore_Evas *ee;
   Evas_Button_Flags flags = EVAS_BUTTON_NONE;

   ee = ecore_evas_window_match(e->window);
   if ((!ee) || (ee->ignore_events)) return 1; /* pass on event */
   ecore_evas_event_modifier_lock_update(ee->evas, e->modifiers);
   if (e->double_click) flags |= EVAS_BUTTON_DOUBLE_CLICK;
   if (e->triple_click) flags |= EVAS_BUTTON_TRIPLE_CLICK;
   if (press == ECORE_DOWN)
     evas_event_feed_mouse_down(ee->evas, e->buttons, flags, e->timestamp, NULL);
   else
     evas_event_feed_mouse_up(ee->evas, e->buttons, flags, e->timestamp, NULL);
   return 1;
}

static int
_ecore_evas_event_mouse_io(Ecore_Event_Mouse_IO *e, Ecore_Event_IO io)
{
   Ecore_Evas *ee;

   ee = ecore_evas_window_match(e->window);
   if ((!ee) || (ee->ignore_events)) return 1; /* pass on event */
   ecore_evas_event_modifier_lock_update(ee->evas, e->modifiers);
   switch (io)
     {
      case ECORE_IN:
	 evas_event_feed_mouse_in(ee->evas, e->timestamp, NULL);
	 break;
      case ECORE_OUT:
	 evas_event_feed_mouse_out(ee->evas, e->timestamp, NULL);
	 break;
      default:
	 break;
     }

   ecore_evas_mouse_move_process(ee, e->x, e->y, e->timestamp);
   return 1;
}
#endif

EAPI int
ecore_evas_event_key_down(void *data __UNUSED__, int type __UNUSED__, void *event)
{
#ifdef BUILD_ECORE_EVAS
   return _ecore_evas_event_key((Ecore_Event_Key*) event, ECORE_DOWN);
#else
   return 0;
#endif
}

EAPI int
ecore_evas_event_key_up(void *data __UNUSED__, int type __UNUSED__, void *event)
{
#ifdef BUILD_ECORE_EVAS
   return _ecore_evas_event_key((Ecore_Event_Key*) event, ECORE_UP);
#else
   return 0;
#endif
}

EAPI int
ecore_evas_event_mouse_button_down(void *data __UNUSED__, int type __UNUSED__, void *event)
{
#ifdef BUILD_ECORE_EVAS
   return _ecore_evas_event_mouse_button((Ecore_Event_Mouse_Button*) event, ECORE_DOWN);
#else
   return 0;
#endif
}

EAPI int
ecore_evas_event_mouse_button_up(void *data __UNUSED__, int type __UNUSED__, void *event)
{
#ifdef BUILD_ECORE_EVAS
   return _ecore_evas_event_mouse_button((Ecore_Event_Mouse_Button*) event, ECORE_UP);
#else
   return 0;
#endif
}

EAPI int
ecore_evas_event_mouse_wheel(void *data __UNUSED__, int type __UNUSED__, void *event)
{
#ifdef BUILD_ECORE_EVAS
   Ecore_Event_Mouse_Wheel *e;
   Ecore_Evas *ee;

   e = event;
   ee = ecore_evas_window_match(e->window);
   if ((!ee) || (ee->ignore_events)) return 1; /* pass on event */
   ecore_evas_event_modifier_lock_update(ee->evas, e->modifiers);
   evas_event_feed_mouse_wheel(ee->evas, e->direction, e->z, e->timestamp, NULL);

   return 1;
#else
   return 0;
#endif
}

EAPI int
ecore_evas_event_mouse_move(void *data __UNUSED__, int type __UNUSED__, void *event)
{
#ifdef BUILD_ECORE_EVAS
   Ecore_Event_Mouse_Move *e;
   Ecore_Evas *ee;

   e = event;
   ee = ecore_evas_window_match(e->window);
   if ((!ee) || (ee->ignore_events)) return 1; /* pass on event */
   ecore_evas_event_modifier_lock_update(ee->evas, e->modifiers);
   ecore_evas_mouse_move_process(ee, e->x, e->y, e->timestamp);
   return 1;
#else
   return 0;
#endif
}

EAPI int
ecore_evas_event_mouse_in(void *data __UNUSED__, int type __UNUSED__, void *event)
{
#ifdef BUILD_ECORE_EVAS
   return _ecore_evas_event_mouse_io((Ecore_Event_Mouse_IO*) event, ECORE_IN);
#else
   return 0;
#endif
}

EAPI int
ecore_evas_event_mouse_out(void *data __UNUSED__, int type __UNUSED__, void *event)
{
#ifdef BUILD_ECORE_EVAS
   return _ecore_evas_event_mouse_io((Ecore_Event_Mouse_IO*) event, ECORE_OUT);
#else
   return 0;
#endif
}

EAPI int
ecore_evas_event_init(void)
{
#ifdef BUILD_ECORE_EVAS
   if (!_ecore_evas_event_init_count)
     {
	ecore_event_init();

	ecore_evas_event_handlers[0] = ecore_event_handler_add(ECORE_EVENT_KEY_DOWN,
							       ecore_evas_event_key_down,
							       NULL);
	ecore_evas_event_handlers[1] = ecore_event_handler_add(ECORE_EVENT_KEY_UP,
							       ecore_evas_event_key_up,
							       NULL);
	ecore_evas_event_handlers[2] = ecore_event_handler_add(ECORE_EVENT_MOUSE_BUTTON_DOWN,
							       ecore_evas_event_mouse_button_down,
							       NULL);
	ecore_evas_event_handlers[3] = ecore_event_handler_add(ECORE_EVENT_MOUSE_BUTTON_UP,
							       ecore_evas_event_mouse_button_up,
							       NULL);
	ecore_evas_event_handlers[4] = ecore_event_handler_add(ECORE_EVENT_MOUSE_MOVE,
							       ecore_evas_event_mouse_move,
							       NULL);
	ecore_evas_event_handlers[5] = ecore_event_handler_add(ECORE_EVENT_MOUSE_WHEEL,
							       ecore_evas_event_mouse_wheel,
							       NULL);
	ecore_evas_event_handlers[6] = ecore_event_handler_add(ECORE_EVENT_MOUSE_IN,
							       ecore_evas_event_mouse_in,
							       NULL);
	ecore_evas_event_handlers[7] = ecore_event_handler_add(ECORE_EVENT_MOUSE_OUT,
							       ecore_evas_event_mouse_out,
							       NULL);
     }
   return ++_ecore_evas_event_init_count;
#else
   return 0;
#endif
}

EAPI int
ecore_evas_event_shutdown(void)
{
#ifdef BUILD_ECORE_EVAS
   if (_ecore_evas_event_init_count == 1)
     {
	int i;

	for (i = 0; i < sizeof(ecore_evas_event_handlers)/sizeof(Ecore_Event_Handler*); ++i)
	  {
	     ecore_event_handler_del(ecore_evas_event_handlers[i]);
	     ecore_evas_event_handlers[i] = NULL;
	  }

	ecore_event_shutdown();
     }
   return --_ecore_evas_event_init_count;
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
