#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#define ECORE_EVAS_INTERNAL

#include <string.h>
#include <stdlib.h>

#include "Ecore.h"
#include "Ecore_Input.h"

#include "Ecore_Input_Evas.h"
#include "ecore_input_evas_private.h"

int _ecore_input_evas_log_dom = -1;

typedef struct _Ecore_Input_Window Ecore_Input_Window;
struct _Ecore_Input_Window
{
   Evas *evas;
   void *window;
   Ecore_Event_Mouse_Move_Cb move_mouse;
   Ecore_Event_Multi_Move_Cb move_multi;
   Ecore_Event_Multi_Down_Cb down_multi;
   Ecore_Event_Multi_Up_Cb up_multi;
   Ecore_Event_Direct_Input_Cb direct;
   int ignore_event;
};

typedef enum _Ecore_Input_State {
  ECORE_INPUT_NONE = 0,
  ECORE_INPUT_DOWN,
  ECORE_INPUT_MOVE,
  ECORE_INPUT_UP,
  ECORE_INPUT_CANCEL
} Ecore_Input_State;

typedef enum _Ecore_Input_Action {
  ECORE_INPUT_CONTINUE = 0,
  ECORE_INPUT_IGNORE,
  ECORE_INPUT_FAKE_UP
} Ecore_Input_Action;

typedef struct _Ecore_Input_Last Ecore_Event_Last;
struct _Ecore_Input_Last
{
   Ecore_Event_Mouse_Button *ev;
   Ecore_Timer *timer;
   Evas_Device *evas_device;

   unsigned int device;
   unsigned int buttons;
   Ecore_Input_State state;
   Ecore_Window win;

   Eina_Bool faked : 1;
};

static int _ecore_event_evas_init_count = 0;
static Ecore_Event_Handler *ecore_event_evas_handlers[10];
static Eina_Hash *_window_hash = NULL;

static Eina_List *_last_events = NULL;
static double _last_events_timeout = 0.5;
static Eina_Bool _last_events_enable = EINA_FALSE;

static Eina_Bool _ecore_event_evas_mouse_button(Ecore_Event_Mouse_Button *e,
                                                Ecore_Event_Press press,
                                                Eina_Bool faked);

static Ecore_Input_Action
_ecore_event_last_check(Ecore_Event_Last *eel, Ecore_Event_Press press)
{
   switch (eel->state)
     {
      case ECORE_INPUT_NONE:
         /* 1. ECORE_INPUT_NONE => ECORE_UP : impossible
          * 2. ECORE_INPUT_NONE => ECORE_CANCEL : impossible
          * 3. ECORE_INPUT_NONE => ECORE_DOWN : ok
          */
         return ECORE_INPUT_CONTINUE;

      case ECORE_INPUT_DOWN:
         /* 1. ECORE_INPUT_DOWN => ECORE_UP : ok
          * 2. ECORE_INPUT_DOWN => ECORE_CANCEL : ok
          */
         if ((press == ECORE_UP) || (press == ECORE_CANCEL))
           return ECORE_INPUT_CONTINUE;

         /* 3. ECORE_INPUT_DOWN => ECORE_DOWN : emit a faked UP then */
         INF("Down event occurs twice. device(%d), button(%d)", eel->device, eel->buttons);
         return ECORE_INPUT_FAKE_UP;

      case ECORE_INPUT_MOVE:
         /* 1. ECORE_INPUT_MOVE => ECORE_UP : ok
          * 2. ECORE_INPUT_MOVE => ECORE_CANCEL : ok
          */
         if ((press == ECORE_UP) || (press == ECORE_CANCEL))
           return ECORE_INPUT_CONTINUE;

         /* 3. ECORE_INPUT_MOVE => ECORE_DOWN : ok
          * FIXME: handle fake button up and push for more delay here */
         //TODO: How to deal with down event after move event?
         INF("Down event occurs after move event. device(%d), button(%d)", eel->device, eel->buttons);
         return ECORE_INPUT_FAKE_UP;

      case ECORE_INPUT_UP:
      case ECORE_INPUT_CANCEL:
          /* 1. ECORE_INPUT_UP     => ECORE_DOWN : ok */
          /* 2. ECORE_INPUT_CANCEL => ECORE_DOWN : ok */
         if (press == ECORE_DOWN)
           return ECORE_INPUT_CONTINUE;

          /* 3. ECORE_INPUT_UP     => ECORE_UP :  ignore */
          /* 4. ECORE_INPUT_UP     => ECORE_CANCEL : ignore */
          /* 5. ECORE_INPUT_CANCEL => ECORE_UP : ignore */
          /* 6. ECORE_INPUT_CANCEL => ECORE_CANCEL : ignore */
         INF("Up/cancel event occurs after up/cancel event. device(%d), button(%d)", eel->device, eel->buttons);
         return ECORE_INPUT_IGNORE;
     }
  return ECORE_INPUT_IGNORE;
}

