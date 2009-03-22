/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include "Ecore.h"
#include "ecore_xcb_private.h"
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

static Ecore_X_Window _ecore_xcb_mouse_down_last_window = 0;
static Ecore_X_Window _ecore_xcb_mouse_down_last_last_window = 0;
static Ecore_X_Window _ecore_xcb_mouse_down_last_event_window = 0;
static Ecore_X_Window _ecore_xcb_mouse_down_last_last_event_window = 0;
static Ecore_X_Time   _ecore_xcb_mouse_down_last_time = 0;
static Ecore_X_Time   _ecore_xcb_mouse_down_last_last_time = 0;
static int            _ecore_xcb_mouse_up_count = 0;
static int            _ecore_xcb_mouse_down_did_triple = 0;


/* FIXME: roundtrip */
EAPI void
ecore_x_event_mask_set(Ecore_X_Window     window,
                       Ecore_X_Event_Mask mask)
{
   xcb_get_window_attributes_cookie_t cookie;
   xcb_get_window_attributes_reply_t *reply;
   uint32_t                           value_list;

   if (!window)
      window = ((xcb_screen_t *)_ecore_xcb_screen)->root;

   cookie = xcb_get_window_attributes_unchecked(_ecore_xcb_conn, window);
   reply = xcb_get_window_attributes_reply(_ecore_xcb_conn, cookie, NULL);
   if (!reply) return;

   value_list = mask | reply->your_event_mask;
   xcb_change_window_attributes(_ecore_xcb_conn, window, XCB_CW_EVENT_MASK, &value_list);
   free(reply);
}

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
   if (!reply) return;

   value_list = reply->your_event_mask & ~mask;
   xcb_change_window_attributes(_ecore_xcb_conn, window, XCB_CW_EVENT_MASK, &value_list);
   free(reply);
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
_ecore_x_event_free_xdnd_enter(void *data __UNUSED__, void *ev)
{
   Ecore_X_Event_Xdnd_Enter *e;
   int                       i;

   e = ev;
   for (i = 0; i < e->num_types; i++)
     free(e->types[i]);
   free(e->types);
   free(e);
}

static void
_ecore_x_event_free_selection_notify(void *data __UNUSED__, void *ev)
{
   Ecore_X_Event_Selection_Notify *e;
   Ecore_X_Selection_Data         *sel;

   e = ev;
   sel = e->data;
   if (sel->free)
     sel->free(sel);
   free(e->target);
   free(e);
}

/* FIXME: handle this event */
void
_ecore_x_event_handle_key_press(xcb_generic_event_t *event)
{
   xcb_key_press_event_t  *ev;
/*    Ecore_X_Event_Key_Down *e; */
/*    char                   *keyname; */
/*    int                     val; */
/*    char                    buf[256]; */
/*    KeySym                  sym; */
/*    XComposeStatus          status; */

   ev = (xcb_key_press_event_t *)event;
/*    e = calloc(1, sizeof(Ecore_X_Event_Key_Down)); */
/*    if (!e) return; */
/*    keyname = XKeysymToString(XKeycodeToKeysym(xevent->xkey.display,  */
/*					      xevent->xkey.keycode, 0)); */
/*    if (!keyname) */
/*      { */
/*	snprintf(buf, sizeof(buf), "Keycode-%i", xevent->xkey.keycode); */
/*	keyname = buf; */
/*      } */
/*    e->keyname = strdup(keyname); */
/*    if (!e->keyname) */
/*      { */
/*	free(e); */
/*	return; */
/*      } */
/*    val = XLookupString((XKeyEvent *)xevent, buf, sizeof(buf), &sym, &status); */
/*    if (val > 0) */
/*      { */
/*	buf[val] = 0; */
/*	e->key_compose = ecore_txt_convert(nl_langinfo(CODESET), "UTF-8", buf); */
/*      } */
/*    else e->key_compose = NULL; */
/*    keyname = XKeysymToString(sym); */
/*    if (keyname) e->keysymbol = strdup(keyname); */
/*    else e->keysymbol = strdup(e->keyname); */
/*    if (!e->keysymbol) */
/*      { */
/*	if (e->keyname) free(e->keyname); */
/*	if (e->key_compose) free(e->key_compose); */
/*	free(e); */
/*	return; */
/*      } */
/*    if (xevent->xkey.subwindow) e->win = xevent->xkey.subwindow; */
/*    else e->win = xevent->xkey.window; */
/*    e->event_win = xevent->xkey.window; */
/*    e->time = xevent->xkey.time; */
/*    e->modifiers = xevent->xkey.state; */
/*    _ecore_x_event_last_time = e->time; */
/*    ecore_event_add(ECORE_X_EVENT_KEY_DOWN, e, _ecore_x_event_free_key_down, NULL); */
}

/* FIXME: handle this event */
void
_ecore_x_event_handle_key_release(xcb_generic_event_t *event)
{
   xcb_key_release_event_t *ev;
/*    Ecore_X_Event_Key_Up    *e; */
/*    char                   *keyname; */
/*    int                     val; */
/*    char                    buf[256]; */
/*    KeySym                  sym; */
/*    XComposeStatus          status; */

   ev = (xcb_key_release_event_t *)event;
/*    e = calloc(1, sizeof(Ecore_X_Event_Key_Up)); */
/*    if (!e) return; */
/*    keyname = XKeysymToString(XKeycodeToKeysym(xevent->xkey.display,  */
/*					      xevent->xkey.keycode, 0)); */
/*    if (!keyname) */
/*      { */
/*	snprintf(buf, sizeof(buf), "Keycode-%i", xevent->xkey.keycode); */
/*	keyname = buf; */
/*      } */
/*    e->keyname = strdup(keyname); */
/*    if (!e->keyname) */
/*      { */
/*	free(e); */
/*	return; */
/*      } */
/*    val = XLookupString((XKeyEvent *)xevent, buf, sizeof(buf), &sym, &status); */
/*    if (val > 0) */
/*      { */
/*	buf[val] = 0; */
/*	e->key_compose = ecore_txt_convert("ISO8859-1", "UTF-8", buf); */
/*      } */
/*    else e->key_compose = NULL; */
/*    keyname = XKeysymToString(sym); */
/*    if (keyname) e->keysymbol = strdup(keyname); */
/*    else e->keysymbol = strdup(e->keyname); */
/*    if (!e->keysymbol) */
/*      { */
/*	if (e->keyname) free(e->keyname); */
/*	if (e->key_compose) free(e->key_compose); */
/*	free(e); */
/*	return; */
/*      } */
/*    if (xevent->xkey.subwindow) e->win = xevent->xkey.subwindow; */
/*    else e->win = xevent->xkey.window; */
/*    e->event_win = xevent->xkey.window; */
/*    e->time = xevent->xkey.time; */
/*    e->modifiers = xevent->xkey.state; */
/*    _ecore_x_event_last_time = e->time; */
/*    ecore_event_add(ECORE_X_EVENT_KEY_UP, e, _ecore_x_event_free_key_up, NULL); */
}

