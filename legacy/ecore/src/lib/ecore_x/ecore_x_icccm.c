/*
 * Various ICCCM related functions.
 * These are normally called only by window managers.
 */
#include "Ecore.h"
#include "ecore_x_private.h"
#include "Ecore_X.h"

void
ecore_x_icccm_state_set(Ecore_X_Window win, Ecore_X_Window_State_Hint state)
{
   unsigned long       c[2];

   if (state == ECORE_X_WINDOW_STATE_HINT_WITHDRAWN)
     c[0] = WithdrawnState;
   else if (state == ECORE_X_WINDOW_STATE_HINT_NORMAL)
     c[0] = NormalState;
   else if (state == ECORE_X_WINDOW_STATE_HINT_ICONIC)
     c[0] = IconicState;
   c[1] = 0;
   XChangeProperty(_ecore_x_disp, win, _ecore_x_atom_wm_state,
		   _ecore_x_atom_wm_state, 32, PropModeReplace,
		   (unsigned char *)c, 2);
}

void
ecore_x_icccm_delete_window_send(Ecore_X_Window win)
{
   ecore_x_client_message32_send(win, _ecore_x_atom_wm_protocols,
				 _ecore_x_atom_wm_delete_window,
				 CurrentTime, 0, 0, 0);
}

void
ecore_x_icccm_take_focus_send(Ecore_X_Window win)
{
   ecore_x_client_message32_send(win, _ecore_x_atom_wm_protocols,
				 _ecore_x_atom_wm_take_focus,
				 CurrentTime, 0, 0, 0);
}

#if 0
void
ecore_x_icccm_save_yourself_send(Ecore_X_Window win)
{
   ecore_x_client_message32_send(win, _ecore_x_atom_wm_protocols,
				 _ecore_x_atom_wm_save_yourself,
				 CurrentTime, 0, 0, 0);
}
#endif

void
ecore_x_icccm_move_resize_send(Ecore_X_Window win,
			       int x, int y, int w, int h)
{
   XEvent              ev;
   
   ev.type = ConfigureNotify;
   ev.xconfigure.display = _ecore_x_disp;
   ev.xconfigure.event = win;
   ev.xconfigure.window = win;
   ev.xconfigure.x = x;
   ev.xconfigure.y = y;
   ev.xconfigure.width = w;
   ev.xconfigure.height = h;
   ev.xconfigure.border_width = 0;
   ev.xconfigure.above = win;
   ev.xconfigure.override_redirect = False;
   XSendEvent(_ecore_x_disp, win, False, StructureNotifyMask, &ev);
}                 

int
ecore_x_icccm_basic_hints_get(Ecore_X_Window win,
			      int *accepts_focus,
			      Ecore_X_Window_State_Hint *initial_state,
			      Ecore_X_Pixmap *icon_pixmap,
			      Ecore_X_Pixmap *icon_mask,
			      Ecore_X_Window *icon_window,
			      Ecore_X_Window *window_group)
{
   XWMHints           *hints;
   
   hints = XGetWMHints(_ecore_x_disp, win);
   if (hints)
     {
	if ((hints->flags & InputHint) && (accepts_focus))
	  {
	     if (hints->input)
	       *accepts_focus = 1;
	     else
	       *accepts_focus = 0;
	  }
	if ((hints->flags & StateHint) && (initial_state))
	  {
	     if (hints->initial_state == WithdrawnState)
	       *initial_state = ECORE_X_WINDOW_STATE_HINT_WITHDRAWN;
	     else if (hints->initial_state == NormalState)
	       *initial_state = ECORE_X_WINDOW_STATE_HINT_NORMAL;
	     else if (hints->initial_state == IconicState)
	       *initial_state = ECORE_X_WINDOW_STATE_HINT_ICONIC;
	  }
	if ((hints->flags & IconPixmapHint) && (icon_pixmap))
	  {
	     *icon_pixmap = hints->icon_pixmap;
	  }
	if ((hints->flags & IconMaskHint) && (icon_mask))
	  {
	     *icon_mask = hints->icon_pixmap;
	  }
	if ((hints->flags & IconWindowHint) && (icon_window))
	  {
	     *icon_window = hints->icon_window;
	  }
	if ((hints->flags & WindowGroupHint) && (window_group))
	  {
	     *window_group = hints->window_group;
	  }
	XFree(hints);
	return 1;
     }
   return 0;
}

/* FIXME: working here */
int
ecore_x_icccm_size_pos_hints_get(Ecore_X_Window win)
{
   XSizeHints          hint;
   long                mask;
   
   if (!XGetWMNormalHints(_ecore_x_disp, win, &hint, &mask)) return 0;
   if ((hint.flags & USPosition) || ((hint.flags & PPosition)))
     {
	int                 x, y, w, h;
	
//	D("%li %li\n", hint.flags & USPosition, hint.flags & PPosition);
//	b->client.pos.requested = 1;
//	b->client.pos.gravity = NorthWestGravity;
//	if (hint.flags & PWinGravity)
//	  b->client.pos.gravity = hint.win_gravity;
//	x = y = w = h = 0;
//	ecore_window_get_geometry(win, &x, &y, &w, &h);
//	b->client.pos.x = x;
//	b->client.pos.y = y;
     }
   else
     {
//	b->client.pos.requested = 0;
     }
   if (hint.flags & PMinSize)
     {
//	min_w = hint.min_width;
//	min_h = hint.min_height;
     }
   if (hint.flags & PMaxSize)
     {
//	max_w = hint.max_width;
//	max_h = hint.max_height;
//	if (max_w < min_w)
//	  max_w = min_w;
//	if (max_h < min_h)
//	  max_h = min_h;
     }
   if (hint.flags & PResizeInc)
     {
//	step_w = hint.width_inc;
//	step_h = hint.height_inc;
//	if (step_w < 1)
//	  step_w = 1;
//	if (step_h < 1)
//	  step_h = 1;
     }
   if (hint.flags & PBaseSize)
     {
//	base_w = hint.base_width;
//	base_h = hint.base_height;
//	if (base_w > max_w)
//	  max_w = base_w;
//	if (base_h > max_h)
//	  max_h = base_h;
     }
   else
     {
//	base_w = min_w;
//	base_h = min_h;
     }
   if (hint.flags & PAspect)
     {
//	if (hint.min_aspect.y > 0)
//	  aspect_min =
//	  ((double)hint.min_aspect.x) / ((double)hint.min_aspect.y);
//	if (hint.max_aspect.y > 0)
//	  aspect_max =
//	  ((double)hint.max_aspect.x) / ((double)hint.max_aspect.y);
     }
   
   return 1;
}

/* FIXME: move these things in here as they are icccm related */
/* get/set title */
/* get/set name/class */
/* get/set machine */
/* get/set command */
/* get/set icon name */

/* FIXME: there are older E hints, gnome hitns and mwm hints and new netwm */
/*        hints. each should go in their own file/section so we know which */
/*        is which. also older kde hints too. we shoudl try support as much */
/*        as makese sense to support */
