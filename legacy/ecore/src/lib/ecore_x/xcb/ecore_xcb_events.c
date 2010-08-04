#ifdef HAVE_CONFIG_H
# include <config.h>
#endif /* ifdef HAVE_CONFIG_H */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <Ecore.h>
#include <Ecore_Input.h>

#include "ecore_xcb_private.h"
#include "Ecore_X_Atoms.h"

/** OpenBSD does not define CODESET
 * FIXME ??
 */

#ifndef CODESET
#define CODESET "INVALID"
#endif /* ifndef CODESET */

#if 0
static void      _ecore_x_event_free_window_prop_name_class_change(void *data, void *ev);
static void      _ecore_x_event_free_window_prop_title_change(void *data, void *ev);
static void      _ecore_x_event_free_window_prop_visible_title_change(void *data, void *ev);
static void      _ecore_x_event_free_window_prop_icon_name_change(void *data, void *ev);
static void      _ecore_x_event_free_window_prop_visible_icon_name_change(void *data, void *ev);
static void      _ecore_x_event_free_window_prop_client_machine_change(void *data, void *ev);
#endif /* if 0 */

static Ecore_X_Window _ecore_xcb_mouse_down_last_window = 0;
static Ecore_X_Window _ecore_xcb_mouse_down_last_last_window = 0;
static Ecore_X_Window _ecore_xcb_mouse_down_last_event_window = 0;
static Ecore_X_Window _ecore_xcb_mouse_down_last_last_event_window = 0;
static Ecore_X_Time _ecore_xcb_mouse_down_last_time = 0;
static Ecore_X_Time _ecore_xcb_mouse_down_last_last_time = 0;
static int _ecore_xcb_mouse_up_count = 0;
static int _ecore_xcb_mouse_down_did_triple = 0;
static int _ecore_xcb_last_event_mouse_move = 0;
static Ecore_Event *_ecore_xcb_last_event_mouse_move_event = NULL;

static void
_ecore_x_event_free_mouse_move(void *data __UNUSED__, void *ev)
{
   Ecore_Event_Mouse_Move *e;

   e = ev;
   if (_ecore_xcb_last_event_mouse_move)
     {
        _ecore_xcb_last_event_mouse_move_event = NULL;
        _ecore_xcb_last_event_mouse_move = 0;
     }

   free(e);
} /* _ecore_x_event_free_mouse_move */

/* FIXME: roundtrip */
EAPI void
ecore_x_event_mask_set(Ecore_X_Window     window,
                       Ecore_X_Event_Mask mask)
{
   xcb_get_window_attributes_cookie_t cookie;
   xcb_get_window_attributes_reply_t *reply;
   uint32_t value_list;

   if (!window)
      window = ((xcb_screen_t *)_ecore_xcb_screen)->root;

   cookie = xcb_get_window_attributes_unchecked(_ecore_xcb_conn, window);
   reply = xcb_get_window_attributes_reply(_ecore_xcb_conn, cookie, NULL);
   if (!reply)
      return;

   value_list = mask | reply->your_event_mask;
   xcb_change_window_attributes(_ecore_xcb_conn, window, XCB_CW_EVENT_MASK, &value_list);
   free(reply);
} /* ecore_x_event_mask_set */

/* FIXME: roundtrip */
EAPI void
ecore_x_event_mask_unset(Ecore_X_Window     window,
                         Ecore_X_Event_Mask mask)
{
   xcb_get_window_attributes_cookie_t cookie;
   xcb_get_window_attributes_reply_t *reply;
   uint32_t value_list;

   if (!window)
      window = ((xcb_screen_t *)_ecore_xcb_screen)->root;

   cookie = xcb_get_window_attributes_unchecked(_ecore_xcb_conn, window);
   reply = xcb_get_window_attributes_reply(_ecore_xcb_conn, cookie, NULL);
   if (!reply)
      return;

   value_list = reply->your_event_mask & ~mask;
   xcb_change_window_attributes(_ecore_xcb_conn, window, XCB_CW_EVENT_MASK, &value_list);
   free(reply);
} /* ecore_x_event_mask_unset */

#if 0
static void
_ecore_x_event_free_window_prop_name_class_change(void *data, void *ev)
{
   Ecore_X_Event_Window_Prop_Name_Class_Change *e;

   e = ev;
   if (e->name)
      free(e->name);

   if (e->clas)
      free(e->clas);

   free(e);
} /* _ecore_x_event_free_window_prop_name_class_change */

static void
_ecore_x_event_free_window_prop_title_change(void *data, void *ev)
{
   Ecore_X_Event_Window_Prop_Title_Change *e;

   e = ev;
   if (e->title)
      free(e->title);

   free(e);
} /* _ecore_x_event_free_window_prop_title_change */

static void
_ecore_x_event_free_window_prop_visible_title_change(void *data, void *ev)
{
   Ecore_X_Event_Window_Prop_Visible_Title_Change *e;

   e = ev;
   if (e->title)
      free(e->title);

   free(e);
} /* _ecore_x_event_free_window_prop_visible_title_change */

static void
_ecore_x_event_free_window_prop_icon_name_change(void *data, void *ev)
{
   Ecore_X_Event_Window_Prop_Icon_Name_Change *e;

   e = ev;
   if (e->name)
      free(e->name);

   free(e);
} /* _ecore_x_event_free_window_prop_icon_name_change */

static void
_ecore_x_event_free_window_prop_visible_icon_name_change(void *data, void *ev)
{
   Ecore_X_Event_Window_Prop_Visible_Icon_Name_Change *e;

   e = ev;
   if (e->name)
      free(e->name);

   free(e);
} /* _ecore_x_event_free_window_prop_visible_icon_name_change */

static void
_ecore_x_event_free_window_prop_client_machine_change(void *data, void *ev)
{
   Ecore_X_Event_Window_Prop_Client_Machine_Change *e;

   e = ev;
   if (e->name)
      free(e->name);

   free(e);
} /* _ecore_x_event_free_window_prop_client_machine_change */

#endif /* if 0 */

static void
_ecore_x_event_free_xdnd_enter(void *data __UNUSED__, void *ev)
{
   Ecore_X_Event_Xdnd_Enter *e;
   int i;

   e = ev;
   for (i = 0; i < e->num_types; i++)
      free(e->types[i]);
   free(e->types);
   free(e);
} /* _ecore_x_event_free_xdnd_enter */

static void
_ecore_x_event_free_selection_notify(void *data __UNUSED__, void *ev)
{
   Ecore_X_Event_Selection_Notify *e;
   Ecore_X_Selection_Data *sel;

   e = ev;
   sel = e->data;
   if (sel->free)
      sel->free(sel);

   free(e->target);
   free(e);
} /* _ecore_x_event_free_selection_notify */

static unsigned int
_ecore_x_event_modifiers(unsigned int state)
{
   unsigned int modifiers = 0;

   if (state & ECORE_X_MODIFIER_SHIFT)
      modifiers |= ECORE_EVENT_MODIFIER_SHIFT;

   if (state & ECORE_X_MODIFIER_CTRL)
      modifiers |= ECORE_EVENT_MODIFIER_CTRL;

   if (state & ECORE_X_MODIFIER_ALT)
      modifiers |= ECORE_EVENT_MODIFIER_ALT;

   if (state & ECORE_X_MODIFIER_WIN)
      modifiers |= ECORE_EVENT_MODIFIER_WIN;

   if (state & ECORE_X_LOCK_SCROLL)
      modifiers |= ECORE_EVENT_LOCK_SCROLL;

   if (state & ECORE_X_LOCK_NUM)
      modifiers |= ECORE_EVENT_LOCK_NUM;

   if (state & ECORE_X_LOCK_CAPS)
      modifiers |= ECORE_EVENT_LOCK_CAPS;

   return modifiers;
} /* _ecore_x_event_modifiers */

static void
_ecore_mouse_move(unsigned int timestamp, unsigned int xmodifiers,
                  int x, int y,
                  int x_root, int y_root,
                  unsigned int event_window,
                  unsigned int window,
                  unsigned int root_win,
                  int same_screen)
{
   Ecore_Event_Mouse_Move *e;
   Ecore_Event *event;

   e = malloc(sizeof(Ecore_Event_Mouse_Move));
   if (!e)
      return;

   e->window = window;
   e->root_window = root_win;
   e->timestamp = timestamp;
   e->same_screen = same_screen;
   e->event_window = event_window;

   e->modifiers = _ecore_x_event_modifiers(xmodifiers);
   e->x = x;
   e->y = y;
   e->root.x = x_root;
   e->root.y = y_root;

   event = ecore_event_add(ECORE_EVENT_MOUSE_MOVE, e, _ecore_x_event_free_mouse_move, NULL);

   _ecore_xcb_event_last_time = timestamp;
   _ecore_xcb_event_last_window = window;
   _ecore_xcb_event_last_root_x = x_root;
   _ecore_xcb_event_last_root_y = y_root;

   _ecore_xcb_last_event_mouse_move_event = event;
} /* _ecore_mouse_move */

