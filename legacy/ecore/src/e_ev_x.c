#include "Ecore.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* private funtion prototypes */
static void         e_ev_x_handle_events(int fd);
static void         e_ev_x_translate_events(XEvent * events, int num_events);

static void         e_ev_key_down_free(void *event);
static void         e_ev_key_up_free(void *event);
static void         e_ev_generic_free(void *event);
static void         e_ev_dnd_drop_request_free(void *event);
static void         e_ev_paste_request_free(void *event);

static void         e_ev_x_handle_key_press(XEvent * xevent);
static void         e_ev_x_handle_key_release(XEvent * xevent);
static void         e_ev_x_handle_button_press(XEvent * xevent);
static void         e_ev_x_handle_button_release(XEvent * xevent);
static void         e_ev_x_handle_motion_notify(XEvent * xevent);
static void         e_ev_x_handle_enter_notify(XEvent * xevent);
static void         e_ev_x_handle_leave_notify(XEvent * xevent);
static void         e_ev_x_handle_focus_in(XEvent * xevent);
static void         e_ev_x_handle_focus_out(XEvent * xevent);
static void         e_ev_x_handle_expose(XEvent * xevent);
static void         e_ev_x_handle_visibility_notify(XEvent * xevent);
static void         e_ev_x_handle_create_notify(XEvent * xevent);
static void         e_ev_x_handle_destroy_notify(XEvent * xevent);
static void         e_ev_x_handle_unmap_notify(XEvent * xevent);
static void         e_ev_x_handle_map_notify(XEvent * xevent);
static void         e_ev_x_handle_map_request(XEvent * xevent);
static void         e_ev_x_handle_reparent_notify(XEvent * xevent);
static void         e_ev_x_handle_configure_notify(XEvent * xevent);
static void         e_ev_x_handle_configure_request(XEvent * xevent);
static void         e_ev_x_handle_circulate_notify(XEvent * xevent);
static void         e_ev_x_handle_circulate_request(XEvent * xevent);
static void         e_ev_x_handle_property_notify(XEvent * xevent);
static void         e_ev_x_handle_colormap_notify(XEvent * xevent);
static void         e_ev_x_handle_selection_notify(XEvent * xevent);
static void         e_ev_x_handle_selection_clear(XEvent * xevent);
static void         e_ev_x_handle_selection_request(XEvent * xevent);
static void         e_ev_x_handle_client_message(XEvent * xevent);
static void         e_ev_x_handle_shape_change(XEvent * xevent);

static int          max_event_id = 0;
static void         (**event_translator) (XEvent * event) = NULL;

static int          lock_mask_scroll = 0, lock_mask_num = 0, lock_mask_caps = 0;
static int          mod_mask_shift = 0, mod_mask_ctrl = 0, mod_mask_alt =
   0, mod_mask_win = 0;

/* convenience macros */
#define GETSET_MODS(state, mods) \
(mods) = EV_KEY_MODIFIER_NONE;\
if ((state) & mod_mask_shift) { e_mod_shift_set(1); (mods) |= EV_KEY_MODIFIER_SHIFT; }\
else                                       e_mod_shift_set(0);\
if ((state) & mod_mask_ctrl)  { e_mod_ctrl_set(1);  (mods) |= EV_KEY_MODIFIER_CTRL; }\
else                                       e_mod_ctrl_set(0);\
if ((state) & mod_mask_alt)   { e_mod_alt_set(1);   (mods) |= EV_KEY_MODIFIER_ALT; }\
else                                       e_mod_alt_set(0);\
if ((state) & mod_mask_win)   { e_mod_win_set(1);   (mods) |= EV_KEY_MODIFIER_WIN; }\
else                                       e_mod_win_set(0);

/* public functions */

