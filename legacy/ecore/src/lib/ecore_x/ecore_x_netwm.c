/*
 * _NET_WM... aka Extended Window Manager Hint (EWMH) functions.
 */
#include "Ecore.h"
#include "ecore_x_private.h"
#include "Ecore_X.h"
#include "Ecore_X_Atoms.h"

/*
 * Convenience macros
 */
#define _ATOM_GET(name) \
   XInternAtom(_ecore_x_disp, name, False)

#define _ATOM_SET_UTF8_STRING(atom, win, string) \
   XChangeProperty(_ecore_x_disp, win, atom, ECORE_X_ATOM_UTF8_STRING, 8, PropModeReplace, \
                   (unsigned char *)string, strlen(string))
#define _ATOM_SET_UTF8_STRING_LIST(atom, win, string, cnt) \
   XChangeProperty(_ecore_x_disp, win, atom, ECORE_X_ATOM_UTF8_STRING, 8, PropModeReplace, \
                   (unsigned char *)string, cnt)
#define _ATOM_SET_WINDOW(atom, win, p_wins, cnt) \
   XChangeProperty(_ecore_x_disp, win, atom, XA_WINDOW, 32, PropModeReplace, \
                   (unsigned char *)p_wins, cnt)
#define _ATOM_SET_ATOM(atom, win, p_atom, cnt) \
   XChangeProperty(_ecore_x_disp, win, atom, XA_ATOM, 32, PropModeReplace, \
                   (unsigned char *)p_atom, cnt)
#define _ATOM_SET_CARD32(atom, win, p_val, cnt) \
   XChangeProperty(_ecore_x_disp, win, atom, XA_CARDINAL, 32, PropModeReplace, \
                   (unsigned char *)p_val, cnt)

/*
 * Convenience functions. Should probably go elsewhere.
 */

/*
 * Set UTF-8 string property
 */
static void
_ecore_x_window_prop_string_utf8_set(Ecore_X_Window win, Ecore_X_Atom atom,
				     const char *str)
{
   _ATOM_SET_UTF8_STRING(win, atom, str);
}

/*
 * Get UTF-8 string property
 */
static char        *
_ecore_x_window_prop_string_utf8_get(Ecore_X_Window win, Ecore_X_Atom atom)
{
   char               *str;
   unsigned char      *prop_return;
   Atom                type_ret;
   unsigned long       bytes_after, num_ret;
   int                 format_ret;

   str = NULL;
   prop_return = NULL;
   XGetWindowProperty(_ecore_x_disp, win, atom, 0, 0x7fffffff, False,
		      ECORE_X_ATOM_UTF8_STRING, &type_ret,
		      &format_ret, &num_ret, &bytes_after, &prop_return);
   if (prop_return && num_ret > 0 && format_ret == 8)
     {
	str = malloc(num_ret + 1);
	if (str)
	  {
	     memcpy(str, prop_return, num_ret);
	     str[num_ret] = '\0';
	  }
     }
   if (prop_return)
      XFree(prop_return);

   return str;
}

/*
 * Root window NetWM hints.
 */
Ecore_X_Atom        ECORE_X_ATOM_NET_SUPPORTED = 0;
Ecore_X_Atom        ECORE_X_ATOM_NET_SUPPORTING_WM_CHECK = 0;

Ecore_X_Atom        ECORE_X_ATOM_NET_NUMBER_OF_DESKTOPS = 0;
Ecore_X_Atom        ECORE_X_ATOM_NET_VIRTUAL_ROOTS = 0;
Ecore_X_Atom        ECORE_X_ATOM_NET_DESKTOP_NAMES = 0;
Ecore_X_Atom        ECORE_X_ATOM_NET_DESKTOP_GEOMETRY = 0;
Ecore_X_Atom        ECORE_X_ATOM_NET_DESKTOP_VIEWPORT = 0;
Ecore_X_Atom        ECORE_X_ATOM_NET_WORKAREA = 0;
Ecore_X_Atom        ECORE_X_ATOM_NET_DESKTOP_LAYOUT = 0;

Ecore_X_Atom        ECORE_X_ATOM_NET_CURRENT_DESKTOP = 0;
Ecore_X_Atom        ECORE_X_ATOM_NET_SHOWING_DESKTOP = 0;

Ecore_X_Atom        ECORE_X_ATOM_NET_CLIENT_LIST = 0;
Ecore_X_Atom        ECORE_X_ATOM_NET_CLIENT_LIST_STACKING = 0;
Ecore_X_Atom        ECORE_X_ATOM_NET_ACTIVE_WINDOW = 0;