void
_ecore_x_event_handle_button_press(xcb_generic_event_t *event)
{
   xcb_button_press_event_t *ev;
   int                       i;

   ev = (xcb_button_press_event_t *)event;
   if ((ev->detail > 3) && (ev->detail < 8))
     {
	Ecore_X_Event_Mouse_Wheel *e;

	e = malloc(sizeof(Ecore_X_Event_Mouse_Wheel));

	if (!e)
	  return;

	e->modifiers = ev->state;
	e->direction = 0;
	e->z = 0;
	if      (ev->detail == 4)
	  {
	     e->direction = 0;
	     e->z = -1;
	  }
	else if (ev->detail == 5)
	  {
	     e->direction = 0;
	     e->z = 1;
	  }
	else if (ev->detail == 6)
	  {
	     e->direction = 1;
	     e->z = -1;
	  }
	else if (ev->detail == 7)
	  {
	     e->direction = 1;
	     e->z = 1;
	  }
	e->x = ev->event_x;
	e->y = ev->event_y;
	e->root.x = ev->root_x;
	e->root.y = ev->root_y;

	if (ev->child)
	  e->win = ev->child;
	else
	  e->win = ev->event;

	e->event_win = ev->event;
	e->same_screen = ev->same_screen;
	e->root_win = ev->root;
	e->time = ev->time;
	_ecore_xcb_event_last_time = e->time;
	_ecore_xcb_event_last_window = e->win;
	_ecore_xcb_event_last_root_x = e->root.x;
	_ecore_xcb_event_last_root_y = e->root.y;
	ecore_event_add(ECORE_X_EVENT_MOUSE_WHEEL, e, NULL, NULL);
	for (i = 0; i < _ecore_window_grabs_num; i++)
	  {
	     if ((_ecore_window_grabs[i] == ev->event) ||
		 (_ecore_window_grabs[i] == ev->child))
	       {
		  int replay = 0;

		  if (_ecore_window_grab_replay_func)
		    replay = _ecore_window_grab_replay_func(_ecore_window_grab_replay_data,
							    ECORE_X_EVENT_MOUSE_WHEEL,
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
	     Ecore_X_Event_Mouse_Move *e;

	     e = calloc(1, sizeof(Ecore_X_Event_Mouse_Move));
	     if (!e) return;
	     e->modifiers = ev->state;
	     e->x = ev->event_x;
	     e->y = ev->event_y;
	     e->root.x = ev->root_x;
	     e->root.y = ev->root_y;
	     if (ev->child) e->win = ev->child;
	     else e->win = ev->event;
	     e->same_screen = ev->same_screen;
	     e->root_win = ev->root;
	     e->event_win = ev->event;
	     e->time = ev->time;
	     _ecore_xcb_event_last_time = e->time;
	     _ecore_xcb_event_last_window = e->win;
	     _ecore_xcb_event_last_root_x = e->root.x;
	     _ecore_xcb_event_last_root_y = e->root.y;
	     ecore_event_add(ECORE_X_EVENT_MOUSE_MOVE, e, NULL, NULL);
	  }
	  {
	     Ecore_X_Event_Mouse_Button_Down *e;

            if (_ecore_xcb_mouse_down_did_triple)
              {
                 _ecore_xcb_mouse_down_last_window = 0;
                 _ecore_xcb_mouse_down_last_last_window = 0;
                 _ecore_xcb_mouse_down_last_event_window = 0;
                 _ecore_xcb_mouse_down_last_last_event_window = 0;
                 _ecore_xcb_mouse_down_last_time = 0;
                 _ecore_xcb_mouse_down_last_last_time = 0;
              }

	     e = calloc(1, sizeof(Ecore_X_Event_Mouse_Button_Down));
	     if (!e) return;
	     e->button = ev->detail;
	     e->modifiers = ev->state;
	     e->x = ev->event_x;
	     e->y = ev->event_y;
	     e->root.x = ev->root_x;
	     e->root.y = ev->root_y;
	     if (ev->child) e->win = ev->child;
	     else e->win = ev->event;
	     e->same_screen = ev->same_screen;
	     e->root_win = ev->root;
	     e->event_win = ev->event;
	     e->time = ev->time;
	     if (e->win == e->event_win)
	       {
		  if (((int)(e->time - _ecore_xcb_mouse_down_last_time) <=
		       (int)(1000 * _ecore_xcb_double_click_time)) &&
		      (e->win == _ecore_xcb_mouse_down_last_window) &&
		      (e->event_win == _ecore_xcb_mouse_down_last_event_window)
		      )
		    e->double_click = 1;
		  if (((int)(e->time - _ecore_xcb_mouse_down_last_last_time) <=
		       (int)(2 * 1000 * _ecore_xcb_double_click_time)) &&
		      (e->win == _ecore_xcb_mouse_down_last_window) &&
                      (e->win == _ecore_xcb_mouse_down_last_last_window) &&
		      (e->event_win == _ecore_xcb_mouse_down_last_event_window) &&
                      (e->event_win == _ecore_xcb_mouse_down_last_last_event_window)
		      )
		    {
                        e->triple_click = 1;
		       _ecore_xcb_mouse_down_did_triple = 1;
		    }
                  else
                    _ecore_xcb_mouse_down_did_triple = 0;
	       }
             if (!e->double_click && !e->triple_click)
               _ecore_xcb_mouse_up_count = 0;
	     _ecore_xcb_event_last_time = e->time;
	     _ecore_xcb_event_last_window = e->win;
	     _ecore_xcb_event_last_root_x = e->root.x;
	     _ecore_xcb_event_last_root_y = e->root.y;
	     ecore_event_add(ECORE_X_EVENT_MOUSE_BUTTON_DOWN, e, NULL, NULL);
	     for (i = 0; i < _ecore_window_grabs_num; i++)
	       {
		  if ((_ecore_window_grabs[i] == ev->event) ||
		      (_ecore_window_grabs[i] == ev->child))
		    {
		       int replay = 0;

		       if (_ecore_window_grab_replay_func)
			 replay = _ecore_window_grab_replay_func(_ecore_window_grab_replay_data,
								 ECORE_X_EVENT_MOUSE_BUTTON_DOWN,
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
	     if (e->win == e->event_win)
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
}

void
_ecore_x_event_handle_button_release(xcb_generic_event_t *event)
{
   xcb_button_release_event_t *ev;

   ev = (xcb_button_release_event_t *)event;
   /* filter out wheel buttons */
   if ((ev->detail <= 3) || (ev->detail > 7))
     {
	  {
	     Ecore_X_Event_Mouse_Move *e;

	     e = calloc(1, sizeof(Ecore_X_Event_Mouse_Move));
	     if (!e) return;
	     e->modifiers = ev->state;
	     e->x = ev->event_x;
	     e->y = ev->event_y;
	     e->root.x = ev->root_x;
	     e->root.y = ev->root_y;
	     if (ev->child) e->win = ev->child;
	     else e->win = ev->event;
	     e->same_screen = ev->same_screen;
	     e->root_win = ev->root;
	     e->event_win = ev->event;
	     e->time = ev->time;
	     _ecore_xcb_event_last_time = e->time;
	     _ecore_xcb_event_last_window = e->win;
	     _ecore_xcb_event_last_root_x = e->root.x;
	     _ecore_xcb_event_last_root_y = e->root.y;
	     ecore_event_add(ECORE_X_EVENT_MOUSE_MOVE, e, NULL, NULL);
	  }
	  {
	     Ecore_X_Event_Mouse_Button_Up *e;

	     e = calloc(1, sizeof(Ecore_X_Event_Mouse_Button_Up));
	     if (!e) return;
	     e->button = ev->detail;
	     e->modifiers = ev->state;
	     e->x = ev->event_x;
	     e->y = ev->event_y;
	     e->root.x = ev->root_x;
	     e->root.y = ev->root_y;
	     if (ev->child) e->win = ev->child;
	     else e->win = ev->event;
	     e->same_screen = ev->same_screen;
	     e->root_win = ev->root;
	     e->event_win = ev->event;
	     e->time = ev->time;
             _ecore_xcb_mouse_up_count++;
	     if (e->win == e->event_win)
	       {
		  if ((_ecore_xcb_mouse_up_count >= 2) &&
                      ((int)(e->time - _ecore_xcb_mouse_down_last_time) <=
		       (int)(1000 * _ecore_xcb_double_click_time)) &&
		      (e->win == _ecore_xcb_mouse_down_last_window) &&
		      (e->event_win == _ecore_xcb_mouse_down_last_event_window)
		      )
		    e->double_click = 1;
		  if ((_ecore_xcb_mouse_up_count >= 3) &&
                     ((int)(e->time - _ecore_xcb_mouse_down_last_last_time) <=
		       (int)(2 * 1000 * _ecore_xcb_double_click_time)) &&
		      (e->win == _ecore_xcb_mouse_down_last_window) &&
                      (e->win == _ecore_xcb_mouse_down_last_last_window) &&
		      (e->event_win == _ecore_xcb_mouse_down_last_event_window) &&
                      (e->event_win == _ecore_xcb_mouse_down_last_last_event_window)
		      )
                    e->triple_click = 1;
	       }
	     _ecore_xcb_event_last_time = e->time;
	     _ecore_xcb_event_last_window = e->win;
	     _ecore_xcb_event_last_root_x = e->root.x;
	     _ecore_xcb_event_last_root_y = e->root.y;
	     ecore_event_add(ECORE_X_EVENT_MOUSE_BUTTON_UP, e, NULL, NULL);
	  }
     }
}

void
_ecore_x_event_handle_motion_notify(xcb_generic_event_t *event)
{
   xcb_motion_notify_event_t *ev;
   Ecore_X_Event_Mouse_Move  *e;

   ev = (xcb_motion_notify_event_t *)event;
   e = calloc(1, sizeof(Ecore_X_Event_Mouse_Move));
   if (!e) return;
   e->modifiers = ev->state;
   e->x = ev->event_x;
   e->y = ev->event_y;
   e->root.x = ev->root_x;
   e->root.y = ev->root_y;
   if (ev->child) e->win = ev->child;
   else e->win = ev->event;
   e->same_screen = ev->same_screen;
   e->root_win = ev->root;
   e->event_win = ev->event;
   e->time = ev->time;
   _ecore_xcb_event_last_time = e->time;
   _ecore_xcb_event_last_window = e->win;
   _ecore_xcb_event_last_root_x = e->root.x;
   _ecore_xcb_event_last_root_y = e->root.y;

   /* Xdnd handling */
   _ecore_x_dnd_drag(e->root_win, e->root.x, e->root.y);

   ecore_event_add(ECORE_X_EVENT_MOUSE_MOVE, e, NULL, NULL);
}

void
_ecore_x_event_handle_enter_notify(xcb_generic_event_t *event)
{
   xcb_enter_notify_event_t *ev;

   ev = (xcb_enter_notify_event_t *)event;

     {
	Ecore_X_Event_Mouse_Move *e;

	e = calloc(1, sizeof(Ecore_X_Event_Mouse_Move));
	if (!e) return;
	e->modifiers = ev->state;
	e->x = ev->event_x;
	e->y = ev->event_y;
	e->root.x = ev->root_x;
	e->root.y = ev->root_y;
	if (ev->child) e->win = ev->child;
	else e->win = ev->event;
	e->same_screen = ev->same_screen_focus;
	e->root_win = ev->root;
	e->event_win = ev->event;
	e->time = ev->time;
	_ecore_xcb_event_last_time = e->time;
	_ecore_xcb_event_last_window = e->win;
	_ecore_xcb_event_last_root_x = e->root.x;
	_ecore_xcb_event_last_root_y = e->root.y;
	ecore_event_add(ECORE_X_EVENT_MOUSE_MOVE, e, NULL, NULL);
     }
     {
	Ecore_X_Event_Mouse_In *e;

	e = calloc(1, sizeof(Ecore_X_Event_Mouse_In));
	if (!e) return;
	e->modifiers = ev->state;
	e->x = ev->event_x;
	e->y = ev->event_y;
	e->root.x = ev->root_x;
	e->root.y = ev->root_y;
	if (ev->child) e->win = ev->child;
	else e->win = ev->event;
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
        }
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
        }
	e->time = ev->time;
	_ecore_xcb_event_last_time = e->time;
	ecore_event_add(ECORE_X_EVENT_MOUSE_IN, e, NULL, NULL);
     }
}

void
_ecore_x_event_handle_leave_notify(xcb_generic_event_t *event)
{
   xcb_leave_notify_event_t *ev;

   ev = (xcb_leave_notify_event_t *)event;
     {
	Ecore_X_Event_Mouse_Move *e;

	e = calloc(1, sizeof(Ecore_X_Event_Mouse_Move));
	if (!e) return;
	e->modifiers = ev->state;
	e->x = ev->event_x;
	e->y = ev->event_y;
	e->root.x = ev->root_x;
	e->root.y = ev->root_y;
	if (ev->child) e->win = ev->child;
	else e->win = ev->event;
	e->same_screen = ev->same_screen_focus;
	e->root_win = ev->root;
	e->event_win = ev->event;
	e->time = ev->time;
	_ecore_xcb_event_last_time = e->time;
	_ecore_xcb_event_last_window = e->win;
	_ecore_xcb_event_last_root_x = e->root.x;
	_ecore_xcb_event_last_root_y = e->root.y;
	ecore_event_add(ECORE_X_EVENT_MOUSE_MOVE, e, NULL, NULL);
     }
     {
	Ecore_X_Event_Mouse_Out *e;

	e = calloc(1, sizeof(Ecore_X_Event_Mouse_Out));
	if (!e) return;
	e->modifiers = ev->state;
	e->x = ev->event_x;
	e->y = ev->event_y;
	e->root.x = ev->root_x;
	e->root.y = ev->root_y;
	if (ev->child) e->win = ev->child;
	else e->win = ev->event;
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
        }
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
        }
	e->time = ev->time;
	_ecore_xcb_event_last_time = e->time;
	_ecore_xcb_event_last_window = e->win;
	_ecore_xcb_event_last_root_x = e->root.x;
	_ecore_xcb_event_last_root_y = e->root.y;
	ecore_event_add(ECORE_X_EVENT_MOUSE_OUT, e, NULL, NULL);
     }
}

void
_ecore_x_event_handle_focus_in(xcb_generic_event_t *event)
{
   xcb_focus_in_event_t          *ev;
   Ecore_X_Event_Window_Focus_In *e;

   ev = (xcb_focus_in_event_t *)event;
   e = calloc(1, sizeof(Ecore_X_Event_Window_Focus_In));
   if (!e) return;
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
   }
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
   }
   e->time = _ecore_xcb_event_last_time;
   _ecore_xcb_event_last_time = e->time;
   ecore_event_add(ECORE_X_EVENT_WINDOW_FOCUS_IN, e, NULL, NULL);
}

void
_ecore_x_event_handle_focus_out(xcb_generic_event_t *event)
{
   xcb_focus_out_event_t            *ev;
   Ecore_X_Event_Window_Focus_Out *e;

   ev = (xcb_focus_out_event_t *)event;
   e = calloc(1, sizeof(Ecore_X_Event_Window_Focus_Out));
   if (!e) return;
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
   }
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
   }
   e->time = _ecore_xcb_event_last_time;
   _ecore_xcb_event_last_time = e->time;
   ecore_event_add(ECORE_X_EVENT_WINDOW_FOCUS_OUT, e, NULL, NULL);
}

