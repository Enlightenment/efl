#include "Ecore.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* private funtion prototypes */
static void         ecore_event_x_handle_events(int fd);
static void         ecore_event_x_translate_events(XEvent * events,
						   int num_events);

static void         ecore_event_key_down_free(void *event);
static void         ecore_event_key_up_free(void *event);
static void         ecore_event_generic_free(void *event);
static void         ecore_event_dnd_drop_request_free(void *event);
static void         ecore_event_paste_request_free(void *event);

static void         ecore_event_x_handle_keypress(XEvent * xevent);
static void         ecore_event_x_handle_keyrelease(XEvent * xevent);
static void         ecore_event_x_handle_button_press(XEvent * xevent);
static void         ecore_event_x_handle_button_release(XEvent * xevent);
static void         ecore_event_x_handle_motion_notify(XEvent * xevent);
static void         ecore_event_x_handle_enter_notify(XEvent * xevent);
static void         ecore_event_x_handle_leave_notify(XEvent * xevent);
static void         ecore_event_x_handle_focus_in(XEvent * xevent);
static void         ecore_event_x_handle_focus_out(XEvent * xevent);
static void         ecore_event_x_handle_expose(XEvent * xevent);
static void         ecore_event_x_handle_visibility_notify(XEvent * xevent);
static void         ecore_event_x_handle_create_notify(XEvent * xevent);
static void         ecore_event_x_handle_destroy_notify(XEvent * xevent);
static void         ecore_event_x_handle_unmap_notify(XEvent * xevent);
static void         ecore_event_x_handle_map_notify(XEvent * xevent);
static void         ecore_event_x_handle_map_request(XEvent * xevent);
static void         ecore_event_x_handle_reparent_notify(XEvent * xevent);
static void         ecore_event_x_handle_configure_notify(XEvent * xevent);
static void         ecore_event_x_handle_configure_request(XEvent * xevent);
static void         ecore_event_x_handle_circulate_notify(XEvent * xevent);
static void         ecore_event_x_handle_circulate_request(XEvent * xevent);
static void         ecore_event_x_handle_property_notify(XEvent * xevent);
static void         ecore_event_x_handle_colormap_notify(XEvent * xevent);
static void         ecore_event_x_handle_selection_notify(XEvent * xevent);
static void         ecore_event_x_handle_selection_clear(XEvent * xevent);
static void         ecore_event_x_handle_selection_request(XEvent * xevent);
static void         ecore_event_x_handle_client_message(XEvent * xevent);
static void         ecore_event_x_handle_shape_change(XEvent * xevent);

static int          max_event_id = 0;
static void         (**event_translator) (XEvent * event) = NULL;

static int          lock_mask_scroll = 0, lock_mask_num = 0, lock_mask_caps = 0;
static int          mod_mask_shift = 0, mod_mask_ctrl = 0, mod_mask_alt =
  0, mod_mask_win = 0;

/* convenience macros */
#define GETSET_MODS(state, mods) \
(mods) = ECORE_EVENT_KEY_MODIFIER_NONE;\
if ((state) & mod_mask_shift) { ecore_mod_shift_set(1); (mods) |= ECORE_EVENT_KEY_MODIFIER_SHIFT; }\
else                                       ecore_mod_shift_set(0);\
if ((state) & mod_mask_ctrl)  { ecore_mod_ctrl_set(1);  (mods) |= ECORE_EVENT_KEY_MODIFIER_CTRL; }\
else                                       ecore_mod_ctrl_set(0);\
if ((state) & mod_mask_alt)   { ecore_mod_alt_set(1);   (mods) |= ECORE_EVENT_KEY_MODIFIER_ALT; }\
else                                       ecore_mod_alt_set(0);\
if ((state) & mod_mask_win)   { ecore_mod_win_set(1);   (mods) |= ECORE_EVENT_KEY_MODIFIER_WIN; }\
else                                       ecore_mod_win_set(0);

/* public functions */