/*
 * Client message types.
 */
Ecore_X_Atom        ECORE_X_ATOM_NET_CLOSE_WINDOW = 0;
Ecore_X_Atom        ECORE_X_ATOM_NET_WM_MOVERESIZE = 0;

/*
 * Application window specific NetWM hints.
 */
Ecore_X_Atom        ECORE_X_ATOM_NET_WM_DESKTOP = 0;
Ecore_X_Atom        ECORE_X_ATOM_NET_WM_NAME = 0;
Ecore_X_Atom        ECORE_X_ATOM_NET_WM_VISIBLE_NAME = 0;
Ecore_X_Atom        ECORE_X_ATOM_NET_WM_ICON_NAME = 0;
Ecore_X_Atom        ECORE_X_ATOM_NET_WM_VISIBLE_ICON_NAME = 0;
Ecore_X_Atom        ECORE_X_ATOM_NET_WM_WINDOW_TYPE = 0;
Ecore_X_Atom        ECORE_X_ATOM_NET_WM_STATE = 0;
Ecore_X_Atom        ECORE_X_ATOM_NET_WM_ALLOWED_ACTIONS = 0;
Ecore_X_Atom        ECORE_X_ATOM_NET_WM_STRUT = 0;
Ecore_X_Atom        ECORE_X_ATOM_NET_WM_STRUT_PARTIAL = 0;
Ecore_X_Atom        ECORE_X_ATOM_NET_WM_ICON_GEOMETRY = 0;
Ecore_X_Atom        ECORE_X_ATOM_NET_WM_ICON = 0;
Ecore_X_Atom        ECORE_X_ATOM_NET_WM_PID = 0;
Ecore_X_Atom        ECORE_X_ATOM_NET_WM_HANDLE_ICONS = 0;
Ecore_X_Atom        ECORE_X_ATOM_NET_WM_USER_TIME = 0;

Ecore_X_Atom        ECORE_X_ATOM_NET_WM_WINDOW_TYPE_DESKTOP = 0;
Ecore_X_Atom        ECORE_X_ATOM_NET_WM_WINDOW_TYPE_DOCK = 0;
Ecore_X_Atom        ECORE_X_ATOM_NET_WM_WINDOW_TYPE_TOOLBAR = 0;
Ecore_X_Atom        ECORE_X_ATOM_NET_WM_WINDOW_TYPE_MENU = 0;
Ecore_X_Atom        ECORE_X_ATOM_NET_WM_WINDOW_TYPE_UTILITY = 0;
Ecore_X_Atom        ECORE_X_ATOM_NET_WM_WINDOW_TYPE_SPLASH = 0;
Ecore_X_Atom        ECORE_X_ATOM_NET_WM_WINDOW_TYPE_DIALOG = 0;
Ecore_X_Atom        ECORE_X_ATOM_NET_WM_WINDOW_TYPE_NORMAL = 0;

Ecore_X_Atom        ECORE_X_ATOM_NET_WM_STATE_MODAL = 0;
Ecore_X_Atom        ECORE_X_ATOM_NET_WM_STATE_STICKY = 0;
Ecore_X_Atom        ECORE_X_ATOM_NET_WM_STATE_MAXIMIZED_VERT = 0;
Ecore_X_Atom        ECORE_X_ATOM_NET_WM_STATE_MAXIMIZED_HORZ = 0;
Ecore_X_Atom        ECORE_X_ATOM_NET_WM_STATE_SHADED = 0;
Ecore_X_Atom        ECORE_X_ATOM_NET_WM_STATE_SKIP_TASKBAR = 0;
Ecore_X_Atom        ECORE_X_ATOM_NET_WM_STATE_SKIP_PAGER = 0;
Ecore_X_Atom        ECORE_X_ATOM_NET_WM_STATE_HIDDEN = 0;
Ecore_X_Atom        ECORE_X_ATOM_NET_WM_STATE_FULLSCREEN = 0;
Ecore_X_Atom        ECORE_X_ATOM_NET_WM_STATE_ABOVE = 0;
Ecore_X_Atom        ECORE_X_ATOM_NET_WM_STATE_BELOW = 0;

Ecore_X_Atom        ECORE_X_ATOM_NET_WM_WINDOW_OPACITY = 0;