static Ecore_Event_Last *
_ecore_event_evas_lookup(Evas_Device *evas_device, unsigned int device,
                         unsigned int buttons, Ecore_Window win,
                         Eina_Bool create_new)
{
   Ecore_Event_Last *eel;
   Eina_List *l;

   //the number of last event is small, simple check is ok.
   EINA_LIST_FOREACH(_last_events, l, eel)
     if ((eel->device == device) && (eel->buttons == buttons) && (eel->evas_device == evas_device))
       return eel;
   if (!create_new) return NULL;
   eel = malloc(sizeof (Ecore_Event_Last));
   if (!eel) return NULL;

   eel->timer = NULL;
   eel->ev = NULL;
   eel->device = device;
   eel->buttons = buttons;
   eel->state = ECORE_INPUT_NONE;
   eel->faked = EINA_FALSE;
   eel->win = win;
   eel->evas_device = evas_device;

   _last_events = eina_list_append(_last_events, eel);
   return eel;
}

static Eina_Bool
_ecore_event_evas_push_fake(void *data)
{
   Ecore_Event_Last *eel = data;

   switch (eel->state)
     {
      case ECORE_INPUT_NONE:
      case ECORE_INPUT_UP:
      case ECORE_INPUT_CANCEL:
         /* should not happen */
         break;
      case ECORE_INPUT_DOWN:
         /* use the saved Ecore_Event */
         /* No up event since timeout started ... */
      case ECORE_INPUT_MOVE:
         /* No up event since timeout started ... */
         _ecore_event_evas_mouse_button(eel->ev, ECORE_UP, EINA_TRUE);
         eel->faked = EINA_TRUE;
         break;
     }

   free(eel->ev);
   eel->ev = NULL;
   eel->timer = NULL;
   return EINA_FALSE;
}

static Eina_Bool
_ecore_event_evas_push_mouse_button(Ecore_Event_Mouse_Button *e, Ecore_Event_Press press)
{
   Ecore_Event_Last *eel;
   Ecore_Input_Action action = ECORE_INPUT_CONTINUE;

   //_ecore_event_evas_mouse_button already check press or cancel without history
   eel = _ecore_event_evas_lookup(e->dev, e->multi.device, e->buttons, e->window, EINA_TRUE);
   if (!eel) return EINA_FALSE;
   INF("dev(%d), button(%d), last_press(%d), press(%d)", e->multi.device, e->buttons, eel->state, press);

   if (e->window == eel->win)
     action = _ecore_event_last_check(eel, press);
   INF("action(%d)", action);
   switch (action)
     {
      case ECORE_INPUT_FAKE_UP:
         _ecore_event_evas_mouse_button(e, ECORE_UP, EINA_TRUE);
      case ECORE_INPUT_CONTINUE:
         break;
      case ECORE_INPUT_IGNORE:
      default:
        eel->win = e->window;
        return EINA_FALSE;
     }

   switch (press)
     {
      case ECORE_DOWN:
        eel->state = ECORE_INPUT_DOWN;
        break;
      case ECORE_UP:
        eel->state = ECORE_INPUT_UP;
        break;
      default:
        break;
     }
   eel->win = e->window;

   //if up event not occurs from under layers of ecore
   //up event is generated by ecore
   if (_last_events_enable &&
       !EINA_DBL_EQ(_last_events_timeout, 0))
     {
        if (eel->timer) ecore_timer_del(eel->timer);
        eel->timer = NULL;
        if (press == ECORE_DOWN)
          {
             /* Save the Ecore_Event somehow */
             if (!eel->ev) eel->ev = malloc(sizeof (Ecore_Event_Mouse_Button));
             if (!eel->ev) return EINA_FALSE;
             memcpy(eel->ev, e, sizeof (Ecore_Event_Mouse_Button));
             eel->timer = ecore_timer_add(_last_events_timeout, _ecore_event_evas_push_fake, eel);
          }
        else
          {
             free(eel->ev);
             eel->ev = NULL;
          }
     }
   return EINA_TRUE;
}

