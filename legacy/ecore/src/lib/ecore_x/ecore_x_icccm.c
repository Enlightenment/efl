/*
 * Various ICCCM related functions.
 * 
 * This is ALL the code involving anything ICCCM related. for both WM and
 * client.
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
ecore_x_icccm_delete_window_send(Ecore_X_Window win, Ecore_X_Time t)
{
   ecore_x_client_message32_send(win, _ecore_x_atom_wm_protocols,
				 _ecore_x_atom_wm_delete_window,
				 CurrentTime, 0, 0, 0);
}

void
ecore_x_icccm_take_focus_send(Ecore_X_Window win, Ecore_X_Time t)
{
   ecore_x_client_message32_send(win, _ecore_x_atom_wm_protocols,
				 _ecore_x_atom_wm_take_focus,
				 CurrentTime, 0, 0, 0);
}

void
ecore_x_icccm_save_yourself_send(Ecore_X_Window win, Ecore_X_Time t)
{
   ecore_x_client_message32_send(win, _ecore_x_atom_wm_protocols,
				 _ecore_x_atom_wm_save_yourself,
				 CurrentTime, 0, 0, 0);
}

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

void
ecore_x_icccm_hints_set(Ecore_X_Window win,
			int accepts_focus,
			Ecore_X_Window_State_Hint initial_state,
			Ecore_X_Pixmap icon_pixmap,
			Ecore_X_Pixmap icon_mask,
			Ecore_X_Window icon_window,
			Ecore_X_Window window_group,
			int is_urgent)
{
   XWMHints *hints;
   
   hints = XAllocWMHints();
   if (!hints) return;
   
   hints->flags = InputHint | StateHint;
   hints->input = accepts_focus;
   if (initial_state == ECORE_X_WINDOW_STATE_HINT_WITHDRAWN)
     hints->initial_state = WithdrawnState;
   else if (initial_state == ECORE_X_WINDOW_STATE_HINT_NORMAL)
     hints->initial_state = NormalState;
   else if (initial_state == ECORE_X_WINDOW_STATE_HINT_ICONIC)
     hints->initial_state = IconicState;
   if (icon_pixmap != 0)
     {
	hints->icon_pixmap = icon_pixmap;
	hints->flags |= IconPixmapHint;
     }
   if (icon_mask != 0)
     {
	hints->icon_mask = icon_mask;
	hints->flags |= IconMaskHint;
     }
   if (icon_window != 0)
     {
	hints->icon_window = icon_window;
	hints->flags |= IconWindowHint;
     }
   if (window_group != 0)
     {
	hints->window_group = window_group;
	hints->flags |= WindowGroupHint;
     }
   if (is_urgent)
     hints->flags |= XUrgencyHint;
   XSetWMHints(_ecore_x_disp, win, hints);
   XFree(hints);
}

int
ecore_x_icccm_hints_get(Ecore_X_Window win,
			int *accepts_focus,
			Ecore_X_Window_State_Hint *initial_state,
			Ecore_X_Pixmap *icon_pixmap,
			Ecore_X_Pixmap *icon_mask,
			Ecore_X_Window *icon_window,
			Ecore_X_Window *window_group,
			int *is_urgent)
{
   XWMHints           *hints;

   if (accepts_focus) *accepts_focus = 0;
   if (initial_state) *initial_state = ECORE_X_WINDOW_STATE_HINT_NORMAL;
   if (icon_pixmap) *icon_pixmap = 0;
   if (icon_mask) *icon_mask = 0;
   if (icon_window) *icon_window = 0;
   if (window_group) *window_group = 0;
   if (is_urgent) *is_urgent = 0;
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
	if ((hints->flags & XUrgencyHint) && (is_urgent))
	  {
	     *is_urgent = 1;
	  }
	XFree(hints);
	return 1;
     }
   return 0;
}

void
ecore_x_icccm_size_pos_hints_set(Ecore_X_Window win,
                                 int request_pos,
				 Ecore_X_Gravity gravity,
				 int min_w, int min_h,
				 int max_w, int max_h,
				 int base_w, int base_h,
				 int step_x, int step_y,
				 double min_aspect,
				 double max_aspect)
{
   /* FIXME: working here */
   XSizeHints hint;
   
   hint.flags = 0;
   if (request_pos)
     {
	hint.flags |= USPosition;
     }
   if (gravity != ECORE_X_GRAVITY_NW)
     {
	hint.flags |= PWinGravity;
	hint.win_gravity = gravity;
     }
   if ((min_w > 0) || (min_h > 0))
     {
	hint.flags |= PMinSize;
	hint.min_width = min_w;
	hint.min_height = min_h;
     }
   if ((max_w > 0) || (max_h > 0))
     {
	hint.flags |= PMaxSize;
	hint.max_width = max_w;
	hint.max_height = max_h;
     }
   if ((base_w > 0) || (base_h > 0))
     {
	hint.flags |= PBaseSize;
	hint.base_width = base_w;
	hint.base_height = base_h;
     }
   if ((step_x > 1) || (step_y > 1))
     {
	hint.flags |= PResizeInc;
	hint.width_inc = step_x;
	hint.height_inc = step_y;
     }
   if ((min_aspect > 0.0) || (max_aspect > 0.0))
     {
	hint.flags |= PAspect;
	hint.min_aspect.x = min_aspect * 10000;
	hint.min_aspect.x = 10000;
	hint.max_aspect.x = max_aspect * 10000;
	hint.max_aspect.x = 10000;
     }
   XSetWMNormalHints(_ecore_x_disp, win, &hint);
}