static void
_ecore_key_press(int                  event,
                 xcb_generic_event_t *ev)
{
   /*
      Ecore_Event_Key *e;
      const char *compose = NULL;
      char *tmp = NULL;
      char *keyname;
      char *key;
      char keyname_buffer[256];
      char compose_buffer[256];
      KeySym sym;
      XComposeStatus status;
      int val;

      _ecore_xcb_last_event_mouse_move = 0;
      keyname = XKeysymToString(XKeycodeToKeysym(xevent->display,
                                               xevent->keycode, 0));
      if (!keyname)
      {
         snprintf(keyname_buffer, sizeof(keyname_buffer), "Keycode-%i", xevent->keycode);
         keyname = keyname_buffer;
         if (!keyname) return ;
      }

      sym = 0;
      key = NULL;
      compose = NULL;
      if (_ecore_x_ic)
      {
         Status mbstatus;
      #ifdef X_HAVE_UTF8_STRING
         val = Xutf8LookupString(_ecore_x_ic, (XKeyEvent *)xevent, compose_buffer, sizeof(compose_buffer) - 1, &sym, &mbstatus);
      #else
         val = XmbLookupString(_ecore_x_ic, (XKeyEvent *)xevent, compose_buffer, sizeof(compose_buffer) - 1, &sym, &mbstatus);
      #endif
         if (mbstatus == XBufferOverflow)
           {
              tmp = malloc(sizeof (char) * (val + 1));
              if (!tmp) return ;

              compose = tmp;

      #ifdef X_HAVE_UTF8_STRING
              val = Xutf8LookupString(_ecore_x_ic, (XKeyEvent *)xevent, tmp, val, &sym, &mbstatus);
      #else
              val = XmbLookupString(_ecore_x_ic, (XKeyEvent *)xevent, tmp, val, &sym, &mbstatus);
      #endif
              if (val > 0)
                {
                   tmp[val] = 0;

      #ifndef X_HAVE_UTF8_STRING
                   compose = eina_str_convert(nl_langinfo(CODESET), "UTF-8", tmp);
                   free(tmp);
                   tmp = compose;
      #endif
                }
              else compose = NULL;
           }
         else
           if (val > 0)
             {
                compose_buffer[val] = 0;
      #ifdef X_HAVE_UTF8_STRING
                compose = compose_buffer;
      #else
                compose = eina_str_convert(nl_langinfo(CODESET), "UTF-8", compose_buffer);
                tmp = compose;
      #endif
             }
      }
      else
      {
         val = XLookupString(xevent, compose_buffer, sizeof(compose_buffer), &sym, &status);
         if (val > 0)
           {
              compose_buffer[val] = 0;
              compose = eina_str_convert(nl_langinfo(CODESET), "UTF-8", compose_buffer);
              tmp = compose;
           }
      }

      key = XKeysymToString(sym);
      if (!key) key = keyname;
      if (!key) goto on_error;

      e = malloc(sizeof(Ecore_Event_Key) + strlen(key) + strlen(keyname) + (compose ? strlen(compose) : 0) + 3);
      if (!e) goto on_error;

      e->keyname = (char*) (e + 1);
      e->key = e->keyname + strlen(keyname) + 1;
      e->compose = (compose) ? e->key + strlen(key) + 1 : NULL;
      e->string = e->compose;

      strcpy((char *) e->keyname, keyname);
      strcpy((char *) e->key, key);
      if (compose) strcpy((char *) e->compose, compose);

      e->modifiers = _ecore_x_event_modifiers(xevent->state);

      e->timestamp = xevent->time;
      e->window = xevent->subwindow ? xevent->subwindow : xevent->window;
      e->event_window = xevent->window;
      e->same_screen = xevent->same_screen;
      e->root_window = xevent->root;

      ecore_event_add(event, e, NULL, NULL);

      _ecore_xcb_event_last_time = e->timestamp;

      on_error:
      if (tmp) free(tmp);
    */
} /* _ecore_key_press */

static Ecore_Event_Mouse_Button *
_ecore_mouse_button(int event,
                    unsigned int timestamp, unsigned int xmodifiers,
                    unsigned int buttons,
                    int x, int y,
                    int x_root, int y_root,
                    unsigned int event_window,
                    unsigned int window,
                    unsigned int root_win,
                    int same_screen)
{
   Ecore_Event_Mouse_Button *e;

   e = malloc(sizeof(Ecore_Event_Mouse_Button));
   if (!e)
      return NULL;

   e->window = window;
   e->root_window = root_win;
   e->timestamp = timestamp;
   e->same_screen = same_screen;
   e->event_window = event_window;

   e->buttons = buttons;
   e->modifiers = _ecore_x_event_modifiers(xmodifiers);
   e->double_click = 0;
   e->triple_click = 0;
   e->x = x;
   e->y = y;
   e->root.x = x_root;
   e->root.y = y_root;

   if (event_window == window)
     {
        if (((int)(timestamp - _ecore_xcb_mouse_down_last_time) <=
             (int)(1000 * _ecore_xcb_double_click_time)) &&
            (window == _ecore_xcb_mouse_down_last_window) &&
            (event_window == _ecore_xcb_mouse_down_last_event_window)
            )
           e->double_click = 1;

        if (((int)(timestamp - _ecore_xcb_mouse_down_last_last_time) <=
             (int)(2 * 1000 * _ecore_xcb_double_click_time)) &&
            (window == _ecore_xcb_mouse_down_last_window) &&
            (window == _ecore_xcb_mouse_down_last_last_window) &&
            (event_window == _ecore_xcb_mouse_down_last_event_window) &&
            (event_window == _ecore_xcb_mouse_down_last_last_event_window)
            )
          {
             e->triple_click = 1;
             _ecore_xcb_mouse_down_did_triple = 1;
          }
        else
           _ecore_xcb_mouse_down_did_triple = 0;
     }

   if (event == ECORE_EVENT_MOUSE_BUTTON_DOWN
       && !e->double_click
       && !e->triple_click)
      _ecore_xcb_mouse_up_count = 0;

   _ecore_xcb_event_last_time = e->timestamp;
   _ecore_xcb_event_last_window = e->window;
   _ecore_xcb_event_last_root_x = x_root;
   _ecore_xcb_event_last_root_y = y_root;

   ecore_event_add(event, e, NULL, NULL);

   return e;
} /* _ecore_mouse_button */

void
_ecore_x_event_handle_any_event(xcb_generic_event_t *event)
{
   xcb_generic_event_t *ev = malloc(sizeof(xcb_generic_event_t));
   memcpy(ev, event, sizeof(xcb_generic_event_t));

   ecore_event_add(ECORE_X_EVENT_ANY, ev, NULL, NULL);
} /* _ecore_x_event_handle_any_event */

/* FIXME: handle this event */
void
_ecore_x_event_handle_key_press(xcb_generic_event_t *event)
{
   _ecore_key_press(ECORE_EVENT_KEY_DOWN, event);

   free(event);
} /* _ecore_x_event_handle_key_press */

/* FIXME: handle this event */
void
_ecore_x_event_handle_key_release(xcb_generic_event_t *event)
{
   _ecore_key_press(ECORE_EVENT_KEY_DOWN, event);

   free(event);
} /* _ecore_x_event_handle_key_release */

void
_ecore_x_event_handle_button_press(xcb_generic_event_t *event)
{
   xcb_button_press_event_t *ev;
   int i;

   ev = (xcb_button_press_event_t *)event;
   if ((ev->detail > 3) && (ev->detail < 8))
     {
        Ecore_Event_Mouse_Wheel *e;

        e = malloc(sizeof(Ecore_Event_Mouse_Wheel));
        if (!e)
           return;

        e->timestamp = ev->time;
        e->modifiers = _ecore_x_event_modifiers(ev->state);
        switch (ev->detail)
          {
           case 4: e->direction = 0; e->z = -1; break;

           case 5: e->direction = 0; e->z = 1; break;

           case 6: e->direction = 1; e->z = -1; break;

           case 7: e->direction = 1; e->z = 1; break;

           default: e->direction = 0; e->z = 0; break;
          } /* switch */

        e->x = ev->event_x;
        e->y = ev->event_y;
        e->root.x = ev->root_x;
        e->root.y = ev->root_y;

        if (ev->child)
           e->window = ev->child;
        else
           e->window = ev->event;

        e->event_window = ev->event;
        e->same_screen = ev->same_screen;
        e->root_window = ev->root;
        _ecore_xcb_event_last_time = e->timestamp;
        _ecore_xcb_event_last_window = e->window;
        _ecore_xcb_event_last_root_x = e->root.x;
        _ecore_xcb_event_last_root_y = e->root.y;
        ecore_event_add(ECORE_EVENT_MOUSE_WHEEL, e, NULL, NULL);
        for (i = 0; i < _ecore_window_grabs_num; i++)
          {
             if ((_ecore_window_grabs[i] == ev->event) ||
                 (_ecore_window_grabs[i] == ev->child))
               {
                  Eina_Bool replay = EINA_FALSE;

                  if (_ecore_window_grab_replay_func)
                     replay = _ecore_window_grab_replay_func(_ecore_window_grab_replay_data,
                                                             ECORE_EVENT_MOUSE_WHEEL,
                                                             e);

                  /* FIXME: xcb_key_press_event_t does not save the */
                  /* connection. So I use the current one */
                  if (replay)
                     xcb_allow_events(_ecore_xcb_conn,
                                      XCB_ALLOW_REPLAY_POINTER,
                                      ev->time);
                  else
                     xcb_allow_events(_ecore_xcb_conn,
                                      XCB_ALLOW_ASYNC_POINTER,
                                      ev->time);

                  break;
               }
          }
     }
   else
     {
        {
           _ecore_mouse_move(ev->time, ev->state,
                             ev->event_x, ev->event_y,
                             ev->root_x, ev->root_y,
                             ev->event,
                             (ev->child ? ev->child : ev->event),
                             ev->root,
                             ev->same_screen);
        }
        {
           Ecore_Event_Mouse_Button *e;
           Ecore_X_Window event_window;
           Ecore_X_Window child_window;

           if (_ecore_xcb_mouse_down_did_triple)
             {
                _ecore_xcb_mouse_down_last_window = 0;
                _ecore_xcb_mouse_down_last_last_window = 0;
                _ecore_xcb_mouse_down_last_event_window = 0;
                _ecore_xcb_mouse_down_last_last_event_window = 0;
                _ecore_xcb_mouse_down_last_time = 0;
                _ecore_xcb_mouse_down_last_last_time = 0;
             }

           event_window = ev->child;
           child_window = ev->child ? ev->child : ev->event;

           e = _ecore_mouse_button(ECORE_EVENT_MOUSE_BUTTON_DOWN,
                                   ev->time, ev->state,
                                   ev->detail,
                                   ev->event_x, ev->event_y,
                                   ev->root_x, ev->root_y,
                                   event_window, child_window,
                                   ev->root, ev->same_screen);

           if (!e)
              return;

           for (i = 0; i < _ecore_window_grabs_num; i++)
             {
                if ((_ecore_window_grabs[i] == ev->event) ||
                    (_ecore_window_grabs[i] == ev->child))
                  {
                     Eina_Bool replay = EINA_FALSE;

                     if (_ecore_window_grab_replay_func)
                        replay = _ecore_window_grab_replay_func(_ecore_window_grab_replay_data,
                                                                ECORE_EVENT_MOUSE_BUTTON_DOWN,
                                                                e);

                     /* FIXME: xcb_key_press_event_t does not save the */
                     /* connection. So I use the current one */
                     if (replay)
                        xcb_allow_events(_ecore_xcb_conn,
                                         XCB_ALLOW_REPLAY_POINTER,
                                         ev->time);
                     else
                        xcb_allow_events(_ecore_xcb_conn,
                                         XCB_ALLOW_ASYNC_POINTER,
                                         ev->time);

                     break;
                  }
             }
           if (child_window == event_window)
             {
                if (!_ecore_xcb_mouse_down_did_triple)
                  {
                     _ecore_xcb_mouse_down_last_last_window = _ecore_xcb_mouse_down_last_window;
                     if (ev->child)
                        _ecore_xcb_mouse_down_last_window = ev->child;
                     else
                        _ecore_xcb_mouse_down_last_window = ev->event;

                     _ecore_xcb_mouse_down_last_last_event_window = _ecore_xcb_mouse_down_last_event_window;
                     _ecore_xcb_mouse_down_last_event_window = ev->event;
                     _ecore_xcb_mouse_down_last_last_time = _ecore_xcb_mouse_down_last_time;
                     _ecore_xcb_mouse_down_last_time = ev->time;
                  }
             }
        }
     }

   free(event);
} /* _ecore_x_event_handle_button_press */