/* initialise event handling for the fd X is on */
void
ecore_event_x_init(void)
{
  int                 i, shape_event_id, current_lock;

  shape_event_id = max_event_id = ecore_event_shape_get_id();
  if (shape_event_id < LASTEvent)
    {
       max_event_id = LASTEvent;
       fprintf(stderr, "ERROR: No shape extension! This is BAD!\n");
    }
  event_translator = NEW_PTR(max_event_id + 1);
  for (i = 0; i < max_event_id + 1; i++)
    event_translator[i] = NULL;
  event_translator[KeyPress] = ecore_event_x_handle_keypress;
  event_translator[KeyRelease] = ecore_event_x_handle_keyrelease;
  event_translator[ButtonPress] = ecore_event_x_handle_button_press;
  event_translator[ButtonRelease] = ecore_event_x_handle_button_release;
  event_translator[MotionNotify] = ecore_event_x_handle_motion_notify;
  event_translator[EnterNotify] = ecore_event_x_handle_enter_notify;
  event_translator[LeaveNotify] = ecore_event_x_handle_leave_notify;
  event_translator[FocusIn] = ecore_event_x_handle_focus_in;
  event_translator[FocusOut] = ecore_event_x_handle_focus_out;
  event_translator[Expose] = ecore_event_x_handle_expose;
  event_translator[VisibilityNotify] = ecore_event_x_handle_visibility_notify;
  event_translator[CreateNotify] = ecore_event_x_handle_create_notify;
  event_translator[DestroyNotify] = ecore_event_x_handle_destroy_notify;
  event_translator[UnmapNotify] = ecore_event_x_handle_unmap_notify;
  event_translator[MapNotify] = ecore_event_x_handle_map_notify;
  event_translator[MapRequest] = ecore_event_x_handle_map_request;
  event_translator[ReparentNotify] = ecore_event_x_handle_reparent_notify;
  event_translator[ConfigureNotify] = ecore_event_x_handle_configure_notify;
  event_translator[ConfigureRequest] = ecore_event_x_handle_configure_request;
  event_translator[CirculateNotify] = ecore_event_x_handle_circulate_notify;
  event_translator[CirculateRequest] = ecore_event_x_handle_circulate_request;
  event_translator[PropertyNotify] = ecore_event_x_handle_property_notify;
  event_translator[ColormapNotify] = ecore_event_x_handle_colormap_notify;
  event_translator[ClientMessage] = ecore_event_x_handle_client_message;
  event_translator[SelectionNotify] = ecore_event_x_handle_selection_notify;
  event_translator[SelectionClear] = ecore_event_x_handle_selection_clear;
  event_translator[SelectionRequest] = ecore_event_x_handle_selection_request;
  if (shape_event_id > SelectionRequest)
     event_translator[shape_event_id] = ecore_event_x_handle_shape_change;

  lock_mask_scroll = ecore_lock_mask_scroll_get();
  lock_mask_num = ecore_lock_mask_num_get();
  lock_mask_caps = ecore_lock_mask_caps_get();

  mod_mask_shift = ecore_mod_mask_shift_get();
  mod_mask_ctrl = ecore_mod_mask_ctrl_get();
  mod_mask_alt = ecore_mod_mask_alt_get();
  mod_mask_win = ecore_mod_mask_win_get();

/* HRRRMMM lets not do this  
   ecorecore_keygrab("Num_Lock", ECORE_EVENT_KEY_MODIFIER_NONE, 1, 1);
   ecorecore_keygrab("Scroll_Lock", ECORE_EVENT_KEY_MODIFIER_NONE, 1, 1);
   ecorecore_keygrab("Caps_Lock", ECORE_EVENT_KEY_MODIFIER_NONE, 1, 1);
 */
  current_lock = ecore_modifier_mask_get();
  if (current_lock & lock_mask_scroll)
    ecore_lock_scroll_set(1);
  if (current_lock & lock_mask_num)
    ecore_lock_num_set(1);
  if (current_lock & lock_mask_caps)
    ecore_lock_caps_set(1);
  if (current_lock & mod_mask_shift)
    ecore_mod_shift_set(1);
  if (current_lock & mod_mask_ctrl)
    ecore_mod_ctrl_set(1);
  if (current_lock & mod_mask_alt)
    ecore_mod_alt_set(1);
  if (current_lock & mod_mask_win)
    ecore_mod_win_set(1);
  ecore_add_event_fd(ecore_x_get_fd(), ecore_event_x_handle_events);
}

/* private functions */
/* get all events onthe event queue and translate them */
static void
ecore_event_x_handle_events(int fd)
{
  int                 num_events = 0, size_events = 0;
  XEvent             *events = NULL;

  /* while there are events in the queue */
  while (ecore_events_pending())
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
	    REALLOC(events, XEvent, size_events)}
	  else
	    events = NEW(XEvent, size_events);
	}
      /* get the next event into the event buffer */
      ecore_get_next_event(&(events[num_events - 1]));
    }
  /* call the XEvent -> Eevent translator */
  if (events)
    {
      ecore_event_x_translate_events(events, num_events);
      /* if theres an event buffer - free it */
      FREE(events);
    }
  return;
  fd = 0;
}

/* take an array of events and translate them into E events */
static void
ecore_event_x_translate_events(XEvent * events, int num_events)
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
ecore_event_key_down_free(void *event)
{
  Ecore_Event_Key_Down *e;

  e = (Ecore_Event_Key_Down *) event;
  IF_FREE(e->key);
  IF_FREE(e->compose);
  FREE(e);
}

static void
ecore_event_key_up_free(void *event)
{
  Ecore_Event_Key_Up *e;

  e = (Ecore_Event_Key_Up *) event;
  IF_FREE(e->key);
  IF_FREE(e->compose);
  FREE(e);
}

static void
ecore_event_generic_free(void *event)
{
  FREE(event);
}

static void
ecore_event_dnd_drop_request_free(void *event)
{
  Ecore_Event_Dnd_Drop_Request *e;

  e = (Ecore_Event_Dnd_Drop_Request *) event;
  if (e->files)
    {
      int                 i;

      for (i = 0; i < e->num_files; i++)
	FREE(e->files[i]);
    }
  FREE(event);
}

static void
ecore_event_paste_request_free(void *event)
{
  Ecore_Event_Paste_Request *e;

  e = (Ecore_Event_Paste_Request *) event;
  IF_FREE(e->string);
  FREE(event);
}

