#include "Ecore.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef XA_CLIPBOARD
#define X_CLIPBOARD_SELECTION  XA_CLIPBOARD(disp)
#define X_CLIPBOARD_PROP       XA_CLIPBOARD(disp)
#else
#define X_CLIPBOARD_SELECTION  XA_PRIMARY
#define X_CLIPBOARD_PROP       XA_CUT_BUFFER0
#endif

typedef struct _window_list Window_List;

struct _window_list
{
   Window              win;
   Window_List        *next;
};

static Display     *disp;
static Visual      *default_vis;
static Colormap     default_cm;
static int          default_depth;
static Window       default_win;
static Window       default_root;

static XContext     xid_context = 0;

static int          lock_scroll = 0;
static int          lock_num = 0;
static int          lock_caps = 0;

static Window       focused_win = 0;

static int          mod_shift = 0;
static int          mod_ctrl = 0;
static int          mod_alt = 0;
static int          mod_win = 0;

static Window       grabkey_win = 0;

static int          mouse_x = 0, mouse_y = 0;

static Window       current_dnd_win = 0;
static int          current_dnd_target_ok = 0;

static int          x_grabs = 0;

static Window_List *ignore_wins = NULL;

static Window       grab_pointer_win = 0;

static int          dnd_copy = 0;
static int          dnd_link = 0;
static int          dnd_move = 1;

static void         e_handle_x_error(Display * d, XErrorEvent * ev);
static void         e_handle_x_io_error(Display * d);
static Window       e_window_at_xy_0(Window base, int bx, int by, int x, int y);

static void
e_handle_x_error(Display * d, XErrorEvent * ev)
{
/*   
   char err[16384];
   
   XGetErrorText(d, ev->error_code, err, 16000);   
    printf("X Error:\n"
	  "Error: %s\nrequest: %i\nminor: %i\n", err, ev->request_code, ev->minor_code);
 */
   /* ignore all X errors */
   return;
   d = NULL;
   ev = NULL;
}

static void
e_handle_x_io_error(Display * d)
{
   /* FIXME: call clean exit handler */
   exit(1);
   d = NULL;
}

void
e_del_child(Window win, Window child)
{
   E_XID              *xid = NULL;

   if (XFindContext(disp, win, xid_context, (XPointer *) & xid) == XCNOENT)
      return;
   if (xid)
     {
	int                 i;

	for (i = 0; i < xid->children_num; i++)
	  {
	     if (xid->children[i] == child)
	       {
		  int                 j;

		  for (j = i; j < xid->children_num - 1; j++)
		     xid->children[j] = xid->children[j + 1];
		  xid->children_num--;
		  REALLOC(xid->children, Window, xid->children_num);
		  return;
	       }
	  }
     }
}

void
e_add_child(Window win, Window child)
{
   E_XID              *xid = NULL;

   if (XFindContext(disp, win, xid_context, (XPointer *) & xid) == XCNOENT)
     return;
   if (xid)
     {
	int i;
	
	for (i = 0; i < xid->children_num; i++)
	  {
	     if (xid->children[i] == child) return;
	  }
	xid->children_num++;
	if (!xid->children)
	   xid->children = NEW(Window, xid->children_num);
	else
	   REALLOC(xid->children, Window, xid->children_num);
	xid->children[xid->children_num - 1] = child;
     }
   xid = e_validate_xid(child);   
}

void
e_raise_child(Window win, Window child)
{
   E_XID              *xid = NULL;

   if (XFindContext(disp, win, xid_context, (XPointer *) & xid) == XCNOENT)
      return;
   if (xid)
     {
	int                 i;

	for (i = 0; i < xid->children_num; i++)
	  {
	     if (xid->children[i] == child)
	       {
		  int                 j;

		  for (j = i; j < xid->children_num - 1; j++)
		     xid->children[j] = xid->children[j + 1];
		  xid->children[xid->children_num - 1] = child;
		  return;
	       }
	  }
     }
}

void
e_lower_child(Window win, Window child)
{
   E_XID              *xid = NULL;

   if (XFindContext(disp, win, xid_context, (XPointer *) & xid) == XCNOENT)
      return;
   if (xid)
     {
	int                 i;

	for (i = 0; i < xid->children_num; i++)
	  {
	     if (xid->children[i] == child)
	       {
		  int                 j;

		  for (j = i; j > 0; j--)
		     xid->children[j] = xid->children[j - 1];
		  xid->children[0] = child;
		  return;
	       }
	  }
     }
}

E_XID              *
e_add_xid(Window win, int x, int y, int w, int h, int depth, Window parent)
{
   E_XID              *xid = NULL;

   e_window_add_events(win, XEV_IN_OUT | XEV_CONFIGURE | XEV_VISIBILITY);
   xid = NEW(E_XID, 1);
   xid->win = win;
   xid->x = x;
   xid->y = y;
   xid->w = w;
   xid->h = h;
   xid->mapped = 0;
   xid->depth = depth;
   xid->mouse_in = 0;
   xid->parent = parent;
   xid->root = e_window_get_root(parent);
   xid->children_num = 0;
   xid->children = NULL;
   xid->gravity = e_window_get_gravity(win);
   xid->bw = 0;
   XSaveContext(disp, xid->win, xid_context, (XPointer) xid);
   e_add_child(parent, win);
   return xid;
}

E_XID              *
e_validate_xid(Window win)
{
   E_XID              *xid = NULL;

   if (XFindContext(disp, win, xid_context, (XPointer *) & xid) == XCNOENT)
     {
	XWindowAttributes   att;
	Window              root_ret = 0, parent_ret = 0, *children_ret = NULL;
	unsigned int        children_ret_num = 0;

	e_window_add_events(win, XEV_IN_OUT | XEV_CONFIGURE |
			    XEV_VISIBILITY | XEV_CHILD_CHANGE);
	xid = NEW(E_XID, 1);
	xid->win = win;
	if (!XGetWindowAttributes(disp, win, &att))
	  {
	     FREE(xid);
	     return NULL;
	  }
	if (!XQueryTree(disp, win, &root_ret, &parent_ret, &children_ret,
			&children_ret_num))
	  {
	     FREE(xid);
	     return NULL;
	  }
	xid->parent = parent_ret;
	if (xid->parent)
	   e_validate_xid(xid->parent);
	if (children_ret)
	  {
	     xid->children_num = children_ret_num;
	     xid->children = NEW(Window, children_ret_num);
	     MEMCPY(children_ret, xid->children, Window, children_ret_num);
	     XFree(children_ret);
	  }
	else
	  {
	     xid->children_num = 0;
	     xid->children = NULL;
	  }
	xid->root = root_ret;
	xid->x = att.x;
	xid->y = att.y;
	xid->w = att.width;
	xid->h = att.height;
	if (att.map_state == IsUnmapped)
	   xid->mapped = 0;
	else
	   xid->mapped = 1;
	xid->depth = att.depth;
	xid->mouse_in = 0;
	xid->gravity = att.win_gravity;
	xid->bw = att.border_width;
	XSaveContext(disp, xid->win, xid_context, (XPointer) xid);
	e_add_child(xid->parent, win);
     }
   return xid;
}

void
e_unvalidate_xid(Window win)
{
   E_XID              *xid = NULL;

   if (XFindContext(disp, win, xid_context, (XPointer *) & xid) == XCNOENT)
     return;
   if (xid)
     {
	int                 i;

	for (i = 0; i < xid->children_num; i++)
	   e_unvalidate_xid(xid->children[i]);
	e_del_child(xid->parent, win);	
	IF_FREE(xid->children);
	FREE(xid);
	XDeleteContext(disp, win, xid_context);
     }
}

void
e_sync(void)
{
   XSync(disp, False);
}

void
e_flush(void)
{
   XFlush(disp);
}

Window
e_window_new(Window parent, int x, int y, int w, int h)
{
   Window              win;
   XSetWindowAttributes attr;

   if (!parent)
      parent = default_root;
   attr.backing_store = NotUseful;
   attr.override_redirect = False;
   attr.colormap = default_cm;
   attr.border_pixel = 0;
   attr.background_pixmap = None;
   attr.save_under = False;
   attr.do_not_propagate_mask = True;
   win = XCreateWindow(disp, parent,
		       x, y, w, h, 0,
		       default_depth, InputOutput, default_vis,
		       CWOverrideRedirect | CWSaveUnder | CWBackingStore |
		       CWColormap | CWBackPixmap | CWBorderPixel |
		       CWDontPropagate, &attr);
   e_add_xid(win, x, y, w, h, default_depth, parent);
   e_add_child(parent, win);
   e_validate_xid(parent);
   return win;
}

Window
e_window_override_new(Window parent, int x, int y, int w, int h)
{
   Window              win;
   XSetWindowAttributes attr;

   if (!parent)
      parent = default_root;
   attr.backing_store = NotUseful;
   attr.override_redirect = True;
   attr.colormap = default_cm;
   attr.border_pixel = 0;
   attr.background_pixmap = None;
   attr.save_under = False;
   attr.do_not_propagate_mask = True;
   win = XCreateWindow(disp, parent,
		       x, y, w, h, 0,
		       default_depth, InputOutput, default_vis,
		       CWOverrideRedirect | CWSaveUnder | CWBackingStore |
		       CWColormap | CWBackPixmap | CWBorderPixel |
		       CWDontPropagate, &attr);
   e_add_xid(win, x, y, w, h, default_depth, parent);
   e_add_child(parent, win);
   e_validate_xid(parent);
   return win;
}

Window
e_window_input_new(Window parent, int x, int y, int w, int h)
{
   Window              win;
   XSetWindowAttributes attr;

   if (!parent)
      parent = default_root;
   attr.override_redirect = True;
   attr.do_not_propagate_mask = True;
   win = XCreateWindow(disp, parent,
		       x, y, w, h, 0,
		       0, InputOnly, default_vis,
		       CWOverrideRedirect | CWDontPropagate, &attr);
   e_add_xid(win, x, y, w, h, 0, parent);
   e_add_child(parent, win);
   e_validate_xid(parent);
   return win;
}

void
e_window_set_events_propagate(Window win, int propagate)
{
   XSetWindowAttributes attr;
   
   if (!win)
     win = default_root;
   if (!propagate)
     attr.do_not_propagate_mask = True;
   else
     attr.do_not_propagate_mask = False;
   XChangeWindowAttributes(disp, win, CWDontPropagate, &attr);
}

void
e_window_show(Window win)
{
   E_XID              *xid = NULL;

   xid = e_validate_xid(win);
   if (xid)
     {
	if (xid->mapped)
	  return;
	xid->mapped = 1;
	XMapWindow(disp, win);
     }
}