void
_ecore_x_event_handle_button_release(xcb_generic_event_t *event)
{
   xcb_button_release_event_t *ev;

   ev = (xcb_button_release_event_t *)event;
   _ecore_xcb_last_event_mouse_move = 0;
   /* filter out wheel buttons */
   if ((ev->detail <= 3) || (ev->detail > 7))
     {
        _ecore_mouse_move(ev->time, ev->state,
                          ev->event_x, ev->event_y,
                          ev->root_x, ev->root_y,
                          ev->event,
                          (ev->child ? ev->child : ev->event),
                          ev->root,
                          ev->same_screen);

        _ecore_mouse_button(ECORE_EVENT_MOUSE_BUTTON_UP,
                            ev->time, ev->state,
                            ev->detail,
                            ev->event_x, ev->event_y,
                            ev->root_x, ev->root_y,
                            ev->event,
                            (ev->child ? ev->child : ev->event),
                            ev->root,
                            ev->same_screen);
     }

   free(event);
} /* _ecore_x_event_handle_button_release */

void
_ecore_x_event_handle_motion_notify(xcb_generic_event_t *event)
{
   xcb_motion_notify_event_t *ev;

   ev = (xcb_motion_notify_event_t *)event;
   if (_ecore_xcb_last_event_mouse_move)
     {
        ecore_event_del(_ecore_xcb_last_event_mouse_move_event);
        _ecore_xcb_last_event_mouse_move = 0;
        _ecore_xcb_last_event_mouse_move_event = NULL;
     }

   _ecore_mouse_move(ev->time, ev->state,
                     ev->event_x, ev->event_y,
                     ev->root_x, ev->root_y,
                     ev->event,
                     (ev->child ? ev->child : ev->event),
                     ev->root,
                     ev->same_screen);

   _ecore_xcb_last_event_mouse_move = 1;

   /* Xdnd handling */
   _ecore_x_dnd_drag(ev->root, ev->root_x, ev->root_y);

   free(event);
} /* _ecore_x_event_handle_motion_notify */

void
_ecore_x_event_handle_enter_notify(xcb_generic_event_t *event)
{
   xcb_enter_notify_event_t *ev;

   ev = (xcb_enter_notify_event_t *)event;
   _ecore_xcb_last_event_mouse_move = 0;

   {
      _ecore_mouse_move(ev->time, ev->state,
                        ev->event_x, ev->event_y,
                        ev->root_x, ev->root_y,
                        ev->event,
                        (ev->child ? ev->child : ev->event),
                        ev->root,
                        ev->same_screen_focus);
   }
   {
      Ecore_X_Event_Mouse_In *e;

      e = calloc(1, sizeof(Ecore_X_Event_Mouse_In));
      if (!e)
         return;

      e->modifiers = _ecore_x_event_modifiers(ev->state);
      e->x = ev->event_x;
      e->y = ev->event_y;
      e->root.x = ev->root_x;
      e->root.y = ev->root_y;
      if (ev->child)
         e->win = ev->child;
      else
         e->win = ev->event;

      e->same_screen = ev->same_screen_focus;
      e->root_win = ev->root;
      e->event_win = ev->event;
      switch (ev->mode) {
         case XCB_NOTIFY_MODE_NORMAL:
            e->mode = ECORE_X_EVENT_MODE_NORMAL;
            break;

         case XCB_NOTIFY_MODE_GRAB:
            e->mode = ECORE_X_EVENT_MODE_GRAB;
            break;

         case XCB_NOTIFY_MODE_UNGRAB:
            e->mode = ECORE_X_EVENT_MODE_UNGRAB;
            break;

         default:
            e->mode = ECORE_X_EVENT_MODE_NORMAL;
            break;
        } /* switch */
      switch (ev->detail) {
         case XCB_NOTIFY_DETAIL_ANCESTOR:
            e->detail = ECORE_X_EVENT_DETAIL_ANCESTOR;
            break;

         case XCB_NOTIFY_DETAIL_VIRTUAL:
            e->detail = ECORE_X_EVENT_DETAIL_VIRTUAL;
            break;

         case XCB_NOTIFY_DETAIL_INFERIOR:
            e->detail = ECORE_X_EVENT_DETAIL_INFERIOR;
            break;

         case XCB_NOTIFY_DETAIL_NONLINEAR:
            e->detail = ECORE_X_EVENT_DETAIL_NON_LINEAR;
            break;

         case XCB_NOTIFY_DETAIL_NONLINEAR_VIRTUAL:
            e->detail = ECORE_X_EVENT_DETAIL_NON_LINEAR_VIRTUAL;
            break;

         default:
            e->detail = ECORE_X_EVENT_DETAIL_ANCESTOR;
            break;
        } /* switch */
      e->time = ev->time;
      _ecore_xcb_event_last_time = e->time;
      ecore_event_add(ECORE_X_EVENT_MOUSE_IN, e, NULL, NULL);
   }

   free(event);
} /* _ecore_x_event_handle_enter_notify */

void
_ecore_x_event_handle_leave_notify(xcb_generic_event_t *event)
{
   xcb_leave_notify_event_t *ev;

   ev = (xcb_leave_notify_event_t *)event;
   _ecore_xcb_last_event_mouse_move = 0;

   {
      _ecore_mouse_move(ev->time, ev->state,
                        ev->event_x, ev->event_y,
                        ev->root_x, ev->root_y,
                        ev->event,
                        (ev->child ? ev->child : ev->event),
                        ev->root,
                        ev->same_screen_focus);
   }
   {
      Ecore_X_Event_Mouse_Out *e;

      e = calloc(1, sizeof(Ecore_X_Event_Mouse_Out));
      if (!e)
         return;

      e->modifiers = _ecore_x_event_modifiers(ev->state);
      e->x = ev->event_x;
      e->y = ev->event_y;
      e->root.x = ev->root_x;
      e->root.y = ev->root_y;
      if (ev->child)
         e->win = ev->child;
      else
         e->win = ev->event;

      e->same_screen = ev->same_screen_focus;
      e->root_win = ev->root;
      e->event_win = ev->event;
      switch (ev->mode) {
         case XCB_NOTIFY_MODE_NORMAL:
            e->mode = ECORE_X_EVENT_MODE_NORMAL;
            break;

         case XCB_NOTIFY_MODE_GRAB:
            e->mode = ECORE_X_EVENT_MODE_GRAB;
            break;

         case XCB_NOTIFY_MODE_UNGRAB:
            e->mode = ECORE_X_EVENT_MODE_UNGRAB;
            break;

         default:
            e->mode = ECORE_X_EVENT_MODE_NORMAL;
            break;
        } /* switch */
      switch (ev->detail) {
         case XCB_NOTIFY_DETAIL_ANCESTOR:
            e->detail = ECORE_X_EVENT_DETAIL_ANCESTOR;
            break;

         case XCB_NOTIFY_DETAIL_VIRTUAL:
            e->detail = ECORE_X_EVENT_DETAIL_VIRTUAL;
            break;

         case XCB_NOTIFY_DETAIL_INFERIOR:
            e->detail = ECORE_X_EVENT_DETAIL_INFERIOR;
            break;

         case XCB_NOTIFY_DETAIL_NONLINEAR:
            e->detail = ECORE_X_EVENT_DETAIL_NON_LINEAR;
            break;

         case XCB_NOTIFY_DETAIL_NONLINEAR_VIRTUAL:
            e->detail = ECORE_X_EVENT_DETAIL_NON_LINEAR_VIRTUAL;
            break;

         default:
            e->detail = ECORE_X_EVENT_DETAIL_ANCESTOR;
            break;
        } /* switch */
      e->time = ev->time;
      _ecore_xcb_event_last_time = e->time;
      _ecore_xcb_event_last_window = e->win;
      _ecore_xcb_event_last_root_x = e->root.x;
      _ecore_xcb_event_last_root_y = e->root.y;
      ecore_event_add(ECORE_X_EVENT_MOUSE_OUT, e, NULL, NULL);
   }

   free(event);
} /* _ecore_x_event_handle_leave_notify */

void
_ecore_x_event_handle_focus_in(xcb_generic_event_t *event)
{
   xcb_focus_in_event_t *ev;
   Ecore_X_Event_Window_Focus_In *e;

   ev = (xcb_focus_in_event_t *)event;
   e = calloc(1, sizeof(Ecore_X_Event_Window_Focus_In));
   if (!e)
      return;

   e->win = ev->event;
   switch (ev->mode) {
      case XCB_NOTIFY_MODE_NORMAL:
         e->mode = ECORE_X_EVENT_MODE_NORMAL;
         break;

      case XCB_NOTIFY_MODE_GRAB:
         e->mode = ECORE_X_EVENT_MODE_GRAB;
         break;

      case XCB_NOTIFY_MODE_UNGRAB:
         e->mode = ECORE_X_EVENT_MODE_UNGRAB;
         break;

      case XCB_NOTIFY_MODE_WHILE_GRABBED:
         e->mode = ECORE_X_EVENT_MODE_WHILE_GRABBED;
         break;
     } /* switch */
   switch (ev->detail) {
      case XCB_NOTIFY_DETAIL_ANCESTOR:
         e->detail = ECORE_X_EVENT_DETAIL_ANCESTOR;
         break;

      case XCB_NOTIFY_DETAIL_VIRTUAL:
         e->detail = ECORE_X_EVENT_DETAIL_VIRTUAL;
         break;

      case XCB_NOTIFY_DETAIL_INFERIOR:
         e->detail = ECORE_X_EVENT_DETAIL_INFERIOR;
         break;

      case XCB_NOTIFY_DETAIL_NONLINEAR:
         e->detail = ECORE_X_EVENT_DETAIL_NON_LINEAR;
         break;

      case XCB_NOTIFY_DETAIL_NONLINEAR_VIRTUAL:
         e->detail = ECORE_X_EVENT_DETAIL_NON_LINEAR_VIRTUAL;
         break;

      case XCB_NOTIFY_DETAIL_POINTER:
         e->detail = ECORE_X_EVENT_DETAIL_POINTER;
         break;

      case XCB_NOTIFY_DETAIL_POINTER_ROOT:
         e->detail = ECORE_X_EVENT_DETAIL_POINTER_ROOT;
         break;

      case XCB_NOTIFY_DETAIL_NONE:
         e->detail = ECORE_X_EVENT_DETAIL_DETAIL_NONE;
         break;
     } /* switch */
   e->time = _ecore_xcb_event_last_time;
   _ecore_xcb_event_last_time = e->time;
   ecore_event_add(ECORE_X_EVENT_WINDOW_FOCUS_IN, e, NULL, NULL);

   free(event);
} /* _ecore_x_event_handle_focus_in */