static void
_ecore_event_evas_push_mouse_move(Ecore_Event_Mouse_Move *e)
{
   Ecore_Event_Last *eel;
   Eina_List *l;

   if (!_last_events_enable) return;

   EINA_LIST_FOREACH(_last_events, l, eel)
     switch (eel->state)
       {
        case ECORE_INPUT_NONE:
        case ECORE_INPUT_UP:
        case ECORE_INPUT_CANCEL:
           /* (none, up, or cancel) => move, sounds fine to me */
           break;
        case ECORE_INPUT_DOWN:
        case ECORE_INPUT_MOVE:
           /* Down and moving, let's see */
           if (eel->ev)
             {
                /* Add some delay to the timer */
                ecore_timer_reset(eel->timer);
                /* Update position */
                eel->ev->x = e->x;
                eel->ev->y = e->y;
                eel->ev->root.x = e->root.x;
                eel->ev->root.y = e->root.y;
                eel->state = ECORE_INPUT_MOVE;
                break;
             }
           /* FIXME: Timer did expire, do something maybe */
           break;
       }
}

EAPI void
ecore_event_evas_seat_modifier_lock_update(Evas *e, unsigned int modifiers,
                                           Evas_Device *seat)
{
   if (modifiers & ECORE_EVENT_MODIFIER_SHIFT)
     evas_seat_key_modifier_on(e, "Shift", seat);
   else evas_seat_key_modifier_off(e, "Shift", seat);

   if (modifiers & ECORE_EVENT_MODIFIER_CTRL)
     evas_seat_key_modifier_on(e, "Control", seat);
   else evas_seat_key_modifier_off(e, "Control", seat);

   if (modifiers & ECORE_EVENT_MODIFIER_ALT)
     evas_seat_key_modifier_on(e, "Alt", seat);
   else evas_seat_key_modifier_off(e, "Alt", seat);

   if (modifiers & ECORE_EVENT_MODIFIER_WIN)
     {
        evas_seat_key_modifier_on(e, "Super", seat);
        evas_seat_key_modifier_on(e, "Hyper", seat);
     }
   else
     {
        evas_seat_key_modifier_off(e, "Super", seat);
        evas_seat_key_modifier_off(e, "Hyper", seat);
     }

   if (modifiers & ECORE_EVENT_MODIFIER_ALTGR)
     evas_seat_key_modifier_on(e, "AltGr", seat);
   else evas_seat_key_modifier_off(e, "AltGr", seat);

   if (modifiers & ECORE_EVENT_LOCK_SCROLL)
     evas_seat_key_lock_on(e, "Scroll_Lock", seat);
   else evas_seat_key_lock_off(e, "Scroll_Lock", seat);

   if (modifiers & ECORE_EVENT_LOCK_NUM)
     evas_seat_key_lock_on(e, "Num_Lock", seat);
   else evas_seat_key_lock_off(e, "Num_Lock", seat);

   if (modifiers & ECORE_EVENT_LOCK_CAPS)
     evas_seat_key_lock_on(e, "Caps_Lock", seat);
   else evas_seat_key_lock_off(e, "Caps_Lock", seat);

   if (modifiers & ECORE_EVENT_LOCK_SHIFT)
     evas_seat_key_lock_on(e, "Shift_Lock", seat);
   else evas_seat_key_lock_off(e, "Shift_Lock", seat);
}

EAPI void
ecore_event_evas_modifier_lock_update(Evas *e, unsigned int modifiers)
{
   ecore_event_evas_seat_modifier_lock_update(e, modifiers, NULL);
}