/* initialise event handling for the fd X is on */
void
e_ev_x_init(void)
{
   int                 i, shape_event_id, current_lock;

   shape_event_id = max_event_id = e_event_shape_get_id();
   event_translator = NEW_PTR(max_event_id + 1);
   for (i = 0; i < max_event_id + 1; i++)
      event_translator[i] = NULL;
   event_translator[KeyPress] = e_ev_x_handle_key_press;
   event_translator[KeyRelease] = e_ev_x_handle_key_release;
   event_translator[ButtonPress] = e_ev_x_handle_button_press;
   event_translator[ButtonRelease] = e_ev_x_handle_button_release;
   event_translator[MotionNotify] = e_ev_x_handle_motion_notify;
   event_translator[EnterNotify] = e_ev_x_handle_enter_notify;
   event_translator[LeaveNotify] = e_ev_x_handle_leave_notify;
   event_translator[FocusIn] = e_ev_x_handle_focus_in;
   event_translator[FocusOut] = e_ev_x_handle_focus_out;
   event_translator[Expose] = e_ev_x_handle_expose;
   event_translator[VisibilityNotify] = e_ev_x_handle_visibility_notify;
   event_translator[CreateNotify] = e_ev_x_handle_create_notify;
   event_translator[DestroyNotify] = e_ev_x_handle_destroy_notify;
   event_translator[UnmapNotify] = e_ev_x_handle_unmap_notify;
   event_translator[MapNotify] = e_ev_x_handle_map_notify;
   event_translator[MapRequest] = e_ev_x_handle_map_request;
   event_translator[ReparentNotify] = e_ev_x_handle_reparent_notify;
   event_translator[ConfigureNotify] = e_ev_x_handle_configure_notify;
   event_translator[ConfigureRequest] = e_ev_x_handle_configure_request;
   event_translator[CirculateNotify] = e_ev_x_handle_circulate_notify;
   event_translator[CirculateRequest] = e_ev_x_handle_circulate_request;
   event_translator[PropertyNotify] = e_ev_x_handle_property_notify;
   event_translator[ColormapNotify] = e_ev_x_handle_colormap_notify;
   event_translator[ClientMessage] = e_ev_x_handle_client_message;
   event_translator[SelectionNotify] = e_ev_x_handle_selection_notify;
   event_translator[SelectionClear] = e_ev_x_handle_selection_clear;
   event_translator[SelectionRequest] = e_ev_x_handle_selection_request;
   event_translator[shape_event_id] = e_ev_x_handle_shape_change;

   lock_mask_scroll = e_lock_mask_scroll_get();
   lock_mask_num = e_lock_mask_num_get();
   lock_mask_caps = e_lock_mask_caps_get();

   mod_mask_shift = e_mod_mask_shift_get();
   mod_mask_ctrl = e_mod_mask_ctrl_get();
   mod_mask_alt = e_mod_mask_alt_get();
   mod_mask_win = e_mod_mask_win_get();

/* HRRRMMM lets not do this  
   e_key_grab("Num_Lock", EV_KEY_MODIFIER_NONE, 1, 1);
   e_key_grab("Scroll_Lock", EV_KEY_MODIFIER_NONE, 1, 1);
   e_key_grab("Caps_Lock", EV_KEY_MODIFIER_NONE, 1, 1);
 */
   current_lock = e_modifier_mask_get();
   if (current_lock & lock_mask_scroll)
      e_lock_scroll_set(1);
   if (current_lock & lock_mask_num)
      e_lock_num_set(1);
   if (current_lock & lock_mask_caps)
      e_lock_caps_set(1);
   if (current_lock & mod_mask_shift)
      e_mod_shift_set(1);
   if (current_lock & mod_mask_ctrl)
      e_mod_ctrl_set(1);
   if (current_lock & mod_mask_alt)
      e_mod_alt_set(1);
   if (current_lock & mod_mask_win)
      e_mod_win_set(1);
   e_add_event_fd(e_x_get_fd(), e_ev_x_handle_events);
}

/* private functions */
/* get all events onthe event queue and translate them */
static void
e_ev_x_handle_events(int fd)
{
   int                 num_events = 0, size_events = 0;
   XEvent             *events = NULL;

   /* while there are events in the queue */
   while (e_events_pending())
     {
	/* incriment our event count */
	num_events++;
	/* if the numebr fo events is > than our buffer size then */
	if (num_events > size_events)
	  {
	     /* increase the buffer size by 64 events */
	     size_events += 64;
	     if (events)
	       {
		  REALLOC(events, XEvent, size_events)
	       }
	     else
		events = NEW(XEvent, size_events);
	  }
	/* get the next event into the event buffer */
	e_get_next_event(&(events[num_events - 1]));
     }
   /* call the XEvent -> Eevent translator */
   if (events)
     {
	e_ev_x_translate_events(events, num_events);
	/* if theres an event buffer - free it */
	FREE(events);
     }
   return;
   fd = 0;
}

/* take an array of events and translate them into E events */
static void
e_ev_x_translate_events(XEvent * events, int num_events)
{
   int                 i;

   for (i = 0; i < num_events; i++)
     {
	if ((events[i].type <= max_event_id) &&
	    (event_translator[events[i].type]))
	   (*(event_translator[events[i].type])) (&(events[i]));
     }
}

static void
e_ev_key_down_free(void *event)
{
   Ev_Key_Down        *e;

   e = (Ev_Key_Down *) event;
   IF_FREE(e->key);
   IF_FREE(e->compose);
   FREE(e);
}

static void
e_ev_key_up_free(void *event)
{
   Ev_Key_Up          *e;

   e = (Ev_Key_Up *) event;
   IF_FREE(e->key);
   IF_FREE(e->compose);
   FREE(e);
}

static void
e_ev_generic_free(void *event)
{
   FREE(event);
}

static void
e_ev_dnd_drop_request_free(void *event)
{
   Ev_Dnd_Drop_Request *e;

   e = (Ev_Dnd_Drop_Request *) event;
   if (e->files)
   {
      int i;
      for (i=0; i< e->num_files; i++)
         FREE(e->files[i]);
   }
   FREE(event);
}

static void
e_ev_paste_request_free(void *event)
{
   Ev_Paste_Request *e;

   e = (Ev_Paste_Request *) event;
   IF_FREE(e->string);
   FREE(event);
}

static void
e_ev_x_handle_key_press(XEvent * xevent)
{
   Ev_Key_Down        *e;
   static KeyCode      previous_code = 0;
   static Time         previous_time = 0;

   /* avoid doubling events up from passive grabs */
   if ((xevent->xkey.keycode == previous_code) &&
       xevent->xkey.time == previous_time)
      return;
   previous_code = xevent->xkey.keycode;
   previous_time = xevent->xkey.time;
/*   
   if (e_key_get_keysym_from_keycode(xevent->xkey.keycode) == XK_Scroll_Lock)
     {
	if (e_lock_scroll_get())
	   e_lock_scroll_set(0);
	else
	   e_lock_scroll_set(1);
	e_event_allow(ReplayKeyboard, xevent->xkey.time);
	e_flush();
     }
   else if (e_key_get_keysym_from_keycode(xevent->xkey.keycode) == XK_Num_Lock)
     {
	if (e_lock_num_get())
	   e_lock_num_set(0);
	else
	   e_lock_num_set(1);
	e_event_allow(ReplayKeyboard, xevent->xkey.time);
	e_flush();
     }
   else if (e_key_get_keysym_from_keycode(xevent->xkey.keycode) == XK_Caps_Lock)
     {
	if (e_lock_caps_get())
	   e_lock_caps_set(0);
	else
	   e_lock_caps_set(1);
	e_event_allow(ReplayKeyboard, xevent->xkey.time);
	e_flush();
     }
 */
   e = NEW(Ev_Key_Down, 1);
   e->win = xevent->xkey.window;
   e->root = xevent->xkey.root;
   GETSET_MODS(xevent->xkey.state, e->mods);
   e->time = xevent->xkey.time;
   e->key = e_key_get_string_from_keycode(xevent->xkey.keycode);
   {
      int                 val;
      char                buf[256];
      KeySym              sym;
      XComposeStatus      stat;

      val = XLookupString((XKeyEvent *) xevent, buf, sizeof(buf), &sym, &stat);
      if (val > 0)
	{
	   buf[val] = 0;
	   e->compose = strdup(buf);
	}
      else
	 e->compose = NULL;
   }
   e_add_event(EV_KEY_DOWN, e, e_ev_key_down_free);
}