static void
ecore_event_x_handle_keypress(XEvent * xevent)
{
  Ecore_Event_Key_Down *e;
  static KeyCode      previous_code = 0;
  static Time         previous_time = 0;

  /* avoid doubling events up from passive grabs */
  if ((xevent->xkey.keycode == previous_code) &&
      xevent->xkey.time == previous_time)
    return;
  previous_code = xevent->xkey.keycode;
  previous_time = xevent->xkey.time;
/*   
   if (ecore_keyget_keysym_from_keycode(xevent->xkey.keycode) == XK_Scroll_Lock)
     {
	if (ecore_lock_scroll_get())
	   ecore_lock_scroll_set(0);
	else
	   ecore_lock_scroll_set(1);
	e_event_allow(ReplayKeyboard, xevent->xkey.time);
	ecore_flush();
     }
   else if (ecore_keyget_keysym_from_keycode(xevent->xkey.keycode) == XK_Num_Lock)
     {
	if (ecore_lock_num_get())
	   ecore_lock_num_set(0);
	else
	   ecore_lock_num_set(1);
	e_event_allow(ReplayKeyboard, xevent->xkey.time);
	ecore_flush();
     }
   else if (ecore_keyget_keysym_from_keycode(xevent->xkey.keycode) == XK_Caps_Lock)
     {
	if (ecore_lock_caps_get())
	   ecore_lock_caps_set(0);
	else
	   ecore_lock_caps_set(1);
	e_event_allow(ReplayKeyboard, xevent->xkey.time);
	ecore_flush();
     }
 */
  e = NEW(Ecore_Event_Key_Down, 1);
  e->win = xevent->xkey.window;
  e->root = xevent->xkey.root;
  GETSET_MODS(xevent->xkey.state, e->mods);
  e->time = xevent->xkey.time;
  e->key = ecore_key_get_string_from_keycode(xevent->xkey.keycode);
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
  ecore_add_event(ECORE_EVENT_KEY_DOWN, e, ecore_event_key_down_free);
}

static void
ecore_event_x_handle_keyrelease(XEvent * xevent)
{
  Ecore_Event_Key_Up *e;
  static KeyCode      previous_code = 0;
  static Time         previous_time = 0;

  /* avoid doubling events up from passive grabs */
  if ((xevent->xkey.keycode == previous_code) &&
      xevent->xkey.time == previous_time)
    return;
  previous_code = xevent->xkey.keycode;
  previous_time = xevent->xkey.time;

  e = NEW(Ecore_Event_Key_Up, 1);
  e->win = xevent->xkey.window;
  e->root = xevent->xkey.root;
  GETSET_MODS(xevent->xkey.state, e->mods);
  e->time = xevent->xkey.time;
  e->key = ecore_key_get_string_from_keycode(xevent->xkey.keycode);
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
  ecore_add_event(ECORE_EVENT_KEY_UP, e, ecore_event_key_up_free);
}