void
_ecore_x_event_handle_focus_out(xcb_generic_event_t *event)
{
   xcb_focus_out_event_t *ev;
   Ecore_X_Event_Window_Focus_Out *e;

   ev = (xcb_focus_out_event_t *)event;
   e = calloc(1, sizeof(Ecore_X_Event_Window_Focus_Out));
   if (!e)
      return;

   e->win = ev->event;
   switch (ev->mode) {
      case XCB_NOTIFY_MODE_NORMAL:
         e->mode = ECORE_X_EVENT_MODE_NORMAL;
         break;

      case XCB_NOTIFY_MODE_GRAB:
         e->mode = ECORE_X_EVENT_MODE_GRAB;
         break;

      case XCB_NOTIFY_MODE_UNGRAB:
         e->mode = ECORE_X_EVENT_MODE_UNGRAB;
         break;

      case XCB_NOTIFY_MODE_WHILE_GRABBED:
         e->mode = ECORE_X_EVENT_MODE_WHILE_GRABBED;
         break;
     } /* switch */
   switch (ev->detail) {
      case XCB_NOTIFY_DETAIL_ANCESTOR:
         e->detail = ECORE_X_EVENT_DETAIL_ANCESTOR;
         break;

      case XCB_NOTIFY_DETAIL_VIRTUAL:
         e->detail = ECORE_X_EVENT_DETAIL_VIRTUAL;
         break;

      case XCB_NOTIFY_DETAIL_INFERIOR:
         e->detail = ECORE_X_EVENT_DETAIL_INFERIOR;
         break;

      case XCB_NOTIFY_DETAIL_NONLINEAR:
         e->detail = ECORE_X_EVENT_DETAIL_NON_LINEAR;
         break;

      case XCB_NOTIFY_DETAIL_NONLINEAR_VIRTUAL:
         e->detail = ECORE_X_EVENT_DETAIL_NON_LINEAR_VIRTUAL;
         break;

      case XCB_NOTIFY_DETAIL_POINTER:
         e->detail = ECORE_X_EVENT_DETAIL_POINTER;
         break;

      case XCB_NOTIFY_DETAIL_POINTER_ROOT:
         e->detail = ECORE_X_EVENT_DETAIL_POINTER_ROOT;
         break;

      case XCB_NOTIFY_DETAIL_NONE:
         e->detail = ECORE_X_EVENT_DETAIL_DETAIL_NONE;
         break;
     } /* switch */
   e->time = _ecore_xcb_event_last_time;
   _ecore_xcb_event_last_time = e->time;
   ecore_event_add(ECORE_X_EVENT_WINDOW_FOCUS_OUT, e, NULL, NULL);

   free(event);
} /* _ecore_x_event_handle_focus_out */

void
_ecore_x_event_handle_keymap_notify(xcb_generic_event_t *event)
{
   /* FIXME: handle this event type */

   free(event);
} /* _ecore_x_event_handle_keymap_notify */

void
_ecore_x_event_handle_expose(xcb_generic_event_t *event)
{
   xcb_expose_event_t *ev;
   Ecore_X_Event_Window_Damage *e;

   ev = (xcb_expose_event_t *)event,
   e = calloc(1, sizeof(Ecore_X_Event_Window_Damage));
   if (!e)
      return;

   e->win = ev->window;
   e->time = _ecore_xcb_event_last_time;
   e->x = ev->x;
   e->y = ev->y;
   e->w = ev->width;
   e->h = ev->height;
   e->count = ev->count;
   ecore_event_add(ECORE_X_EVENT_WINDOW_DAMAGE, e, NULL, NULL);

   free(event);
} /* _ecore_x_event_handle_expose */

void
_ecore_x_event_handle_graphics_expose(xcb_generic_event_t *event)
{
   xcb_graphics_exposure_event_t *ev;
   Ecore_X_Event_Window_Damage *e;

   ev = (xcb_graphics_exposure_event_t *)event;
   e = calloc(1, sizeof(Ecore_X_Event_Window_Damage));
   if (!e)
      return;

   e->win = ev->drawable;
   e->time = _ecore_xcb_event_last_time;
   e->x = ev->x;
   e->y = ev->y;
   e->w = ev->width;
   e->h = ev->height;
   e->count = ev->count;
   ecore_event_add(ECORE_X_EVENT_WINDOW_DAMAGE, e, NULL, NULL);

   free(event);
} /* _ecore_x_event_handle_graphics_expose */

void
_ecore_x_event_handle_visibility_notify(xcb_generic_event_t *event)
{
   xcb_visibility_notify_event_t *ev;

   ev = (xcb_visibility_notify_event_t *)event;
   if (ev->state != XCB_VISIBILITY_PARTIALLY_OBSCURED)
     {
        Ecore_X_Event_Window_Visibility_Change *e;

        e = calloc(1, sizeof(Ecore_X_Event_Window_Visibility_Change));
        if (!e)
           return;

        e->win = ev->window;
        e->time = _ecore_xcb_event_last_time;
        if (ev->state == XCB_VISIBILITY_FULLY_OBSCURED)
           e->fully_obscured = 1;
        else
           e->fully_obscured = 0;

        ecore_event_add(ECORE_X_EVENT_WINDOW_VISIBILITY_CHANGE, e, NULL, NULL);
     }

   free(event);
} /* _ecore_x_event_handle_visibility_notify */

void
_ecore_x_event_handle_create_notify(xcb_generic_event_t *event)
{
   xcb_create_notify_event_t *ev;
   Ecore_X_Event_Window_Create *e;

   ev = (xcb_create_notify_event_t *)event;
   e = calloc(1, sizeof(Ecore_X_Event_Window_Create));
   if (!e)
      return;

   e->win = ev->window;
   if (ev->override_redirect)
      e->override = 1;
   else
      e->override = 0;

   e->time = _ecore_xcb_event_last_time;
   ecore_event_add(ECORE_X_EVENT_WINDOW_CREATE, e, NULL, NULL);

   free(event);
} /* _ecore_x_event_handle_create_notify */

void
_ecore_x_event_handle_destroy_notify(xcb_generic_event_t *event)
{
   xcb_destroy_notify_event_t *ev;
   Ecore_X_Event_Window_Destroy *e;

   ev = (xcb_destroy_notify_event_t *)event;
   e = calloc(1, sizeof(Ecore_X_Event_Window_Destroy));
   if (!e)
      return;

   e->win = ev->window;
   e->time = _ecore_xcb_event_last_time;
   if (e->win == _ecore_xcb_event_last_window)
      _ecore_xcb_event_last_window = 0;

   ecore_event_add(ECORE_X_EVENT_WINDOW_DESTROY, e, NULL, NULL);

   free(event);
} /* _ecore_x_event_handle_destroy_notify */

void
_ecore_x_event_handle_unmap_notify(xcb_generic_event_t *event)
{
   xcb_unmap_notify_event_t *ev;
   Ecore_X_Event_Window_Hide *e;

   ev = (xcb_unmap_notify_event_t *)event;
   e = calloc(1, sizeof(Ecore_X_Event_Window_Hide));
   if (!e)
      return;

   e->win = ev->window;
   e->time = _ecore_xcb_event_last_time;
   ecore_event_add(ECORE_X_EVENT_WINDOW_HIDE, e, NULL, NULL);

   free(event);
} /* _ecore_x_event_handle_unmap_notify */

void
_ecore_x_event_handle_map_notify(xcb_generic_event_t *event)
{
   xcb_map_notify_event_t *ev;
   Ecore_X_Event_Window_Show *e;

   ev = (xcb_map_notify_event_t *)event;
   e = calloc(1, sizeof(Ecore_X_Event_Window_Show));
   if (!e)
      return;

   e->win = ev->window;
   e->time = _ecore_xcb_event_last_time;
   ecore_event_add(ECORE_X_EVENT_WINDOW_SHOW, e, NULL, NULL);

   free(event);
} /* _ecore_x_event_handle_map_notify */

void
_ecore_x_event_handle_map_request(xcb_generic_event_t *event)
{
   xcb_map_request_event_t *ev;
   Ecore_X_Event_Window_Show_Request *e;

   ev = (xcb_map_request_event_t *)event;
   e = calloc(1, sizeof(Ecore_X_Event_Window_Show_Request));
   if (!e)
      return;

   e->win = ev->window;
   e->parent = ev->parent;
   e->time = _ecore_xcb_event_last_time;
   ecore_event_add(ECORE_X_EVENT_WINDOW_SHOW_REQUEST, e, NULL, NULL);

   free(event);
} /* _ecore_x_event_handle_map_request */

void
_ecore_x_event_handle_reparent_notify(xcb_generic_event_t *event)
{
   xcb_reparent_notify_event_t *ev;
   Ecore_X_Event_Window_Reparent *e;

   ev = (xcb_reparent_notify_event_t *)event;
   e = calloc(1, sizeof(Ecore_X_Event_Window_Reparent));
   if (!e)
      return;

   e->win = ev->window;
   e->parent = ev->parent;
   e->time = _ecore_xcb_event_last_time;
   ecore_event_add(ECORE_X_EVENT_WINDOW_REPARENT, e, NULL, NULL);

   free(event);
} /* _ecore_x_event_handle_reparent_notify */