void
e_window_hide(Window win)
{
   E_XID              *xid = NULL;

   xid = e_validate_xid(win);
   if (xid)
     {
	if (!xid->mapped)
	   return;
	xid->mapped = 0;
	XUnmapWindow(disp, win);
     }
}

Pixmap
e_pixmap_new(Window win, int w, int h, int dep)
{
   if (!win)
      win = default_win;
   if (dep == 0)
      dep = default_depth;
   return XCreatePixmap(disp, win, w, h, dep);
}

void
e_pixmap_free(Pixmap pmap)
{
   if (!pmap)
      return;
   XFreePixmap(disp, pmap);
}

void
e_window_set_background_pixmap(Window win, Pixmap pmap)
{
   XSetWindowBackgroundPixmap(disp, win, pmap);
}

void
e_window_set_shape_mask(Window win, Pixmap mask)
{
   XShapeCombineMask(disp, win, ShapeBounding, 0, 0, mask, ShapeSet);
}

void
e_window_add_shape_mask(Window win, Pixmap mask)
{
   XShapeCombineMask(disp, win, ShapeBounding, 0, 0, mask, ShapeUnion);
}

void
e_window_set_shape_window(Window win, Window src, int x, int y)
{
   XShapeCombineShape(disp, win, ShapeBounding, x, y, src, ShapeBounding, ShapeSet);
}

void
e_window_add_shape_window(Window win, Window src, int x, int y)
{
   XShapeCombineShape(disp, win, ShapeBounding, x, y, src, ShapeBounding, ShapeUnion);
}

void
e_window_set_shape_rectangle(Window win, int x, int y, int w, int h)
{
   XRectangle rect;
   
   rect.x = x;
   rect.y = y;
   rect.width = w;
   rect.height = h;
   XShapeCombineRectangles(disp, win, ShapeBounding, 0, 0, &rect, 1, ShapeSet, Unsorted);
}

void
e_window_add_shape_rectangle(Window win, int x, int y, int w, int h)
{
   XRectangle rect;
   
   rect.x = x;
   rect.y = y;
   rect.width = w;
   rect.height = h;
   XShapeCombineRectangles(disp, win, ShapeBounding, 0, 0, &rect, 1, ShapeUnion, Unsorted);
}

void
e_window_set_shape_rectangles(Window win, XRectangle *rect, int num)
{
   XShapeCombineRectangles(disp, win, ShapeBounding, 0, 0, rect, num, ShapeSet, Unsorted);
}

void
e_window_add_shape_rectangles(Window win, XRectangle *rect, int num)
{
   XShapeCombineRectangles(disp, win, ShapeBounding, 0, 0, rect, num, ShapeUnion, Unsorted);
}

void
e_window_clip_shape_by_rectangle(Window win, int x, int y, int w, int h)
{
   XRectangle rect;
   
   rect.x = x;
   rect.y = y;
   rect.width = w;
   rect.height = h;
   XShapeCombineRectangles(disp, win, ShapeBounding, 0, 0, &rect, 1, ShapeIntersect, Unsorted);
}

XRectangle *
e_window_get_shape_rectangles(Window win, int *num)
{
   int ord;
   
   return XShapeGetRectangles(disp, win, ShapeBounding, num, &ord);
}

void
e_window_select_shape_events(Window win)
{
   XShapeSelectInput(disp, win, ShapeNotifyMask);
}

void
e_window_unselect_shape_events(Window win)
{
   XShapeSelectInput(disp, win, 0);
}

void
e_window_clear(Window win)
{
   XClearWindow(disp, win);
}

void
e_window_clear_area(Window win, int x, int y, int w, int h)
{
   XClearArea(disp, win, x, y, w, h, False);
}

void
e_pointer_xy(Window win, int *x, int *y)
{
   Window              dw;
   unsigned int        dm;
   int                 wx, wy;

   if (win == 0)
      win = default_root;
   XQueryPointer(disp, win, &dw, &dw, &mouse_x, &mouse_y, &wx, &wy, &dm);
   if (x)
      *x = wx;
   if (y)
      *y = wy;
}

void
e_pointer_xy_set(int x, int y)
{
   mouse_x = x;
   mouse_y = y;
}

void
e_pointer_xy_get(int *x, int *y)
{
   if (x)
      *x = mouse_x;
   if (y)
      *y = mouse_y;
}

void
e_window_set_events(Window win, long mask)
{
   if (win == 0)
      win = default_root;
   XSelectInput(disp, win, mask);
}

void
e_window_remove_events(Window win, long mask)
{
   XWindowAttributes   att;

   if (win == 0)
      win = default_root;
   if (XGetWindowAttributes(disp, win, &att) == True)
     {
	mask = att.your_event_mask & (~mask);
	e_window_set_events(win, mask);
     }
}

void
e_window_add_events(Window win, long mask)
{
   XWindowAttributes   att;

   if (win == 0)
      win = default_root;
   if (XGetWindowAttributes(disp, win, &att) == True)
     {
	mask = att.your_event_mask | mask;
	e_window_set_events(win, mask);
     }
}

void
e_window_move(Window win, int x, int y)
{
   E_XID              *xid = NULL;

   xid = e_validate_xid(win);
   if (xid)
     {
	if (!xid->coords_invalid)
	  {
	     if ((xid->x == x) && (xid->y == y))
	       return;
	  }
	xid->x = x;
	xid->y = y;
	xid->coords_invalid = 0;
	XMoveWindow(disp, win, x, y);
     }
}

#define REGRAVITATE \
if (xid->children) \
{ \
   int                 j; \
\
   for (j = 0; j < xid->children_num; j++) \
     { \
	E_XID *xid2; \
\
	xid2 = e_validate_xid(xid->children[j]); \
	if (xid2) \
	  { \
             xid2->coords_invalid = 1; \
          } \
     } \
}

#if 0
             switch (xid2->gravity) \
	       { \
		case UnmapGravity: \
                  xid2->mapped = 0; \
		  break; \
		case NorthWestGravity: \
		  break; \
		case NorthGravity: \
		  xid2->x += (w - xid->w) / 2; \
		  break; \
		case NorthEastGravity: \
		  xid2->x += (w - xid->w); \
		  break; \
		case WestGravity: \
		  xid2->h += (h - xid->h) / 2; \
		  break; \
		case CenterGravity: \
		  xid2->x += (w - xid->w) / 2; \
		  xid2->h += (h - xid->h) / 2; \
		  break; \
		case EastGravity: \
		  xid2->x += (w - xid->w); \
		  break; \
		case SouthWestGravity: \
		  xid2->y += (h - xid->h); \
		  break; \
		case SouthGravity: \
		  xid2->x += (w - xid->w) / 2; \
		  xid2->y += (h - xid->h); \
		  break; \
		case SouthEastGravity: \
		  xid2->x += (w - xid->w); \
		  xid2->y += (h - xid->h); \
		  break; \
		case StaticGravity: \
                  xid2->coords_invalid = 1; \
		  break; \
		default: \
		  break; \
	       } \
	  } \
     } \
}
#endif

void
e_window_resize(Window win, int w, int h)
{
   E_XID              *xid = NULL;

   xid = e_validate_xid(win);
   if (xid)
     {
	if (!xid->coords_invalid)
	  {
	     if ((xid->w == w) && (xid->h == h))
	       return;
	  }
	REGRAVITATE;
	xid->w = w;
	xid->h = h;
	xid->coords_invalid = 0;
	XResizeWindow(disp, win, w, h);
     }
}

void
e_window_move_resize(Window win, int x, int y, int w, int h)
{
   E_XID              *xid = NULL;

   xid = e_validate_xid(win);
   if (xid)
     {
	if (!xid->coords_invalid)
	  {
	     if ((xid->x == x) && (xid->y == y) && (xid->w == w) && (xid->h == h))
	       return;
	  }
	REGRAVITATE;
	xid->x = x;
	xid->y = y;
	xid->w = w;
	xid->h = h;
	xid->coords_invalid = 0;
	XMoveResizeWindow(disp, win, x, y, w, h);
     }
}

int
e_x_get_fd(void)
{
   return ConnectionNumber(disp);
}

void
e_set_error_handler(void (*func) (Display * d, XErrorEvent * ev))
		    
{
   XSetErrorHandler((XErrorHandler)func);
}

void
e_reset_error_handler(void)
{
   XSetErrorHandler((XErrorHandler) e_handle_x_error);   
}

int
e_display_init(char *display)
{
   int                 revert;

   xid_context = XUniqueContext();
   disp = XOpenDisplay(display);
   if (!disp)
     {
	char               *d;

	d = getenv("DISPLAY");
/* no need for this anymore	
	if (d)
	   fprintf(stderr,
		   "Fatal Error:\n"
		   "Cannot connect to the display nominated by your DISPLAY variable:\n"
		   "%s\n"
		   "Try changing your DISPLAY variable like:\n"
		   "DISPLAY=host:0 application_name\n", d);
	else
	   fprintf(stderr,
		   "Fatal Error:\n"
		   "No DISPLAY variable set so cannot determine display to connect to.\n"
		   "Try setting your DISPLAY variable like:\n"
		   "DISPLAY=host:0 appication_name\n");
*/
	return 0;
     }
   XSetErrorHandler((XErrorHandler) e_handle_x_error);
   XSetIOErrorHandler((XIOErrorHandler) e_handle_x_io_error);
   default_vis = DefaultVisual(disp, DefaultScreen(disp));
   default_depth = DefaultDepth(disp, DefaultScreen(disp));
   default_cm = DefaultColormap(disp, DefaultScreen(disp));
   default_win = DefaultRootWindow(disp);
   default_root = DefaultRootWindow(disp);
   mod_shift = e_mod_mask_shift_get();
   mod_ctrl = e_mod_mask_ctrl_get();
   mod_alt = e_mod_mask_alt_get();
   mod_win = e_mod_mask_win_get();
   XGetInputFocus(disp, &focused_win, &revert);
   e_window_set_events(default_root, XEV_KEY | XEV_IN_OUT | XEV_MOUSE_MOVE |
		       XEV_CONFIGURE | XEV_CHILD_CHANGE | XEV_PROPERTY |
		       XEV_COLORMAP | XEV_VISIBILITY);
   e_pointer_xy(0, NULL, NULL);
   return 1;
}

int
e_events_pending(void)
{
   if (!disp) return 0;
   return XPending(disp);
}

void
e_get_next_event(XEvent * event)
{
   XNextEvent(disp, event);
}

int
e_event_shape_get_id(void)
{
   int                 base = -1, err_base;

   XShapeQueryExtension(disp, &base, &err_base);
   base += ShapeNotify;
   return base;
}

KeySym
e_key_get_keysym_from_keycode(KeyCode keycode)
{
   return XKeycodeToKeysym(disp, keycode, 0);
}

