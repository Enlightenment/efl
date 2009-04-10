/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>

#include "Ecore.h"
#include "ecore_x_private.h"
#include "Ecore_X.h"
#include "Ecore_X_Atoms.h"

static int             ignore_num = 0;
static Ecore_X_Window *ignore_list = NULL;

/**
 * @defgroup Ecore_X_Window_Create_Group X Window Creation Functions
 *
 * Functions that can be used to create an X window.
 */

/**
 * Creates a new window.
 * @param   parent The parent window to use.  If @p parent is @c 0, the root
 *                 window of the default display is used.
 * @param   x      X position.
 * @param   y      Y position.
 * @param   w      Width.
 * @param   h      Height.
 * @return  The new window handle.
 * @ingroup Ecore_X_Window_Create_Group
 */
EAPI Ecore_X_Window
ecore_x_window_new(Ecore_X_Window parent, int x, int y, int w, int h)
{
   Window               win;
   XSetWindowAttributes attr;
   
   if (parent == 0) parent = DefaultRootWindow(_ecore_x_disp);
   attr.backing_store         = NotUseful;
   attr.override_redirect     = False;
   attr.border_pixel          = 0;
   attr.background_pixmap     = None;
   attr.bit_gravity            = NorthWestGravity;
   attr.win_gravity            = NorthWestGravity;
   attr.save_under            = False;
   attr.do_not_propagate_mask = NoEventMask;
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
		       CopyFromParent, /*DefaultDepth(_ecore_x_disp, DefaultScreen(_ecore_x_disp)),*/
		       InputOutput, 
		       CopyFromParent, /*DefaultVisual(_ecore_x_disp, DefaultScreen(_ecore_x_disp)),*/
		       CWBackingStore |
		       CWOverrideRedirect | 
/*		       CWColormap | */
		       CWBorderPixel |
		       CWBackPixmap | 
		       CWSaveUnder | 
		       CWDontPropagate | 
		       CWEventMask |
		       CWBitGravity |
		       CWWinGravity,
		       &attr);

   if (parent == DefaultRootWindow(_ecore_x_disp)) ecore_x_window_defaults_set(win);
   return win;
}

/**
 * Creates a window with the override redirect attribute set to @c True.
 * @param   parent The parent window to use.  If @p parent is @c 0, the root
 *                 window of the default display is used.
 * @param   x      X position.
 * @param   y      Y position.
 * @param   w      Width.
 * @param   h      Height.
 * @return  The new window handle.
 * @ingroup Ecore_X_Window_Create_Group
 */
EAPI Ecore_X_Window
ecore_x_window_override_new(Ecore_X_Window parent, int x, int y, int w, int h)
{
   Window               win;
   XSetWindowAttributes attr;
   
   if (parent == 0) parent = DefaultRootWindow(_ecore_x_disp);
   attr.backing_store         = NotUseful;
   attr.override_redirect     = True;
   attr.border_pixel          = 0;
   attr.background_pixmap     = None;
   attr.bit_gravity           = NorthWestGravity;
   attr.win_gravity           = NorthWestGravity;
   attr.save_under            = False;
   attr.do_not_propagate_mask = NoEventMask;
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
		       CopyFromParent, /*DefaultDepth(_ecore_x_disp, DefaultScreen(_ecore_x_disp)),*/
		       InputOutput, 
		       CopyFromParent, /*DefaultVisual(_ecore_x_disp, DefaultScreen(_ecore_x_disp)),*/
		       CWBackingStore |
		       CWOverrideRedirect | 
/*		       CWColormap | */
		       CWBorderPixel |
		       CWBackPixmap | 
		       CWSaveUnder | 
		       CWDontPropagate | 
		       CWEventMask |
		       CWBitGravity |
		       CWWinGravity,
		       &attr);
   return win;
}

/**
 * Creates a new input window.
 * @param   parent The parent window to use.    If @p parent is @c 0, the root
 *                 window of the default display is used.
 * @param   x      X position.
 * @param   y      Y position.
 * @param   w      Width.
 * @param   h      Height.
 * @return  The new window.
 * @ingroup Ecore_X_Window_Create_Group
 */