void
_ecore_x_event_handle_configure_notify(xcb_generic_event_t *event)
{
   xcb_configure_notify_event_t *ev;
   Ecore_X_Event_Window_Configure *e;

   ev = (xcb_configure_notify_event_t *)event;
   e = calloc(1, sizeof(Ecore_X_Event_Window_Configure));
   if (!e)
      return;

   e->win = ev->window;
   e->abovewin = ev->above_sibling;
   e->x = ev->x;
   e->y = ev->y;
   e->w = ev->width;
   e->h = ev->height;
   e->border = ev->border_width;
   e->override = ev->override_redirect;
   /* send_event is bit 7 (0x80) of response_type */
   e->from_wm = (ev->response_type & 0x80) ? 1 : 0;
   e->time = _ecore_xcb_event_last_time;
   ecore_event_add(ECORE_X_EVENT_WINDOW_CONFIGURE, e, NULL, NULL);

   free(event);
} /* _ecore_x_event_handle_configure_notify */

void
_ecore_x_event_handle_configure_request(xcb_generic_event_t *event)
{
   xcb_configure_request_event_t *ev;
   Ecore_X_Event_Window_Configure_Request *e;

   ev = (xcb_configure_request_event_t *)event;
   e = calloc(1, sizeof(Ecore_X_Event_Window_Configure_Request));
   if (!e)
      return;

   e->win = ev->window;
   e->abovewin = ev->sibling;
   e->x = ev->x;
   e->y = ev->y;
   e->w = ev->width;
   e->h = ev->height;
   e->border = ev->border_width;
   e->value_mask = ev->value_mask;
   switch (ev->stack_mode) {
      case XCB_STACK_MODE_ABOVE:
         e->detail = ECORE_X_WINDOW_STACK_ABOVE;
         break;

      case XCB_STACK_MODE_BELOW:
         e->detail = ECORE_X_WINDOW_STACK_BELOW;
         break;

      case XCB_STACK_MODE_TOP_IF:
         e->detail = ECORE_X_WINDOW_STACK_TOP_IF;
         break;

      case XCB_STACK_MODE_BOTTOM_IF:
         e->detail = ECORE_X_WINDOW_STACK_BOTTOM_IF;
         break;

      case XCB_STACK_MODE_OPPOSITE:
         e->detail = ECORE_X_WINDOW_STACK_OPPOSITE;
         break;
     } /* switch */
   e->time = _ecore_xcb_event_last_time;
   ecore_event_add(ECORE_X_EVENT_WINDOW_CONFIGURE_REQUEST, e, NULL, NULL);

   free(event);
} /* _ecore_x_event_handle_configure_request */

void
_ecore_x_event_handle_gravity_notify(xcb_generic_event_t *event)
{
   /* FIXME: handle this event type */

   free(event);
} /* _ecore_x_event_handle_gravity_notify */

void
_ecore_x_event_handle_resize_request(xcb_generic_event_t *event)
{
   xcb_resize_request_event_t *ev;
   Ecore_X_Event_Window_Resize_Request *e;

   ev = (xcb_resize_request_event_t *)event;
   e = calloc(1, sizeof(Ecore_X_Event_Window_Resize_Request));
   if (!e)
      return;

   e->win = ev->window;
   e->w = ev->width;
   e->h = ev->height;
   e->time = _ecore_xcb_event_last_time;
   ecore_event_add(ECORE_X_EVENT_WINDOW_RESIZE_REQUEST, e, NULL, NULL);

   free(event);
} /* _ecore_x_event_handle_resize_request */

void
_ecore_x_event_handle_circulate_notify(xcb_generic_event_t *event)
{
   xcb_circulate_notify_event_t *ev;
   Ecore_X_Event_Window_Stack *e;

   ev = (xcb_circulate_notify_event_t *)event;
   e = calloc(1, sizeof(Ecore_X_Event_Window_Stack));
   if (!e)
      return;

   e->win = ev->window;
   e->event_win = ev->event;
   if (ev->place == XCB_PLACE_ON_TOP)
      e->detail = ECORE_X_WINDOW_STACK_ABOVE;
   else
      e->detail = ECORE_X_WINDOW_STACK_BELOW;

   e->time = _ecore_xcb_event_last_time;
   ecore_event_add(ECORE_X_EVENT_WINDOW_STACK, e, NULL, NULL);

   free(event);
} /* _ecore_x_event_handle_circulate_notify */

void
_ecore_x_event_handle_circulate_request(xcb_generic_event_t *event)
{
   xcb_circulate_request_event_t *ev;
   Ecore_X_Event_Window_Stack_Request *e;

   ev = (xcb_circulate_request_event_t *)event;
   e = calloc(1, sizeof(Ecore_X_Event_Window_Stack_Request));
   if (!e)
      return;

   e->win = ev->window;
   e->parent = ev->event;
   if (ev->place == XCB_PLACE_ON_TOP)
      e->detail = ECORE_X_WINDOW_STACK_ABOVE;
   else
      e->detail = ECORE_X_WINDOW_STACK_BELOW;

   e->time = _ecore_xcb_event_last_time;
   ecore_event_add(ECORE_X_EVENT_WINDOW_STACK_REQUEST, e, NULL, NULL);

   free(event);
} /* _ecore_x_event_handle_circulate_request */

void
_ecore_x_event_handle_property_notify(xcb_generic_event_t *event)
{
#if 0 /* for now i disabled this. nice idea though this is - it leaves a lot
       * to be desired for efficiency that is better left to the app layer
       */
   if (xevent->xproperty.atom == ECORE_X_ATOM_WM_CLASS)
     {
        Ecore_X_Event_Window_Prop_Name_Class_Change *e;

        e = calloc(1, sizeof(Ecore_X_Event_Window_Prop_Name_Class_Change));
        if (!e)
           return;

        ecore_x_window_prop_name_class_get(xevent->xproperty.window,
                                           &(e->name), &(e->clas));
        e->time = xevent->xproperty.time;
        _ecore_x_event_last_time = e->time;
        ecore_event_add(ECORE_X_EVENT_WINDOW_PROP_NAME_CLASS_CHANGE, e, _ecore_x_event_free_window_prop_name_class_change, NULL);
     }
   else if ((xevent->xproperty.atom == ECORE_X_ATOM_WM_NAME) || (xevent->xproperty.atom == ECORE_X_ATOM_NET_WM_NAME))
     {
        Ecore_X_Event_Window_Prop_Title_Change *e;

        e = calloc(1, sizeof(Ecore_X_Event_Window_Prop_Title_Change));
        if (!e)
           return;

        e->title = ecore_x_window_prop_title_get(xevent->xproperty.window);
        e->time = xevent->xproperty.time;
        _ecore_x_event_last_time = e->time;
        ecore_event_add(ECORE_X_EVENT_WINDOW_PROP_TITLE_CHANGE, e, _ecore_x_event_free_window_prop_title_change, NULL);
     }
   else if (xevent->xproperty.atom == ECORE_X_ATOM_NET_WM_VISIBLE_NAME)
     {
        Ecore_X_Event_Window_Prop_Visible_Title_Change *e;

        e = calloc(1, sizeof(Ecore_X_Event_Window_Prop_Visible_Title_Change));
        if (!e)
           return;

        e->title = ecore_x_window_prop_visible_title_get(xevent->xproperty.window);
        e->time = xevent->xproperty.time;
        _ecore_x_event_last_time = e->time;
        ecore_event_add(ECORE_X_EVENT_WINDOW_PROP_VISIBLE_TITLE_CHANGE, e, _ecore_x_event_free_window_prop_visible_title_change, NULL);
     }
   else if ((xevent->xproperty.atom == ECORE_X_ATOM_WM_ICON_NAME) || (xevent->xproperty.atom == ECORE_X_ATOM_NET_WM_ICON_NAME))
     {
        Ecore_X_Event_Window_Prop_Icon_Name_Change *e;

        e = calloc(1, sizeof(Ecore_X_Event_Window_Prop_Icon_Name_Change));
        if (!e)
           return;

        e->name = ecore_x_window_prop_icon_name_get(xevent->xproperty.window);
        e->time = xevent->xproperty.time;
        _ecore_x_event_last_time = e->time;
        ecore_event_add(ECORE_X_EVENT_WINDOW_PROP_ICON_NAME_CHANGE, e, _ecore_x_event_free_window_prop_icon_name_change, NULL);
     }
   else if (xevent->xproperty.atom == ECORE_X_ATOM_NET_WM_VISIBLE_ICON_NAME)
     {
        Ecore_X_Event_Window_Prop_Visible_Icon_Name_Change *e;

        e = calloc(1, sizeof(Ecore_X_Event_Window_Prop_Visible_Icon_Name_Change));
        if (!e)
           return;

        e->name = ecore_x_window_prop_visible_icon_name_get(xevent->xproperty.window);
        e->time = xevent->xproperty.time;
        _ecore_x_event_last_time = e->time;
        ecore_event_add(ECORE_X_EVENT_WINDOW_PROP_VISIBLE_ICON_NAME_CHANGE, e, _ecore_x_event_free_window_prop_visible_icon_name_change, NULL);
     }
   else if (xevent->xproperty.atom == ECORE_X_ATOM_WM_CLIENT_MACHINE)
     {
        Ecore_X_Event_Window_Prop_Client_Machine_Change *e;

        e = calloc(1, sizeof(Ecore_X_Event_Window_Prop_Client_Machine_Change));
        if (!e)
           return;

        e->name = ecore_x_window_prop_client_machine_get(xevent->xproperty.window);
        e->time = xevent->xproperty.time;
        _ecore_x_event_last_time = e->time;
        ecore_event_add(ECORE_X_EVENT_WINDOW_PROP_CLIENT_MACHINE_CHANGE, e, _ecore_x_event_free_window_prop_client_machine_change, NULL);
     }
   else if (xevent->xproperty.atom == ECORE_X_ATOM_NET_WM_PID)
     {
        Ecore_X_Event_Window_Prop_Pid_Change *e;

        e = calloc(1, sizeof(Ecore_X_Event_Window_Prop_Pid_Change));
        if (!e)
           return;

        e->pid = ecore_x_window_prop_pid_get(xevent->xproperty.window);
        e->time = xevent->xproperty.time;
        _ecore_x_event_last_time = e->time;
        ecore_event_add(ECORE_X_EVENT_WINDOW_PROP_PID_CHANGE, e, NULL, NULL);
     }
   else if (xevent->xproperty.atom == ECORE_X_ATOM_NET_WM_DESKTOP)
     {
        Ecore_X_Event_Window_Prop_Desktop_Change *e;

        e = calloc(1, sizeof(Ecore_X_Event_Window_Prop_Desktop_Change));
        if (!e)
           return;

        e->desktop = ecore_x_window_prop_desktop_get(xevent->xproperty.window);
        ecore_event_add(ECORE_X_EVENT_WINDOW_PROP_PID_CHANGE, e, NULL, NULL);
     }
   else
#endif /* if 0 */
   {
      xcb_property_notify_event_t *ev;
      Ecore_X_Event_Window_Property *e;

      ev = (xcb_property_notify_event_t *)event;
      e = calloc(1,sizeof(Ecore_X_Event_Window_Property));
      if (!e)
         return;

      e->win = ev->window;
      e->atom = ev->atom;
      e->time = ev->time;
      _ecore_xcb_event_last_time = e->time;
      ecore_event_add(ECORE_X_EVENT_WINDOW_PROPERTY, e, NULL, NULL);
   }

   free(event);
} /* _ecore_x_event_handle_property_notify */