void
ecore_x_netwm_init(void)
{
   ECORE_X_ATOM_NET_SUPPORTED = _ATOM_GET("_NET_SUPPORTED");
   ECORE_X_ATOM_NET_SUPPORTING_WM_CHECK = _ATOM_GET("_NET_SUPPORTING_WM_CHECK");

   ECORE_X_ATOM_NET_NUMBER_OF_DESKTOPS = _ATOM_GET("_NET_NUMBER_OF_DESKTOPS");
   ECORE_X_ATOM_NET_VIRTUAL_ROOTS = _ATOM_GET("_NET_VIRTUAL_ROOTS");
   ECORE_X_ATOM_NET_DESKTOP_GEOMETRY = _ATOM_GET("_NET_DESKTOP_GEOMETRY");
   ECORE_X_ATOM_NET_DESKTOP_NAMES = _ATOM_GET("_NET_DESKTOP_NAMES");
   ECORE_X_ATOM_NET_CURRENT_DESKTOP = _ATOM_GET("_NET_CURRENT_DESKTOP");
   ECORE_X_ATOM_NET_DESKTOP_VIEWPORT = _ATOM_GET("_NET_DESKTOP_VIEWPORT");
   ECORE_X_ATOM_NET_WORKAREA = _ATOM_GET("_NET_WORKAREA");

   ECORE_X_ATOM_NET_CLIENT_LIST = _ATOM_GET("_NET_CLIENT_LIST");
   ECORE_X_ATOM_NET_CLIENT_LIST_STACKING =
      _ATOM_GET("_NET_CLIENT_LIST_STACKING");
   ECORE_X_ATOM_NET_ACTIVE_WINDOW = _ATOM_GET("_NET_ACTIVE_WINDOW");

   ECORE_X_ATOM_NET_CLOSE_WINDOW = _ATOM_GET("_NET_CLOSE_WINDOW");
   ECORE_X_ATOM_NET_WM_MOVERESIZE = _ATOM_GET("_NET_WM_MOVERESIZE");

   ECORE_X_ATOM_NET_WM_NAME = _ATOM_GET("_NET_WM_NAME");
   ECORE_X_ATOM_NET_WM_VISIBLE_NAME = _ATOM_GET("_NET_WM_VISIBLE_NAME");
   ECORE_X_ATOM_NET_WM_ICON_NAME = _ATOM_GET("_NET_WM_ICON_NAME");
   ECORE_X_ATOM_NET_WM_VISIBLE_ICON_NAME =
      _ATOM_GET("_NET_WM_VISIBLE_ICON_NAME");
   ECORE_X_ATOM_NET_WM_DESKTOP = _ATOM_GET("_NET_WM_DESKTOP");
   ECORE_X_ATOM_NET_WM_WINDOW_TYPE = _ATOM_GET("_NET_WM_WINDOW_TYPE");
   ECORE_X_ATOM_NET_WM_STATE = _ATOM_GET("_NET_WM_STATE");
   ECORE_X_ATOM_NET_WM_ALLOWED_ACTIONS = _ATOM_GET("_NET_WM_ALLOWED_ACTIONS");
   ECORE_X_ATOM_NET_WM_STRUT = _ATOM_GET("_NET_WM_STRUT");
   ECORE_X_ATOM_NET_WM_STRUT_PARTIAL = _ATOM_GET("_NET_WM_STRUT_PARTIAL");
   ECORE_X_ATOM_NET_WM_ICON_GEOMETRY = _ATOM_GET("_NET_WM_ICON_GEOMETRY");
   ECORE_X_ATOM_NET_WM_ICON = _ATOM_GET("_NET_WM_ICON");
   ECORE_X_ATOM_NET_WM_PID = _ATOM_GET("_NET_WM_PID");
   ECORE_X_ATOM_NET_WM_USER_TIME = _ATOM_GET("_NET_WM_USER_TIME");

   ECORE_X_ATOM_NET_WM_WINDOW_TYPE_DESKTOP =
      _ATOM_GET("_NET_WM_WINDOW_TYPE_DESKTOP");
   ECORE_X_ATOM_NET_WM_WINDOW_TYPE_DOCK = _ATOM_GET("_NET_WM_WINDOW_TYPE_DOCK");
   ECORE_X_ATOM_NET_WM_WINDOW_TYPE_TOOLBAR =
      _ATOM_GET("_NET_WM_WINDOW_TYPE_TOOLBAR");
   ECORE_X_ATOM_NET_WM_WINDOW_TYPE_MENU = _ATOM_GET("_NET_WM_WINDOW_TYPE_MENU");
   ECORE_X_ATOM_NET_WM_WINDOW_TYPE_UTILITY =
      _ATOM_GET("_NET_WM_WINDOW_TYPE_UTILITY");
   ECORE_X_ATOM_NET_WM_WINDOW_TYPE_SPLASH =
      _ATOM_GET("_NET_WM_WINDOW_TYPE_SPLASH");
   ECORE_X_ATOM_NET_WM_WINDOW_TYPE_DIALOG =
      _ATOM_GET("_NET_WM_WINDOW_TYPE_DIALOG");
   ECORE_X_ATOM_NET_WM_WINDOW_TYPE_NORMAL =
      _ATOM_GET("_NET_WM_WINDOW_TYPE_NORMAL");

   ECORE_X_ATOM_NET_WM_STATE_MODAL = _ATOM_GET("_NET_WM_STATE_MODAL");
   ECORE_X_ATOM_NET_WM_STATE_STICKY = _ATOM_GET("_NET_WM_STATE_STICKY");
   ECORE_X_ATOM_NET_WM_STATE_MAXIMIZED_VERT =
      _ATOM_GET("_NET_WM_STATE_MAXIMIZED_VERT");
   ECORE_X_ATOM_NET_WM_STATE_MAXIMIZED_HORZ =
      _ATOM_GET("_NET_WM_STATE_MAXIMIZED_HORZ");
   ECORE_X_ATOM_NET_WM_STATE_SHADED = _ATOM_GET("_NET_WM_STATE_SHADED");
   ECORE_X_ATOM_NET_WM_STATE_SKIP_TASKBAR =
      _ATOM_GET("_NET_WM_STATE_SKIP_TASKBAR");
   ECORE_X_ATOM_NET_WM_STATE_SKIP_PAGER = _ATOM_GET("_NET_WM_STATE_SKIP_PAGER");
   ECORE_X_ATOM_NET_WM_STATE_HIDDEN = _ATOM_GET("_NET_WM_STATE_HIDDEN");
   ECORE_X_ATOM_NET_WM_STATE_FULLSCREEN = _ATOM_GET("_NET_WM_STATE_FULLSCREEN");
   ECORE_X_ATOM_NET_WM_STATE_ABOVE = _ATOM_GET("_NET_WM_STATE_ABOVE");
   ECORE_X_ATOM_NET_WM_STATE_BELOW = _ATOM_GET("_NET_WM_STATE_BELOW");

   ECORE_X_ATOM_NET_WM_WINDOW_OPACITY = _ATOM_GET("_NET_WM_WINDOW_OPACITY");
}

