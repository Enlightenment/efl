/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>
#include <string.h>

#include <langinfo.h>

#include "ecore_private.h"
#include "Ecore.h"
#include "ecore_x_private.h"
#include "Ecore_X.h"
#include "Ecore_X_Atoms.h"

/** OpenBSD does not define CODESET
 * FIXME ??
 */

#ifndef CODESET
#define CODESET "INVALID"
#endif

#if 0
static void _ecore_x_event_free_window_prop_name_class_change(void *data, void *ev);
static void _ecore_x_event_free_window_prop_title_change(void *data, void *ev);
static void _ecore_x_event_free_window_prop_visible_title_change(void *data, void *ev);
static void _ecore_x_event_free_window_prop_icon_name_change(void *data, void *ev);
static void _ecore_x_event_free_window_prop_visible_icon_name_change(void *data, void *ev);
static void _ecore_x_event_free_window_prop_client_machine_change(void *data, void *ev);
#endif
static void _ecore_x_event_free_key_down(void *data, void *ev);
static void _ecore_x_event_free_key_up(void *data, void *ev);
static void _ecore_x_event_free_mouse_move(void *data, void *ev);

static Window _ecore_x_mouse_down_last_win = 0;
static Window _ecore_x_mouse_down_last_last_win = 0;
static Window _ecore_x_mouse_down_last_event_win = 0;
static Window _ecore_x_mouse_down_last_last_event_win = 0;
static Time _ecore_x_mouse_down_last_time = 0;
static Time _ecore_x_mouse_down_last_last_time = 0;
static int _ecore_x_mouse_up_count = 0;
static int _ecore_x_mouse_down_did_triple = 0;
static int _ecore_x_last_event_mouse_move = 0;
static Ecore_Event *_ecore_x_last_event_mouse_move_event = NULL;

EAPI void
ecore_x_event_mask_set(Ecore_X_Window w, Ecore_X_Event_Mask mask)
{
   XWindowAttributes attr;
   XSetWindowAttributes s_attr;

   if (!w) w = DefaultRootWindow(_ecore_x_disp);
   memset(&attr, 0, sizeof(XWindowAttributes));
   XGetWindowAttributes(_ecore_x_disp, w, &attr);
   s_attr.event_mask = mask | attr.your_event_mask;
   XChangeWindowAttributes(_ecore_x_disp, w, CWEventMask, &s_attr);
}

EAPI void
ecore_x_event_mask_unset(Ecore_X_Window w, Ecore_X_Event_Mask mask)
{
   XWindowAttributes attr;
   XSetWindowAttributes s_attr;

   if (!w) w = DefaultRootWindow(_ecore_x_disp);
   memset(&attr, 0, sizeof(XWindowAttributes));
   XGetWindowAttributes(_ecore_x_disp, w, &attr);
   s_attr.event_mask = attr.your_event_mask & ~mask;
   XChangeWindowAttributes(_ecore_x_disp, w, CWEventMask, &s_attr);
}

#if 0
static void
_ecore_x_event_free_window_prop_name_class_change(void *data, void *ev)
{
   Ecore_X_Event_Window_Prop_Name_Class_Change *e;

   e = ev;
   if (e->name) free(e->name);
   if (e->clas) free(e->clas);
   free(e);
}

static void
_ecore_x_event_free_window_prop_title_change(void *data, void *ev)
{
   Ecore_X_Event_Window_Prop_Title_Change *e;

   e = ev;
   if (e->title) free(e->title);
   free(e);
}

static void
_ecore_x_event_free_window_prop_visible_title_change(void *data, void *ev)
{
   Ecore_X_Event_Window_Prop_Visible_Title_Change *e;

   e = ev;
   if (e->title) free(e->title);
   free(e);
}

static void
_ecore_x_event_free_window_prop_icon_name_change(void *data, void *ev)
{
   Ecore_X_Event_Window_Prop_Icon_Name_Change *e;

   e = ev;
   if (e->name) free(e->name);
   free(e);
}

static void
_ecore_x_event_free_window_prop_visible_icon_name_change(void *data, void *ev)
{
   Ecore_X_Event_Window_Prop_Visible_Icon_Name_Change *e;

   e = ev;
   if (e->name) free(e->name);
   free(e);
}

static void
_ecore_x_event_free_window_prop_client_machine_change(void *data, void *ev)
{
   Ecore_X_Event_Window_Prop_Client_Machine_Change *e;

   e = ev;
   if (e->name) free(e->name);
   free(e);
}
#endif

static void
_ecore_x_event_free_key_down(void *data __UNUSED__, void *ev)
{
   Ecore_X_Event_Key_Down *e;

   e = ev;
   if (e->keyname) free(e->keyname);
   if (e->keysymbol) free(e->keysymbol);
   if (e->key_compose) free(e->key_compose);
   free(e);
}

static void
_ecore_x_event_free_key_up(void *data __UNUSED__, void *ev)
{
   Ecore_X_Event_Key_Up *e;

   e = ev;
   if (e->keyname) free(e->keyname);
   if (e->keysymbol) free(e->keysymbol);
   if (e->key_compose) free(e->key_compose);
   free(e);
}

static void
_ecore_x_event_free_mouse_move(void *data __UNUSED__, void *ev)
{
   Ecore_X_Event_Mouse_Move *e;

   e = ev;
   if (_ecore_x_last_event_mouse_move)
     {
        _ecore_x_last_event_mouse_move_event = NULL;
        _ecore_x_last_event_mouse_move = 0;
     }
   free(e);
}

static void
_ecore_x_event_free_xdnd_enter(void *data __UNUSED__, void *ev)
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
_ecore_x_event_free_selection_notify(void *data __UNUSED__, void *ev)
{
   Ecore_X_Event_Selection_Notify *e;
   Ecore_X_Selection_Data *sel;

   e = ev;
   sel = e->data;
   if (sel->free) sel->free(sel);
   free(e->target);
   free(e);
}