static void
e_ev_x_handle_key_release(XEvent * xevent)
{
   Ev_Key_Up          *e;
   static KeyCode      previous_code = 0;
   static Time         previous_time = 0;

   /* avoid doubling events up from passive grabs */
   if ((xevent->xkey.keycode == previous_code) &&
       xevent->xkey.time == previous_time)
      return;
   previous_code = xevent->xkey.keycode;
   previous_time = xevent->xkey.time;

   e = NEW(Ev_Key_Up, 1);
   e->win = xevent->xkey.window;
   e->root = xevent->xkey.root;
   GETSET_MODS(xevent->xkey.state, e->mods);
   e->time = xevent->xkey.time;
   e->key = e_key_get_string_from_keycode(xevent->xkey.keycode);
   {
      int                 val;
      char                buf[256];
      KeySym              sym;
      XComposeStatus      stat;

      val = XLookupString((XKeyEvent *) xevent, buf, sizeof(buf), &sym, &stat);
      if (val > 0)
	{
	   buf[val] = 0;
	   e->compose = strdup(buf);
	}
      else
	 e->compose = NULL;
   }
   e_add_event(EV_KEY_UP, e, e_ev_key_up_free);
}

static void
e_ev_x_handle_button_press(XEvent * xevent)
{
   static Time         last_time = 0, last_last_time = 0;
   static int          last_button = 0, last_last_button = 0;
   static Window       last_window = 0, last_last_window = 0;

   e_pointer_xy_set(xevent->xbutton.x_root, xevent->xbutton.y_root);
   if ((xevent->xbutton.button == 4) || (xevent->xbutton.button == 5))
     {
	Ev_Wheel           *e;

	e = NEW(Ev_Wheel, 1);
	e->win = xevent->xbutton.window;
	e->root = xevent->xbutton.root;
	e->x = xevent->xbutton.x;
	e->y = xevent->xbutton.y;
	e->rx = xevent->xbutton.x_root;
	e->ry = xevent->xbutton.y_root;
	e->time = xevent->xbutton.time;
	if (xevent->xbutton.button == 5)
	   e->z = 1;
	else
	   e->z = -1;
	if (xevent->xbutton.time - last_time < 15)
	   e->z *= 16;
	else if (xevent->xbutton.time - last_time < 30)
	   e->z *= 4;
	GETSET_MODS(xevent->xbutton.state, e->mods);
	e_add_event(EV_MOUSE_WHEEL, e, e_ev_generic_free);
     }
   else
     {
	Ev_Mouse_Down      *e;

	e = NEW(Ev_Mouse_Down, 1);
	e->win = xevent->xbutton.window;
	e->root = xevent->xbutton.root;
	e->button = xevent->xbutton.button;
	e->x = xevent->xbutton.x;
	e->y = xevent->xbutton.y;
	e->rx = xevent->xbutton.x_root;
	e->ry = xevent->xbutton.y_root;
	e->time = xevent->xbutton.time;
	e->double_click = 0;
	e->triple_click = 0;
	GETSET_MODS(xevent->xbutton.state, e->mods);
	if (xevent->xbutton.time - last_last_time < 500)
	  {
	     if ((xevent->xbutton.window == (unsigned int)last_window) &&
		 (last_window == last_last_window) &&
		 (xevent->xbutton.button == (unsigned int)last_button) &&
		 (last_button == last_button))
		e->triple_click = 1;
	  }
	else if (xevent->xbutton.time - last_time < 250)
	  {
	     if ((xevent->xbutton.window == (unsigned int)last_window) &&
		 (xevent->xbutton.button == (unsigned int)last_button))
		e->double_click = 1;
	  }
	e_add_event(EV_MOUSE_DOWN, e, e_ev_generic_free);
	  {
	     E_XID              *xid = NULL;
	     
	     if (XFindContext(xevent->xbutton.display, e->win, 
			      xid_context, (XPointer *) & xid) != XCNOENT)
	       {
		  if (xid->grab_button_auto_replay)
		    {
		       if ((xid->grab_button_button == 0) ||
			   (xid->grab_button_button == e->button))
			 {
			    if ((xid->grab_button_any_mod) ||
				(xid->grab_button_mods == e->mods))
			      {
				 e_pointer_replay(e->time);
			      }
			 }
		    }
	       }
	  }
     }
   last_last_window = last_window;
   last_window = xevent->xbutton.window;
   last_last_button = last_button;
   last_button = xevent->xbutton.button;
   last_last_time = last_time;
   last_time = xevent->xbutton.time;
}

