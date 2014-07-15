#ifdef HAVE_CONFIG_H
# include <config.h>
#endif /* ifdef HAVE_CONFIG_H */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <langinfo.h>

#include "Ecore.h"
#include "ecore_private.h"
#include "ecore_x_private.h"
#include "Ecore_X.h"
#include "Ecore_X_Atoms.h"

/** OpenBSD does not define CODESET
 * FIXME ??
 */

#ifndef CODESET
#define CODESET "INVALID"
#endif /* ifndef CODESET */

typedef struct _Ecore_X_Mouse_Down_Info
{
   EINA_INLIST;
   int       dev;
   Window    last_win;
   Window    last_last_win;
   Window    last_event_win;
   Window    last_last_event_win;
   Time      last_time;
   Time      last_last_time;
   Eina_Bool did_double : 1;
   Eina_Bool did_triple : 1;
} Ecore_X_Mouse_Down_Info;

static int _ecore_x_last_event_mouse_move = 0;
static Ecore_Event *_ecore_x_last_event_mouse_move_event = NULL;
static Eina_Inlist *_ecore_x_mouse_down_info_list = NULL;

static void
_ecore_x_mouse_down_info_clear(void)
{
   Eina_Inlist *l = _ecore_x_mouse_down_info_list;
   Ecore_X_Mouse_Down_Info *info = NULL;
   while (l)
     {
        info = EINA_INLIST_CONTAINER_GET(l, Ecore_X_Mouse_Down_Info);
        l = eina_inlist_remove(l, l);
        free(info);
     }
   _ecore_x_mouse_down_info_list = NULL;
}

void
_ecore_x_events_init(void)
{
   //Actually, Nothing to do.
}

void
_ecore_x_events_shutdown(void)
{
   _ecore_x_mouse_down_info_clear();
}

static Ecore_X_Mouse_Down_Info *
_ecore_x_mouse_down_info_get(int dev)
{
   Eina_Inlist *l = _ecore_x_mouse_down_info_list;
   Ecore_X_Mouse_Down_Info *info = NULL;

   //Return the exist info
   EINA_INLIST_FOREACH(l, info)
     if (info->dev == dev) return info;

   //New Device. Add it.
   info = calloc(1, sizeof(Ecore_X_Mouse_Down_Info));
   if (!info) return NULL;

   info->dev = dev;
   l = eina_inlist_append(l, (Eina_Inlist *)info);
   _ecore_x_mouse_down_info_list = l;
   return info;
}

static void
_ecore_x_event_free_mouse_move(void *data EINA_UNUSED,
                               void *ev)
{
   Ecore_Event_Mouse_Move *e;

   e = ev;
   if (_ecore_x_last_event_mouse_move)
     {
        _ecore_x_last_event_mouse_move_event = NULL;
        _ecore_x_last_event_mouse_move = 0;
     }

   free(e);
}

EAPI void
ecore_x_event_mask_set(Ecore_X_Window w,
                       Ecore_X_Event_Mask mask)
{
   XWindowAttributes attr;
   XSetWindowAttributes s_attr;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   if (!w)
     w = DefaultRootWindow(_ecore_x_disp);

   memset(&attr, 0, sizeof(XWindowAttributes));
   XGetWindowAttributes(_ecore_x_disp, w, &attr);
   if (_ecore_xlib_sync) ecore_x_sync();
   s_attr.event_mask = mask | attr.your_event_mask;
   XChangeWindowAttributes(_ecore_x_disp, w, CWEventMask, &s_attr);
   if (_ecore_xlib_sync) ecore_x_sync();
}

EAPI void
ecore_x_event_mask_unset(Ecore_X_Window w,
                         Ecore_X_Event_Mask mask)
{
   XWindowAttributes attr;
   XSetWindowAttributes s_attr;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   if (!w)
     w = DefaultRootWindow(_ecore_x_disp);

   memset(&attr, 0, sizeof(XWindowAttributes));
   XGetWindowAttributes(_ecore_x_disp, w, &attr);
   if (_ecore_xlib_sync) ecore_x_sync();
   s_attr.event_mask = attr.your_event_mask & ~mask;
   XChangeWindowAttributes(_ecore_x_disp, w, CWEventMask, &s_attr);
   if (_ecore_xlib_sync) ecore_x_sync();
}

static void
_ecore_x_event_free_xdnd_enter(void *data EINA_UNUSED,
                               void *ev)
{
   Ecore_X_Event_Xdnd_Enter *e;
   int i;

   e = ev;
   for (i = 0; i < e->num_types; i++)
     XFree(e->types[i]);
   free(e->types);
   free(e);
}

static void
_ecore_x_event_free_selection_notify(void *data EINA_UNUSED,
                                     void *ev)
{
   Ecore_X_Event_Selection_Notify *e;
   Ecore_X_Selection_Data *sel;

   e = ev;
   sel = e->data;
   if (sel->free)
     sel->free(sel);

   free(e->target);
   free(e);
}

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

   if (state & ECORE_X_MODIFIER_ALTGR)
     modifiers |= ECORE_EVENT_MODIFIER_ALTGR;

   if (state & ECORE_X_LOCK_SCROLL)
     modifiers |= ECORE_EVENT_LOCK_SCROLL;

   if (state & ECORE_X_LOCK_NUM)
     modifiers |= ECORE_EVENT_LOCK_NUM;

   if (state & ECORE_X_LOCK_CAPS)
     modifiers |= ECORE_EVENT_LOCK_CAPS;

   if (state & ECORE_X_LOCK_SHIFT)
     modifiers |= ECORE_EVENT_LOCK_SHIFT;

   return modifiers;
}

void
_ecore_mouse_move(unsigned int timestamp,
                  unsigned int xmodifiers,
                  int x,
                  int y,
                  int x_root,
                  int y_root,
                  unsigned int event_window,
                  unsigned int window,
                  unsigned int root_win,
                  int same_screen,
                  int dev,
                  double radx,
                  double rady,
                  double pressure,
                  double angle,
                  double mx,
                  double my,
                  double mrx,
                  double mry)
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

   e->multi.device = dev;
   e->multi.radius = (radx + rady) / 2;
   e->multi.radius_x = radx;
   e->multi.radius_y = rady;
   e->multi.pressure = pressure;
   e->multi.angle = angle;
   e->multi.x = mx;
   e->multi.y = my;
   e->multi.root.x = mrx;
   e->multi.root.y = mry;

   event = ecore_event_add(ECORE_EVENT_MOUSE_MOVE,
                           e,
                           _ecore_x_event_free_mouse_move,
                           NULL);

   _ecore_x_event_last_time = timestamp;
   _ecore_x_event_last_win = window;
   _ecore_x_event_last_root_x = x_root;
   _ecore_x_event_last_root_y = y_root;

   _ecore_x_last_event_mouse_move_event = event;
}

static void
_ecore_key_press(int event,
                 XKeyEvent *xevent)
{
   Ecore_Event_Key *e;
   char *compose = NULL;
   char *tmp = NULL;
   char *keyname;
   char *key;
   char keyname_buffer[256];
   char compose_buffer[256];
   KeySym sym;
   XComposeStatus status;
   int val;

   _ecore_x_last_event_mouse_move = 0;
   keyname = XKeysymToString(_ecore_x_XKeycodeToKeysym(xevent->display,
                                                       xevent->keycode, 0));
   if (!keyname)
     {
        snprintf(keyname_buffer,
                 sizeof(keyname_buffer),
                 "Keycode-%i",
                 xevent->keycode);
        keyname = keyname_buffer;
     }

   sym = 0;
   key = NULL;
   compose = NULL;
   val = XLookupString(xevent,
                       compose_buffer,
                       sizeof(compose_buffer),
                       &sym,
                       &status);
   if (val > 0)
     {
        compose_buffer[val] = 0;
        compose = eina_str_convert(nl_langinfo(CODESET), "UTF-8",
                                   compose_buffer);
        if (!compose)
          ERR("Ecore_X cannot convert input key string '%s' to UTF-8. "
              "Is Eina built with iconv support?", compose_buffer);
        tmp = compose;
     }

   key = XKeysymToString(sym);
   if (!key)
     key = keyname;

   e =
     malloc(sizeof(Ecore_Event_Key) + strlen(key) + strlen(keyname) +
            (compose ? strlen(compose) : 0) + 3);
   if (!e)
     goto on_error;

   e->keyname = (char *)(e + 1);
   e->key = e->keyname + strlen(keyname) + 1;
   e->compose = (compose) ? e->key + strlen(key) + 1 : NULL;
   e->string = e->compose;

   strcpy((char *)e->keyname, keyname);
   strcpy((char *)e->key, key);
   if (compose)
     strcpy((char *)e->compose, compose);

   e->modifiers = _ecore_x_event_modifiers(xevent->state);

   e->timestamp = xevent->time;
   e->window = xevent->subwindow ? xevent->subwindow : xevent->window;
   e->event_window = xevent->window;
   e->same_screen = xevent->same_screen;
   e->root_window = xevent->root;
   e->keycode = xevent->keycode;

   ecore_event_add(event, e, NULL, NULL);

   _ecore_x_event_last_time = e->timestamp;

on_error:
   if (tmp)
     free(tmp);
}

Ecore_Event_Mouse_Button *
_ecore_mouse_button(int event,
                    unsigned int timestamp,
                    unsigned int xmodifiers,
                    unsigned int buttons,
                    int x,
                    int y,
                    int x_root,
                    int y_root,
                    unsigned int event_window,
                    unsigned int window,
                    unsigned int root_win,
                    int same_screen,
                    int dev,
                    double radx,
                    double rady,
                    double pressure,
                    double angle,
                    double mx,
                    double my,
                    double mrx,
                    double mry)
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

   Ecore_X_Mouse_Down_Info *down_info = _ecore_x_mouse_down_info_get(dev);

   if (down_info)
     {
        if ((event == ECORE_EVENT_MOUSE_BUTTON_DOWN) &&
            down_info->did_triple)
          {
             down_info->last_win = 0;
             down_info->last_last_win = 0;
             down_info->last_event_win = 0;
             down_info->last_last_event_win = 0;
             down_info->last_time = 0;
             down_info->last_last_time = 0;
          }
        if (event_window == window)
          {
             if (event == ECORE_EVENT_MOUSE_BUTTON_DOWN)
               {
                  //Check Double Clicked
                  if (((int)(timestamp - down_info->last_time) <=
                       (int)(1000 * _ecore_x_double_click_time)) &&
                      (window == down_info->last_win) &&
                      (event_window == down_info->last_event_win))
                    {
                       e->double_click = 1;
                       down_info->did_double = EINA_TRUE;
                    }
                  else
                    {
                       down_info->did_double = EINA_FALSE;
                       down_info->did_triple = EINA_FALSE;
                    }

                  //Check Triple Clicked
                  if (((int)(timestamp - down_info->last_last_time) <=
                       (int)(2 * 1000 * _ecore_x_double_click_time)) &&
                      (window == down_info->last_win) &&
                      (window == down_info->last_last_win) &&
                      (event_window == down_info->last_event_win) &&
                      (event_window == down_info->last_last_event_win)
                      )
                    {
                       e->triple_click = 1;
                       down_info->did_triple = EINA_TRUE;
                    }
                  else
                    {
                       down_info->did_triple = EINA_FALSE;
                    }
               }
             else
               {
                  if (down_info->did_double)
                    e->double_click = 1;
                  if (down_info->did_triple)
                    e->triple_click = 1;
               }
          }
     }

   /* NB: Block commented out as _ecore_x_mouse_up_count appears to have
    * no use. The variable is also commented out above. This code block is
    * the only place that this variable is used, and appears to serve no
    * purpose. - dh
      if (event == ECORE_EVENT_MOUSE_BUTTON_DOWN
       && !e->double_click
       && !e->triple_click)
      _ecore_x_mouse_up_count = 0;
    */

   e->multi.device = dev;
   e->multi.radius = (radx + rady) / 2;
   e->multi.radius_x = radx;
   e->multi.radius_y = rady;
   e->multi.pressure = pressure;
   e->multi.angle = angle;
   e->multi.x = mx;
   e->multi.y = my;
   e->multi.root.x = mrx;
   e->multi.root.y = mry;

   _ecore_x_event_last_time = e->timestamp;
   _ecore_x_event_last_win = e->window;
   _ecore_x_event_last_root_x = x_root;
   _ecore_x_event_last_root_y = y_root;

   ecore_event_add(event, e, NULL, NULL);

   if ((down_info) &&
       (event == ECORE_EVENT_MOUSE_BUTTON_DOWN) &&
       (window == event_window) &&
       (!down_info->did_triple))
     {
        down_info->last_last_win = down_info->last_win;
        down_info->last_win = window;
        down_info->last_last_event_win = down_info->last_event_win;
        down_info->last_event_win = event_window;
        down_info->last_last_time = down_info->last_time;
        down_info->last_time = timestamp;
     }

   return e;
}