void
_ecore_x_event_handle_keymap_notify(xcb_generic_event_t *event __UNUSED__)
{
   /* FIXME: handle this event type */
}

void
_ecore_x_event_handle_expose(xcb_generic_event_t *event)
{
   xcb_expose_event_t            *ev;
   Ecore_X_Event_Window_Damage *e;

   ev = (xcb_expose_event_t *)event,
   e = calloc(1, sizeof(Ecore_X_Event_Window_Damage));
   if (!e) return;
   e->win = ev->window;
   e->time = _ecore_xcb_event_last_time;
   e->x = ev->x;
   e->y = ev->y;
   e->w = ev->width;
   e->h = ev->height;
   e->count = ev->count;
   ecore_event_add(ECORE_X_EVENT_WINDOW_DAMAGE, e, NULL, NULL);
}

void
_ecore_x_event_handle_graphics_expose(xcb_generic_event_t *event)
{
   xcb_graphics_exposure_event_t *ev;
   Ecore_X_Event_Window_Damage *e;

   ev = (xcb_graphics_exposure_event_t *)event;
   e = calloc(1, sizeof(Ecore_X_Event_Window_Damage));
   if (!e) return;
   e->win = ev->drawable;
   e->time = _ecore_xcb_event_last_time;
   e->x = ev->x;
   e->y = ev->y;
   e->w = ev->width;
   e->h = ev->height;
   e->count = ev->count;
   ecore_event_add(ECORE_X_EVENT_WINDOW_DAMAGE, e, NULL, NULL);
}