void
_ecore_x_event_handle_selection_clear(xcb_generic_event_t *event)
{
   xcb_selection_clear_event_t *ev;
   Ecore_X_Selection_Intern *d;
   Ecore_X_Event_Selection_Clear *e;
   Ecore_X_Atom sel;

   ev = (xcb_selection_clear_event_t *)event;
   d = _ecore_x_selection_get(ev->selection);
   if (d && (ev->time > d->time))
     {
        _ecore_x_selection_set(XCB_NONE, NULL, 0,
                               ev->selection);
     }

   /* Generate event for app cleanup */
   e = malloc(sizeof(Ecore_X_Event_Selection_Clear));
   e->win = ev->owner;
   e->time = ev->time;
   e->atom = sel = ev->selection;
   if (sel == ECORE_X_ATOM_SELECTION_PRIMARY)
      e->selection = ECORE_X_SELECTION_PRIMARY;
   else if (sel == ECORE_X_ATOM_SELECTION_SECONDARY)
      e->selection = ECORE_X_SELECTION_SECONDARY;
   else if (sel == ECORE_X_ATOM_SELECTION_CLIPBOARD)
      e->selection = ECORE_X_SELECTION_CLIPBOARD;
   else
      e->selection = ECORE_X_SELECTION_OTHER;

   ecore_event_add(ECORE_X_EVENT_SELECTION_CLEAR, e, NULL, NULL);

   free(event);
} /* _ecore_x_event_handle_selection_clear */

void
_ecore_x_event_handle_selection_request(xcb_generic_event_t *event)
{
   xcb_selection_request_event_t *ev;
   Ecore_X_Event_Selection_Request *e;
   Ecore_X_Selection_Intern *sd;
   void *data;
   int len;
   int typesize;

   ev = (xcb_selection_request_event_t *)event;
   _ecore_xcb_last_event_mouse_move = 0;

   /*
    * Generate a selection request event.
    */
   e = malloc(sizeof(Ecore_X_Event_Selection_Request));
   e->owner = ev->owner;
   e->requestor = ev->requestor;
   e->time = ev->time;
   e->selection = ev->selection;
   e->target = ev->target;
   e->property = ev->property;
   ecore_event_add(ECORE_X_EVENT_SELECTION_REQUEST, e, NULL, NULL);

   if ((sd = _ecore_x_selection_get(ev->selection)) &&
       (sd->win == ev->owner))
     {
        Ecore_X_Selection_Intern *si;

        si = _ecore_x_selection_get(ev->selection);
        if (si->data)
          {
             Ecore_X_Atom property;
             Ecore_X_Atom type;

             /* Set up defaults for strings first */
             type = ev->target;
             typesize = 8;
             len = sd->length;

             if (!ecore_x_selection_convert(ev->selection, ev->target,
                                            &data, &len, &type, &typesize))
               {
                  /* Refuse selection, conversion to requested target failed */
                  property = XCB_NONE;
               }
             else
               {
                  /* FIXME: This does not properly handle large data transfers */
                  ecore_x_window_prop_property_set(ev->requestor,
                                                   ev->property,
                                                   ev->target,
                                                   8, data, sd->length);
                  property = ev->property;
                  free(data);
               }

             ecore_x_selection_notify_send(ev->requestor,
                                           ev->selection,
                                           ev->target,
                                           property,
                                           ev->time);
          }
     }
} /* _ecore_x_event_handle_selection_request */

/* FIXME: round trip */
void
_ecore_x_event_handle_selection_notify(xcb_generic_event_t *event)
{
   xcb_selection_notify_event_t *ev;
   Ecore_X_Event_Selection_Notify *e;
   unsigned char *data = NULL;
   Ecore_X_Atom selection;
   int num_ret;
   uint8_t format;

   ev = (xcb_selection_notify_event_t *)event;
   selection = ev->selection;

   if (ev->target == ECORE_X_ATOM_SELECTION_TARGETS)
     {
        ecore_x_window_prop_property_get_prefetch(ev->requestor,
                                                  ev->property,
                                                  ECORE_X_ATOM_ATOM);
        ecore_x_window_prop_property_get_fetch();
        format = ecore_x_window_prop_property_get(ev->requestor,
                                                  ev->property,
                                                  ECORE_X_ATOM_ATOM,
                                                  32,
                                                  &data,
                                                  &num_ret);
        if (!format)
           return;
     }
   else
     {
        ecore_x_window_prop_property_get_prefetch(ev->requestor,
                                                  ev->property,
                                                  XCB_GET_PROPERTY_TYPE_ANY);
        ecore_x_window_prop_property_get_fetch();
        format = ecore_x_window_prop_property_get(ev->requestor,
                                                  ev->property,
                                                  ECORE_X_ATOM_ATOM,
                                                  8,
                                                  &data,
                                                  &num_ret);
        if (!format)
           return;
     }

   e = calloc(1, sizeof(Ecore_X_Event_Selection_Notify));
   if (!e)
      return;

   e->win = ev->requestor;
   e->time = ev->time;
   e->atom = selection;
   e->target = _ecore_x_selection_target_get(ev->target);

   if (selection == ECORE_X_ATOM_SELECTION_PRIMARY)
      e->selection = ECORE_X_SELECTION_PRIMARY;
   else if (selection == ECORE_X_ATOM_SELECTION_SECONDARY)
      e->selection = ECORE_X_SELECTION_SECONDARY;
   else if (selection == ECORE_X_ATOM_SELECTION_XDND)
      e->selection = ECORE_X_SELECTION_XDND;
   else if (selection == ECORE_X_ATOM_SELECTION_CLIPBOARD)
      e->selection = ECORE_X_SELECTION_CLIPBOARD;
   else
      e->selection = ECORE_X_SELECTION_OTHER;

   e->data = _ecore_x_selection_parse(e->target, data, num_ret, format);

   ecore_event_add(ECORE_X_EVENT_SELECTION_NOTIFY, e, _ecore_x_event_free_selection_notify, NULL);

   free(event);
} /* _ecore_x_event_handle_selection_notify */

void
_ecore_x_event_handle_colormap_notify(xcb_generic_event_t *event)
{
   xcb_colormap_notify_event_t *ev;
   Ecore_X_Event_Window_Colormap *e;

   ev = (xcb_colormap_notify_event_t *)event;
   e = calloc(1,sizeof(Ecore_X_Event_Window_Colormap));
   if (!e)
      return;

   e->win = ev->window;
   e->cmap = ev->colormap;
   if (ev->state == XCB_COLORMAP_STATE_INSTALLED)
      e->installed = 1;
   else
      e->installed = 0;

   e->time = _ecore_xcb_event_last_time;
   ecore_event_add(ECORE_X_EVENT_WINDOW_COLORMAP, e, NULL, NULL);

   free(event);
} /* _ecore_x_event_handle_colormap_notify */

