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

XContext            xid_context = 0;

static Display     *disp;
static Visual      *default_vis;
static Colormap     default_cm;
static int          default_depth;
static Window       default_win;
static Window       default_root;

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
static int          dnd_await_target_status = 0;

static int          x_grabs = 0;

static Window_List *ignore_wins = NULL;

static Window       grab_pointer_win = 0;

static Window       keyboard_grab_win = 0;

static int          dnd_copy = 0;
static int          dnd_link = 0;
static int          dnd_move = 1;

static void         ecore_handle_x_error(Display * d, XErrorEvent * ev);
static void         ecore_handle_x_io_error(Display * d);
static Window       ecore_window_at_xy_0(Window base, int bx, int by, int x,
					 int y);

static void
ecore_handle_x_error(Display * d, XErrorEvent * ev)
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
ecore_handle_x_io_error(Display * d)
{
  /* FIXME: call clean exit handler */
  exit(1);
  d = NULL;
}

void
ecore_del_child(Window win, Window child)
{
  Ecore_XID          *xid = NULL;

  if (!disp)
    return;
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
ecore_add_child(Window win, Window child)
{
  Ecore_XID          *xid = NULL;

  if (!disp)
     return;
  if (XFindContext(disp, win, xid_context, (XPointer *) & xid) == XCNOENT)
    return;
  if (xid)
    {
      int                 i;

      for (i = 0; i < xid->children_num; i++)
	{
	  if (xid->children[i] == child)
	    return;
	}
      xid->children_num++;
      REALLOC(xid->children, Window, xid->children_num);
      xid->children[xid->children_num - 1] = child;
    }
  xid = ecore_validate_xid(child);
}

void
ecore_raise_child(Window win, Window child)
{
  Ecore_XID          *xid = NULL;

  if (!disp)
    return;
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
ecore_lower_child(Window win, Window child)
{
  Ecore_XID          *xid = NULL;

  if (!disp)
    return;
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

Ecore_XID          *
ecore_add_xid(Window win, int x, int y, int w, int h, int depth, Window parent)
{
  Ecore_XID          *xid = NULL;

  ecore_window_add_events(win, XEV_IN_OUT | XEV_CONFIGURE | XEV_VISIBILITY);
  xid = NEW(Ecore_XID, 1);
  xid->win = win;
  xid->x = x;
  xid->y = y;
  xid->w = w;
  xid->h = h;
  xid->mapped = 0;
  xid->depth = depth;
  xid->mouse_in = 0;
  xid->parent = parent;
  xid->root = ecore_window_get_root(parent);
  xid->children_num = 0;
  xid->children = NULL;
  xid->gravity = ecore_window_get_gravity(win);
  xid->coords_invalid = 0;
  xid->bw = 0;
  xid->grab_button_auto_replay = NULL;
  XSaveContext(disp, xid->win, xid_context, (XPointer) xid);
  ecore_add_child(parent, win);
  return xid;
}

Ecore_XID          *
ecore_validate_xid(Window win)
{
  Ecore_XID          *xid = NULL;

  if (!disp)
    return NULL;
  if (XFindContext(disp, win, xid_context, (XPointer *) & xid) == XCNOENT)
    {
      XWindowAttributes   att;
      Window              root_ret = 0, parent_ret = 0, *children_ret = NULL;
      unsigned int        children_ret_num = 0;

      ecore_window_add_events(win, XEV_IN_OUT | XEV_CONFIGURE |
			      XEV_VISIBILITY | XEV_CHILD_CHANGE);
      xid = NEW(Ecore_XID, 1);
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
	ecore_validate_xid(xid->parent);
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
      xid->coords_invalid = 0;
      xid->grab_button_auto_replay = NULL;
      XSaveContext(disp, xid->win, xid_context, (XPointer) xid);
      ecore_add_child(xid->parent, win);
    }
  return xid;
}

void
ecore_unvalidate_xid(Window win)
{
  Ecore_XID          *xid = NULL;

  if (!disp)
    return;
  if (XFindContext(disp, win, xid_context, (XPointer *) & xid) == XCNOENT)
    return;
  if (xid)
    {
      int                 i;

      for (i = 0; i < xid->children_num; i++)
	ecore_unvalidate_xid(xid->children[i]);
      ecore_del_child(xid->parent, win);
      IF_FREE(xid->children);
      FREE(xid);
      XDeleteContext(disp, win, xid_context);
    }
}

void
ecore_sync(void)
{
  if (!disp)
    return;

  XSync(disp, False);
}

void
ecore_flush(void)
{
  if (!disp)
    return;
  XFlush(disp);
}

Window
ecore_window_new(Window parent, int x, int y, int w, int h)
{
  Window              win;
  XSetWindowAttributes attr;

  if (!disp)
    return 0;
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
  ecore_add_xid(win, x, y, w, h, default_depth, parent);
  ecore_add_child(parent, win);
  ecore_validate_xid(parent);
  return win;
}

Window
ecore_window_override_new(Window parent, int x, int y, int w, int h)
{
  Window              win;
  XSetWindowAttributes attr;

  if (!disp)
    return 0;
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
  ecore_add_xid(win, x, y, w, h, default_depth, parent);
  ecore_add_child(parent, win);
  ecore_validate_xid(parent);
  return win;
}

Window
ecore_window_input_new(Window parent, int x, int y, int w, int h)
{
  Window              win;
  XSetWindowAttributes attr;

  if (!disp)
    return 0;
  if (!parent)
    parent = default_root;
  attr.override_redirect = True;
  attr.do_not_propagate_mask = True;
  win = XCreateWindow(disp, parent,
		      x, y, w, h, 0,
		      0, InputOnly, default_vis,
		      CWOverrideRedirect | CWDontPropagate, &attr);
  ecore_add_xid(win, x, y, w, h, 0, parent);
  ecore_add_child(parent, win);
  ecore_validate_xid(parent);
  return win;
}

void
ecore_window_set_events_propagate(Window win, int propagate)
{
  XSetWindowAttributes attr;

  if (!disp)
    return;
  if (!win)
    win = default_root;
  if (!propagate)
    attr.do_not_propagate_mask = True;
  else
    attr.do_not_propagate_mask = False;
  XChangeWindowAttributes(disp, win, CWDontPropagate, &attr);
}

void
ecore_window_show(Window win)
{
  Ecore_XID          *xid = NULL;

  if (!disp)
    return;
  xid = ecore_validate_xid(win);
  if (xid)
    {
      if (xid->mapped)
	return;
      xid->mapped = 1;
      XMapWindow(disp, win);
    }
}

void
ecore_window_hide(Window win)
{
  Ecore_XID          *xid = NULL;

  if (!disp)
    return;
  xid = ecore_validate_xid(win);
  if (xid)
    {
      if (!xid->mapped)
	return;
      xid->mapped = 0;
      XUnmapWindow(disp, win);
    }
}

Pixmap
ecore_pixmap_new(Window win, int w, int h, int dep)
{
  if (!disp)
    return 0;
  if (!win)
    win = default_win;
  if (dep == 0)
    dep = default_depth;
  return XCreatePixmap(disp, win, w, h, dep);
}

void
ecore_pixmap_free(Pixmap pmap)
{
  if (!disp)
    return;
  if (!pmap)
    return;
  XFreePixmap(disp, pmap);
}

void
ecore_window_set_background_pixmap(Window win, Pixmap pmap)
{
  if (!disp)
    return;
  if (win == 0)
    win = default_root;
  XSetWindowBackgroundPixmap(disp, win, pmap);
}

void
ecore_window_set_shape_mask(Window win, Pixmap mask)
{
  if (!disp)
    return;
  XShapeCombineMask(disp, win, ShapeBounding, 0, 0, mask, ShapeSet);
}

void
ecore_window_add_shape_mask(Window win, Pixmap mask)
{
  if (!disp)
    return;
  XShapeCombineMask(disp, win, ShapeBounding, 0, 0, mask, ShapeUnion);
}

void
ecore_window_set_shape_window(Window win, Window src, int x, int y)
{
  if (!disp)
    return;
  XShapeCombineShape(disp, win, ShapeBounding, x, y, src, ShapeBounding,
		     ShapeSet);
}

void
ecore_window_add_shape_window(Window win, Window src, int x, int y)
{
  if (!disp)
    return;
  XShapeCombineShape(disp, win, ShapeBounding, x, y, src, ShapeBounding,
		     ShapeUnion);
}

void
ecore_window_set_shape_rectangle(Window win, int x, int y, int w, int h)
{
  XRectangle          rect;

  if (!disp)
    return;
  rect.x = x;
  rect.y = y;
  rect.width = w;
  rect.height = h;
  XShapeCombineRectangles(disp, win, ShapeBounding, 0, 0, &rect, 1, ShapeSet,
			  Unsorted);
}

void
ecore_window_add_shape_rectangle(Window win, int x, int y, int w, int h)
{
  XRectangle          rect;

  if (!disp)
    return;
  rect.x = x;
  rect.y = y;
  rect.width = w;
  rect.height = h;
  XShapeCombineRectangles(disp, win, ShapeBounding, 0, 0, &rect, 1, ShapeUnion,
			  Unsorted);
}

void
ecore_window_set_shape_rectangles(Window win, XRectangle * rect, int num)
{
  if (!disp)
    return;
  XShapeCombineRectangles(disp, win, ShapeBounding, 0, 0, rect, num, ShapeSet,
			  Unsorted);
}

void
ecore_window_add_shape_rectangles(Window win, XRectangle * rect, int num)
{
  if (!disp)
    return;
  XShapeCombineRectangles(disp, win, ShapeBounding, 0, 0, rect, num, ShapeUnion,
			  Unsorted);
}

void
ecore_window_clip_shape_by_rectangle(Window win, int x, int y, int w, int h)
{
  XRectangle          rect;

  if (!disp)
    return;
  rect.x = x;
  rect.y = y;
  rect.width = w;
  rect.height = h;
  XShapeCombineRectangles(disp, win, ShapeBounding, 0, 0, &rect, 1,
			  ShapeIntersect, Unsorted);
}

XRectangle         *
ecore_window_get_shape_rectangles(Window win, int *num)
{
  int                 ord;

  if (!disp)
    return NULL;
  return XShapeGetRectangles(disp, win, ShapeBounding, num, &ord);
}

void
ecore_window_select_shape_events(Window win)
{
  if (!disp)
    return;

  XShapeSelectInput(disp, win, ShapeNotifyMask);
}

void
ecore_window_unselect_shape_events(Window win)
{
  if (!disp)
    return;

  XShapeSelectInput(disp, win, 0);
}

void
ecore_window_clear(Window win)
{
  if (!disp)
    return;
  if (win == 0)
    win = default_root;
  XClearWindow(disp, win);
}

void
ecore_window_clear_area(Window win, int x, int y, int w, int h)
{
  if (!disp)
    return;
  if (win == 0)
    win = default_root;
  XClearArea(disp, win, x, y, w, h, False);
}

void
ecore_pointer_xy(Window win, int *x, int *y)
{
  Window              dw;
  unsigned int        dm;
  int                 wx, wy;

  if (!disp)
    return;

  if (win == 0)
    win = default_root;

  XQueryPointer(disp, win, &dw, &dw, &mouse_x, &mouse_y, &wx, &wy, &dm);

  if (x)
    *x = wx;
  if (y)
    *y = wy;
}

void
ecore_pointer_xy_set(int x, int y)
{
   if (!disp) return;
  mouse_x = x;
  mouse_y = y;
}

void
ecore_pointer_xy_get(int *x, int *y)
{
   if (!disp) return;
  if (x)
    *x = mouse_x;
  if (y)
    *y = mouse_y;
}

void
ecore_window_set_events(Window win, long mask)
{
   if (!disp) return;
  if (win == 0)
    win = default_root;
  XSelectInput(disp, win, mask);
}

void
ecore_window_remove_events(Window win, long mask)
{
  XWindowAttributes   att;

   if (!disp) return;
  if (win == 0)
    win = default_root;
  if (XGetWindowAttributes(disp, win, &att) == True)
    {
      mask = att.your_event_mask & (~mask);
      ecore_window_set_events(win, mask);
    }
}

void
ecore_window_add_events(Window win, long mask)
{
  XWindowAttributes   att;

   if (!disp) return;
  if (win == 0)
    win = default_root;
  if (XGetWindowAttributes(disp, win, &att) == True)
    {
      mask = att.your_event_mask | mask;
      ecore_window_set_events(win, mask);
    }
}

void
ecore_window_move(Window win, int x, int y)
{
  Ecore_XID          *xid = NULL;

   if (!disp) return;
  xid = ecore_validate_xid(win);
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
	Ecore_XID *xid2; \
\
	xid2 = ecore_validate_xid(xid->children[j]); \
	if (xid2) \
	  { \
             xid2->coords_invalid = 1; \
          } \
     } \
}

#if 0
switch (xid2->gravity)
  {
  case UnmapGravity:
    xid2->mapped = 0;
    break;
  case NorthWestGravity:
    break;
  case NorthGravity:
    xid2->x += (w - xid->w) / 2;
    break;
  case NorthEastGravity:
    xid2->x += (w - xid->w);
    break;
  case WestGravity:
    xid2->h += (h - xid->h) / 2;
    break;
  case CenterGravity:
    xid2->x += (w - xid->w) / 2;
    xid2->h += (h - xid->h) / 2;
    break;
  case EastGravity:
    xid2->x += (w - xid->w);
    break;
  case SouthWestGravity:
    xid2->y += (h - xid->h);
    break;
  case SouthGravity:
    xid2->x += (w - xid->w) / 2;
    xid2->y += (h - xid->h);
    break;
  case SouthEastGravity:
    xid2->x += (w - xid->w);
    xid2->y += (h - xid->h);
    break;
  case StaticGravity:
    xid2->coords_invalid = 1;
    break;
  default:
    break;
  }
}
}
}
#endif