void
_ecore_x_event_handle_visibility_notify(xcb_generic_event_t *event)
{
   xcb_visibility_notify_event_t *ev;

   ev = (xcb_visibility_notify_event_t *)event;
   if (ev->state != XCB_VISIBILITY_PARTIALLY_OBSCURED)
   {
      Ecore_X_Event_Window_Visibility_Change *e;

      e = calloc(1, sizeof(Ecore_X_Event_Window_Visibility_Change));
      if (!e) return;
      e->win = ev->window;
      e->time = _ecore_xcb_event_last_time;
      if (ev->state == XCB_VISIBILITY_FULLY_OBSCURED)
	 e->fully_obscured = 1;
      else
	 e->fully_obscured = 0;
      ecore_event_add(ECORE_X_EVENT_WINDOW_VISIBILITY_CHANGE, e, NULL, NULL);
   }
}

void
_ecore_x_event_handle_create_notify(xcb_generic_event_t *event)
{
   xcb_create_notify_event_t     *ev;
   Ecore_X_Event_Window_Create *e;

   ev = (xcb_create_notify_event_t *)event;
   e = calloc(1, sizeof(Ecore_X_Event_Window_Create));
   if (!e) return;
   e->win = ev->window;
   if (ev->override_redirect)
      e->override = 1;
   else
      e->override = 0;
   e->time = _ecore_xcb_event_last_time;
   ecore_event_add(ECORE_X_EVENT_WINDOW_CREATE, e, NULL, NULL);
}