EAPI Ecore_X_Window
ecore_x_window_input_new(Ecore_X_Window parent, int x, int y, int w, int h)
{
   Window               win;
   XSetWindowAttributes attr;
   
   if (parent == 0) parent = DefaultRootWindow(_ecore_x_disp);
   attr.override_redirect     = True;
   attr.do_not_propagate_mask = NoEventMask;
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
		       CopyFromParent, 
		       InputOnly,
		       CopyFromParent, /*DefaultVisual(_ecore_x_disp, DefaultScreen(_ecore_x_disp)),*/
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
 * @defgroup Evas_X_Window_Properties_Group X Window Property Functions
 *
 * Functions that set window properties.
 */

/**
 * Sets the default properties for the given window.
 *
 * The default properties set for the window are @c WM_CLIENT_MACHINE and
 * @c _NET_WM_PID.
 *
 * @param   win The given window.
 * @ingroup Evas_X_Window_Properties_Groups
 */
EAPI void
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
   /* ecore_x_window_prop_string_set(win, ECORE_X_ATOM_WM_CLIENT_MACHINE,
				  (char *)buf); */
   if (XStringListToTextProperty(hostname, 1, &xprop))
     {
	XSetWMClientMachine(_ecore_x_disp, win, &xprop);
	XFree(xprop.value);
     }

   /*
    * Set _NET_WM_PID
    */
   pid = getpid();
   ecore_x_netwm_pid_set(win, pid);

   ecore_x_netwm_window_type_set(win, ECORE_X_WINDOW_TYPE_NORMAL);

   ecore_app_args_get(&argc, &argv);
   ecore_x_icccm_command_set(win, argc, argv);
}

EAPI void
ecore_x_window_configure(Ecore_X_Window win,
                         Ecore_X_Window_Configure_Mask mask,
                         int x, int y, int w, int h,
                         int border_width, Ecore_X_Window sibling,
                         int stack_mode)
{
   XWindowChanges xwc;

   if (!win)
      return;

   xwc.x = x;
   xwc.y = y;
   xwc.width = w;
   xwc.height = h;
   xwc.border_width = border_width;
   xwc.sibling = sibling;
   xwc.stack_mode = stack_mode;

   XConfigureWindow(_ecore_x_disp, win, mask, &xwc);
}

/**
 * @defgroup Evas_X_Window_Destroy_Group X Window Destroy Functions
 *
 * Functions to destroy X windows.
 */

/**
 * Deletes the given window.
 * @param   win The given window.
 * @ingroup Evas_X_Window_Destroy_Group
 */
EAPI void
ecore_x_window_free(Ecore_X_Window win)
{
   /* sorry sir, deleting the root window doesn't sound like
    * a smart idea.
    */
   if (win)
      XDestroyWindow(_ecore_x_disp, win);
}

/**
 * Set if a window should be ignored.
 * @param   win The given window.
 * @param   ignore if to ignore
 */
EAPI void
ecore_x_window_ignore_set(Ecore_X_Window win, int ignore)
{
   int i, j;

   if (ignore)
     {
	if (ignore_list)
	  {
	     for (i = 0; i < ignore_num; i++)
	       {
		  if (win == ignore_list[i])
		    return;
	       }
	     ignore_list = realloc(ignore_list, (ignore_num + 1) * sizeof(Ecore_X_Window));
	     if (!ignore_list) return;
	     ignore_list[ignore_num++] = win;
	  }
	else
	  {
	     ignore_num = 0;
	     ignore_list = malloc(sizeof(Ecore_X_Window));
	     ignore_list[ignore_num++] = win;
	  }
     }
   else
     {
	if (!ignore_list) return;
	for (i = 0, j = 0; i < ignore_num; i++)
	  {
	     if (win != ignore_list[i])
	       ignore_list[i] = ignore_list[j++];
	     else
	       ignore_num--;
	  }
	ignore_list = realloc(ignore_list, ignore_num * sizeof(Ecore_X_Window));
     }
}

/**
 * Get the ignore list
 * @param   num number of windows in the list
 * @return  list of windows to ignore
 */
EAPI Ecore_X_Window *
ecore_x_window_ignore_list(int *num)
{
   if (num) *num = ignore_num;
   return ignore_list;
}

/**
 * Sends a delete request to the given window.
 * @param   win The given window.
 * @ingroup Evas_X_Window_Destroy_Group
 */
EAPI void
ecore_x_window_delete_request_send(Ecore_X_Window win)
{
   XEvent xev;

   /* sorry sir, deleting the root window doesn't sound like
    * a smart idea.
    */
   if (!win)
      return;

   xev.xclient.type = ClientMessage;
   xev.xclient.display = _ecore_x_disp;
   xev.xclient.window = win;
   xev.xclient.message_type = ECORE_X_ATOM_WM_PROTOCOLS;
   xev.xclient.format = 32;
   xev.xclient.data.l[0] = ECORE_X_ATOM_WM_DELETE_WINDOW;
   xev.xclient.data.l[1] = CurrentTime;

   XSendEvent(_ecore_x_disp, win, False, NoEventMask, &xev);
}

/**
 * @defgroup Evas_X_Window_Visibility_Group X Window Visibility Functions
 *
 * Functions to access and change the visibility of X windows.
 */

/**
 * Shows a window.
 *
 * Synonymous to "mapping" a window in X Window System terminology.
 *
 * @param   win The window to show.
 * @ingroup Evas_X_Window_Visibility
 */
EAPI void
ecore_x_window_show(Ecore_X_Window win)
{
   XMapWindow(_ecore_x_disp, win);
}

