#include "Ecore.h"
#include "ecore_x_private.h"
#include "Ecore_X.h"

/**
 * Create a new window.
 * @param parent The parent window 
 * @param x X
 * @param y Y
 * @param w Width
 * @param h Height
 * @return The new window handle
 * 
 * Create a new window
 */
Ecore_X_Window
ecore_x_window_new(Ecore_X_Window parent, int x, int y, int w, int h)
{
   Window               win;
   XSetWindowAttributes attr;
   
   if (parent == 0) parent = DefaultRootWindow(_ecore_x_disp);
   attr.backing_store         = NotUseful;
   attr.override_redirect     = False;
   attr.colormap              = DefaultColormap(_ecore_x_disp, DefaultScreen(_ecore_x_disp));
   attr.border_pixel          = 0;
   attr.background_pixmap     = None;
   attr.save_under            = False;
   attr.do_not_propagate_mask = True;
   attr.event_mask            = KeyPressMask |
                                KeyReleaseMask |
                                ButtonPressMask |
                                ButtonReleaseMask |
                                EnterWindowMask |
                                LeaveWindowMask |
                                PointerMotionMask |
                                ExposureMask |
                                VisibilityChangeMask |
                                StructureNotifyMask |
                                FocusChangeMask |
                                PropertyChangeMask |
                                ColormapChangeMask;
   win = XCreateWindow(_ecore_x_disp, parent,
		       x, y, w, h, 0,
		       DefaultDepth(_ecore_x_disp, DefaultScreen(_ecore_x_disp)),
		       InputOutput, 
		       DefaultVisual(_ecore_x_disp, DefaultScreen(_ecore_x_disp)),
		       CWBackingStore |
		       CWOverrideRedirect | 
		       CWColormap | 
		       CWBorderPixel |
		       CWBackPixmap | 
		       CWSaveUnder | 
		       CWDontPropagate | 
		       CWEventMask,
		       &attr);

   if (parent == DefaultRootWindow(_ecore_x_disp)) ecore_x_window_defaults_set(win);
   return win;
}

/**
 * Create a window.
 * @param parent The parent window
 * @param x X
 * @param y Y
 * @param w Width
 * @param h Height
 * @return The new window handle
 * 
 * Create a new window
 */
Ecore_X_Window
ecore_x_window_override_new(Ecore_X_Window parent, int x, int y, int w, int h)
{
   Window               win;
   XSetWindowAttributes attr;
   
   if (parent == 0) parent = DefaultRootWindow(_ecore_x_disp);
   attr.backing_store         = NotUseful;
   attr.override_redirect     = True;
   attr.colormap              = DefaultColormap(_ecore_x_disp, DefaultScreen(_ecore_x_disp));
   attr.border_pixel          = 0;
   attr.background_pixmap     = None;
   attr.save_under            = False;
   attr.do_not_propagate_mask = True;
   attr.event_mask            = KeyPressMask |
                                KeyReleaseMask |
                                ButtonPressMask |
                                ButtonReleaseMask |
                                EnterWindowMask |
                                LeaveWindowMask |
                                PointerMotionMask |
                                ExposureMask |
                                VisibilityChangeMask |
                                StructureNotifyMask |
                                FocusChangeMask |
                                PropertyChangeMask |
                                ColormapChangeMask;
   win = XCreateWindow(_ecore_x_disp, parent,
		       x, y, w, h, 0,
		       DefaultDepth(_ecore_x_disp, DefaultScreen(_ecore_x_disp)),
		       InputOutput, 
		       DefaultVisual(_ecore_x_disp, DefaultScreen(_ecore_x_disp)),
		       CWBackingStore |
		       CWOverrideRedirect | 
		       CWColormap | 
		       CWBorderPixel |
		       CWBackPixmap | 
		       CWSaveUnder | 
		       CWDontPropagate | 
		       CWEventMask,
		       &attr);

   if (parent == DefaultRootWindow(_ecore_x_disp)) ecore_x_window_defaults_set(win);
   return win;
}

/**
 * Create a window.
 * @param parent The parent window
 * @param x X
 * @param y Y
 * @param w Width
 * @param h Height
 * @return The new window handle
 * 
 * Create a new window
 */