static void
e_ev_x_handle_button_release(XEvent * xevent)
{
   Ev_Mouse_Up        *e;

   if (xevent->xbutton.button > 3)
      return;

   e = NEW(Ev_Mouse_Up, 1);
   e->win = xevent->xbutton.window;
   e->root = xevent->xbutton.root;
   e->button = xevent->xbutton.button;
   e->x = xevent->xbutton.x;
   e->y = xevent->xbutton.y;
   e->rx = xevent->xbutton.x_root;
   e->ry = xevent->xbutton.y_root;
   e->time = xevent->xbutton.time;
   GETSET_MODS(xevent->xbutton.state, e->mods);
   e_add_event(EV_MOUSE_UP, e, e_ev_generic_free);
}

static void
e_ev_x_handle_motion_notify(XEvent * xevent)
{
   Ev_Mouse_Move      *e;

   e_pointer_xy_set(xevent->xmotion.x_root, xevent->xmotion.y_root);
   e = NEW(Ev_Mouse_Move, 1);
   e->win = xevent->xmotion.window;
   e->root = xevent->xmotion.root;
   e->x = xevent->xmotion.x;
   e->y = xevent->xmotion.y;
   e->rx = xevent->xmotion.x_root;
   e->ry = xevent->xmotion.y_root;
   e->time = xevent->xmotion.time;
   GETSET_MODS(xevent->xmotion.state, e->mods);
   e_add_event(EV_MOUSE_MOVE, e, e_ev_generic_free);
}

static void
e_ev_x_handle_enter_notify(XEvent * xevent)
{
   Ev_Window_Enter    *e;
   
/*   if ((xevent->xcrossing.mode == NotifyGrab) || (xevent->xcrossing.mode == NotifyUngrab)) return;*/
   e_pointer_xy_set(xevent->xcrossing.x_root, xevent->xcrossing.y_root);
   e = NEW(Ev_Window_Enter, 1);
   e->win = xevent->xcrossing.window;
   e->root = xevent->xcrossing.root;
   e->x = xevent->xcrossing.x;
   e->y = xevent->xcrossing.y;
   e->rx = xevent->xcrossing.x_root;
   e->ry = xevent->xcrossing.y_root;
   e->time = xevent->xcrossing.time;
   GETSET_MODS(xevent->xcrossing.state, e->mods);
   e_add_event(EV_MOUSE_IN, e, e_ev_generic_free);
   e_window_mouse_set_in(e->win, 1);
     {
	Ev_Mouse_Move      *e;
	
	e = NEW(Ev_Mouse_Move, 1);
	e->win = xevent->xcrossing.window;
	e->root = xevent->xcrossing.root;
	e->x = xevent->xcrossing.x;
	e->y = xevent->xcrossing.y;
	e->rx = xevent->xcrossing.x_root;
	e->ry = xevent->xcrossing.y_root;
	e->time = xevent->xcrossing.time;
	GETSET_MODS(xevent->xcrossing.state, e->mods);
	e_add_event(EV_MOUSE_MOVE, e, e_ev_generic_free);
     }
}

static void
e_ev_x_handle_leave_notify(XEvent * xevent)
{
   Ev_Window_Leave    *e;

/*   if ((xevent->xcrossing.mode == NotifyGrab) || (xevent->xcrossing.mode == NotifyUngrab)) return;*/
   e_pointer_xy_set(xevent->xcrossing.x_root, xevent->xcrossing.y_root);
     {
	Ev_Mouse_Move      *e;
	
	e = NEW(Ev_Mouse_Move, 1);
	e->win = xevent->xcrossing.window;
	e->root = xevent->xcrossing.root;
	e->x = xevent->xcrossing.x;
	e->y = xevent->xcrossing.y;
	e->rx = xevent->xcrossing.x_root;
	e->ry = xevent->xcrossing.y_root;
	e->time = xevent->xcrossing.time;
	GETSET_MODS(xevent->xcrossing.state, e->mods);
	e_add_event(EV_MOUSE_MOVE, e, e_ev_generic_free);
     }
   e = NEW(Ev_Window_Leave, 1);
   e->win = xevent->xcrossing.window;
   e->root = xevent->xcrossing.root;
   e->x = xevent->xcrossing.x;
   e->y = xevent->xcrossing.y;
   e->rx = xevent->xcrossing.x_root;
   e->ry = xevent->xcrossing.y_root;
   e->time = xevent->xcrossing.time;
   GETSET_MODS(xevent->xcrossing.state, e->mods);
   e_add_event(EV_MOUSE_OUT, e, e_ev_generic_free);
   e_window_mouse_set_in(e->win, 0);
}

static void
e_ev_x_handle_focus_in(XEvent * xevent)
{
   Ev_Window_Focus_In *e;

   e = NEW(Ev_Window_Focus_In, 1);
   e->win = xevent->xfocus.window;
   e->root = e_window_get_root(e->win);
   if (xevent->xfocus.mode != NotifyNormal) e->key_grab = 1;
   else e->key_grab = 0;
   e_add_event(EV_WINDOW_FOCUS_IN, e, e_ev_generic_free);
   e_focus_window_set(e->win);
}

static void
e_ev_x_handle_focus_out(XEvent * xevent)
{
   Ev_Window_Focus_Out *e;

   e = NEW(Ev_Window_Focus_Out, 1);
   e->win = xevent->xfocus.window;
   e->root = e_window_get_root(e->win);
   if (xevent->xfocus.mode != NotifyNormal) e->key_grab = 1;
   else e->key_grab = 0;
   e_add_event(EV_WINDOW_FOCUS_OUT, e, e_ev_generic_free);
   e_focus_window_set(0);
}