/**
 * Hides a window.
 *
 * Synonymous to "unmapping" a window in X Window System terminology.
 *
 * @param   win The window to hide.
 * @ingroup Evas_X_Window_Visibility
 */
EAPI void
ecore_x_window_hide(Ecore_X_Window win)
{
   XEvent xev;
   Window root;
   int idum;
   unsigned int uidum;

   /* ICCCM: SEND unmap event... */
   root = win;
   if (ScreenCount(_ecore_x_disp) == 1)
     root = DefaultRootWindow(_ecore_x_disp);
   else
     XGetGeometry(_ecore_x_disp, win, &root, &idum, &idum, &uidum, &uidum, &uidum, &uidum);
   xev.xunmap.type = UnmapNotify;
   xev.xunmap.serial = 0;
   xev.xunmap.send_event = True;
   xev.xunmap.display = _ecore_x_disp;
   xev.xunmap.event = root;
   xev.xunmap.window = win;
   xev.xunmap.from_configure = False;
   XSendEvent(_ecore_x_disp, xev.xunmap.event, False,
	      SubstructureRedirectMask | SubstructureNotifyMask, &xev);
   XUnmapWindow(_ecore_x_disp, win);
}

/**
 * @defgroup Ecore_X_Window_Geometry_Group X Window Geometry Functions
 *
 * Functions that change or retrieve the geometry of X windows.
 */

/**
 * Moves a window to the position @p x, @p y.
 *
 * The position is relative to the upper left hand corner of the
 * parent window.
 *
 * @param   win The window to move.
 * @param   x   X position.
 * @param   y   Y position.
 * @ingroup Ecore_X_Window_Geometry_Group
 */
EAPI void
ecore_x_window_move(Ecore_X_Window win, int x, int y)
{
   XMoveWindow(_ecore_x_disp, win, x, y);
}

/**
 * Resizes a window.
 * @param   win The window to resize.
 * @param   w   New width of the window.
 * @param   h   New height of the window.
 * @ingroup Ecore_X_Window_Geometry_Group
 */
EAPI void
ecore_x_window_resize(Ecore_X_Window win, int w, int h)
{
   if (w < 1) w = 1;
   if (h < 1) h = 1;
   XResizeWindow(_ecore_x_disp, win, w, h);
}

/**
 * Moves and resizes a window.
 * @param   win The window to move and resize.
 * @param   x   New X position of the window.
 * @param   y   New Y position of the window.
 * @param   w   New width of the window.
 * @param   h   New height of the window.
 * @ingroup Ecore_X_Window_Geometry_Group
 */
EAPI void
ecore_x_window_move_resize(Ecore_X_Window win, int x, int y, int w, int h)
{
   if (w < 1) w = 1;
   if (h < 1) h = 1;
   XMoveResizeWindow(_ecore_x_disp, win, x, y, w, h);
}

/**
 * @defgroup Ecore_X_Window_Focus_Functions X Window Focus Functions
 *
 * Functions that give the focus to an X Window.
 */

/**
 * Sets the focus to the window @p win.
 * @param   win The window to focus.
 * @ingroup Ecore_X_Window_Focus_Functions
 */
EAPI void
ecore_x_window_focus(Ecore_X_Window win)
{
   if (win == 0) win = DefaultRootWindow(_ecore_x_disp);   
//   XSetInputFocus(_ecore_x_disp, win, RevertToNone, CurrentTime);
   XSetInputFocus(_ecore_x_disp, win, RevertToPointerRoot, CurrentTime);
}

/**
 * Sets the focus to the given window at a specific time.
 * @param   win The window to focus.
 * @param   t   When to set the focus to the window.
 * @ingroup Ecore_X_Window_Focus_Functions
 */
EAPI void
ecore_x_window_focus_at_time(Ecore_X_Window win, Ecore_X_Time t)
{
   if (win == 0) win = DefaultRootWindow(_ecore_x_disp);   
//   XSetInputFocus(_ecore_x_disp, win, RevertToNone, t);
   XSetInputFocus(_ecore_x_disp, win, PointerRoot, t);
}

/**
 * gets the focus to the window @p win.
 * @return  The window that has focus.
 * @ingroup Ecore_X_Window_Focus_Functions
 */
EAPI Ecore_X_Window
ecore_x_window_focus_get(void)
{
   Window win;
   int revert_mode;
   
   win = 0;
   
   XGetInputFocus(_ecore_x_disp, &win, &revert_mode);
   return win;
}

/**
 * @defgroup Ecore_X_Window_Z_Order_Group X Window Z Order Functions
 *
 * Functions that change the Z order of X windows.
 */

/**
 * Raises the given window.
 * @param   win The window to raise.
 * @ingroup Ecore_X_Window_Z_Order_Group
 */
EAPI void
ecore_x_window_raise(Ecore_X_Window win)
{
   XRaiseWindow(_ecore_x_disp, win);
}