void
_ecore_x_event_handle_any_event(XEvent *xevent)
{
   XEvent *ev = malloc(sizeof(XEvent));
   if (!ev) return;
   memcpy(ev, xevent, sizeof(XEvent));
   ecore_event_add(ECORE_X_EVENT_ANY, ev, NULL, NULL);
}

void
_ecore_x_event_handle_key_press(XEvent *xevent)
{
   _ecore_key_press(ECORE_EVENT_KEY_DOWN, (XKeyEvent *)xevent);
}

void
_ecore_x_event_handle_key_release(XEvent *xevent)
{
   _ecore_key_press(ECORE_EVENT_KEY_UP, (XKeyEvent *)xevent);
}

void
_ecore_x_event_handle_button_press(XEvent *xevent)
{
   int i;

   INF("ButtonEvent:press time=%u x=%d y=%d button=%d", (unsigned int)xevent->xbutton.time, (int)xevent->xbutton.x, (int)xevent->xbutton.y, xevent->xbutton.button);

   _ecore_x_last_event_mouse_move = 0;
   if ((xevent->xbutton.button > 3) && (xevent->xbutton.button < 8))
     {
        Ecore_Event_Mouse_Wheel *e;

        e = malloc(sizeof(Ecore_Event_Mouse_Wheel));
        if (!e)
          return;

        e->timestamp = xevent->xbutton.time;
        e->modifiers = _ecore_x_event_modifiers(xevent->xbutton.state);
        switch (xevent->xbutton.button)
          {
           case 4: e->direction = 0; e->z = -1; break;

           case 5: e->direction = 0; e->z = 1; break;

           case 6: e->direction = 1; e->z = -1; break;

           case 7: e->direction = 1; e->z = 1; break;

           default: e->direction = 0; e->z = 0; break;
          }

        e->x = xevent->xbutton.x;
        e->y = xevent->xbutton.y;
        e->root.x = xevent->xbutton.x_root;
        e->root.y = xevent->xbutton.y_root;

        if (xevent->xbutton.subwindow)
          e->window = xevent->xbutton.subwindow;
        else
          e->window = xevent->xbutton.window;

        e->event_window = xevent->xbutton.window;
        e->same_screen = xevent->xbutton.same_screen;
        e->root_window = xevent->xbutton.root;

        _ecore_x_event_last_time = e->timestamp;
        _ecore_x_event_last_win = e->window;
        _ecore_x_event_last_root_x = xevent->xbutton.x_root;
        _ecore_x_event_last_root_y = xevent->xbutton.y_root;
        ecore_event_add(ECORE_EVENT_MOUSE_WHEEL, e, NULL, NULL);

        for (i = 0; i < _ecore_window_grabs_num; i++)
          {
             if ((_ecore_window_grabs[i].win == xevent->xbutton.window) ||
                 (_ecore_window_grabs[i].win == xevent->xbutton.subwindow))
               {
                  Eina_Bool replay = EINA_FALSE;

                  if (_ecore_window_grab_replay_func)
                    replay = _ecore_window_grab_replay_func(
                        _ecore_window_grab_replay_data,
                        ECORE_EVENT_MOUSE_WHEEL,
                        e);

                  if (replay)
                    XAllowEvents(xevent->xbutton.display,
                                 ReplayPointer, xevent->xbutton.time);
                  else
                    XAllowEvents(xevent->xbutton.display,
                                 AsyncPointer, xevent->xbutton.time);

                  break;
               }
          }
     }
   else
     {
        {
           _ecore_mouse_move(xevent->xbutton.time, xevent->xbutton.state,
                             xevent->xbutton.x, xevent->xbutton.y,
                             xevent->xbutton.x_root, xevent->xbutton.y_root,
                             xevent->xbutton.window,
                             (xevent->xbutton.subwindow ? xevent->xbutton.
                              subwindow : xevent->xbutton.window),
                             xevent->xbutton.root,
                             xevent->xbutton.same_screen,
                             0, 1, 1,
                             1.0, // pressure
                             0.0, // angle
                             xevent->xbutton.x, xevent->xbutton.y,
                             xevent->xbutton.x_root, xevent->xbutton.y_root);
        }
        {
           Ecore_Event_Mouse_Button *e;
           int event_window;
           int window;

           window =
             (xevent->xbutton.subwindow ? xevent->xbutton.subwindow : xevent->
              xbutton.window);
           event_window = xevent->xbutton.window;

           e = _ecore_mouse_button(ECORE_EVENT_MOUSE_BUTTON_DOWN,
                                   xevent->xbutton.time,
                                   xevent->xbutton.state,
                                   xevent->xbutton.button,
                                   xevent->xbutton.x,
                                   xevent->xbutton.y,
                                   xevent->xbutton.x_root,
                                   xevent->xbutton.y_root,
                                   event_window,
                                   window,
                                   xevent->xbutton.root,
                                   xevent->xbutton.same_screen,
                                   0,
                                   1,
                                   1,
                                   1.0,
// pressure
                                   0.0,
// angle
                                   xevent->xbutton.x,
                                   xevent->xbutton.y,
                                   xevent->xbutton.x_root,
                                   xevent->xbutton.y_root);
           if (e)
             for (i = 0; i < _ecore_window_grabs_num; i++)
               {
                  if ((_ecore_window_grabs[i].win == xevent->xbutton.window) ||
                      (_ecore_window_grabs[i].win == xevent->xbutton.subwindow))
                    {
                       Eina_Bool replay = EINA_FALSE;

                       if (_ecore_window_grab_replay_func)
                         replay = _ecore_window_grab_replay_func(
                             _ecore_window_grab_replay_data,
                             ECORE_EVENT_MOUSE_BUTTON_DOWN,
                             e);

                       if (replay)
                         XAllowEvents(xevent->xbutton.display,
                                      ReplayPointer, xevent->xbutton.time);
                       else
                         XAllowEvents(xevent->xbutton.display,
                                      AsyncPointer, xevent->xbutton.time);

                       break;
                    }
               }
        }
     }
}

void
_ecore_x_event_handle_button_release(XEvent *xevent)
{
   _ecore_x_last_event_mouse_move = 0;
   INF("ButtonEvent:release time=%u x=%d y=%d button=%d", (unsigned int)xevent->xbutton.time, (int)xevent->xbutton.x, (int)xevent->xbutton.y, xevent->xbutton.button);
   /* filter out wheel buttons */
   if ((xevent->xbutton.button <= 3) || (xevent->xbutton.button > 7))
     {
        _ecore_mouse_move(xevent->xbutton.time, xevent->xbutton.state,
                          xevent->xbutton.x, xevent->xbutton.y,
                          xevent->xbutton.x_root, xevent->xbutton.y_root,
                          xevent->xbutton.window,
                          (xevent->xbutton.subwindow ? xevent->xbutton.
                           subwindow : xevent->xbutton.window),
                          xevent->xbutton.root,
                          xevent->xbutton.same_screen,
                          0, 1, 1,
                          1.0, // pressure
                          0.0, // angle
                          xevent->xbutton.x, xevent->xbutton.y,
                          xevent->xbutton.x_root, xevent->xbutton.y_root);

        _ecore_mouse_button(ECORE_EVENT_MOUSE_BUTTON_UP,
                            xevent->xbutton.time, xevent->xbutton.state,
                            xevent->xbutton.button,
                            xevent->xbutton.x, xevent->xbutton.y,
                            xevent->xbutton.x_root, xevent->xbutton.y_root,
                            xevent->xbutton.window,
                            (xevent->xbutton.subwindow ? xevent->xbutton.
                             subwindow : xevent->xbutton.window),
                            xevent->xbutton.root,
                            xevent->xbutton.same_screen,
                            0, 1, 1,
                            1.0, // pressure
                            0.0, // angle
                            xevent->xbutton.x, xevent->xbutton.y,
                            xevent->xbutton.x_root, xevent->xbutton.y_root);
     }
}

void
_ecore_x_event_handle_motion_notify(XEvent *xevent)
{
/*
   if (_ecore_x_last_event_mouse_move)
     {
        ecore_event_del(_ecore_x_last_event_mouse_move_event);
        _ecore_x_last_event_mouse_move = 0;
        _ecore_x_last_event_mouse_move_event = NULL;
     }
 */
   _ecore_mouse_move(xevent->xmotion.time, xevent->xmotion.state,
                     xevent->xmotion.x, xevent->xmotion.y,
                     xevent->xmotion.x_root, xevent->xmotion.y_root,
                     xevent->xmotion.window,
                     (xevent->xmotion.subwindow ? xevent->xmotion.subwindow :
                      xevent->xmotion.window),
                     xevent->xmotion.root,
                     xevent->xmotion.same_screen,
                     0, 1, 1,
                     1.0,   // pressure
                     0.0,   // angle
                     xevent->xmotion.x, xevent->xmotion.y,
                     xevent->xmotion.x_root, xevent->xmotion.y_root);

   _ecore_x_last_event_mouse_move = 1;

   /* Xdnd handling */
   _ecore_x_dnd_drag(xevent->xmotion.root,
                     xevent->xmotion.x_root,
                     xevent->xmotion.y_root);
}

