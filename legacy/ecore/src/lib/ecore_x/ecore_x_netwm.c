/*
 * _NET_WM... aka Extended Window Manager Hint (EWMH) functions.
 */
#include "Ecore.h"
#include "ecore_x_private.h"
#include "Ecore_X.h"

/*
 * Convenience macros
 */
#define _ATOM_GET(name) \
   XInternAtom(_ecore_x_disp, name, False)

#define _ATOM_SET_UTF8_STRING(atom, win, string) \
   XChangeProperty(_ecore_x_disp, win, atom, _ecore_x_atom_utf8_string, 8, PropModeReplace, \
                   (unsigned char *)string, strlen(string))
#define _ATOM_SET_UTF8_STRING_LIST(atom, win, string, cnt) \
   XChangeProperty(_ecore_x_disp, win, atom, _ecore_x_atom_utf8_string, 8, PropModeReplace, \
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
_ecore_x_window_prop_string_utf8_set(Ecore_X_Window win, Ecore_X_Atom atom, const char *str)
{
   _ATOM_SET_UTF8_STRING(win, atom, str);
}

/*
 * Get UTF-8 string property
 */
static char *
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
		      _ecore_x_atom_utf8_string, &type_ret,
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
Atom     _ecore_x_atom_net_supported = 0;
Atom     _ecore_x_atom_net_supporting_wm_check = 0;

Atom     _ecore_x_atom_net_number_of_desktops = 0;
Atom     _ecore_x_atom_net_virtual_roots = 0;
Atom     _ecore_x_atom_net_desktop_names = 0;
Atom     _ecore_x_atom_net_desktop_geometry = 0;
Atom     _ecore_x_atom_net_desktop_viewport = 0;
Atom     _ecore_x_atom_net_workarea = 0;
Atom     _ecore_x_atom_net_desktop_layout = 0;

Atom     _ecore_x_atom_net_current_desktop = 0;
Atom     _ecore_x_atom_net_showing_desktop = 0;

Atom     _ecore_x_atom_net_client_list = 0;
Atom     _ecore_x_atom_net_client_list_stacking = 0;
Atom     _ecore_x_atom_net_active_window = 0;

/*
 * Client message types.
 */
Atom     _ecore_x_atom_net_close_window = 0;
Atom     _ecore_x_atom_net_wm_moveresize = 0;

/*
 * Application window specific NetWM hints.
 */
Atom     _ecore_x_atom_net_wm_desktop = 0;
Atom     _ecore_x_atom_net_wm_name = 0;
Atom     _ecore_x_atom_net_wm_visible_name = 0;
Atom     _ecore_x_atom_net_wm_icon_name = 0;
Atom     _ecore_x_atom_net_wm_visible_icon_name = 0;
Atom     _ecore_x_atom_net_wm_window_type = 0;
Atom     _ecore_x_atom_net_wm_state = 0;
Atom     _ecore_x_atom_net_wm_allowed_actions = 0;
Atom     _ecore_x_atom_net_wm_strut = 0;
Atom     _ecore_x_atom_net_wm_strut_partial = 0;
Atom     _ecore_x_atom_net_wm_icon_geometry = 0;
Atom     _ecore_x_atom_net_wm_icon = 0;
Atom     _ecore_x_atom_net_wm_pid = 0;
Atom     _ecore_x_atom_net_wm_handle_icons = 0;
Atom     _ecore_x_atom_net_wm_user_time = 0;

Atom     _ecore_x_atom_net_wm_window_type_desktop = 0;
Atom     _ecore_x_atom_net_wm_window_type_dock = 0;
Atom     _ecore_x_atom_net_wm_window_type_toolbar = 0;
Atom     _ecore_x_atom_net_wm_window_type_menu = 0;
Atom     _ecore_x_atom_net_wm_window_type_utility = 0;
Atom     _ecore_x_atom_net_wm_window_type_splash = 0;
Atom     _ecore_x_atom_net_wm_window_type_dialog = 0;
Atom     _ecore_x_atom_net_wm_window_type_normal = 0;

Atom     _ecore_x_atom_net_wm_state_modal = 0;
Atom     _ecore_x_atom_net_wm_state_sticky = 0;
Atom     _ecore_x_atom_net_wm_state_maximized_vert = 0;
Atom     _ecore_x_atom_net_wm_state_maximized_horz = 0;
Atom     _ecore_x_atom_net_wm_state_shaded = 0;
Atom     _ecore_x_atom_net_wm_state_skip_taskbar = 0;
Atom     _ecore_x_atom_net_wm_state_skip_pager = 0;
Atom     _ecore_x_atom_net_wm_state_hidden = 0;
Atom     _ecore_x_atom_net_wm_state_fullscreen = 0;
Atom     _ecore_x_atom_net_wm_state_above = 0;
Atom     _ecore_x_atom_net_wm_state_below = 0;

Atom     _ecore_x_atom_net_wm_window_opacity = 0;

void
ecore_x_netwm_init(void)
{
   _ecore_x_atom_net_supported            = _ATOM_GET("_NET_SUPPORTED");
   _ecore_x_atom_net_supporting_wm_check  = _ATOM_GET("_NET_SUPPORTING_WM_CHECK");

   _ecore_x_atom_net_number_of_desktops   = _ATOM_GET("_NET_NUMBER_OF_DESKTOPS");
   _ecore_x_atom_net_virtual_roots        = _ATOM_GET("_NET_VIRTUAL_ROOTS");
   _ecore_x_atom_net_desktop_geometry     = _ATOM_GET("_NET_DESKTOP_GEOMETRY");
   _ecore_x_atom_net_desktop_names        = _ATOM_GET("_NET_DESKTOP_NAMES");
   _ecore_x_atom_net_current_desktop      = _ATOM_GET("_NET_CURRENT_DESKTOP");
   _ecore_x_atom_net_desktop_viewport     = _ATOM_GET("_NET_DESKTOP_VIEWPORT");
   _ecore_x_atom_net_workarea             = _ATOM_GET("_NET_WORKAREA");

   _ecore_x_atom_net_client_list          = _ATOM_GET("_NET_CLIENT_LIST");
   _ecore_x_atom_net_client_list_stacking = _ATOM_GET("_NET_CLIENT_LIST_STACKING");
   _ecore_x_atom_net_active_window        = _ATOM_GET("_NET_ACTIVE_WINDOW");

   _ecore_x_atom_net_close_window         = _ATOM_GET("_NET_CLOSE_WINDOW");
   _ecore_x_atom_net_wm_moveresize        = _ATOM_GET("_NET_WM_MOVERESIZE");

   _ecore_x_atom_net_wm_name              = _ATOM_GET("_NET_WM_NAME");
   _ecore_x_atom_net_wm_visible_name      = _ATOM_GET("_NET_WM_VISIBLE_NAME");
   _ecore_x_atom_net_wm_icon_name         = _ATOM_GET("_NET_WM_ICON_NAME");
   _ecore_x_atom_net_wm_visible_icon_name = _ATOM_GET("_NET_WM_VISIBLE_ICON_NAME");
   _ecore_x_atom_net_wm_desktop           = _ATOM_GET("_NET_WM_DESKTOP");
   _ecore_x_atom_net_wm_window_type       = _ATOM_GET("_NET_WM_WINDOW_TYPE");
   _ecore_x_atom_net_wm_state             = _ATOM_GET("_NET_WM_STATE");
   _ecore_x_atom_net_wm_allowed_actions   = _ATOM_GET("_NET_WM_ALLOWED_ACTIONS");
   _ecore_x_atom_net_wm_strut             = _ATOM_GET("_NET_WM_STRUT");
   _ecore_x_atom_net_wm_strut_partial     = _ATOM_GET("_NET_WM_STRUT_PARTIAL");
   _ecore_x_atom_net_wm_icon_geometry     = _ATOM_GET("_NET_WM_ICON_GEOMETRY");
   _ecore_x_atom_net_wm_icon              = _ATOM_GET("_NET_WM_ICON");
   _ecore_x_atom_net_wm_pid               = _ATOM_GET("_NET_WM_PID");
   _ecore_x_atom_net_wm_user_time         = _ATOM_GET("_NET_WM_USER_TIME");

   _ecore_x_atom_net_wm_window_type_desktop = _ATOM_GET("_NET_WM_WINDOW_TYPE_DESKTOP");
   _ecore_x_atom_net_wm_window_type_dock    = _ATOM_GET("_NET_WM_WINDOW_TYPE_DOCK");
   _ecore_x_atom_net_wm_window_type_toolbar = _ATOM_GET("_NET_WM_WINDOW_TYPE_TOOLBAR");
   _ecore_x_atom_net_wm_window_type_menu    = _ATOM_GET("_NET_WM_WINDOW_TYPE_MENU");
   _ecore_x_atom_net_wm_window_type_utility = _ATOM_GET("_NET_WM_WINDOW_TYPE_UTILITY");
   _ecore_x_atom_net_wm_window_type_splash  = _ATOM_GET("_NET_WM_WINDOW_TYPE_SPLASH");
   _ecore_x_atom_net_wm_window_type_dialog  = _ATOM_GET("_NET_WM_WINDOW_TYPE_DIALOG");
   _ecore_x_atom_net_wm_window_type_normal  = _ATOM_GET("_NET_WM_WINDOW_TYPE_NORMAL");

   _ecore_x_atom_net_wm_state_modal          = _ATOM_GET("_NET_WM_STATE_MODAL");
   _ecore_x_atom_net_wm_state_sticky         = _ATOM_GET("_NET_WM_STATE_STICKY");
   _ecore_x_atom_net_wm_state_maximized_vert = _ATOM_GET("_NET_WM_STATE_MAXIMIZED_VERT");
   _ecore_x_atom_net_wm_state_maximized_horz = _ATOM_GET("_NET_WM_STATE_MAXIMIZED_HORZ");
   _ecore_x_atom_net_wm_state_shaded         = _ATOM_GET("_NET_WM_STATE_SHADED");
   _ecore_x_atom_net_wm_state_skip_taskbar   = _ATOM_GET("_NET_WM_STATE_SKIP_TASKBAR");
   _ecore_x_atom_net_wm_state_skip_pager     = _ATOM_GET("_NET_WM_STATE_SKIP_PAGER");
   _ecore_x_atom_net_wm_state_hidden         = _ATOM_GET("_NET_WM_STATE_HIDDEN");
   _ecore_x_atom_net_wm_state_fullscreen     = _ATOM_GET("_NET_WM_STATE_FULLSCREEN");
   _ecore_x_atom_net_wm_state_above          = _ATOM_GET("_NET_WM_STATE_ABOVE");
   _ecore_x_atom_net_wm_state_below          = _ATOM_GET("_NET_WM_STATE_BELOW");

   _ecore_x_atom_net_wm_window_opacity       = _ATOM_GET("_NET_WM_WINDOW_OPACITY");
}

/*
 * WM identification
 */
void
ecore_x_netwm_wm_identify(Ecore_X_Window root, Ecore_X_Window check,
                          const char *wm_name)
{
   _ATOM_SET_WINDOW(_ecore_x_atom_net_supporting_wm_check, root, &check, 1);
   _ATOM_SET_WINDOW(_ecore_x_atom_net_supporting_wm_check, check, &check, 1);
   _ATOM_SET_UTF8_STRING(_ecore_x_atom_net_wm_name, check, wm_name);
   /* This one isn't mandatory */
   _ATOM_SET_UTF8_STRING(_ecore_x_atom_net_wm_name, root, wm_name);
}  


/*
 * Desktop configuration and status
 */

void
ecore_x_netwm_desk_count_set(Ecore_X_Window root, int n_desks)
{
   CARD32              val;

   val = n_desks;
   _ATOM_SET_CARD32(_ecore_x_atom_net_number_of_desktops, root, &val, 1);
}

void
ecore_x_netwm_desk_roots_set(Ecore_X_Window root, int n_desks,
			     Ecore_X_Window * vroots)
{
   _ATOM_SET_WINDOW(_ecore_x_atom_net_virtual_roots, root, vroots, n_desks);
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

   _ATOM_SET_UTF8_STRING_LIST(_ecore_x_atom_net_desktop_names, root, buf, len);

   free(buf);
}

void
ecore_x_netwm_desk_size_set(Ecore_X_Window root, int width, int height)
{
   CARD32              size[2];

   size[0] = width;
   size[1] = height;
   _ATOM_SET_CARD32(_ecore_x_atom_net_desktop_geometry, root, &size, 2);
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

   _ATOM_SET_CARD32(_ecore_x_atom_net_workarea, root, p_coord, n_coord);

   free(p_coord);
}

void
ecore_x_netwm_desk_current_set(Ecore_X_Window root, int desk)
{
   CARD32              val;

   val = desk;
   _ATOM_SET_CARD32(_ecore_x_atom_net_current_desktop, root, &val, 1);
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

   _ATOM_SET_CARD32(_ecore_x_atom_net_desktop_viewport, root, p_coord, n_coord);

   free(p_coord);
}

void
ecore_x_netwm_showing_desktop_set(Ecore_X_Window root, int on)
{
   CARD32              val;

   val = on;
   _ATOM_SET_CARD32(_ecore_x_atom_net_showing_desktop, root, &val, 1);
}

/*
 * Client status
 */

/* Mapping order */
void
ecore_x_netwm_client_list_set(Ecore_X_Window root, int n_clients,
			      Ecore_X_Window * p_clients)
{
   _ATOM_SET_WINDOW(_ecore_x_atom_net_client_list, root, p_clients, n_clients);
}

/* Stacking order */
void
ecore_x_netwm_client_list_stacking_set(Ecore_X_Window root, int n_clients,
				       Ecore_X_Window * p_clients)
{
   _ATOM_SET_WINDOW(_ecore_x_atom_net_client_list_stacking, root, p_clients,
		    n_clients);
}

void
ecore_x_netwm_client_active_set(Ecore_X_Window root, Ecore_X_Window win)
{
   _ATOM_SET_WINDOW(_ecore_x_atom_net_active_window, root, &win, 1);
}

void
ecore_x_netwm_name_set(Ecore_X_Window win, const char *name)
{
   _ecore_x_window_prop_string_utf8_set(_ecore_x_atom_net_wm_name, win, name);
}

char *
ecore_x_netwm_name_get(Ecore_X_Window win)
{
   return _ecore_x_window_prop_string_utf8_get(win, _ecore_x_atom_net_wm_name);
}

void
ecore_x_netwm_icon_name_set(Ecore_X_Window win, const char *name)
{  
   _ecore_x_window_prop_string_utf8_set(_ecore_x_atom_net_wm_icon_name, win, name);
}  
   
char               *
ecore_x_netwm_icon_name_get(Ecore_X_Window win) 
{
   return _ecore_x_window_prop_string_utf8_get(win, _ecore_x_atom_net_wm_icon_name);
}

void
ecore_x_netwm_visible_name_set(Ecore_X_Window win, const char *name)
{                                    
   _ecore_x_window_prop_string_utf8_set(_ecore_x_atom_net_wm_visible_name, win, name);
}

char               *
ecore_x_netwm_visible_name_get(Ecore_X_Window win)
{
   return _ecore_x_window_prop_string_utf8_get(win, _ecore_x_atom_net_wm_visible_name);
}
   
void
ecore_x_netwm_visible_icon_name_set(Ecore_X_Window win, const char *name)
{  
   _ecore_x_window_prop_string_utf8_set(_ecore_x_atom_net_wm_visible_icon_name, win, name);
}  
   
char               *
ecore_x_netwm_visible_icon_name_get(Ecore_X_Window win)
{                     
   return _ecore_x_window_prop_string_utf8_get(win, _ecore_x_atom_net_wm_visible_icon_name);
}       