/**
 * Lowers the given window.
 * @param   win The window to lower.
 * @ingroup Ecore_X_Window_Z_Order_Group
 */
EAPI void
ecore_x_window_lower(Ecore_X_Window win)
{
   XLowerWindow(_ecore_x_disp, win);
}

/**
 * @defgroup Ecore_X_Window_Parent_Group X Window Parent Functions
 *
 * Functions that retrieve or changes the parent window of a window.
 */

/**
 * Moves a window to within another window at a given position.
 * @param   win        The window to reparent.
 * @param   new_parent The new parent window.
 * @param   x          X position within new parent window.
 * @param   y          Y position within new parent window.
 * @ingroup Ecore_X_Window_Parent_Group
 */
EAPI void
ecore_x_window_reparent(Ecore_X_Window win, Ecore_X_Window new_parent, int x, int y)
{
   if (new_parent == 0) new_parent = DefaultRootWindow(_ecore_x_disp);   
   XReparentWindow(_ecore_x_disp, win, new_parent, x, y);
}

/**
 * Retrieves the size of the given window.
 * @param   win The given window.
 * @param   w   Pointer to an integer into which the width is to be stored.
 * @param   h   Pointer to an integer into which the height is to be stored.
 * @ingroup Ecore_X_Window_Geometry_Group
 */
EAPI void
ecore_x_window_size_get(Ecore_X_Window win, int *w, int *h)
{
   int dummy_x, dummy_y;
   
   if (win == 0) 
      win = DefaultRootWindow(_ecore_x_disp);

   ecore_x_drawable_geometry_get(win, &dummy_x, &dummy_y, w, h);
}

/**
 * Retrieves the geometry of the given window.
 * @param   win The given window.
 * @param   x   Pointer to an integer in which the X position is to be stored.
 * @param   y   Pointer to an integer in which the Y position is to be stored.
 * @param   w   Pointer to an integer in which the width is to be stored.
 * @param   h   Pointer to an integer in which the height is to be stored.
 * @ingroup Ecore_X_Window_Geometry_Group
 */
EAPI void
ecore_x_window_geometry_get(Ecore_X_Window win, int *x, int *y, int *w, int *h)
{
   if (!win)
      win = DefaultRootWindow(_ecore_x_disp);

   ecore_x_drawable_geometry_get(win, x, y, w, h);
}

/**
 * Retrieves the width of the border of the given window.
 * @param   win The given window.
 * @return  Width of the border of @p win.
 * @ingroup Ecore_X_Window_Geometry_Group
 */
EAPI int
ecore_x_window_border_width_get(Ecore_X_Window win)
{
   /* doesn't make sense to call this on a root window */
   if (!win)
      return 0;

   return ecore_x_drawable_border_width_get(win);
}

/**
 * Sets the width of the border of the given window.
 * @param   win The given window.
 * @param   width The new border width.
 * @ingroup Ecore_X_Window_Geometry_Group
 */
EAPI void
ecore_x_window_border_width_set(Ecore_X_Window win, int width)
{
   /* doesn't make sense to call this on a root window */
   if (!win)
      return;

   XSetWindowBorderWidth (_ecore_x_disp, win, width);
}

/**
 * Retrieves the depth of the given window.
 * @param  win The given window.
 * @return Depth of the window.
 */
EAPI int
ecore_x_window_depth_get(Ecore_X_Window win)
{
   return ecore_x_drawable_depth_get(win);
}
  
/**
 * To be documented.
 *
 * FIXME: To be fixed.
 */
EAPI void
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

EAPI void
ecore_x_window_cursor_set(Ecore_X_Window win, Ecore_X_Cursor c)
{
   if (c == 0)
     XUndefineCursor(_ecore_x_disp, win);
   else
     XDefineCursor(_ecore_x_disp, win, c);
}

/**
 * Finds out whether the given window is currently visible.
 * @param   win The given window.
 * @return  1 if the window is visible, otherwise 0.
 * @ingroup Ecore_X_Window_Visibility_Group
 */
EAPI int
ecore_x_window_visible_get(Ecore_X_Window win)
{
   XWindowAttributes attr;

   return (XGetWindowAttributes(_ecore_x_disp, win, &attr) &&
           (attr.map_state == IsViewable));
}



typedef struct _Shadow Shadow;
struct _Shadow
{
   Shadow         *parent;
   Shadow        **children;
   Window          win;
   int             children_num;
   short           x, y;
   unsigned short  w, h;
};

static int shadow_count = 0;
static Shadow **shadow_base = NULL;
static int shadow_num = 0;