void
_ecore_x_event_handle_enter_notify(XEvent *xevent)
{
   _ecore_x_last_event_mouse_move = 0;
   {
      _ecore_mouse_move(xevent->xcrossing.time, xevent->xcrossing.state,
                        xevent->xcrossing.x, xevent->xcrossing.y,
                        xevent->xcrossing.x_root, xevent->xcrossing.y_root,
                        xevent->xcrossing.window,
                        (xevent->xcrossing.subwindow ? xevent->xcrossing.
                         subwindow : xevent->xcrossing.window),
                        xevent->xcrossing.root,
                        xevent->xcrossing.same_screen,
                        0, 1, 1,
                        1.0, // pressure
                        0.0, // angle
                        xevent->xcrossing.x, xevent->xcrossing.y,
                        xevent->xcrossing.x_root, xevent->xcrossing.y_root);
   }
   {
      Ecore_X_Event_Mouse_In *e;

      e = calloc(1, sizeof(Ecore_X_Event_Mouse_In));
      if (!e)
        return;

      e->modifiers = _ecore_x_event_modifiers(xevent->xcrossing.state);
      e->x = xevent->xcrossing.x;
      e->y = xevent->xcrossing.y;
      e->root.x = xevent->xcrossing.x_root;
      e->root.y = xevent->xcrossing.y_root;
      if (xevent->xcrossing.subwindow)
        e->win = xevent->xcrossing.subwindow;
      else
        e->win = xevent->xcrossing.window;

      e->same_screen = xevent->xcrossing.same_screen;
      e->root_win = xevent->xcrossing.root;
      e->event_win = xevent->xcrossing.window;

      if (xevent->xcrossing.mode == NotifyNormal)
        e->mode = ECORE_X_EVENT_MODE_NORMAL;
      else if (xevent->xcrossing.mode == NotifyGrab)
        e->mode = ECORE_X_EVENT_MODE_GRAB;
      else if (xevent->xcrossing.mode == NotifyUngrab)
        e->mode = ECORE_X_EVENT_MODE_UNGRAB;

      if (xevent->xcrossing.detail == NotifyAncestor)
        e->detail = ECORE_X_EVENT_DETAIL_ANCESTOR;
      else if (xevent->xcrossing.detail == NotifyVirtual)
        e->detail = ECORE_X_EVENT_DETAIL_VIRTUAL;
      else if (xevent->xcrossing.detail == NotifyInferior)
        e->detail = ECORE_X_EVENT_DETAIL_INFERIOR;
      else if (xevent->xcrossing.detail == NotifyNonlinear)
        e->detail = ECORE_X_EVENT_DETAIL_NON_LINEAR;
      else if (xevent->xcrossing.detail == NotifyNonlinearVirtual)
        e->detail = ECORE_X_EVENT_DETAIL_NON_LINEAR_VIRTUAL;

      e->time = xevent->xcrossing.time;
      _ecore_x_event_last_time = e->time;
      ecore_event_add(ECORE_X_EVENT_MOUSE_IN, e, NULL, NULL);
   }
}

void
_ecore_x_event_handle_leave_notify(XEvent *xevent)
{
   _ecore_x_last_event_mouse_move = 0;
   {
      _ecore_mouse_move(xevent->xcrossing.time, xevent->xcrossing.state,
                        xevent->xcrossing.x, xevent->xcrossing.y,
                        xevent->xcrossing.x_root, xevent->xcrossing.y_root,
                        xevent->xcrossing.window,
                        (xevent->xcrossing.subwindow ? xevent->xcrossing.
                         subwindow : xevent->xcrossing.window),
                        xevent->xcrossing.root,
                        xevent->xcrossing.same_screen,
                        0, 1, 1,
                        1.0, // pressure
                        0.0, // angle
                        xevent->xcrossing.x, xevent->xcrossing.y,
                        xevent->xcrossing.x_root, xevent->xcrossing.y_root);
   }
   {
      Ecore_X_Event_Mouse_Out *e;

      e = calloc(1, sizeof(Ecore_X_Event_Mouse_Out));
      if (!e)
        return;

      e->modifiers = _ecore_x_event_modifiers(xevent->xcrossing.state);
      e->x = xevent->xcrossing.x;
      e->y = xevent->xcrossing.y;
      e->root.x = xevent->xcrossing.x_root;
      e->root.y = xevent->xcrossing.y_root;
      if (xevent->xcrossing.subwindow)
        e->win = xevent->xcrossing.subwindow;
      else
        e->win = xevent->xcrossing.window;

      e->same_screen = xevent->xcrossing.same_screen;
      e->root_win = xevent->xcrossing.root;
      e->event_win = xevent->xcrossing.window;

      if (xevent->xcrossing.mode == NotifyNormal)
        e->mode = ECORE_X_EVENT_MODE_NORMAL;
      else if (xevent->xcrossing.mode == NotifyGrab)
        e->mode = ECORE_X_EVENT_MODE_GRAB;
      else if (xevent->xcrossing.mode == NotifyUngrab)
        e->mode = ECORE_X_EVENT_MODE_UNGRAB;

      if (xevent->xcrossing.detail == NotifyAncestor)
        e->detail = ECORE_X_EVENT_DETAIL_ANCESTOR;
      else if (xevent->xcrossing.detail == NotifyVirtual)
        e->detail = ECORE_X_EVENT_DETAIL_VIRTUAL;
      else if (xevent->xcrossing.detail == NotifyInferior)
        e->detail = ECORE_X_EVENT_DETAIL_INFERIOR;
      else if (xevent->xcrossing.detail == NotifyNonlinear)
        e->detail = ECORE_X_EVENT_DETAIL_NON_LINEAR;
      else if (xevent->xcrossing.detail == NotifyNonlinearVirtual)
        e->detail = ECORE_X_EVENT_DETAIL_NON_LINEAR_VIRTUAL;

      e->time = xevent->xcrossing.time;
      _ecore_x_event_last_time = e->time;
      _ecore_x_event_last_win = e->win;
      _ecore_x_event_last_root_x = e->root.x;
      _ecore_x_event_last_root_y = e->root.y;
      ecore_event_add(ECORE_X_EVENT_MOUSE_OUT, e, NULL, NULL);
   }
}

void
_ecore_x_event_handle_focus_in(XEvent *xevent)
{
   Ecore_X_Event_Window_Focus_In *e;

   _ecore_x_last_event_mouse_move = 0;

   e = calloc(1, sizeof(Ecore_X_Event_Window_Focus_In));
   if (!e)
     return;

   e->win = xevent->xfocus.window;

   if (xevent->xfocus.mode == NotifyNormal)
     e->mode = ECORE_X_EVENT_MODE_NORMAL;
   else if (xevent->xfocus.mode == NotifyWhileGrabbed)
     e->mode = ECORE_X_EVENT_MODE_WHILE_GRABBED;
   else if (xevent->xfocus.mode == NotifyGrab)
     e->mode = ECORE_X_EVENT_MODE_GRAB;
   else if (xevent->xfocus.mode == NotifyUngrab)
     e->mode = ECORE_X_EVENT_MODE_UNGRAB;

   if (xevent->xfocus.detail == NotifyAncestor)
     e->detail = ECORE_X_EVENT_DETAIL_ANCESTOR;
   else if (xevent->xfocus.detail == NotifyVirtual)
     e->detail = ECORE_X_EVENT_DETAIL_VIRTUAL;
   else if (xevent->xfocus.detail == NotifyInferior)
     e->detail = ECORE_X_EVENT_DETAIL_INFERIOR;
   else if (xevent->xfocus.detail == NotifyNonlinear)
     e->detail = ECORE_X_EVENT_DETAIL_NON_LINEAR;
   else if (xevent->xfocus.detail == NotifyNonlinearVirtual)
     e->detail = ECORE_X_EVENT_DETAIL_NON_LINEAR_VIRTUAL;
   else if (xevent->xfocus.detail == NotifyPointer)
     e->detail = ECORE_X_EVENT_DETAIL_POINTER;
   else if (xevent->xfocus.detail == NotifyPointerRoot)
     e->detail = ECORE_X_EVENT_DETAIL_POINTER_ROOT;
   else if (xevent->xfocus.detail == NotifyDetailNone)
     e->detail = ECORE_X_EVENT_DETAIL_DETAIL_NONE;

   e->time = _ecore_x_event_last_time;
   _ecore_x_event_last_time = e->time;
   ecore_event_add(ECORE_X_EVENT_WINDOW_FOCUS_IN, e, NULL, NULL);
}

void
_ecore_x_event_handle_focus_out(XEvent *xevent)
{
   Ecore_X_Event_Window_Focus_Out *e;

   _ecore_x_last_event_mouse_move = 0;

   e = calloc(1, sizeof(Ecore_X_Event_Window_Focus_Out));
   if (!e)
     return;

   e->win = xevent->xfocus.window;

   if (xevent->xfocus.mode == NotifyNormal)
     e->mode = ECORE_X_EVENT_MODE_NORMAL;
   else if (xevent->xfocus.mode == NotifyWhileGrabbed)
     e->mode = ECORE_X_EVENT_MODE_WHILE_GRABBED;
   else if (xevent->xfocus.mode == NotifyGrab)
     e->mode = ECORE_X_EVENT_MODE_GRAB;
   else if (xevent->xfocus.mode == NotifyUngrab)
     e->mode = ECORE_X_EVENT_MODE_UNGRAB;

   if (xevent->xfocus.detail == NotifyAncestor)
     e->detail = ECORE_X_EVENT_DETAIL_ANCESTOR;
   else if (xevent->xfocus.detail == NotifyVirtual)
     e->detail = ECORE_X_EVENT_DETAIL_VIRTUAL;
   else if (xevent->xfocus.detail == NotifyInferior)
     e->detail = ECORE_X_EVENT_DETAIL_INFERIOR;
   else if (xevent->xfocus.detail == NotifyNonlinear)
     e->detail = ECORE_X_EVENT_DETAIL_NON_LINEAR;
   else if (xevent->xfocus.detail == NotifyNonlinearVirtual)
     e->detail = ECORE_X_EVENT_DETAIL_NON_LINEAR_VIRTUAL;
   else if (xevent->xfocus.detail == NotifyPointer)
     e->detail = ECORE_X_EVENT_DETAIL_POINTER;
   else if (xevent->xfocus.detail == NotifyPointerRoot)
     e->detail = ECORE_X_EVENT_DETAIL_POINTER_ROOT;
   else if (xevent->xfocus.detail == NotifyDetailNone)
     e->detail = ECORE_X_EVENT_DETAIL_DETAIL_NONE;

   e->time = _ecore_x_event_last_time;
   _ecore_x_event_last_time = e->time;
   ecore_event_add(ECORE_X_EVENT_WINDOW_FOCUS_OUT, e, NULL, NULL);
}

void
_ecore_x_event_handle_keymap_notify(XEvent *xevent EINA_UNUSED)
{
   _ecore_x_last_event_mouse_move = 0;
   /* FIXME: handle this event type */
}

void
_ecore_x_event_handle_expose(XEvent *xevent)
{
   Ecore_X_Event_Window_Damage *e;

   _ecore_x_last_event_mouse_move = 0;
   e = calloc(1, sizeof(Ecore_X_Event_Window_Damage));
   if (!e)
     return;

   e->win = xevent->xexpose.window;
   e->time = _ecore_x_event_last_time;
   e->x = xevent->xexpose.x;
   e->y = xevent->xexpose.y;
   e->w = xevent->xexpose.width;
   e->h = xevent->xexpose.height;
   e->count = xevent->xexpose.count;
   ecore_event_add(ECORE_X_EVENT_WINDOW_DAMAGE, e, NULL, NULL);
}

