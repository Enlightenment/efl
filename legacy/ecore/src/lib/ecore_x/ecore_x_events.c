#include "Ecore.h"
#include "ecore_x_private.h"
#include "Ecore_X.h"
#include "Ecore_Txt.h"

static void _ecore_x_event_free_window_prop_name_class_change(void *data, void *ev);
static void _ecore_x_event_free_window_prop_title_change(void *data, void *ev);
static void _ecore_x_event_free_window_prop_visible_title_change(void *data, void *ev);
static void _ecore_x_event_free_window_prop_icon_name_change(void *data, void *ev);
static void _ecore_x_event_free_window_prop_visible_icon_name_change(void *data, void *ev);
static void _ecore_x_event_free_window_prop_client_machine_change(void *data, void *ev);
static void _ecore_x_event_free_window_prop_pid_change(void *data, void *ev);
static void _ecore_x_event_free_window_prop_desktop_change(void *data, void *ev);
static void _ecore_x_event_free_key_down(void *data, void *ev);
static void _ecore_x_event_free_key_up(void *data, void *ev);
static void _ecore_x_event_free_generic(void *data, void *ev);

void
ecore_x_event_mask_set(Ecore_X_Window w, long mask)
{
   XSelectInput(_ecore_x_disp, w, mask);
}

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

static void
_ecore_x_event_free_window_prop_pid_change(void *data, void *ev)
{
   Ecore_X_Event_Window_Prop_Pid_Change *e;
   
   e = ev;
   free(e);
}

static void
_ecore_x_event_free_window_prop_desktop_change(void *data, void *ev)
{
   Ecore_X_Event_Window_Prop_Desktop_Change *e;
   
   e = ev;
   free(e);
}

static void
_ecore_x_event_free_key_down(void *data, void *ev)
{
   Ecore_X_Event_Key_Down *e;

   e = ev;
   if (e->keyname) free(e->keyname);
   if (e->keysymbol) free(e->keysymbol);
   if (e->key_compose) free(e->key_compose);
   free(e);
}

static void
_ecore_x_event_free_key_up(void *data, void *ev)
{
   Ecore_X_Event_Key_Up *e;

   e = ev;
   if (e->keyname) free(e->keyname);
   if (e->keysymbol) free(e->keysymbol);
   if (e->key_compose) free(e->key_compose);
   free(e);
}

static void
_ecore_x_event_free_selection_notify(void *data, void *ev)
{
   Ecore_X_Event_Selection_Notify *e;

   e = ev;
   if (e->target) free(e->target);
   free(e);
}

static void
_ecore_x_event_free_generic(void *data, void *ev)
{
   free(ev);
}