void
_ecore_x_event_handle_destroy_notify(xcb_generic_event_t *event)
{
   xcb_destroy_notify_event_t     *ev;
   Ecore_X_Event_Window_Destroy *e;

   ev = (xcb_destroy_notify_event_t *)event;
   e = calloc(1, sizeof(Ecore_X_Event_Window_Destroy));
   if (!e) return;
   e->win =  ev->window;
   e->time = _ecore_xcb_event_last_time;
   if (e->win == _ecore_xcb_event_last_window) _ecore_xcb_event_last_window = 0;
   ecore_event_add(ECORE_X_EVENT_WINDOW_DESTROY, e, NULL, NULL);
}

void
_ecore_x_event_handle_unmap_notify(xcb_generic_event_t *event)
{
   xcb_unmap_notify_event_t    *ev;
   Ecore_X_Event_Window_Hide *e;

   ev = (xcb_unmap_notify_event_t *)event;
   e = calloc(1, sizeof(Ecore_X_Event_Window_Hide));
   if (!e) return;
   e->win = ev->window;
   e->time = _ecore_xcb_event_last_time;
   ecore_event_add(ECORE_X_EVENT_WINDOW_HIDE, e, NULL, NULL);
}

void
_ecore_x_event_handle_map_notify(xcb_generic_event_t *event)
{
   xcb_map_notify_event_t      *ev;
   Ecore_X_Event_Window_Show *e;

   ev = (xcb_map_notify_event_t *)event;
   e = calloc(1, sizeof(Ecore_X_Event_Window_Show));
   if (!e) return;
   e->win = ev->window;
   e->time = _ecore_xcb_event_last_time;
   ecore_event_add(ECORE_X_EVENT_WINDOW_SHOW, e, NULL, NULL);
}

void
_ecore_x_event_handle_map_request(xcb_generic_event_t *event)
{
   xcb_map_request_event_t             *ev;
   Ecore_X_Event_Window_Show_Request *e;

   ev = (xcb_map_request_event_t *)event;
   e = calloc(1, sizeof(Ecore_X_Event_Window_Show_Request));
   if (!e) return;
   e->win = ev->window;
   e->parent = ev->parent;
   e->time = _ecore_xcb_event_last_time;
   ecore_event_add(ECORE_X_EVENT_WINDOW_SHOW_REQUEST, e, NULL, NULL);
}

void
_ecore_x_event_handle_reparent_notify(xcb_generic_event_t *event)
{
   xcb_reparent_notify_event_t     *ev;
   Ecore_X_Event_Window_Reparent *e;

   ev = (xcb_reparent_notify_event_t *)event;
   e = calloc(1, sizeof(Ecore_X_Event_Window_Reparent));
   if (!e) return;
   e->win = ev->window;
   e->parent = ev->parent;
   e->time = _ecore_xcb_event_last_time;
   ecore_event_add(ECORE_X_EVENT_WINDOW_REPARENT, e, NULL, NULL);
}

void
_ecore_x_event_handle_configure_notify(xcb_generic_event_t *event)
{
   xcb_configure_notify_event_t     *ev;
   Ecore_X_Event_Window_Configure *e;

   ev = (xcb_configure_notify_event_t *)event;
   e = calloc(1, sizeof(Ecore_X_Event_Window_Configure));
   if (!e) return;
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
}

void
_ecore_x_event_handle_configure_request(xcb_generic_event_t *event)
{
   xcb_configure_request_event_t            *ev;
   Ecore_X_Event_Window_Configure_Request *e;

   ev = (xcb_configure_request_event_t *)event;
   e = calloc(1, sizeof(Ecore_X_Event_Window_Configure_Request));
   if (!e) return;
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
   }
   e->time = _ecore_xcb_event_last_time;
   ecore_event_add(ECORE_X_EVENT_WINDOW_CONFIGURE_REQUEST, e, NULL, NULL);
}

void
_ecore_x_event_handle_gravity_notify(xcb_generic_event_t *event __UNUSED__)
{
   /* FIXME: handle this event type */
}

void
_ecore_x_event_handle_resize_request(xcb_generic_event_t *event)
{
   xcb_resize_request_event_t            *ev;
   Ecore_X_Event_Window_Resize_Request *e;

   ev = (xcb_resize_request_event_t *)event;
   e = calloc(1, sizeof(Ecore_X_Event_Window_Resize_Request));
   if (!e) return;
   e->win = ev->window;
   e->w = ev->width;
   e->h = ev->height;
   e->time = _ecore_xcb_event_last_time;
   ecore_event_add(ECORE_X_EVENT_WINDOW_RESIZE_REQUEST, e, NULL, NULL);
}

