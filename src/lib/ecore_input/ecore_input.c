#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Ecore.h"
#include "ecore_private.h"

#include "Ecore_Input.h"
#include "ecore_input_private.h"

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
  { "ISO_Level3_Shift", ECORE_MODE, ECORE_EVENT_MODIFIER_ALTGR },
  { "Scroll_Lock", ECORE_SCROLL, ECORE_EVENT_MODIFIER_SCROLL }
};

/* local variables */
static int _ecore_input_init_count = 0;

/* externval variables */
int _ecore_input_log_dom = -1;

EAPI int ECORE_EVENT_KEY_DOWN = 0;
EAPI int ECORE_EVENT_KEY_UP = 0;
EAPI int ECORE_EVENT_MOUSE_BUTTON_DOWN = 0;
EAPI int ECORE_EVENT_MOUSE_BUTTON_UP = 0;
EAPI int ECORE_EVENT_MOUSE_MOVE = 0;
EAPI int ECORE_EVENT_MOUSE_WHEEL = 0;
EAPI int ECORE_EVENT_MOUSE_IN = 0;
EAPI int ECORE_EVENT_MOUSE_OUT = 0;

/* API functions */
EAPI int 
ecore_input_init(void)
{
   if (++_ecore_input_init_count != 1)
     return _ecore_input_init_count;

   if (!ecore_init())
     {
        ERR("Could not init Ecore");
        _ecore_input_init_count--;
        return 0;
     }

   _ecore_input_log_dom = 
     eina_log_domain_register("ecore_input", ECORE_INPUT_DEFAULT_LOG_COLOR);

   if (_ecore_input_log_dom < 0)
     {
        EINA_LOG_ERR("Impossible to create a log domain for the ecore input module.");
        return --_ecore_input_init_count;
     }

   ECORE_EVENT_KEY_DOWN = ecore_event_type_new();
   ECORE_EVENT_KEY_UP = ecore_event_type_new();
   ECORE_EVENT_MOUSE_BUTTON_DOWN = ecore_event_type_new();
   ECORE_EVENT_MOUSE_BUTTON_UP = ecore_event_type_new();
   ECORE_EVENT_MOUSE_MOVE = ecore_event_type_new();
   ECORE_EVENT_MOUSE_WHEEL = ecore_event_type_new();
   ECORE_EVENT_MOUSE_IN = ecore_event_type_new();
   ECORE_EVENT_MOUSE_OUT = ecore_event_type_new();

   return _ecore_input_init_count;
}

EAPI int 
ecore_input_shutdown(void)
{
   if (--_ecore_input_init_count != 0)
     return _ecore_input_init_count;

   ECORE_EVENT_KEY_DOWN = 0;
   ECORE_EVENT_KEY_UP = 0;
   ECORE_EVENT_MOUSE_BUTTON_DOWN = 0;
   ECORE_EVENT_MOUSE_BUTTON_UP = 0;
   ECORE_EVENT_MOUSE_MOVE = 0;
   ECORE_EVENT_MOUSE_WHEEL = 0;
   ECORE_EVENT_MOUSE_IN = 0;
   ECORE_EVENT_MOUSE_OUT = 0;

   eina_log_domain_unregister(_ecore_input_log_dom);
   _ecore_input_log_dom = -1;

   ecore_shutdown();

   return _ecore_input_init_count;
}

EAPI unsigned int
ecore_input_event_modifier_mask(Ecore_Event_Modifier modifier)
{
   size_t i;

   for (i = 0; i < sizeof (matchs) / sizeof (Ecore_Event_Modifier_Match); i++)
     if (matchs[i].modifier == modifier)
       return matchs[i].event_modifier;

   return 0;
}

EAPI Ecore_Event_Modifier
ecore_input_event_update_modifier(const char *key, Ecore_Event_Modifiers *modifiers, int inc)
{
   size_t i;

   for (i = 0; i < sizeof (matchs) / sizeof (Ecore_Event_Modifier_Match); i++)
     if (strcmp(matchs[i].key, key) == 0)
       {
          if (modifiers && matchs[i].modifier < modifiers->size)
            modifiers->array[matchs[i].modifier] += inc;
          return matchs[i].modifier;
       }

   return ECORE_NONE;
}

/* deprecated functions */
EINA_DEPRECATED EAPI int
ecore_event_init(void)
{
   return ecore_input_init();
}

EINA_DEPRECATED EAPI int
ecore_event_shutdown(void)
{
   return ecore_input_shutdown();
}

EINA_DEPRECATED EAPI unsigned int
ecore_event_modifier_mask(Ecore_Event_Modifier modifier)
{
   return ecore_input_event_modifier_mask(modifier);
}

EINA_DEPRECATED EAPI Ecore_Event_Modifier
ecore_event_update_modifier(const char *key, Ecore_Event_Modifiers *modifiers, int inc)
{
   return ecore_input_event_update_modifier(key, modifiers, inc);
}