void
_ecore_x_event_handle_key_press(XEvent *xevent)
{
   Ecore_X_Event_Key_Down *e;
   char                   *keyname;
   int                     val;
   char                    buf[256];
   KeySym                  sym;
   XComposeStatus          stat;
   
   e = calloc(1, sizeof(Ecore_X_Event_Key_Down));
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
   val = XLookupString((XKeyEvent *)xevent, buf, sizeof(buf), &sym, &stat);
   if (val > 0)
     {
	buf[val] = 0;
	e->key_compose = ecore_txt_convert("LATIN1", "UTF-8", buf);
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
   _ecore_x_event_last_time = e->time;
   ecore_event_add(ECORE_X_EVENT_KEY_DOWN, e, _ecore_x_event_free_key_down, NULL);
}

void
_ecore_x_event_handle_key_release(XEvent *xevent)
{
   Ecore_X_Event_Key_Up *e;
   char                   *keyname;
   int                     val;
   char                    buf[256];
   KeySym                  sym;
   XComposeStatus          stat;
   
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
   val = XLookupString((XKeyEvent *)xevent, buf, sizeof(buf), &sym, &stat);
   if (val > 0)
     {
	buf[val] = 0;
	e->key_compose = ecore_txt_convert("LATIN1", "UTF-8", buf);
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
   _ecore_x_event_last_time = e->time;
   ecore_event_add(ECORE_X_EVENT_KEY_UP, e, _ecore_x_event_free_key_up, NULL);
}

void
_ecore_x_event_handle_button_press(XEvent *xevent)
{
   static Window last_win = 0;
   static Window last_last_win = 0;
   static Window last_event_win = 0;
   static Window last_last_event_win = 0;
   static Time last_time = 0;
   static Time last_last_time = 0;
   int did_triple = 0;

   if ((xevent->xbutton.button > 3) && (xevent->xbutton.button < 6))
     {
         Ecore_X_Event_Mouse_Wheel *e;

         e = malloc(sizeof(Ecore_X_Event_Mouse_Wheel));

         if (!e)
            return;
         
	e->direction = 0;
	e->z = 0;
	if      (xevent->xbutton.button == 4) e->z = -1;
	else if (xevent->xbutton.button == 5) e->z = 1;
	e->x = xevent->xbutton.x;
	e->y = xevent->xbutton.y;
	e->root.x = xevent->xbutton.x_root;
	e->root.y = xevent->xbutton.y_root;

         if (xevent->xbutton.subwindow)
            e->win = xevent->xbutton.subwindow;
         else
            e->win = xevent->xbutton.window;
		 
         e->event_win = xevent->xbutton.window;
         e->time = xevent->xbutton.time;
         _ecore_x_event_last_time = e->time;
         _ecore_x_event_last_win = e->win;
         _ecore_x_event_last_root_x = e->root.x;
         _ecore_x_event_last_root_y = e->root.y;
         ecore_event_add(ECORE_X_EVENT_MOUSE_WHEEL, e, _ecore_x_event_free_generic, NULL);
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
	     e->event_win = xevent->xbutton.window;
	     e->time = xevent->xbutton.time;
	     _ecore_x_event_last_time = e->time;
	     _ecore_x_event_last_win = e->win;
	     _ecore_x_event_last_root_x = e->root.x;
	     _ecore_x_event_last_root_y = e->root.y;
	     ecore_event_add(ECORE_X_EVENT_MOUSE_MOVE, e, _ecore_x_event_free_generic, NULL);
	  }
	  {
	     Ecore_X_Event_Mouse_Button_Down *e;
	     
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
	     e->event_win = xevent->xbutton.window;
	     e->time = xevent->xbutton.time;
	     if (((e->time - last_time) <= 
		  (int)(1000 * _ecore_x_double_click_time)) &&
		 (e->win == last_win) &&
		 (e->event_win == last_event_win))
	       e->double_click = 1;
	     if (((e->time - last_last_time) <= 
		  (int)(2 * 1000 * _ecore_x_double_click_time)) &&
		 (e->win == last_win) && (e->win == last_last_win) &&
		 (e->event_win == last_event_win) && (e->event_win == last_last_event_win))
	       {
		  did_triple = 1;
		  e->triple_click = 1;
	       }
	     _ecore_x_event_last_time = e->time;
	     _ecore_x_event_last_win = e->win;
	     _ecore_x_event_last_root_x = e->root.x;
	     _ecore_x_event_last_root_y = e->root.y;
	     ecore_event_add(ECORE_X_EVENT_MOUSE_BUTTON_DOWN, e, _ecore_x_event_free_generic, NULL);
	  }
	if (did_triple)
	  {
	     last_win = 0;
	     last_last_win = 0;
	     last_event_win = 0;
	     last_last_event_win = 0;
	     last_time = 0;
	     last_last_time = 0;
	  }
	else
	  {
	     last_last_win = last_win;
	     if (xevent->xbutton.subwindow)
	       last_win = xevent->xbutton.subwindow;
	     else
	       last_win = xevent->xbutton.window;
	     last_last_event_win = last_event_win;
	     last_event_win = xevent->xbutton.window;
	     last_last_time = last_time;
	     last_time = xevent->xbutton.time;
	  }
     }
}

void
_ecore_x_event_handle_button_release(XEvent *xevent)
{
   /* filter out wheel buttons */
   if (xevent->xbutton.button <= 3)
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
	     e->event_win = xevent->xbutton.window;
	     e->time = xevent->xbutton.time;
	     _ecore_x_event_last_time = e->time;
	     _ecore_x_event_last_win = e->win;
	     _ecore_x_event_last_root_x = e->root.x;
	     _ecore_x_event_last_root_y = e->root.y;
	     ecore_event_add(ECORE_X_EVENT_MOUSE_MOVE, e, _ecore_x_event_free_generic, NULL);
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
	     e->event_win = xevent->xbutton.window;
	     e->time = xevent->xbutton.time;
	     _ecore_x_event_last_time = e->time;
	     _ecore_x_event_last_win = e->win;
	     _ecore_x_event_last_root_x = e->root.x;
	     _ecore_x_event_last_root_y = e->root.y;
	     ecore_event_add(ECORE_X_EVENT_MOUSE_BUTTON_UP, e, _ecore_x_event_free_generic, NULL);
	  }
     }
}

void
_ecore_x_event_handle_motion_notify(XEvent *xevent)
{
   Ecore_X_Event_Mouse_Move *e;

   e = calloc(1, sizeof(Ecore_X_Event_Mouse_Move));
   if (!e) return;
   e->modifiers = xevent->xmotion.state;
   e->x = xevent->xmotion.x;
   e->y = xevent->xmotion.y;
   e->root.x = xevent->xmotion.x_root;
   e->root.y = xevent->xmotion.y_root;
   if (xevent->xmotion.subwindow) e->win = xevent->xmotion.subwindow;
   else e->win = xevent->xmotion.window;
   e->event_win = xevent->xmotion.window;
   e->time = xevent->xmotion.time;
   _ecore_x_event_last_time = e->time;
   _ecore_x_event_last_win = e->win;
   _ecore_x_event_last_root_x = e->root.x;
   _ecore_x_event_last_root_y = e->root.y;
   ecore_event_add(ECORE_X_EVENT_MOUSE_MOVE, e, _ecore_x_event_free_generic, NULL);
}