static void
e_ev_x_handle_expose(XEvent * xevent)
{
   Ev_Window_Expose   *e;

   e = NEW(Ev_Window_Expose, 1);
   e->win = xevent->xexpose.window;
   e->root = e_window_get_root(e->win);
   e->x = xevent->xexpose.x;
   e->y = xevent->xexpose.y;
   e->w = xevent->xexpose.width;
   e->h = xevent->xexpose.height;
   e_add_event(EV_WINDOW_EXPOSE, e, e_ev_generic_free);
}

static void
e_ev_x_handle_visibility_notify(XEvent * xevent)
{
   if (xevent->xvisibility.state != VisibilityPartiallyObscured)
     {
	Ev_Window_Visibility *e;

	e = NEW(Ev_Window_Visibility, 1);
	e->win = xevent->xvisibility.window;
	e->root = e_window_get_root(e->win);
	if (xevent->xvisibility.state == VisibilityFullyObscured)
	   e->fully_obscured = 1;
	else
	   e->fully_obscured = 0;
	e_add_event(EV_WINDOW_VISIBILITY, e, e_ev_generic_free);
     }
}

static void
e_ev_x_handle_create_notify(XEvent * xevent)
{
   Ev_Window_Create   *e;

   e = NEW(Ev_Window_Create, 1);
   e->win = xevent->xcreatewindow.window;
   e_validate_xid(e->win);
   e->root = e_window_get_root(e->win);
   if (xevent->xcreatewindow.override_redirect)
      e->override = 1;
   else
      e->override = 0;
   e_add_event(EV_WINDOW_CREATE, e, e_ev_generic_free);
}

static void
e_ev_x_handle_destroy_notify(XEvent * xevent)
{
   Ev_Window_Destroy  *e;

   e = NEW(Ev_Window_Destroy, 1);
   e->win = xevent->xdestroywindow.window;
   e->root = e_window_get_root(e->win);
   e_add_event(EV_WINDOW_DESTROY, e, e_ev_generic_free);
   e_unvalidate_xid(e->win);
}

static void
e_ev_x_handle_unmap_notify(XEvent * xevent)
{
   Ev_Window_Unmap    *e;
   E_XID              *xid = NULL;

   e = NEW(Ev_Window_Unmap, 1);
   e->win = xevent->xunmap.window;
   e->root = e_window_get_root(e->win);
   e_add_event(EV_WINDOW_UNMAP, e, e_ev_generic_free);
   xid = e_validate_xid(e->win);
   if (xid)
      xid->mapped = 0;
}

static void
e_ev_x_handle_map_notify(XEvent * xevent)
{
   Ev_Window_Map      *e;
   E_XID              *xid = NULL;

   e = NEW(Ev_Window_Map, 1);
   e->win = xevent->xmap.window;
   e->root = e_window_get_root(e->win);
   e_add_event(EV_WINDOW_MAP, e, e_ev_generic_free);
   xid = e_validate_xid(e->win);
   if (xid)
      xid->mapped = 1;
}

static void
e_ev_x_handle_map_request(XEvent * xevent)
{
   Ev_Window_Map_Request *e;

   e = NEW(Ev_Window_Map_Request, 1);
   e->win = xevent->xmaprequest.window;
   e->root = e_window_get_root(e->win);
   e_add_event(EV_WINDOW_MAP_REQUEST, e, e_ev_generic_free);
   e_validate_xid(e->win);
}

static void
e_ev_x_handle_reparent_notify(XEvent * xevent)
{
   Ev_Window_Reparent *e;
   Window              parent;
   E_XID              *xid = NULL;

   e = NEW(Ev_Window_Reparent, 1);
   e->win = xevent->xreparent.window;
   xid = e_validate_xid(e->win);
   e->root = e_window_get_root(e->win);
   parent = e_window_get_parent(e->win);
   e->parent_from = parent;
   e->parent = xevent->xreparent.parent;
   e_validate_xid(e->parent);
   e_del_child(parent, e->win);
   e_add_child(xevent->xreparent.parent, xevent->xreparent.window);
   if (xid)
      xid->parent = e->parent;
   e_add_event(EV_WINDOW_REPARENT, e, e_ev_generic_free);
}

static void
e_ev_x_handle_configure_notify(XEvent * xevent)
{
   Ev_Window_Configure *e;
   E_XID              *xid;

   e = NEW(Ev_Window_Configure, 1);
   e->win = xevent->xconfigure.window;
   e->root = e_window_get_root(e->win);
   e->x = xevent->xconfigure.x;
   e->y = xevent->xconfigure.y;
   e->w = xevent->xconfigure.width;
   e->h = xevent->xconfigure.height;
   if (!xevent->xconfigure.send_event)
     {
	xid = e_validate_xid(e->win);
	if (xid)
	  {
	     xid->x = e->x;
	     xid->y = e->y;
	     xid->w = e->w;
	     xid->h = e->h;
	  }
	e->wm_generated = 0;
	/* FIXME: dont handle redioing stack for xevent->xconfigure.above */
	/* member (the window is stacked immediately in stack above this) */
     }
   else
      e->wm_generated = 1;
   e_add_event(EV_WINDOW_CONFIGURE, e, e_ev_generic_free);
}

static void
e_ev_x_handle_configure_request(XEvent * xevent)
{
   Ev_Window_Configure_Request *e;

   e = NEW(Ev_Window_Configure_Request, 1);
   e->win = xevent->xconfigurerequest.window;
   e->root = e_window_get_root(e->win);
   e->x = xevent->xconfigurerequest.x;
   e->y = xevent->xconfigurerequest.y;
   e->w = xevent->xconfigurerequest.width;
   e->h = xevent->xconfigurerequest.height;
   e->stack_win = xevent->xconfigurerequest.above;
   e->detail = xevent->xconfigurerequest.detail;
   e->mask = xevent->xconfigurerequest.value_mask;
   e_add_event(EV_WINDOW_CONFIGURE_REQUEST, e, e_ev_generic_free);
}