char               *
e_key_get_string_from_keycode(KeyCode keycode)
{
   char *str;
   
   str = XKeysymToString(e_key_get_keysym_from_keycode(keycode));
   if (!str) return strdup("");
   return strdup(str);
}

void
e_event_allow(int mode, Time t)
{
   XAllowEvents(disp, mode, t);
}

int
e_lock_mask_scroll_get(void)
{
   static int          have_mask = 0;
   static int          mask = 0;
   XModifierKeymap    *mod;
   KeyCode             nl;
   int                 i;
   int                 masks[8] = {
      ShiftMask, LockMask, ControlMask, Mod1Mask, Mod2Mask, Mod3Mask,
      Mod4Mask, Mod5Mask
   };

   if (have_mask)
      return mask;
   mod = XGetModifierMapping(disp);
   nl = XKeysymToKeycode(disp, XK_Scroll_Lock);
   if ((mod) && (mod->max_keypermod > 0))
     {
	for (i = 0; i < (8 * mod->max_keypermod); i++)
	  {
	     if ((nl) && (mod->modifiermap[i] == nl))
	       {
		  mask = masks[i / mod->max_keypermod];
                  if (mod->modifiermap)
		     XFree(mod->modifiermap);
		  XFree(mod);
		  have_mask = 1;
		  return mask;
	       }
	  }
     }
   if (mod)
     {
	if (mod->modifiermap)
	   XFree(mod->modifiermap);
	XFree(mod);
     }
   return 0;
}

int
e_lock_mask_num_get(void)
{
   static int          have_mask = 0;
   static int          mask = 0;
   XModifierKeymap    *mod;
   KeyCode             nl;
   int                 i;
   int                 masks[8] = {
      ShiftMask, LockMask, ControlMask, Mod1Mask, Mod2Mask, Mod3Mask,
      Mod4Mask, Mod5Mask
   };

   if (have_mask)
      return mask;
   mod = XGetModifierMapping(disp);
   nl = XKeysymToKeycode(disp, XK_Num_Lock);
   if ((mod) && (mod->max_keypermod > 0))
     {
	for (i = 0; i < (8 * mod->max_keypermod); i++)
	  {
	     if ((nl) && (mod->modifiermap[i] == nl))
	       {
		  mask = masks[i / mod->max_keypermod];
                  if (mod->modifiermap)
		     XFree(mod->modifiermap);
		  XFree(mod);
		  have_mask = 1;
		  return mask;
	       }
	  }
     }
   if (mod)
     {
	if (mod->modifiermap)
	   XFree(mod->modifiermap);
	XFree(mod);
     }
   return 0;
}

int
e_lock_mask_caps_get(void)
{
   static int          have_mask = 0;
   static int          mask = 0;
   XModifierKeymap    *mod;
   KeyCode             nl;
   int                 i;
   int                 masks[8] = {
      ShiftMask, LockMask, ControlMask, Mod1Mask, Mod2Mask, Mod3Mask,
      Mod4Mask, Mod5Mask
   };

   if (have_mask)
      return mask;
   mod = XGetModifierMapping(disp);
   nl = XKeysymToKeycode(disp, XK_Caps_Lock);
   if ((mod) && (mod->max_keypermod > 0))
     {
	for (i = 0; i < (8 * mod->max_keypermod); i++)
	  {
	     if ((nl) && (mod->modifiermap[i] == nl))
	       {
		  mask = masks[i / mod->max_keypermod];
                  if (mod->modifiermap)
		     XFree(mod->modifiermap);
		  XFree(mod);
		  have_mask = 1;
		  return mask;
	       }
	  }
     }
   if (mod)
     {
	if (mod->modifiermap)
	   XFree(mod->modifiermap);
	XFree(mod);
     }
   return 0;
}

int
e_mod_mask_shift_get(void)
{
   static int          have_mask = 0;
   static int          mask = 0;
   XModifierKeymap    *mod;
   KeyCode             nl;
   int                 i;
   int                 masks[8] = {
      ShiftMask, LockMask, ControlMask, Mod1Mask, Mod2Mask, Mod3Mask,
      Mod4Mask, Mod5Mask
   };

   if (have_mask)
      return mask;
   mod = XGetModifierMapping(disp);
   nl = XKeysymToKeycode(disp, XK_Shift_L);
   if ((mod) && (mod->max_keypermod > 0))
     {
	for (i = 0; i < (8 * mod->max_keypermod); i++)
	  {
	     if ((nl) && (mod->modifiermap[i] == nl))
	       {
		  mask = masks[i / mod->max_keypermod];
                  if (mod->modifiermap)
		     XFree(mod->modifiermap);
		  XFree(mod);
		  have_mask = 1;
		  return mask;
	       }
	  }
     }
   if (mod)
     {
	if (mod->modifiermap)
	   XFree(mod->modifiermap);
	XFree(mod);
     }
   return 0;
}

int
e_mod_mask_ctrl_get(void)
{
   static int          have_mask = 0;
   static int          mask = 0;
   XModifierKeymap    *mod;
   KeyCode             nl;
   int                 i;
   int                 masks[8] = {
      ShiftMask, LockMask, ControlMask, Mod1Mask, Mod2Mask, Mod3Mask,
      Mod4Mask, Mod5Mask
   };

   if (have_mask)
      return mask;
   mod = XGetModifierMapping(disp);
   nl = XKeysymToKeycode(disp, XK_Control_L);
   if ((mod) && (mod->max_keypermod > 0))
     {
	for (i = 0; i < (8 * mod->max_keypermod); i++)
	  {
	     if ((nl) && (mod->modifiermap[i] == nl))
	       {
		  mask = masks[i / mod->max_keypermod];
                  if (mod->modifiermap)
		     XFree(mod->modifiermap);
		  XFree(mod);
		  have_mask = 1;
		  return mask;
	       }
	  }
     }
   if (mod)
     {
	if (mod->modifiermap)
	   XFree(mod->modifiermap);
	XFree(mod);
     }
   return 0;
}

int
e_mod_mask_alt_get(void)
{
   static int          have_mask = 0;
   static int          mask = 0;
   XModifierKeymap    *mod;
   KeyCode             nl;
   int                 i;
   int                 masks[8] = {
      ShiftMask, LockMask, ControlMask, Mod1Mask, Mod2Mask, Mod3Mask,
      Mod4Mask, Mod5Mask
   };

   if (have_mask)
      return mask;
   mod = XGetModifierMapping(disp);
   nl = XKeysymToKeycode(disp, XK_Alt_L);
   if ((mod) && (mod->max_keypermod > 0))
     {
	for (i = 0; i < (8 * mod->max_keypermod); i++)
	  {
	     if ((nl) && (mod->modifiermap[i] == nl))
	       {
		  mask = masks[i / mod->max_keypermod];
                  if (mod->modifiermap)
		     XFree(mod->modifiermap);
		  XFree(mod);
		  have_mask = 1;
		  return mask;
	       }
	  }
     }
   if (mod)
     {
	if (mod->modifiermap)
	   XFree(mod->modifiermap);
	XFree(mod);
     }
   return 0;
}

int
e_mod_mask_win_get(void)
{
   static int          have_mask = 0;
   static int          mask = 0;
   XModifierKeymap    *mod;
   KeyCode             nl;
   int                 i;
   int                 masks[8] = {
      ShiftMask, LockMask, ControlMask, Mod1Mask, Mod2Mask, Mod3Mask,
      Mod4Mask, Mod5Mask
   };

   if (have_mask)
      return mask;
   mod = XGetModifierMapping(disp);
   nl = XKeysymToKeycode(disp, XK_Meta_L);
   if ((mod) && (mod->max_keypermod > 0))
     {
	for (i = 0; i < (8 * mod->max_keypermod); i++)
	  {
	     if ((nl) && (mod->modifiermap[i] == nl))
	       {
		  mask = masks[i / mod->max_keypermod];
                  if (mod->modifiermap)
		     XFree(mod->modifiermap);
		  XFree(mod);
		  if (mask == e_mod_mask_alt_get())
		     mask = 0;
		  if (mask == e_mod_mask_ctrl_get())
		     mask = 0;
		  have_mask = 1;
		  return mask;
	       }
	  }
     }
   if (mod)
     {
	if (mod->modifiermap)
	   XFree(mod->modifiermap);
	XFree(mod);
     }
   return 0;
}

int
e_lock_mask_get(void)
{
   static int          mask = 0;
   Window              root_ret, child_ret;
   int                 root_x_ret, root_y_ret, win_x_ret, win_y_ret;
   unsigned int        mask_ret;

   if (!mask)
      mask = e_lock_mask_scroll_get() | e_lock_mask_num_get() |
	 e_lock_mask_caps_get();
   XQueryPointer(disp, default_root, &root_ret, &child_ret, &root_x_ret,
		 &root_y_ret, &win_x_ret, &win_y_ret, &mask_ret);
   return (mask_ret & mask);
}

int
e_modifier_mask_get(void)
{
   Window              root_ret, child_ret;
   int                 root_x_ret, root_y_ret, win_x_ret, win_y_ret;
   unsigned int        mask_ret;

   XQueryPointer(disp, default_root, &root_ret, &child_ret, &root_x_ret,
		 &root_y_ret, &win_x_ret, &win_y_ret, &mask_ret);
   return (mask_ret);
}

Window
e_get_key_grab_win(void)
{
   return grabkey_win;
}

void
e_key_grab(char *key, Ev_Key_Modifiers mods, int anymod, int sync)
{
   KeyCode             keycode;
   int                 i, mod, mask_scroll, mask_num, mask_caps, masks[8], mode;

   keycode = e_key_get_keycode(key);
   mod = 0;
   mode = GrabModeAsync;
   if (sync)
      mode = GrabModeSync;
   if (!grabkey_win)
     grabkey_win = default_root;
   if (mods & EV_KEY_MODIFIER_SHIFT)
      mod |= e_mod_mask_shift_get();
   if (mods & EV_KEY_MODIFIER_CTRL)
      mod |= e_mod_mask_ctrl_get();
   if (mods & EV_KEY_MODIFIER_ALT)
      mod |= e_mod_mask_alt_get();
   if (mods & EV_KEY_MODIFIER_WIN)
      mod |= e_mod_mask_win_get();
   mask_scroll = e_lock_mask_scroll_get();
   mask_num = e_lock_mask_num_get();
   mask_caps = e_lock_mask_caps_get();
   masks[0] = 0;
   masks[1] = mask_scroll;
   masks[2] = mask_num;
   masks[3] = mask_caps;
   masks[4] = mask_scroll | mask_num;
   masks[5] = mask_scroll | mask_caps;
   masks[6] = mask_num | mask_caps;
   masks[7] = mask_scroll | mask_num | mask_caps;
   if (anymod)
     XGrabKey(disp, keycode, AnyModifier, grabkey_win, False, mode, mode);
   else
     {
	for (i = 0; i < 8; i++)
	  XGrabKey(disp, keycode, masks[i] | mod, grabkey_win, False,
		   mode, mode);
     }
}