void
_ecore_x_event_handle_enter_notify(XEvent *xevent)
{
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
	e->event_win = xevent->xcrossing.window;
	e->time = xevent->xcrossing.time;
	_ecore_x_event_last_time = e->time;
	_ecore_x_event_last_win = e->win;
	_ecore_x_event_last_root_x = e->root.x;
	_ecore_x_event_last_root_y = e->root.y;
	ecore_event_add(ECORE_X_EVENT_MOUSE_MOVE, e, _ecore_x_event_free_generic, NULL);
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
	e->event_win = xevent->xcrossing.window;
	if      (xevent->xcrossing.mode == NotifyNormal) e->mode = ECORE_X_EVENT_MODE_NORMAL;
	else if (xevent->xcrossing.mode == NotifyGrab)   e->mode = ECORE_X_EVENT_MODE_GRAB;
	else if (xevent->xcrossing.mode == NotifyUngrab) e->mode = ECORE_X_EVENT_MODE_UNGRAB;
	if      (xevent->xcrossing.detail == NotifyAncestor)         e->detail = ECORE_X_EVENT_DETAIL_ANCESTOR;
	else if (xevent->xcrossing.detail == NotifyVirtual)          e->detail = ECORE_X_EVENT_DETAIL_VIRTUAL;
	else if (xevent->xcrossing.detail == NotifyInferior)         e->detail = ECORE_X_EVENT_DETAIL_INFERIOR;
	else if (xevent->xcrossing.detail == NotifyNonlinear)        e->detail = ECORE_X_EVENT_DETAIL_NON_LINEAR;
	else if (xevent->xcrossing.detail == NotifyNonlinearVirtual) e->detail = ECORE_X_EVENT_DETAIL_NON_LINEAR_VIRTUAL;
	e->time = xevent->xcrossing.time;
	_ecore_x_event_last_time = e->time;
	ecore_event_add(ECORE_X_EVENT_MOUSE_IN, e, _ecore_x_event_free_generic, NULL);
     }
}

void
_ecore_x_event_handle_leave_notify(XEvent *xevent)
{
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
	e->event_win = xevent->xcrossing.window;
	e->time = xevent->xcrossing.time;
	_ecore_x_event_last_time = e->time;
	_ecore_x_event_last_win = e->win;
	_ecore_x_event_last_root_x = e->root.x;
	_ecore_x_event_last_root_y = e->root.y;
	ecore_event_add(ECORE_X_EVENT_MOUSE_MOVE, e, _ecore_x_event_free_generic, NULL);
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
	e->event_win = xevent->xcrossing.window;
	if      (xevent->xcrossing.mode == NotifyNormal) e->mode = ECORE_X_EVENT_MODE_NORMAL;
	else if (xevent->xcrossing.mode == NotifyGrab)   e->mode = ECORE_X_EVENT_MODE_GRAB;
	else if (xevent->xcrossing.mode == NotifyUngrab) e->mode = ECORE_X_EVENT_MODE_UNGRAB;
	if      (xevent->xcrossing.detail == NotifyAncestor)         e->detail = ECORE_X_EVENT_DETAIL_ANCESTOR;
	else if (xevent->xcrossing.detail == NotifyVirtual)          e->detail = ECORE_X_EVENT_DETAIL_VIRTUAL;
	else if (xevent->xcrossing.detail == NotifyInferior)         e->detail = ECORE_X_EVENT_DETAIL_INFERIOR;
	else if (xevent->xcrossing.detail == NotifyNonlinear)        e->detail = ECORE_X_EVENT_DETAIL_NON_LINEAR;
	else if (xevent->xcrossing.detail == NotifyNonlinearVirtual) e->detail = ECORE_X_EVENT_DETAIL_NON_LINEAR_VIRTUAL;
	e->time = xevent->xcrossing.time;
	_ecore_x_event_last_time = e->time;
	_ecore_x_event_last_win = e->win;
	_ecore_x_event_last_root_x = e->root.x;
	_ecore_x_event_last_root_y = e->root.y;
	ecore_event_add(ECORE_X_EVENT_MOUSE_OUT, e, _ecore_x_event_free_generic, NULL);
     }
}

void
_ecore_x_event_handle_focus_in(XEvent *xevent)
{
   Ecore_X_Event_Window_Focus_In *e;
	
   e = calloc(1, sizeof(Ecore_X_Event_Window_Focus_In));
   if (!e) return;
   e->win = xevent->xfocus.window;
   if      (xevent->xfocus.mode == NotifyNormal)       e->mode = ECORE_X_EVENT_MODE_NORMAL;
   else if (xevent->xfocus.mode == NotifyWhileGrabbed) e->mode = ECORE_X_EVENT_MODE_WHILE_GRABBED;
   else if (xevent->xfocus.mode == NotifyGrab)         e->mode = ECORE_X_EVENT_MODE_GRAB;
   else if (xevent->xfocus.mode == NotifyUngrab)       e->mode = ECORE_X_EVENT_MODE_UNGRAB;
   if      (xevent->xfocus.detail == NotifyAncestor)         e->detail = ECORE_X_EVENT_DETAIL_ANCESTOR;
   else if (xevent->xfocus.detail == NotifyVirtual)          e->detail = ECORE_X_EVENT_DETAIL_VIRTUAL;
   else if (xevent->xfocus.detail == NotifyInferior)         e->detail = ECORE_X_EVENT_DETAIL_INFERIOR;
   else if (xevent->xfocus.detail == NotifyNonlinear)        e->detail = ECORE_X_EVENT_DETAIL_NON_LINEAR;
   else if (xevent->xfocus.detail == NotifyNonlinearVirtual) e->detail = ECORE_X_EVENT_DETAIL_NON_LINEAR_VIRTUAL;
   else if (xevent->xfocus.detail == NotifyPointer)          e->detail = ECORE_X_EVENT_DETAIL_POINTER;
   else if (xevent->xfocus.detail == NotifyPointerRoot)      e->detail = ECORE_X_EVENT_DETAIL_POINTER_ROOT;
   else if (xevent->xfocus.detail == NotifyDetailNone)       e->detail = ECORE_X_EVENT_DETAIL_DETAIL_NONE;
   e->time = _ecore_x_event_last_time;
   _ecore_x_event_last_time = e->time;
   ecore_event_add(ECORE_X_EVENT_WINDOW_FOCUS_IN, e, _ecore_x_event_free_generic, NULL);
}