void
_ecore_x_event_handle_client_message(xcb_generic_event_t *event)
{
   /* Special client message event handling here. need to put LOTS of if */
   /* checks here and generate synthetic events per special message known */
   /* otherwise generate generic client message event. this would handle*/
   /* netwm, ICCCM, gnomewm, old kde and mwm hint client message protocols */

   xcb_client_message_event_t *ev;

   ev = (xcb_client_message_event_t *)event;
   if ((ev->type == ECORE_X_ATOM_WM_PROTOCOLS) &&
       (ev->format == 32) &&
       (ev->data.data32[0] == (uint32_t)ECORE_X_ATOM_WM_DELETE_WINDOW))
     {
        Ecore_X_Event_Window_Delete_Request *e;

        e = calloc(1, sizeof(Ecore_X_Event_Window_Delete_Request));
        if (!e)
           return;

        e->win = ev->window;
        e->time = _ecore_xcb_event_last_time;
        ecore_event_add(ECORE_X_EVENT_WINDOW_DELETE_REQUEST, e, NULL, NULL);
     }
   else if ((ev->type == ECORE_X_ATOM_NET_WM_MOVERESIZE) &&
            (ev->format == 32) &&
     /* Ignore move and resize with keyboard */
            (ev->data.data32[2] < 9))
     {
        Ecore_X_Event_Window_Move_Resize_Request *e;

        e = calloc(1, sizeof(Ecore_X_Event_Window_Move_Resize_Request));
        if (!e)
           return;

        e->win = ev->window;
        e->x = ev->data.data32[0];
        e->y = ev->data.data32[1];
        e->direction = ev->data.data32[2];
        e->button = ev->data.data32[3];
        e->source = ev->data.data32[4];
        ecore_event_add(ECORE_X_EVENT_WINDOW_MOVE_RESIZE_REQUEST, e, NULL, NULL);
     }
   /* Xdnd Client Message Handling Begin */
   /* Message Type: XdndEnter target */
   else if (ev->type == ECORE_X_ATOM_XDND_ENTER)
     {
        Ecore_X_Event_Xdnd_Enter *e;
        Ecore_X_DND_Target *target;
        uint32_t three;

        e = calloc(1, sizeof(Ecore_X_Event_Xdnd_Enter));
        if (!e)
           return;

        target = _ecore_x_dnd_target_get();
        target->state = ECORE_X_DND_TARGET_ENTERED;

        target = _ecore_x_dnd_target_get();
        target->source = ev->data.data32[0];
        target->win = ev->window;
        target->version = ev->data.data32[1] >> 24;
        if (target->version > ECORE_X_DND_VERSION)
          {
             WRN("DND: Requested version %d, we only support up to %d", target->version,
                 ECORE_X_DND_VERSION);
             return;
          }

        /* FIXME: roud trip, but I don't know how to suppress it */
        if ((three = ev->data.data32[1] & 0x1UL))
          {
             /* source supports more than 3 types, fetch property */
             unsigned char *data;
             Ecore_X_Atom *types;
             int num_ret;
             int i;
             uint8_t format;

             ecore_x_window_prop_property_get_prefetch(target->source,
                                                       ECORE_X_ATOM_XDND_TYPE_LIST,
                                                       ECORE_X_ATOM_ATOM);
             ecore_x_window_prop_property_get_fetch();
             format = ecore_x_window_prop_property_get(target->source,
                                                       ECORE_X_ATOM_XDND_TYPE_LIST,
                                                       ECORE_X_ATOM_ATOM,
                                                       32,
                                                       &data,
                                                       &num_ret);
             if (!format)
               {
                  ERR("DND: Could not fetch data type list from source window, aborting.");
                  return;
               }

             types = (Ecore_X_Atom *)data;
             e->types = calloc(num_ret, sizeof(char *));
             if (e->types)
               {
                  xcb_get_atom_name_cookie_t *cookies;

                  cookies = (xcb_get_atom_name_cookie_t *)malloc(sizeof(xcb_get_atom_name_cookie_t) * num_ret);
                  for (i = 0; i < num_ret; i++)
                     cookies[i] = xcb_get_atom_name_unchecked(_ecore_xcb_conn, types[i]);
                  for (i = 0; i < num_ret; i++)
                    {
                       xcb_get_atom_name_reply_t *reply;
                       char *name;

                       reply = xcb_get_atom_name_reply(_ecore_xcb_conn, cookies[i], NULL);
                       if (reply)
                         {
                            name = (char *)malloc(sizeof (char) * (reply->name_len + 1));
                            memcpy(name,
                                   xcb_get_atom_name_name(reply),
                                   reply->name_len);
                            name[reply->name_len] = '\0';
                            e->types[i] = name;
                            free(reply);
                         }
                    }
                  free(cookies);
               }

             e->num_types = num_ret;
          }
        else
          {
             int i = 0;

             e->types = calloc(3, sizeof(char *));
             if (e->types)
               {
                  xcb_get_atom_name_cookie_t cookies[3];

                  for (i = 0; i < 3; i++)
                     cookies[i] = xcb_get_atom_name_unchecked(_ecore_xcb_conn, ev->data.data32[i + 2]);
                  for (i = 0; i < 3; i++)
                    {
                       xcb_get_atom_name_reply_t *reply;
                       char *name;

                       reply = xcb_get_atom_name_reply(_ecore_xcb_conn, cookies[i], NULL);
                       if (reply && (ev->data.data32[i + 2]))
                         {
                            name = (char *)malloc(sizeof (char) * (reply->name_len + 1));
                            memcpy(name,
                                   xcb_get_atom_name_name(reply),
                                   reply->name_len);
                            name[reply->name_len] = '\0';
                            e->types[i] = name;
                         }

                       if (reply)
                          free(reply);
                    }
               }

             e->num_types = i;
          }

        e->win = target->win;
        e->source = target->source;
        ecore_event_add(ECORE_X_EVENT_XDND_ENTER, e, _ecore_x_event_free_xdnd_enter, NULL);
     }
   /* Message Type: XdndPosition target */
   else if (ev->type == ECORE_X_ATOM_XDND_POSITION)
     {
        Ecore_X_Event_Xdnd_Position *e;
        Ecore_X_DND_Target *target;

        target = _ecore_x_dnd_target_get();
        if ((target->source != (Ecore_X_Window)ev->data.data32[0]) ||
            (target->win != ev->window))
           return;

        target->pos.x = (int16_t)ev->data.data32[2] >> 16;
        target->pos.y = (int16_t)ev->data.data32[2] & 0xFFFFUL;
        target->action = ev->data.data32[4]; /* Version 2 */

        target->time = (target->version >= 1) ?
           ev->data.data32[3] : XCB_CURRENT_TIME;

        e = calloc(1, sizeof(Ecore_X_Event_Xdnd_Position));
        if (!e)
           return;

        e->win = target->win;
        e->source = target->source;
        e->position.x = target->pos.x;
        e->position.y = target->pos.y;
        e->action = target->action;
        ecore_event_add(ECORE_X_EVENT_XDND_POSITION, e, NULL, NULL);
     }
   /* Message Type: XdndStatus source */
   else if (ev->type == ECORE_X_ATOM_XDND_STATUS)
     {
        Ecore_X_Event_Xdnd_Status *e;
        Ecore_X_DND_Source *source;

        source = _ecore_x_dnd_source_get();
        /* Make sure source/target match */
        if ((source->win != ev->window) ||
            (source->dest != ev->data.data32[0]))
           return;

        source->await_status = 0;

        source->will_accept = ev->data.data32[1] & 0x1UL;
        source->suppress = (ev->data.data32[1] & 0x2UL) ? 0 : 1;

        source->rectangle.x = (int16_t)ev->data.data32[2] >> 16;
        source->rectangle.y = (int16_t)ev->data.data32[2] & 0xFFFFUL;
        source->rectangle.width = (uint16_t)ev->data.data32[3] >> 16;
        source->rectangle.height = (uint16_t)ev->data.data32[3] & 0xFFFFUL;

        source->accepted_action = ev->data.data32[4];

        e = calloc(1, sizeof(Ecore_X_Event_Xdnd_Status));
        if (!e)
           return;

        e->win = source->win;
        e->target = source->dest;
        e->will_accept = source->will_accept;
        e->rectangle.x = source->rectangle.x;
        e->rectangle.y = source->rectangle.y;
        e->rectangle.width = source->rectangle.width;
        e->rectangle.height = source->rectangle.height;
        e->action = source->accepted_action;

        ecore_event_add(ECORE_X_EVENT_XDND_STATUS, e, NULL, NULL);
     }
   /* Message Type: XdndLeave target */
   /* Pretend the whole thing never happened, sort of */
   else if (ev->type == ECORE_X_ATOM_XDND_LEAVE)
     {
        Ecore_X_Event_Xdnd_Leave *e;
        Ecore_X_DND_Target *target;

        target = _ecore_x_dnd_target_get();
        if ((target->source != (Ecore_X_Window)ev->data.data32[0]) ||
            (target->win != ev->window))
           return;

        target->state = ECORE_X_DND_TARGET_IDLE;

        e = calloc(1, sizeof(Ecore_X_Event_Xdnd_Leave));
        if (!e)
           return;

        e->win = ev->window;
        e->source = ev->data.data32[0];
        ecore_event_add(ECORE_X_EVENT_XDND_LEAVE, e, NULL, NULL);
     }
   /* Message Type: XdndDrop target */
   else if (ev->type == ECORE_X_ATOM_XDND_DROP)
     {
        Ecore_X_Event_Xdnd_Drop *e;
        Ecore_X_DND_Target *target;

        target = _ecore_x_dnd_target_get();
        /* Match source/target */
        if ((target->source != (Ecore_X_Window)ev->data.data32[0]) ||
            (target->win != ev->window))
           return;

        target->time = (target->version >= 1) ?
           ev->data.data32[2] : _ecore_xcb_event_last_time;

        e = calloc(1, sizeof(Ecore_X_Event_Xdnd_Drop));
        if (!e)
           return;

        e->win = target->win;
        e->source = target->source;
        e->action = target->action;
        e->position.x = target->pos.x;
        e->position.y = target->pos.y;
        ecore_event_add(ECORE_X_EVENT_XDND_DROP, e, NULL, NULL);
     }
   /* Message Type: XdndFinished source */
   else if (ev->type == ECORE_X_ATOM_XDND_FINISHED)
     {
        Ecore_X_Event_Xdnd_Finished *e;
        Ecore_X_DND_Source *source;
        uint8_t completed = 1;

        source = _ecore_x_dnd_source_get();
        /* Match source/target */
        if ((source->win != ev->window) ||
            (source->dest != ev->data.data32[0]))
           return;

        if ((source->version >= 5) && (ev->data.data32[1] & 0x1UL))
          {
             /* Target successfully performed drop action */
             ecore_x_selection_xdnd_clear();
             source->state = ECORE_X_DND_SOURCE_IDLE;
          }
        else
          {
             completed = 0;
             source->state = ECORE_X_DND_SOURCE_CONVERTING;

             /* FIXME: Probably need to add a timer to switch back to idle
              * and discard the selection data */
          }

        e = calloc(1, sizeof(Ecore_X_Event_Xdnd_Finished));
        if (!e)
           return;

        e->win = source->win;
        e->target = source->dest;
        e->completed = completed;
        if (source->version >= 5)
          {
             source->accepted_action = ev->data.data32[2];
             e->action = source->accepted_action;
          }
        else
          {
             source->accepted_action = 0;
             e->action = source->action;
          }

        ecore_event_add(ECORE_X_EVENT_XDND_FINISHED, e, NULL, NULL);
     }
   else if (ev->type == ECORE_X_ATOM_NET_WM_STATE)
     {
        Ecore_X_Event_Window_State_Request *e;

        e = calloc(1, sizeof(Ecore_X_Event_Window_State_Request));
        if (!e)
           return;

        e->win = ev->window;
        if (ev->data.data32[0] == 0)
           e->action = ECORE_X_WINDOW_STATE_ACTION_REMOVE;
        else if (ev->data.data32[0] == 1)
           e->action = ECORE_X_WINDOW_STATE_ACTION_ADD;
        else if (ev->data.data32[0] == 2)
           e->action = ECORE_X_WINDOW_STATE_ACTION_TOGGLE;
        else
          {
             free(e);
             return;
          }

        e->state[0] = _ecore_x_netwm_state_get(ev->data.data32[1]);
        if (e->state[0] == ECORE_X_WINDOW_STATE_UNKNOWN)
          {
             xcb_get_atom_name_reply_t *reply;
             char *name;

             /* FIXME: round trip */
             reply = xcb_get_atom_name_reply(_ecore_xcb_conn,
                                             xcb_get_atom_name_unchecked(_ecore_xcb_conn, ev->data.data32[1]),
                                             NULL);
             if (reply)
               {
                  name = (char *)malloc(sizeof (char) * (reply->name_len + 1));
                  memcpy(name,
                         xcb_get_atom_name_name(reply),
                         reply->name_len);
                  name[reply->name_len] = '\0';
                  ERR("Unknown state: %s", name);
                  free(name);
                  free(reply);
               }
          }

        e->state[1] = _ecore_x_netwm_state_get(ev->data.data32[2]);
        if (e->state[1] == ECORE_X_WINDOW_STATE_UNKNOWN)
          {
             xcb_get_atom_name_reply_t *reply;
             char *name;

             reply = xcb_get_atom_name_reply(_ecore_xcb_conn,
                                             xcb_get_atom_name_unchecked(_ecore_xcb_conn, ev->data.data32[2]),
                                             NULL);
             if (reply)
               {
                  name = (char *)malloc(sizeof (char) * (reply->name_len + 1));
                  memcpy(name,
                         xcb_get_atom_name_name(reply),
                         reply->name_len);
                  name[reply->name_len] = '\0';
                  WRN("Unknown state: %s", name);
                  free(name);
               }
          }

        e->source = ev->data.data32[3];

        ecore_event_add(ECORE_X_EVENT_WINDOW_STATE_REQUEST, e, NULL, NULL);
     }
   else if ((ev->type == ECORE_X_ATOM_WM_CHANGE_STATE)
            && (ev->format == 32)
            && (ev->data.data32[0] == XCB_WM_HINT_STATE))
     {
        Ecore_X_Event_Window_State_Request *e;

        e = calloc(1, sizeof(Ecore_X_Event_Window_State_Request));
        if (!e)
           return;

        e->win = ev->window;
        e->action = ECORE_X_WINDOW_STATE_ACTION_ADD;
        e->state[0] = ECORE_X_WINDOW_STATE_ICONIFIED;

        ecore_event_add(ECORE_X_EVENT_WINDOW_STATE_REQUEST, e, NULL, NULL);
     }
   else if ((ev->type == ECORE_X_ATOM_NET_WM_DESKTOP)
            && (ev->format == 32))
     {
        Ecore_X_Event_Desktop_Change *e;

        e = calloc(1, sizeof(Ecore_X_Event_Desktop_Change));
        if (!e)
           return;

        e->win = ev->window;
        e->desk = ev->data.data32[0];
        e->source = ev->data.data32[1];

        ecore_event_add(ECORE_X_EVENT_DESKTOP_CHANGE, e, NULL, NULL);
     }
   else if ((ev->type == ECORE_X_ATOM_NET_REQUEST_FRAME_EXTENTS))
     {
        Ecore_X_Event_Frame_Extents_Request *e;

        e = calloc(1, sizeof(Ecore_X_Event_Frame_Extents_Request));
        if (!e)
           return;

        e->win = ev->window;

        ecore_event_add(ECORE_X_EVENT_FRAME_EXTENTS_REQUEST, e, NULL, NULL);
     }
   else if ((ev->type == ECORE_X_ATOM_WM_PROTOCOLS)
            && ((Ecore_X_Atom)ev->data.data32[0] == ECORE_X_ATOM_NET_WM_PING)
            && (ev->format == 32))
     {
        Ecore_X_Event_Ping *e;

        e = calloc(1, sizeof(Ecore_X_Event_Ping));
        if (!e)
           return;

        e->win = ev->window;
        e->time = ev->data.data32[1];
        e->event_win = ev->data.data32[2];

        ecore_event_add(ECORE_X_EVENT_PING, e, NULL, NULL);
     }
   else if ((ev->type == ECORE_X_ATOM_NET_STARTUP_INFO_BEGIN) &&
            (ev->format == 8))
     {
        _ecore_x_netwm_startup_info_begin(ev->window, (char *)ev->data.data8);
     }
   else if ((ev->type == ECORE_X_ATOM_NET_STARTUP_INFO) &&
            (ev->format == 8))
     {
        _ecore_x_netwm_startup_info(ev->window, (char *)ev->data.data8);
     }
   else if ((ev->type == 27777)
            && (ev->data.data32[0] == 0x7162534)
            && (ev->format == 32)
            && (ev->window == _ecore_xcb_private_window))
     {
        /* a grab sync marker */
        if (ev->data.data32[1] == 0x10000001)
           _ecore_x_window_grab_remove(ev->data.data32[2]);
        else if (ev->data.data32[1] == 0x10000002)
           _ecore_x_key_grab_remove(ev->data.data32[2]);
     }
   else
     {
        Ecore_X_Event_Client_Message *e;
        int i;

        e = calloc(1, sizeof(Ecore_X_Event_Client_Message));
        if (!e)
           return;

        e->win = ev->window;
        e->message_type = ev->type;
        e->format = ev->format;
        for (i = 0; i < 5; i++)
           e->data.l[i] = ev->data.data32[i];

        ecore_event_add(ECORE_X_EVENT_CLIENT_MESSAGE, e, NULL, NULL);
     }

   free(event);
} /* _ecore_x_event_handle_client_message */