void
e_key_ungrab(char *key, Ev_Key_Modifiers mods, int anymod)
{
   KeyCode             keycode;

   keycode = e_key_get_keycode(key);
   if (anymod)
      XUngrabKey(disp, keycode, AnyModifier, default_root);
   else
     {
	int                 i, mod, mask_scroll, mask_num, mask_caps, masks[8];

	mod = 0;
	if (mods & EV_KEY_MODIFIER_SHIFT)
	  mod |= e_mod_mask_shift_get();
	if (mods & EV_KEY_MODIFIER_CTRL)
	  mod |= e_mod_mask_ctrl_get();
	if (mods & EV_KEY_MODIFIER_ALT)
	  mod |= e_mod_mask_alt_get();
	if (mods & EV_KEY_MODIFIER_WIN)
	  mod |= e_mod_mask_win_get();
	mask_scroll = e_lock_mask_scroll_get();
	mask_num = e_lock_mask_num_get();
	mask_caps = e_lock_mask_caps_get();
	masks[0] = 0;
	masks[1] = mask_scroll;
	masks[2] = mask_num;
	masks[3] = mask_caps;
	masks[4] = mask_scroll | mask_num;
	masks[5] = mask_scroll | mask_caps;
	masks[6] = mask_num | mask_caps;
	masks[7] = mask_scroll | mask_num | mask_caps;
	for (i = 0; i < 8; i++)
	   XUngrabKey(disp, keycode, masks[i] | mod, grabkey_win);
     }
}

KeyCode
e_key_get_keycode(char *key)
{
   return XKeysymToKeycode(disp, XStringToKeysym(key));
}

void
e_window_destroy(Window win)
{
   e_unvalidate_xid(win);
   XDestroyWindow(disp, win);
}

void
e_window_reparent(Window win, Window parent, int x, int y)
{
   E_XID              *xid = NULL;

   xid = e_validate_xid(win);
   if (xid)
     {
	if (parent == 0)
	   parent = default_root;
	XReparentWindow(disp, win, parent, x, y);
	e_del_child(xid->parent, win);
	e_add_child(parent, win);
	xid->parent = parent;
	xid->x = x;
	xid->y = y;
     }
}

void
e_window_raise(Window win)
{
   E_XID              *xid = NULL;

   xid = e_validate_xid(win);
   if (xid)
     {
	XRaiseWindow(disp, win);
	e_raise_child(xid->parent, win);
     }
}

void
e_window_lower(Window win)
{
   E_XID              *xid = NULL;

   xid = e_validate_xid(win);
   if (xid)
     {
	XLowerWindow(disp, win);
	e_lower_child(xid->parent, win);
     }
}

void
e_window_get_geometry(Window win, int *x, int *y, int *w, int *h)
{
   E_XID              *xid = NULL;

   if (win == 0)
      win = default_root;
   xid = e_validate_xid(win);
   if ((xid) && (xid->coords_invalid))
     {
	Window dw;
	int rx, ry;
	unsigned int rw, rh, di;
	
	XGetGeometry(disp, win, &dw, &rx, &ry, &rw, &rh, &di, &di);
	xid->x = rx;
	xid->y = ry;
	xid->w = (int)rw;
	xid->h = (int)rh;
	xid->coords_invalid = 0;
     }
   if (xid)
     {
	if (x)
	   *x = xid->x;
	if (y)
	   *y = xid->y;
	if (w)
	   *w = xid->w;
	if (h)
	   *h = xid->h;
     }
   else
     {
	if (x)
	   *x = 0;
	if (y)
	   *y = 0;
	if (w)
	   *w = 0;
	if (h)
	   *h = 0;
     }
}

int
e_window_get_depth(Window win)
{
   E_XID              *xid = NULL;

   if (win == 0)
      win = default_root;
   xid = e_validate_xid(win);
   if (xid)
      return xid->depth;
   return -1;
}

int
e_window_exists(Window win)
{
   E_XID              *xid = NULL;

   xid = e_validate_xid(win);
   if (xid)
      return 1;
   return 0;
}

Window
e_window_get_parent(Window win)
{
   E_XID              *xid = NULL;

   xid = e_validate_xid(win);
   if (xid)
      return xid->parent;
   return 0;
}

Window             *
e_window_get_children(Window win, int *num)
{
   E_XID              *xid = NULL;

   if (win == 0)
      win = default_root;
   xid = e_validate_xid(win);
   if (xid)
     {
	Window             *wlist = NULL;

	*num = xid->children_num;
	if (xid->children)
	  {
	     wlist = NEW(Window, xid->children_num);
	     MEMCPY(xid->children, wlist, Window, xid->children_num);
	  }
	return wlist;
     }
   *num = 0;
   return NULL;
}

int
e_window_mouse_in(Window win)
{
   E_XID              *xid = NULL;

   if (win == 0)
      win = default_root;
   xid = e_validate_xid(win);
   if (xid)
      return xid->mouse_in;
   return 0;
}

void
e_window_mouse_set_in(Window win, int in)
{
   E_XID              *xid = NULL;

   if (win == 0)
      win = default_root;
   xid = e_validate_xid(win);
   if (xid)
      xid->mouse_in = in;
}

Display            *
e_display_get(void)
{
   return disp;
}

Window
e_window_get_root(Window win)
{
   E_XID              *xid = NULL;

   xid = e_validate_xid(win);
   if (xid)
      return xid->root;
   return 0;
}

void
e_lock_scroll_set(int onoff)
{
   lock_scroll = onoff;
}

int
e_lock_scroll_get(void)
{
   return lock_scroll;
}

void
e_lock_num_set(int onoff)
{
   lock_num = onoff;
}

int
e_lock_num_get(void)
{
   return lock_num;
}

void
e_lock_caps_set(int onoff)
{
   lock_caps = onoff;
}

int
e_lock_caps_get(void)
{
   return lock_caps;
}

void
e_mod_shift_set(int onoff)
{
   mod_shift = onoff;
}

int
e_mod_shift_get(void)
{
   return mod_shift;
}

void
e_mod_ctrl_set(int onoff)
{
   mod_ctrl = onoff;
}

int
e_mod_ctrl_get(void)
{
   return mod_ctrl;
}

void
e_mod_alt_set(int onoff)
{
   mod_alt = onoff;
}

int
e_mod_alt_get(void)
{
   return mod_alt;
}

void
e_mod_win_set(int onoff)
{
   mod_win = onoff;
}

int
e_mod_win_get(void)
{
   return mod_win;
}

void
e_focus_window_set(Window win)
{
   focused_win = win;
}

Window
e_focus_window_get(void)
{
   return focused_win;
}

void
e_focus_to_window(Window win)
{
   if (win == 0)
      win = default_root;
   XSetInputFocus(disp, win, RevertToPointerRoot, CurrentTime);
}

Atom
e_atom_get(char *name)
{
   return XInternAtom(disp, name, False);
}

void
e_window_set_delete_inform(Window win)
{
   static Atom         protocols[1] = { 0 };

   E_ATOM(protocols[0], "WM_DELETE_WINDOW");
   XSetWMProtocols(disp, win, protocols, 1);
}

void
e_window_property_set(Window win, Atom type, Atom format, int size, void *data,
		      int number)
{
   if (win == 0)
      win = default_root;
   if (size != 32)
      XChangeProperty(disp, win, type, format, size, PropModeReplace,
		      (unsigned char *)data, number);
   else
     {
	long               *dat;
	int                 i, *ptr;

	dat = NEW(long, number);

	for (ptr = (int *)data, i = 0; i < number; i++)
	   dat[i] = ptr[i];
	XChangeProperty(disp, win, type, format, size, PropModeReplace,
			(unsigned char *)dat, number);
	FREE(dat);
     }
}

void               *
e_window_property_get(Window win, Atom type, Atom format, int *size)
{
   unsigned char      *retval;
   Atom                type_ret;
   unsigned long       bytes_after, num_ret;
   int                 format_ret;
   void               *data = NULL;

   retval = NULL;
   if (win == 0)
      win = default_root;
   XGetWindowProperty(disp, win, type, 0, 0x7fffffffL, False, format,
		      &type_ret, &format_ret, &num_ret, &bytes_after, &retval);
   if (retval)
     {
	if (format_ret == 32)
	  {
	     int                 i;

	     *size = num_ret * sizeof(unsigned int);

	     data = NEW(unsigned int, num_ret);

	     for (i = 0; i < (int)num_ret; i++)
		((unsigned int *)data)[i] = ((unsigned long *)retval)[i];
	  }
	else if (format_ret == 16)
	  {
	     int                 i;

	     *size = num_ret * sizeof(unsigned short);

	     data = NEW(unsigned short, *size);

	     for (i = 0; i < (int)num_ret; i++)
		((unsigned short *)data)[i] = ((unsigned short *)retval)[i];
	  }
	else if (format_ret == 8)
	  {
	     /* format_ret == 8 */
	     *size = num_ret;
	     data = NEW(char, num_ret);

	     if (data)
		memcpy(data, retval, num_ret);
	  }
	XFree(retval);
	return data;
     }
   *size = 0;
   return NULL;
}

void
e_window_dnd_advertise(Window win)
{
   static Atom         atom_xdndaware = 0;
   int                 dnd_version = 3;

   E_ATOM(atom_xdndaware, "XdndAware");
   e_window_property_set(win, atom_xdndaware, XA_ATOM, 32, &dnd_version, 1);
}

void
e_grab(void)
{
   x_grabs++;
   if (x_grabs == 1)
      XGrabServer(disp);
}

void
e_ungrab(void)
{
   x_grabs--;
   if (x_grabs == 0)
     {
	XUngrabServer(disp);
	e_sync();
     }
}

void
e_window_ignore(Window win)
{
   Window_List        *w;

   if (win == 0)
      win = default_root;
   w = NEW(Window_List, 1);
   w->win = win;
   w->next = ignore_wins;
   ignore_wins = w;
}

void
e_window_no_ignore(Window win)
{
   Window_List        *w, *pw;

   if (win == 0)
      win = default_root;
   for (pw = NULL, w = ignore_wins; w; pw = w, w = w->next)
     {
	if (w->win == win)
	  {
	     if (pw)
		pw->next = w->next;
	     else
		ignore_wins = w->next;
	     FREE(w);
	     return;
	  }
     }
}

int
e_window_is_ignored(Window win)
{
   Window_List        *w;

   if (win == 0)
      win = default_root;
   for (w = ignore_wins; w; w = w->next)
     {
	if (w->win == win)
	   return 1;
     }
   return 0;
}