void
_ecore_x_event_handle_focus_out(XEvent *xevent)
{
   Ecore_X_Event_Window_Focus_Out *e;
	
   e = calloc(1, sizeof(Ecore_X_Event_Window_Focus_Out));
   if (!e) return;
   e->win = xevent->xfocus.window;
   if      (xevent->xfocus.mode == NotifyNormal)       e->mode = ECORE_X_EVENT_MODE_NORMAL;
   else if (xevent->xfocus.mode == NotifyWhileGrabbed) e->mode = ECORE_X_EVENT_MODE_WHILE_GRABBED;
   else if (xevent->xfocus.mode == NotifyGrab)         e->mode = ECORE_X_EVENT_MODE_GRAB;
   else if (xevent->xfocus.mode == NotifyUngrab)       e->mode = ECORE_X_EVENT_MODE_UNGRAB;
   if      (xevent->xfocus.detail == NotifyAncestor)         e->detail = ECORE_X_EVENT_DETAIL_ANCESTOR;
   else if (xevent->xfocus.detail == NotifyVirtual)          e->detail = ECORE_X_EVENT_DETAIL_VIRTUAL;
   else if (xevent->xfocus.detail == NotifyInferior)         e->detail = ECORE_X_EVENT_DETAIL_INFERIOR;
   else if (xevent->xfocus.detail == NotifyNonlinear)        e->detail = ECORE_X_EVENT_DETAIL_NON_LINEAR;
   else if (xevent->xfocus.detail == NotifyNonlinearVirtual) e->detail = ECORE_X_EVENT_DETAIL_NON_LINEAR_VIRTUAL;
   else if (xevent->xfocus.detail == NotifyPointer)          e->detail = ECORE_X_EVENT_DETAIL_POINTER;
   else if (xevent->xfocus.detail == NotifyPointerRoot)      e->detail = ECORE_X_EVENT_DETAIL_POINTER_ROOT;
   else if (xevent->xfocus.detail == NotifyDetailNone)       e->detail = ECORE_X_EVENT_DETAIL_DETAIL_NONE;
   e->time = _ecore_x_event_last_time;
   _ecore_x_event_last_time = e->time;
   ecore_event_add(ECORE_X_EVENT_WINDOW_FOCUS_OUT, e, _ecore_x_event_free_generic, NULL);
}

void
_ecore_x_event_handle_keymap_notify(XEvent *xevent)
{
   /* FIXME: handle this event type */   
}

void
_ecore_x_event_handle_expose(XEvent *xevent)
{
   Ecore_X_Event_Window_Damage *e;
   
   e = calloc(1, sizeof(Ecore_X_Event_Window_Damage));
   if (!e) return;
   e->win = xevent->xexpose.window;
   e->time = _ecore_x_event_last_time;
   e->x = xevent->xexpose.x;
   e->y = xevent->xexpose.y;
   e->w = xevent->xexpose.width;
   e->h = xevent->xexpose.height;
   ecore_event_add(ECORE_X_EVENT_WINDOW_DAMAGE, e, _ecore_x_event_free_generic, NULL);   
}

void
_ecore_x_event_handle_graphics_expose(XEvent *xevent)
{
   Ecore_X_Event_Window_Damage *e;
   
   e = calloc(1, sizeof(Ecore_X_Event_Window_Damage));
   if (!e) return;
   e->win = xevent->xgraphicsexpose.drawable;
   e->time = _ecore_x_event_last_time;
   e->x = xevent->xgraphicsexpose.x;
   e->y = xevent->xgraphicsexpose.y;
   e->w = xevent->xgraphicsexpose.width;
   e->h = xevent->xgraphicsexpose.height;
   ecore_event_add(ECORE_X_EVENT_WINDOW_DAMAGE, e, _ecore_x_event_free_generic, NULL);   
}

void
_ecore_x_event_handle_visibility_notify(XEvent *xevent)
{
   if (xevent->xvisibility.state != VisibilityPartiallyObscured)
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
      ecore_event_add(ECORE_X_EVENT_WINDOW_VISIBILITY_CHANGE, e, _ecore_x_event_free_generic, NULL);
   }
}

void
_ecore_x_event_handle_create_notify(XEvent *xevent)
{
   Ecore_X_Event_Window_Create *e;

   e = calloc(1, sizeof(Ecore_X_Event_Window_Create));
   e->win = xevent->xcreatewindow.window;
   if (xevent->xcreatewindow.override_redirect)
      e->override = 1;
   else
      e->override = 0;
   e->time = _ecore_x_event_last_time;
   ecore_event_add(ECORE_X_EVENT_WINDOW_CREATE, e, _ecore_x_event_free_generic, NULL);
}