void
_ecore_x_event_handle_circulate_notify(xcb_generic_event_t *event)
{
   xcb_circulate_notify_event_t *ev;
   Ecore_X_Event_Window_Stack *e;

   ev = (xcb_circulate_notify_event_t *)event;
   e = calloc(1, sizeof(Ecore_X_Event_Window_Stack));
   if (!e) return;
   e->win = ev->window;
   e->event_win = ev->event;
   if (ev->place == XCB_PLACE_ON_TOP)
     e->detail = ECORE_X_WINDOW_STACK_ABOVE;
   else
     e->detail = ECORE_X_WINDOW_STACK_BELOW;
   e->time = _ecore_xcb_event_last_time;
   ecore_event_add(ECORE_X_EVENT_WINDOW_STACK, e, NULL, NULL);
}

void
_ecore_x_event_handle_circulate_request(xcb_generic_event_t *event)
{
   xcb_circulate_request_event_t        *ev;
   Ecore_X_Event_Window_Stack_Request *e;

   ev = (xcb_circulate_request_event_t *)event;
   e = calloc(1, sizeof(Ecore_X_Event_Window_Stack_Request));
   if (!e) return;
   e->win = ev->window;
   e->parent = ev->event;
   if (ev->place == XCB_PLACE_ON_TOP)
     e->detail = ECORE_X_WINDOW_STACK_ABOVE;
   else
     e->detail = ECORE_X_WINDOW_STACK_BELOW;
   e->time = _ecore_xcb_event_last_time;
   ecore_event_add(ECORE_X_EVENT_WINDOW_STACK_REQUEST, e, NULL, NULL);
}

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
	if (!e) return;
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
	if (!e) return;
	e->title = ecore_x_window_prop_title_get(xevent->xproperty.window);
   e->time = xevent->xproperty.time;
   _ecore_x_event_last_time = e->time;
	ecore_event_add(ECORE_X_EVENT_WINDOW_PROP_TITLE_CHANGE, e, _ecore_x_event_free_window_prop_title_change, NULL);
     }
   else if (xevent->xproperty.atom == ECORE_X_ATOM_NET_WM_VISIBLE_NAME)
     {
	Ecore_X_Event_Window_Prop_Visible_Title_Change *e;

	e = calloc(1, sizeof(Ecore_X_Event_Window_Prop_Visible_Title_Change));
	if (!e) return;
	e->title = ecore_x_window_prop_visible_title_get(xevent->xproperty.window);
   e->time = xevent->xproperty.time;
   _ecore_x_event_last_time = e->time;
	ecore_event_add(ECORE_X_EVENT_WINDOW_PROP_VISIBLE_TITLE_CHANGE, e, _ecore_x_event_free_window_prop_visible_title_change, NULL);
     }
   else if ((xevent->xproperty.atom == ECORE_X_ATOM_WM_ICON_NAME) || (xevent->xproperty.atom == ECORE_X_ATOM_NET_WM_ICON_NAME))
     {
	Ecore_X_Event_Window_Prop_Icon_Name_Change *e;

	e = calloc(1, sizeof(Ecore_X_Event_Window_Prop_Icon_Name_Change));
	if (!e) return;
	e->name = ecore_x_window_prop_icon_name_get(xevent->xproperty.window);
   e->time = xevent->xproperty.time;
   _ecore_x_event_last_time = e->time;
	ecore_event_add(ECORE_X_EVENT_WINDOW_PROP_ICON_NAME_CHANGE, e, _ecore_x_event_free_window_prop_icon_name_change, NULL);
     }
   else if (xevent->xproperty.atom == ECORE_X_ATOM_NET_WM_VISIBLE_ICON_NAME)
     {
	Ecore_X_Event_Window_Prop_Visible_Icon_Name_Change *e;

	e = calloc(1, sizeof(Ecore_X_Event_Window_Prop_Visible_Icon_Name_Change));
	if (!e) return;
	e->name = ecore_x_window_prop_visible_icon_name_get(xevent->xproperty.window);
   e->time = xevent->xproperty.time;
   _ecore_x_event_last_time = e->time;
	ecore_event_add(ECORE_X_EVENT_WINDOW_PROP_VISIBLE_ICON_NAME_CHANGE, e, _ecore_x_event_free_window_prop_visible_icon_name_change, NULL);
     }
   else if (xevent->xproperty.atom == ECORE_X_ATOM_WM_CLIENT_MACHINE)
     {
	Ecore_X_Event_Window_Prop_Client_Machine_Change *e;

	e = calloc(1, sizeof(Ecore_X_Event_Window_Prop_Client_Machine_Change));
	if (!e) return;
	e->name = ecore_x_window_prop_client_machine_get(xevent->xproperty.window);
   e->time = xevent->xproperty.time;
   _ecore_x_event_last_time = e->time;
	ecore_event_add(ECORE_X_EVENT_WINDOW_PROP_CLIENT_MACHINE_CHANGE, e, _ecore_x_event_free_window_prop_client_machine_change, NULL);
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
      xcb_property_notify_event_t     *ev;
      Ecore_X_Event_Window_Property *e;

      ev = (xcb_property_notify_event_t *)event;
      e = calloc(1,sizeof(Ecore_X_Event_Window_Property));
      if (!e) return;
      e->win = ev->window;
      e->atom = ev->atom;
      e->time = ev->time;
      _ecore_xcb_event_last_time = e->time;
      ecore_event_add(ECORE_X_EVENT_WINDOW_PROPERTY, e, NULL, NULL);
   }
}

void
_ecore_x_event_handle_selection_clear(xcb_generic_event_t *event)
{
   xcb_selection_clear_event_t     *ev;
   Ecore_X_Selection_Intern      *d;
   Ecore_X_Event_Selection_Clear *e;
   Ecore_X_Atom                   sel;

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

}