/*
 * WM identification
 */
void
ecore_x_netwm_wm_identify(Ecore_X_Window root, Ecore_X_Window check,
			  const char *wm_name)
{
   _ATOM_SET_WINDOW(ECORE_X_ATOM_NET_SUPPORTING_WM_CHECK, root, &check, 1);
   _ATOM_SET_WINDOW(ECORE_X_ATOM_NET_SUPPORTING_WM_CHECK, check, &check, 1);
   _ATOM_SET_UTF8_STRING(ECORE_X_ATOM_NET_WM_NAME, check, wm_name);
   /* This one isn't mandatory */
   _ATOM_SET_UTF8_STRING(ECORE_X_ATOM_NET_WM_NAME, root, wm_name);
}

/*
 * Desktop configuration and status
 */

void
ecore_x_netwm_desk_count_set(Ecore_X_Window root, int n_desks)
{
   CARD32              val;

   val = n_desks;
   _ATOM_SET_CARD32(ECORE_X_ATOM_NET_NUMBER_OF_DESKTOPS, root, &val, 1);
}

void
ecore_x_netwm_desk_roots_set(Ecore_X_Window root, int n_desks,
			     Ecore_X_Window * vroots)
{
   _ATOM_SET_WINDOW(ECORE_X_ATOM_NET_VIRTUAL_ROOTS, root, vroots, n_desks);
}

void
ecore_x_netwm_desk_names_set(Ecore_X_Window root, int n_desks,
			     const char **names)
{
   char                ss[32], *buf;
   const char         *s;
   int                 i, l, len;

   buf = NULL;
   len = 0;

   for (i = 0; i < n_desks; i++)
     {
	s = (names) ? names[i] : NULL;
	if (!s)
	  {
	     /* Default to "Desk-<number>" */
	     sprintf(ss, "Desk-%d", i);
	     s = ss;
	  }

	l = strlen(s) + 1;
	buf = realloc(buf, len + l);
	memcpy(buf + len, s, l);
	len += l;
     }

   _ATOM_SET_UTF8_STRING_LIST(ECORE_X_ATOM_NET_DESKTOP_NAMES, root, buf, len);

   free(buf);
}

void
ecore_x_netwm_desk_size_set(Ecore_X_Window root, int width, int height)
{
   CARD32              size[2];

   size[0] = width;
   size[1] = height;
   _ATOM_SET_CARD32(ECORE_X_ATOM_NET_DESKTOP_GEOMETRY, root, &size, 2);
}