void
_ecore_x_event_handle_destroy_notify(XEvent *xevent)
{
   Ecore_X_Event_Window_Destroy *e;
   
   e = calloc(1, sizeof(Ecore_X_Event_Window_Destroy));
   if (!e) return;
   e->win =  xevent->xdestroywindow.window;
   e->time = _ecore_x_event_last_time;
   if (e->win == _ecore_x_event_last_win) _ecore_x_event_last_win = 0;
   ecore_event_add(ECORE_X_EVENT_WINDOW_DESTROY, e, _ecore_x_event_free_generic, NULL);   
}

void
_ecore_x_event_handle_unmap_notify(XEvent *xevent)
{
   Ecore_X_Event_Window_Hide *e;
   
   e = calloc(1, sizeof(Ecore_X_Event_Window_Hide));
   if (!e) return;
   e->win = xevent->xunmap.window;
   e->time = _ecore_x_event_last_time;
   ecore_event_add(ECORE_X_EVENT_WINDOW_HIDE, e, _ecore_x_event_free_generic, NULL);
}

void
_ecore_x_event_handle_map_notify(XEvent *xevent)
{
   Ecore_X_Event_Window_Show *e;
   
   e = calloc(1, sizeof(Ecore_X_Event_Window_Show));
   if (!e) return;
   e->win = xevent->xmap.window;
   e->time = _ecore_x_event_last_time;
   ecore_event_add(ECORE_X_EVENT_WINDOW_SHOW, e, _ecore_x_event_free_generic, NULL);
}

void
_ecore_x_event_handle_map_request(XEvent *xevent)
{
   Ecore_X_Event_Window_Show_Request *e;
   
   e = calloc(1, sizeof(Ecore_X_Event_Window_Show_Request));
   if (!e) return;
   e->win = xevent->xmaprequest.window;
   e->time = _ecore_x_event_last_time;
   e->parent = xevent->xmaprequest.parent;
   ecore_event_add(ECORE_X_EVENT_WINDOW_SHOW_REQUEST, e, _ecore_x_event_free_generic, NULL);
}

void
_ecore_x_event_handle_reparent_notify(XEvent *xevent)
{
   Ecore_X_Event_Window_Reparent *e;
   
   e = calloc(1, sizeof(Ecore_X_Event_Window_Reparent));
   if (!e) return;
   e->win = xevent->xreparent.window;
   e->parent = xevent->xreparent.parent;
   e->time = _ecore_x_event_last_time;
   ecore_event_add(ECORE_X_EVENT_WINDOW_REPARENT, e, _ecore_x_event_free_generic, NULL);
}

void
_ecore_x_event_handle_configure_notify(XEvent *xevent)
{
   Ecore_X_Event_Window_Configure *e;
   
   e = calloc(1, sizeof(Ecore_X_Event_Window_Configure));
   if (!e) return;
   e->win = xevent->xconfigure.window;
   e->abovewin = xevent->xconfigure.above;
   e->x = xevent->xconfigure.x;
   e->y = xevent->xconfigure.y;
   e->w = xevent->xconfigure.width;
   e->h = xevent->xconfigure.height;
   e->border = xevent->xconfigure.border_width;
   e->override = xevent->xconfigure.override_redirect;
   e->from_wm = xevent->xconfigure.send_event;
   e->time = _ecore_x_event_last_time;
   ecore_event_add(ECORE_X_EVENT_WINDOW_CONFIGURE, e, _ecore_x_event_free_generic, NULL);      
}

void
_ecore_x_event_handle_configure_request(XEvent *xevent)
{
   /* FIXME: handle this event type */
}

void
_ecore_x_event_handle_gravity_notify(XEvent *xevent)
{
   /* FIXME: handle this event type */
}

void
_ecore_x_event_handle_resize_request(XEvent *xevent)
{
   /* FIXME: handle this event type */
}

void
_ecore_x_event_handle_circulate_notify(XEvent *xevent)
{
   /* FIXME: handle this event type */
}

void
_ecore_x_event_handle_circulate_request(XEvent *xevent)
{
   /* FIXME: handle this event type */
}