static Shadow *
_ecore_x_window_tree_walk(Window win)
{
   Window           *list = NULL;
   Window            parent_win = 0, root_win = 0;
   int               i, j;
   unsigned int      num;
   Shadow           *s, **sl;
   XWindowAttributes att;

   if (!XGetWindowAttributes(_ecore_x_disp, win, &att)) return NULL;
//   if (att.class == InputOnly) return NULL;
   if (att.map_state != IsViewable) return NULL;
   
   s = calloc(1, sizeof(Shadow));
   if (!s) return NULL;
   s->win = win;
   s->x = att.x;
   s->y = att.y;
   s->w = att.width;
   s->h = att.height;
   if (XQueryTree(_ecore_x_disp, s->win, &root_win, &parent_win,
		   &list, &num))
     {
	s->children = calloc(1, sizeof(Shadow *) * num);
	if (s->children)
	  {
	     s->children_num = num;
	     for (i = 0; i < num; i++)
	       {
		  s->children[i] = _ecore_x_window_tree_walk(list[i]);
		  if (s->children[i]) s->children[i]->parent = s;
	       }
	     /* compress list down */
	     j = 0;
	     for (i = 0; i < num; i++)
	       {
		  if (s->children[i])
		    {
		       s->children[j] = s->children[i];
		       j++;
		    }
	       }
	     if (j == 0)
	       {
		  free(s->children);
		  s->children = NULL;
		  s->children_num = 0;
	       }
	     else
	       {
		  s->children_num = j;
		  sl = realloc(s->children, sizeof(Shadow *) * j);
		  if (sl) s->children = sl;
	       }
	  }
     }
   return s;
}

static void
_ecore_x_window_tree_shadow_free1(Shadow *s)
{
   int i;

   if (!s) return;
   if (s->children)
     {
	for (i = 0; i < s->children_num; i++)
	  {
	     if (s->children[i])
	       _ecore_x_window_tree_shadow_free1(s->children[i]);
	  }
	free(s->children);
     }
   free(s);
}

static void
_ecore_x_window_tree_shadow_free(void)
{
   int i;
   
   if (!shadow_base) return;
   for (i = 0; i < shadow_num; i++)
     {
	if (!shadow_base[i]) continue;
	_ecore_x_window_tree_shadow_free1(shadow_base[i]);
     }
   free(shadow_base);
   shadow_base = NULL;
   shadow_num = 0;
}

static void
_ecore_x_window_tree_shadow_populate(void)
{
   Ecore_X_Window   *roots;
   int               i, num;
   
   roots = ecore_x_window_root_list(&num);
   if (roots)
     {
	shadow_base = calloc(1, sizeof(Shadow *) * num);
	if (shadow_base)
	  {
	     shadow_num = num;
	     for (i = 0; i < num; i++)
	       shadow_base[i] = _ecore_x_window_tree_walk(roots[i]);
	  }
	free(roots);
     }
}

static void
_ecore_x_window_tree_shadow_start(void)
{
   shadow_count++;
   if (shadow_count > 1) return;
   _ecore_x_window_tree_shadow_populate();
}

static void
_ecore_x_window_tree_shadow_stop(void)
{
   shadow_count--;
   if (shadow_count != 0) return;
   _ecore_x_window_tree_shadow_free();
}

Shadow *
_ecore_x_window_shadow_tree_find_shadow(Shadow *s, Window win)
{
   Shadow *ss;
   int i;

   if (s->win == win) return s;
   if (s->children)
     {
	for (i = 0; i < s->children_num; i++)
	  {
	     if (!s->children[i]) continue;
	     if ((ss = _ecore_x_window_shadow_tree_find_shadow(s->children[i], win)))
	       return ss;
	  }
     }
   return NULL;
}

Shadow *
_ecore_x_window_shadow_tree_find(Window base)
{
   Shadow *s;
   int i;

   for (i = 0; i < shadow_num; i++)
     {
	if (!shadow_base[i]) continue;
	if ((s = _ecore_x_window_shadow_tree_find_shadow(shadow_base[i], base)))
	  return s;
     }
   return NULL;
}

static Window
_ecore_x_window_shadow_tree_at_xy_get_shadow(Shadow *s, int bx, int by, int x, int y,
					     Ecore_X_Window *skip, int skip_num)
{
   Window child;
   int i, j;
   int wx, wy;
   
   wx = s->x + bx;
   wy = s->y + by;
   if (!((x >= wx) && (y >= wy) && (x < (wx + s->w)) && (y < (wy + s->h))))
     return 0;
   if (s->children)
     {
	int skipit = 0;
	
	for (i = s->children_num - 1; i >= 0; --i)
	  {
	     if (!s->children[i]) continue;
	     skipit = 0;
	     if (skip)
	       {
		  for (j = 0; j < skip_num; j++)
		    {
		       if (s->children[i]->win == skip[j])
			 {
			    skipit = 1;
			    goto onward;
			 }
		    }
	       }
	     onward:
	     if (!skipit)
	       {
		  if ((child = _ecore_x_window_shadow_tree_at_xy_get_shadow(s->children[i], wx, wy, x, y, skip, skip_num)))
		    {
		       return child;
		    }
	       }
	  }
     }
   return s->win;
}