void
ecore_window_resize(Window win, int w, int h)
{
  Ecore_XID          *xid = NULL;

   if (!disp) return;
  xid = ecore_validate_xid(win);
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
ecore_window_move_resize(Window win, int x, int y, int w, int h)
{
  Ecore_XID          *xid = NULL;

   if (!disp) return;
  xid = ecore_validate_xid(win);
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
ecore_x_get_fd(void)
{
   if (!disp) return 0;
  return ConnectionNumber(disp);
}

void
ecore_set_error_handler(Ecore_Error_Function func)
{
   if (!disp) return;
  XSetErrorHandler((XErrorHandler) func);
}

void
ecore_reset_error_handler(void)
{
   if (!disp) return;
  XSetErrorHandler((XErrorHandler) ecore_handle_x_error);
}

int
ecore_display_init(char *display)
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
  XSetErrorHandler((XErrorHandler) ecore_handle_x_error);
  XSetIOErrorHandler((XIOErrorHandler) ecore_handle_x_io_error);
  default_vis = DefaultVisual(disp, DefaultScreen(disp));
  default_depth = DefaultDepth(disp, DefaultScreen(disp));
  default_cm = DefaultColormap(disp, DefaultScreen(disp));
  default_win = DefaultRootWindow(disp);
  default_root = DefaultRootWindow(disp);
  mod_shift = ecore_mod_mask_shift_get();
  mod_ctrl = ecore_mod_mask_ctrl_get();
  mod_alt = ecore_mod_mask_alt_get();
  mod_win = ecore_mod_mask_win_get();
  XGetInputFocus(disp, &focused_win, &revert);
  ecore_window_set_events(default_root, XEV_KEY | XEV_IN_OUT | XEV_MOUSE_MOVE |
			  XEV_CONFIGURE | XEV_CHILD_CHANGE | XEV_PROPERTY |
			  XEV_COLORMAP | XEV_VISIBILITY);
  ecore_pointer_xy(0, NULL, NULL);
  return 1;
}

int
ecore_events_pending(void)
{
  if (!disp) return 0;
  return XPending(disp);
}

void
ecore_get_next_event(XEvent * event)
{
   if (!disp) return;
  XNextEvent(disp, event);
}

int
ecore_event_shape_get_id(void)
{
  int                 base = -1, err_base;

   if (!disp) return 0;
  XShapeQueryExtension(disp, &base, &err_base);
  base += ShapeNotify;
  return base;
}

KeySym
ecore_key_get_keysym_from_keycode(KeyCode keycode)
{
   if (!disp) return 0;
  return XKeycodeToKeysym(disp, keycode, 0);
}

char               *
ecore_key_get_string_from_keycode(KeyCode keycode)
{
  char               *str;

   if (!disp) return strdup("");
  str = XKeysymToString(ecore_key_get_keysym_from_keycode(keycode));
  if (!str)
    return strdup("");
  return strdup(str);
}

void
ecore_event_allow(int mode, Time t)
{
   if (!disp) return;
   XAllowEvents(disp, mode, t);
}

int
ecore_lock_mask_scroll_get(void)
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

   if (!disp) return 0;
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
ecore_lock_mask_num_get(void)
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

   if (!disp) return 0;
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
ecore_lock_mask_caps_get(void)
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

   if (!disp) return 0;
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
ecore_mod_mask_shift_get(void)
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

   if (!disp) return 0;
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
ecore_mod_mask_ctrl_get(void)
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

   if (!disp) return 0;
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
ecore_mod_mask_alt_get(void)
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

   if (!disp) return 0;
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
ecore_mod_mask_win_get(void)
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

   if (!disp) return 0;
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
	      if (mask == ecore_mod_mask_alt_get())
		mask = 0;
	      if (mask == ecore_mod_mask_ctrl_get())
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
ecore_lock_mask_get(void)
{
  static int          mask = 0;
  Window              root_ret, child_ret;
  int                 root_x_ret, root_y_ret, win_x_ret, win_y_ret;
  unsigned int        mask_ret;

   if (!disp) return 0;
  if (!mask)
    mask = ecore_lock_mask_scroll_get() | ecore_lock_mask_num_get() |
      ecore_lock_mask_caps_get();
  XQueryPointer(disp, default_root, &root_ret, &child_ret, &root_x_ret,
		&root_y_ret, &win_x_ret, &win_y_ret, &mask_ret);
  return (mask_ret & mask);
}

int
ecore_modifier_mask_get(void)
{
  Window              root_ret, child_ret;
  int                 root_x_ret, root_y_ret, win_x_ret, win_y_ret;
  unsigned int        mask_ret;

   if (!disp) return 0;
  XQueryPointer(disp, default_root, &root_ret, &child_ret, &root_x_ret,
		&root_y_ret, &win_x_ret, &win_y_ret, &mask_ret);
  return (mask_ret);
}

Window
ecore_get_key_grab_win(void)
{
   if (!disp) return 0;
  return grabkey_win;
}

void
ecore_key_grab(char *key, Ecore_Event_Key_Modifiers mods, int anymod, int sync)
{
  KeyCode             keycode;
  int                 i, mod, mask_scroll, mask_num, mask_caps, masks[8], mode;

   if (!disp) return;
  keycode = ecore_key_get_keycode(key);
  mod = 0;
  mode = GrabModeAsync;
  if (sync)
    mode = GrabModeSync;
  if (!grabkey_win)
    grabkey_win = default_root;
  if (mods & ECORE_EVENT_KEY_MODIFIER_SHIFT)
    mod |= ecore_mod_mask_shift_get();
  if (mods & ECORE_EVENT_KEY_MODIFIER_CTRL)
    mod |= ecore_mod_mask_ctrl_get();
  if (mods & ECORE_EVENT_KEY_MODIFIER_ALT)
    mod |= ecore_mod_mask_alt_get();
  if (mods & ECORE_EVENT_KEY_MODIFIER_WIN)
    mod |= ecore_mod_mask_win_get();
  mask_scroll = ecore_lock_mask_scroll_get();
  mask_num = ecore_lock_mask_num_get();
  mask_caps = ecore_lock_mask_caps_get();
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
	XGrabKey(disp, keycode, masks[i] | mod, grabkey_win, False, mode, mode);
    }
}