void
_ecore_x_event_handle_property_notify(XEvent *xevent)
{
   if (xevent->xproperty.atom == _ecore_x_atom_wm_class)
     {
	Ecore_X_Event_Window_Prop_Name_Class_Change *e;
	
	e = calloc(1, sizeof(Ecore_X_Event_Window_Prop_Name_Class_Change));
	if (!e) return;
	ecore_x_window_prop_name_class_get(xevent->xproperty.window, 
					   &(e->name), &(e->clas));
   e->time = xevent->xproperty.time;
   _ecore_x_event_last_time = e->time;
	ecore_event_add(ECORE_X_EVENT_WINDOW_PROP_NAME_CLASS_CHANGE, e, _ecore_x_event_free_window_prop_name_class_change, NULL);
     }
   else if ((xevent->xproperty.atom == _ecore_x_atom_wm_name) || (xevent->xproperty.atom == _ecore_x_atom_net_wm_name))
     {
	Ecore_X_Event_Window_Prop_Title_Change *e;
	
	e = calloc(1, sizeof(Ecore_X_Event_Window_Prop_Title_Change));
	if (!e) return;
	e->title = ecore_x_window_prop_title_get(xevent->xproperty.window);
   e->time = xevent->xproperty.time;
   _ecore_x_event_last_time = e->time;
	ecore_event_add(ECORE_X_EVENT_WINDOW_PROP_TITLE_CHANGE, e, _ecore_x_event_free_window_prop_title_change, NULL);
     }
   else if (xevent->xproperty.atom == _ecore_x_atom_net_wm_visible_name)
     {
	Ecore_X_Event_Window_Prop_Visible_Title_Change *e;
	
	e = calloc(1, sizeof(Ecore_X_Event_Window_Prop_Visible_Title_Change));
	if (!e) return;
	e->title = ecore_x_window_prop_visible_title_get(xevent->xproperty.window);
   e->time = xevent->xproperty.time;
   _ecore_x_event_last_time = e->time;
	ecore_event_add(ECORE_X_EVENT_WINDOW_PROP_VISIBLE_TITLE_CHANGE, e, _ecore_x_event_free_window_prop_visible_title_change, NULL);
     }
   else if ((xevent->xproperty.atom == _ecore_x_atom_wm_icon_name) || (xevent->xproperty.atom == _ecore_x_atom_net_wm_icon_name))
     {
	Ecore_X_Event_Window_Prop_Icon_Name_Change *e;
	
	e = calloc(1, sizeof(Ecore_X_Event_Window_Prop_Icon_Name_Change));
	if (!e) return;
	e->name = ecore_x_window_prop_icon_name_get(xevent->xproperty.window);
   e->time = xevent->xproperty.time;
   _ecore_x_event_last_time = e->time;
	ecore_event_add(ECORE_X_EVENT_WINDOW_PROP_ICON_NAME_CHANGE, e, _ecore_x_event_free_window_prop_icon_name_change, NULL);
     }
   else if (xevent->xproperty.atom == _ecore_x_atom_net_wm_visible_icon_name)
     {
	Ecore_X_Event_Window_Prop_Visible_Icon_Name_Change *e;
	
	e = calloc(1, sizeof(Ecore_X_Event_Window_Prop_Visible_Icon_Name_Change));
	if (!e) return;
	e->name = ecore_x_window_prop_visible_icon_name_get(xevent->xproperty.window);
   e->time = xevent->xproperty.time;
   _ecore_x_event_last_time = e->time;
	ecore_event_add(ECORE_X_EVENT_WINDOW_PROP_VISIBLE_ICON_NAME_CHANGE, e, _ecore_x_event_free_window_prop_visible_icon_name_change, NULL);
     }
   else if (xevent->xproperty.atom == _ecore_x_atom_wm_client_machine)
     {
	Ecore_X_Event_Window_Prop_Client_Machine_Change *e;
	
	e = calloc(1, sizeof(Ecore_X_Event_Window_Prop_Client_Machine_Change));
	if (!e) return;
	e->name = ecore_x_window_prop_client_machine_get(xevent->xproperty.window);
   e->time = xevent->xproperty.time;
   _ecore_x_event_last_time = e->time;
	ecore_event_add(ECORE_X_EVENT_WINDOW_PROP_CLIENT_MACHINE_CHANGE, e, _ecore_x_event_free_window_prop_client_machine_change, NULL);
     }
   else if (xevent->xproperty.atom == _ecore_x_atom_net_wm_pid)
     {
	Ecore_X_Event_Window_Prop_Pid_Change *e;
	
	e = calloc(1, sizeof(Ecore_X_Event_Window_Prop_Pid_Change));
	if (!e) return;
	e->pid = ecore_x_window_prop_pid_get(xevent->xproperty.window);
   e->time = xevent->xproperty.time;
   _ecore_x_event_last_time = e->time;
	ecore_event_add(ECORE_X_EVENT_WINDOW_PROP_PID_CHANGE, e, _ecore_x_event_free_window_prop_pid_change, NULL);
     }
   else if (xevent->xproperty.atom == _ecore_x_atom_net_wm_desktop)
     {
	Ecore_X_Event_Window_Prop_Desktop_Change *e;
	
	e = calloc(1, sizeof(Ecore_X_Event_Window_Prop_Desktop_Change));
	if (!e) return;
	e->desktop = ecore_x_window_prop_desktop_get(xevent->xproperty.window);
	ecore_event_add(ECORE_X_EVENT_WINDOW_PROP_PID_CHANGE, e, _ecore_x_event_free_window_prop_desktop_change, NULL);
     }
   else 
   {
      Ecore_X_Event_Window_Property *e;

      e = calloc(1,sizeof(Ecore_X_Event_Window_Property));
      if (!e) return;
      e->win = xevent->xproperty.window;
      e->atom = xevent->xproperty.atom;
      e->time = xevent->xproperty.time;
      _ecore_x_event_last_time = e->time;
      ecore_event_add(ECORE_X_EVENT_WINDOW_PROPERTY, e, _ecore_x_event_free_generic, NULL);
   }
}

void
_ecore_x_event_handle_selection_clear(XEvent *xevent)
{
   Ecore_X_Selection_Data *d;
   Ecore_X_Event_Selection_Clear *e;
   Atom sel;

   if(!(d = _ecore_x_selection_get(xevent->xselectionclear.selection)))
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
   if (sel == _ecore_x_atom_selection_primary)
      e->selection = ECORE_X_SELECTION_PRIMARY;
   else if (sel == _ecore_x_atom_selection_secondary)
      e->selection = ECORE_X_SELECTION_SECONDARY;
   else
      e->selection = ECORE_X_SELECTION_CLIPBOARD;
   ecore_event_add(ECORE_X_EVENT_SELECTION_CLEAR, e, _ecore_x_event_free_generic, NULL);
   
}