void
ecore_x_netwm_desk_workareas_set(Ecore_X_Window root, int n_desks, int *areas)
{
   CARD32             *p_coord;
   int                 n_coord, i;

   n_coord = 4 * n_desks;
   p_coord = malloc(n_coord * sizeof(CARD32));
   if (!p_coord)
      return;

   for (i = 0; i < n_coord; i++)
      p_coord[i] = areas[i];

   _ATOM_SET_CARD32(ECORE_X_ATOM_NET_WORKAREA, root, p_coord, n_coord);

   free(p_coord);
}

void
ecore_x_netwm_desk_current_set(Ecore_X_Window root, int desk)
{
   CARD32              val;

   val = desk;
   _ATOM_SET_CARD32(ECORE_X_ATOM_NET_CURRENT_DESKTOP, root, &val, 1);
}

void
ecore_x_netwm_desk_viewports_set(Ecore_X_Window root, int n_desks, int *origins)
{
   CARD32             *p_coord;
   int                 n_coord, i;

   n_coord = 2 * n_desks;
   p_coord = malloc(n_coord * sizeof(CARD32));
   if (!p_coord)
      return;

   for (i = 0; i < n_coord; i++)
      p_coord[i] = origins[i];

   _ATOM_SET_CARD32(ECORE_X_ATOM_NET_DESKTOP_VIEWPORT, root, p_coord, n_coord);

   free(p_coord);
}

void
ecore_x_netwm_showing_desktop_set(Ecore_X_Window root, int on)
{
   CARD32              val;

   val = on;
   _ATOM_SET_CARD32(ECORE_X_ATOM_NET_SHOWING_DESKTOP, root, &val, 1);
}

/*
 * Client status
 */

/* Mapping order */
void
ecore_x_netwm_client_list_set(Ecore_X_Window root, int n_clients,
			      Ecore_X_Window * p_clients)
{
   _ATOM_SET_WINDOW(ECORE_X_ATOM_NET_CLIENT_LIST, root, p_clients, n_clients);
}

/* Stacking order */
void
ecore_x_netwm_client_list_stacking_set(Ecore_X_Window root, int n_clients,
				       Ecore_X_Window * p_clients)
{
   _ATOM_SET_WINDOW(ECORE_X_ATOM_NET_CLIENT_LIST_STACKING, root, p_clients,
		    n_clients);
}

void
ecore_x_netwm_client_active_set(Ecore_X_Window root, Ecore_X_Window win)
{
   _ATOM_SET_WINDOW(ECORE_X_ATOM_NET_ACTIVE_WINDOW, root, &win, 1);
}

void
ecore_x_netwm_name_set(Ecore_X_Window win, const char *name)
{
   _ecore_x_window_prop_string_utf8_set(ECORE_X_ATOM_NET_WM_NAME, win, name);
}

char               *
ecore_x_netwm_name_get(Ecore_X_Window win)
{
   return _ecore_x_window_prop_string_utf8_get(win, ECORE_X_ATOM_NET_WM_NAME);
}

void
ecore_x_netwm_icon_name_set(Ecore_X_Window win, const char *name)
{
   _ecore_x_window_prop_string_utf8_set(ECORE_X_ATOM_NET_WM_ICON_NAME, win,
					name);
}

char               *
ecore_x_netwm_icon_name_get(Ecore_X_Window win)
{
   return _ecore_x_window_prop_string_utf8_get(win,
					       ECORE_X_ATOM_NET_WM_ICON_NAME);
}

void
ecore_x_netwm_visible_name_set(Ecore_X_Window win, const char *name)
{
   _ecore_x_window_prop_string_utf8_set(ECORE_X_ATOM_NET_WM_VISIBLE_NAME, win,
					name);
}

char               *
ecore_x_netwm_visible_name_get(Ecore_X_Window win)
{
   return _ecore_x_window_prop_string_utf8_get(win,
					       ECORE_X_ATOM_NET_WM_VISIBLE_NAME);
}

void
ecore_x_netwm_visible_icon_name_set(Ecore_X_Window win, const char *name)
{
   _ecore_x_window_prop_string_utf8_set(ECORE_X_ATOM_NET_WM_VISIBLE_ICON_NAME,
					win, name);
}

char               *
ecore_x_netwm_visible_icon_name_get(Ecore_X_Window win)
{
   return _ecore_x_window_prop_string_utf8_get(win,
					       ECORE_X_ATOM_NET_WM_VISIBLE_ICON_NAME);
}