static void
ecore_event_x_handle_button_press(XEvent * xevent)
{
  static Time         last_time = 0, last_last_time = 0;
  static int          last_button = 0, last_last_button = 0;
  static Window       last_window = 0, last_last_window = 0;

  ecore_pointer_xy_set(xevent->xbutton.x_root, xevent->xbutton.y_root);
  if ((xevent->xbutton.button == 4) || (xevent->xbutton.button == 5))
    {
      Ecore_Event_Wheel  *e;

      e = NEW(Ecore_Event_Wheel, 1);
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
      ecore_add_event(ECORE_EVENT_MOUSE_WHEEL, e, ecore_event_generic_free);
    }
  else
    {
      Ecore_Event_Mouse_Down *e;

      e = NEW(Ecore_Event_Mouse_Down, 1);
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
      ecore_add_event(ECORE_EVENT_MOUSE_DOWN, e, ecore_event_generic_free);
        {
          Ecore_XID              *xid = NULL;

          if (XFindContext(xevent->xbutton.display, e->win,
                           xid_context, (XPointer *) & xid) != XCNOENT)
            {
              if ((xid->grab_button_auto_replay) &&
		  (xid->grab_button_auto_replay(e)))
                {
                   ecore_pointer_replay(e->time);
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
ecore_event_x_handle_button_release(XEvent * xevent)
{
  Ecore_Event_Mouse_Up *e;

  if (xevent->xbutton.button > 3)
    return;

  e = NEW(Ecore_Event_Mouse_Up, 1);
  e->win = xevent->xbutton.window;
  e->root = xevent->xbutton.root;
  e->button = xevent->xbutton.button;
  e->x = xevent->xbutton.x;
  e->y = xevent->xbutton.y;
  e->rx = xevent->xbutton.x_root;
  e->ry = xevent->xbutton.y_root;
  e->time = xevent->xbutton.time;
  GETSET_MODS(xevent->xbutton.state, e->mods);
  ecore_add_event(ECORE_EVENT_MOUSE_UP, e, ecore_event_generic_free);
}

static void
ecore_event_x_handle_motion_notify(XEvent * xevent)
{
  Ecore_Event_Mouse_Move *e;

  ecore_pointer_xy_set(xevent->xmotion.x_root, xevent->xmotion.y_root);
  e = NEW(Ecore_Event_Mouse_Move, 1);
  e->win = xevent->xmotion.window;
  e->root = xevent->xmotion.root;
  e->x = xevent->xmotion.x;
  e->y = xevent->xmotion.y;
  e->rx = xevent->xmotion.x_root;
  e->ry = xevent->xmotion.y_root;
  e->time = xevent->xmotion.time;
  GETSET_MODS(xevent->xmotion.state, e->mods);
  ecore_add_event(ECORE_EVENT_MOUSE_MOVE, e, ecore_event_generic_free);
}

static void
ecore_event_x_handle_enter_notify(XEvent * xevent)
{
  Ecore_Event_Window_Enter *e;

/*   if ((xevent->xcrossing.mode == NotifyGrab) || (xevent->xcrossing.mode == NotifyUngrab)) return;*/
  ecore_pointer_xy_set(xevent->xcrossing.x_root, xevent->xcrossing.y_root);
  e = NEW(Ecore_Event_Window_Enter, 1);
  e->win = xevent->xcrossing.window;
  e->root = xevent->xcrossing.root;
  e->x = xevent->xcrossing.x;
  e->y = xevent->xcrossing.y;
  e->rx = xevent->xcrossing.x_root;
  e->ry = xevent->xcrossing.y_root;
  e->time = xevent->xcrossing.time;
  GETSET_MODS(xevent->xcrossing.state, e->mods);
  ecore_add_event(ECORE_EVENT_MOUSE_IN, e, ecore_event_generic_free);
  ecore_window_mouse_set_in(e->win, 1);
  {
    Ecore_Event_Mouse_Move *e;

    e = NEW(Ecore_Event_Mouse_Move, 1);
    e->win = xevent->xcrossing.window;
    e->root = xevent->xcrossing.root;
    e->x = xevent->xcrossing.x;
    e->y = xevent->xcrossing.y;
    e->rx = xevent->xcrossing.x_root;
    e->ry = xevent->xcrossing.y_root;
    e->time = xevent->xcrossing.time;
    GETSET_MODS(xevent->xcrossing.state, e->mods);
    ecore_add_event(ECORE_EVENT_MOUSE_MOVE, e, ecore_event_generic_free);
  }
}

static void
ecore_event_x_handle_leave_notify(XEvent * xevent)
{
  Ecore_Event_Window_Leave *e;

/*   if ((xevent->xcrossing.mode == NotifyGrab) || (xevent->xcrossing.mode == NotifyUngrab)) return;*/
  ecore_pointer_xy_set(xevent->xcrossing.x_root, xevent->xcrossing.y_root);
  {
    Ecore_Event_Mouse_Move *e;

    e = NEW(Ecore_Event_Mouse_Move, 1);
    e->win = xevent->xcrossing.window;
    e->root = xevent->xcrossing.root;
    e->x = xevent->xcrossing.x;
    e->y = xevent->xcrossing.y;
    e->rx = xevent->xcrossing.x_root;
    e->ry = xevent->xcrossing.y_root;
    e->time = xevent->xcrossing.time;
    GETSET_MODS(xevent->xcrossing.state, e->mods);
    ecore_add_event(ECORE_EVENT_MOUSE_MOVE, e, ecore_event_generic_free);
  }
  e = NEW(Ecore_Event_Window_Leave, 1);
  e->win = xevent->xcrossing.window;
  e->root = xevent->xcrossing.root;
  e->x = xevent->xcrossing.x;
  e->y = xevent->xcrossing.y;
  e->rx = xevent->xcrossing.x_root;
  e->ry = xevent->xcrossing.y_root;
  e->time = xevent->xcrossing.time;
  GETSET_MODS(xevent->xcrossing.state, e->mods);
  ecore_add_event(ECORE_EVENT_MOUSE_OUT, e, ecore_event_generic_free);
  ecore_window_mouse_set_in(e->win, 0);
}

static void
ecore_event_x_handle_focus_in(XEvent * xevent)
{
  Ecore_Event_Window_Focus_In *e;

  e = NEW(Ecore_Event_Window_Focus_In, 1);
  e->win = xevent->xfocus.window;
  e->root = ecore_window_get_root(e->win);
  if (xevent->xfocus.mode != NotifyNormal)
    e->key_grab = 1;
  else
    e->key_grab = 0;
  ecore_add_event(ECORE_EVENT_WINDOW_FOCUS_IN, e, ecore_event_generic_free);
  ecore_focus_window_set(e->win);
}

static void
ecore_event_x_handle_focus_out(XEvent * xevent)
{
  Ecore_Event_Window_Focus_Out *e;

  e = NEW(Ecore_Event_Window_Focus_Out, 1);
  e->win = xevent->xfocus.window;
  e->root = ecore_window_get_root(e->win);
  if (xevent->xfocus.mode != NotifyNormal)
    e->key_grab = 1;
  else
    e->key_grab = 0;
  ecore_add_event(ECORE_EVENT_WINDOW_FOCUS_OUT, e, ecore_event_generic_free);
  ecore_focus_window_set(0);
}

static void
ecore_event_x_handle_expose(XEvent * xevent)
{
  Ecore_Event_Window_Expose *e;

  e = NEW(Ecore_Event_Window_Expose, 1);
  e->win = xevent->xexpose.window;
  e->root = ecore_window_get_root(e->win);
  e->x = xevent->xexpose.x;
  e->y = xevent->xexpose.y;
  e->w = xevent->xexpose.width;
  e->h = xevent->xexpose.height;
  ecore_add_event(ECORE_EVENT_WINDOW_EXPOSE, e, ecore_event_generic_free);
}

static void
ecore_event_x_handle_visibility_notify(XEvent * xevent)
{
  if (xevent->xvisibility.state != VisibilityPartiallyObscured)
    {
      Ecore_Event_Window_Visibility *e;

      e = NEW(Ecore_Event_Window_Visibility, 1);
      e->win = xevent->xvisibility.window;
      e->root = ecore_window_get_root(e->win);
      if (xevent->xvisibility.state == VisibilityFullyObscured)
	e->fully_obscured = 1;
      else
	e->fully_obscured = 0;
      ecore_add_event(ECORE_EVENT_WINDOW_VISIBILITY, e,
		      ecore_event_generic_free);
    }
}

static void
ecore_event_x_handle_create_notify(XEvent * xevent)
{
  Ecore_Event_Window_Create *e;

  e = NEW(Ecore_Event_Window_Create, 1);
  e->win = xevent->xcreatewindow.window;
  ecore_validate_xid(e->win);
  e->root = ecore_window_get_root(e->win);
  if (xevent->xcreatewindow.override_redirect)
    e->override = 1;
  else
    e->override = 0;
  ecore_add_event(ECORE_EVENT_WINDOW_CREATE, e, ecore_event_generic_free);
}

static void
ecore_event_x_handle_destroy_notify(XEvent * xevent)
{
  Ecore_Event_Window_Destroy *e;

  e = NEW(Ecore_Event_Window_Destroy, 1);
  e->win = xevent->xdestroywindow.window;
  e->root = ecore_window_get_root(e->win);
  ecore_add_event(ECORE_EVENT_WINDOW_DESTROY, e, ecore_event_generic_free);
  ecore_unvalidate_xid(e->win);
}

static void
ecore_event_x_handle_unmap_notify(XEvent * xevent)
{
  Ecore_Event_Window_Unmap *e;
  Ecore_XID          *xid = NULL;

  e = NEW(Ecore_Event_Window_Unmap, 1);
  e->win = xevent->xunmap.window;
  e->root = ecore_window_get_root(e->win);
  ecore_add_event(ECORE_EVENT_WINDOW_UNMAP, e, ecore_event_generic_free);
  xid = ecore_validate_xid(e->win);
  if (xid)
    xid->mapped = 0;
}

static void
ecore_event_x_handle_map_notify(XEvent * xevent)
{
  Ecore_Event_Window_Map *e;
  Ecore_XID          *xid = NULL;

  e = NEW(Ecore_Event_Window_Map, 1);
  e->win = xevent->xmap.window;
  e->root = ecore_window_get_root(e->win);
  ecore_add_event(ECORE_EVENT_WINDOW_MAP, e, ecore_event_generic_free);
  xid = ecore_validate_xid(e->win);
  if (xid)
    xid->mapped = 1;
}

static void
ecore_event_x_handle_map_request(XEvent * xevent)
{
  Ecore_Event_Window_Map_Request *e;

  e = NEW(Ecore_Event_Window_Map_Request, 1);
  e->win = xevent->xmaprequest.window;
  e->root = ecore_window_get_root(e->win);
  ecore_add_event(ECORE_EVENT_WINDOW_MAP_REQUEST, e, ecore_event_generic_free);
  ecore_validate_xid(e->win);
}

static void
ecore_event_x_handle_reparent_notify(XEvent * xevent)
{
  Ecore_Event_Window_Reparent *e;
  Window              parent;
  Ecore_XID          *xid = NULL;

  e = NEW(Ecore_Event_Window_Reparent, 1);
  e->win = xevent->xreparent.window;
  xid = ecore_validate_xid(e->win);
  e->root = ecore_window_get_root(e->win);
  parent = ecore_window_get_parent(e->win);
  e->parent_from = parent;
  e->parent = xevent->xreparent.parent;
  ecore_validate_xid(e->parent);
  ecore_del_child(parent, e->win);
  ecore_add_child(xevent->xreparent.parent, xevent->xreparent.window);
  if (xid)
    xid->parent = e->parent;
  ecore_add_event(ECORE_EVENT_WINDOW_REPARENT, e, ecore_event_generic_free);
}

static void
ecore_event_x_handle_configure_notify(XEvent * xevent)
{
  Ecore_Event_Window_Configure *e;
  Ecore_XID          *xid;

  e = NEW(Ecore_Event_Window_Configure, 1);
  e->win = xevent->xconfigure.window;
  e->root = ecore_window_get_root(e->win);
  e->x = xevent->xconfigure.x;
  e->y = xevent->xconfigure.y;
  e->w = xevent->xconfigure.width;
  e->h = xevent->xconfigure.height;
  if (!xevent->xconfigure.send_event)
    {
      xid = ecore_validate_xid(e->win);
      if (xid)
	{
	  xid->x = e->x;
	  xid->y = e->y;
	  xid->w = e->w;
	  xid->h = e->h;
	}
      e->wm_generated = 0;
      /* FIXME: don't handle redoing stack for xevent->xconfigure.above */
      /* member (the window is stacked immediately in stack above this) */
    }
  else
    e->wm_generated = 1;
  ecore_add_event(ECORE_EVENT_WINDOW_CONFIGURE, e, ecore_event_generic_free);
}

static void
ecore_event_x_handle_configure_request(XEvent * xevent)
{
  Ecore_Event_Window_Configure_Request *e;

  e = NEW(Ecore_Event_Window_Configure_Request, 1);
  e->win = xevent->xconfigurerequest.window;
  e->root = ecore_window_get_root(e->win);
  e->x = xevent->xconfigurerequest.x;
  e->y = xevent->xconfigurerequest.y;
  e->w = xevent->xconfigurerequest.width;
  e->h = xevent->xconfigurerequest.height;
  e->stack_win = xevent->xconfigurerequest.above;
  e->detail = xevent->xconfigurerequest.detail;
  e->mask = xevent->xconfigurerequest.value_mask;
  ecore_add_event(ECORE_EVENT_WINDOW_CONFIGURE_REQUEST, e,
		  ecore_event_generic_free);
}

static void
ecore_event_x_handle_circulate_notify(XEvent * xevent)
{
  Ecore_Event_Window_Circulate *e;

  e = NEW(Ecore_Event_Window_Circulate, 1);
  e->win = xevent->xcirculate.window;
  e->root = ecore_window_get_root(e->win);
  if (xevent->xcirculate.place == PlaceOnBottom)
    e->lower = 1;
  else
    e->lower = 0;
  ecore_add_event(ECORE_EVENT_WINDOW_CIRCULATE, e, ecore_event_generic_free);
}

static void
ecore_event_x_handle_circulate_request(XEvent * xevent)
{
  Ecore_Event_Window_Circulate_Request *e;

  e = NEW(Ecore_Event_Window_Circulate_Request, 1);
  e->win = xevent->xcirculaterequest.window;
  e->root = ecore_window_get_root(e->win);
  if (xevent->xcirculaterequest.place == PlaceOnBottom)
    e->lower = 1;
  else
    e->lower = 0;
  ecore_add_event(ECORE_EVENT_WINDOW_CIRCULATE_REQUEST, e,
		  ecore_event_generic_free);
}

static void
ecore_event_x_handle_property_notify(XEvent * xevent)
{
  Ecore_Event_Window_Property *e;

  e = NEW(Ecore_Event_Window_Property, 1);
  e->win = xevent->xproperty.window;
  e->root = ecore_window_get_root(e->win);
  e->atom = xevent->xproperty.atom;
  e->time = xevent->xproperty.time;
  ecore_add_event(ECORE_EVENT_WINDOW_PROPERTY, e, ecore_event_generic_free);
}

static void
ecore_event_x_handle_colormap_notify(XEvent * xevent)
{
  Ecore_Event_Colormap *e;

  e = NEW(Ecore_Event_Colormap, 1);
  e->win = xevent->xcolormap.window;
  e->root = ecore_window_get_root(e->win);
  e->cmap = xevent->xcolormap.colormap;
  if (xevent->xcolormap.state == ColormapInstalled)
    e->installed = 1;
  else
    e->installed = 0;
  ecore_add_event(ECORE_EVENT_COLORMAP, e, ecore_event_generic_free);
}

Ecore_Event_Dnd_Drop_Request *ev_drop_request_pending = NULL;

static void
ecore_event_x_handle_selection_notify(XEvent * xevent)
{
  Ecore_Event_Dnd_Drop_Request *e;
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
      Ecore_Event_Paste_Request *e2;

      e2 = NEW(Ecore_Event_Paste_Request, 1);
      e2->string = ecore_selection_get_data(xevent->xselection.requestor,
					    xevent->xselection.property);
      if (e2->string)
	{
	  e2->win = xevent->xselection.requestor;
	  e2->root = ecore_window_get_root(e2->win);
	  e2->source_win = xevent->xselection.requestor;
	  ecore_add_event(ECORE_EVENT_PASTE_REQUEST, e2,
			  ecore_event_paste_request_free);
	}
      else
	{
	  FREE(e2);
	}
      return;
    }

  ECORE_ATOM(atom_xdndactioncopy, "XdndActionCopy");
  ECORE_ATOM(atom_xdndactionmove, "XdndActionMove");
  ECORE_ATOM(atom_xdndactionlink, "XdndActionLink");
  ECORE_ATOM(atom_xdndactionask, "XdndActionAsk");
  ECORE_ATOM(atom_xdndactionlist, "XdndActionList");
  data = ecore_dnd_selection_get(xevent->xany.window, e->source_win,
				 xevent->xselection.property, &size);
  if (data)
    {
      char               *s, *buf;
      int                 i, is;
      Atom               *method = NULL;

      method = ecore_window_property_get(e->source_win,
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
		  REALLOC_PTR(e->files, e->num_files);
		  e->files[e->num_files - 1] = strdup(buf);
		  buf[0] = 0;
		  i = 0;
		  is++;
		}
	      is++;
	    }
	}
      if (i > 0)
	 {
	    buf[i] = 0;
	    e->num_files++;
	    REALLOC_PTR(e->files, e->num_files);
	    e->files[e->num_files - 1] = strdup(buf);
	 }
      FREE(buf);
      FREE(data);
    }
  ecore_add_event(ECORE_EVENT_DND_DROP_REQUEST, e,
		  ecore_event_dnd_drop_request_free);
  ev_drop_request_pending = NULL;
}

static void
ecore_event_x_handle_selection_clear(XEvent * xevent)
{
  Ecore_Event_Clear_Selection *e;

  e = NEW(Ecore_Event_Clear_Selection, 1);
  e->win = xevent->xselectionclear.window;
  e->root = ecore_window_get_root(e->win);
  e->selection = xevent->xselectionclear.selection;
  ecore_add_event(ECORE_EVENT_CLEAR_SELECTION, e, ecore_event_generic_free);
}

static void
ecore_event_x_handle_selection_request(XEvent * xevent)
{
  static Atom         atom_xdndselection = 0;
  static Atom         atom_text_plain = 0;
  static Atom         atom_text_uri_list = 0;
  static Atom         atom_text_moz_url = 0;
  static Atom         atom_netscape_url = 0;
  static Atom         atom_text_selection = 0;
  Ecore_Event_Dnd_Data_Request *e;

  ECORE_ATOM(atom_xdndselection, "XdndSelection");
  ECORE_ATOM(atom_text_plain, "text/plain");
  ECORE_ATOM(atom_text_uri_list, "text/uri-list");
  ECORE_ATOM(atom_text_moz_url, "text/x-moz-url");
  ECORE_ATOM(atom_netscape_url, "_NETSCAPE_URL");
  ECORE_ATOM(atom_text_selection, "TEXT_SELECTION");
  if (xevent->xselectionrequest.selection == atom_xdndselection)
    {
      e = NEW(Ecore_Event_Dnd_Data_Request, 1);
      e->win = xevent->xselectionrequest.owner;
      e->root = ecore_window_get_root(e->win);
      e->source_win = xevent->xselectionrequest.requestor;
      e->plain_text =0;
      e->uri_list = 0;
      e->moz_url = 0;
      e->netscape_url = 0;
      
      if (xevent->xselectionrequest.target == atom_text_plain) e->plain_text = 1;
      if (xevent->xselectionrequest.target == atom_text_uri_list) e->uri_list = 1;
      if (xevent->xselectionrequest.target == atom_text_moz_url) e->moz_url = 1;
      if (xevent->xselectionrequest.target == atom_netscape_url) e->netscape_url = 1;
      e->destination_atom = xevent->xselectionrequest.property;
      ecore_add_event(ECORE_EVENT_DND_DATA_REQUEST, e,
		      ecore_event_generic_free);
    }
  else
    {
      XEvent              ev;
      Atom                target_list[2];
      static Atom         xa_targets = None;

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
			  (unsigned char *)target_list,
			  (sizeof(target_list) / sizeof(target_list[0])));
	  ev.xselection.property = xevent->xselectionrequest.property;
	}
      else if (xevent->xselectionrequest.target == XA_STRING)
	{
	  void               *data;
	  int                 size;

	  data = ecore_window_property_get(xevent->xselectionrequest.owner,
					   atom_text_selection, XA_STRING,
					   &size);
	  if (data)
	    {
	      XChangeProperty(xevent->xselectionrequest.display,
			      xevent->xselectionrequest.requestor,
			      xevent->xselectionrequest.property,
			      xevent->xselectionrequest.target,
			      8, PropModeReplace, data, size);
	      FREE(data);
	    }
	  ev.xselection.property = xevent->xselectionrequest.property;
	}
      XSendEvent(xevent->xselectionrequest.display,
		 xevent->xselectionrequest.requestor, False, 0, &ev);
    }
}