Ecore_X_Window
ecore_x_window_input_new(Ecore_X_Window parent, int x, int y, int w, int h)
{
   Window               win;
   XSetWindowAttributes attr;
   
   if (parent == 0) parent = DefaultRootWindow(_ecore_x_disp);
   attr.override_redirect     = True;
   attr.do_not_propagate_mask = True;
   attr.event_mask            = KeyPressMask |
                                KeyReleaseMask |
                                ButtonPressMask |
                                ButtonReleaseMask |
                                EnterWindowMask |
                                LeaveWindowMask |
                                PointerMotionMask |
                                ExposureMask |
                                VisibilityChangeMask |
                                StructureNotifyMask |
                                FocusChangeMask |
                                PropertyChangeMask |
                                ColormapChangeMask;
   win = XCreateWindow(_ecore_x_disp, parent,
		       x, y, w, h, 0,
		       0, 
		       InputOnly,
		       DefaultVisual(_ecore_x_disp, DefaultScreen(_ecore_x_disp)),
		       CWOverrideRedirect | 
		       CWDontPropagate | 
		       CWEventMask,
		       &attr);

   if (parent == DefaultRootWindow(_ecore_x_disp))
     {
     }
   return win;
}

/**
 * Set defaults for a window
 * @param win The window to set defaults
 *
 * Set defaults for a window
 */
void
ecore_x_window_defaults_set(Ecore_X_Window win)
{
   long pid;
   char buf[MAXHOSTNAMELEN];
   char *hostname[1];
   int argc;
   char **argv;
   XTextProperty xprop;

   /*
    * Set WM_CLIENT_MACHINE.
    */
   gethostname(buf, MAXHOSTNAMELEN);
   buf[MAXHOSTNAMELEN - 1] = '\0';
   hostname[0] = buf;
   /* The ecore function uses UTF8 which Xlib may not like (especially
    * with older clients) */
   /* ecore_x_window_prop_string_set(win, _ecore_x_atom_wm_client_machine,
				  (char *)buf); */
   if (XStringListToTextProperty(hostname, 1, &xprop))
      XSetWMClientMachine(_ecore_x_disp, win, &xprop);

   /*
    * Set _NET_WM_PID
    */
   pid = getpid();
   ecore_x_window_prop_property_set(win, _ecore_x_atom_net_wm_pid, XA_CARDINAL,
		                    32, &pid, 1);

   ecore_x_window_prop_window_type_set(win, ECORE_X_WINDOW_TYPE_NORMAL);

   ecore_app_args_get(&argc, &argv);
   ecore_x_window_prop_command_set(win, argc, argv);
}

/**
 * Delete a window.
 * @param win The window to delete
 * 
 * Delete a window
 */
void
ecore_x_window_del(Ecore_X_Window win)
{
   XDestroyWindow(_ecore_x_disp, win);
}

/**
 * Show a window.
 * @param win The window to show
 * 
 * Show a window
 */
void
ecore_x_window_show(Ecore_X_Window win)
{
   XMapWindow(_ecore_x_disp, win);
}

/**
 * Hide a window
 * @param win The window to hide
 * 
 * Hide a window
 */
void
ecore_x_window_hide(Ecore_X_Window win)
{
   XUnmapWindow(_ecore_x_disp, win);
}

/**
 * Move a window.
 * @param win The window to move
 * @param x X
 * @param y Y
 * 
 * Move a window to @p x, @p y
 */
void
ecore_x_window_move(Ecore_X_Window win, int x, int y)
{
   XMoveWindow(_ecore_x_disp, win, x, y);
}

/**
 * Resize a window.
 * @param win The window to resize
 * @param w Width
 * @param h Height
 * 
 * Resize a window to @p w x @p h
 */
void
ecore_x_window_resize(Ecore_X_Window win, int w, int h)
{
   if (w < 1) w = 1;
   if (h < 1) h = 1;
   XResizeWindow(_ecore_x_disp, win, w, h);
}

/**
 * Move and resize a window.
 * @param win The window to move and resize
 * @param x X
 * @param y Y
 * @param w Width
 * @param h Height
 * 
 * Move a window to @p x, @p y and resize it to @p w x @p h
 */
void
ecore_x_window_move_resize(Ecore_X_Window win, int x, int y, int w, int h)
{
   if (w < 1) w = 1;
   if (h < 1) h = 1;
   XMoveResizeWindow(_ecore_x_disp, win, x, y, w, h);
}

/**
 * Set the focus to the window.
 * @param win The window to focus
 * 
 * Set the focus to the window @p win
 */