void
_ecore_x_event_handle_key_press(XEvent *xevent)
{
   Ecore_X_Event_Key_Down *e;
   char *keyname, *buf;
   int val, buflen = 256;
   KeySym sym;
   XComposeStatus status;

   _ecore_x_last_event_mouse_move = 0;
   e = calloc(1, sizeof(Ecore_X_Event_Key_Down));
   if (!e) return;
   buf = malloc(buflen);
   if (!buf)
     {
	free(e);
	return;
     }
   keyname = XKeysymToString(XKeycodeToKeysym(xevent->xkey.display, 
					      xevent->xkey.keycode, 0));
   if (!keyname)
     {
	snprintf(buf, buflen, "Keycode-%i", xevent->xkey.keycode);
	keyname = buf;
     }
   e->keyname = strdup(keyname);
   if (!e->keyname)
     {
	free(buf);
	free(e);
	return;
     }
   if (_ecore_x_ic)
     {
	Status mbstatus;

#ifdef X_HAVE_UTF8_STRING
	val = Xutf8LookupString(_ecore_x_ic, (XKeyEvent *)xevent, buf, buflen - 1, &sym, &mbstatus);
#else
	val = XmbLookupString(_ecore_x_ic, (XKeyEvent *)xevent, buf, buflen - 1, &sym, &mbstatus);
#endif
	if (mbstatus == XBufferOverflow)
	  {
	     buflen = val + 1;
	     buf = realloc(buf, buflen);
#ifdef X_HAVE_UTF8_STRING
	     val = Xutf8LookupString(_ecore_x_ic, (XKeyEvent *)xevent, buf, buflen - 1, &sym, &mbstatus);
#else
	     val = XmbLookupString(_ecore_x_ic, (XKeyEvent *)xevent, buf, buflen - 1, &sym, &mbstatus);
#endif
	  }
	if (val > 0)
	  {
	     buf[val] = 0;
#ifdef X_HAVE_UTF8_STRING
	     e->key_compose = strdup(buf);
#else
	     e->key_compose = ecore_txt_convert(nl_langinfo(CODESET), "UTF-8", buf);
#endif
	  }
     }
   else
     {
	val = XLookupString((XKeyEvent *)xevent, buf, sizeof(buf), &sym, &status);
	if (val > 0)
	  {
	     buf[val] = 0;
	     e->key_compose = ecore_txt_convert(nl_langinfo(CODESET), "UTF-8", buf);
	  }
        else e->key_compose = NULL;
     }
   keyname = XKeysymToString(sym);
   if (keyname) e->keysymbol = strdup(keyname);
   else e->keysymbol = strdup(e->keyname);
   if (!e->keysymbol)
     {
	if (e->keyname) free(e->keyname);
	if (e->key_compose) free(e->key_compose);
	free(buf);
	free(e);
	return;
     }
   if (xevent->xkey.subwindow) e->win = xevent->xkey.subwindow;
   else e->win = xevent->xkey.window;
   e->event_win = xevent->xkey.window;
   e->time = xevent->xkey.time;
   e->modifiers = xevent->xkey.state;
   e->same_screen = xevent->xkey.same_screen;
   e->root_win = xevent->xkey.root;
   _ecore_x_event_last_time = e->time;
   ecore_event_add(ECORE_X_EVENT_KEY_DOWN, e, _ecore_x_event_free_key_down, NULL);
   free(buf);
}

void
_ecore_x_event_handle_key_release(XEvent *xevent)
{
   Ecore_X_Event_Key_Up *e;
   char *keyname;
   int val;
   char buf[256];
   KeySym sym;
   XComposeStatus status;

   _ecore_x_last_event_mouse_move = 0;
   e = calloc(1, sizeof(Ecore_X_Event_Key_Up));
   if (!e) return;
   keyname = XKeysymToString(XKeycodeToKeysym(xevent->xkey.display, 
					      xevent->xkey.keycode, 0));
   if (!keyname)
     {
	snprintf(buf, sizeof(buf), "Keycode-%i", xevent->xkey.keycode);
	keyname = buf;
     }
   e->keyname = strdup(keyname);
   if (!e->keyname)
     {
	free(e);
	return;
     }
   val = XLookupString((XKeyEvent *)xevent, buf, sizeof(buf), &sym, &status);
   if (val > 0)
     {
	buf[val] = 0;
	e->key_compose = ecore_txt_convert("ISO8859-1", "UTF-8", buf);
     }
   else e->key_compose = NULL;
   keyname = XKeysymToString(sym);
   if (keyname) e->keysymbol = strdup(keyname);
   else e->keysymbol = strdup(e->keyname);
   if (!e->keysymbol)
     {
	if (e->keyname) free(e->keyname);
	if (e->key_compose) free(e->key_compose);
	free(e);
	return;
     }
   if (xevent->xkey.subwindow) e->win = xevent->xkey.subwindow;
   else e->win = xevent->xkey.window;
   e->event_win = xevent->xkey.window;
   e->time = xevent->xkey.time;
   e->modifiers = xevent->xkey.state;
   e->same_screen = xevent->xkey.same_screen;
   e->root_win = xevent->xkey.root;
   _ecore_x_event_last_time = e->time;
   ecore_event_add(ECORE_X_EVENT_KEY_UP, e, _ecore_x_event_free_key_up, NULL);
}