static              Window
e_window_at_xy_0(Window base, int bx, int by, int x, int y)
{
   Window             *list = NULL;
   XWindowAttributes   att;
   Window              child = 0, parent_win = 0, root_win = 0;
   int                 i;
   unsigned int        ww, wh, num;
   int                 wx, wy;

   if ((!XGetWindowAttributes(disp, base, &att)) ||
       (att.map_state != IsViewable))
      return 0;

   wx = att.x;
   wy = att.y;
   ww = att.width;
   wh = att.height;

   wx += bx;
   wy += by;

   if (!((x >= wx) &&
	 (y >= wy) && (x < (int)(wx + ww)) && (y < (int)(wy + wh))))
      return 0;

   if (!XQueryTree(disp, base, &root_win, &parent_win, &list, &num))
      return base;
   if (list)
     {
	for (i = num - 1;; i--)
	  {
	     if (!e_window_is_ignored(list[i]))
	       {
		  if ((child = e_window_at_xy_0(list[i], wx, wy, x, y)) != 0)
		    {
		       XFree(list);
		       return child;
		    }
	       }
	     if (!i)
		break;
	  }
	XFree(list);
     }
   return base;
}

Window
e_window_get_at_xy(int x, int y)
{
   Window              child;

   e_grab();
   child = e_window_at_xy_0(default_root, 0, 0, x, y);
   if (child)
     {
	e_ungrab();
	return child;
     }
   e_ungrab();
   return default_root;
}

int
e_window_dnd_capable(Window win)
{
   static Atom         atom_xdndaware = 0;
   int                 dnd_version = 3;
   int                *atom_ret;
   int                 size = 0;

   E_ATOM(atom_xdndaware, "XdndAware");
   atom_ret = e_window_property_get(win, atom_xdndaware, XA_ATOM, &size);
   if ((atom_ret) && (size >= (int)sizeof(int)))
     {
	if (atom_ret[0] == dnd_version)
	  {
	     FREE(atom_ret);
	     return 1;
	  }
	FREE(atom_ret);
     }
   return 0;
}

void
e_window_dnd_handle_motion(Window source_win, int x, int y, int dragging)
{
   static Atom         atom_xdndenter = 0;
   static Atom         atom_xdndleave = 0;
   static Atom         atom_xdnddrop = 0;
   static Atom         atom_xdndposition = 0;
   static Atom         atom_xdndactioncopy = 0;
   static Atom         atom_xdndactionmove = 0;
   static Atom         atom_xdndactionlink = 0;
   static Atom         atom_xdndactionask = 0;
   static Atom         atom_text_uri_list = 0;
   static Atom         atom_text_plain = 0;
   Window              win;
   XEvent              xevent;

   win = e_window_get_at_xy(x, y);
   while ((win) && (!e_window_dnd_capable(win)))
      win = e_window_get_parent(win);
   E_ATOM(atom_xdndenter, "XdndEnter");
   E_ATOM(atom_xdndleave, "XdndLeave");
   E_ATOM(atom_xdnddrop, "XdndDrop");
   E_ATOM(atom_xdndposition, "XdndPosition");
   E_ATOM(atom_xdndactioncopy, "XdndActionCopy");
   E_ATOM(atom_xdndactionmove, "XdndActionMove");
   E_ATOM(atom_xdndactionlink, "XdndActionLink");
   E_ATOM(atom_xdndactionask, "XdndActionAsk");
   E_ATOM(atom_text_uri_list, "text/uri-list");
   E_ATOM(atom_text_plain, "text/plain");
   if ((win != current_dnd_win) && (current_dnd_win))
     {
	/* send leave to old dnd win */
	xevent.xany.type = ClientMessage;
	xevent.xany.display = disp;
	xevent.xclient.format = 32;
	xevent.xclient.window = current_dnd_win;
	xevent.xclient.message_type = atom_xdndleave;
	xevent.xclient.data.l[0] = source_win;
	xevent.xclient.data.l[1] = 0;
	xevent.xclient.data.l[2] = 0;
	xevent.xclient.data.l[3] = 0;
	xevent.xclient.data.l[4] = 0;
	XSendEvent(disp, current_dnd_win, False, 0, &xevent);
     }
   if (win)
     {
	if (win != current_dnd_win)
	  {
	     /* send enter on new  win */
	     xevent.xany.type = ClientMessage;
	     xevent.xany.display = disp;
	     xevent.xclient.window = win;
	     xevent.xclient.message_type = atom_xdndenter;
	     xevent.xclient.format = 32;
	     xevent.xclient.data.l[0] = source_win;
	     xevent.xclient.data.l[1] = (3 << 24);
	     xevent.xclient.data.l[2] = atom_text_uri_list;
	     xevent.xclient.data.l[3] = atom_text_plain;
	     xevent.xclient.data.l[4] = 0;
	     XSendEvent(disp, win, False, 0, &xevent);
	  }
	/* send position information */
	xevent.xany.type = ClientMessage;
	xevent.xany.display = disp;
	xevent.xclient.window = win;
	xevent.xclient.message_type = atom_xdndposition;
	xevent.xclient.format = 32;
	xevent.xclient.data.l[0] = source_win;
	xevent.xclient.data.l[1] = (3 << 24);
	xevent.xclient.data.l[2] = ((x << 16) & 0xffff0000) | (y & 0xffff);
	xevent.xclient.data.l[3] = CurrentTime;
	if (dnd_copy)
	   xevent.xclient.data.l[4] = atom_xdndactioncopy;
	else if (dnd_link)
	   xevent.xclient.data.l[4] = atom_xdndactionlink;
	else if (dnd_move)
	   xevent.xclient.data.l[4] = atom_xdndactionmove;
	else
	   xevent.xclient.data.l[4] = atom_xdndactionask;
	XSendEvent(disp, win, False, 0, &xevent);
     }
   if (!dragging)
     {
	if (win)
	  {
	     if (current_dnd_target_ok)
	       {
		  xevent.xany.type = ClientMessage;
		  xevent.xany.display = disp;
		  xevent.xclient.window = win;
		  xevent.xclient.message_type = atom_xdnddrop;
		  xevent.xclient.format = 32;
		  xevent.xclient.data.l[0] = source_win;
		  xevent.xclient.data.l[1] = 0;
		  xevent.xclient.data.l[2] = CurrentTime;
		  xevent.xclient.data.l[3] = 0;
		  xevent.xclient.data.l[4] = 0;
		  XSendEvent(disp, win, False, 0, &xevent);
	       }
	     else
	       {
		  xevent.xany.type = ClientMessage;
		  xevent.xany.display = disp;
		  xevent.xclient.window = win;
		  xevent.xclient.message_type = atom_xdndleave;
		  xevent.xclient.format = 32;
		  xevent.xclient.data.l[0] = source_win;
		  xevent.xclient.data.l[1] = 0;
		  xevent.xclient.data.l[2] = 0;
		  xevent.xclient.data.l[3] = 0;
		  xevent.xclient.data.l[4] = 0;
		  XSendEvent(disp, win, False, 0, &xevent);
	       }
	  }
	current_dnd_target_ok = 0;
     }
   current_dnd_win = win;
}

int
e_dnd_selection_convert(Window win, Window req, Atom type)
{
   static Atom         atom_xdndselection = 0;
   static Atom         atom_jxselectionwindowproperty = 0;

   E_ATOM(atom_xdndselection, "XdndSelection");
   E_ATOM(atom_jxselectionwindowproperty, "JXSelectionWindowProperty");
   if (win == XGetSelectionOwner(disp, atom_xdndselection))
     {
	XConvertSelection(disp, atom_xdndselection, type,
			  atom_jxselectionwindowproperty, req, CurrentTime);
	return 1;
     }
   return 0;
}

void               *
e_dnd_selection_get(Window win, Window req, Atom type, int *size)
{
   unsigned char      *data = NULL;
   long                bytes_read;
   unsigned long       remaining = 1;

   *size = 0;
   bytes_read = 0;
   while (remaining)
     {
	unsigned char      *s;
	Atom                actual;
	int                 format;
	unsigned long       count;

	s = NULL;
	if (XGetWindowProperty(disp, win, type, bytes_read / 4, 0x10000, 1,
			       AnyPropertyType, &actual, &format, &count,
			       &remaining, &s) != Success)
	  {
	     /* error occured */
	     XFree(s);
	     IF_FREE(data);
	     *size = 0;
	     return NULL;
	  }
	if (s)
	  {
	     /* got some mroe data - append it */
	     bytes_read += count;
	     if (!data)
		data = NEW(char, bytes_read);

	     else
		REALLOC(data, char, bytes_read);
	     MEMCPY(s, data + (bytes_read - count), char, count);

	     XFree(s);
	  }
     }
   *size = bytes_read;
   return data;
   req = 0;
}

void
e_dnd_set_data(Window win)
{
   static int         atom_xdndactioncopy = 0;
   static int         atom_xdndactionmove = 0;
   static int         atom_xdndactionlink = 0;
   static int         atom_xdndactionask = 0;
   static Atom        atom_xdndactionlist = 0;
   static Atom         atom_xdndselection = 0;
   
   E_ATOM(atom_xdndactioncopy, "XdndActionCopy");
   E_ATOM(atom_xdndactionmove, "XdndActionMove");
   E_ATOM(atom_xdndactionlink, "XdndActionLink");
   E_ATOM(atom_xdndactionask, "XdndActionAsk");
   E_ATOM(atom_xdndactionlist, "XdndActionList");
   E_ATOM(atom_xdndselection, "XdndSelection");
   if (dnd_copy)
      e_window_property_set(win, atom_xdndactionlist, XA_ATOM, 32,
			    &atom_xdndactioncopy, 1);
   else if (dnd_link)
      e_window_property_set(win, atom_xdndactionlist, XA_ATOM, 32,
			    &atom_xdndactionlink, 1);
   else if (dnd_move)
      e_window_property_set(win, atom_xdndactionlist, XA_ATOM, 32,
			    &atom_xdndactionmove, 1);
   else
      e_window_property_set(win, atom_xdndactionlist, XA_ATOM, 32,
			    &atom_xdndactionask, 1);
   XSetSelectionOwner(disp, atom_xdndselection, win, CurrentTime);
}