void
_ecore_x_event_handle_selection_request(xcb_generic_event_t *event)
{
   xcb_selection_request_event_t *ev;
   Ecore_X_Selection_Intern    *sd;
   xcb_selection_notify_event_t   sn_event;
   void                          *data;

   ev = (xcb_selection_request_event_t *)event;
   /* FIXME: is it the correct value ? */
   sn_event.response_type = XCB_SELECTION_NOTIFY;
   sn_event.pad0 = 0;
   /* FIXME: is it the correct value ? */
   sn_event.sequence = 0;
   sn_event.time = XCB_CURRENT_TIME;
   sn_event.requestor = ev->requestor;
   sn_event.selection = ev->selection;
   sn_event.target = ev->target;

   if ((sd = _ecore_x_selection_get(ev->selection)) &&
       (sd->win == ev->owner))
     {
	if (!ecore_x_selection_convert(ev->selection, ev->target,
                                       &data))
	  {
	     /* Refuse selection, conversion to requested target failed */
	     sn_event.property = XCB_NONE;
	  }
	else
	  {
	     /* FIXME: This does not properly handle large data transfers */
	     ecore_x_window_prop_property_set(ev->requestor,
                                              ev->property,
                                              ev->target,
                                              8, data, sd->length);
	     sn_event.property = ev->property;
	     free(data);
	  }
     }
   else
     {
	sn_event.property = XCB_NONE;
	return;
     }

   /* FIXME: I use _ecore_xcb_conn, as ev has no information on the connection */
   xcb_send_event(_ecore_xcb_conn, 0,
                  ev->requestor, 0, (const char *)&sn_event);
}

/* FIXME: round trip */
void
_ecore_x_event_handle_selection_notify(xcb_generic_event_t *event)
{
   xcb_selection_notify_event_t   *ev;
   Ecore_X_Event_Selection_Notify *e;
   unsigned char                 *data = NULL;
   Ecore_X_Atom                   selection;
   int                            num_ret;
   uint8_t                        format;

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
	if (!format) return;
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
	if (!format) return;
     }

   e = calloc(1, sizeof(Ecore_X_Event_Selection_Notify));
   if (!e) return;
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
}

void
_ecore_x_event_handle_colormap_notify(xcb_generic_event_t *event)
{
   xcb_colormap_notify_event_t     *ev;
   Ecore_X_Event_Window_Colormap *e;

   ev = (xcb_colormap_notify_event_t *)event;
   e = calloc(1,sizeof(Ecore_X_Event_Window_Colormap));
   if (!e) return;
   e->win = ev->window;
   e->cmap = ev->colormap;
   if (ev->state == XCB_COLORMAP_STATE_INSTALLED)
      e->installed = 1;
   else
      e->installed = 0;
   e->time = _ecore_xcb_event_last_time;
   ecore_event_add(ECORE_X_EVENT_WINDOW_COLORMAP, e, NULL, NULL);
}

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
	if (!e) return;
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
	if (!e) return;
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
	Ecore_X_DND_Target       *target;
	uint32_t                  three;

	e = calloc(1, sizeof(Ecore_X_Event_Xdnd_Enter));
	if (!e) return;

	target = _ecore_x_dnd_target_get();
	target->state = ECORE_X_DND_TARGET_ENTERED;

	target = _ecore_x_dnd_target_get();
	target->source = ev->data.data32[0];
	target->win = ev->window;
	target->version = ev->data.data32[1] >> 24;
	if (target->version > ECORE_X_DND_VERSION)
	  {
             printf("DND: Requested version %d, we only support up to %d\n", target->version,
                    ECORE_X_DND_VERSION);
	     return;
	  }

        /* FIXME: roud trip, but I don't know how to suppress it */
	if ((three = ev->data.data32[1] & 0x1UL))
	  {
	     /* source supports more than 3 types, fetch property */
	     unsigned char *data;
	     Ecore_X_Atom  *types;
	     int            num_ret;
	     int            i;
             uint8_t        format;

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
		  printf("DND: Could not fetch data type list from source window, aborting.\n");
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
                       char                      *name;

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
                       char                      *name;

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
                       if (reply) free(reply);
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
	Ecore_X_DND_Target          *target;

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
	if (!e) return;
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
	Ecore_X_DND_Source        *source;

	source = _ecore_x_dnd_source_get();
	/* Make sure source/target match */
	if ((source->win != ev->window ) ||
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
   else if (ev->type == ECORE_X_ATOM_XDND_LEAVE)
     {
	Ecore_X_Event_Xdnd_Leave *e;
	Ecore_X_DND_Target       *target;

	target = _ecore_x_dnd_target_get();
	if ((target->source != (Ecore_X_Window)ev->data.data32[0]) ||
	    (target->win != ev->window))
	  return;

	target->state = ECORE_X_DND_TARGET_IDLE;

	e = calloc(1, sizeof(Ecore_X_Event_Xdnd_Leave));
	if (!e) return;
	e->win = ev->window;
	e->source = ev->data.data32[0];
	ecore_event_add(ECORE_X_EVENT_XDND_LEAVE, e, NULL, NULL);
     }

   /* Message Type: XdndDrop target */
   else if (ev->type == ECORE_X_ATOM_XDND_DROP)
     {
	Ecore_X_Event_Xdnd_Drop *e;
	Ecore_X_DND_Target      *target;

	target = _ecore_x_dnd_target_get();
	/* Match source/target */
	if ((target->source != (Ecore_X_Window)ev->data.data32[0]) ||
	    (target->win != ev->window))
	  return;

	target->time = (target->version >= 1) ?
	   ev->data.data32[2] : _ecore_xcb_event_last_time;

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
   else if (ev->type == ECORE_X_ATOM_XDND_FINISHED)
     {
	Ecore_X_Event_Xdnd_Finished *e;
	Ecore_X_DND_Source          *source;
	uint8_t                        completed = 1;

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
	if (!e) return;
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
	if (!e) return;
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
	     char                      *name;

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
                 printf("Unknown state: %s\n", name);
                 free(name);
                 free(reply);
               }
	  }
	e->state[1] = _ecore_x_netwm_state_get(ev->data.data32[2]);
	if (e->state[1] == ECORE_X_WINDOW_STATE_UNKNOWN)
	  {
             xcb_get_atom_name_reply_t *reply;
	     char                      *name;

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
                 printf("Unknown state: %s\n", name);
                 free(name);
               }
	  }
	e->source = ev->data.data32[3];

	ecore_event_add(ECORE_X_EVENT_WINDOW_STATE_REQUEST, e, NULL, NULL);
     }
   else if ((ev->type == ECORE_X_ATOM_WM_CHANGE_STATE)
	    && (ev->format == 32)
	    && (ev->data.data32[0] == XCB_WM_ICONIC_STATE))
     {
	Ecore_X_Event_Window_State_Request *e;

	e = calloc(1, sizeof(Ecore_X_Event_Window_State_Request));
	if (!e) return;
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
	if (!e) return;
	e->win = ev->window;
	e->desk = ev->data.data32[0];
	e->source = ev->data.data32[1];

	ecore_event_add(ECORE_X_EVENT_DESKTOP_CHANGE, e, NULL, NULL);
     }
   else if ((ev->type == ECORE_X_ATOM_NET_REQUEST_FRAME_EXTENTS))
     {
	Ecore_X_Event_Frame_Extents_Request *e;

	e = calloc(1, sizeof(Ecore_X_Event_Frame_Extents_Request));
	if (!e) return;
	e->win = ev->window;

	ecore_event_add(ECORE_X_EVENT_FRAME_EXTENTS_REQUEST, e, NULL, NULL);
     }
   else if ((ev->type == ECORE_X_ATOM_WM_PROTOCOLS)
	    && ((Ecore_X_Atom)ev->data.data32[0] == ECORE_X_ATOM_NET_WM_PING)
	    && (ev->format == 32))
     {
	Ecore_X_Event_Ping *e;

	e = calloc(1, sizeof(Ecore_X_Event_Ping));
	if (!e) return;
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
	if (!e) return;
	e->win = ev->window;
	e->message_type = ev->type;
	e->format = ev->format;
	for (i = 0; i < 5; i++)
	  e->data.l[i] = ev->data.data32[i];

	ecore_event_add(ECORE_X_EVENT_CLIENT_MESSAGE, e, NULL, NULL);
     }
}