static Window
_ecore_x_window_shadow_tree_at_xy_get(Window base, int bx, int by, int x, int y,
				      Ecore_X_Window *skip, int skip_num)
{
   Shadow *s;
   
   if (!shadow_base)
     {
	_ecore_x_window_tree_shadow_populate();
	if (!shadow_base) return 0;
     }
   s = _ecore_x_window_shadow_tree_find(base);
   if (!s) return 0;
   return _ecore_x_window_shadow_tree_at_xy_get_shadow(s, bx, by, x, y, skip, skip_num);
}

/**
 * Retrieves the top, visible window at the given location,
 * but skips the windows in the list. This uses a shadow tree built from the
 * window tree that is only updated the first time
 * ecore_x_window_shadow_tree_at_xy_with_skip_get() is called, or the next time
 * it is called after a  ecore_x_window_shadow_tree_flush()
 * @param   base The base window to start searching from (normally root).
 * @param   x The given X position.
 * @param   y The given Y position.
 * @return  The window at that position.
 * @ingroup Ecore_X_Window_Geometry_Group
 */
EAPI Ecore_X_Window
ecore_x_window_shadow_tree_at_xy_with_skip_get(Ecore_X_Window base, int x, int y, Ecore_X_Window *skip, int skip_num)
{
   return _ecore_x_window_shadow_tree_at_xy_get(base, 0, 0, x, y, skip, skip_num);
}

/**
 * Retrieves the parent window a given window has. This uses the shadow window
 * tree.
 * @param   root The root window of @p win - if 0, this will be automatically determined with extra processing overhead
 * @param   win The window to get the parent window of
 * @return  The parent window of @p win
 * @ingroup Ecore_X_Window_Geometry_Group
 */
EAPI Ecore_X_Window
ecore_x_window_shadow_parent_get(Ecore_X_Window root, Ecore_X_Window win)
{
   Shadow *s;
   int i;
   
   if (!shadow_base)
     {
	_ecore_x_window_tree_shadow_populate();
	if (!shadow_base) return 0;
     }
   for (i = 0; i < shadow_num; i++)
     {
	if (!shadow_base[i]) continue;
	s = _ecore_x_window_shadow_tree_find_shadow(shadow_base[i], win);
	if (s)
	  {
	     if (!s->parent) return 0;
	     return s->parent->win;
	  }
     }
   return 0;
}

/**
 * Flushes the window shadow tree so nothing is stored.
 * @ingroup Ecore_X_Window_Geometry_Group
 */
EAPI void
ecore_x_window_shadow_tree_flush(void)
{
   _ecore_x_window_tree_shadow_free();
}

/**
 * Retrieves the root window a given window is on.
 * @param   win The window to get the root window of
 * @return  The root window of @p win
 * @ingroup Ecore_X_Window_Geometry_Group
 */
EAPI Ecore_X_Window
ecore_x_window_root_get(Ecore_X_Window win)
{
   XWindowAttributes att;

   if (!XGetWindowAttributes(_ecore_x_disp, win, &att)) return 0;
   return att.root;
}

static Window
_ecore_x_window_at_xy_get(Window base, int bx, int by, int x, int y,
			  Ecore_X_Window *skip, int skip_num)
{
   Window           *list = NULL;
   Window            parent_win = 0, child = 0, root_win = 0;
   int               i, j, wx, wy, ww, wh;
   unsigned int      num;

   if (!ecore_x_window_visible_get(base))
     return 0;

   ecore_x_window_geometry_get(base, &wx, &wy, &ww, &wh);
   wx += bx;
   wy += by;

   if (!((x >= wx) && (y >= wy) && (x < (wx + ww)) && (y < (wy + wh))))
     return 0;

   if (!XQueryTree(_ecore_x_disp, base, &root_win, &parent_win, &list, &num))
     return base;

   if (list)
     {
	int skipit = 0;
	
	for (i = num - 1; i >= 0; --i)
	  {
	     skipit = 0;
	     
	     if (skip)
	       {
		  for (j = 0; j < skip_num; j++)
		    {
		       if (list[i] == skip[j])
			 {
			    skipit = 1;
			    goto onward;
			 }
		    }
	       }
	     onward:
	     if (!skipit)
	       {
		  if ((child = _ecore_x_window_at_xy_get(list[i], wx, wy, x, y, skip, skip_num)))
		    {
		       XFree(list);
		       return child;
		    }
	       }
	  }
	XFree(list);
     }
   return base;
}

/**
 * Retrieves the top, visible window at the given location.
 * @param   x The given X position.
 * @param   y The given Y position.
 * @return  The window at that position.
 * @ingroup Ecore_X_Window_Geometry_Group
 */