void
_ecore_x_event_handle_graphics_expose(XEvent *xevent)
{
   Ecore_X_Event_Window_Damage *e;

   _ecore_x_last_event_mouse_move = 0;
   e = calloc(1, sizeof(Ecore_X_Event_Window_Damage));
   if (!e)
     return;

   e->win = xevent->xgraphicsexpose.drawable;
   e->time = _ecore_x_event_last_time;
   e->x = xevent->xgraphicsexpose.x;
   e->y = xevent->xgraphicsexpose.y;
   e->w = xevent->xgraphicsexpose.width;
   e->h = xevent->xgraphicsexpose.height;
   e->count = xevent->xgraphicsexpose.count;
   ecore_event_add(ECORE_X_EVENT_WINDOW_DAMAGE, e, NULL, NULL);
}

void
_ecore_x_event_handle_visibility_notify(XEvent *xevent)
{
   _ecore_x_last_event_mouse_move = 0;
//   if (xevent->xvisibility.state != VisibilityPartiallyObscured)
   {
      Ecore_X_Event_Window_Visibility_Change *e;

      e = calloc(1, sizeof(Ecore_X_Event_Window_Visibility_Change));
      if (!e)
        return;

      e->win = xevent->xvisibility.window;
      e->time = _ecore_x_event_last_time;
      if (xevent->xvisibility.state == VisibilityFullyObscured)
        e->fully_obscured = 1;
      else
        e->fully_obscured = 0;

      ecore_event_add(ECORE_X_EVENT_WINDOW_VISIBILITY_CHANGE, e, NULL, NULL);
   }
}

void
_ecore_x_event_handle_create_notify(XEvent *xevent)
{
   Ecore_X_Event_Window_Create *e;

   _ecore_x_last_event_mouse_move = 0;
   e = calloc(1, sizeof(Ecore_X_Event_Window_Create));
   if (!e)
     return;

   e->win = xevent->xcreatewindow.window;
   e->parent = xevent->xcreatewindow.parent;
   if (xevent->xcreatewindow.override_redirect)
     e->override = 1;
   else
     e->override = 0;

   e->x = xevent->xcreatewindow.x;
   e->y = xevent->xcreatewindow.y;
   e->w = xevent->xcreatewindow.width;
   e->h = xevent->xcreatewindow.height;
   e->border = xevent->xcreatewindow.border_width;
   e->time = _ecore_x_event_last_time;
   ecore_event_add(ECORE_X_EVENT_WINDOW_CREATE, e, NULL, NULL);
}

void
_ecore_x_event_handle_destroy_notify(XEvent *xevent)
{
   Ecore_X_Event_Window_Destroy *e;

   _ecore_x_last_event_mouse_move = 0;
   e = calloc(1, sizeof(Ecore_X_Event_Window_Destroy));
   if (!e)
     return;

   e->win = xevent->xdestroywindow.window;
   e->event_win = xevent->xdestroywindow.event;
   e->time = _ecore_x_event_last_time;
   if (e->win == _ecore_x_event_last_win)
     _ecore_x_event_last_win = 0;

   ecore_event_add(ECORE_X_EVENT_WINDOW_DESTROY, e, NULL, NULL);
   while (_ecore_x_window_grab_remove(e->win, -1, 0, 0));
   while (_ecore_x_key_grab_remove(e->win, NULL, 0, 0));
}

void
_ecore_x_event_handle_unmap_notify(XEvent *xevent)
{
   Ecore_X_Event_Window_Hide *e;

   _ecore_x_last_event_mouse_move = 0;
   e = calloc(1, sizeof(Ecore_X_Event_Window_Hide));
   if (!e)
     return;

   e->win = xevent->xunmap.window;
   e->event_win = xevent->xunmap.event;
   e->time = _ecore_x_event_last_time;
   e->send_event = xevent->xunmap.send_event;
   ecore_event_add(ECORE_X_EVENT_WINDOW_HIDE, e, NULL, NULL);
}

void
_ecore_x_event_handle_map_notify(XEvent *xevent)
{
   Ecore_X_Event_Window_Show *e;

   _ecore_x_last_event_mouse_move = 0;
   e = calloc(1, sizeof(Ecore_X_Event_Window_Show));
   if (!e)
     return;

   e->win = xevent->xmap.window;
   e->event_win = xevent->xmap.event;
   e->time = _ecore_x_event_last_time;
   ecore_event_add(ECORE_X_EVENT_WINDOW_SHOW, e, NULL, NULL);
}

void
_ecore_x_event_handle_map_request(XEvent *xevent)
{
   Ecore_X_Event_Window_Show_Request *e;

   _ecore_x_last_event_mouse_move = 0;
   e = calloc(1, sizeof(Ecore_X_Event_Window_Show_Request));
   if (!e)
     return;

   e->win = xevent->xmaprequest.window;
   e->time = _ecore_x_event_last_time;
   e->parent = xevent->xmaprequest.parent;
   ecore_event_add(ECORE_X_EVENT_WINDOW_SHOW_REQUEST, e, NULL, NULL);
}

void
_ecore_x_event_handle_reparent_notify(XEvent *xevent)
{
   Ecore_X_Event_Window_Reparent *e;

   _ecore_x_last_event_mouse_move = 0;
   e = calloc(1, sizeof(Ecore_X_Event_Window_Reparent));
   if (!e)
     return;

   e->win = xevent->xreparent.window;
   e->event_win = xevent->xreparent.event;
   e->parent = xevent->xreparent.parent;
   e->time = _ecore_x_event_last_time;
   ecore_event_add(ECORE_X_EVENT_WINDOW_REPARENT, e, NULL, NULL);
}

void
_ecore_x_event_handle_configure_notify(XEvent *xevent)
{
   Ecore_X_Event_Window_Configure *e;

   _ecore_x_last_event_mouse_move = 0;
   e = calloc(1, sizeof(Ecore_X_Event_Window_Configure));
   if (!e)
     return;

   e->win = xevent->xconfigure.window;
   e->event_win = xevent->xconfigure.event;
   e->abovewin = xevent->xconfigure.above;
   e->x = xevent->xconfigure.x;
   e->y = xevent->xconfigure.y;
   e->w = xevent->xconfigure.width;
   e->h = xevent->xconfigure.height;
   e->border = xevent->xconfigure.border_width;
   e->override = xevent->xconfigure.override_redirect;
   e->from_wm = xevent->xconfigure.send_event;
   e->time = _ecore_x_event_last_time;
   ecore_event_add(ECORE_X_EVENT_WINDOW_CONFIGURE, e, NULL, NULL);
}

void
_ecore_x_event_handle_configure_request(XEvent *xevent)
{
   Ecore_X_Event_Window_Configure_Request *e;

   _ecore_x_last_event_mouse_move = 0;
   e = calloc(1, sizeof(Ecore_X_Event_Window_Configure_Request));
   if (!e)
     return;

   e->win = xevent->xconfigurerequest.window;
   e->parent_win = xevent->xconfigurerequest.parent;
   e->abovewin = xevent->xconfigurerequest.above;
   e->x = xevent->xconfigurerequest.x;
   e->y = xevent->xconfigurerequest.y;
   e->w = xevent->xconfigurerequest.width;
   e->h = xevent->xconfigurerequest.height;
   e->border = xevent->xconfigurerequest.border_width;
   e->value_mask = xevent->xconfigurerequest.value_mask;
   e->time = _ecore_x_event_last_time;

   if (xevent->xconfigurerequest.detail == Above)
     e->detail = ECORE_X_WINDOW_STACK_ABOVE;
   else if (xevent->xconfigurerequest.detail == Below)
     e->detail = ECORE_X_WINDOW_STACK_BELOW;
   else if (xevent->xconfigurerequest.detail == TopIf)
     e->detail = ECORE_X_WINDOW_STACK_TOP_IF;
   else if (xevent->xconfigurerequest.detail == BottomIf)
     e->detail = ECORE_X_WINDOW_STACK_BOTTOM_IF;
   else if (xevent->xconfigurerequest.detail == Opposite)
     e->detail = ECORE_X_WINDOW_STACK_OPPOSITE;

   ecore_event_add(ECORE_X_EVENT_WINDOW_CONFIGURE_REQUEST, e, NULL, NULL);
}

void
_ecore_x_event_handle_gravity_notify(XEvent *xevent EINA_UNUSED)
{
   _ecore_x_last_event_mouse_move = 0;
   /* FIXME: handle this event type */
}

void
_ecore_x_event_handle_resize_request(XEvent *xevent)
{
   Ecore_X_Event_Window_Resize_Request *e;

   _ecore_x_last_event_mouse_move = 0;
   e = calloc(1, sizeof(Ecore_X_Event_Window_Resize_Request));
   if (!e)
     return;

   e->win = xevent->xresizerequest.window;
   e->w = xevent->xresizerequest.width;
   e->h = xevent->xresizerequest.height;
   e->time = _ecore_x_event_last_time;
   ecore_event_add(ECORE_X_EVENT_WINDOW_RESIZE_REQUEST, e, NULL, NULL);
}

void
_ecore_x_event_handle_circulate_notify(XEvent *xevent)
{
   Ecore_X_Event_Window_Stack *e;

   _ecore_x_last_event_mouse_move = 0;
   e = calloc(1, sizeof(Ecore_X_Event_Window_Stack));
   if (!e)
     return;

   e->win = xevent->xcirculate.window;
   e->event_win = xevent->xcirculate.event;
   if (xevent->xcirculate.place == PlaceOnTop)
     e->detail = ECORE_X_WINDOW_STACK_ABOVE;
   else
     e->detail = ECORE_X_WINDOW_STACK_BELOW;

   e->time = _ecore_x_event_last_time;
   ecore_event_add(ECORE_X_EVENT_WINDOW_STACK, e, NULL, NULL);
}

void
_ecore_x_event_handle_circulate_request(XEvent *xevent)
{
   Ecore_X_Event_Window_Stack_Request *e;

   _ecore_x_last_event_mouse_move = 0;
   e = calloc(1, sizeof(Ecore_X_Event_Window_Stack_Request));
   if (!e)
     return;

   e->win = xevent->xcirculaterequest.window;
   e->parent = xevent->xcirculaterequest.parent;
   if (xevent->xcirculaterequest.place == PlaceOnTop)
     e->detail = ECORE_X_WINDOW_STACK_ABOVE;
   else
     e->detail = ECORE_X_WINDOW_STACK_BELOW;

   e->time = _ecore_x_event_last_time;
   ecore_event_add(ECORE_X_EVENT_WINDOW_STACK_REQUEST, e, NULL, NULL);
}

void
_ecore_x_event_handle_property_notify(XEvent *xevent)
{
   _ecore_x_last_event_mouse_move = 0;
   {
      Ecore_X_Event_Window_Property *e;

      e = calloc(1, sizeof(Ecore_X_Event_Window_Property));
      if (!e)
        return;

      e->win = xevent->xproperty.window;
      e->atom = xevent->xproperty.atom;
      e->time = xevent->xproperty.time;
      _ecore_x_event_last_time = e->time;
      ecore_event_add(ECORE_X_EVENT_WINDOW_PROPERTY, e, NULL, NULL);
   }
}