void
_ecore_x_event_handle_selection_request(XEvent *xevent)
{
   Ecore_X_Selection_Data           *sd;
   XSelectionEvent                  xnotify;
   XEvent                           *xev;
   void                             *data;

   xev = calloc(1, sizeof(XEvent));
   
   xnotify.type = SelectionNotify;
   xnotify.display = xevent->xselectionrequest.display;
   xnotify.requestor = xevent->xselectionrequest.requestor;
   xnotify.selection = xevent->xselectionrequest.selection;
   xnotify.target = xevent->xselectionrequest.target;
   xnotify.time = CurrentTime;

   if((sd = _ecore_x_selection_get(xnotify.selection)) 
         && (sd->win == xevent->xselectionrequest.owner))
   {
      if (_ecore_x_selection_convert(xnotify.selection, xnotify.target,
               &data) == -1)
      {
         /* Refuse selection, conversion to requested target failed */
         xnotify.property = None;
      }
      else
      {
         /* FIXME: This does not properly handle large data transfers */
         ecore_x_window_prop_property_set(xevent->xselectionrequest.requestor,
               xevent->xselectionrequest.property,
               xevent->xselectionrequest.target,
               8, data, sd->length);
         xnotify.property = xevent->xselectionrequest.property;
         free(data);
      }
   }
   else
   {
      xnotify.property = None;
      return;
   }
   
   xev->xselection = xnotify;
   XSendEvent(xevent->xselectionrequest.display, 
              xevent->xselectionrequest.requestor, False, 0, xev);
   XFree(xev);
   
}

void
_ecore_x_event_handle_selection_notify(XEvent *xevent)
{
   Ecore_X_Event_Selection_Notify   *e;
   unsigned char                    *data = NULL;
   Atom                             selection;
   int                              num_ret;
   Ecore_X_Selection_Data           sel_data;

   e = calloc(1, sizeof(Ecore_X_Event_Selection_Notify));
   e->win = xevent->xselection.requestor;
   e->time = xevent->xselection.time;
   e->target = _ecore_x_selection_target_get(xevent->xselection.target);
   selection = xevent->xselection.selection;
   if (selection == _ecore_x_atom_selection_primary)
      e->selection = ECORE_X_SELECTION_PRIMARY;
   else if (selection == _ecore_x_atom_selection_secondary)
      e->selection = ECORE_X_SELECTION_SECONDARY;
   else if (selection == _ecore_x_atom_selection_clipboard)
      e->selection = ECORE_X_SELECTION_CLIPBOARD;
   else
   {
      free(e);
      return;
   }

   if (!ecore_x_window_prop_property_get(e->win, xevent->xselection.property,
            AnyPropertyType, 8, &data, &num_ret))
   {
      free(e);
      return;
   }

   sel_data.win = e->win;
   sel_data.selection = selection;
   sel_data.data = data;
   sel_data.length = num_ret;
   _ecore_x_selection_request_data_set(sel_data);
   ecore_event_add(ECORE_X_EVENT_SELECTION_NOTIFY, e, _ecore_x_event_free_selection_notify, NULL);

}

void
_ecore_x_event_handle_colormap_notify(XEvent *xevent)
{
   Ecore_X_Event_Window_Colormap *e;

   e = calloc(1,sizeof(Ecore_X_Event_Window_Colormap));
   e->win = xevent->xcolormap.window;
   e->cmap = xevent->xcolormap.colormap;
   e->time = _ecore_x_event_last_time;
   if (xevent->xcolormap.state == ColormapInstalled)
      e->installed = 1;
   else
      e->installed = 0;
   ecore_event_add(ECORE_X_EVENT_WINDOW_COLORMAP, e, _ecore_x_event_free_generic, NULL);
}