EAPI void
ecore_event_window_register(Ecore_Window id, void *window, Evas *evas,
                            Ecore_Event_Mouse_Move_Cb move_mouse,
                            Ecore_Event_Multi_Move_Cb move_multi,
                            Ecore_Event_Multi_Down_Cb down_multi,
                            Ecore_Event_Multi_Up_Cb up_multi)
{
   Ecore_Input_Window *w;

   w = calloc(1, sizeof(Ecore_Input_Window));
   if (!w) return;

   w->evas = evas;
   w->window = window;
   w->move_mouse = move_mouse;
   w->move_multi = move_multi;
   w->down_multi = down_multi;
   w->up_multi = up_multi;
   w->ignore_event = 0;

   eina_hash_add(_window_hash, &id, w);

   evas_key_modifier_add(evas, "Shift");
   evas_key_modifier_add(evas, "Control");
   evas_key_modifier_add(evas, "Alt");
   evas_key_modifier_add(evas, "Meta");
   evas_key_modifier_add(evas, "Hyper");
   evas_key_modifier_add(evas, "Super");
   evas_key_modifier_add(evas, "AltGr");
   evas_key_lock_add(evas, "Caps_Lock");
   evas_key_lock_add(evas, "Num_Lock");
   evas_key_lock_add(evas, "Scroll_Lock");
}

EAPI void
ecore_event_window_unregister(Ecore_Window id)
{
   eina_hash_del(_window_hash, &id, NULL);
}

EAPI void
_ecore_event_window_direct_cb_set(Ecore_Window id, Ecore_Event_Direct_Input_Cb fptr)
{
   Ecore_Input_Window *lookup;

   lookup = eina_hash_find(_window_hash, &id);
   if (!lookup) return;
   lookup->direct = fptr;
}

EAPI void *
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
   if (!lookup) return;
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