static void
ecore_event_x_handle_client_message(XEvent * xevent)
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
  static Atom         atom_xdndactionprivate = 0;

  /* setup some known atoms to translate this message into a sensible event */
  ECORE_ATOM(atom_wm_delete_window, "WM_DELETE_WINDOW");
  ECORE_ATOM(atom_wm_protocols, "WM_PROTOCOLS");
  ECORE_ATOM(atom_xdndstatus, "XdndStatus");
  ECORE_ATOM(atom_xdndfinished, "XdndFinished");
  ECORE_ATOM(atom_xdndenter, "XdndEnter");
  ECORE_ATOM(atom_xdndleave, "XdndLeave");
  ECORE_ATOM(atom_xdnddrop, "XdndDrop");
  ECORE_ATOM(atom_xdndposition, "XdndPosition");
  ECORE_ATOM(atom_xdndactioncopy, "XdndActionCopy");
  ECORE_ATOM(atom_xdndactionlink, "XdndActionLink");
  ECORE_ATOM(atom_xdndactionmove, "XdndActionMove");
  ECORE_ATOM(atom_xdndactionprivate, "XdndActionPrivate");
  ECORE_ATOM(atom_text_uri_list, "text/uri-list");
   
  /* first type = delete event sent to client */
  if ((xevent->xclient.message_type == atom_wm_protocols) &&
      (xevent->xclient.format == 32) &&
      (xevent->xclient.data.l[0] == (long)atom_wm_delete_window))
    {
      Ecore_Event_Window_Delete *e;

      e = NEW(Ecore_Event_Window_Delete, 1);
      e->win = xevent->xclient.window;
      e->root = ecore_window_get_root(e->win);
      ecore_add_event(ECORE_EVENT_WINDOW_DELETE, e, ecore_event_generic_free);
    }
  else if ((xevent->xclient.message_type == atom_xdndenter) &&
	   (xevent->xclient.format == 32))
    {
/*      if ((xevent->xclient.data.l[2] == (long)atom_text_uri_list) ||
 *	  (xevent->xclient.data.l[3] == (long)atom_text_uri_list) ||
 *	  (xevent->xclient.data.l[4] == (long)atom_text_uri_list))
 */	{
	  Ecore_Event_Dnd_Drop_Request *e;

	   
	  if (ev_drop_request_pending)
	    {
	      ecore_event_dnd_drop_request_free(ev_drop_request_pending);
	      ev_drop_request_pending = NULL;
	    }
	  e = NEW(Ecore_Event_Dnd_Drop_Request, 1);
	  e->win = xevent->xclient.window;
	  e->root = ecore_window_get_root(e->win);
	  e->source_win = (Window) xevent->xclient.data.l[0];
	  if (!ecore_dnd_selection_convert
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
      Ecore_Event_Dnd_Drop_End *e;

      e = NEW(Ecore_Event_Dnd_Drop_End, 1);
      e->win = xevent->xclient.window;
      e->root = ecore_window_get_root(e->win);
      e->source_win = (Window) xevent->xclient.data.l[0];
      ecore_add_event(ECORE_EVENT_DND_DROP_END, e, ecore_event_generic_free);
    }
  else if ((xevent->xclient.message_type == atom_xdndposition) &&
	   (xevent->xclient.format == 32))
    {
      Ecore_Event_Dnd_Drop_Position *e;

      e = NEW(Ecore_Event_Dnd_Drop_Position, 1);
      e->win = xevent->xclient.window;
      e->root = ecore_window_get_root(e->win);
      e->source_win = (Window) xevent->xclient.data.l[0];
      e->x = (xevent->xclient.data.l[2] >> 16) & 0xffff;
      e->y = xevent->xclient.data.l[2] & 0xffff;
      ecore_add_event(ECORE_EVENT_DND_DROP_POSITION, e,
		      ecore_event_generic_free);
    }
  else if ((xevent->xclient.message_type == atom_xdndstatus) &&
	   (xevent->xclient.format == 32))
    {
      Ecore_Event_Dnd_Drop_Status *e;

      ecore_clear_target_status();
      e = NEW(Ecore_Event_Dnd_Drop_Status, 1);
      e->win = xevent->xclient.window;
      e->root = ecore_window_get_root(e->win);
      e->source_win = (Window) xevent->xclient.data.l[0];
      e->x = (xevent->xclient.data.l[2] >> 16) & 0xffff;
      e->y = xevent->xclient.data.l[2] & 0xffff;
      e->w = (xevent->xclient.data.l[3] >> 16) & 0xffff;
      e->h = xevent->xclient.data.l[3] & 0xffff;

      e->copy = e->link = e->move = e->e_private = 0;
      if( xevent->xclient.data.l[4] == atom_xdndactioncopy )
	e->copy = 1;
      else if( xevent->xclient.data.l[4] == atom_xdndactionlink )
	e->link = 1;
      else if( xevent->xclient.data.l[4] == atom_xdndactionmove )
	e->move = 1;
      else if( xevent->xclient.data.l[4] == atom_xdndactionprivate )
	e->e_private = 1;

      if (xevent->xclient.data.l[1] & 0x1)
	e->ok = 1;
      else
	e->ok = 0;
      if (xevent->xclient.data.l[1] & 0x2)
	e->all_position_msgs = 1;
      else
	e->all_position_msgs = 0;
      ecore_add_event(ECORE_EVENT_DND_DROP_STATUS, e, ecore_event_generic_free);
    }
  else if ((xevent->xclient.message_type == atom_xdndfinished) &&
	   (xevent->xclient.format == 32))
    {
      Ecore_Event_Dnd_Drop_End *e;

      e = NEW(Ecore_Event_Dnd_Drop_End, 1);
      e->win = xevent->xclient.window;
      e->root = ecore_window_get_root(e->win);
      e->source_win = (Window) xevent->xclient.data.l[0];
      ecore_add_event(ECORE_EVENT_DND_DROP_END, e, ecore_event_generic_free);
    }
  else if ((xevent->xclient.message_type == atom_xdnddrop) &&
	   (xevent->xclient.format == 32))
    {
      Ecore_Event_Dnd_Drop *e;

      e = NEW(Ecore_Event_Dnd_Drop, 1);
      e->win = xevent->xclient.window;
      e->root = ecore_window_get_root(e->win);
      e->source_win = (Window) xevent->xclient.data.l[0];
      ecore_add_event(ECORE_EVENT_DND_DROP, e, ecore_event_generic_free);
    }
  else
    {
      Ecore_Event_Message *e;

      e = NEW(Ecore_Event_Message, 1);
      e->win = xevent->xclient.window;
      e->format = xevent->xclient.format;
      e->atom = xevent->xclient.message_type;
      MEMCPY(xevent->xclient.data.b, e->data.b, char, 20);

      ecore_add_event(ECORE_EVENT_MESSAGE, e, ecore_event_generic_free);
    }
}

static void
ecore_event_x_handle_shape_change(XEvent * xevent)
{
  Ecore_Event_Window_Shape *e;
  XShapeEvent        *shape_event;

  shape_event = (XShapeEvent *) xevent;
  e = NEW(Ecore_Event_Window_Shape, 1);
  e->win = shape_event->window;
  e->root = ecore_window_get_root(e->win);
  e->time = shape_event->time;
  ecore_add_event(ECORE_EVENT_WINDOW_SHAPE, e, ecore_event_generic_free);
}

char               *
ecore_keypress_translate_into_typeable(Ecore_Event_Key_Down * e)
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
      (!strcmp(e->key, "KP_Add")) || (!strcmp(e->key, "Enter")))
    return NULL;
  return e->compose;
}