void
ecore_key_ungrab(char *key, Ecore_Event_Key_Modifiers mods, int anymod)
{
  KeyCode             keycode;

   if (!disp) return;
  keycode = ecore_key_get_keycode(key);
  if (anymod)
    XUngrabKey(disp, keycode, AnyModifier, default_root);
  else
    {
      int                 i, mod, mask_scroll, mask_num, mask_caps, masks[8];

      mod = 0;
      if (mods & ECORE_EVENT_KEY_MODIFIER_SHIFT)
	mod |= ecore_mod_mask_shift_get();
      if (mods & ECORE_EVENT_KEY_MODIFIER_CTRL)
	mod |= ecore_mod_mask_ctrl_get();
      if (mods & ECORE_EVENT_KEY_MODIFIER_ALT)
	mod |= ecore_mod_mask_alt_get();
      if (mods & ECORE_EVENT_KEY_MODIFIER_WIN)
	mod |= ecore_mod_mask_win_get();
      mask_scroll = ecore_lock_mask_scroll_get();
      mask_num = ecore_lock_mask_num_get();
      mask_caps = ecore_lock_mask_caps_get();
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
ecore_key_get_keycode(char *key)
{
   if (!disp) return 0;
  return XKeysymToKeycode(disp, XStringToKeysym(key));
}

void
ecore_window_destroy(Window win)
{
   if (!disp) return;
  ecore_unvalidate_xid(win);
  XDestroyWindow(disp, win);
}

void
ecore_window_reparent(Window win, Window parent, int x, int y)
{
  Ecore_XID          *xid = NULL;

   if (!disp) return;
  xid = ecore_validate_xid(win);
  if (xid)
    {
      if (parent == 0)
	parent = default_root;
      XReparentWindow(disp, win, parent, x, y);
      ecore_del_child(xid->parent, win);
      ecore_add_child(parent, win);
      xid->parent = parent;
      xid->x = x;
      xid->y = y;
    }
}

void
ecore_window_raise(Window win)
{
  Ecore_XID          *xid = NULL;

   if (!disp) return;
  xid = ecore_validate_xid(win);
  if (xid)
    {
      XRaiseWindow(disp, win);
      ecore_raise_child(xid->parent, win);
    }
}

void
ecore_window_lower(Window win)
{
  Ecore_XID          *xid = NULL;

   if (!disp) return;
  xid = ecore_validate_xid(win);
  if (xid)
    {
      XLowerWindow(disp, win);
      ecore_lower_child(xid->parent, win);
    }
}

void
ecore_window_get_geometry(Window win, int *x, int *y, int *w, int *h)
{
  Ecore_XID          *xid = NULL;

   if (!disp) return;
  if (win == 0)
    win = default_root;
  xid = ecore_validate_xid(win);
  if ((xid) && (xid->coords_invalid))
    {
      Window              dw;
      int                 rx, ry;
      unsigned int        rw, rh, di;

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
ecore_window_get_depth(Window win)
{
  Ecore_XID          *xid = NULL;

   if (!disp) return 0;
  if (win == 0)
    win = default_root;
  xid = ecore_validate_xid(win);
  if (xid)
    return xid->depth;
  return -1;
}

int
ecore_window_exists(Window win)
{
  Ecore_XID          *xid = NULL;

   if (!disp) return 0;
  xid = ecore_validate_xid(win);
  if (xid)
    return 1;
  return 0;
}

Window
ecore_window_get_parent(Window win)
{
  Ecore_XID          *xid = NULL;

   if (!disp) return 0;
  xid = ecore_validate_xid(win);
  if (xid)
    return xid->parent;
  return 0;
}

Window             *
ecore_window_get_children(Window win, int *num)
{
  Ecore_XID          *xid = NULL;

   if (!disp) return NULL;
  if (win == 0)
    win = default_root;
  xid = ecore_validate_xid(win);
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
ecore_window_mouse_in(Window win)
{
  Ecore_XID          *xid = NULL;

   if (!disp) return 0;
  if (win == 0)
    win = default_root;
  xid = ecore_validate_xid(win);
  if (xid)
    return xid->mouse_in;
  return 0;
}

void
ecore_window_mouse_set_in(Window win, int in)
{
  Ecore_XID          *xid = NULL;

   if (!disp) return;
  if (win == 0)
    win = default_root;
  xid = ecore_validate_xid(win);
  if (xid)
    xid->mouse_in = in;
}

Display            *
ecore_display_get(void)
{
  return disp;
}

Window
ecore_window_get_root(Window win)
{
  Ecore_XID          *xid = NULL;

   if (!disp) return 0;
  xid = ecore_validate_xid(win);
  if (xid)
    return xid->root;
  return 0;
}

void
ecore_lock_scroll_set(int onoff)
{
  lock_scroll = onoff;
}

int
ecore_lock_scroll_get(void)
{
  return lock_scroll;
}

void
ecore_lock_num_set(int onoff)
{
  lock_num = onoff;
}

int
ecore_lock_num_get(void)
{
  return lock_num;
}

void
ecore_lock_caps_set(int onoff)
{
  lock_caps = onoff;
}

int
ecore_lock_caps_get(void)
{
  return lock_caps;
}

void
ecore_mod_shift_set(int onoff)
{
  mod_shift = onoff;
}

int
ecore_mod_shift_get(void)
{
  return mod_shift;
}

void
ecore_mod_ctrl_set(int onoff)
{
  mod_ctrl = onoff;
}

int
ecore_mod_ctrl_get(void)
{
  return mod_ctrl;
}

void
ecore_mod_alt_set(int onoff)
{
  mod_alt = onoff;
}

int
ecore_mod_alt_get(void)
{
  return mod_alt;
}

void
ecore_mod_win_set(int onoff)
{
  mod_win = onoff;
}

int
ecore_mod_win_get(void)
{
  return mod_win;
}

void
ecore_focus_window_set(Window win)
{
  focused_win = win;
}

Window
ecore_focus_window_get(void)
{
  return focused_win;
}

void
ecore_focus_to_window(Window win)
{
   if (!disp) return;
  if (win == 0)
    win = default_root;
  XSetInputFocus(disp, win, RevertToNone, CurrentTime);
}

void
ecore_focus_mode_reset(void)
{
   if (!disp) return;
   XSetInputFocus(disp, default_root, RevertToPointerRoot, CurrentTime);
}

Atom
ecore_atom_get(char *name)
{
   if (!disp) return 0;
  return XInternAtom(disp, name, False);
}

void
ecore_window_set_delete_inform(Window win)
{
  static Atom         protocols[1] = { 0 };

   if (!disp) return;
  ECORE_ATOM(protocols[0], "WM_DELETE_WINDOW");
  XSetWMProtocols(disp, win, protocols, 1);
}

void
ecore_window_property_set(Window win, Atom type, Atom format, int size,
			  void *data, int number)
{
   if (!disp) return;
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
ecore_window_property_get(Window win, Atom type, Atom format, int *size)
{
  unsigned char      *retval;
  Atom                type_ret;
  unsigned long       bytes_after, num_ret;
  int                 format_ret;
  void               *data = NULL;

  if (!disp)
    return NULL;

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
ecore_window_dnd_advertise(Window win)
{
  static Atom         atom_xdndaware = 0;
  int                 dnd_version = 3;

   if (!disp) return;
  ECORE_ATOM(atom_xdndaware, "XdndAware");
  ecore_window_property_set(win, atom_xdndaware, XA_ATOM, 32, &dnd_version, 1);
}

void
ecore_grab(void)
{
  if (!disp)
    return;

  x_grabs++;

  if (x_grabs == 1)
    XGrabServer(disp);
}

void
ecore_ungrab(void)
{
  if (!disp)
    return;

  x_grabs--;

  if (x_grabs == 0)
    {
      XUngrabServer(disp);
      ecore_sync();
    }
}

void
ecore_window_ignore(Window win)
{
  Window_List        *w;

   if (!disp) return;
  if (win == 0)
    win = default_root;
  w = NEW(Window_List, 1);
  w->win = win;
  w->next = ignore_wins;
  ignore_wins = w;
}

void
ecore_window_no_ignore(Window win)
{
  Window_List        *w, *pw;

   if (!disp) return;
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
ecore_window_is_ignored(Window win)
{
  Window_List        *w;

   if (!disp) return 0;
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
ecore_window_at_xy_0(Window base, int bx, int by, int x, int y)
{
  Window             *list = NULL;
  XWindowAttributes   att;
  Window              child = 0, parent_win = 0, root_win = 0;
  int                 i;
  unsigned int        ww, wh, num;
  int                 wx, wy;

   if (!disp) return 0;
  if ((!XGetWindowAttributes(disp, base, &att)) ||
      (att.map_state != IsViewable))
    return 0;

  wx = att.x;
  wy = att.y;
  ww = att.width;
  wh = att.height;

  wx += bx;
  wy += by;

  if (!((x >= wx) && (y >= wy) && (x < (int)(wx + ww)) && (y < (int)(wy + wh))))
    return 0;

  if (!XQueryTree(disp, base, &root_win, &parent_win, &list, &num))
    return base;
  if (list)
    {
      for (i = num - 1;; i--)
	{
	  if (!ecore_window_is_ignored(list[i]))
	    {
	      if ((child = ecore_window_at_xy_0(list[i], wx, wy, x, y)) != 0)
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
ecore_window_get_at_xy(int x, int y)
{
  Window              child;

   if (!disp) return 0;
  ecore_grab();
  child = ecore_window_at_xy_0(default_root, 0, 0, x, y);
  if (child)
    {
      ecore_ungrab();
      return child;
    }
  ecore_ungrab();
  return default_root;
}

int
ecore_window_dnd_capable(Window win)
{
  static Atom         atom_xdndaware = 0;
  int                 dnd_version = 3;
  int                *atom_ret;
  int                 size = 0;

   if (!disp) return 0;
  ECORE_ATOM(atom_xdndaware, "XdndAware");
  atom_ret = ecore_window_property_get(win, atom_xdndaware, XA_ATOM, &size);
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
ecore_window_dnd_handle_motion(Window source_win, int x, int y, int dragging)
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
  static Atom         atom_text_moz_url = 0;
  static Atom         atom_netscape_url = 0;
  Window              win;
  XEvent              xevent;

   if (!disp) return;
  win = ecore_window_get_at_xy(x, y);
  while ((win) && (!ecore_window_dnd_capable(win)))
    win = ecore_window_get_parent(win);
  ECORE_ATOM(atom_xdndenter, "XdndEnter");
  ECORE_ATOM(atom_xdndleave, "XdndLeave");
  ECORE_ATOM(atom_xdnddrop, "XdndDrop");
  ECORE_ATOM(atom_xdndposition, "XdndPosition");
  ECORE_ATOM(atom_xdndactioncopy, "XdndActionCopy");
  ECORE_ATOM(atom_xdndactionmove, "XdndActionMove");
  ECORE_ATOM(atom_xdndactionlink, "XdndActionLink");
  ECORE_ATOM(atom_xdndactionask, "XdndActionAsk");
  ECORE_ATOM(atom_text_uri_list, "text/uri-list");
  ECORE_ATOM(atom_text_plain, "text/plain");
  ECORE_ATOM(atom_text_moz_url, "text/x-moz-url");
  ECORE_ATOM(atom_netscape_url, "_NETSCAPE_URL");
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
	  xevent.xclient.data.l[4] = atom_text_moz_url;
	  XSendEvent(disp, win, False, 0, &xevent);
	  ecore_clear_target_status();
	}
      /* kjb cep */
      if(!dnd_await_target_status)
	{
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
	  dnd_await_target_status = 1;
	}
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


void
ecore_clear_target_status(void)
{
  dnd_await_target_status = 0;
}


int
ecore_dnd_selection_convert(Window win, Window req, Atom type)
{
  static Atom         atom_xdndselection = 0;
  static Atom         atom_jxselectionwindowproperty = 0;

  if (!disp) return 0;
  ECORE_ATOM(atom_xdndselection, "XdndSelection");
  ECORE_ATOM(atom_jxselectionwindowproperty, "JXSelectionWindowProperty");
  if (win == XGetSelectionOwner(disp, atom_xdndselection))
    {
      XConvertSelection(disp, atom_xdndselection, type,
			atom_jxselectionwindowproperty, req, CurrentTime);
      return 1;
    }
  return 0;
}

void               *
ecore_dnd_selection_get(Window win, Window req, Atom type, int *size)
{
  unsigned char      *data = NULL;
  long                bytes_read;
  unsigned long       remaining = 1;

   if (!disp) return NULL;
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
ecore_dnd_set_data(Window win)
{
   static Atom         atom_xdndselection = 0;
   
   if (!disp) return;
   ECORE_ATOM(atom_xdndselection, "XdndSelection");
   ecore_dnd_set_action(win);
   XSetSelectionOwner(disp, atom_xdndselection, win, CurrentTime);
}

void
ecore_dnd_set_action(Window win)
{
   static int          atom_xdndactioncopy = 0;
   static int          atom_xdndactionmove = 0;
   static int          atom_xdndactionlink = 0;
   static int          atom_xdndactionask = 0;
   static Atom         atom_xdndactionlist = 0;
   
   if (!disp) return;
   ECORE_ATOM(atom_xdndactioncopy, "XdndActionCopy");
   ECORE_ATOM(atom_xdndactionmove, "XdndActionMove");
   ECORE_ATOM(atom_xdndactionlink, "XdndActionLink");
   ECORE_ATOM(atom_xdndactionask, "XdndActionAsk");
   ECORE_ATOM(atom_xdndactionlist, "XdndActionList");
   if (dnd_copy)
     ecore_window_property_set(win, atom_xdndactionlist, XA_ATOM, 32,
			       &atom_xdndactioncopy, 1);
   else if (dnd_link)
     ecore_window_property_set(win, atom_xdndactionlist, XA_ATOM, 32,
			       &atom_xdndactionlink, 1);
   else if (dnd_move)
     ecore_window_property_set(win, atom_xdndactionlist, XA_ATOM, 32,
			       &atom_xdndactionmove, 1);
   else
     ecore_window_property_set(win, atom_xdndactionlist, XA_ATOM, 32,
			       &atom_xdndactionask, 1);
}

void
ecore_dnd_send_data(Window win, Window source_win, void *data, int size,
		    Atom dest_atom, int type)
{
  XEvent              xevent;
  static Atom         atom_text_plain = 0;
  static Atom         atom_text_uri_list = 0;
  static Atom         atom_text_moz_url = 0;
  static Atom         atom_netscape_url = 0; 
  static Atom         atom_xdndselection = 0;
  Atom                target;

   if (!disp) return;
  ECORE_ATOM(atom_xdndselection, "XdndSelection");
  ECORE_ATOM(atom_text_uri_list, "text/uri-list");
  ECORE_ATOM(atom_text_plain,    "text/plain");
  ECORE_ATOM(atom_text_moz_url,  "text/x-moz-url");
  ECORE_ATOM(atom_text_moz_url,  "_NETSCAPE_URL");
  ECORE_ATOM(atom_text_plain,    "text/plain");
  if      (type == DND_TYPE_URI_LIST)     target = atom_text_uri_list;
  else if (type == DND_TYPE_PLAIN_TEXT)   target = atom_text_plain;
  else if (type == DND_TYPE_MOZ_URL)      target = atom_text_moz_url;
  else if (type == DND_TYPE_NETSCAPE_URL) target = atom_netscape_url;
  else                                    target = 0;
  ecore_window_property_set(win, dest_atom, target, 8, data, size);
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
ecore_dnd_set_mode_copy(void)
{
  dnd_copy = 1;
  dnd_link = 0;
  dnd_move = 0;
}

void
ecore_dnd_set_mode_link(void)
{
  dnd_copy = 0;
  dnd_link = 1;
  dnd_move = 0;
}

void
ecore_dnd_set_mode_move(void)
{
  dnd_copy = 0;
  dnd_link = 0;
  dnd_move = 1;
}

void
ecore_dnd_set_mode_ask(void)
{
  dnd_copy = 0;
  dnd_link = 0;
  dnd_move = 0;
}

void
ecore_dnd_own_selection(Window win)
{
  static Atom         atom_xdndselection = 0;
  static Atom         atom_jxselectionwindowproperty = 0;

   if (!disp) return;
  ECORE_ATOM(atom_xdndselection, "XdndSelection");
  ECORE_ATOM(atom_jxselectionwindowproperty, "JXSelectionWindowProperty");

  if (!XSetSelectionOwner(disp, atom_xdndselection, win, CurrentTime))
    return;
}

void
ecore_dnd_send_drop(Window win, Window source_win)
{
  static Atom         atom_xdnddrop = 0;
  XEvent              xevent;

   if (!disp) return;
  ECORE_ATOM(atom_xdnddrop, "XdndDrop");

  ecore_dnd_own_selection(source_win);

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
ecore_window_dnd_send_status_ok(Window source_win, Window win, int x, int y,
				int w, int h)
{
  static Atom         atom_xdndstatus = 0;
  static Atom         atom_xdndactioncopy = 0;
  static Atom         atom_xdndactionmove = 0;
  static Atom         atom_xdndactionlink = 0;
  static Atom         atom_xdndactionask = 0;
  XEvent              xevent;

   if (!disp) return;
  ECORE_ATOM(atom_xdndstatus, "XdndStatus");
  ECORE_ATOM(atom_xdndactioncopy, "XdndActionCopy");
  ECORE_ATOM(atom_xdndactionmove, "XdndActionMove");
  ECORE_ATOM(atom_xdndactionlink, "XdndActionLink");
  ECORE_ATOM(atom_xdndactionask, "XdndActionAsk");
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
ecore_window_dnd_send_finished(Window source_win, Window win)
{
  static Atom         atom_xdndfinished = 0;
  XEvent              xevent;

   if (!disp) return;
  ECORE_ATOM(atom_xdndfinished, "XdndFinished");
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
ecore_window_dnd_ok(int ok)
{
  current_dnd_target_ok = ok;
}

void
ecore_window_dnd_finished(void)
{
  current_dnd_win = 0;
}

void
ecore_window_set_title(Window win, char *title)
{
   if (!disp) return;
  XStoreName(disp, win, title);
}

void
ecore_window_set_name_class(Window win, char *name, char *class)
{
  XClassHint          hint;

   if (!disp) return;
  hint.res_name = name;
  hint.res_class = class;
  XSetClassHint(disp, win, &hint);
}

void
ecore_window_get_name_class(Window win, char **name, char **class)
{
  XClassHint          xch;

   if (!disp) return;
  if (name)
    *name = NULL;
  if (class)
    *class = NULL;
  if (XGetClassHint(disp, win, &xch))
    {
      if (name)
	{
	  if (xch.res_name)
	    *name = strdup(xch.res_name);
	}
      if (class)
	{
	  if (xch.res_class)
	    *class = strdup(xch.res_class);
	}
      XFree(xch.res_name);
      XFree(xch.res_class);
    }
}

void
ecore_window_get_hints(Window win, int *accepts_focus, int *initial_state,
		       Pixmap * icon_pixmap, Pixmap * icon_mask,
		       Window * icon_window, Window * window_group)
{
  XWMHints           *hints;

   if (!disp) return;
  hints = XGetWMHints(disp, win);
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

char               *
ecore_window_get_machine(Window win)
{
  XTextProperty       xtp;

   if (!disp) return NULL;
  if (XGetWMClientMachine(disp, win, &xtp))
    {
      char               *s;

      if (!xtp.value)
	return NULL;
      s = strdup(xtp.value);
      XFree(xtp.value);
      return s;
    }
  return NULL;
}

char               *
ecore_window_get_command(Window win)
{
   int                 cargc;
   char              **cargv;
   
   if (!disp) return NULL;
   if (XGetCommand(disp, win, &cargv, &cargc))
     {
	if (cargc > 0)
	  {
	     char               *s;
	     int                 size, i;
	     
	     size = 0;
	     for (i = 0; i < cargc; i++)
	       size += strlen(cargv[i]);
	     size += cargc - 1;
	     s = NEW(char, size + 1);
	     s[0] = 0;
	     
	     strcpy(s, cargv[0]);
	     for (i = 1; i < cargc; i++)
	       {
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

char               *
ecore_window_get_icon_name(Window win)
{
  XTextProperty       xtp;

   if (!disp) return NULL;
  if (XGetWMIconName(disp, win, &xtp))
    {
      char               *s;

      if (!xtp.value)
	return NULL;
      s = strdup(xtp.value);
      XFree(xtp.value);
      return s;
    }
  return NULL;
}

void
ecore_window_set_min_size(Window win, int w, int h)
{
  XSizeHints          hints;
  long                ret;

   if (!disp) return;
  memset(&hints, 0, sizeof(XSizeHints));
  XGetWMNormalHints(disp, win, &hints, &ret);
  hints.flags |= PMinSize | PSize | USSize;
  hints.min_width = w;
  hints.min_height = h;
  XSetWMNormalHints(disp, win, &hints);
}

void
ecore_window_set_max_size(Window win, int w, int h)
{
  XSizeHints          hints;
  long                ret;

   if (!disp) return;
  memset(&hints, 0, sizeof(XSizeHints));
  XGetWMNormalHints(disp, win, &hints, &ret);
  hints.flags |= PMaxSize | PSize | USSize;
  hints.max_width = w;
  hints.max_height = h;
  XSetWMNormalHints(disp, win, &hints);
}

void
ecore_window_set_xy_hints(Window win, int x, int y)
{
  XSizeHints          hints;
  long                ret;

   if (!disp) return;
  memset(&hints, 0, sizeof(XSizeHints));
  XGetWMNormalHints(disp, win, &hints, &ret);
  hints.flags |= PPosition | USPosition | PSize | USSize;
  hints.x = x;
  hints.y = y;
  XSetWMNormalHints(disp, win, &hints);
}

void
ecore_window_get_frame_size(Window win, int *l, int *r, int *t, int *b)
{
  static Atom         atom_e_frame_size = 0;
  int                *data, size;

   if (!disp) return;
  ECORE_ATOM(atom_e_frame_size, "_E_FRAME_SIZE");
  data = ecore_window_property_get(win, atom_e_frame_size, XA_CARDINAL, &size);
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
ecore_window_save_under(Window win)
{
  XSetWindowAttributes att;
  XWindowAttributes   gatt;

   if (!disp) return 0;
  att.save_under = True;
  XChangeWindowAttributes(disp, win, CWSaveUnder, &att);
  XGetWindowAttributes(disp, win, &gatt);
  return (gatt.save_under == True) ? 1 : 0;
}

GC
ecore_gc_new(Drawable d)
{
  XGCValues           gcv;

   if (!disp) return 0;
  if (d == 0)
    d = default_root;
  return XCreateGC(disp, d, 0, &gcv);
}

void
ecore_gc_free(GC gc)
{
   if (!disp) return;
  XFreeGC(disp, gc);
}

void
ecore_gc_set_fg(GC gc, int val)
{
   if (!disp) return;
  XSetForeground(disp, gc, val);
}

void
ecore_fill_rectangle(Drawable d, GC gc, int x, int y, int w, int h)
{
   if (!disp) return;
  XFillRectangle(disp, d, gc, x, y, w, h);
}

void
ecore_draw_rectangle(Drawable d, GC gc, int x, int y, int w, int h)
{
   if (!disp) return;
  XDrawRectangle(disp, d, gc, x, y, w - 1, h - 1);
}

void
ecore_draw_line(Drawable d, GC gc, int x1, int y1, int x2, int y2)
{
   if (!disp) return;
  XDrawLine(disp, d, gc, x1, y1, x2, y2);
}

void
ecore_draw_point(Drawable d, GC gc, int x, int y)
{
   if (!disp) return;
  XDrawPoint(disp, d, gc, x, y);
}

void
ecore_window_hint_set_layer(Window win, int layer)
{
  static Atom         atom_win_layer = 0;

   if (!disp) return;
  ECORE_ATOM(atom_win_layer, "_WIN_LAYER");
  ecore_window_property_set(win, atom_win_layer, XA_CARDINAL, 32, &layer, 1);
}

void
ecore_window_hint_set_sticky(Window win, int sticky)
{
  static Atom         atom_win_state = 0;
  static Atom         atom_win_hints = 0;
  int                 data;

   if (!disp) return;
  ECORE_ATOM(atom_win_state, "_WIN_STATE");
  ECORE_ATOM(atom_win_hints, "_WIN_HINTS");
  if (sticky)
    {
      data = ((1 << 0) | (1 << 8) | (1 << 9));
      ecore_window_property_set(win, atom_win_state, XA_CARDINAL, 32, &data, 1);
      data = ((1 << 0) | (1 << 1) | (1 << 2));
      ecore_window_property_set(win, atom_win_hints, XA_CARDINAL, 32, &data, 1);
    }
  else
    {
      data = 0;
      ecore_window_property_set(win, atom_win_state, XA_CARDINAL, 32, &data, 1);
      ecore_window_property_set(win, atom_win_hints, XA_CARDINAL, 32, &data, 1);
    }
}

void
ecore_window_hint_set_borderless(Window win)
{
  static Atom         atom_motif_wm_hints = 0;
  int                 data[5];

   if (!disp) return;
  ECORE_ATOM(atom_motif_wm_hints, "_MOTIF_WM_HINTS");
  data[0] = 0x3;
  data[1] = 0x0;
  data[2] = 0x0;
  data[3] = 0x2ada27b0;
  data[4] = 0x2aabd6b0;
  ecore_window_property_set(win, atom_motif_wm_hints, atom_motif_wm_hints, 32,
			    data, 5);
}

void
ecore_grab_mouse(Window win, int confine, Cursor cursor)
{
  int                 ret;

  if (!disp)
    return;
  if (confine)
    ret = XGrabPointer(disp, win, False,
		       XEV_BUTTON | XEV_MOUSE_MOVE | XEV_IN_OUT,
		       GrabModeAsync, GrabModeAsync, win, cursor, CurrentTime);
  else
    ret = XGrabPointer(disp, win, False,
		       XEV_BUTTON | XEV_MOUSE_MOVE | XEV_IN_OUT,
		       GrabModeAsync, GrabModeAsync, None, cursor, CurrentTime);
  if (ret == GrabSuccess)
    grab_pointer_win = win;
}

void
ecore_ungrab_mouse(void)
{
  if (!disp) return;
  XUngrabPointer(disp, CurrentTime);
  grab_pointer_win = 0;
}

Window
ecore_grab_window_get(void)
{
  return grab_pointer_win;
}

int
ecore_window_get_gravity(Window win)
{
  XWindowAttributes   att;

  if (!disp)
    return 0;

  XGetWindowAttributes(disp, win, &att);
  return att.win_gravity;
}

void
ecore_window_gravity_reset(Window win)
{
  Ecore_XID          *xid = NULL;

  if (!disp) return;
  xid = ecore_validate_xid(win);
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
ecore_window_gravity_set(Window win, int gravity)
{
  Ecore_XID          *xid = NULL;

   if (!disp) return;
  xid = ecore_validate_xid(win);
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
ecore_window_bit_gravity_set(Window win, int gravity)
{
  Ecore_XID          *xid = NULL;

   if (!disp) return;
  xid = ecore_validate_xid(win);
  if (xid)
    {
      XSetWindowAttributes att;

      att.bit_gravity = gravity;
      XChangeWindowAttributes(disp, win, CWBitGravity, &att);
    }
}

void
ecore_pointer_warp_by(int dx, int dy)
{
   if (!disp) return;
  XWarpPointer(disp, None, None, 0, 0, 0, 0, dx, dy);
}

void
ecore_pointer_warp_to(int x, int y)
{
   if (!disp) return;
  XWarpPointer(disp, None, default_root, 0, 0, 0, 0, x, y);
}

void
ecore_gc_set_include_inferiors(GC gc)
{
  XGCValues           gcv;

   if (!disp) return;
  gcv.subwindow_mode = IncludeInferiors;
  XChangeGC(disp, gc, GCSubwindowMode, &gcv);
}

void
ecore_area_copy(Drawable src, Drawable dest, GC gc,
		int sx, int sy, int sw, int sh, int dx, int dy)
{
   if (!disp) return;
  if (src == 0)
    src = default_root;
  if (dest == 0)
    dest = default_root;
  XCopyArea(disp, src, dest, gc, sx, sy, sw, sh, dx, dy);
}

Window
ecore_window_root(void)
{
  return default_root;
}

void
ecore_window_get_virtual_area(Window win, int *area_x, int *area_y)
{
  static Atom         atom_win_area = 0;
  int                *data, size;

   if (!disp) return;
  ECORE_ATOM(atom_win_area, "_WIN_AREA");
  data = ecore_window_property_get(win, atom_win_area, XA_CARDINAL, &size);
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
ecore_get_virtual_area(int *area_x, int *area_y)
{
  static Atom         atom_win_area = 0;
  int                *data, size;

   if (!disp) return;
  ECORE_ATOM(atom_win_area, "_WIN_AREA");
  data = ecore_window_property_get(default_root, atom_win_area, XA_CARDINAL, &size);
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
ecore_window_get_root_relative_location(Window win, int *x, int *y)
{
  int                 dx, dy;
  Window              parent;
  Ecore_XID          *xid = NULL;

   if (!disp) return;
  if (win == 0)
    win = default_root;
  if (win == default_root)
    {
      if (x)
	*x = 0;
      if (y)
	*y = 0;
      return;
    }
  xid = ecore_validate_xid(win);
  if (!xid)
    {
      if (x)
	*x = 0;
      if (y)
	*y = 0;
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
	  xid = ecore_validate_xid(parent);
	  if (!xid)
	    {
	      if (x)
		*x = dx;
	      if (y)
		*y = dy;
	      return;
	    }
	}
    }
  while (parent != default_root);
  if (x)
    *x = dx;
  if (y)
    *y = dy;
}

void
ecore_window_button_grab_auto_replay_set(Window win, int (*func) (Ecore_Event_Mouse_Down *ev))
{
  Ecore_XID *xid = NULL;

   if (!disp) return;
  xid = ecore_validate_xid(win);
  if (!xid) return;
  xid->grab_button_auto_replay = func;
}

void *
ecore_window_button_grab_auto_replay_get(Window win)
{
  Ecore_XID *xid = NULL;

   if (!disp) return NULL;
  xid = ecore_validate_xid(win);
  if (!xid) return NULL;
  return xid->grab_button_auto_replay;
}

void
ecore_button_grab(Window win, int button, int events,
		  Ecore_Event_Key_Modifiers mod, int any_mod)
{
  unsigned int        b;
  unsigned int        m;
  unsigned int        locks[8];
  int                 i;

   if (!disp) return;
  b = button;
  if (b == 0)
    b = AnyButton;
  m = 0;
  if (any_mod)
    m = AnyModifier;
  else
    {
      if (mod & ECORE_EVENT_KEY_MODIFIER_SHIFT)
	m |= ecore_mod_mask_shift_get();
      if (mod & ECORE_EVENT_KEY_MODIFIER_CTRL)
	m |= ecore_mod_mask_ctrl_get();
      if (mod & ECORE_EVENT_KEY_MODIFIER_ALT)
	m |= ecore_mod_mask_alt_get();
      if (mod & ECORE_EVENT_KEY_MODIFIER_WIN)
	m |= ecore_mod_mask_win_get();
    }
  locks[0] = 0;
  locks[1] = ecore_lock_mask_caps_get();
  locks[2] = ecore_lock_mask_num_get();
  locks[3] = ecore_lock_mask_scroll_get();
  locks[4] = ecore_lock_mask_caps_get() | ecore_lock_mask_num_get();
  locks[5] = ecore_lock_mask_caps_get() | ecore_lock_mask_scroll_get();
  locks[6] = ecore_lock_mask_num_get() | ecore_lock_mask_scroll_get();
  locks[7] =
    ecore_lock_mask_caps_get() | ecore_lock_mask_num_get() |
    ecore_lock_mask_scroll_get();
  for (i = 0; i < 8; i++)
    XGrabButton(disp, b, m | locks[i],
		win, False, events, GrabModeSync, GrabModeAsync, None, None);
}

void
ecore_button_ungrab(Window win, int button, Ecore_Event_Key_Modifiers mod,
		    int any_mod)
{
  unsigned int        b;
  unsigned int        m;
  unsigned int        locks[8];
  int                 i;

   if (!disp) return;
  b = button;
  if (b == 0)
    b = AnyButton;
  m = 0;
  if (any_mod)
    m = AnyModifier;
  else
    {
      if (mod & ECORE_EVENT_KEY_MODIFIER_SHIFT)
	m |= ecore_mod_mask_shift_get();
      if (mod & ECORE_EVENT_KEY_MODIFIER_CTRL)
	m |= ecore_mod_mask_ctrl_get();
      if (mod & ECORE_EVENT_KEY_MODIFIER_ALT)
	m |= ecore_mod_mask_alt_get();
      if (mod & ECORE_EVENT_KEY_MODIFIER_WIN)
	m |= ecore_mod_mask_win_get();
    }
  locks[0] = 0;
  locks[1] = ecore_lock_mask_caps_get();
  locks[2] = ecore_lock_mask_num_get();
  locks[3] = ecore_lock_mask_scroll_get();
  locks[4] = ecore_lock_mask_caps_get() | ecore_lock_mask_num_get();
  locks[5] = ecore_lock_mask_caps_get() | ecore_lock_mask_scroll_get();
  locks[6] = ecore_lock_mask_num_get() | ecore_lock_mask_scroll_get();
  locks[7] =
    ecore_lock_mask_caps_get() | ecore_lock_mask_num_get() |
    ecore_lock_mask_scroll_get();
  for (i = 0; i < 8; i++)
    XUngrabButton(disp, b, m | locks[i], win);
}

void
ecore_pointer_replay(Time t)
{
   if (!disp) return;
   XSync(disp, False);
   XAllowEvents(disp, ReplayPointer, t);
   XSync(disp, False);
}

void
ecore_pointer_grab(Window win, Time t)
{
   if (!disp) return;
  XGrabPointer(disp, win, False, XEV_BUTTON | XEV_MOUSE_MOVE | XEV_IN_OUT,
	       GrabModeAsync, GrabModeAsync, None, None, t);
}

void
ecore_pointer_ungrab(Time t)
{
   if (!disp) return;
  XUngrabPointer(disp, t);
}

void
ecore_window_send_event_move_resize(Window win, int x, int y, int w, int h)
{
  XEvent              ev;

   if (!disp) return;
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
ecore_window_send_client_message(Window win, Atom type, int format, void *data)
{
  XClientMessageEvent ev;
  int                 i;

   if (!disp) return;
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
ecore_window_add_to_save_set(Window win)
{
   if (!disp) return;
  XAddToSaveSet(disp, win);
}

void
ecore_window_del_from_save_set(Window win)
{
   if (!disp) return;
  XRemoveFromSaveSet(disp, win);
}

void
ecore_window_kill_client(Window win)
{
   if (!disp) return;
  XKillClient(disp, (XID) win);
}

void
ecore_window_set_border_width(Window win, int bw)
{
  Ecore_XID          *xid = NULL;

   if (!disp) return;
  xid = ecore_validate_xid(win);
  if (xid)
    {
      xid->bw = bw;
    }
  XSetWindowBorderWidth(disp, win, bw);
}

int
ecore_window_get_border_width(Window win)
{
  Ecore_XID          *xid = NULL;

  if (!disp) return 0;

  xid = ecore_validate_xid(win);
  if (xid)
    {
      return xid->bw;
    }

  return 0;
}

int
ecore_window_get_wm_size_hints(Window win, XSizeHints * hints, int *mask)
{
  long                sup_ret;
  Status              ok;

  if (!disp) return 0;

  ok = XGetWMNormalHints(disp, win, hints, &sup_ret);
  *mask = (int)sup_ret;
  return ok;
}

int
ecore_window_is_visible(Window win)
{
  XWindowAttributes   att;

  if (!disp) return 0;
  if (win == 0)
    win = default_root;
  if (XGetWindowAttributes(disp, win, &att) == True)
    {
      if (att.map_state == IsUnmapped)
	return 0;
      return 1;
    }
  return 0;
}

int
ecore_window_is_normal(Window win)
{
  XWindowAttributes   att;

  if (!disp) return 0;
  if (win == 0)
    win = default_root;
  if (XGetWindowAttributes(disp, win, &att) == True)
    {
      if ((att.override_redirect) || (att.class == InputOnly))
	return 0;
      return 1;
    }
  return 0;
}

int
ecore_window_is_manageable(Window win)
{
  XWindowAttributes   att;

  if (!disp) return 0;
  if (win == 0)
    win = default_root;
  if (XGetWindowAttributes(disp, win, &att) == True)
    {
      if ((att.map_state == IsUnmapped) || (att.override_redirect)
	  || (att.class == InputOnly))
	return 0;
      return 1;
    }
  return 0;
}

void
ecore_windows_restack(Window * wins, int num)
{
   if (!disp) return;
  XRestackWindows(disp, wins, num);
}

void
ecore_window_stack_above(Window win, Window above)
{
  XWindowChanges      xwc;

   if (!disp) return;
  if (win == 0)
    win = default_root;
  xwc.sibling = above;
  xwc.stack_mode = Above;
  XConfigureWindow(disp, win, CWSibling | CWStackMode, &xwc);
}

void
ecore_window_stack_below(Window win, Window below)
{
  XWindowChanges      xwc;

  if (!disp) return;

  if (win == 0)
    win = default_root;
  xwc.sibling = below;
  xwc.stack_mode = Below;
  XConfigureWindow(disp, win, CWSibling | CWStackMode, &xwc);
}

char               *
ecore_window_get_title(Window win)
{
  XTextProperty       xtp;

  if (!disp) return 0;

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
	  else
	    title = strdup((char *)xtp.value);
	}
      else
	title = strdup((char *)xtp.value);
      XFree(xtp.value);
      return title;
    }
  return NULL;
}

void
ecore_keyboard_grab(Window win)
{
  int                 status;

   if (!disp) return;
  if (keyboard_grab_win)
    return;
  if (win == 0)
    win = default_root;
  keyboard_grab_win = win;
  status =
    XGrabKeyboard(disp, win, False, GrabModeAsync, GrabModeAsync, CurrentTime);
  if ((status == AlreadyGrabbed) || (status == GrabNotViewable)
      || (status == GrabFrozen) || (status == GrabInvalidTime))
    keyboard_grab_win = 0;
}

void
ecore_keyboard_ungrab(void)
{
   if (!disp) return;
  if (!keyboard_grab_win)
    return;
  keyboard_grab_win = 0;
  XUngrabKeyboard(disp, CurrentTime);
}

Window
ecore_keyboard_grab_window_get(void)
{
   if (!disp) return 0;
   return keyboard_grab_win;
}

Window
ecore_selection_set(char *string)
{
  Window              target = 0;
  static Atom         dest = 0;
  Atom                selection;

   if (!disp) return 0;
  selection = X_CLIPBOARD_SELECTION;
  ECORE_ATOM(dest, "TEXT_SELECTION");
  target = ecore_window_new(0, 0, 0, 77, 7);
  ecore_window_add_events(target, XEV_CONFIGURE | XEV_PROPERTY);
  XSetSelectionOwner(disp, selection, target, CurrentTime);
  if (XGetSelectionOwner(disp, XA_PRIMARY) != target)
    {
      ecore_window_destroy(target);
      return 0;
    }
  XChangeProperty(disp, target, dest,
		  XA_STRING, 8, PropModeReplace, string, strlen(string));
  return target;
}

Window
ecore_selection_request(void)
{
  static Atom         dest = 0;
  Atom                selection;
  Window              target = 0;

   if (!disp) return 0;
  selection = X_CLIPBOARD_SELECTION;
  ECORE_ATOM(dest, "TEXT_SELECTION");
  target = ecore_window_new(0, 0, 0, 7, 77);
  ecore_window_add_events(target, XEV_CONFIGURE | XEV_PROPERTY);
  XConvertSelection(disp, XA_PRIMARY, XA_STRING, dest, target, CurrentTime);
  return target;
}

char               *
ecore_selection_get_data(Window win, Atom prop)
{
  char               *string = NULL;
  long                nread;
  unsigned long       bytes_after, nitems;
  unsigned char      *data;
  Atom                actual_type;
  int                 actual_fmt;

   if (!disp) return NULL;
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
	  int                 size, i;
	  XTextProperty       xtextp;
	  char              **cl = NULL;

	  xtextp.value = data;
	  xtextp.encoding = actual_type;
	  xtextp.format = actual_fmt;
	  xtextp.nitems = nitems;
	  XmbTextPropertyToTextList(disp, &xtextp, &cl, &size);

	  if (cl)
	    {
	      for (i = 0; i < size; i++)
		{
		  if (cl[i])
		    {
		      if (string)
			string =
			  realloc(string, strlen(string) + strlen(cl[i]) + 1);
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
ecore_set_blank_pointer(Window w)
{
  Cursor              c;
  XColor              cl;
  Pixmap              p, m;
  GC                  gc;
  XGCValues           gcv;

   if (!disp) return;
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
ecore_cursor_new(Pixmap pmap, Pixmap mask, int x, int y, int fr, int fg, int fb,
		 int br, int bg, int bb)
{
  XColor              cl1, cl2;

   if (!disp) return 0;
  cl1.pixel = 0;
  cl1.red = fr << 8 | fr;
  cl1.green = fg << 8 | fg;
  cl1.blue = fb << 8 | fb;
  cl1.flags = DoRed | DoGreen | DoBlue;
  cl2.pixel = 0;
  cl2.red = br << 8 | br;
  cl2.green = bg << 8 | bg;
  cl2.blue = bb << 8 | bb;
  cl2.flags = DoRed | DoGreen | DoBlue;
  return XCreatePixmapCursor(disp, pmap, mask, &cl1, &cl2, x, y);
}

void
ecore_cursor_free(Cursor c)
{
   if (!disp) return;
  XFreeCursor(disp, c);
}

void
ecore_cursor_set(Window win, Cursor c)
{
   if (!disp) return;
  if (win == 0)
    win = default_root;
  XDefineCursor(disp, win, c);
}