static Eina_Bool
_ecore_event_evas_key(Ecore_Event_Key *e, Ecore_Event_Press press)
{
   Ecore_Input_Window *lookup;
   Eo *seat;

   lookup = _ecore_event_window_match(e->event_window);
   if (!lookup) return ECORE_CALLBACK_PASS_ON;
   seat = e->dev ? efl_input_device_seat_get(e->dev) : NULL;
   ecore_event_evas_seat_modifier_lock_update(lookup->evas, e->modifiers, seat);
   if (press == ECORE_DOWN)
     {
        if (!lookup->direct ||
            !lookup->direct(lookup->window, ECORE_EVENT_KEY_DOWN, e))
          {
             evas_event_feed_key_down_with_keycode(lookup->evas,
                                                   e->keyname,
                                                   e->key,
                                                   e->string,
                                                   e->compose,
                                                   e->timestamp,
                                                   e->data,
                                                   e->keycode);
          }
     }
   else
     {
        if (!lookup->direct ||
            !lookup->direct(lookup->window, ECORE_EVENT_KEY_UP, e))
          {
             evas_event_feed_key_up_with_keycode(lookup->evas,
                                                 e->keyname,
                                                 e->key,
                                                 e->string,
                                                 e->compose,
                                                 e->timestamp,
                                                 e->data,
                                                 e->keycode);
          }
     }

   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_ecore_event_evas_mouse_button_cancel(Ecore_Event_Mouse_Button *e)
{
   Ecore_Input_Window *lookup;
   Ecore_Event_Last *eel;
   Eina_List *l;

   lookup = _ecore_event_window_match(e->event_window);
   if (!lookup) return ECORE_CALLBACK_PASS_ON;

   INF("ButtonEvent cancel, device(%d), button(%d)", e->multi.device, e->buttons);
   if (!lookup->direct ||
       !lookup->direct(lookup->window, ECORE_EVENT_MOUSE_BUTTON_CANCEL, e))
     {
        evas_event_feed_mouse_cancel(lookup->evas, e->timestamp, NULL);
     }

   //the number of last event is small, simple check is ok.
   EINA_LIST_FOREACH(_last_events, l, eel)
     {
        Ecore_Input_Action act = _ecore_event_last_check(eel, ECORE_CANCEL);
        INF("ButtonEvent cancel, dev(%d), button(%d), last_press(%d), action(%d)", eel->device, eel->buttons, eel->state, act);
        if (act == ECORE_INPUT_CONTINUE) eel->state = ECORE_INPUT_CANCEL;
     }

   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_ecore_event_evas_mouse_button(Ecore_Event_Mouse_Button *e, Ecore_Event_Press press, Eina_Bool faked)
{
   Ecore_Event_Last *eel;
   Ecore_Input_Window *lookup;
   Evas_Button_Flags flags = EVAS_BUTTON_NONE;

   lookup = _ecore_event_window_match(e->event_window);
   if (!lookup) return ECORE_CALLBACK_PASS_ON;
   if (e->double_click) flags |= EVAS_BUTTON_DOUBLE_CLICK;
   if (e->triple_click) flags |= EVAS_BUTTON_TRIPLE_CLICK;
   INF("\tButtonEvent:ecore_event_evas press(%d), device(%d), button(%d), fake(%d)", press, e->multi.device, e->buttons, faked);

   //handle all mouse error from under layers of ecore
   //error handle
   // 1. ecore up without ecore down
   // 2. ecore cancel without ecore down
   if (press != ECORE_DOWN)
     {
        //ECORE_UP or ECORE_CANCEL
        eel = _ecore_event_evas_lookup(e->dev, e->multi.device, e->buttons, e->window, EINA_FALSE);
        if (!eel)
          {
             WRN("ButtonEvent has no history.");
             return ECORE_CALLBACK_PASS_ON;
          }

        if ((e->window == eel->win) &&
            ((eel->state == ECORE_INPUT_UP) ||
             (eel->state == ECORE_INPUT_CANCEL)))
          {
             WRN("ButtonEvent has wrong history. Last state=%d", eel->state);
             return ECORE_CALLBACK_PASS_ON;
          }
     }

   if (!faked)
     {
        Eina_Bool ret = EINA_FALSE;
        ret = _ecore_event_evas_push_mouse_button(e, press);
        /* This ButtonEvent is worng */
        if (!ret) return ECORE_CALLBACK_PASS_ON;
     }

   if (e->multi.device == 0)
     {
        Eo *seat = e->dev ? efl_input_device_seat_get(e->dev) : NULL;
        ecore_event_evas_seat_modifier_lock_update(lookup->evas, e->modifiers, seat);
        if (press == ECORE_DOWN)
          {
             if (!lookup->direct ||
                 !lookup->direct(lookup->window, ECORE_EVENT_MOUSE_BUTTON_DOWN, e))
               {
                  evas_event_feed_mouse_down(lookup->evas, e->buttons, flags,
                                             e->timestamp, NULL);
               }
          }
        else
          {
             if (!lookup->direct ||
                 !lookup->direct(lookup->window, ECORE_EVENT_MOUSE_BUTTON_UP, e))
               {
                  evas_event_feed_mouse_up(lookup->evas, e->buttons, flags,
                                           e->timestamp, NULL);
               }
          }
     }
   else
     {
        if (press == ECORE_DOWN)
          {
             if (!lookup->direct ||
                 !lookup->direct(lookup->window, ECORE_EVENT_MOUSE_BUTTON_DOWN, e))
               {
                  if (lookup->down_multi)
                    lookup->down_multi(lookup->window, e->multi.device,
                                       e->x, e->y, e->multi.radius,
                                       e->multi.radius_x, e->multi.radius_y,
                                       e->multi.pressure, e->multi.angle,
                                       e->multi.x, e->multi.y, flags,
                                       e->timestamp);
                  else
                     evas_event_input_multi_down(lookup->evas, e->multi.device,
                                                 e->x, e->y, e->multi.radius,
                                                 e->multi.radius_x, e->multi.radius_y,
                                                 e->multi.pressure, e->multi.angle,
                                                 e->multi.x, e->multi.y, flags,
                                                 e->timestamp, NULL);
               }
          }
        else
          {
             if (!lookup->direct ||
                 !lookup->direct(lookup->window, ECORE_EVENT_MOUSE_BUTTON_UP, e))
               {
                  if (lookup->up_multi)
                    lookup->up_multi(lookup->window, e->multi.device,
                                     e->x, e->y, e->multi.radius,
                                     e->multi.radius_x, e->multi.radius_y,
                                     e->multi.pressure, e->multi.angle,
                                     e->multi.x, e->multi.y, flags,
                                     e->timestamp);
                  else
                     evas_event_input_multi_up(lookup->evas, e->multi.device,
                                               e->x, e->y, e->multi.radius,
                                               e->multi.radius_x, e->multi.radius_y,
                                               e->multi.pressure, e->multi.angle,
                                               e->multi.x, e->multi.y, flags,
                                               e->timestamp, NULL);
               }
          }
     }
   return ECORE_CALLBACK_PASS_ON;
}

EAPI Eina_Bool
ecore_event_evas_mouse_move(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Event_Mouse_Move *e;
   Ecore_Input_Window *lookup;

   e = event;
   lookup = _ecore_event_window_match(e->event_window);
   if (!lookup) return ECORE_CALLBACK_PASS_ON;
   if (e->multi.device == 0)
     {
        Eo *seat = e->dev ? efl_input_device_seat_get(e->dev) : NULL;
        _ecore_event_evas_push_mouse_move(e);
        ecore_event_evas_seat_modifier_lock_update(lookup->evas, e->modifiers, seat);
        if (!lookup->direct ||
            !lookup->direct(lookup->window, ECORE_EVENT_MOUSE_MOVE, e))
          {
             if (lookup->move_mouse)
               lookup->move_mouse(lookup->window, e->x, e->y, e->timestamp);
             else
                evas_event_input_mouse_move(lookup->evas, e->x, e->y, e->timestamp,
                                            NULL);
          }
     }
   else
     {
        if (!lookup->direct ||
            !lookup->direct(lookup->window, ECORE_EVENT_MOUSE_MOVE, e))
          {
             if (lookup->move_multi)
               lookup->move_multi(lookup->window, e->multi.device,
                                  e->x, e->y, e->multi.radius,
                                  e->multi.radius_x, e->multi.radius_y,
                                  e->multi.pressure, e->multi.angle,
                                  e->multi.x, e->multi.y, e->timestamp);
             else
               evas_event_input_multi_move(lookup->evas, e->multi.device,
                                           e->x, e->y, e->multi.radius,
                                           e->multi.radius_x, e->multi.radius_y,
                                           e->multi.pressure, e->multi.angle,
                                           e->multi.x, e->multi.y, e->timestamp,
                                           NULL);
          }
     }
   return ECORE_CALLBACK_PASS_ON;
}

EAPI Eina_Bool
ecore_event_evas_mouse_button_down(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   return _ecore_event_evas_mouse_button((Ecore_Event_Mouse_Button *)event, ECORE_DOWN, EINA_FALSE);
}

EAPI Eina_Bool
ecore_event_evas_mouse_button_up(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   return _ecore_event_evas_mouse_button((Ecore_Event_Mouse_Button *)event, ECORE_UP, EINA_FALSE);
}

EAPI Eina_Bool
ecore_event_evas_mouse_button_cancel(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   return _ecore_event_evas_mouse_button_cancel((Ecore_Event_Mouse_Button *)event);
}

static Eina_Bool
_ecore_event_evas_mouse_io(Ecore_Event_Mouse_IO *e, Ecore_Event_IO io)
{
   Ecore_Input_Window *lookup;
   Eo *seat;

   lookup = _ecore_event_window_match(e->event_window);
   if (!lookup) return ECORE_CALLBACK_PASS_ON;
   seat = e->dev ? efl_input_device_seat_get(e->dev) : NULL;
   ecore_event_evas_seat_modifier_lock_update(lookup->evas, e->modifiers, seat);

   switch (io)
     {
      case ECORE_IN:
        if (!lookup->direct ||
            !lookup->direct(lookup->window, ECORE_EVENT_MOUSE_IN, e))
          {
             evas_event_feed_mouse_in(lookup->evas, e->timestamp, NULL);
          }
         break;
      case ECORE_OUT:
        if (!lookup->direct ||
            !lookup->direct(lookup->window, ECORE_EVENT_MOUSE_OUT, e))
          {
             evas_event_feed_mouse_out(lookup->evas, e->timestamp, NULL);
          }
         break;
      default:
         break;
     }

   lookup->move_mouse(lookup->window, e->x, e->y, e->timestamp);
   return ECORE_CALLBACK_PASS_ON;
}

EAPI Eina_Bool
ecore_event_evas_key_down(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   return _ecore_event_evas_key((Ecore_Event_Key *)event, ECORE_DOWN);
}

EAPI Eina_Bool
ecore_event_evas_key_up(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   return _ecore_event_evas_key((Ecore_Event_Key *)event, ECORE_UP);
}

EAPI Eina_Bool
ecore_event_evas_mouse_wheel(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Event_Mouse_Wheel *e;
   Ecore_Input_Window *lookup;
   Eo *seat;

   e = event;
   lookup = _ecore_event_window_match(e->event_window);
   if (!lookup) return ECORE_CALLBACK_PASS_ON;
   seat = e->dev ? efl_input_device_seat_get(e->dev) : NULL;
   ecore_event_evas_seat_modifier_lock_update(lookup->evas, e->modifiers, seat);
   if (!lookup->direct ||
       !lookup->direct(lookup->window, ECORE_EVENT_MOUSE_WHEEL, e))
     {
        evas_event_feed_mouse_wheel(lookup->evas, e->direction, e->z, e->timestamp, NULL);
     }

   return ECORE_CALLBACK_PASS_ON;
}

EAPI Eina_Bool
ecore_event_evas_mouse_in(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   return _ecore_event_evas_mouse_io((Ecore_Event_Mouse_IO *)event, ECORE_IN);
}

EAPI Eina_Bool
ecore_event_evas_mouse_out(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   return _ecore_event_evas_mouse_io((Ecore_Event_Mouse_IO *)event, ECORE_OUT);
}

EAPI Eina_Bool
ecore_event_evas_axis_update(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Event_Axis_Update *e;
   Ecore_Input_Window *lookup;

   e = event;
   lookup = _ecore_event_window_match(e->event_window);
   if (!lookup) return ECORE_CALLBACK_PASS_ON;
   if (!lookup->direct ||
       !lookup->direct(lookup->window, ECORE_EVENT_AXIS_UPDATE, e))
     {
        evas_event_feed_axis_update(lookup->evas, e->timestamp, e->device,
                                    e->toolid, e->naxis,
                                    (Evas_Axis *)e->axis, NULL);
     }

   return ECORE_CALLBACK_PASS_ON;
}

EAPI int
ecore_event_evas_init(void)
{
   if (++_ecore_event_evas_init_count !=  1)
     return _ecore_event_evas_init_count;

   _ecore_input_evas_log_dom = eina_log_domain_register
     ("ecore_input_evas",  ECORE_INPUT_EVAS_DEFAULT_LOG_COLOR);
   if (_ecore_input_evas_log_dom < 0)
     {
        EINA_LOG_ERR("Impossible to create a log domain for the ecore input evas_module.");
        return --_ecore_event_evas_init_count;
     }

   if (!ecore_init())
     {
        return --_ecore_event_evas_init_count;
     }

   if (!ecore_event_init())
     {
        goto shutdown_ecore;
     }

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
   ecore_event_evas_handlers[8] = ecore_event_handler_add(ECORE_EVENT_AXIS_UPDATE,
                                                          ecore_event_evas_axis_update,
                                                          NULL);
   ecore_event_evas_handlers[9] = ecore_event_handler_add(ECORE_EVENT_MOUSE_BUTTON_CANCEL,
                                                          ecore_event_evas_mouse_button_cancel,
                                                          NULL);

   _window_hash = eina_hash_pointer_new(free);

   if (getenv("ECORE_INPUT_FIX"))
     {
        const char *tmp;

        _last_events_enable = EINA_TRUE;

        tmp = getenv("ECORE_INPUT_TIMEOUT_FIX");
        if (tmp)
          _last_events_timeout = ((double) atoi(tmp)) / 60;
     }

   return _ecore_event_evas_init_count;

   shutdown_ecore:
   ecore_shutdown();

   return --_ecore_event_evas_init_count;
}

EAPI int
ecore_event_evas_shutdown(void)
{
   size_t i;

   if (--_ecore_event_evas_init_count != 0)
     return _ecore_event_evas_init_count;

   eina_hash_free(_window_hash);
   _window_hash = NULL;
   for (i = 0; i < sizeof(ecore_event_evas_handlers) / sizeof(Ecore_Event_Handler *); i++)
     {
        ecore_event_handler_del(ecore_event_evas_handlers[i]);
        ecore_event_evas_handlers[i] = NULL;
     }

   ecore_event_shutdown();
   ecore_shutdown();

   eina_log_domain_unregister(_ecore_input_evas_log_dom);
   _ecore_input_evas_log_dom = -1;

   return _ecore_event_evas_init_count;
}