void
_ecore_x_event_handle_button_press(XEvent *xevent)
{
   int i;

   _ecore_x_last_event_mouse_move = 0;
   if ((xevent->xbutton.button > 3) && (xevent->xbutton.button < 8))
     {
	Ecore_X_Event_Mouse_Wheel *e;

	e = malloc(sizeof(Ecore_X_Event_Mouse_Wheel));
	if (!e) return;

	e->modifiers = xevent->xbutton.state;
	e->direction = 0;
	e->z = 0;
	if (xevent->xbutton.button == 4)
	  {
	     e->direction = 0;
	     e->z = -1;
	  }
	else if (xevent->xbutton.button == 5)
	  {
	     e->direction = 0;
	     e->z = 1;
	  }
	else if (xevent->xbutton.button == 6)
	  {
	     e->direction = 1;
	     e->z = -1;
	  }
	else if (xevent->xbutton.button == 7)
	  {
	     e->direction = 1;
	     e->z = 1;
	  }
	e->x = xevent->xbutton.x;
	e->y = xevent->xbutton.y;
	e->root.x = xevent->xbutton.x_root;
	e->root.y = xevent->xbutton.y_root;

	if (xevent->xbutton.subwindow)
	  e->win = xevent->xbutton.subwindow;
	else
	  e->win = xevent->xbutton.window;

	e->event_win = xevent->xbutton.window;
	e->same_screen = xevent->xbutton.same_screen;
	e->root_win = xevent->xbutton.root;
	e->time = xevent->xbutton.time;
	_ecore_x_event_last_time = e->time;
	_ecore_x_event_last_win = e->win;
	_ecore_x_event_last_root_x = e->root.x;
	_ecore_x_event_last_root_y = e->root.y;
	ecore_event_add(ECORE_X_EVENT_MOUSE_WHEEL, e, NULL, NULL);
	for (i = 0; i < _ecore_window_grabs_num; i++)
	  {
	     if ((_ecore_window_grabs[i] == xevent->xbutton.window) ||
		 (_ecore_window_grabs[i] == xevent->xbutton.subwindow))
	       {
		  int replay = 0;

		  if (_ecore_window_grab_replay_func)
		    replay = _ecore_window_grab_replay_func(_ecore_window_grab_replay_data, 
							    ECORE_X_EVENT_MOUSE_WHEEL,
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
	     Ecore_X_Event_Mouse_Move *e;

	     e = calloc(1, sizeof(Ecore_X_Event_Mouse_Move));
	     if (!e) return;
	     e->modifiers = xevent->xbutton.state;
	     e->x = xevent->xbutton.x;
	     e->y = xevent->xbutton.y;
	     e->root.x = xevent->xbutton.x_root;
	     e->root.y = xevent->xbutton.y_root;
	     if (xevent->xbutton.subwindow) e->win = xevent->xbutton.subwindow;
	     else e->win = xevent->xbutton.window;
	     e->same_screen = xevent->xbutton.same_screen;
	     e->root_win = xevent->xbutton.root;
	     e->event_win = xevent->xbutton.window;
	     e->time = xevent->xbutton.time;
	     _ecore_x_event_last_time = e->time;
	     _ecore_x_event_last_win = e->win;
	     _ecore_x_event_last_root_x = e->root.x;
	     _ecore_x_event_last_root_y = e->root.y;
	     ecore_event_add(ECORE_X_EVENT_MOUSE_MOVE, e, NULL, NULL);
	  }
	  {
	     Ecore_X_Event_Mouse_Button_Down *e;

            if (_ecore_x_mouse_down_did_triple)
              {
                 _ecore_x_mouse_down_last_win = 0;
                 _ecore_x_mouse_down_last_last_win = 0;
                 _ecore_x_mouse_down_last_event_win = 0;
                 _ecore_x_mouse_down_last_last_event_win = 0;
                 _ecore_x_mouse_down_last_time = 0;
                 _ecore_x_mouse_down_last_last_time = 0;
              }

	     e = calloc(1, sizeof(Ecore_X_Event_Mouse_Button_Down));
	     if (!e) return;
	     e->button = xevent->xbutton.button;
	     e->modifiers = xevent->xbutton.state;
	     e->x = xevent->xbutton.x;
	     e->y = xevent->xbutton.y;
	     e->root.x = xevent->xbutton.x_root;
	     e->root.y = xevent->xbutton.y_root;
	     if (xevent->xbutton.subwindow) e->win = xevent->xbutton.subwindow;
	     else e->win = xevent->xbutton.window;
	     e->same_screen = xevent->xbutton.same_screen;
	     e->root_win = xevent->xbutton.root;
	     e->event_win = xevent->xbutton.window;
	     e->time = xevent->xbutton.time;
	     if (e->win == e->event_win)
	       {
		  if (((int)(e->time - _ecore_x_mouse_down_last_time) <= 
		       (int)(1000 * _ecore_x_double_click_time)) &&
		      (e->win == _ecore_x_mouse_down_last_win) &&
		      (e->event_win == _ecore_x_mouse_down_last_event_win)
		      )
		    e->double_click = 1;
		  if (((int)(e->time - _ecore_x_mouse_down_last_last_time) <= 
		       (int)(2 * 1000 * _ecore_x_double_click_time)) &&
		      (e->win == _ecore_x_mouse_down_last_win) &&
                      (e->win == _ecore_x_mouse_down_last_last_win) &&
		      (e->event_win == _ecore_x_mouse_down_last_event_win) &&
                      (e->event_win == _ecore_x_mouse_down_last_last_event_win)
		      )
		    {
                        e->triple_click = 1;
		       _ecore_x_mouse_down_did_triple = 1;
		    }
                  else
                    _ecore_x_mouse_down_did_triple = 0;
	       }
             if (!e->double_click && !e->triple_click)
               _ecore_x_mouse_up_count = 0;
	     _ecore_x_event_last_time = e->time;
	     _ecore_x_event_last_win = e->win;
	     _ecore_x_event_last_root_x = e->root.x;
	     _ecore_x_event_last_root_y = e->root.y;
	     ecore_event_add(ECORE_X_EVENT_MOUSE_BUTTON_DOWN, e, NULL, NULL);
	     for (i = 0; i < _ecore_window_grabs_num; i++)
	       {
		  if ((_ecore_window_grabs[i] == xevent->xbutton.window) ||
		      (_ecore_window_grabs[i] == xevent->xbutton.subwindow))
		    {
		       int replay = 0;

		       if (_ecore_window_grab_replay_func)
			 replay = _ecore_window_grab_replay_func(_ecore_window_grab_replay_data, 
								 ECORE_X_EVENT_MOUSE_BUTTON_DOWN,
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
	     if (e->win == e->event_win)
	       {
		  if (!_ecore_x_mouse_down_did_triple)
		    {
		       _ecore_x_mouse_down_last_last_win = _ecore_x_mouse_down_last_win;
		       if (xevent->xbutton.subwindow)
			 _ecore_x_mouse_down_last_win = xevent->xbutton.subwindow;
		       else
			 _ecore_x_mouse_down_last_win = xevent->xbutton.window;
		       _ecore_x_mouse_down_last_last_event_win = _ecore_x_mouse_down_last_event_win;
		       _ecore_x_mouse_down_last_event_win = xevent->xbutton.window;
		       _ecore_x_mouse_down_last_last_time = _ecore_x_mouse_down_last_time;
		       _ecore_x_mouse_down_last_time = xevent->xbutton.time;
		    }
	       }
	  }
     }
}

void
_ecore_x_event_handle_button_release(XEvent *xevent)
{
   _ecore_x_last_event_mouse_move = 0;
   /* filter out wheel buttons */
   if ((xevent->xbutton.button <= 3) || (xevent->xbutton.button > 7))
     {
	  {
	     Ecore_X_Event_Mouse_Move *e;

	     e = calloc(1, sizeof(Ecore_X_Event_Mouse_Move));
	     if (!e) return;
	     e->modifiers = xevent->xbutton.state;
	     e->x = xevent->xbutton.x;
	     e->y = xevent->xbutton.y;
	     e->root.x = xevent->xbutton.x_root;
	     e->root.y = xevent->xbutton.y_root;
	     if (xevent->xbutton.subwindow) e->win = xevent->xbutton.subwindow;
	     else e->win = xevent->xbutton.window;
	     e->same_screen = xevent->xbutton.same_screen;
	     e->root_win = xevent->xbutton.root;
	     e->event_win = xevent->xbutton.window;
	     e->time = xevent->xbutton.time;
	     _ecore_x_event_last_time = e->time;
	     _ecore_x_event_last_win = e->win;
	     _ecore_x_event_last_root_x = e->root.x;
	     _ecore_x_event_last_root_y = e->root.y;
	     ecore_event_add(ECORE_X_EVENT_MOUSE_MOVE, e, NULL, NULL);
	  }
	  {
	     Ecore_X_Event_Mouse_Button_Up *e;

	     e = calloc(1, sizeof(Ecore_X_Event_Mouse_Button_Up));
	     if (!e) return;
	     e->button = xevent->xbutton.button;
	     e->modifiers = xevent->xbutton.state;
	     e->x = xevent->xbutton.x;
	     e->y = xevent->xbutton.y;
	     e->root.x = xevent->xbutton.x_root;
	     e->root.y = xevent->xbutton.y_root;
	     if (xevent->xbutton.subwindow) e->win = xevent->xbutton.subwindow;
	     else e->win = xevent->xbutton.window;
	     e->same_screen = xevent->xbutton.same_screen;
	     e->root_win = xevent->xbutton.root;
	     e->event_win = xevent->xbutton.window;
	     e->time = xevent->xbutton.time;
             _ecore_x_mouse_up_count++;
	     if (e->win == e->event_win)
	       {
		  if ((_ecore_x_mouse_up_count >= 2) &&
                      ((int)(e->time - _ecore_x_mouse_down_last_time) <= 
		       (int)(1000 * _ecore_x_double_click_time)) &&
		      (e->win == _ecore_x_mouse_down_last_win) &&
		      (e->event_win == _ecore_x_mouse_down_last_event_win)
		      )
		    e->double_click = 1;
		  if ((_ecore_x_mouse_up_count >= 3) &&
                     ((int)(e->time - _ecore_x_mouse_down_last_last_time) <= 
		       (int)(2 * 1000 * _ecore_x_double_click_time)) &&
		      (e->win == _ecore_x_mouse_down_last_win) &&
                      (e->win == _ecore_x_mouse_down_last_last_win) &&
		      (e->event_win == _ecore_x_mouse_down_last_event_win) &&
                      (e->event_win == _ecore_x_mouse_down_last_last_event_win)
		      )
                    e->triple_click = 1;
	       }
	     _ecore_x_event_last_time = e->time;
	     _ecore_x_event_last_win = e->win;
	     _ecore_x_event_last_root_x = e->root.x;
	     _ecore_x_event_last_root_y = e->root.y;
	     ecore_event_add(ECORE_X_EVENT_MOUSE_BUTTON_UP, e, NULL, NULL);
	  }
     }
}

void
_ecore_x_event_handle_motion_notify(XEvent *xevent)
{
   Ecore_X_Event_Mouse_Move *e;
   Ecore_Event *event;
   
   if (_ecore_x_last_event_mouse_move)
     {
        ecore_event_del(_ecore_x_last_event_mouse_move_event);
        _ecore_x_last_event_mouse_move = 0;
        _ecore_x_last_event_mouse_move_event = NULL;
     }
   e = calloc(1, sizeof(Ecore_X_Event_Mouse_Move));
   if (!e) return;
   e->modifiers = xevent->xmotion.state;
   e->x = xevent->xmotion.x;
   e->y = xevent->xmotion.y;
   e->root.x = xevent->xmotion.x_root;
   e->root.y = xevent->xmotion.y_root;
   if (xevent->xmotion.subwindow) e->win = xevent->xmotion.subwindow;
   else e->win = xevent->xmotion.window;
   e->same_screen = xevent->xmotion.same_screen;
   e->root_win = xevent->xmotion.root;
   e->event_win = xevent->xmotion.window;
   e->time = xevent->xmotion.time;
   _ecore_x_event_last_time = e->time;
   _ecore_x_event_last_win = e->win;
   _ecore_x_event_last_root_x = e->root.x;
   _ecore_x_event_last_root_y = e->root.y;

   /* Xdnd handling */
   _ecore_x_dnd_drag(xevent->xmotion.root, e->root.x, e->root.y);

   event = ecore_event_add(ECORE_X_EVENT_MOUSE_MOVE, e, _ecore_x_event_free_mouse_move, NULL);
   _ecore_x_last_event_mouse_move = 1;
   _ecore_x_last_event_mouse_move_event  = event;
}

void
_ecore_x_event_handle_enter_notify(XEvent *xevent)
{
   _ecore_x_last_event_mouse_move = 0;
     {
	Ecore_X_Event_Mouse_Move *e;

	e = calloc(1, sizeof(Ecore_X_Event_Mouse_Move));
	if (!e) return;
	e->modifiers = xevent->xcrossing.state;
	e->x = xevent->xcrossing.x;
	e->y = xevent->xcrossing.y;
	e->root.x = xevent->xcrossing.x_root;
	e->root.y = xevent->xcrossing.y_root;
	if (xevent->xcrossing.subwindow) e->win = xevent->xcrossing.subwindow;
	else e->win = xevent->xcrossing.window;
	e->same_screen = xevent->xcrossing.same_screen;
	e->root_win = xevent->xcrossing.root;
	e->event_win = xevent->xcrossing.window;
	e->time = xevent->xcrossing.time;
	_ecore_x_event_last_time = e->time;
	_ecore_x_event_last_win = e->win;
	_ecore_x_event_last_root_x = e->root.x;
	_ecore_x_event_last_root_y = e->root.y;
	ecore_event_add(ECORE_X_EVENT_MOUSE_MOVE, e, NULL, NULL);
     }
     {
	Ecore_X_Event_Mouse_In *e;

	e = calloc(1, sizeof(Ecore_X_Event_Mouse_In));
	if (!e) return;
	e->modifiers = xevent->xcrossing.state;
	e->x = xevent->xcrossing.x;
	e->y = xevent->xcrossing.y;
	e->root.x = xevent->xcrossing.x_root;
	e->root.y = xevent->xcrossing.y_root;
	if (xevent->xcrossing.subwindow) e->win = xevent->xcrossing.subwindow;
	else e->win = xevent->xcrossing.window;
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
	Ecore_X_Event_Mouse_Move *e;

	e = calloc(1, sizeof(Ecore_X_Event_Mouse_Move));
	if (!e) return;
	e->modifiers = xevent->xcrossing.state;
	e->x = xevent->xcrossing.x;
	e->y = xevent->xcrossing.y;
	e->root.x = xevent->xcrossing.x_root;
	e->root.y = xevent->xcrossing.y_root;
	if (xevent->xcrossing.subwindow) e->win = xevent->xcrossing.subwindow;
	else e->win = xevent->xcrossing.window;
	e->same_screen = xevent->xcrossing.same_screen;
	e->root_win = xevent->xcrossing.root;
	e->event_win = xevent->xcrossing.window;
	e->time = xevent->xcrossing.time;
	_ecore_x_event_last_time = e->time;
	_ecore_x_event_last_win = e->win;
	_ecore_x_event_last_root_x = e->root.x;
	_ecore_x_event_last_root_y = e->root.y;
	ecore_event_add(ECORE_X_EVENT_MOUSE_MOVE, e, NULL, NULL);
     }
     {
	Ecore_X_Event_Mouse_Out *e;

	e = calloc(1, sizeof(Ecore_X_Event_Mouse_Out));
	if (!e) return;
	e->modifiers = xevent->xcrossing.state;
	e->x = xevent->xcrossing.x;
	e->y = xevent->xcrossing.y;
	e->root.x = xevent->xcrossing.x_root;
	e->root.y = xevent->xcrossing.y_root;
	if (xevent->xcrossing.subwindow) e->win = xevent->xcrossing.subwindow;
	else e->win = xevent->xcrossing.window;
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
   if (_ecore_x_ic)
     {
	char *str;

	XSetICValues(_ecore_x_ic, XNFocusWindow, xevent->xfocus.window, NULL);
	if ((str = XmbResetIC(_ecore_x_ic)))
	  XFree(str);
	XSetICFocus(_ecore_x_ic);
     }
   e = calloc(1, sizeof(Ecore_X_Event_Window_Focus_In));
   if (!e) return;
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
   if (_ecore_x_ic) XUnsetICFocus(_ecore_x_ic);
   e = calloc(1, sizeof(Ecore_X_Event_Window_Focus_Out));
   if (!e) return;
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
_ecore_x_event_handle_keymap_notify(XEvent *xevent __UNUSED__)
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
   if (!e) return;
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
   if (!e) return;
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
      if (!e) return;
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
   if (!e) return;
   e->win = xevent->xcreatewindow.window;
   if (xevent->xcreatewindow.override_redirect)
      e->override = 1;
   else
      e->override = 0;
   e->time = _ecore_x_event_last_time;
   ecore_event_add(ECORE_X_EVENT_WINDOW_CREATE, e, NULL, NULL);
}

void
_ecore_x_event_handle_destroy_notify(XEvent *xevent)
{
   Ecore_X_Event_Window_Destroy *e;

   _ecore_x_last_event_mouse_move = 0;
   e = calloc(1, sizeof(Ecore_X_Event_Window_Destroy));
   if (!e) return;
   e->win =  xevent->xdestroywindow.window;
   e->event_win = xevent->xdestroywindow.event;
   e->time = _ecore_x_event_last_time;
   if (e->win == _ecore_x_event_last_win) _ecore_x_event_last_win = 0;
   ecore_event_add(ECORE_X_EVENT_WINDOW_DESTROY, e, NULL, NULL);   
}

void
_ecore_x_event_handle_unmap_notify(XEvent *xevent)
{
   Ecore_X_Event_Window_Hide *e;

   _ecore_x_last_event_mouse_move = 0;
   e = calloc(1, sizeof(Ecore_X_Event_Window_Hide));
   if (!e) return;
   e->win = xevent->xunmap.window;
   e->event_win = xevent->xunmap.event;
   e->time = _ecore_x_event_last_time;
   ecore_event_add(ECORE_X_EVENT_WINDOW_HIDE, e, NULL, NULL);
}

void
_ecore_x_event_handle_map_notify(XEvent *xevent)
{
   Ecore_X_Event_Window_Show *e;

   _ecore_x_last_event_mouse_move = 0;
   e = calloc(1, sizeof(Ecore_X_Event_Window_Show));
   if (!e) return;
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
   if (!e) return;
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
   if (!e) return;
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
   if (!e) return;
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
   if (!e) return;
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
_ecore_x_event_handle_gravity_notify(XEvent *xevent __UNUSED__)
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
   if (!e) return;
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
   if (!e) return;
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
   if (!e) return;
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
#if 0 /* for now i disabled this. nice idea though this is - it leaves a lot
       * to be desired for efficiency that is better left to the app layer
       */
   if (xevent->xproperty.atom == ECORE_X_ATOM_WM_CLASS)
     {
	Ecore_X_Event_Window_Prop_Name_Class_Change *e;

	e = calloc(1, sizeof(Ecore_X_Event_Window_Prop_Name_Class_Change));
	if (!e) return;
	ecore_x_window_prop_name_class_get(xevent->xproperty.window, 
					   &(e->name), &(e->clas));
        e->time = xevent->xproperty.time;
        _ecore_x_event_last_time = e->time;
	ecore_event_add(ECORE_X_EVENT_WINDOW_PROP_NAME_CLASS_CHANGE, e, 
                        _ecore_x_event_free_window_prop_name_class_change, NULL);
     }
   else if ((xevent->xproperty.atom == ECORE_X_ATOM_WM_NAME) || 
            (xevent->xproperty.atom == ECORE_X_ATOM_NET_WM_NAME))
     {
	Ecore_X_Event_Window_Prop_Title_Change *e;

	e = calloc(1, sizeof(Ecore_X_Event_Window_Prop_Title_Change));
	if (!e) return;
	e->title = ecore_x_window_prop_title_get(xevent->xproperty.window);
        e->time = xevent->xproperty.time;
        _ecore_x_event_last_time = e->time;
	ecore_event_add(ECORE_X_EVENT_WINDOW_PROP_TITLE_CHANGE, e, 
                        _ecore_x_event_free_window_prop_title_change, NULL);
     }
   else if (xevent->xproperty.atom == ECORE_X_ATOM_NET_WM_VISIBLE_NAME)
     {
	Ecore_X_Event_Window_Prop_Visible_Title_Change *e;

	e = calloc(1, sizeof(Ecore_X_Event_Window_Prop_Visible_Title_Change));
	if (!e) return;
	e->title = ecore_x_window_prop_visible_title_get(xevent->xproperty.window);
        e->time = xevent->xproperty.time;
        _ecore_x_event_last_time = e->time;
	ecore_event_add(ECORE_X_EVENT_WINDOW_PROP_VISIBLE_TITLE_CHANGE, e, 
                        _ecore_x_event_free_window_prop_visible_title_change, NULL);
     }
   else if ((xevent->xproperty.atom == ECORE_X_ATOM_WM_ICON_NAME) || 
            (xevent->xproperty.atom == ECORE_X_ATOM_NET_WM_ICON_NAME))
     {
	Ecore_X_Event_Window_Prop_Icon_Name_Change *e;

	e = calloc(1, sizeof(Ecore_X_Event_Window_Prop_Icon_Name_Change));
	if (!e) return;
	e->name = ecore_x_window_prop_icon_name_get(xevent->xproperty.window);
        e->time = xevent->xproperty.time;
        _ecore_x_event_last_time = e->time;
	ecore_event_add(ECORE_X_EVENT_WINDOW_PROP_ICON_NAME_CHANGE, e, 
                        _ecore_x_event_free_window_prop_icon_name_change, NULL);
     }
   else if (xevent->xproperty.atom == ECORE_X_ATOM_NET_WM_VISIBLE_ICON_NAME)
     {
	Ecore_X_Event_Window_Prop_Visible_Icon_Name_Change *e;

	e = calloc(1, sizeof(Ecore_X_Event_Window_Prop_Visible_Icon_Name_Change));
	if (!e) return;
	e->name = ecore_x_window_prop_visible_icon_name_get(xevent->xproperty.window);
        e->time = xevent->xproperty.time;
        _ecore_x_event_last_time = e->time;
	ecore_event_add(ECORE_X_EVENT_WINDOW_PROP_VISIBLE_ICON_NAME_CHANGE, e, 
                        _ecore_x_event_free_window_prop_visible_icon_name_change, NULL);
     }
   else if (xevent->xproperty.atom == ECORE_X_ATOM_WM_CLIENT_MACHINE)
     {
	Ecore_X_Event_Window_Prop_Client_Machine_Change *e;

	e = calloc(1, sizeof(Ecore_X_Event_Window_Prop_Client_Machine_Change));
	if (!e) return;
	e->name = ecore_x_window_prop_client_machine_get(xevent->xproperty.window);
        e->time = xevent->xproperty.time;
        _ecore_x_event_last_time = e->time;
	ecore_event_add(ECORE_X_EVENT_WINDOW_PROP_CLIENT_MACHINE_CHANGE, e, 
                        _ecore_x_event_free_window_prop_client_machine_change, NULL);
     }
   else if (xevent->xproperty.atom == ECORE_X_ATOM_NET_WM_PID)
     {
	Ecore_X_Event_Window_Prop_Pid_Change *e;

	e = calloc(1, sizeof(Ecore_X_Event_Window_Prop_Pid_Change));
	if (!e) return;
	e->pid = ecore_x_window_prop_pid_get(xevent->xproperty.window);
        e->time = xevent->xproperty.time;
        _ecore_x_event_last_time = e->time;
	ecore_event_add(ECORE_X_EVENT_WINDOW_PROP_PID_CHANGE, e, NULL, NULL);
     }
   else if (xevent->xproperty.atom == ECORE_X_ATOM_NET_WM_DESKTOP)
     {
	Ecore_X_Event_Window_Prop_Desktop_Change *e;

	e = calloc(1, sizeof(Ecore_X_Event_Window_Prop_Desktop_Change));
	if (!e) return;
	e->desktop = ecore_x_window_prop_desktop_get(xevent->xproperty.window);
	ecore_event_add(ECORE_X_EVENT_WINDOW_PROP_PID_CHANGE, e, NULL, NULL);
     }
   else 
#endif     
   {
      Ecore_X_Event_Window_Property *e;

      e = calloc(1,sizeof(Ecore_X_Event_Window_Property));
      if (!e) return;
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
   Ecore_X_Selection_Intern *d;
   Ecore_X_Event_Selection_Clear *e;
   Ecore_X_Atom sel;

   _ecore_x_last_event_mouse_move = 0;
   if (!(d = _ecore_x_selection_get(xevent->xselectionclear.selection)))
     return;
   if (xevent->xselectionclear.time > d->time)
     {
	_ecore_x_selection_set(None, NULL, 0, 
			       xevent->xselectionclear.selection);
     }

   /* Generate event for app cleanup */
   e = malloc(sizeof(Ecore_X_Event_Selection_Clear));
   e->win = xevent->xselectionclear.window;
   e->time = xevent->xselectionclear.time;
   sel = xevent->xselectionclear.selection;
   if (sel == ECORE_X_ATOM_SELECTION_PRIMARY)
     e->selection = ECORE_X_SELECTION_PRIMARY;
   else if (sel == ECORE_X_ATOM_SELECTION_SECONDARY)
     e->selection = ECORE_X_SELECTION_SECONDARY;
   else
     e->selection = ECORE_X_SELECTION_CLIPBOARD;
   ecore_event_add(ECORE_X_EVENT_SELECTION_CLEAR, e, NULL, NULL);
}

void
_ecore_x_event_handle_selection_request(XEvent *xevent)
{
   Ecore_X_Event_Selection_Request *e;
   Ecore_X_Selection_Intern *sd;
   void *data;

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
	     Ecore_X_Atom property;

	     if (!ecore_x_selection_convert(xevent->xselectionrequest.selection,
					    xevent->xselectionrequest.target,
					    &data))
	       {
		  /* Refuse selection, conversion to requested target failed */
		  property = None;
	       }
	     else
	       {
		  /* FIXME: This does not properly handle large data transfers */
		  ecore_x_window_prop_property_set(xevent->xselectionrequest.requestor,
						   xevent->xselectionrequest.property,
						   xevent->xselectionrequest.target,
						   8, data, sd->length);
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
   return;
}

void
_ecore_x_event_handle_selection_notify(XEvent *xevent)
{
   Ecore_X_Event_Selection_Notify *e;
   unsigned char *data = NULL;
   Ecore_X_Atom selection;
   int num_ret, format;

   _ecore_x_last_event_mouse_move = 0;
   selection = xevent->xselection.selection;

   if (xevent->xselection.target == ECORE_X_ATOM_SELECTION_TARGETS)
     {
	format = ecore_x_window_prop_property_get(xevent->xselection.requestor,
						xevent->xselection.property,
						XA_ATOM, 32, &data, &num_ret);
	if (!format) return;
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
   if (!e) return;
   e->win = xevent->xselection.requestor;
   e->time = xevent->xselection.time;
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
     {
	free(e);
	return;
     }
   e->data = _ecore_x_selection_parse(e->target, data, num_ret, format);

   ecore_event_add(ECORE_X_EVENT_SELECTION_NOTIFY, e, 
                   _ecore_x_event_free_selection_notify, NULL);
}

void
_ecore_x_event_handle_colormap_notify(XEvent *xevent)
{
   Ecore_X_Event_Window_Colormap *e;

   _ecore_x_last_event_mouse_move = 0;
   e = calloc(1,sizeof(Ecore_X_Event_Window_Colormap));
   if (!e) return;
   e->win = xevent->xcolormap.window;
   e->cmap = xevent->xcolormap.colormap;
   e->time = _ecore_x_event_last_time;
   if (xevent->xcolormap.state == ColormapInstalled)
      e->installed = 1;
   else
      e->installed = 0;
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
	if (!e) return;
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
	if (!e) return;
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
	unsigned long three;

	e = calloc(1, sizeof(Ecore_X_Event_Xdnd_Enter));
	if (!e) return;

	target = _ecore_x_dnd_target_get();
	target->state = ECORE_X_DND_TARGET_ENTERED;
	target->source = xevent->xclient.data.l[0];
	target->win = xevent->xclient.window;
	target->version = (int) (xevent->xclient.data.l[1] >> 24);
	if (target->version > ECORE_X_DND_VERSION)
	  {
	     printf("DND: Requested version %d, we only support up to %d\n", 
                    target->version, ECORE_X_DND_VERSION);
	     return;
	  }

	if ((three = xevent->xclient.data.l[1] & 0x1UL))
	  {
	     /* source supports more than 3 types, fetch property */
	     unsigned char *data;
	     Ecore_X_Atom *types;
	     int i, num_ret;

	     if (!(ecore_x_window_prop_property_get(target->source, 
						    ECORE_X_ATOM_XDND_TYPE_LIST,
						    XA_ATOM,
						    32, &data, &num_ret)))
	       {
		  printf("DND: Could not fetch data type list from source window, aborting.\n");
		  return;
	       }
	     types = (Ecore_X_Atom *)data;
	     e->types = calloc(num_ret, sizeof(char *));
	     if (e->types)
	       {
		  for (i = 0; i < num_ret; i++)
		    e->types[i] = XGetAtomName(_ecore_x_disp, types[i]);
	       }
	     e->num_types = num_ret;
	  }
	else
	  {
	     int i = 0;

	     e->types = calloc(3, sizeof(char *));
	     if (e->types)
	       {
		  while ((i < 3) && (xevent->xclient.data.l[i + 2]))
		    {
		       e->types[i] = XGetAtomName(_ecore_x_disp, xevent->xclient.data.l[i + 2]);
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

	source = _ecore_x_dnd_source_get();
	/* Make sure source/target match */
	if ((source->win != xevent->xclient.window ) ||
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
	int completed = 1;

	source = _ecore_x_dnd_source_get();
	/* Match source/target */
	if ((source->win != xevent->xclient.window) ||
	    (source->dest != (Window)xevent->xclient.data.l[0]))
	  return;

	if ((source->version < 5) || (xevent->xclient.data.l[1] & 0x1UL))
	  {
	     /* Target successfully performed drop action */
	     ecore_x_selection_xdnd_clear();
	     source->state = ECORE_X_DND_SOURCE_IDLE;
	  }
	else if (source->version >= 5)
	  {
	       completed = 0;
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
	e->state[0] = _ecore_x_netwm_state_get(xevent->xclient.data.l[1]);
	if (e->state[0] == ECORE_X_WINDOW_STATE_UNKNOWN)
	  {
	     char *name;

	     name = XGetAtomName(_ecore_x_disp, xevent->xclient.data.l[1]);
	     if (name) printf("Unknown state: %s\n", name);
	     XFree(name);
	  }
	e->state[1] = _ecore_x_netwm_state_get(xevent->xclient.data.l[2]);
	if (e->state[1] == ECORE_X_WINDOW_STATE_UNKNOWN)
	  {
	     char *name;

	     name = XGetAtomName(_ecore_x_disp, xevent->xclient.data.l[2]);
	     if (name) printf("Unknown state: %s\n", name);
	     XFree(name);
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
	if (!e) return;
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
	if (!e) return;
	e->win = xevent->xclient.window;
	e->desk = xevent->xclient.data.l[0];
	e->source = xevent->xclient.data.l[1];

	ecore_event_add(ECORE_X_EVENT_DESKTOP_CHANGE, e, NULL, NULL);
     }
   else if ((xevent->xclient.message_type == ECORE_X_ATOM_NET_REQUEST_FRAME_EXTENTS))
     {
	Ecore_X_Event_Frame_Extents_Request *e;

	e = calloc(1, sizeof(Ecore_X_Event_Frame_Extents_Request));
	if (!e) return;
	e->win = xevent->xclient.window;

	ecore_event_add(ECORE_X_EVENT_FRAME_EXTENTS_REQUEST, e, NULL, NULL);
     }
   else if ((xevent->xclient.message_type == ECORE_X_ATOM_WM_PROTOCOLS)
	    && ((Ecore_X_Atom)xevent->xclient.data.l[0] == ECORE_X_ATOM_NET_WM_PING)
	    && (xevent->xclient.format == 32))
     {
	Ecore_X_Event_Ping *e;

	e = calloc(1, sizeof(Ecore_X_Event_Ping));
	if (!e) return;
	e->win = xevent->xclient.window;
	e->time = xevent->xclient.data.l[1];
	e->event_win = xevent->xclient.data.l[2];

	ecore_event_add(ECORE_X_EVENT_PING, e, NULL, NULL);
     }
   else if ((xevent->xclient.message_type == ECORE_X_ATOM_NET_STARTUP_INFO_BEGIN) &&
	    (xevent->xclient.format == 8))
     {
	_ecore_x_netwm_startup_info_begin(xevent->xclient.window, xevent->xclient.data.b);
     }
   else if ((xevent->xclient.message_type == ECORE_X_ATOM_NET_STARTUP_INFO) &&
	    (xevent->xclient.format == 8))
     {
	_ecore_x_netwm_startup_info(xevent->xclient.window, xevent->xclient.data.b);
     }
   else if ((xevent->xclient.message_type == 27777)
	    && (xevent->xclient.data.l[0] == 0x7162534)
	    && (xevent->xclient.format == 32)
	    && (xevent->xclient.window == _ecore_x_private_win))
     {
	/* a grab sync marker */
	if (xevent->xclient.data.l[1] == 0x10000001)
	  _ecore_x_window_grab_remove(xevent->xclient.data.l[2]);
	else if (xevent->xclient.data.l[1] == 0x10000002)
	  _ecore_x_key_grab_remove(xevent->xclient.data.l[2]);
     }
   else
     {
	Ecore_X_Event_Client_Message *e;
	int i;

	e = calloc(1, sizeof(Ecore_X_Event_Client_Message));
	if (!e) return;
	e->win = xevent->xclient.window;
	e->message_type = xevent->xclient.message_type;
	e->format = xevent->xclient.format;
	for (i = 0; i < 5; i++) 
	  e->data.l[i] = xevent->xclient.data.l[i];

	ecore_event_add(ECORE_X_EVENT_CLIENT_MESSAGE, e, NULL, NULL);
     }
}

void
_ecore_x_event_handle_mapping_notify(XEvent *xevent __UNUSED__)
{
   _ecore_x_last_event_mouse_move = 0;
   /* FIXME: handle this event type */
}

void
_ecore_x_event_handle_shape_change(XEvent *xevent)
{
   XShapeEvent *shape_event;
   Ecore_X_Event_Window_Shape *e;

   _ecore_x_last_event_mouse_move = 0;
   shape_event = (XShapeEvent *)xevent;
   e = calloc(1, sizeof(Ecore_X_Event_Window_Shape));
   if (!e) return;
   e->win = shape_event->window;
   e->time = shape_event->time;
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
   if (!e) return;
   e->win = screensaver_event->window;
   if (screensaver_event->state == ScreenSaverOn)
     e->on = 1;
  else 
     e->on = 0;
   e->time = screensaver_event->time;
   ecore_event_add(ECORE_X_EVENT_SCREENSAVER_NOTIFY, e, NULL, NULL);
#else
   xevent = NULL;
#endif   
}

void
_ecore_x_event_handle_sync_counter(XEvent *xevent)
{
   XSyncCounterNotifyEvent *sync_counter_event;
   Ecore_X_Event_Sync_Counter *e;

   _ecore_x_last_event_mouse_move = 0;
   sync_counter_event = (XSyncCounterNotifyEvent *)xevent;
   e = calloc(1, sizeof(Ecore_X_Event_Sync_Counter));
   if (!e) return;
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
   if (!e) return;
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
     printf("ERROR: Can't update RR config!\n");

   e = calloc(1, sizeof(Ecore_X_Event_Screen_Change));
   if (!e) return;
   e->win = randr_event->window;
   e->root = randr_event->root;
   e->width = randr_event->width;
   e->height = randr_event->height;
   ecore_event_add(ECORE_X_EVENT_SCREEN_CHANGE, e, NULL, NULL);
}
#endif

#ifdef ECORE_XFIXES
void
_ecore_x_event_handle_fixes_selection_notify(XEvent *event)
{
   _ecore_x_last_event_mouse_move = 0;
   /* Nothing here yet */
   event = NULL;
}
#endif

#ifdef ECORE_XDAMAGE
void
_ecore_x_event_handle_damage_notify(XEvent *event)
{
   XDamageNotifyEvent *damage_event;
   Ecore_X_Event_Damage *e;

   _ecore_x_last_event_mouse_move = 0;
   damage_event = (XDamageNotifyEvent *)event;

   e = calloc(1, sizeof(Ecore_X_Event_Damage));
   if (!e) return;

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
#endif