void
e_dnd_send_data(Window win, Window source_win, void *data, int size,
		Atom dest_atom, int plain_text)
{
   XEvent              xevent;
   static Atom         atom_text_plain = 0;
   static Atom         atom_text_uri_list = 0;
   static Atom         atom_xdndselection = 0;
   Atom                target;

   E_ATOM(atom_xdndselection, "XdndSelection");
   E_ATOM(atom_text_uri_list, "text/uri-list");
   E_ATOM(atom_text_plain, "text/plain");
   target = atom_text_uri_list;
   if (plain_text)
      target = atom_text_plain;
   e_window_property_set(win, dest_atom, target, 8, data, size);
   xevent.xselection.type = SelectionNotify;
   xevent.xselection.property = dest_atom;
   xevent.xselection.display = disp;
   xevent.xselection.requestor = win;
   xevent.xselection.selection = atom_xdndselection;
   xevent.xselection.target = target;
   xevent.xselection.time = CurrentTime;
   XSendEvent(disp, win, False, 0, &xevent);
   return;
   source_win = 0;
}

void
e_dnd_set_mode_copy(void)
{
   dnd_copy = 1;
   dnd_link = 0;
   dnd_move = 0;
}

void
e_dnd_set_mode_link(void)
{
   dnd_copy = 0;
   dnd_link = 1;
   dnd_move = 0;
}

void
e_dnd_set_mode_move(void)
{
   dnd_copy = 0;
   dnd_link = 0;
   dnd_move = 1;
}

void
e_dnd_set_mode_ask(void)
{
   dnd_copy = 0;
   dnd_link = 0;
   dnd_move = 0;
}

void
e_dnd_own_selection(Window win)
{
   static Atom         atom_xdndselection = 0;
   static Atom         atom_jxselectionwindowproperty = 0;

   E_ATOM(atom_xdndselection, "XdndSelection");
   E_ATOM(atom_jxselectionwindowproperty, "JXSelectionWindowProperty");

   if (!XSetSelectionOwner(disp, atom_xdndselection, win, CurrentTime))
      return;
}

void
e_dnd_send_drop(Window win, Window source_win)
{
   static Atom         atom_xdnddrop = 0;
   XEvent              xevent;

   E_ATOM(atom_xdnddrop, "XdndDrop");

   e_dnd_own_selection(source_win);

   memset(&xevent, 0, sizeof(xevent));
   xevent.xany.type = ClientMessage;
   xevent.xany.display = disp;
   xevent.xclient.window = win;
   xevent.xclient.message_type = atom_xdnddrop;
   xevent.xclient.format = 32;

   xevent.xclient.data.l[0] = source_win;
   xevent.xclient.data.l[1] = CurrentTime;
   xevent.xclient.data.l[2] = 0;
   xevent.xclient.data.l[3] = 0;
   xevent.xclient.data.l[4] = 0;
   XSendEvent(disp, win, False, 0, &xevent);
}

void
e_window_dnd_send_status_ok(Window source_win, Window win, int x, int y, int w,
			    int h)
{
   static Atom         atom_xdndstatus = 0;
   static Atom         atom_xdndactioncopy = 0;
   static Atom         atom_xdndactionmove = 0;
   static Atom         atom_xdndactionlink = 0;
   static Atom         atom_xdndactionask = 0;
   XEvent              xevent;

   E_ATOM(atom_xdndstatus, "XdndStatus");
   E_ATOM(atom_xdndactioncopy, "XdndActionCopy");
   E_ATOM(atom_xdndactionmove, "XdndActionMove");
   E_ATOM(atom_xdndactionlink, "XdndActionLink");
   E_ATOM(atom_xdndactionask, "XdndActionAsk");
   memset(&xevent, 0, sizeof(xevent));

   xevent.xany.type = ClientMessage;
   xevent.xany.display = disp;
   xevent.xclient.window = win;
   xevent.xclient.message_type = atom_xdndstatus;
   xevent.xclient.format = 32;

   xevent.xclient.data.l[0] = source_win;
   xevent.xclient.data.l[1] = 3;
   xevent.xclient.data.l[2] = ((x << 16) & 0xffff0000) | (y & 0xffff);
   xevent.xclient.data.l[3] = ((h << 16) & 0xffff0000) | (w & 0xffff);
   if (dnd_copy)
      xevent.xclient.data.l[4] = atom_xdndactioncopy;
   else if (dnd_link)
      xevent.xclient.data.l[4] = atom_xdndactionlink;
   else if (dnd_move)
      xevent.xclient.data.l[4] = atom_xdndactionmove;
   else
      xevent.xclient.data.l[4] = atom_xdndactionask;
   XSendEvent(disp, win, False, 0, &xevent);
}

void
e_window_dnd_send_finished(Window source_win, Window win)
{
   static Atom         atom_xdndfinished = 0;
   XEvent              xevent;

   E_ATOM(atom_xdndfinished, "XdndFinished");
   memset(&xevent, 0, sizeof(xevent));

   xevent.xany.type = ClientMessage;
   xevent.xany.display = disp;
   xevent.xclient.window = win;
   xevent.xclient.message_type = atom_xdndfinished;
   xevent.xclient.format = 32;

   xevent.xclient.data.l[0] = source_win;
   xevent.xclient.data.l[1] = 0;
   xevent.xclient.data.l[2] = 0;
   xevent.xclient.data.l[3] = 0;
   xevent.xclient.data.l[4] = 0;
   XSendEvent(disp, win, False, 0, &xevent);
}

void
e_window_dnd_ok(int ok)
{
   current_dnd_target_ok = ok;
}

void
e_window_dnd_finished(void)
{
   current_dnd_win = 0;
}

void
e_window_set_title(Window win, char *title)
{
   XStoreName(disp, win, title);
}

void
e_window_set_name_class(Window win, char *name, char *class)
{
   XClassHint          hint;

   hint.res_name = name;
   hint.res_class = class;
   XSetClassHint(disp, win, &hint);
}

void
e_window_get_name_class(Window win, char **name, char **class)
{
   XClassHint xch;
   
   if (name) *name = NULL;
   if (class) *class = NULL;
   if (XGetClassHint(disp, win, &xch))
     {
	if (name)
	  {
	     if (xch.res_name) *name = strdup(xch.res_name);
	  }
	if (class)
	  {
	     if (xch.res_class) *class = strdup(xch.res_class);
	  }
	XFree(xch.res_name);
	XFree(xch.res_class);
     }
}