static void
e_ev_x_handle_circulate_notify(XEvent * xevent)
{
   Ev_Window_Circulate *e;

   e = NEW(Ev_Window_Circulate, 1);
   e->win = xevent->xcirculate.window;
   e->root = e_window_get_root(e->win);
   if (xevent->xcirculate.place == PlaceOnBottom)
      e->lower = 1;
   else
      e->lower = 0;
   e_add_event(EV_WINDOW_CIRCULATE, e, e_ev_generic_free);
}

static void
e_ev_x_handle_circulate_request(XEvent * xevent)
{
   Ev_Window_Circulate_Request *e;

   e = NEW(Ev_Window_Circulate_Request, 1);
   e->win = xevent->xcirculaterequest.window;
   e->root = e_window_get_root(e->win);
   if (xevent->xcirculaterequest.place == PlaceOnBottom)
      e->lower = 1;
   else
      e->lower = 0;
   e_add_event(EV_WINDOW_CIRCULATE_REQUEST, e, e_ev_generic_free);
}

static void
e_ev_x_handle_property_notify(XEvent * xevent)
{
   Ev_Window_Property *e;

   e = NEW(Ev_Window_Property, 1);
   e->win = xevent->xproperty.window;
   e->root = e_window_get_root(e->win);
   e->atom = xevent->xproperty.atom;
   e->time = xevent->xproperty.time;
   e_add_event(EV_WINDOW_PROPERTY, e, e_ev_generic_free);
}

static void
e_ev_x_handle_colormap_notify(XEvent * xevent)
{
   Ev_Colormap        *e;

   e = NEW(Ev_Colormap, 1);
   e->win = xevent->xcolormap.window;
   e->root = e_window_get_root(e->win);
   e->cmap = xevent->xcolormap.colormap;
   if (xevent->xcolormap.state == ColormapInstalled)
      e->installed = 1;
   else
      e->installed = 0;
   e_add_event(EV_COLORMAP, e, e_ev_generic_free);
}

Ev_Dnd_Drop_Request *ev_drop_request_pending = NULL;

static void
e_ev_x_handle_selection_notify(XEvent * xevent)
{
   Ev_Dnd_Drop_Request *e;
   char               *data;
   int                 size;
   static Atom         atom_xdndactioncopy = 0;
   static Atom         atom_xdndactionmove = 0;
   static Atom         atom_xdndactionlink = 0;
   static Atom         atom_xdndactionask = 0;
   static Atom         atom_xdndactionlist = 0;

   e = ev_drop_request_pending;
   if (!e)
     {
	Ev_Paste_Request *e2;
	
	e2 = NEW(Ev_Paste_Request, 1);	
	e2->string = e_selection_get_data(xevent->xselection.requestor,
					  xevent->xselection.property);
	if (e2->string)
	  {
	     e2->win = xevent->xselection.requestor;
	     e2->root = e_window_get_root(e2->win);
	     e2->source_win = xevent->xselection.requestor;
	     e_add_event(EV_PASTE_REQUEST, e2, e_ev_paste_request_free);
	  }
	else
	  {
	     FREE(e2);
	  }
	return;
     }

   E_ATOM(atom_xdndactioncopy, "XdndActionCopy");
   E_ATOM(atom_xdndactionmove, "XdndActionMove");
   E_ATOM(atom_xdndactionlink, "XdndActionLink");
   E_ATOM(atom_xdndactionask, "XdndActionAsk");
   E_ATOM(atom_xdndactionlist, "XdndActionList");
   data = e_dnd_selection_get(xevent->xany.window, e->source_win,
			      xevent->xselection.property, &size);
   if (data)
     {
	char               *s, *buf;
	int                 i, is;
	Atom               *method = NULL;

	method = e_window_property_get(e->source_win,
				       atom_xdndactionlist, XA_ATOM, &is);
	if (method)
	  {
	     e->copy = 0;
	     e->link = 0;
	     e->move = 0;
	     if (*method == atom_xdndactioncopy)
		e->copy = 1;
	     else if (*method == atom_xdndactionmove)
		e->move = 1;
	     else if (*method == atom_xdndactionlink)
		e->link = 1;
	     FREE(method);
	  }
	else
	  {
	     e->copy = 0;
	     e->link = 0;
	     e->move = 0;
	  }
	s = data;
	buf = NEW(char, size);

	i = 0;
	is = 0;
	e->files = NULL;
	while ((s[is]) && (is < size))
	  {
	     if ((i == 0) && (s[is] == '#'))
	       {
		  for (; ((s[is] != 0) && (s[is] != '\n')); is++);
	       }
	     else
	       {
		  if (s[is] != '\r')
		    {
		       buf[i++] = s[is];
		    }
		  else
		    {
		       buf[i] = 0;
		       e->num_files++;
		       if (e->files)
			 {
			    REALLOC_PTR(e->files, e->num_files);
			 }
		       else
			  e->files = NEW_PTR(e->num_files);
		       e->files[e->num_files - 1] = strdup(buf);
		       buf[0] = 0;
		       i = 0;
		       is++;
		    }
		  is++;
	       }
	  }
	FREE(buf);
	FREE(data);
     }
   e_add_event(EV_DND_DROP_REQUEST, e, e_ev_dnd_drop_request_free);
   ev_drop_request_pending = NULL;
}