void
_ecore_x_event_handle_selection_clear(XEvent *xevent)
{
//   Ecore_X_Selection_Intern *d;
   Ecore_X_Event_Selection_Clear *e;
   Ecore_X_Atom sel;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   _ecore_x_last_event_mouse_move = 0;
/* errr..... why? paranoia.
   d = _ecore_x_selection_get(xevent->xselectionclear.selection);
   if (d && (xevent->xselectionclear.time > d->time))
     {
        _ecore_x_selection_set(None, NULL, 0,
                               xevent->xselectionclear.selection);
     }
 */
/* Generate event for app cleanup */
   e = malloc(sizeof(Ecore_X_Event_Selection_Clear));
   e->win = xevent->xselectionclear.window;
   e->time = xevent->xselectionclear.time;
   e->atom = sel = xevent->xselectionclear.selection;
   if (sel == ECORE_X_ATOM_SELECTION_PRIMARY)
     e->selection = ECORE_X_SELECTION_PRIMARY;
   else if (sel == ECORE_X_ATOM_SELECTION_SECONDARY)
     e->selection = ECORE_X_SELECTION_SECONDARY;
   else if (sel == ECORE_X_ATOM_SELECTION_XDND)
     e->selection = ECORE_X_SELECTION_XDND;
   else if (sel == ECORE_X_ATOM_SELECTION_CLIPBOARD)
     e->selection = ECORE_X_SELECTION_CLIPBOARD;
   else
     e->selection = ECORE_X_SELECTION_OTHER;

   ecore_event_add(ECORE_X_EVENT_SELECTION_CLEAR, e, NULL, NULL);
}

void
_ecore_x_event_handle_selection_request(XEvent *xevent)
{
   Ecore_X_Event_Selection_Request *e;
   Ecore_X_Selection_Intern *sd;
   void *data = NULL;
   int len;
   int typesize;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   _ecore_x_last_event_mouse_move = 0;
   /*
    * Generate a selection request event.
    */
   e = malloc(sizeof(Ecore_X_Event_Selection_Request));
   e->owner = xevent->xselectionrequest.owner;
   e->requestor = xevent->xselectionrequest.requestor;
   e->time = xevent->xselectionrequest.time;
   e->selection = xevent->xselectionrequest.selection;
   e->target = xevent->xselectionrequest.target;
   e->property = xevent->xselectionrequest.property;
   ecore_event_add(ECORE_X_EVENT_SELECTION_REQUEST, e, NULL, NULL);

   if ((sd = _ecore_x_selection_get(xevent->xselectionrequest.selection)) &&
       (sd->win == xevent->xselectionrequest.owner))
     {
        Ecore_X_Selection_Intern *si;

        si = _ecore_x_selection_get(xevent->xselectionrequest.selection);
        if (si->data)
          {
             Ecore_X_Atom property = None;
             Ecore_X_Atom type;

             /* Set up defaults for strings first */
             type = xevent->xselectionrequest.target;
             typesize = 8;
             len = sd->length;

             if (!ecore_x_selection_convert(xevent->xselectionrequest.selection,
                                            xevent->xselectionrequest.target,
                                            &data, &len, &type, &typesize))
               /* Refuse selection, conversion to requested target failed */
               property = None;
             else if (data)
               {
                  /* FIXME: This does not properly handle large data transfers */
                  ecore_x_window_prop_property_set(
                    xevent->xselectionrequest.requestor,
                    xevent->xselectionrequest.
                    property,
                    type,
                    typesize,
                    data,
                    len);
                  property = xevent->xselectionrequest.property;
                  free(data);
               }

             ecore_x_selection_notify_send(xevent->xselectionrequest.requestor,
                                           xevent->xselectionrequest.selection,
                                           xevent->xselectionrequest.target,
                                           property,
                                           xevent->xselectionrequest.time);
          }
     }
}

void
_ecore_x_event_handle_selection_notify(XEvent *xevent)
{
   Ecore_X_Event_Selection_Notify *e;
   unsigned char *data = NULL;
   Ecore_X_Atom selection;
   int num_ret, format;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   _ecore_x_last_event_mouse_move = 0;
   selection = xevent->xselection.selection;

   if (xevent->xselection.target == ECORE_X_ATOM_SELECTION_TARGETS)
     {
        format = ecore_x_window_prop_property_get(xevent->xselection.requestor,
                                                  xevent->xselection.property,
                                                  XA_ATOM, 32, &data, &num_ret);
        if (!format)
          {
             /* fallback if targets handling is not working and try get the
              * selection directly */
             XConvertSelection(_ecore_x_disp, selection,
                               ECORE_X_ATOM_UTF8_STRING,
                               selection,
                               xevent->xselection.requestor,
                               CurrentTime);
             if (data) free(data);
             return;
          }
     }
   else
     {
        format = ecore_x_window_prop_property_get(xevent->xselection.requestor,
                                                  xevent->xselection.property,
                                                  AnyPropertyType, 8, &data,
                                                  &num_ret);
        if (!format) return;
     }

   e = calloc(1, sizeof(Ecore_X_Event_Selection_Notify));
   if (!e)
     {
        if (data) free(data);
        return;
     }

   e->win = xevent->xselection.requestor;
   e->time = xevent->xselection.time;
   e->atom = selection;
   e->target = _ecore_x_selection_target_get(xevent->xselection.target);

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

   ecore_event_add(ECORE_X_EVENT_SELECTION_NOTIFY, e,
                   _ecore_x_event_free_selection_notify, NULL);
}

void
_ecore_x_event_handle_colormap_notify(XEvent *xevent)
{
   Ecore_X_Event_Window_Colormap *e;

   _ecore_x_last_event_mouse_move = 0;
   e = calloc(1, sizeof(Ecore_X_Event_Window_Colormap));
   if (!e)
     return;

   e->win = xevent->xcolormap.window;
   e->cmap = xevent->xcolormap.colormap;
   e->time = _ecore_x_event_last_time;
   if (xevent->xcolormap.state == ColormapInstalled)
     e->installed = EINA_TRUE;
   else
     e->installed = EINA_FALSE;

   ecore_event_add(ECORE_X_EVENT_WINDOW_COLORMAP, e, NULL, NULL);
}