EAPI Ecore_X_Window
ecore_x_window_at_xy_get(int x, int y)
{
   Ecore_X_Window    win, root;
   
   /* FIXME: Proper function to determine current root/virtual root
    * window missing here */
   root = DefaultRootWindow(_ecore_x_disp);
   
   ecore_x_grab();
   win = _ecore_x_window_at_xy_get(root, 0, 0, x, y, NULL, 0);
   ecore_x_ungrab();
   
   return win ? win : root;
}

/**
 * Retrieves the top, visible window at the given location,
 * but skips the windows in the list.
 * @param   x The given X position.
 * @param   y The given Y position.
 * @return  The window at that position.
 * @ingroup Ecore_X_Window_Geometry_Group
 */
EAPI Ecore_X_Window
ecore_x_window_at_xy_with_skip_get(int x, int y, Ecore_X_Window *skip, int skip_num)
{
   Ecore_X_Window    win, root;
   
   /* FIXME: Proper function to determine current root/virtual root
    * window missing here */
   root = DefaultRootWindow(_ecore_x_disp);
   
   ecore_x_grab();
   win = _ecore_x_window_at_xy_get(root, 0, 0, x, y, skip, skip_num);
   ecore_x_ungrab();
   
   return win ? win : root;
}

EAPI Ecore_X_Window
ecore_x_window_at_xy_begin_get(Ecore_X_Window begin, int x, int y)
{
   Ecore_X_Window    win;
   
   ecore_x_grab();
   win = _ecore_x_window_at_xy_get(begin, 0, 0, x, y, NULL, 0);
   ecore_x_ungrab();
   
   return win ? win : begin;
}

/**
 * Retrieves the parent window of the given window.
 * @param   win The given window.
 * @return  The parent window of @p win.
 * @ingroup Ecore_X_Window_Parent_Group
 */
EAPI Ecore_X_Window
ecore_x_window_parent_get(Ecore_X_Window win)
{
   Window         root, parent, *children = NULL;
   unsigned int   num;

   if (!XQueryTree(_ecore_x_disp, win, &root, &parent, &children, &num))
      return 0;
   if (children)
      XFree(children);

   return parent;
}

/**
 * Sets the background color of the given window.
 * @param win   The given window
 * @param r     red value (0...65536, 16 bits)
 * @param g     green value (0...65536, 16 bits)
 * @param b     blue value (0...65536, 16 bits)
 */
EAPI void
ecore_x_window_background_color_set(Ecore_X_Window win, unsigned short r,
				    unsigned short g, unsigned short b)
{
   XSetWindowAttributes attr;
   Colormap map;
   XColor col;

   col.red = r;
   col.green = g;
   col.blue = b;

   map = DefaultColormap(_ecore_x_disp, DefaultScreen(_ecore_x_disp));
   XAllocColor(_ecore_x_disp, map, &col);

   attr.background_pixel = col.pixel;
   XChangeWindowAttributes(_ecore_x_disp, win, CWBackPixel, &attr);
}

EAPI void
ecore_x_window_gravity_set(Ecore_X_Window win, Ecore_X_Gravity grav)
{
   XSetWindowAttributes att;
   
   att.win_gravity = grav;
   XChangeWindowAttributes(_ecore_x_disp, win, CWWinGravity, &att);
}

EAPI void
ecore_x_window_pixel_gravity_set(Ecore_X_Window win, Ecore_X_Gravity grav)
{
   XSetWindowAttributes att;
   
   att.bit_gravity = grav;
   XChangeWindowAttributes(_ecore_x_disp, win, CWBitGravity, &att);
}

EAPI void
ecore_x_window_pixmap_set(Ecore_X_Window win, Ecore_X_Pixmap pmap)
{
   XSetWindowBackgroundPixmap(_ecore_x_disp, win, pmap);
}

EAPI void
ecore_x_window_area_clear(Ecore_X_Window win, int x, int y, int w, int h)
{
   XClearArea(_ecore_x_disp, win, x, y, w, h, False);
}

EAPI void
ecore_x_window_area_expose(Ecore_X_Window win, int x, int y, int w, int h)
{
   XClearArea(_ecore_x_disp, win, x, y, w, h, True);
}

EAPI void
ecore_x_window_override_set(Ecore_X_Window win, int override)
{
   XSetWindowAttributes att;
   
   att.override_redirect = override;
   XChangeWindowAttributes(_ecore_x_disp, win, CWOverrideRedirect, &att);
}