void
_ecore_x_event_handle_client_message(XEvent *xevent)
{
   /* Special client message event handling here. need to put LOTS of if */
   /* checks here and generate synthetic events per special message known */
   /* otherwise generate generic client message event. this would handle*/
   /* netwm, ICCCM, gnomewm, old kde and mwm hint client message protocols */
   if ((xevent->xclient.message_type == _ecore_x_atom_wm_protocols) &&
       (xevent->xclient.format == 32) &&
       (xevent->xclient.data.l[0] == (long)_ecore_x_atom_wm_delete_window))
   {
	   Ecore_X_Event_Window_Delete_Request *e;
	
   	e = calloc(1, sizeof(Ecore_X_Event_Window_Delete_Request));
	   if (!e) return;
   	e->win = xevent->xclient.window;
	   e->time = _ecore_x_event_last_time;
   	ecore_event_add(ECORE_X_EVENT_WINDOW_DELETE_REQUEST, e, _ecore_x_event_free_generic, NULL);
   }
   /* Xdnd Client Message Handling */
   else if (xevent->xclient.message_type == _ecore_x_atom_xdnd_enter)
   {
      Ecore_X_Event_Xdnd_Enter *e;
      Ecore_X_DND_Protocol *_xdnd;
      unsigned long three;
      
      _xdnd = _ecore_x_dnd_protocol_get();
      _xdnd->source = xevent->xclient.data.l[0];
      _xdnd->dest = xevent->xclient.window;
      _xdnd->version = (int) (xevent->xclient.data.l[1] >> 24);
      if (_xdnd->version > ECORE_X_DND_VERSION)
      {
         printf("DND: Requested version %d, we only support up to %d\n", _xdnd->version,
                ECORE_X_DND_VERSION);
         return;
      }
      
      if ((three = xevent->xclient.data.l[1] & 0x1UL))
      {
         /* source supports more than 3 types, fetch property */
         unsigned char *data;
         Atom *types;
         int i, num_ret;
         if (!(ecore_x_window_prop_property_get(_xdnd->source, 
                                              _ecore_x_atom_xdnd_type_list,
                                              XA_ATOM,
                                              32,
                                              &data,
                                              &num_ret)))
         {
            printf("DND: Could not fetch data type list from source window, aborting.\n");
            return;
         }
         types = (Atom *) data;
         _xdnd->types = calloc(num_ret + 1, sizeof(Atom));
         for (i = 0; i < num_ret; i++)
            _xdnd->types[i] = types[i];
         _xdnd->num_types = num_ret;
         free(types);
      }
      else
      {
         _xdnd->types = calloc(4, sizeof(Atom));
         _xdnd->types[0] = xevent->xclient.data.l[2];
         _xdnd->types[1] = xevent->xclient.data.l[3];
         _xdnd->types[2] = xevent->xclient.data.l[4];
         _xdnd->num_types = 3;
      }

      _xdnd->state = ECORE_X_DND_TARGET_ENTERED;

      e = calloc(1, sizeof(Ecore_X_Event_Xdnd_Enter));
      if (!e) return;
      e->win = _xdnd->dest;
      e->source = _xdnd->source;
      e->time = CurrentTime;
      ecore_event_add(ECORE_X_EVENT_XDND_ENTER, e, _ecore_x_event_free_generic, NULL);
   }
   
   else if (xevent->xclient.message_type == _ecore_x_atom_xdnd_position)
   {
      Ecore_X_Event_Xdnd_Position *e;
      Ecore_X_DND_Protocol *_xdnd;

      _xdnd = _ecore_x_dnd_protocol_get();
      _xdnd->source = xevent->xclient.data.l[0];
      _xdnd->dest = xevent->xclient.window;
      _xdnd->pos.x = xevent->xclient.data.l[2] >> 16;
      _xdnd->pos.y = xevent->xclient.data.l[2] & 0xFFFFUL;
      _xdnd->action = xevent->xclient.data.l[4]; /* Version 2 */
      /* TODO: Resolve a suitable method for enumerating Xdnd actions */

      /* Would it be feasible to handle the processing of this message
       * within ecore? I think not, but someone might have an idea here. */

      e = calloc(1, sizeof(Ecore_X_Event_Xdnd_Position));
      if (!e) return;
      e->win = _xdnd->dest;
      e->source = _xdnd->source;
      e->position.x = _xdnd->pos.x;
      e->position.y = _xdnd->pos.y;
      e->time = xevent->xclient.data.l[3]; /* Version 1 */
      e->action = _xdnd->action;
      ecore_event_add(ECORE_X_EVENT_XDND_POSITION, e, _ecore_x_event_free_generic, NULL);
   }
   else if (xevent->xclient.message_type == _ecore_x_atom_xdnd_status)
   {
      Ecore_X_Event_Xdnd_Status *e;
      Ecore_X_DND_Protocol *_xdnd;

      _xdnd = _ecore_x_dnd_protocol_get();
      /* Make sure source/target match */
      if (_xdnd->source != xevent->xclient.window 
            || _xdnd->dest != xevent->xclient.data.l[0])
         return;
      _xdnd->will_accept = xevent->xclient.data.l[1] & 0x1UL;
      _xdnd->suppress = (xevent->xclient.data.l[1] & 0x2UL) ? 0 : 1;

      _xdnd->rectangle.x = xevent->xclient.data.l[2] >> 16;
      _xdnd->rectangle.y = xevent->xclient.data.l[2] & 0xFFFFUL;
      _xdnd->rectangle.width = xevent->xclient.data.l[3] >> 16;
      _xdnd->rectangle.height = xevent->xclient.data.l[3] & 0xFFFFUL;

      _xdnd->accepted_action = xevent->xclient.data.l[4];

      e = calloc(1, sizeof(Ecore_X_Event_Xdnd_Status));
      if (!e) return;
      e->win = _xdnd->source;
      e->target = _xdnd->dest;
      e->will_accept = _xdnd->will_accept;
      e->rectangle.x = _xdnd->rectangle.x;
      e->rectangle.y = _xdnd->rectangle.y;
      e->rectangle.width = _xdnd->rectangle.width;
      e->rectangle.height = _xdnd->rectangle.height;
      e->action = _xdnd->accepted_action;

      ecore_event_add(ECORE_X_EVENT_XDND_STATUS, e, _ecore_x_event_free_generic, NULL);
   }
   else
   {
	/* FIXME: handle this event type */
   }
}

void
_ecore_x_event_handle_mapping_notify(XEvent *xevent)
{
   /* FIXME: handle this event type */
}

void
_ecore_x_event_handle_shape_change(XEvent *xevent)
{
   /* FIXME: handle this event type */
}