void
_ecore_x_event_handle_client_message(XEvent *xevent)
{
   _ecore_x_last_event_mouse_move = 0;
   /* Special client message event handling here. need to put LOTS of if */
   /* checks here and generate synthetic events per special message known */
   /* otherwise generate generic client message event. this would handle*/
   /* netwm, ICCCM, gnomewm, old kde and mwm hint client message protocols */
   if ((xevent->xclient.message_type == ECORE_X_ATOM_WM_PROTOCOLS) &&
       (xevent->xclient.format == 32) &&
       (xevent->xclient.data.l[0] == (long)ECORE_X_ATOM_WM_DELETE_WINDOW))
     {
        Ecore_X_Event_Window_Delete_Request *e;

        e = calloc(1, sizeof(Ecore_X_Event_Window_Delete_Request));
        if (!e)
          return;

        e->win = xevent->xclient.window;
        e->time = _ecore_x_event_last_time;
        ecore_event_add(ECORE_X_EVENT_WINDOW_DELETE_REQUEST, e, NULL, NULL);
     }
   else if ((xevent->xclient.message_type == ECORE_X_ATOM_NET_WM_MOVERESIZE) &&
            (xevent->xclient.format == 32) &&
/* Ignore move and resize with keyboard */
            (xevent->xclient.data.l[2] < 9))
     {
        Ecore_X_Event_Window_Move_Resize_Request *e;

        e = calloc(1, sizeof(Ecore_X_Event_Window_Move_Resize_Request));
        if (!e)
          return;

        e->win = xevent->xclient.window;
        e->x = xevent->xclient.data.l[0];
        e->y = xevent->xclient.data.l[1];
        e->direction = xevent->xclient.data.l[2];
        e->button = xevent->xclient.data.l[3];
        e->source = xevent->xclient.data.l[4];
        ecore_event_add(ECORE_X_EVENT_WINDOW_MOVE_RESIZE_REQUEST, e, NULL, NULL);
     }
   /* Xdnd Client Message Handling Begin */
   /* Message Type: XdndEnter target */
   else if (xevent->xclient.message_type == ECORE_X_ATOM_XDND_ENTER)
     {
        Ecore_X_Event_Xdnd_Enter *e;
        Ecore_X_DND_Target *target;

        e = calloc(1, sizeof(Ecore_X_Event_Xdnd_Enter));
        if (!e) return;

        LOGFN(__FILE__, __LINE__, __FUNCTION__);

        target = _ecore_x_dnd_target_get();
        target->state = ECORE_X_DND_TARGET_ENTERED;
        target->source = xevent->xclient.data.l[0];
        target->win = xevent->xclient.window;
        target->version = (int)(xevent->xclient.data.l[1] >> 24);
        if (target->version > ECORE_X_DND_VERSION)
          {
             WRN("DND: Requested version %d, we only support up to %d",
                 target->version, ECORE_X_DND_VERSION);
             free(e);
             return;
          }

        if (xevent->xclient.data.l[1] & 0x1UL)
          {
             /* source supports more than 3 types, fetch property */
             unsigned char *data;
             Ecore_X_Atom *types;
             int i, num_ret;

             LOGFN(__FILE__, __LINE__, __FUNCTION__);
             if (!(ecore_x_window_prop_property_get(target->source,
                                                    ECORE_X_ATOM_XDND_TYPE_LIST,
                                                    XA_ATOM,
                                                    32, &data, &num_ret)))
               {
                  WRN(
                    "DND: Could not fetch data type list from source window, aborting.");
                  if (data) free(data);
                  free(e);
                  return;
               }

             types = (Ecore_X_Atom *)data;
             e->types = calloc(num_ret, sizeof(char *));
             if (e->types)
               {
                  LOGFN(__FILE__, __LINE__, __FUNCTION__);
                  for (i = 0; i < num_ret; i++)
                    e->types[i] = XGetAtomName(_ecore_x_disp, types[i]);
               }

             e->num_types = num_ret;
             if (data) free(data);
          }
        else
          {
             int i = 0;

             e->types = calloc(3, sizeof(char *));
             if (e->types)
               {
                  LOGFN(__FILE__, __LINE__, __FUNCTION__);
                  while ((i < 3) && (xevent->xclient.data.l[i + 2]))
                    {
                       e->types[i] = XGetAtomName(_ecore_x_disp,
                                                  xevent->xclient.data.l[i + 2]);
                       i++;
                    }
               }

             e->num_types = i;
          }

        e->win = target->win;
        e->source = target->source;
        ecore_event_add(ECORE_X_EVENT_XDND_ENTER, e,
                        _ecore_x_event_free_xdnd_enter, NULL);
     }
   /* Message Type: XdndPosition target */
   else if (xevent->xclient.message_type == ECORE_X_ATOM_XDND_POSITION)
     {
        Ecore_X_Event_Xdnd_Position *e;
        Ecore_X_DND_Target *target;

        LOGFN(__FILE__, __LINE__, __FUNCTION__);

        target = _ecore_x_dnd_target_get();
        if ((target->source != (Ecore_X_Window)xevent->xclient.data.l[0]) ||
            (target->win != xevent->xclient.window))
          return;

        target->pos.x = xevent->xclient.data.l[2] >> 16;
        target->pos.y = xevent->xclient.data.l[2] & 0xFFFFUL;
        target->action = xevent->xclient.data.l[4]; /* Version 2 */

        target->time = (target->version >= 1) ?
          (Time)xevent->xclient.data.l[3] : CurrentTime;

        e = calloc(1, sizeof(Ecore_X_Event_Xdnd_Position));
        if (!e) return;

        e->win = target->win;
        e->source = target->source;
        e->position.x = target->pos.x;
        e->position.y = target->pos.y;
        e->action = target->action;
        ecore_event_add(ECORE_X_EVENT_XDND_POSITION, e, NULL, NULL);
     }
   /* Message Type: XdndStatus source */
   else if (xevent->xclient.message_type == ECORE_X_ATOM_XDND_STATUS)
     {
        Ecore_X_Event_Xdnd_Status *e;
        Ecore_X_DND_Source *source;

        LOGFN(__FILE__, __LINE__, __FUNCTION__);

        source = _ecore_x_dnd_source_get();
        /* Make sure source/target match */
        if ((source->win != xevent->xclient.window) ||
            (source->dest != (Window)xevent->xclient.data.l[0]))
          return;

        source->await_status = 0;

        source->will_accept = xevent->xclient.data.l[1] & 0x1UL;
        source->suppress = (xevent->xclient.data.l[1] & 0x2UL) ? 0 : 1;

        source->rectangle.x = xevent->xclient.data.l[2] >> 16;
        source->rectangle.y = xevent->xclient.data.l[2] & 0xFFFFUL;
        source->rectangle.width = xevent->xclient.data.l[3] >> 16;
        source->rectangle.height = xevent->xclient.data.l[3] & 0xFFFFUL;

        source->accepted_action = xevent->xclient.data.l[4];

        e = calloc(1, sizeof(Ecore_X_Event_Xdnd_Status));
        if (!e) return;

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
   else if (xevent->xclient.message_type == ECORE_X_ATOM_XDND_LEAVE)
     {
        Ecore_X_Event_Xdnd_Leave *e;
        Ecore_X_DND_Target *target;

        LOGFN(__FILE__, __LINE__, __FUNCTION__);

        target = _ecore_x_dnd_target_get();
        if ((target->source != (Ecore_X_Window)xevent->xclient.data.l[0]) ||
            (target->win != xevent->xclient.window))
          return;

        target->state = ECORE_X_DND_TARGET_IDLE;

        e = calloc(1, sizeof(Ecore_X_Event_Xdnd_Leave));
        if (!e) return;

        e->win = xevent->xclient.window;
        e->source = (Window)xevent->xclient.data.l[0];
        ecore_event_add(ECORE_X_EVENT_XDND_LEAVE, e, NULL, NULL);
     }
   /* Message Type: XdndDrop target */
   else if (xevent->xclient.message_type == ECORE_X_ATOM_XDND_DROP)
     {
        Ecore_X_Event_Xdnd_Drop *e;
        Ecore_X_DND_Target *target;

        LOGFN(__FILE__, __LINE__, __FUNCTION__);

        target = _ecore_x_dnd_target_get();
        /* Match source/target */
        if ((target->source != (Window)xevent->xclient.data.l[0]) ||
            (target->win != xevent->xclient.window))
          return;

        target->time = (target->version >= 1) ?
          (Time)xevent->xclient.data.l[2] : _ecore_x_event_last_time;

        e = calloc(1, sizeof(Ecore_X_Event_Xdnd_Drop));
        if (!e) return;

        e->win = target->win;
        e->source = target->source;
        e->action = target->action;
        e->position.x = target->pos.x;
        e->position.y = target->pos.y;
        ecore_event_add(ECORE_X_EVENT_XDND_DROP, e, NULL, NULL);
     }
   /* Message Type: XdndFinished source */
   else if (xevent->xclient.message_type == ECORE_X_ATOM_XDND_FINISHED)
     {
        Ecore_X_Event_Xdnd_Finished *e;
        Ecore_X_DND_Source *source;
        Eina_Bool completed = EINA_TRUE;

        LOGFN(__FILE__, __LINE__, __FUNCTION__);

        source = _ecore_x_dnd_source_get();
        /* Match source/target */
        if ((source->win != xevent->xclient.window) ||
            (source->dest != (Window)xevent->xclient.data.l[0]))
          return;

        if ((source->version < 5) || (xevent->xclient.data.l[1] & 0x1UL))
          {
             LOGFN(__FILE__, __LINE__, __FUNCTION__);
             /* Target successfully performed drop action */
             ecore_x_selection_xdnd_clear();
             source->state = ECORE_X_DND_SOURCE_IDLE;
          }
        else if (source->version >= 5)
          {
             completed = EINA_FALSE;
             source->state = ECORE_X_DND_SOURCE_CONVERTING;

             /* FIXME: Probably need to add a timer to switch back to idle
              * and discard the selection data */
          }

        e = calloc(1, sizeof(Ecore_X_Event_Xdnd_Finished));
        if (!e) return;

        e->win = source->win;
        e->target = source->dest;
        e->completed = completed;
        if (source->version >= 5)
          {
             source->accepted_action = xevent->xclient.data.l[2];
             e->action = source->accepted_action;
          }
        else
          {
             source->accepted_action = 0;
             e->action = source->action;
          }

        ecore_event_add(ECORE_X_EVENT_XDND_FINISHED, e, NULL, NULL);
     }
   else if (xevent->xclient.message_type == ECORE_X_ATOM_NET_WM_STATE)
     {
        Ecore_X_Event_Window_State_Request *e;

        e = calloc(1, sizeof(Ecore_X_Event_Window_State_Request));
        if (!e) return;

        e->win = xevent->xclient.window;
        if (xevent->xclient.data.l[0] == 0)
          e->action = ECORE_X_WINDOW_STATE_ACTION_REMOVE;
        else if (xevent->xclient.data.l[0] == 1)
          e->action = ECORE_X_WINDOW_STATE_ACTION_ADD;
        else if (xevent->xclient.data.l[0] == 2)
          e->action = ECORE_X_WINDOW_STATE_ACTION_TOGGLE;
        else
          {
             free(e);
             return;
          }

        LOGFN(__FILE__, __LINE__, __FUNCTION__);
        e->state[0] = _ecore_x_netwm_state_get(xevent->xclient.data.l[1]);
        if (e->state[0] == ECORE_X_WINDOW_STATE_UNKNOWN)
          {
//	     char *name;
             LOGFN(__FILE__, __LINE__, __FUNCTION__);

//	     name = XGetAtomName(_ecore_x_disp, xevent->xclient.data.l[1]);
//	     if (name) ERR("Unknown state: %s", name);
//	     XFree(name);
          }
        e->state[1] = _ecore_x_netwm_state_get(xevent->xclient.data.l[2]);
        if (e->state[1] == ECORE_X_WINDOW_STATE_UNKNOWN)
          {
//	     char *name;
             LOGFN(__FILE__, __LINE__, __FUNCTION__);

//	     name = XGetAtomName(_ecore_x_disp, xevent->xclient.data.l[2]);
//	     if (name) ERR("Unknown state: %s", name);
//	     XFree(name);
          }

        e->source = xevent->xclient.data.l[3];

        ecore_event_add(ECORE_X_EVENT_WINDOW_STATE_REQUEST, e, NULL, NULL);
     }
   else if ((xevent->xclient.message_type == ECORE_X_ATOM_WM_CHANGE_STATE)
            && (xevent->xclient.format == 32)
            && (xevent->xclient.data.l[0] == IconicState))
     {
        Ecore_X_Event_Window_State_Request *e;

        e = calloc(1, sizeof(Ecore_X_Event_Window_State_Request));
        if (!e)
          return;

        e->win = xevent->xclient.window;
        e->action = ECORE_X_WINDOW_STATE_ACTION_ADD;
        e->state[0] = ECORE_X_WINDOW_STATE_ICONIFIED;

        ecore_event_add(ECORE_X_EVENT_WINDOW_STATE_REQUEST, e, NULL, NULL);
     }
   else if ((xevent->xclient.message_type == ECORE_X_ATOM_NET_WM_DESKTOP)
            && (xevent->xclient.format == 32))
     {
        Ecore_X_Event_Desktop_Change *e;

        e = calloc(1, sizeof(Ecore_X_Event_Desktop_Change));
        if (!e)
          return;

        e->win = xevent->xclient.window;
        e->desk = xevent->xclient.data.l[0];
        e->source = xevent->xclient.data.l[1];

        ecore_event_add(ECORE_X_EVENT_DESKTOP_CHANGE, e, NULL, NULL);
     }
   else if (xevent->xclient.message_type ==
            ECORE_X_ATOM_NET_REQUEST_FRAME_EXTENTS)
     {
        Ecore_X_Event_Frame_Extents_Request *e;

        e = calloc(1, sizeof(Ecore_X_Event_Frame_Extents_Request));
        if (!e)
          return;

        e->win = xevent->xclient.window;

        ecore_event_add(ECORE_X_EVENT_FRAME_EXTENTS_REQUEST, e, NULL, NULL);
     }
   else if ((xevent->xclient.message_type == ECORE_X_ATOM_WM_PROTOCOLS)
            && ((Ecore_X_Atom)xevent->xclient.data.l[0] ==
                ECORE_X_ATOM_NET_WM_PING)
            && (xevent->xclient.format == 32))
     {
        Ecore_X_Event_Ping *e;
        Ecore_X_Window root = 0;

        e = calloc(1, sizeof(Ecore_X_Event_Ping));
        if (!e)
          return;

        e->win = xevent->xclient.window;
        e->time = xevent->xclient.data.l[1];
        e->event_win = xevent->xclient.data.l[2];

        /* send a reply anyway - we are alive... eventloop at least */
        ecore_event_add(ECORE_X_EVENT_PING, e, NULL, NULL);
        if (ScreenCount(_ecore_x_disp) > 1)
          {
             LOGFN(__FILE__, __LINE__, __FUNCTION__);
             root = ecore_x_window_root_get(e->win);
          }
        else
          root = DefaultRootWindow(_ecore_x_disp);

        if (xevent->xclient.window != root)
          {
             xevent->xclient.window = root;
             XSendEvent(_ecore_x_disp, root, False,
                        SubstructureRedirectMask | SubstructureNotifyMask,
                        xevent);
             if (_ecore_xlib_sync) ecore_x_sync();
          }
     }
   else if ((xevent->xclient.message_type ==
             ECORE_X_ATOM_NET_STARTUP_INFO_BEGIN) &&
            (xevent->xclient.format == 8))
     _ecore_x_netwm_startup_info_begin(xevent->xclient.window,
                                       xevent->xclient.data.b);
   else if ((xevent->xclient.message_type == ECORE_X_ATOM_NET_STARTUP_INFO) &&
            (xevent->xclient.format == 8))
     _ecore_x_netwm_startup_info(xevent->xclient.window,
                                 xevent->xclient.data.b);
   else if ((xevent->xclient.message_type == 27777)
            && (xevent->xclient.data.l[0] == 0x7162534)
            && (xevent->xclient.format == 32)
            && (xevent->xclient.window == _ecore_x_private_win))
     {
        int val = xevent->xclient.data.l[1] & 0xff;
        int anymod = (xevent->xclient.data.l[1] >> 8) & 0xff;
        int mod = xevent->xclient.data.l[4];
        int b = xevent->xclient.data.l[3];
        Ecore_X_Window swin = xevent->xclient.data.l[2];

        /* a grab sync marker */
        if (val == 1)
          {
             _ecore_x_window_grab_remove(swin, b, mod, anymod);
          }
        else if (val == 2)
          {
             const char *str;

             str = ecore_x_keysym_string_get(b);
             if (str) _ecore_x_key_grab_remove(swin, str, mod, anymod);
          }
     }
   else
     {
        Ecore_X_Event_Client_Message *e;
        int i;

        e = calloc(1, sizeof(Ecore_X_Event_Client_Message));
        if (!e)
          return;

        e->win = xevent->xclient.window;
        e->message_type = xevent->xclient.message_type;
        e->format = xevent->xclient.format;
        for (i = 0; i < 5; i++)
          e->data.l[i] = xevent->xclient.data.l[i];

        ecore_event_add(ECORE_X_EVENT_CLIENT_MESSAGE, e, NULL, NULL);
     }
}

void
_ecore_x_event_handle_mapping_notify(XEvent *xevent)
{
   Ecore_X_Event_Mapping_Change *e;

   _ecore_x_last_event_mouse_move = 0;

   _ecore_x_window_grab_suspend();
   _ecore_x_key_grab_suspend();

   XRefreshKeyboardMapping((XMappingEvent *)xevent);
   _ecore_x_modifiers_get();

   _ecore_x_window_grab_resume();
   _ecore_x_key_grab_resume();
   e = calloc(1, sizeof(Ecore_X_Event_Mapping_Change));
   if (!e) return;
   switch (xevent->xmapping.request)
     {
      case MappingModifier:
        e->type = ECORE_X_MAPPING_MODIFIER;
        break;

      case MappingKeyboard:
        e->type = ECORE_X_MAPPING_KEYBOARD;
        break;

      case MappingPointer:
      default:
        e->type = ECORE_X_MAPPING_MOUSE;
        break;
     }
   e->keycode = xevent->xmapping.first_keycode;
   e->num = xevent->xmapping.count;
   ecore_event_add(ECORE_X_EVENT_MAPPING_CHANGE, e, NULL, NULL);
}

void
_ecore_x_event_handle_shape_change(XEvent *xevent)
{
   XShapeEvent *shape_event;
   Ecore_X_Event_Window_Shape *e;

   _ecore_x_last_event_mouse_move = 0;
   shape_event = (XShapeEvent *)xevent;
   e = calloc(1, sizeof(Ecore_X_Event_Window_Shape));
   if (!e)
     return;

   e->win = shape_event->window;
   e->time = shape_event->time;
   switch (shape_event->kind)
     {
      case ShapeBounding:
        e->type = ECORE_X_SHAPE_BOUNDING;
        break;

      case ShapeClip:
        e->type = ECORE_X_SHAPE_CLIP;
        break;

      case ShapeInput:
        e->type = ECORE_X_SHAPE_INPUT;
        break;

      default:
        break;
     }
   e->x = shape_event->x;
   e->y = shape_event->y;
   e->w = shape_event->width;
   e->h = shape_event->height;
   e->shaped = shape_event->shaped;
   ecore_event_add(ECORE_X_EVENT_WINDOW_SHAPE, e, NULL, NULL);
}

void
_ecore_x_event_handle_screensaver_notify(XEvent *xevent)
{
#ifdef ECORE_XSS
   XScreenSaverNotifyEvent *screensaver_event;
   Ecore_X_Event_Screensaver_Notify *e;

   _ecore_x_last_event_mouse_move = 0;
   screensaver_event = (XScreenSaverNotifyEvent *)xevent;
   e = calloc(1, sizeof(Ecore_X_Event_Screensaver_Notify));
   if (!e)
     return;

   e->win = screensaver_event->window;
   if ((screensaver_event->state == ScreenSaverOn) ||
       (screensaver_event->state == ScreenSaverCycle))
     e->on = EINA_TRUE;
   else
     e->on = EINA_FALSE;

   e->time = screensaver_event->time;
   ecore_event_add(ECORE_X_EVENT_SCREENSAVER_NOTIFY, e, NULL, NULL);
#else /* ifdef ECORE_XSS */
   xevent = NULL;
#endif /* ifdef ECORE_XSS */
}

void
_ecore_x_event_handle_sync_counter(XEvent *xevent)
{
   XSyncCounterNotifyEvent *sync_counter_event;
   Ecore_X_Event_Sync_Counter *e;

   _ecore_x_last_event_mouse_move = 0;
   sync_counter_event = (XSyncCounterNotifyEvent *)xevent;
   e = calloc(1, sizeof(Ecore_X_Event_Sync_Counter));
   if (!e)
     return;

   e->time = sync_counter_event->time;
   ecore_event_add(ECORE_X_EVENT_SYNC_COUNTER, e, NULL, NULL);
}

void
_ecore_x_event_handle_sync_alarm(XEvent *xevent)
{
   XSyncAlarmNotifyEvent *sync_alarm_event;
   Ecore_X_Event_Sync_Alarm *e;

   _ecore_x_last_event_mouse_move = 0;
   sync_alarm_event = (XSyncAlarmNotifyEvent *)xevent;

   e = calloc(1, sizeof(Ecore_X_Event_Sync_Alarm));
   if (!e)
     return;

   e->time = sync_alarm_event->time;
   e->alarm = sync_alarm_event->alarm;
   ecore_event_add(ECORE_X_EVENT_SYNC_ALARM, e, NULL, NULL);
}

#ifdef ECORE_XRANDR
void
_ecore_x_event_handle_randr_change(XEvent *xevent)
{
   XRRScreenChangeNotifyEvent *randr_event;
   Ecore_X_Event_Screen_Change *e;

   _ecore_x_last_event_mouse_move = 0;
   randr_event = (XRRScreenChangeNotifyEvent *)xevent;
   if (!XRRUpdateConfiguration(xevent))
     ERR("Can't update RR config!");

   e = calloc(1, sizeof(Ecore_X_Event_Screen_Change));
   if (!e)
     return;

   e->win = randr_event->window;
   e->root = randr_event->root;
   e->size.width = randr_event->width;
   e->size.height = randr_event->height;
   e->time = randr_event->timestamp;
   e->config_time = randr_event->config_timestamp;
   e->size.width_mm = randr_event->mwidth;
   e->size.height_mm = randr_event->mheight;
   e->orientation = randr_event->rotation;
   e->subpixel_order = randr_event->subpixel_order;
   ecore_event_add(ECORE_X_EVENT_SCREEN_CHANGE, e, NULL, NULL);
}

static void
_ecore_x_event_handle_randr_notify_crtc_change(const XRRNotifyEvent *xevent)
{
   const XRRCrtcChangeNotifyEvent *randr_event;
   Ecore_X_Event_Randr_Crtc_Change *e;

   randr_event = (const XRRCrtcChangeNotifyEvent *)xevent;

   e = calloc(1, sizeof(Ecore_X_Event_Randr_Crtc_Change));
   if (!e)
     return;

   e->win = randr_event->window;
   e->crtc = randr_event->crtc;
   e->mode = randr_event->mode;
   e->orientation = randr_event->rotation;
   e->geo.x = randr_event->x;
   e->geo.y = randr_event->y;
   e->geo.w = randr_event->width;
   e->geo.h = randr_event->height;
   ecore_event_add(ECORE_X_EVENT_RANDR_CRTC_CHANGE, e, NULL, NULL);
}

static void
_ecore_x_event_handle_randr_notify_output_change(const XRRNotifyEvent *xevent)
{
   const XRROutputChangeNotifyEvent *randr_event;
   Ecore_X_Event_Randr_Output_Change *e;

   randr_event = (const XRROutputChangeNotifyEvent *)xevent;

   e = calloc(1, sizeof(Ecore_X_Event_Randr_Output_Change));
   if (!e)
     return;

   e->win = randr_event->window;
   e->output = randr_event->output;
   e->crtc = randr_event->crtc;
   e->mode = randr_event->mode;
   e->orientation = randr_event->rotation;
   e->connection = randr_event->connection;
   e->subpixel_order = randr_event->subpixel_order;
   ecore_event_add(ECORE_X_EVENT_RANDR_OUTPUT_CHANGE, e, NULL, NULL);
}

static void
_ecore_x_event_handle_randr_notify_output_property(const XRRNotifyEvent *xevent)
{
   const XRROutputPropertyNotifyEvent *randr_event;
   Ecore_X_Event_Randr_Output_Property_Notify *e;

   randr_event = (const XRROutputPropertyNotifyEvent *)xevent;

   e = calloc(1, sizeof(Ecore_X_Event_Randr_Output_Property_Notify));
   if (!e)
     return;

   e->win = randr_event->window;
   e->output = randr_event->output;
   e->property = randr_event->property;
   e->time = randr_event->timestamp;
   if (randr_event->state == PropertyNewValue)
     e->state = ECORE_X_RANDR_PROPERTY_CHANGE_ADD;
   else
     e->state = ECORE_X_RANDR_PROPERTY_CHANGE_DEL;
   ecore_event_add(ECORE_X_EVENT_RANDR_OUTPUT_PROPERTY_NOTIFY, e, NULL, NULL);
}

void
_ecore_x_event_handle_randr_notify(XEvent *xevent)
{
   const XRRNotifyEvent *randr_event;

   _ecore_x_last_event_mouse_move = 0;
   randr_event = (const XRRNotifyEvent *)xevent;
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   switch (randr_event->subtype)
     {
      case RRNotify_CrtcChange:
        _ecore_x_event_handle_randr_notify_crtc_change(randr_event);
        break;

      case RRNotify_OutputChange:
        _ecore_x_event_handle_randr_notify_output_change(randr_event);
        break;

      case RRNotify_OutputProperty:
        _ecore_x_event_handle_randr_notify_output_property(randr_event);
        break;

      default:
        ERR("Unknown XRandR RRNotify subtype: %d.",
            randr_event->subtype);
        break;
     }
}

#endif /* ifdef ECORE_XRANDR */

#ifdef ECORE_XFIXES
void
_ecore_x_event_handle_fixes_selection_notify(XEvent *event)
{
   XFixesSelectionNotifyEvent *notify_event =
     (XFixesSelectionNotifyEvent *)event;
   Ecore_X_Event_Fixes_Selection_Notify *e;
   Ecore_X_Atom sel;

   _ecore_x_last_event_mouse_move = 0;
   /* Nothing here yet */

   e = calloc(1, sizeof(*e));
   if (!e)
     return;

   e->win = notify_event->window;
   e->owner = notify_event->owner;
   e->time = notify_event->timestamp;
   e->selection_time = notify_event->selection_timestamp;
   e->atom = sel = notify_event->selection;
   if (sel == ECORE_X_ATOM_SELECTION_PRIMARY)
     e->selection = ECORE_X_SELECTION_PRIMARY;
   else if (sel == ECORE_X_ATOM_SELECTION_SECONDARY)
     e->selection = ECORE_X_SELECTION_SECONDARY;
   else if (sel == ECORE_X_ATOM_SELECTION_XDND)
     e->selection = ECORE_X_SELECTION_XDND;
   else if (sel == ECORE_X_ATOM_SELECTION_CLIPBOARD)
     e->selection = ECORE_X_SELECTION_CLIPBOARD;
   else
     e->selection = ECORE_X_SELECTION_OTHER;
   e->reason = notify_event->subtype;

   ecore_event_add(ECORE_X_EVENT_FIXES_SELECTION_NOTIFY, e, NULL, NULL);
}

#endif /* ifdef ECORE_XFIXES */

#ifdef ECORE_XDAMAGE
void
_ecore_x_event_handle_damage_notify(XEvent *event)
{
   XDamageNotifyEvent *damage_event;   
   Ecore_X_Event_Damage *e;

   _ecore_x_last_event_mouse_move = 0;
   damage_event = (XDamageNotifyEvent *)event;

   e = calloc(1, sizeof(Ecore_X_Event_Damage));
   if (!e)
     return;

   e->level = damage_event->level;
   e->drawable = damage_event->drawable;
   e->damage = damage_event->damage;
   e->more = damage_event->more;
   e->time = damage_event->timestamp;
   e->area.x = damage_event->area.x;
   e->area.y = damage_event->area.y;
   e->area.width = damage_event->area.width;
   e->area.height = damage_event->area.height;
   e->geometry.x = damage_event->geometry.x;
   e->geometry.y = damage_event->geometry.y;
   e->geometry.width = damage_event->geometry.width;
   e->geometry.height = damage_event->geometry.height;

   ecore_event_add(ECORE_X_EVENT_DAMAGE_NOTIFY, e, NULL, NULL);
}

#endif /* ifdef ECORE_XDAMAGE */

static void
_ecore_x_event_free_generic_event(void *data,
                                  void *ev)
{
   Ecore_X_Event_Generic *e = (Ecore_X_Event_Generic *)ev;

   if (data)
     {
        if (e->data)
          XFreeEventData(_ecore_x_disp, (XGenericEventCookie *)data);
        free(data);
     }
   free(e);
}

void
_ecore_x_event_handle_generic_event(XEvent *event)
{
   XGenericEvent *generic_event;
   Ecore_X_Event_Generic *e;
   XGenericEventCookie *data;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   generic_event = (XGenericEvent *)event;

#ifdef ECORE_XPRESENT
   if (generic_event->extension == _ecore_x_present_major)
     {
        _ecore_x_present_handler(generic_event);
        return;
     }
#endif

   e = calloc(1, sizeof(Ecore_X_Event_Generic));
   if (!e)
     return;

   if (XGetEventData(_ecore_x_disp, &(event->xcookie)))
     {
        e->cookie = event->xcookie.cookie;
        e->data = event->xcookie.data;
     }
   else
     {
        e->cookie = 0;
        e->data = NULL;
     }

   e->extension = generic_event->extension;
   e->evtype = generic_event->evtype;
#ifdef ECORE_XI2
   if (e->extension == _ecore_x_xi2_opcode)
     _ecore_x_input_handler(event);
#endif /* ifdef ECORE_XI2 */
   data = malloc(sizeof(XGenericEventCookie));
   if (data) memcpy(data, &(event->xcookie), sizeof(XGenericEventCookie));
   ecore_event_add(ECORE_X_EVENT_GENERIC,
                   e,
                   _ecore_x_event_free_generic_event,
                   data);
}

#ifdef ECORE_XGESTURE
void
_ecore_x_event_handle_gesture_notify_flick(XEvent *xevent)
{
   XGestureNotifyFlickEvent *xfe;
   Ecore_X_Event_Gesture_Notify_Flick *e;

   _ecore_x_last_event_mouse_move = 0;
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   xfe = (XGestureNotifyFlickEvent *)xevent;
   e = calloc(1, sizeof(Ecore_X_Event_Gesture_Notify_Flick));
   if (!e)
     return;

   e->win = xfe->window;
   e->time = xfe->time;
   e->subtype = xfe->kind;
   e->num_fingers = xfe->num_finger;
   e->distance = xfe->distance;
   e->duration = xfe->duration;
   e->direction = xfe->direction;
   e->angle = XFixedToDouble(xfe->angle);

   ecore_event_add(ECORE_X_EVENT_GESTURE_NOTIFY_FLICK, e, NULL, NULL);
}

void
_ecore_x_event_handle_gesture_notify_pan(XEvent *xevent)
{
   XGestureNotifyPanEvent *xpe;
   Ecore_X_Event_Gesture_Notify_Pan *e;

   _ecore_x_last_event_mouse_move = 0;
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   xpe = (XGestureNotifyPanEvent *)xevent;
   e = calloc(1, sizeof(Ecore_X_Event_Gesture_Notify_Pan));
   if (!e)
     return;

   e->win = xpe->window;
   e->time = xpe->time;
   e->subtype = xpe->kind;
   e->num_fingers = xpe->num_finger;
   e->dx = xpe->dx;
   e->dy = xpe->dy;
   e->distance = xpe->distance;
   e->duration = xpe->duration;
   e->direction = xpe->direction;

   ecore_event_add(ECORE_X_EVENT_GESTURE_NOTIFY_PAN, e, NULL, NULL);
}

void
_ecore_x_event_handle_gesture_notify_pinchrotation(XEvent *xevent)
{
   XGestureNotifyPinchRotationEvent *xpre;
   Ecore_X_Event_Gesture_Notify_PinchRotation *e;

   _ecore_x_last_event_mouse_move = 0;
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   xpre = (XGestureNotifyPinchRotationEvent *)xevent;
   e = calloc(1, sizeof(Ecore_X_Event_Gesture_Notify_PinchRotation));
   if (!e)
     return;

   e->win = xpre->window;
   e->time = xpre->time;
   e->subtype = xpre->kind;
   e->num_fingers = xpre->num_finger;
   e->distance = xpre->distance;
   e->cx = xpre->cx;
   e->cy = xpre->cy;
   e->zoom = XFixedToDouble(xpre->zoom);
   e->angle = XFixedToDouble(xpre->angle);

   ecore_event_add(ECORE_X_EVENT_GESTURE_NOTIFY_PINCHROTATION, e, NULL, NULL);
}

void
_ecore_x_event_handle_gesture_notify_tap(XEvent *xevent)
{
   XGestureNotifyTapEvent *xte;
   Ecore_X_Event_Gesture_Notify_Tap *e;

   _ecore_x_last_event_mouse_move = 0;
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   xte = (XGestureNotifyTapEvent *)xevent;
   e = calloc(1, sizeof(Ecore_X_Event_Gesture_Notify_Tap));
   if (!e)
     return;

   e->win = xte->window;
   e->time = xte->time;
   e->subtype = xte->kind;
   e->num_fingers = xte->num_finger;
   e->cx = xte->cx;
   e->cy = xte->cy;
   e->tap_repeat = xte->tap_repeat;
   e->interval = xte->interval;

   ecore_event_add(ECORE_X_EVENT_GESTURE_NOTIFY_TAP, e, NULL, NULL);
}

void
_ecore_x_event_handle_gesture_notify_tapnhold(XEvent *xevent)
{
   XGestureNotifyTapNHoldEvent *xthe;
   Ecore_X_Event_Gesture_Notify_TapNHold *e;

   _ecore_x_last_event_mouse_move = 0;
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   xthe = (XGestureNotifyTapNHoldEvent *)xevent;
   e = calloc(1, sizeof(Ecore_X_Event_Gesture_Notify_TapNHold));
   if (!e)
     return;

   e->win = xthe->window;
   e->time = xthe->time;
   e->subtype = xthe->kind;
   e->num_fingers = xthe->num_finger;
   e->cx = xthe->cx;
   e->cy = xthe->cy;
   e->interval = xthe->interval;
   e->hold_time = xthe->holdtime;

   ecore_event_add(ECORE_X_EVENT_GESTURE_NOTIFY_TAPNHOLD, e, NULL, NULL);
}

void
_ecore_x_event_handle_gesture_notify_hold(XEvent *xevent)
{
   XGestureNotifyHoldEvent *xhe;
   Ecore_X_Event_Gesture_Notify_Hold *e;

   _ecore_x_last_event_mouse_move = 0;
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   xhe = (XGestureNotifyHoldEvent *)xevent;
   e = calloc(1, sizeof(Ecore_X_Event_Gesture_Notify_Hold));
   if (!e)
     return;

   e->win = xhe->window;
   e->time = xhe->time;
   e->subtype = xhe->kind;
   e->num_fingers = xhe->num_finger;
   e->cx = xhe->cx;
   e->cy = xhe->cy;
   e->hold_time = xhe->holdtime;

   ecore_event_add(ECORE_X_EVENT_GESTURE_NOTIFY_HOLD, e, NULL, NULL);
}

void
_ecore_x_event_handle_gesture_notify_group(XEvent *xevent)
{
   XGestureNotifyGroupEvent *xge;
   Ecore_X_Event_Gesture_Notify_Group *e;

   _ecore_x_last_event_mouse_move = 0;
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   xge = (XGestureNotifyGroupEvent *)xevent;
   e = calloc(1, sizeof(Ecore_X_Event_Gesture_Notify_Group));
   if (!e)
     return;

   e->win = xge->window;
   e->time = xge->time;
   e->subtype = xge->kind;
   e->num_groups = xge->num_group;
   e->group_id = xge->groupid;

   ecore_event_add(ECORE_X_EVENT_GESTURE_NOTIFY_GROUP, e, NULL, NULL);
}

#endif /* ifdef ECORE_XGESTURE */
#ifdef ECORE_XKB
void
_ecore_x_event_handle_xkb(XEvent *xevent)
{
   XkbEvent *xkbev;
   Ecore_X_Event_Xkb *e;
   
   xkbev = (XkbEvent *) xevent;
   e = calloc(1, sizeof(Ecore_X_Event_Xkb));
   if (!e)
     return;
   e->group = xkbev->state.group;
   if (xkbev->any.xkb_type == XkbStateNotify)
     ecore_event_add(ECORE_X_EVENT_XKB_STATE_NOTIFY, e, NULL, NULL);
   else if ((xkbev->any.xkb_type == XkbNewKeyboardNotify) ||
            (xkbev->any.xkb_type == XkbMapNotify))
     {
        if (xkbev->any.xkb_type == XkbMapNotify)
          {
             XkbMapNotifyEvent *xkbmapping;

             xkbmapping = (XkbMapNotifyEvent *)xkbev;
             XkbRefreshKeyboardMapping(xkbmapping);
          }
        ecore_event_add(ECORE_X_EVENT_XKB_NEWKBD_NOTIFY, e, NULL, NULL);
     }
}
#endif /* ifdef ECORE_XKB */