void
e_window_get_hints(Window win, int *accepts_focus, int *initial_state,
		   Pixmap *icon_pixmap, Pixmap *icon_mask,
		   Window *icon_window, Window *window_group)
{
   XWMHints *hints;
   
   hints = XGetWMHints(disp, win);
   if (hints)
     {
	if ((hints->flags & InputHint) && (accepts_focus))
	  {
	     if (hints->input) *accepts_focus = 1;
	     else *accepts_focus = 0;
	  }
	if ((hints->flags & StateHint) && (initial_state))
	  {
	     *initial_state = hints->initial_state;
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
     }
}

char *
e_window_get_machine(Window win)
{
   XTextProperty       xtp;
   
   if (XGetWMClientMachine(disp, win, &xtp))
     {
	char *s;
	
	if (!xtp.value) return NULL;	
	s = strdup(xtp.value);
	XFree(xtp.value);
	return s;
     }
   return NULL;
}

char *
e_window_get_command(Window win)
{
   int                 cargc;
   char              **cargv;
   
   if (XGetCommand(disp, win, &cargv, &cargc))
     {
	if (cargc > 0)
	  {
	     char *s;
	     int size, i;
	     
	     s = NULL;
	     size = strlen(cargv[0]);
	     REALLOC(s, char, size + 1);
	     strcpy(s, cargv[0]);
	     for (i = 1; i < cargc; i++)
	       {
		  size += strlen(cargv[i]);
		  REALLOC(s, char, size + 2);
		  strcat(s, " ");
		  strcat(s, cargv[i]);
	       }
	     XFreeStringList(cargv);
	     return s;
	  }
	else
	  return NULL;
     }
   return NULL;
}

char *
e_window_get_icon_name(Window win)
{
   XTextProperty       xtp;
   
   if (XGetWMIconName(disp, win, &xtp))
     {
	char *s;
	
	if (!xtp.value) return NULL;	
	s = strdup(xtp.value);
	XFree(xtp.value);
	return s;
     }
   return NULL;
}

void
e_window_set_min_size(Window win, int w, int h)
{
   XSizeHints          hints;
   long                ret;

   memset(&hints, 0, sizeof(XSizeHints));
   XGetWMNormalHints(disp, win, &hints, &ret);
   hints.flags |= PMinSize | PSize | USSize;
   hints.min_width = w;
   hints.min_height = h;
   XSetWMNormalHints(disp, win, &hints);
}

void
e_window_set_max_size(Window win, int w, int h)
{
   XSizeHints          hints;
   long                ret;

   memset(&hints, 0, sizeof(XSizeHints));
   XGetWMNormalHints(disp, win, &hints, &ret);
   hints.flags |= PMaxSize | PSize | USSize;
   hints.max_width = w;
   hints.max_height = h;
   XSetWMNormalHints(disp, win, &hints);
}

void
e_window_set_xy_hints(Window win, int x, int y)
{
   XSizeHints          hints;
   long                ret;

   memset(&hints, 0, sizeof(XSizeHints));
   XGetWMNormalHints(disp, win, &hints, &ret);
   hints.flags |= PPosition | USPosition | PSize | USSize;
   hints.x = x;
   hints.y = y;
   XSetWMNormalHints(disp, win, &hints);
}

void
e_window_get_frame_size(Window win, int *l, int *r, int *t, int *b)
{
   static Atom         atom_e_frame_size = 0;
   int                *data, size;

   E_ATOM(atom_e_frame_size, "_E_FRAME_SIZE");
   data = e_window_property_get(win, atom_e_frame_size, XA_CARDINAL, &size);
   if (data)
     {
	if (size == (4 * sizeof(int)))
	  {
	     if (l)
		*l = data[0];
	     if (r)
		*r = data[1];
	     if (t)
		*t = data[2];
	     if (b)
		*b = data[3];
	  }
	else
	  {
	     if (l)
		*l = 0;
	     if (r)
		*r = 0;
	     if (t)
		*t = 0;
	     if (b)
		*b = 0;
	  }
	FREE(data);
     }
   else
     {
	if (l)
	   *l = 0;
	if (r)
	   *r = 0;
	if (t)
	   *t = 0;
	if (b)
	   *b = 0;
     }
}

int
e_window_save_under(Window win)
{
   XSetWindowAttributes att;
   XWindowAttributes   gatt;

   att.save_under = True;
   XChangeWindowAttributes(disp, win, CWSaveUnder, &att);
   XGetWindowAttributes(disp, win, &gatt);
   if (gatt.save_under == True)
      return 1;
   return 0;
}

GC
e_gc_new(Drawable d)
{
   XGCValues           gcv;

   if (d == 0)
      d = default_root;
   return XCreateGC(disp, d, 0, &gcv);
}

void
e_gc_free(GC gc)
{
   XFreeGC(disp, gc);
}

void
e_gc_set_fg(GC gc, int val)
{
   XSetForeground(disp, gc, val);
}

void
e_fill_rectangle(Drawable d, GC gc, int x, int y, int w, int h)
{
   XFillRectangle(disp, d, gc, x, y, w, h);
}

void
e_draw_rectangle(Drawable d, GC gc, int x, int y, int w, int h)
{
   XDrawRectangle(disp, d, gc, x, y, w - 1, h - 1);
}

void
e_draw_line(Drawable d, GC gc, int x1, int y1, int x2, int y2)
{
   XDrawLine(disp, d, gc, x1, y1, x2, y2);
}

void
e_draw_point(Drawable d, GC gc, int x, int y)
{
   XDrawPoint(disp, d, gc, x, y);
}

void
e_window_hint_set_layer(Window win, int layer)
{
   static Atom         atom_win_layer = 0;

   E_ATOM(atom_win_layer, "_WIN_LAYER");
   e_window_property_set(win, atom_win_layer, XA_CARDINAL, 32, &layer, 1);
}

void
e_window_hint_set_sticky(Window win, int sticky)
{
   static Atom         atom_win_state = 0;
   static Atom         atom_win_hints = 0;
   int                 data;

   E_ATOM(atom_win_state, "_WIN_STATE");
   E_ATOM(atom_win_hints, "_WIN_HINTS");
   if (sticky)
     {
	data = ((1 << 0) | (1 << 8) | (1 << 9));
	e_window_property_set(win, atom_win_state, XA_CARDINAL, 32, &data, 1);
	data = ((1 << 0) | (1 << 1) | (1 << 2));
	e_window_property_set(win, atom_win_hints, XA_CARDINAL, 32, &data, 1);
     }
   else
     {
	data = 0;
	e_window_property_set(win, atom_win_state, XA_CARDINAL, 32, &data, 1);
	e_window_property_set(win, atom_win_hints, XA_CARDINAL, 32, &data, 1);
     }
}

void
e_window_hint_set_borderless(Window win)
{
   static Atom         atom_motif_wm_hints = 0;
   int                 data[5];

   E_ATOM(atom_motif_wm_hints, "_MOTIF_WM_HINTS");
   data[0] = 0x3;
   data[1] = 0x0;
   data[2] = 0x0;
   data[3] = 0x2ada27b0;
   data[4] = 0x2aabd6b0;
   e_window_property_set(win, atom_motif_wm_hints, atom_motif_wm_hints, 32,
			 data, 5);
}

void
e_grab_mouse(Window win, int confine, Cursor cursor)
{
   int                 ret;

   if (confine)
      ret = XGrabPointer(disp, win, False,
			 XEV_BUTTON | XEV_MOUSE_MOVE | XEV_IN_OUT,
			 GrabModeAsync, GrabModeAsync,
			 win, cursor, CurrentTime);
   else
      ret = XGrabPointer(disp, win, False,
			 XEV_BUTTON | XEV_MOUSE_MOVE | XEV_IN_OUT,
			 GrabModeAsync, GrabModeAsync,
			 None, cursor, CurrentTime);
   if (ret == GrabSuccess)
      grab_pointer_win = win;
}

void
e_ungrab_mouse(void)
{
   XUngrabPointer(disp, CurrentTime);
   grab_pointer_win = 0;
}

Window
e_grab_window_get(void)
{
   return grab_pointer_win;
}

int
e_window_get_gravity(Window win)
{
   XWindowAttributes att;
   
   XGetWindowAttributes(disp, win, &att);
   return att.win_gravity;
}

void
e_window_gravity_reset(Window win)
{
   E_XID              *xid = NULL;

   xid = e_validate_xid(win);
   if (xid)
     {
	XSetWindowAttributes att;
	
/*	if (xid->gravity != NorthWestGravity)*/
	  {
	     att.win_gravity = NorthWestGravity;
	     XChangeWindowAttributes(disp, win, CWWinGravity, &att);
	     xid->gravity = NorthWestGravity;
	     xid->coords_invalid = 1;
	  }
     }
}

void
e_window_gravity_set(Window win, int gravity)
{
   E_XID              *xid = NULL;

   xid = e_validate_xid(win);
   if (xid)
     {
/*	if (xid->gravity != gravity)*/
	  {
	     XSetWindowAttributes att;
	     
	     att.win_gravity = gravity;
	     XChangeWindowAttributes(disp, win, CWWinGravity, &att);
	     xid->gravity = gravity;
	     xid->coords_invalid = 1;
	  }
     }
}

void
e_window_bit_gravity_set(Window win, int gravity)
{
   E_XID              *xid = NULL;

   xid = e_validate_xid(win);
   if (xid)
     {
	XSetWindowAttributes att;
	
	att.bit_gravity = gravity;
	XChangeWindowAttributes(disp, win, CWBitGravity, &att);
     }
}

void
e_pointer_warp_by(int dx, int dy)
{
   XWarpPointer(disp, None, None, 0, 0, 0, 0, dx, dy);
}

void
e_pointer_warp_to(int x, int y)
{
   XWarpPointer(disp, None, default_root, 0, 0, 0, 0, x, y);
}

void
e_gc_set_include_inferiors(GC gc)
{
   XGCValues           gcv;

   gcv.subwindow_mode = IncludeInferiors;
   XChangeGC(disp, gc, GCSubwindowMode, &gcv);
}

void
e_area_copy(Drawable src, Drawable dest, GC gc,
	    int sx, int sy, int sw, int sh, int dx, int dy)
{
   if (src == 0)
      src = default_root;
   if (dest == 0)
      dest = default_root;
   XCopyArea(disp, src, dest, gc, sx, sy, sw, sh, dx, dy);
}

Window e_window_root(void)
{
   return default_root;
}

void
e_window_get_virtual_area(Window win, int *area_x, int *area_y)
{
   static Atom         atom_win_area = 0;
   int                *data, size;

   E_ATOM(atom_win_area, "_WIN_AREA");
   data = e_window_property_get(win, atom_win_area, XA_CARDINAL, &size);
   if (data)
     {
	if (size == (sizeof(int) * 2))
	  {
	     if (area_x)
		*area_x = data[0];
	     if (area_y)
		*area_y = data[1];
	  }
	FREE(data);
     }
}

void
e_get_virtual_area(int *area_x, int *area_y)
{
   static Atom         atom_win_area = 0;
   int                *data, size;

   E_ATOM(atom_win_area, "_WIN_AREA");
   data =
      e_window_property_get(default_root, atom_win_area, XA_CARDINAL, &size);
   if (data)
     {
	if (size == (sizeof(int) * 2))
	  {
	     if (area_x)
		*area_x = data[0];
	     if (area_y)
		*area_y = data[1];
	  }
	FREE(data);
     }
}

void
e_window_get_root_relative_location(Window win, int *x, int *y)
{
   int dx, dy;
   Window parent;
   E_XID              *xid = NULL;
   
   if (win == 0)
      win = default_root;
   if (win == default_root)
     {
	if (x) *x = 0;
	if (y) *y = 0;
	return;
     }   
   xid = e_validate_xid(win);
   if (!xid)
     {
	if (x) *x = 0;
	if (y) *y = 0;
	return;
     }
   dx = 0;
   dy = 0;
   do
     {
	parent = xid->parent;
	dx += xid->x;
	dy += xid->y;
	if (parent != default_root)
	  {
	     xid = e_validate_xid(parent);
	     if (!xid)
	       {
		  if (x) *x = dx;
		  if (y) *y = dy;
		  return;
	       }
	  }
     }
   while (parent != default_root);
   if (x) *x = dx;
   if (y) *y = dy;
}

void
e_button_grab(Window win, int button, int events, Ev_Key_Modifiers mod, int any_mod)
{
   unsigned int b;
   unsigned int m;
   unsigned int locks[8];
   int i;
   
   b = button;
   if (b == 0) b = AnyButton;
   m = 0;
   if (any_mod) m = AnyModifier;
   else
     {
	if (mod & EV_KEY_MODIFIER_SHIFT) m |= e_mod_mask_shift_get();
	if (mod & EV_KEY_MODIFIER_CTRL) m |= e_mod_mask_ctrl_get();
	if (mod & EV_KEY_MODIFIER_ALT) m |= e_mod_mask_alt_get();
	if (mod & EV_KEY_MODIFIER_WIN) m |= e_mod_mask_win_get();
     }
   locks[0] = 0;
   locks[1] = e_lock_mask_caps_get();
   locks[2] =                          e_lock_mask_num_get();
   locks[3] =                                                  e_lock_mask_scroll_get();
   locks[4] = e_lock_mask_caps_get() | e_lock_mask_num_get();
   locks[5] = e_lock_mask_caps_get() |                         e_lock_mask_scroll_get();
   locks[6] =                          e_lock_mask_num_get() | e_lock_mask_scroll_get();
   locks[7] = e_lock_mask_caps_get() | e_lock_mask_num_get() | e_lock_mask_scroll_get();
   for (i = 0; i < 8; i++)
      XGrabButton(disp, b, m | locks[i], 
		  win, False, events, 
		  GrabModeSync, GrabModeAsync, None, None);
}

void
e_button_ungrab(Window win, int button, Ev_Key_Modifiers mod, int any_mod)
{
   unsigned int b;
   unsigned int m;
   unsigned int locks[8];
   int i;
   
   b = button;
   if (b == 0) b = AnyButton;
   m = 0;
   if (any_mod) m = AnyModifier;
   else
     {
	if (mod & EV_KEY_MODIFIER_SHIFT) m |= e_mod_mask_shift_get();
	if (mod & EV_KEY_MODIFIER_CTRL) m |= e_mod_mask_ctrl_get();
	if (mod & EV_KEY_MODIFIER_ALT) m |= e_mod_mask_alt_get();
	if (mod & EV_KEY_MODIFIER_WIN) m |= e_mod_mask_win_get();
     }
   locks[0] = 0;
   locks[1] = e_lock_mask_caps_get();
   locks[2] =                          e_lock_mask_num_get();
   locks[3] =                                                  e_lock_mask_scroll_get();
   locks[4] = e_lock_mask_caps_get() | e_lock_mask_num_get();
   locks[5] = e_lock_mask_caps_get() |                         e_lock_mask_scroll_get();
   locks[6] =                          e_lock_mask_num_get() | e_lock_mask_scroll_get();
   locks[7] = e_lock_mask_caps_get() | e_lock_mask_num_get() | e_lock_mask_scroll_get();
   for (i = 0; i < 8; i++)
      XUngrabButton(disp, b, m | locks[i], win);
}

void
e_pointer_replay(Time t)
{
   XSync(disp, False);
   XAllowEvents(disp, ReplayPointer, t);
   XSync(disp, False);   
}

void
e_pointer_grab(Window win, Time t)
{
   XGrabPointer(disp, win, False, XEV_BUTTON | XEV_MOUSE_MOVE |  XEV_IN_OUT,
		GrabModeAsync, GrabModeAsync, None, None, t);
}

void
e_pointer_ungrab(Time t)
{
   XUngrabPointer(disp, t);
}

void
e_window_send_event_move_resize(Window win, int x, int y, int w, int h)
{
   XEvent              ev;

   ev.type = ConfigureNotify;
   ev.xconfigure.display = disp;
   ev.xconfigure.event = win;
   ev.xconfigure.window = win;
   ev.xconfigure.x = x;
   ev.xconfigure.y = y;
   ev.xconfigure.width = w;
   ev.xconfigure.height = h;
   ev.xconfigure.border_width = 0;
   ev.xconfigure.above = win;
   ev.xconfigure.override_redirect = False;
   XSendEvent(disp, win, False, StructureNotifyMask, &ev);
}

void
e_window_send_client_message(Window win, Atom type, int format, void *data)
{
   XClientMessageEvent ev;
   int i;
   
   ev.type = ClientMessage;
   ev.window = win;
   ev.message_type = type;
   ev.format = format;
   if (format == 32)
     {
	for (i = 0; i < 5; i++)
	  ev.data.l[i] = ((unsigned int *)data)[i];
     }
   else if (format == 16)
     {
	for (i = 0; i < 10; i++)
	  ev.data.s[i] = ((unsigned short *)data)[i];
     }
   else if (format == 8)
     {
	for (i = 0; i < 20; i++)
	  ev.data.b[i] = ((unsigned char *)data)[i];
     }
   XSendEvent(disp, win, False, 0, (XEvent *) & ev);
}

void
e_window_add_to_save_set(Window win)
{
   XAddToSaveSet(disp, win);
}

void
e_window_del_from_save_set(Window win)
{
   XRemoveFromSaveSet(disp, win);
}

void
e_window_kill_client(Window win)
{
   XKillClient(disp, (XID)win);   
}

void
e_window_set_border_width(Window win, int bw)
{
   E_XID              *xid = NULL;

   xid = e_validate_xid(win);
   if (xid)
     {
	xid->bw = bw;
     }
   XSetWindowBorderWidth(disp, win, bw);
}

int
e_window_get_border_width(Window win)
{
   E_XID              *xid = NULL;

   xid = e_validate_xid(win);
   if (xid)
     {
	return xid->bw;
     }
   return 0;
}

int
e_window_get_wm_size_hints(Window win, XSizeHints *hints, int *mask)
{
   long sup_ret;
   Status ok;
   
   ok = XGetWMNormalHints(disp, win, hints, &sup_ret);
   *mask = (int)sup_ret;
   return ok;
}

int
e_window_is_visible(Window win)
{
   XWindowAttributes   att;

   if (win == 0)
      win = default_root;
   if (XGetWindowAttributes(disp, win, &att) == True)
     {
	if (att.map_state == IsUnmapped) return 0;
	return 1;
     }
   return 0;
}

int
e_window_is_normal(Window win)
{
   XWindowAttributes   att;
   
   if (win == 0)
      win = default_root;
   if (XGetWindowAttributes(disp, win, &att) == True)
     {
	if ((att.override_redirect) || (att.class ==  InputOnly)) return 0;
	return 1;
     }
   return 0;
}

int
e_window_is_manageable(Window win)
{
   XWindowAttributes   att;

   if (win == 0)
      win = default_root;
   if (XGetWindowAttributes(disp, win, &att) == True)
     {
	if ((att.map_state == IsUnmapped) || (att.override_redirect) || (att.class ==  InputOnly)) return 0;
	return 1;
     }
   return 0;
}

void
e_windows_restack(Window *wins, int num)
{
   XRestackWindows(disp, wins, num);
}

void
e_window_stack_above(Window win, Window above)
{
   XWindowChanges xwc;
   
   if (win == 0)
     win = default_root;
   xwc.sibling = above;
   xwc.stack_mode = Above;
   XConfigureWindow(disp, win, CWSibling | CWStackMode, &xwc);
}

void
e_window_stack_below(Window win, Window below)
{
   XWindowChanges xwc;
   
   if (win == 0)
     win = default_root;
   xwc.sibling = below;
   xwc.stack_mode = Below;
   XConfigureWindow(disp, win, CWSibling | CWStackMode, &xwc);
}

char *
e_window_get_title(Window win)
{
   XTextProperty       xtp;
   
   if (win == 0)
     win = default_root;
   if (XGetWMName(disp, win, &xtp))
     {
	int                 items;
	char              **list;
	Status              s;
	char               *title = NULL;
	
	if (xtp.format == 8)
	  {
	     s = XmbTextPropertyToTextList(disp, &xtp, &list, &items);
	     if ((s == Success) && (items > 0))
	       {
		  title = strdup(*list);
		  XFreeStringList(list);
	       }
	     else title = strdup((char *)xtp.value);
	  }
	else title = strdup((char *)xtp.value);
	XFree(xtp.value);
	return title;
     }
   return NULL;
}

static Window keyboard_grab_win = 0;

void
e_keyboard_grab(Window win)
{
   int status;
   
   if (keyboard_grab_win) return;
   if (win == 0)
     win = default_root;
   keyboard_grab_win = win;
   status = XGrabKeyboard(disp, win, False, GrabModeAsync, GrabModeAsync, CurrentTime);
   if ((status == AlreadyGrabbed) || (status == GrabNotViewable) ||
       (status == GrabFrozen) || (status == GrabInvalidTime))
       keyboard_grab_win = 0;
}

void
e_keyboard_ungrab(void)
{
   if (!keyboard_grab_win) return;
   keyboard_grab_win = 0;
   XUngrabKeyboard(disp, CurrentTime);
}

Window
e_selection_set(char *string)
{
   Window target = 0;
   static Atom dest = 0;
   Atom selection;
   
   selection = X_CLIPBOARD_SELECTION;
   E_ATOM(dest, "TEXT_SELECTION");
   target = e_window_new(0, 0, 0, 77, 7);
   e_window_add_events(target, XEV_CONFIGURE | XEV_PROPERTY);
   XSetSelectionOwner(disp, selection, target, CurrentTime);
   if (XGetSelectionOwner(disp, XA_PRIMARY) != target) 
     {
	e_window_destroy(target);
	return 0;
     }      
   XChangeProperty(disp, target, dest, 
		   XA_STRING, 8, PropModeReplace, 
		   string, strlen(string));
   return target;
}

Window
e_selection_request(void)
{
   static Atom dest = 0;
   Atom selection;
   Window target = 0;
   
   selection = X_CLIPBOARD_SELECTION;
   E_ATOM(dest, "TEXT_SELECTION");
   target = e_window_new(0, 0, 0, 7, 77);
   e_window_add_events(target, XEV_CONFIGURE | XEV_PROPERTY);
   XConvertSelection(disp, XA_PRIMARY, 
		     XA_STRING, dest,
		     target, 
		     CurrentTime);
   return target;
}

char *
e_selection_get_data(Window win, Atom prop)
{
   char *string = NULL;
   long nread;
   unsigned long bytes_after, nitems;
   unsigned char *data;
   Atom actual_type;
   int actual_fmt;
   
   if (prop == None)
     return NULL;
   for (nread = 0, bytes_after = 1; bytes_after > 0;) 
     {
	if ((XGetWindowProperty(disp, win, prop, nread / 4, 
				0x10000, True, AnyPropertyType, 
				&actual_type, &actual_fmt, &nitems, 
				&bytes_after, &data) != Success)) 
	  {
	     IF_FREE(string);
	     if (data) 
	       {
		  XFree(data);
	       }
	     return NULL;
	  }
	nread += nitems;
	
	if (actual_type == XA_STRING) 
	  {
	     if (string)
	       string = realloc(string, strlen(string) + nitems + 1);
	     else
	       {
		  string = malloc(nitems + 1);
		  string[0] = 0;
	       }
	     string[strlen(string) + nitems] = 0;
	     strncat(string, data, nitems);
	  } 
	else 
	  {
	     int size, i;
	     XTextProperty xtextp;
	     char **cl = NULL;
	     
	     xtextp.value = data;
	     xtextp.encoding = actual_type;
	     xtextp.format = actual_fmt;
	     xtextp.nitems = nitems;
	     XmbTextPropertyToTextList(disp, &xtextp, &cl, &size);
	     
	     if (cl) 
	       {
		  for (i = 0 ; i < size ; i ++) 
		    {
		       if (cl[i]) 
			 {
			    if (string)
			      string = realloc(string, strlen(string) + strlen(cl[i]) + 1);
			    else
			      {
				 string = malloc(strlen(cl[i]) + 1);
				 string[0] = 0;
			      }
			    string[strlen(string) + strlen(cl[i])] = 0;
			    strcat(string, cl[i]);
			 }
		    }
		  XFreeStringList(cl);
	       }
	  }
	if (data) 
	  {
	     XFree(data);
	  }
     }   
   return string;
}

void
e_set_blank_pointer(Window w)
{
   Cursor c;
   XColor cl;
   Pixmap p, m;
   GC gc;
   XGCValues gcv;
   
   if (w == 0)
     w = default_root;
   p = XCreatePixmap(disp, w, 1, 1, 1);
   m = XCreatePixmap(disp, w, 1, 1, 1);
   gc = XCreateGC(disp, m, 0, &gcv);
   XSetForeground(disp, gc, 0);
   XDrawPoint(disp, m, gc, 0, 0);
   XFreeGC(disp, gc);
   c = XCreatePixmapCursor(disp, p, m, &cl, &cl, 0, 0);
   XDefineCursor(disp, w, c);
   XFreeCursor(disp, c);
   XFreePixmap(disp, p);
   XFreePixmap(disp, m);
}

Cursor
e_cursor_new(Pixmap pmap, Pixmap mask, int x, int y, int fr, int fg, int fb, int br, int bg, int bb)
{
   XColor cl1, cl2;
   
   cl1.pixel = 0;
   cl1.red   = fr << 8 | fr;
   cl1.green = fg << 8 | fg;
   cl1.blue  = fb << 8 | fb;
   cl1.flags = DoRed|DoGreen|DoBlue;
   cl2.pixel = 0;
   cl2.red   = br << 8 | br;
   cl2.green = bg << 8 | bg;
   cl2.blue  = bb << 8 | bb;
   cl2.flags = DoRed|DoGreen|DoBlue;
   return XCreatePixmapCursor(disp, pmap, mask, &cl1, &cl2, x, y);   
}

void
e_cursor_free(Cursor c)
{
   XFreeCursor(disp, c);
}

void
e_cursor_set(Window win, Cursor c)
{
   if (win == 0)
     win = default_root;
   XDefineCursor(disp, win, c);
}