int
ecore_x_icccm_size_pos_hints_get(Ecore_X_Window win,
				 int *request_pos,
				 Ecore_X_Gravity *gravity,
				 int *min_w, int *min_h,
				 int *max_w, int *max_h,
				 int *base_w, int *base_h,
				 int *step_x, int *step_y,
				 double *min_aspect,
				 double *max_aspect)
{
   XSizeHints          hint;
   long                mask;
   
   int minw = 0, minh = 0;
   int maxw = 32767, maxh = 32767;
   int basew = 0, baseh = 0;
   int stepx = 1, stepy = 1;
   double mina = 0.0, maxa = 0.0;
   
   if (!XGetWMNormalHints(_ecore_x_disp, win, &hint, &mask)) return 0;
   if ((hint.flags & USPosition) || ((hint.flags & PPosition)))
     {
	if (*request_pos) *request_pos = 1;
     }
   else
     {
	if (*request_pos) *request_pos = 0;
     }
   if (hint.flags & PWinGravity)
     {
	if (*gravity) *gravity = hint.win_gravity;
     }
   else
     {
	if (*gravity) *gravity = ECORE_X_GRAVITY_NW;
     }
   if (hint.flags & PMinSize)
     {
	minw = hint.min_width;
	minh = hint.min_height;
     }
   if (hint.flags & PMaxSize)
     {
	maxw = hint.max_width;
	maxh = hint.max_height;
	if (maxw < minw) maxw = minw;
	if (maxh < minh) maxh = minh;
     }
   if (hint.flags & PBaseSize)
     {
	basew = hint.base_width;
	baseh = hint.base_height;
	if (basew > minw) minw = basew;
	if (baseh > minh) minh = baseh;
     }
   if (hint.flags & PResizeInc)
     {
	stepx = hint.width_inc;
	stepy = hint.height_inc;
	if (stepx < 1) stepx = 1;
	if (stepy < 1) stepy = 1;
     }
   if (hint.flags & PAspect)
     {
	if (hint.min_aspect.y > 0)
	  mina = ((double)hint.min_aspect.x) / ((double)hint.min_aspect.y);
	if (hint.max_aspect.y > 0)
	  maxa = ((double)hint.max_aspect.x) / ((double)hint.max_aspect.y);
     }
   if (min_w) *min_w = minw;
   if (min_h) *min_h = minh;
   if (max_w) *max_w = maxw;
   if (max_h) *max_h = maxh;
   if (base_w) *base_w = basew;
   if (base_h) *base_h = baseh;
   if (step_x) *step_x = stepx;
   if (step_y) *step_y = stepy;
   if (min_aspect) *min_aspect = mina;
   if (max_aspect) *max_aspect = maxa;
   return 1;
}

void
ecore_x_icccm_title_set(Ecore_X_Window win, const char *t)
{
   char *list[1];
   XTextProperty xprop;
   int ret;
   
#ifdef X_HAVE_UTF8_STRING
   list[0] = strdup(t);
   ret = Xutf8TextListToTextProperty(_ecore_x_disp, list, 1, XUTF8StringStyle, &xprop);
#else
   list[0] = strdup(t);
   ret = XmbTextListToTextProperty(_ecore_x_disp, list, 1, XStdICCTextStyle, &xprop);
#endif
   if (ret >= Success)
     {
	XSetWMName(_ecore_x_disp, win, &xprop);
	XFree(xprop.value);
     }
   else
     {
	if (XStringListToTextProperty(list, 1, &xprop) >= Success)
	  {
	     XSetWMName(_ecore_x_disp, win, &xprop);
	     XFree(xprop.value);
	  }
     }
   free(list[0]);
}

char *
ecore_x_icccm_title_get(Ecore_X_Window win)
{
   XTextProperty xprop;   
   
   if (XGetWMName(_ecore_x_disp, win, &xprop))
     {
	if (xprop.value)
	  {
	     char **list = NULL;
	     char *t = NULL;
	     int num = 0;
	     
	     if (xprop.encoding == _ecore_x_atom_string)
	       {
		  t = strdup(xprop.value);
	       }
	     else if (xprop.encoding == _ecore_x_atom_utf8_string)
	       {
		  t = strdup(xprop.value);
	       }
	     else
	       {
		  int ret;
		  
#ifdef X_HAVE_UTF8_STRING
		  ret = Xutf8TextPropertyToTextList(_ecore_x_disp, &xprop, 
						    &list, &num);
#else		  
		  ret = XmbTextPropertyToTextList(_ecore_x_disp, &xprop, 
						  &list, &num);
#endif		  
		  if ((ret == XLocaleNotSupported) ||
		      (ret == XNoMemory) ||
		      (ret == XConverterNotFound))
		    {
		       t = strdup(xprop.value);
		    }
		  else if (ret >= Success)
		    {
		       if ((num >= 1) && (list))
			 {
			    /* FIXME: convert to utf8 */
			    t = strdup(list[0]);
			 }
		       if (list) XFreeStringList(list);
		    }
	       }
	     XFree(xprop.value);
	     return t;
	  }
     }
   return NULL;
}

/* FIXME: move these things in here as they are icccm related */
/* get/set wm protocols */
/* get/set name/class */
/* get/set machine */
/* get/set command */
/* get/set icon name */
/* get/set colormap windows */
/* get/set window role */
/* get/set client leader */
/* get/set transient for */
/* send iconify request */

/* FIXME: there are older E hints, gnome hints and mwm hints and new netwm */
/*        hints. each should go in their own file/section so we know which */
/*        is which. also older kde hints too. we should try support as much */
/*        as makese sense to support */