void
ecore_x_window_focus(Ecore_X_Window win)
{
   if (win == 0) win = DefaultRootWindow(_ecore_x_disp);   
   XSetInputFocus(_ecore_x_disp, win, RevertToNone, CurrentTime);
}

/**
 * Set the focus to the window at a specific time.
 * @param win The window to focus
 * @param t When
 * 
 * Set the focus to the window @p win at time @p t
 */
void
ecore_x_window_focus_at_time(Ecore_X_Window win, Ecore_X_Time t)
{
   if (win == 0) win = DefaultRootWindow(_ecore_x_disp);   
   XSetInputFocus(_ecore_x_disp, win, RevertToNone, t);
}

/**
 * Raise window.
 * @param win The window to raise
 * 
 * Raise window @p win
 */
void
ecore_x_window_raise(Ecore_X_Window win)
{
   XRaiseWindow(_ecore_x_disp, win);
}

/**
 * Lower window.
 * @param win The window to lower
 * 
 * Lower window @p win
 */
void
ecore_x_window_lower(Ecore_X_Window win)
{
   XLowerWindow(_ecore_x_disp, win);
}

/**
 * Reparent a window.
 * @param win The window to reparent
 * @param new_parent The new parent window
 * @param x X
 * @param y Y
 * 
 * Reparent @p win to the parent @p new_parent at @p x, @p y
 */
void
ecore_x_window_reparent(Ecore_X_Window win, Ecore_X_Window new_parent, int x, int y)
{
   if (new_parent == 0) new_parent = DefaultRootWindow(_ecore_x_disp);   
   XReparentWindow(_ecore_x_disp, win, new_parent, x, y);
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 */
void
ecore_x_window_size_get(Ecore_X_Window win, int *w, int *h)
{
   Window dummy_win;
   int dummy_x, dummy_y;
   unsigned int ret_w, ret_h, dummy_border, dummy_depth;
   
   if (win == 0) win = DefaultRootWindow(_ecore_x_disp);
   ret_w = 0;
   ret_h = 0;
   if (!XGetGeometry(_ecore_x_disp, win, &dummy_win, &dummy_x, &dummy_y,
		     &ret_w, &ret_h, &dummy_border, &dummy_depth))
     {
	ret_w = 0;
	ret_h = 0;	
     }
   if (w) *w = (int)ret_w;
   if (h) *h = (int)ret_h;
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 */
void
ecore_x_window_geometry_get(Ecore_X_Window win, int *x, int *y, int *w, int *h)
{
   Window         dummy_win;
   int            ret_x, ret_y;
   unsigned int   ret_w, ret_h, dummy_border, dummy_depth;

   if (!win)
      win = DefaultRootWindow(_ecore_x_disp);
   ret_w = 0;
   ret_h = 0;
   if (!XGetGeometry(_ecore_x_disp, win, &dummy_win, &ret_x, &ret_y,
                     &ret_w, &ret_h, &dummy_border, &dummy_depth))
   {
      ret_x = 0;
      ret_y = 0;
      ret_w = 0;
      ret_h = 0;
   }

   if (x) *x = ret_x;
   if (y) *y = ret_y;
   if (w) *w = (int) ret_w;
   if (h) *h = (int) ret_h;
}
   
/**
 * To be documented.
 *
 * FIXME: To be fixed.
 */
void
ecore_x_window_cursor_show(Ecore_X_Window win, int show)
{
   if (win == 0) win = DefaultRootWindow(_ecore_x_disp);
   if (!show)
     {
	Cursor              c;
	XColor              cl;
	Pixmap              p, m;
	GC                  gc;
	XGCValues           gcv;
	
	p = XCreatePixmap(_ecore_x_disp, win, 1, 1, 1);
	m = XCreatePixmap(_ecore_x_disp, win, 1, 1, 1);
	gc = XCreateGC(_ecore_x_disp, m, 0, &gcv);
	XSetForeground(_ecore_x_disp, gc, 0);
	XDrawPoint(_ecore_x_disp, m, gc, 0, 0);
	XFreeGC(_ecore_x_disp, gc);
	c = XCreatePixmapCursor(_ecore_x_disp, p, m, &cl, &cl, 0, 0);
	XDefineCursor(_ecore_x_disp, win, c);
	XFreeCursor(_ecore_x_disp, c);
	XFreePixmap(_ecore_x_disp, p);
	XFreePixmap(_ecore_x_disp, m);
     }
   else
     {
	XDefineCursor(_ecore_x_disp, win, 0);	
     }
}