static void
e_ev_x_handle_selection_clear(XEvent * xevent)
{
   Ev_Clear_Selection *e;
   
   e = NEW(Ev_Clear_Selection, 1);
   e->win = xevent->xselectionclear.window;
   e->root = e_window_get_root(e->win);
   e->selection = xevent->xselectionclear.selection;
   e_add_event(EV_CLEAR_SELECTION, e, e_ev_generic_free);
}

static void
e_ev_x_handle_selection_request(XEvent * xevent)
{
   static Atom         atom_xdndselection = 0;
   static Atom         atom_text_plain = 0;
   static Atom         atom_text_selection = 0;
   Ev_Dnd_Data_Request *e;

   E_ATOM(atom_xdndselection, "XdndSelection");
   E_ATOM(atom_text_plain, "text/plain");
   E_ATOM(atom_text_selection, "TEXT_SELECTION");
   if (xevent->xselectionrequest.selection == atom_xdndselection)
     {
	e = NEW(Ev_Dnd_Data_Request, 1);
	e->win = xevent->xselectionrequest.owner;
	e->root = e_window_get_root(e->win);
	e->source_win = xevent->xselectionrequest.requestor;
	if (xevent->xselectionrequest.target == atom_text_plain)
	   e->plain_text = 1;
	else
	   e->plain_text = 0;
	e->destination_atom = xevent->xselectionrequest.property;
	e_add_event(EV_DND_DATA_REQUEST, e, e_ev_generic_free);
     }
   else
     {
	XEvent ev;
	Atom target_list[2];
	static Atom xa_targets = None;
	
	if (xa_targets == None)
	  xa_targets = XInternAtom(xevent->xselectionrequest.display, 
				   "TARGETS", False);
	ev.xselection.type = SelectionNotify;
	ev.xselection.property = None;
	ev.xselection.display = xevent->xselectionrequest.display;
	ev.xselection.requestor = xevent->xselectionrequest.requestor;
	ev.xselection.selection = xevent->xselectionrequest.selection;
	ev.xselection.target = xevent->xselectionrequest.target;
	ev.xselection.time = xevent->xselectionrequest.time;
	if (xevent->xselectionrequest.target == xa_targets) 
	  {
	     target_list[0] = (Atom) xa_targets;
	     target_list[1] = (Atom) XA_STRING;
	     XChangeProperty(xevent->xselectionrequest.display, 
			     xevent->xselectionrequest.requestor, 
			     xevent->xselectionrequest.property, 
			     xevent->xselectionrequest.target,
			     (8 * sizeof(target_list[0])), 
			     PropModeReplace,
			     (unsigned char *) target_list,
			     (sizeof(target_list) / sizeof(target_list[0])));
	     ev.xselection.property = xevent->xselectionrequest.property;
	  } 
	else if (xevent->xselectionrequest.target == XA_STRING) 
	  {
	     void *data;
	     int size;
	     
	     data = e_window_property_get(xevent->xselectionrequest.owner,
					  atom_text_selection, XA_STRING,
					  &size);
	     if (data)
	       {
		  XChangeProperty(xevent->xselectionrequest.display, 
				  xevent->xselectionrequest.requestor, 
				  xevent->xselectionrequest.property, 
				  xevent->xselectionrequest.target, 
				  8, 
				  PropModeReplace,
				  data, size);
		  FREE(data);
	       }
	     ev.xselection.property = xevent->xselectionrequest.property;
	  }
	XSendEvent(xevent->xselectionrequest.display, 
		   xevent->xselectionrequest.requestor, False, 0, &ev);
     }
}