void
_ecore_x_event_handle_mapping_notify(xcb_generic_event_t *event)
{
   /* FIXME: handle this event type */

   free(event);
} /* _ecore_x_event_handle_mapping_notify */

void
_ecore_x_event_handle_shape_change(xcb_generic_event_t *event)
{
#ifdef ECORE_X_SHAPE
   xcb_shape_notify_event_t *ev;
   Ecore_X_Event_Window_Shape *e;

   ev = (xcb_shape_notify_event_t *)event;
   e = calloc(1, sizeof(Ecore_X_Event_Window_Shape));
   if (!e)
      return;

   e->win = ev->affected_window;
   e->time = ev->server_time;
   ecore_event_add(ECORE_X_EVENT_WINDOW_SHAPE, e, NULL, NULL);
#endif /* ECORE_X_SHAPE */

   free(event);
} /* _ecore_x_event_handle_shape_change */

void
_ecore_x_event_handle_screensaver_notify(xcb_generic_event_t *event)
{
#ifdef ECORE_X_SCREENSAVER
   xcb_screensaver_notify_event_t *ev;
   Ecore_X_Event_Screensaver_Notify *e;

   ev = (xcb_screensaver_notify_event_t *)event;
   e = calloc(1, sizeof(Ecore_X_Event_Screensaver_Notify));
   if (!e)
      return;

   e->win = ev->window;
   if (ev->state == XCB_SCREENSAVER_STATE_ON)
      e->on = 1;
   else
      e->on = 0;

   e->time = ev->time;
   ecore_event_add(ECORE_X_EVENT_SCREENSAVER_NOTIFY, e, NULL, NULL);
#endif /* ECORE_X_SCREENSAVER */

   free(event);
} /* _ecore_x_event_handle_screensaver_notify */

void
_ecore_x_event_handle_sync_counter(xcb_generic_event_t *event)
{
#ifdef ECORE_X_SYNC
   xcb_sync_counter_notify_event_t *ev;
   Ecore_X_Event_Sync_Counter *e;

   ev = (xcb_sync_counter_notify_event_t *)event;
   e = calloc(1, sizeof(Ecore_X_Event_Sync_Counter));
   if (!e)
      return;

   e->time = ev->timestamp;
   ecore_event_add(ECORE_X_EVENT_SYNC_COUNTER, e, NULL, NULL);
#endif /* ECORE_X_SYNC */

   free(event);
} /* _ecore_x_event_handle_sync_counter */

void
_ecore_x_event_handle_sync_alarm(xcb_generic_event_t *event)
{
#ifdef ECORE_X_SYNC
   xcb_sync_alarm_notify_event_t *ev;
   Ecore_X_Event_Sync_Alarm *e;

   ev = (xcb_sync_alarm_notify_event_t *)event;
   e = calloc(1, sizeof(Ecore_X_Event_Sync_Alarm));
   if (!e)
      return;

   e->time = ev->timestamp;
   e->alarm = ev->alarm;
   ecore_event_add(ECORE_X_EVENT_SYNC_ALARM, e, NULL, NULL);
#endif /* ECORE_X_SYNC */

   free(event);
} /* _ecore_x_event_handle_sync_alarm */

/* FIXME: round trip */
void
_ecore_x_event_handle_randr_change(xcb_generic_event_t *event)
{
#ifdef ECORE_X_RANDR
   xcb_randr_screen_change_notify_event_t *ev;
   Ecore_X_Event_Screen_Change *e;

   ev = (xcb_randr_screen_change_notify_event_t *)event;

   if ((ev->response_type & ~0x80) != XCB_CONFIGURE_NOTIFY)
     {
        xcb_query_extension_reply_t *rep;

        rep = xcb_query_extension_reply(_ecore_xcb_conn,
                                        xcb_query_extension_unchecked(_ecore_xcb_conn,
                                                                      strlen("randr"),
                                                                      "randr"),
                                        NULL);

        if ((!rep) ||
            (((ev->response_type & ~0x80) - rep->first_event) != XCB_RANDR_SCREEN_CHANGE_NOTIFY))
           WRN("ERROR: Can't update RandR config!");

        if (rep)
           free(rep);
     }

   e = calloc(1, sizeof(Ecore_X_Event_Screen_Change));
   if (!e)
      return;

   e->win = ev->request_window;
   e->root = ev->root;
   e->width = ev->width;
   e->height = ev->height;
   ecore_event_add(ECORE_X_EVENT_SCREEN_CHANGE, e, NULL, NULL);
#endif /* ECORE_X_RANDR */

   free(event);
} /* _ecore_x_event_handle_randr_change */

void
_ecore_x_event_handle_fixes_selection_notify(xcb_generic_event_t *event)
{
#ifdef ECORE_X_FIXES
   /* Nothing here yet */
#endif /* ECORE_X_FIXES */

   free(event);
} /* _ecore_x_event_handle_fixes_selection_notify */

void
_ecore_x_event_handle_damage_notify(xcb_generic_event_t *event)
{
#ifdef ECORE_XCBDAMAGE
   xcb_damage_notify_event_t *ev;
   Ecore_X_Event_Damage *e;

   ev = (xcb_damage_notify_event_t *)event;
   e = calloc(1, sizeof(Ecore_X_Event_Damage));
   if (!e)
      return;

   e->level = ev->level;
   e->drawable = ev->drawable;
   e->damage = ev->damage;
   /* FIXME: XCB has no 'more' member in xcb_damage_notify_event_t */
/*    e->more = ev->more; */
   e->time = ev->timestamp;
   e->area.x = ev->area.x;
   e->area.y = ev->area.y;
   e->area.width = ev->area.width;
   e->area.height = ev->area.height;
   e->geometry.x = ev->geometry.x;
   e->geometry.y = ev->geometry.y;
   e->geometry.width = ev->geometry.width;
   e->geometry.height = ev->geometry.height;

   ecore_event_add(ECORE_X_EVENT_DAMAGE_NOTIFY, e, NULL, NULL);
#endif /* ECORE_XCBDAMAGE */

   free(event);
} /* _ecore_x_event_handle_damage_notify */