void
_ecore_x_event_handle_mapping_notify(xcb_generic_event_t *event __UNUSED__)
{
   /* FIXME: handle this event type */
}

void
_ecore_x_event_handle_shape_change(xcb_generic_event_t *event)
{
#ifdef ECORE_X_SHAPE
   xcb_shape_notify_event_t     *ev;
   Ecore_X_Event_Window_Shape *e;

   ev = (xcb_shape_notify_event_t *)event;
   e = calloc(1, sizeof(Ecore_X_Event_Window_Shape));
   if (!e) return;
   e->win = ev->affected_window;
   e->time = ev->server_time;
   ecore_event_add(ECORE_X_EVENT_WINDOW_SHAPE, e, NULL, NULL);
#else
   event = NULL;
#endif /* ECORE_X_SHAPE */
}

void
_ecore_x_event_handle_screensaver_notify(xcb_generic_event_t *event)
{
#ifdef ECORE_X_SCREENSAVER
   xcb_screensaver_notify_event_t     *ev;
   Ecore_X_Event_Screensaver_Notify *e;

   ev = (xcb_screensaver_notify_event_t *)event;
   e = calloc(1, sizeof(Ecore_X_Event_Screensaver_Notify));
   if (!e) return;
   e->win = ev->window;
   if (ev->state == XCB_SCREENSAVER_STATE_ON)
     e->on = 1;
   else
     e->on = 0;
   e->time = ev->time;
   ecore_event_add(ECORE_X_EVENT_SCREENSAVER_NOTIFY, e, NULL, NULL);
#else
   event = NULL;
#endif /* ECORE_X_SCREENSAVER */
}

void
_ecore_x_event_handle_sync_counter(xcb_generic_event_t *event)
{
#ifdef ECORE_X_SYNC
   xcb_sync_counter_notify_event_t *ev;
   Ecore_X_Event_Sync_Counter    *e;

   ev = (xcb_sync_counter_notify_event_t *)event;
   e = calloc(1, sizeof(Ecore_X_Event_Sync_Counter));
   if (!e) return;
   e->time = ev->timestamp;
   ecore_event_add(ECORE_X_EVENT_SYNC_COUNTER, e, NULL, NULL);
#else
   event = NULL;
#endif /* ECORE_X_SYNC */
}

void
_ecore_x_event_handle_sync_alarm(xcb_generic_event_t *event)
{
#ifdef ECORE_X_SYNC
   xcb_sync_alarm_notify_event_t *ev;
   Ecore_X_Event_Sync_Alarm    *e;

   ev = (xcb_sync_alarm_notify_event_t *)event;
   e = calloc(1, sizeof(Ecore_X_Event_Sync_Alarm));
   if (!e) return;
   e->time = ev->timestamp;
   e->alarm = ev->alarm;
   ecore_event_add(ECORE_X_EVENT_SYNC_ALARM, e, NULL, NULL);
#else
   event = NULL;
#endif /* ECORE_X_SYNC */
}

/* FIXME: round trip */
void
_ecore_x_event_handle_randr_change(xcb_generic_event_t *event)
{
#ifdef ECORE_X_RANDR
   xcb_randr_screen_change_notify_event_t *ev;
   Ecore_X_Event_Screen_Change            *e;

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
       printf("ERROR: Can't update RandR config!\n");
     if (rep)
       free(rep);
     }

   e = calloc(1, sizeof(Ecore_X_Event_Screen_Change));
   if (!e) return;
   e->win = ev->request_window;
   e->root = ev->root;
   e->width = ev->width;
   e->height = ev->height;
   ecore_event_add(ECORE_X_EVENT_SCREEN_CHANGE, e, NULL, NULL);
#else
   event = NULL;
#endif /* ECORE_X_RANDR */
}

void
_ecore_x_event_handle_fixes_selection_notify(xcb_generic_event_t *event)
{
#ifdef ECORE_X_FIXES
   /* Nothing here yet */
#else
   event = NULL;
#endif /* ECORE_X_FIXES */
}

void
_ecore_x_event_handle_damage_notify(xcb_generic_event_t *event)
{
#ifdef ECORE_XCBDAMAGE
   xcb_damage_notify_event_t *ev;
   Ecore_X_Event_Damage      *e;

   ev = (xcb_damage_notify_event_t *)event;
   e = calloc(1, sizeof(Ecore_X_Event_Damage));
   if (!e) return;

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
#else
   event = NULL;
#endif /* ECORE_XCBDAMAGE */
}