static void
e_ev_x_handle_client_message(XEvent * xevent)
{
   static Atom         atom_wm_delete_window = 0;
   static Atom         atom_wm_protocols = 0;
   static Atom         atom_xdndstatus = 0;
   static Atom         atom_xdndenter = 0;
   static Atom         atom_xdndleave = 0;
   static Atom         atom_xdndfinished = 0;
   static Atom         atom_xdndposition = 0;
   static Atom         atom_xdnddrop = 0;
   static Atom         atom_text_uri_list = 0;
   static Atom         atom_xdndactioncopy = 0;
   static Atom         atom_xdndactionlink = 0;
   static Atom         atom_xdndactionmove = 0;

   /* setup some known atoms to translate this message into a sensible event */
   E_ATOM(atom_wm_delete_window, "WM_DELETE_WINDOW");
   E_ATOM(atom_wm_protocols, "WM_PROTOCOLS");
   E_ATOM(atom_xdndstatus, "XdndStatus");
   E_ATOM(atom_xdndfinished, "XdndFinished");
   E_ATOM(atom_xdndenter, "XdndEnter");
   E_ATOM(atom_xdndleave, "XdndLeave");
   E_ATOM(atom_xdnddrop, "XdndDrop");
   E_ATOM(atom_xdndposition, "XdndPosition");
   E_ATOM(atom_xdndactioncopy, "XdndActionCopy");
   E_ATOM(atom_xdndactionlink, "XdndActionLink");
   E_ATOM(atom_xdndactionmove, "XdndActionMove");
   E_ATOM(atom_text_uri_list, "text/uri-list");
   /* forst type = delete event sent to client */
   if ((xevent->xclient.message_type == atom_wm_protocols) &&
       (xevent->xclient.format == 32) &&
       (xevent->xclient.data.l[0] == (long)atom_wm_delete_window))
     {
	Ev_Window_Delete   *e;

	e = NEW(Ev_Window_Delete, 1);
	e->win = xevent->xclient.window;
	e->root = e_window_get_root(e->win);
	e_add_event(EV_WINDOW_DELETE, e, e_ev_generic_free);
     }
   else if ((xevent->xclient.message_type == atom_xdndenter) &&
	    (xevent->xclient.format == 32))
     {
	if (xevent->xclient.data.l[2] == (long)atom_text_uri_list)
	  {
	     Ev_Dnd_Drop_Request *e;

	     if (ev_drop_request_pending)
	       {
		  e_ev_dnd_drop_request_free(ev_drop_request_pending);
		  ev_drop_request_pending = NULL;
	       }
	     e = NEW(Ev_Dnd_Drop_Request, 1);
	     e->win = xevent->xclient.window;
	     e->root = e_window_get_root(e->win);
	     e->source_win = (Window) xevent->xclient.data.l[0];
	     if (!e_dnd_selection_convert
		 (e->source_win, e->win, atom_text_uri_list))
	       {
		  FREE(e);
		  return;
	       }
	     e->files = NULL;
	     e->num_files = 0;
	     ev_drop_request_pending = e;
	  }
     }
   else if ((xevent->xclient.message_type == atom_xdndleave) &&
	    (xevent->xclient.format == 32))
     {
	Ev_Dnd_Drop_End    *e;

	e = NEW(Ev_Dnd_Drop_End, 1);
	e->win = xevent->xclient.window;
	e->root = e_window_get_root(e->win);
	e->source_win = (Window) xevent->xclient.data.l[0];
	e_add_event(EV_DND_DROP_END, e, e_ev_generic_free);
     }
   else if ((xevent->xclient.message_type == atom_xdndposition) &&
	    (xevent->xclient.format == 32))
     {
	Ev_Dnd_Drop_Position *e;

	e = NEW(Ev_Dnd_Drop_Position, 1);
	e->win = xevent->xclient.window;
	e->root = e_window_get_root(e->win);
	e->source_win = (Window) xevent->xclient.data.l[0];
	e->x = (xevent->xclient.data.l[2] >> 16) & 0xffff;
	e->y = xevent->xclient.data.l[2] & 0xffff;
	e_add_event(EV_DND_DROP_POSITION, e, e_ev_generic_free);
     }
   else if ((xevent->xclient.message_type == atom_xdndstatus) &&
	    (xevent->xclient.format == 32))
     {
	Ev_Dnd_Drop_Status *e;

	e = NEW(Ev_Dnd_Drop_Status, 1);
	e->win = xevent->xclient.window;
	e->root = e_window_get_root(e->win);
	e->source_win = (Window) xevent->xclient.data.l[0];
	e->x = (xevent->xclient.data.l[2] >> 16) & 0xffff;
	e->y = xevent->xclient.data.l[2] & 0xffff;
	e->w = (xevent->xclient.data.l[3] >> 16) & 0xffff;
	e->h = xevent->xclient.data.l[3] & 0xffff;
	if (xevent->xclient.data.l[1] & 0x1)
	   e->ok = 1;
	else
	   e->ok = 0;
	e_add_event(EV_DND_DROP_STATUS, e, e_ev_generic_free);
     }
   else if ((xevent->xclient.message_type == atom_xdndfinished) &&
	    (xevent->xclient.format == 32))
     {
	Ev_Dnd_Drop_End    *e;

	e = NEW(Ev_Dnd_Drop_End, 1);
	e->win = xevent->xclient.window;
	e->root = e_window_get_root(e->win);
	e->source_win = (Window) xevent->xclient.data.l[0];
	e_add_event(EV_DND_DROP_END, e, e_ev_generic_free);
     }
   else if ((xevent->xclient.message_type == atom_xdnddrop) &&
	    (xevent->xclient.format == 32))
     {
	Ev_Dnd_Drop        *e;

	e = NEW(Ev_Dnd_Drop, 1);
	e->win = xevent->xclient.window;
	e->root = e_window_get_root(e->win);
	e->source_win = (Window) xevent->xclient.data.l[0];
	e_add_event(EV_DND_DROP, e, e_ev_generic_free);
     }
   else
     {
	Ev_Message         *e;

	e = NEW(Ev_Message, 1);
	e->win = xevent->xclient.window;
	e->format = xevent->xclient.format;
	e->atom = xevent->xclient.message_type;
	MEMCPY(xevent->xclient.data.b, e->data.b, char, 20);

	e_add_event(EV_MESSAGE, e, e_ev_generic_free);
     }
}

static void
e_ev_x_handle_shape_change(XEvent * xevent)
{
   Ev_Window_Shape    *e;
   XShapeEvent        *shape_event;

   shape_event = (XShapeEvent *) xevent;
   e = NEW(Ev_Window_Shape, 1);
   e->win = shape_event->window;
   e->root = e_window_get_root(e->win);
   e->time = shape_event->time;
   e_add_event(EV_WINDOW_SHAPE, e, e_ev_generic_free);
}

char               *
e_key_press_translate_into_typeable(Ev_Key_Down * e)
{
   /* exceptions */
   if ((!strcmp(e->key, "Delete")) ||
       (!strcmp(e->key, "BackSpace")) ||
       (!strcmp(e->key, "Tab")) ||
       (!strcmp(e->key, "Escape")) ||
       (!strcmp(e->key, "Return")) ||
       (!strcmp(e->key, "KP_Enter")) ||
       (!strcmp(e->key, "Enter")) ||
       (!strcmp(e->key, "KP_Divide")) ||
       (!strcmp(e->key, "KP_Multiply")) ||
       (!strcmp(e->key, "KP_Subtract")) ||
       (!strcmp(e->key, "KP_Add")) || 
       (!strcmp(e->key, "Enter")))
      return NULL;
   return e->compose;
}