#ifdef ECORE_XRENDER   
static Ecore_X_Window
_ecore_x_window_argb_internal_new(Ecore_X_Window parent, int x, int y, int w, int h, int override, int saveunder)
{
   Window                win;
   XSetWindowAttributes  attr;
   XWindowAttributes     att;
   XVisualInfo          *xvi;
   XVisualInfo           vi_in;
   int                   nvi, i, scr = 0;
   XRenderPictFormat    *fmt;
   Visual               *vis;
   
   if (parent == 0)
     {
	parent = DefaultRootWindow(_ecore_x_disp);
	scr = DefaultScreen(_ecore_x_disp);
     }
   else
     {
	/* ewww - round trip */
	XGetWindowAttributes(_ecore_x_disp, parent, &att);
	for (i = 0; i < ScreenCount(_ecore_x_disp); i++)
	  {
	     if (att.screen == ScreenOfDisplay(_ecore_x_disp, i))
	       {
		  scr = i;
		  break;
	       }
	  }
     }
   vi_in.screen = scr;
   vi_in.depth = 32;
   vi_in.class = TrueColor;
   xvi = XGetVisualInfo(_ecore_x_disp,
			VisualScreenMask |
			VisualDepthMask |
			VisualClassMask,
			&vi_in,
			&nvi);
   if (xvi == NULL) return 0;
   vis = NULL;
   for (i = 0; i < nvi; i++)
     {
	fmt = XRenderFindVisualFormat(_ecore_x_disp, xvi[i].visual);
	if ((fmt->type == PictTypeDirect) && (fmt->direct.alphaMask))
	  {
	     vis = xvi[i].visual;
	     break;
	  }
     }
   XFree (xvi);
   
   attr.backing_store         = NotUseful;
   attr.override_redirect     = override;
   attr.colormap              = XCreateColormap(_ecore_x_disp, parent,
						vis, AllocNone);
   attr.border_pixel          = 0;
   attr.background_pixmap     = None;
   attr.bit_gravity           = NorthWestGravity;
   attr.win_gravity           = NorthWestGravity;
   attr.save_under            = saveunder;
   attr.do_not_propagate_mask = NoEventMask;
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
		       32, 
		       InputOutput, 
		       vis,
		       CWBackingStore |
		       CWOverrideRedirect | 
		       CWColormap |
		       CWBorderPixel |
		       CWBackPixmap | 
		       CWSaveUnder | 
		       CWDontPropagate | 
		       CWEventMask |
		       CWBitGravity |
		       CWWinGravity,
		       &attr);
   XFreeColormap(_ecore_x_disp, attr.colormap);

   if (parent == DefaultRootWindow(_ecore_x_disp)) ecore_x_window_defaults_set(win);
   return win;
}
#endif

EAPI int
ecore_x_window_argb_get(Ecore_X_Window win)
{
#ifdef ECORE_XRENDER   
   XWindowAttributes att;
   XRenderPictFormat *fmt;
   
   XGetWindowAttributes(_ecore_x_disp, win, &att);
   fmt = XRenderFindVisualFormat(_ecore_x_disp, att.visual);
   if (!fmt) return 0;
   if ((fmt->type == PictTypeDirect) && (fmt->direct.alphaMask)) return 1;
   return 0;
#else
   return 0;
#endif   
}

/**
 * Creates a new window.
 * @param   parent The parent window to use.  If @p parent is @c 0, the root
 *                 window of the default display is used.
 * @param   x      X position.
 * @param   y      Y position.
 * @param   w      Width.
 * @param   h      Height.
 * @return  The new window handle.
 * @ingroup Ecore_X_Window_Create_Group
 */
EAPI Ecore_X_Window
ecore_x_window_manager_argb_new(Ecore_X_Window parent, int x, int y, int w, int h)
{
#ifdef ECORE_XRENDER   
   return _ecore_x_window_argb_internal_new(parent, x, y, w, h, 1, 0);
#else
   return 0;
#endif   
}

/**
 * Creates a new window.
 * @param   parent The parent window to use.  If @p parent is @c 0, the root
 *                 window of the default display is used.
 * @param   x      X position.
 * @param   y      Y position.
 * @param   w      Width.
 * @param   h      Height.
 * @return  The new window handle.
 * @ingroup Ecore_X_Window_Create_Group
 */
EAPI Ecore_X_Window
ecore_x_window_argb_new(Ecore_X_Window parent, int x, int y, int w, int h)
{
#ifdef ECORE_XRENDER   
   return _ecore_x_window_argb_internal_new(parent, x, y, w, h, 0, 0);
#else
   return 0;
#endif   
}

/**
 * Creates a window with the override redirect attribute set to @c True.
 * @param   parent The parent window to use.  If @p parent is @c 0, the root
 *                 window of the default display is used.
 * @param   x      X position.
 * @param   y      Y position.
 * @param   w      Width.
 * @param   h      Height.
 * @return  The new window handle.
 * @ingroup Ecore_X_Window_Create_Group
 */
EAPI Ecore_X_Window
ecore_x_window_override_argb_new(Ecore_X_Window parent, int x, int y, int w, int h)
{
#ifdef ECORE_XRENDER   
   return _ecore_x_window_argb_internal_new(parent, x, y, w, h, 1, 0);
#else
   return 0;
#endif   
}
