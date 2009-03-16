/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

/*
 * _NET_WM... aka Extended Window Manager Hint (EWMH) functions.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <string.h>

#include "Ecore.h"
#include "Ecore_Data.h"
#include "ecore_x_private.h"
#include "Ecore_X.h"
#include "Ecore_X_Atoms.h"

typedef struct _Ecore_X_Startup_Info Ecore_X_Startup_Info;

struct _Ecore_X_Startup_Info
{
   Ecore_X_Window win;

   int   init;

   int   buffer_size;
   char *buffer;

   int   length;

   /* These are the sequence info fields */
   char *id;
   char *name;
   int   screen;
   char *bin;
   char *icon;
   int   desktop;
   int   timestamp;
   char *description;
   char *wmclass;
   int   silent;
};

static void  _ecore_x_window_prop_string_utf8_set(Ecore_X_Window win, Ecore_X_Atom atom, const char *str);
static char *_ecore_x_window_prop_string_utf8_get(Ecore_X_Window win, Ecore_X_Atom atom);
#if 0 /* Unused */
static int   _ecore_x_netwm_startup_info_process(Ecore_X_Startup_Info *info);
static int   _ecore_x_netwm_startup_info_parse(Ecore_X_Startup_Info *info, char *data);
#endif
static void  _ecore_x_netwm_startup_info_free(void *data);

/*
 * Convenience macros
 */
#define _ATOM_SET_UTF8_STRING_LIST(win, atom, string, cnt) \
   XChangeProperty(_ecore_x_disp, win, atom, ECORE_X_ATOM_UTF8_STRING, 8, PropModeReplace, \
                   (unsigned char *)string, cnt)

/*
 * Local variables
 */

static Eina_Hash *startup_info = NULL;

EAPI void
ecore_x_netwm_init(void)
{
   startup_info = eina_hash_string_superfast_new(_ecore_x_netwm_startup_info_free);
}

EAPI void
ecore_x_netwm_shutdown(void)
{
   if (startup_info)
     eina_hash_free(startup_info);
   startup_info = NULL;
}

/*
 * WM identification
 */
EAPI void
ecore_x_netwm_wm_identify(Ecore_X_Window root, Ecore_X_Window check,
			  const char *wm_name)
{
   ecore_x_window_prop_window_set(root, ECORE_X_ATOM_NET_SUPPORTING_WM_CHECK, &check, 1);
   ecore_x_window_prop_window_set(check, ECORE_X_ATOM_NET_SUPPORTING_WM_CHECK, &check, 1);
   _ecore_x_window_prop_string_utf8_set(check, ECORE_X_ATOM_NET_WM_NAME, wm_name);
   /* This one isn't mandatory */
   _ecore_x_window_prop_string_utf8_set(root, ECORE_X_ATOM_NET_WM_NAME, wm_name);
}

/*
 * Set supported atoms
 */
EAPI void
ecore_x_netwm_supported_set(Ecore_X_Window root, Ecore_X_Atom *supported, int num)
{
   ecore_x_window_prop_atom_set(root, ECORE_X_ATOM_NET_SUPPORTED, supported, num);
}

EAPI int
ecore_x_netwm_supported_get(Ecore_X_Window root, Ecore_X_Atom **supported, int *num)
{
   int            num_ret;

   if (num) *num = 0;
   if (supported) *supported = NULL;

   num_ret = ecore_x_window_prop_atom_list_get(root, ECORE_X_ATOM_NET_SUPPORTED,
					       supported);
   if (num_ret <= 0)
      return 0;

   if (num) *num = num_ret;
   return 1;
}

/*
 * Desktop configuration and status
 */
EAPI void
ecore_x_netwm_desk_count_set(Ecore_X_Window root, unsigned int n_desks)
{
   ecore_x_window_prop_card32_set(root, ECORE_X_ATOM_NET_NUMBER_OF_DESKTOPS,
				  &n_desks, 1);
}

EAPI void
ecore_x_netwm_desk_roots_set(Ecore_X_Window root,
			     Ecore_X_Window *vroots, unsigned int n_desks)
{
   ecore_x_window_prop_window_set(root, ECORE_X_ATOM_NET_VIRTUAL_ROOTS, vroots, n_desks);
}

EAPI void
ecore_x_netwm_desk_names_set(Ecore_X_Window root,
			     const char **names, unsigned int n_desks)
{
   char                ss[32], *buf;
   const char         *s;
   unsigned int        i;
   int                 l, len;

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

   _ATOM_SET_UTF8_STRING_LIST(root, ECORE_X_ATOM_NET_DESKTOP_NAMES, buf, len);

   free(buf);
}

EAPI void
ecore_x_netwm_desk_size_set(Ecore_X_Window root, unsigned int width,
			    unsigned int height)
{
   unsigned int        size[2];

   size[0] = width;
   size[1] = height;
   ecore_x_window_prop_card32_set(root, ECORE_X_ATOM_NET_DESKTOP_GEOMETRY, size,
				  2);
}

EAPI void
ecore_x_netwm_desk_viewports_set(Ecore_X_Window root,
				 unsigned int *origins, unsigned int n_desks)
{
   ecore_x_window_prop_card32_set(root, ECORE_X_ATOM_NET_DESKTOP_VIEWPORT,
				  origins, 2 * n_desks);
}

EAPI void
ecore_x_netwm_desk_layout_set(Ecore_X_Window root, int orientation,
			      int columns, int rows,
			      int starting_corner)
{
   unsigned int layout[4];

   layout[0] = orientation;
   layout[1] = columns;
   layout[2] = rows;
   layout[3] = starting_corner;
   ecore_x_window_prop_card32_set(root, ECORE_X_ATOM_NET_DESKTOP_LAYOUT,
				  layout, 4);
}

EAPI void
ecore_x_netwm_desk_workareas_set(Ecore_X_Window root,
				 unsigned int *areas, unsigned int n_desks)
{
   ecore_x_window_prop_card32_set(root, ECORE_X_ATOM_NET_WORKAREA, areas,
				  4 * n_desks);
}

EAPI void
ecore_x_netwm_desk_current_set(Ecore_X_Window root, unsigned int desk)
{
   ecore_x_window_prop_card32_set(root, ECORE_X_ATOM_NET_CURRENT_DESKTOP, &desk,
				  1);
}

EAPI void
ecore_x_netwm_showing_desktop_set(Ecore_X_Window root, int on)
{
   unsigned int val;

   val = (on) ? 1 : 0;
   ecore_x_window_prop_card32_set(root, ECORE_X_ATOM_NET_SHOWING_DESKTOP, &val,
				  1);
}

/*
 * Client status
 */

/* Mapping order */
EAPI void
ecore_x_netwm_client_list_set(Ecore_X_Window root,
			      Ecore_X_Window *p_clients, unsigned int n_clients)
{
   ecore_x_window_prop_window_set(root, ECORE_X_ATOM_NET_CLIENT_LIST,
				  p_clients, n_clients);
}

/* Stacking order */
EAPI void
ecore_x_netwm_client_list_stacking_set(Ecore_X_Window root,
				       Ecore_X_Window *p_clients,
				       unsigned int n_clients)
{
   ecore_x_window_prop_window_set(root, ECORE_X_ATOM_NET_CLIENT_LIST_STACKING,
				  p_clients, n_clients);
}

EAPI void
ecore_x_netwm_client_active_set(Ecore_X_Window root, Ecore_X_Window win)
{
   ecore_x_window_prop_window_set(root, ECORE_X_ATOM_NET_ACTIVE_WINDOW,
				  &win, 1);
}

EAPI void
ecore_x_netwm_client_active_request(Ecore_X_Window root, Ecore_X_Window win, int type, Ecore_X_Window current_win)
{
   XEvent xev;

   if (!root) root = DefaultRootWindow(_ecore_x_disp);
   
   xev.xclient.type = ClientMessage;
   xev.xclient.display = _ecore_x_disp;
   xev.xclient.window = win;
   xev.xclient.message_type = ECORE_X_ATOM_NET_ACTIVE_WINDOW;
   xev.xclient.format = 32;
   xev.xclient.data.l[0] = type;
   xev.xclient.data.l[1] = CurrentTime;
   xev.xclient.data.l[2] = current_win;
   xev.xclient.data.l[3] = 0;
   xev.xclient.data.l[4] = 0;
   xev.xclient.data.l[5] = 0;

   XSendEvent(_ecore_x_disp, root, False, 
	      SubstructureRedirectMask | SubstructureNotifyMask, &xev);
}

EAPI void
ecore_x_netwm_name_set(Ecore_X_Window win, const char *name)
{
   _ecore_x_window_prop_string_utf8_set(win, ECORE_X_ATOM_NET_WM_NAME, name);
}

EAPI int
ecore_x_netwm_name_get(Ecore_X_Window win, char **name)
{
   if (name)
     *name = _ecore_x_window_prop_string_utf8_get(win, ECORE_X_ATOM_NET_WM_NAME);
   return 1;
}

EAPI void
ecore_x_netwm_startup_id_set(Ecore_X_Window win, const char *id)
{
   _ecore_x_window_prop_string_utf8_set(win, ECORE_X_ATOM_NET_STARTUP_ID, id);
}

EAPI int
ecore_x_netwm_startup_id_get(Ecore_X_Window win, char **id)
{
   if (id)
     *id = _ecore_x_window_prop_string_utf8_get(win, ECORE_X_ATOM_NET_STARTUP_ID);
   return 1;
}

EAPI void
ecore_x_netwm_visible_name_set(Ecore_X_Window win, const char *name)
{
   _ecore_x_window_prop_string_utf8_set(win, ECORE_X_ATOM_NET_WM_VISIBLE_NAME,
					name);
}

EAPI int
ecore_x_netwm_visible_name_get(Ecore_X_Window win, char **name)
{
   if (name)
     *name = _ecore_x_window_prop_string_utf8_get(win,
						  ECORE_X_ATOM_NET_WM_VISIBLE_NAME);
   return 1;
}

EAPI void
ecore_x_netwm_icon_name_set(Ecore_X_Window win, const char *name)
{
   _ecore_x_window_prop_string_utf8_set(win, ECORE_X_ATOM_NET_WM_ICON_NAME,
					name);
}

EAPI int
ecore_x_netwm_icon_name_get(Ecore_X_Window win, char **name)
{
   if (name)
     *name = _ecore_x_window_prop_string_utf8_get(win,
						  ECORE_X_ATOM_NET_WM_ICON_NAME);
   return 1;
}

EAPI void
ecore_x_netwm_visible_icon_name_set(Ecore_X_Window win, const char *name)
{
   _ecore_x_window_prop_string_utf8_set(win,
					ECORE_X_ATOM_NET_WM_VISIBLE_ICON_NAME,
					name);
}

EAPI int
ecore_x_netwm_visible_icon_name_get(Ecore_X_Window win, char **name)
{
   if (name)
     *name = _ecore_x_window_prop_string_utf8_get(win,
						  ECORE_X_ATOM_NET_WM_VISIBLE_ICON_NAME);
   return 1;
}

EAPI void
ecore_x_netwm_desktop_set(Ecore_X_Window win, unsigned int desk)
{
   ecore_x_window_prop_card32_set(win, ECORE_X_ATOM_NET_WM_DESKTOP, &desk, 1);
}

EAPI int
ecore_x_netwm_desktop_get(Ecore_X_Window win, unsigned int *desk)
{
   int ret;
   unsigned int tmp;

   ret = ecore_x_window_prop_card32_get(win, ECORE_X_ATOM_NET_WM_DESKTOP,
					&tmp, 1);

   if (desk) *desk = tmp;
   return ret == 1 ? 1 : 0;
}

/*
 * _NET_WM_STRUT is deprecated
 */
EAPI void
ecore_x_netwm_strut_set(Ecore_X_Window win, int left, int right,
			int top, int bottom)
{
   unsigned int strut[4];

   strut[0] = left;
   strut[1] = right;
   strut[2] = top;
   strut[3] = bottom;
   ecore_x_window_prop_card32_set(win, ECORE_X_ATOM_NET_WM_STRUT, strut, 4);
}

/*
 * _NET_WM_STRUT is deprecated
 */
EAPI int
ecore_x_netwm_strut_get(Ecore_X_Window win, int *left, int *right,
			int *top, int *bottom)
{
   int ret = 0;
   unsigned int strut[4];

   ret = ecore_x_window_prop_card32_get(win, ECORE_X_ATOM_NET_WM_STRUT, strut, 4);
   if (ret != 4)
     return 0;

   if (left) *left = strut[0];
   if (right) *right = strut[1];
   if (top) *top = strut[2];
   if (bottom) *bottom = strut[3];
   return 1;
}

EAPI void
ecore_x_netwm_strut_partial_set(Ecore_X_Window win, int left, int right,
				int top, int bottom, int left_start_y, int left_end_y,
				int right_start_y, int right_end_y, int top_start_x,
				int top_end_x, int bottom_start_x, int bottom_end_x)
{
   unsigned int strut[12];

   strut[0] = left;
   strut[1] = right;
   strut[2] = top;
   strut[3] = bottom;
   strut[4] = left_start_y;
   strut[5] = left_end_y;
   strut[6] = right_start_y;
   strut[7] = right_end_y;
   strut[8] = top_start_x;
   strut[9] = top_end_x;
   strut[10] = bottom_start_x;
   strut[11] = bottom_end_x;
   ecore_x_window_prop_card32_set(win, ECORE_X_ATOM_NET_WM_STRUT_PARTIAL, strut, 12);
}

EAPI int
ecore_x_netwm_strut_partial_get(Ecore_X_Window win, int *left, int *right,
				int *top, int *bottom, int *left_start_y, int *left_end_y,
				int *right_start_y, int *right_end_y, int *top_start_x,
				int *top_end_x, int *bottom_start_x, int *bottom_end_x)
{
   int ret = 0;
   unsigned int strut[12];

   ret = ecore_x_window_prop_card32_get(win, ECORE_X_ATOM_NET_WM_STRUT_PARTIAL, strut, 12);
   if (ret != 12)
     return 0;

   if (left) *left = strut[0];
   if (right) *right = strut[1];
   if (top) *top = strut[2];
   if (bottom) *bottom = strut[3];
   if (left_start_y) *left_start_y = strut[4];
   if (left_end_y) *left_end_y = strut[5];
   if (right_start_y) *right_start_y = strut[6];
   if (right_end_y) *right_end_y = strut[7];
   if (top_start_x) *top_start_x = strut[8];
   if (top_end_x) *top_end_x = strut[9];
   if (bottom_start_x) *bottom_start_x = strut[10];
   if (bottom_end_x) *bottom_end_x = strut[11];
   return 1;
}

EAPI int
ecore_x_netwm_icons_get(Ecore_X_Window win, Ecore_X_Icon **icon, int *num)
{
   unsigned int  *data, *p;
   unsigned int  *src;
   unsigned int   len, icons, i;
   int            num_ret;

   if (num) *num = 0;
   if (icon) *icon = NULL;

   num_ret = ecore_x_window_prop_card32_list_get(win, ECORE_X_ATOM_NET_WM_ICON,
						 &data);
   if (num_ret <= 0)
     return 0;
   if (!data) return 0;
   if (num_ret < 2)
     {
	free(data);
	return 0;
     }

   /* Check how many icons there are */
   icons = 0;
   p = data;
   while (p)
     {
	len = p[0] * p[1];
	p += (len + 2);
	if ((p - data) > num_ret)
	  {
	     free(data);
	     return 0;
	  }
	icons++;

	if ((p - data) == num_ret)
	  p = NULL;
     }
   if (num) *num = icons;

   /* If the user doesn't want the icons, return */
   if (!icon)
     {
	free(data);
	return 1;
     }

   /* Allocate memory */
   *icon = malloc(icons * sizeof(Ecore_X_Icon));
   if (!(*icon))
     {
	free(data);
	return 0;
     }

   /* Fetch the icons */
   p = data;
   for (i = 0; i < icons; i++)
     {
	unsigned int *ps, *pd, *pe;
	
	len = p[0] * p[1];
	((*icon)[i]).width = p[0];
	((*icon)[i]).height = p[1];
	src = &(p[2]);
	((*icon)[i]).data = malloc(len * sizeof(unsigned int));
	if (!((*icon)[i]).data)
	  {
	     while (i)
	       free(((*icon)[--i]).data);
	     free(*icon);
	     free(data);
	     return 0;
	  }

	pd = ((*icon)[i]).data;
	ps = src;
	pe = ps + len;
	for (; ps < pe; ps++)
	  {
	     unsigned int r, g, b, a;
	     
	     a = (*ps >> 24) & 0xff;
	     r = (((*ps >> 16) & 0xff) * a) / 255;
	     g = (((*ps >>  8) & 0xff) * a) / 255;
	     b = (((*ps      ) & 0xff) * a) / 255;
	     *pd = (a << 24) | (r << 16) | (g << 8) | (b);
	     pd++;
	  }
	p += (len + 2);
     }

   free(data);

   return 1;
}

EAPI void
ecore_x_netwm_icon_geometry_set(Ecore_X_Window win, int x, int y, int width, int height)
{
   unsigned int geometry[4];

   geometry[0] = x;
   geometry[1] = y;
   geometry[2] = width;
   geometry[3] = height;
   ecore_x_window_prop_card32_set(win, ECORE_X_ATOM_NET_WM_ICON_GEOMETRY, geometry, 4);
}

EAPI int
ecore_x_netwm_icon_geometry_get(Ecore_X_Window win, int *x, int *y, int *width, int *height)
{
   int ret;
   unsigned int geometry[4];

   ret = ecore_x_window_prop_card32_get(win, ECORE_X_ATOM_NET_WM_ICON_GEOMETRY, geometry, 4);
   if (ret != 4)
     return 0;

   if (x) *x = geometry[0];
   if (y) *y = geometry[1];
   if (width) *width = geometry[2];
   if (height) *height = geometry[3];
   return 1;
}

EAPI void
ecore_x_netwm_pid_set(Ecore_X_Window win, int pid)
{
   unsigned int tmp;

   tmp = pid;
   ecore_x_window_prop_card32_set(win, ECORE_X_ATOM_NET_WM_PID,
				  &tmp, 1);
}

EAPI int
ecore_x_netwm_pid_get(Ecore_X_Window win, int *pid)
{
   int ret;
   unsigned int tmp;

   ret = ecore_x_window_prop_card32_get(win, ECORE_X_ATOM_NET_WM_PID,
					&tmp, 1);
   if (pid) *pid = tmp;
   return ret == 1 ? 1 : 0;
}

EAPI void
ecore_x_netwm_handled_icons_set(Ecore_X_Window win)
{
   ecore_x_window_prop_card32_set(win, ECORE_X_ATOM_NET_WM_HANDLED_ICONS,
				  NULL, 0);
}

EAPI int
ecore_x_netwm_handled_icons_get(Ecore_X_Window win)
{
   int ret = 0;
   ret = ecore_x_window_prop_card32_get(win, ECORE_X_ATOM_NET_WM_HANDLED_ICONS,
					NULL, 0);
   return ret == 0 ? 1 : 0;
}

EAPI void
ecore_x_netwm_user_time_set(Ecore_X_Window win, unsigned int time)
{
   ecore_x_window_prop_card32_set(win, ECORE_X_ATOM_NET_WM_USER_TIME,
				  &time, 1);
}

EAPI int
ecore_x_netwm_user_time_get(Ecore_X_Window win, unsigned int *time)
{
   int ret;
   unsigned int tmp;

   ret = ecore_x_window_prop_card32_get(win, ECORE_X_ATOM_NET_WM_USER_TIME,
					&tmp, 1);
   if (time) *time = tmp;
   return ret == 1 ? 1 : 0;
}

Ecore_X_Window_State
_ecore_x_netwm_state_get(Ecore_X_Atom a)
{
   if (a == ECORE_X_ATOM_NET_WM_STATE_MODAL)
     return ECORE_X_WINDOW_STATE_MODAL;
   else if (a == ECORE_X_ATOM_NET_WM_STATE_STICKY)
     return ECORE_X_WINDOW_STATE_STICKY;
   else if (a == ECORE_X_ATOM_NET_WM_STATE_MAXIMIZED_VERT)
     return ECORE_X_WINDOW_STATE_MAXIMIZED_VERT;
   else if (a == ECORE_X_ATOM_NET_WM_STATE_MAXIMIZED_HORZ)
     return ECORE_X_WINDOW_STATE_MAXIMIZED_HORZ;
   else if (a == ECORE_X_ATOM_NET_WM_STATE_SHADED)
     return ECORE_X_WINDOW_STATE_SHADED;
   else if (a == ECORE_X_ATOM_NET_WM_STATE_SKIP_TASKBAR)
     return ECORE_X_WINDOW_STATE_SKIP_TASKBAR;
   else if (a == ECORE_X_ATOM_NET_WM_STATE_SKIP_PAGER)
     return ECORE_X_WINDOW_STATE_SKIP_PAGER;
   else if (a == ECORE_X_ATOM_NET_WM_STATE_HIDDEN)
     return ECORE_X_WINDOW_STATE_HIDDEN;
   else if (a == ECORE_X_ATOM_NET_WM_STATE_FULLSCREEN)
     return ECORE_X_WINDOW_STATE_FULLSCREEN;
   else if (a == ECORE_X_ATOM_NET_WM_STATE_ABOVE)
     return ECORE_X_WINDOW_STATE_ABOVE;
   else if (a == ECORE_X_ATOM_NET_WM_STATE_BELOW)
     return ECORE_X_WINDOW_STATE_BELOW;
   else if (a == ECORE_X_ATOM_NET_WM_STATE_DEMANDS_ATTENTION)
     return ECORE_X_WINDOW_STATE_DEMANDS_ATTENTION;
   else
     return ECORE_X_WINDOW_STATE_UNKNOWN;
}

static Ecore_X_Atom
_ecore_x_netwm_state_atom_get(Ecore_X_Window_State s)
{
   switch(s)
     {
      case ECORE_X_WINDOW_STATE_MODAL:
	 return ECORE_X_ATOM_NET_WM_STATE_MODAL;
      case ECORE_X_WINDOW_STATE_STICKY:
	 return ECORE_X_ATOM_NET_WM_STATE_STICKY;
      case ECORE_X_WINDOW_STATE_MAXIMIZED_VERT:
	 return ECORE_X_ATOM_NET_WM_STATE_MAXIMIZED_VERT;
      case ECORE_X_WINDOW_STATE_MAXIMIZED_HORZ:
	 return ECORE_X_ATOM_NET_WM_STATE_MAXIMIZED_HORZ;
      case ECORE_X_WINDOW_STATE_SHADED:
	 return ECORE_X_ATOM_NET_WM_STATE_SHADED;
      case ECORE_X_WINDOW_STATE_SKIP_TASKBAR:
	 return ECORE_X_ATOM_NET_WM_STATE_SKIP_TASKBAR;
      case ECORE_X_WINDOW_STATE_SKIP_PAGER:
	 return ECORE_X_ATOM_NET_WM_STATE_SKIP_PAGER;
      case ECORE_X_WINDOW_STATE_HIDDEN:
	 return ECORE_X_ATOM_NET_WM_STATE_HIDDEN;
      case ECORE_X_WINDOW_STATE_FULLSCREEN:
	 return ECORE_X_ATOM_NET_WM_STATE_FULLSCREEN;
      case ECORE_X_WINDOW_STATE_ABOVE:
	 return ECORE_X_ATOM_NET_WM_STATE_ABOVE;
      case ECORE_X_WINDOW_STATE_BELOW:
	 return ECORE_X_ATOM_NET_WM_STATE_BELOW;
      case ECORE_X_WINDOW_STATE_DEMANDS_ATTENTION:
	 return ECORE_X_ATOM_NET_WM_STATE_DEMANDS_ATTENTION;
      default:
	 return 0;
     }
}

EAPI void
ecore_x_netwm_window_state_set(Ecore_X_Window win, Ecore_X_Window_State *state, unsigned int num)
{
   Ecore_X_Atom  *set;
   unsigned int   i;

   if (!num)
     {
	ecore_x_window_prop_property_del(win, ECORE_X_ATOM_NET_WM_STATE);
	return;
     }

   set = malloc(num * sizeof(Ecore_X_Atom));
   if (!set) return;

   for (i = 0; i < num; i++)
     set[i] = _ecore_x_netwm_state_atom_get(state[i]);

   ecore_x_window_prop_atom_set(win, ECORE_X_ATOM_NET_WM_STATE, set, num);

   free(set);
}

EAPI int
ecore_x_netwm_window_state_get(Ecore_X_Window win, Ecore_X_Window_State **state, unsigned int *num)
{
   int                   num_ret, i;
   Ecore_X_Atom         *atoms;

   if (num) *num = 0;
   if (state) *state = NULL;

   num_ret = ecore_x_window_prop_atom_list_get(win, ECORE_X_ATOM_NET_WM_STATE,
					       &atoms);
   if (num_ret <= 0)
      return 0;

   if (state)
     {
	*state = malloc(num_ret * sizeof(Ecore_X_Window_State));
	if (*state)
	  for (i = 0; i < num_ret; ++i)
	    (*state)[i] = _ecore_x_netwm_state_get(atoms[i]);

	if (num) *num = num_ret;
     }

   free(atoms);
   return 1;
}

static Ecore_X_Window_Type
_ecore_x_netwm_window_type_type_get(Ecore_X_Atom atom)
{
   if (atom == ECORE_X_ATOM_NET_WM_WINDOW_TYPE_DESKTOP)
     return ECORE_X_WINDOW_TYPE_DESKTOP;
   else if (atom == ECORE_X_ATOM_NET_WM_WINDOW_TYPE_DOCK)
     return ECORE_X_WINDOW_TYPE_DOCK;
   else if (atom == ECORE_X_ATOM_NET_WM_WINDOW_TYPE_TOOLBAR)
     return ECORE_X_WINDOW_TYPE_TOOLBAR;
   else if (atom == ECORE_X_ATOM_NET_WM_WINDOW_TYPE_MENU)
     return ECORE_X_WINDOW_TYPE_MENU;
   else if (atom == ECORE_X_ATOM_NET_WM_WINDOW_TYPE_UTILITY)
     return ECORE_X_WINDOW_TYPE_UTILITY;
   else if (atom == ECORE_X_ATOM_NET_WM_WINDOW_TYPE_SPLASH)
     return ECORE_X_WINDOW_TYPE_SPLASH;
   else if (atom == ECORE_X_ATOM_NET_WM_WINDOW_TYPE_DIALOG)
     return ECORE_X_WINDOW_TYPE_DIALOG;
   else if (atom == ECORE_X_ATOM_NET_WM_WINDOW_TYPE_NORMAL)
     return ECORE_X_WINDOW_TYPE_NORMAL;
   else
     return ECORE_X_WINDOW_TYPE_UNKNOWN;
}

static Ecore_X_Atom
_ecore_x_netwm_window_type_atom_get(Ecore_X_Window_Type type)
{
   switch (type)
     {
      case ECORE_X_WINDOW_TYPE_DESKTOP:
	 return ECORE_X_ATOM_NET_WM_WINDOW_TYPE_DESKTOP;
      case ECORE_X_WINDOW_TYPE_DOCK:
	 return ECORE_X_ATOM_NET_WM_WINDOW_TYPE_DOCK;
      case ECORE_X_WINDOW_TYPE_TOOLBAR:
	 return ECORE_X_ATOM_NET_WM_WINDOW_TYPE_TOOLBAR;
      case ECORE_X_WINDOW_TYPE_MENU:
	 return ECORE_X_ATOM_NET_WM_WINDOW_TYPE_MENU;
      case ECORE_X_WINDOW_TYPE_UTILITY:
	 return ECORE_X_ATOM_NET_WM_WINDOW_TYPE_UTILITY;
      case ECORE_X_WINDOW_TYPE_SPLASH:
	 return ECORE_X_ATOM_NET_WM_WINDOW_TYPE_SPLASH;
      case ECORE_X_WINDOW_TYPE_DIALOG:
	 return ECORE_X_ATOM_NET_WM_WINDOW_TYPE_DIALOG;
      case ECORE_X_WINDOW_TYPE_NORMAL:
	 return ECORE_X_ATOM_NET_WM_WINDOW_TYPE_NORMAL;
      default:
	 return 0;
     }
}

/*
 * FIXME: We should set WM_TRANSIENT_FOR if type is ECORE_X_WINDOW_TYPE_TOOLBAR
 * , ECORE_X_WINDOW_TYPE_MENU or ECORE_X_WINDOW_TYPE_DIALOG
 */
EAPI void
ecore_x_netwm_window_type_set(Ecore_X_Window win, Ecore_X_Window_Type type)
{
   Ecore_X_Atom atom;

   atom = _ecore_x_netwm_window_type_atom_get(type);
   ecore_x_window_prop_atom_set(win, ECORE_X_ATOM_NET_WM_WINDOW_TYPE,
				&atom, 1);
}

/* FIXME: Maybe return 0 on some conditions? */
EAPI int
ecore_x_netwm_window_type_get(Ecore_X_Window win, Ecore_X_Window_Type *type)
{
   int                  num;
   Ecore_X_Atom        *atoms = NULL;

   if (type) *type = ECORE_X_WINDOW_TYPE_NORMAL;

   num = ecore_x_window_prop_atom_list_get(win, 
					   ECORE_X_ATOM_NET_WM_WINDOW_TYPE,
					   &atoms);
   if ((type) && (num >= 1) && (atoms))
     *type = _ecore_x_netwm_window_type_type_get(atoms[0]);

   free(atoms);
   if (num >= 1) return 1;
   return 0;
}

EAPI int
ecore_x_netwm_window_types_get(Ecore_X_Window win, Ecore_X_Window_Type **types)
{
   int                  num, i;
   Ecore_X_Atom        *atoms = NULL;
   Ecore_X_Window_Type *atoms2 = NULL;
   
   if (types) *types = NULL;
   num = ecore_x_window_prop_atom_list_get(win,
					   ECORE_X_ATOM_NET_WM_WINDOW_TYPE,
					   &atoms);
   if ((num <= 0) || (!atoms))
     {
	if (atoms) free(atoms);
	return 0;
     }
   atoms2 = malloc(num * sizeof(Ecore_X_Window_Type));
   if (!atoms2) return 0;
   for (i = 0; i < num; i++)
     atoms2[i] = _ecore_x_netwm_window_type_type_get(atoms[i]);
   free(atoms);
   if (types) *types = atoms2;
   else free(atoms2);
   return num;
}

static Ecore_X_Atom
_ecore_x_netwm_action_atom_get(Ecore_X_Action action)
{
   switch (action)
     {
      case ECORE_X_ACTION_MOVE:
	 return ECORE_X_ATOM_NET_WM_ACTION_MOVE;
      case ECORE_X_ACTION_RESIZE:
	 return ECORE_X_ATOM_NET_WM_ACTION_RESIZE;
      case ECORE_X_ACTION_MINIMIZE:
	 return ECORE_X_ATOM_NET_WM_ACTION_MINIMIZE;
      case ECORE_X_ACTION_SHADE:
	 return ECORE_X_ATOM_NET_WM_ACTION_SHADE;
      case ECORE_X_ACTION_STICK:
	 return ECORE_X_ATOM_NET_WM_ACTION_STICK;
      case ECORE_X_ACTION_MAXIMIZE_HORZ:
	 return ECORE_X_ATOM_NET_WM_ACTION_MAXIMIZE_HORZ;
      case ECORE_X_ACTION_MAXIMIZE_VERT:
	 return ECORE_X_ATOM_NET_WM_ACTION_MAXIMIZE_VERT;
      case ECORE_X_ACTION_FULLSCREEN:
	 return ECORE_X_ATOM_NET_WM_ACTION_FULLSCREEN;
      case ECORE_X_ACTION_CHANGE_DESKTOP:
	 return ECORE_X_ATOM_NET_WM_ACTION_CHANGE_DESKTOP;
      case ECORE_X_ACTION_CLOSE:
	 return ECORE_X_ATOM_NET_WM_ACTION_CLOSE;
      case ECORE_X_ACTION_ABOVE:
	 return ECORE_X_ATOM_NET_WM_ACTION_ABOVE;
      case ECORE_X_ACTION_BELOW:
	 return ECORE_X_ATOM_NET_WM_ACTION_BELOW;
      default:
	 return 0;
     }
}

/* FIXME: Get complete list */
EAPI int
ecore_x_netwm_allowed_action_isset(Ecore_X_Window win, Ecore_X_Action action)
{
   int                  num, i, ret = 0;
   Ecore_X_Atom        *atoms, atom;

   num = ecore_x_window_prop_atom_list_get(win, ECORE_X_ATOM_NET_WM_WINDOW_TYPE,
					   &atoms);
   if (num <= 0)
     return ret;

   atom = _ecore_x_netwm_action_atom_get(action);

   for (i = 0; i < num; ++i)
     {
	if (atom == atoms[i])
	  {
	     ret = 1;
	     break;
	  }
     }

   free(atoms);
   return ret;
}

/* FIXME: Set complete list */
EAPI void
ecore_x_netwm_allowed_action_set(Ecore_X_Window win, Ecore_X_Action *action, unsigned int num)
{
   Ecore_X_Atom  *set;
   unsigned int   i;

   if (!num)
     {
	ecore_x_window_prop_property_del(win, ECORE_X_ATOM_NET_WM_ALLOWED_ACTIONS);
	return;
     }

   set = malloc(num * sizeof(Ecore_X_Atom));
   if (!set) return;

   for (i = 0; i < num; i++)
     set[i] = _ecore_x_netwm_action_atom_get(action[i]);

   ecore_x_window_prop_atom_set(win, ECORE_X_ATOM_NET_WM_ALLOWED_ACTIONS, set, num);

   free(set);
}

EAPI int
ecore_x_netwm_allowed_action_get(Ecore_X_Window win, Ecore_X_Action **action, unsigned int *num)
{
   int                   num_ret, i;
   Ecore_X_Atom         *atoms;

   if (num) *num = 0;
   if (action) *action = NULL;

   num_ret = ecore_x_window_prop_atom_list_get(win, ECORE_X_ATOM_NET_WM_ALLOWED_ACTIONS,
					       &atoms);
   if (num_ret <= 0)
      return 0;

   if (action)
     {
	*action = malloc(num_ret * sizeof(Ecore_X_Action));
	if (*action)
	  for (i = 0; i < num_ret; ++i)
	    (*action)[i] = _ecore_x_netwm_action_atom_get(atoms[i]);

	if (num) *num = num_ret;
     }

   free(atoms);
   return 1;
}

EAPI void
ecore_x_netwm_opacity_set(Ecore_X_Window win, unsigned int opacity)
{
   ecore_x_window_prop_card32_set(win, ECORE_X_ATOM_NET_WM_WINDOW_OPACITY,
				  &opacity, 1);
}

EAPI int
ecore_x_netwm_opacity_get(Ecore_X_Window win, unsigned int *opacity)
{
   int ret;
   unsigned int tmp;

   ret = ecore_x_window_prop_card32_get(win, ECORE_X_ATOM_NET_WM_WINDOW_OPACITY,
					&tmp, 1);
   if (opacity) *opacity = tmp;
   return ret == 1 ? 1 : 0;
}

EAPI void
ecore_x_netwm_frame_size_set(Ecore_X_Window win, int fl, int fr, int ft, int fb)
{
   unsigned int frames[4];

   frames[0] = fl;
   frames[1] = fr;
   frames[2] = ft;
   frames[3] = fb;
   ecore_x_window_prop_card32_set(win, ECORE_X_ATOM_NET_FRAME_EXTENTS, frames, 4);
}

EAPI int
ecore_x_netwm_frame_size_get(Ecore_X_Window win, int *fl, int *fr, int *ft, int *fb)
{
   int ret = 0;
   unsigned int frames[4];

   ret = ecore_x_window_prop_card32_get(win, ECORE_X_ATOM_NET_FRAME_EXTENTS, frames, 4);
   if (ret != 4)
     return 0;

   if (fl) *fl = frames[0];
   if (fr) *fr = frames[1];
   if (ft) *ft = frames[2];
   if (fb) *fb = frames[3];
   return 1;
}

EAPI int
ecore_x_netwm_sync_counter_get(Ecore_X_Window win, Ecore_X_Sync_Counter *counter)
{
   int          ret;
   unsigned int tmp;

   ret = ecore_x_window_prop_card32_get(win, ECORE_X_ATOM_NET_WM_SYNC_REQUEST_COUNTER,
					&tmp, 1);

   if (counter) *counter = tmp;
   return ret == 1 ? 1 : 0;
}

EAPI void
ecore_x_netwm_ping_send(Ecore_X_Window win)
{
   XEvent xev;

   if (!win) return;

   xev.xclient.type = ClientMessage;
   xev.xclient.display = _ecore_x_disp;
   xev.xclient.window = win;
   xev.xclient.message_type = ECORE_X_ATOM_WM_PROTOCOLS;
   xev.xclient.format = 32;
   xev.xclient.data.l[0] = ECORE_X_ATOM_NET_WM_PING;
   xev.xclient.data.l[1] = CurrentTime;
   xev.xclient.data.l[2] = win;
   xev.xclient.data.l[3] = 0;
   xev.xclient.data.l[4] = 0;
   xev.xclient.data.l[5] = 0;

   XSendEvent(_ecore_x_disp, win, False, NoEventMask, &xev);
}

EAPI void
ecore_x_netwm_sync_request_send(Ecore_X_Window win, unsigned int serial)
{
   XSyncValue value;
   XEvent xev;

   if (!win) return;

   XSyncIntToValue(&value, (int)serial);

   xev.xclient.type = ClientMessage;
   xev.xclient.display = _ecore_x_disp;
   xev.xclient.window = win;
   xev.xclient.message_type = ECORE_X_ATOM_WM_PROTOCOLS;
   xev.xclient.format = 32;
   xev.xclient.data.l[0] = ECORE_X_ATOM_NET_WM_SYNC_REQUEST;
   xev.xclient.data.l[1] = CurrentTime;
   xev.xclient.data.l[2] = XSyncValueLow32(value);
   xev.xclient.data.l[3] = XSyncValueHigh32(value);
   xev.xclient.data.l[4] = 0;

   XSendEvent(_ecore_x_disp, win, False, NoEventMask, &xev);
}

EAPI void
ecore_x_netwm_state_request_send(Ecore_X_Window win, Ecore_X_Window root,
				 Ecore_X_Window_State s1, Ecore_X_Window_State s2, int set)
{
   XEvent xev;

   if (!win) return;
   if (!root) root = DefaultRootWindow(_ecore_x_disp);

   xev.xclient.type = ClientMessage;
   xev.xclient.serial = 0;
   xev.xclient.send_event = True;
   xev.xclient.display = _ecore_x_disp;
   xev.xclient.window = win;
   xev.xclient.format = 32;
   xev.xclient.message_type = ECORE_X_ATOM_NET_WM_STATE;
   xev.xclient.data.l[0] = !!set;
   xev.xclient.data.l[1] = _ecore_x_netwm_state_atom_get(s1);
   xev.xclient.data.l[2] = _ecore_x_netwm_state_atom_get(s2);
   /* 1 == normal client, if someone wants to use this
    * function in a pager, this should be 2 */
   xev.xclient.data.l[3] = 1;
   xev.xclient.data.l[4] = 0;

   XSendEvent(_ecore_x_disp, root, False,
	      SubstructureNotifyMask | SubstructureRedirectMask, &xev);
}

EAPI void
ecore_x_netwm_desktop_request_send(Ecore_X_Window win, Ecore_X_Window root, unsigned int desktop)
{
   XEvent xev;

   if (!win) return;
   if (!root) root = DefaultRootWindow(_ecore_x_disp);

   xev.xclient.type = ClientMessage;
   xev.xclient.serial = 0;
   xev.xclient.send_event = True;
   xev.xclient.display = _ecore_x_disp;
   xev.xclient.window = win;
   xev.xclient.format = 32;
   xev.xclient.message_type = ECORE_X_ATOM_NET_WM_DESKTOP;
   xev.xclient.data.l[0] = desktop;

   XSendEvent(_ecore_x_disp, root, False,
	      SubstructureNotifyMask | SubstructureRedirectMask, &xev);
}

int
_ecore_x_netwm_startup_info_begin(Ecore_X_Window win, char *data)
{
#if 0
   Ecore_X_Startup_Info *info;
   unsigned char	*exists = 0;

   if (!startup_info) return 0;
   info = eina_hash_find(startup_info, (void *)win);
   if (info)
     {
        exists = 1;
	printf("Already got info for win: 0x%x\n", win);
	_ecore_x_netwm_startup_info_free(info);
     }
   info = calloc(1, sizeof(Ecore_X_Startup_Info));
   if (!info) return 0;
   info->win = win;
   info->length = 0;
   info->buffer_size = 161;
   info->buffer = calloc(info->buffer_size, sizeof(char));
   if (!info->buffer)
     {
	_ecore_x_netwm_startup_info_free(info);
	return 0;
     }
   memcpy(info->buffer, data, 20);
   info->length += 20;
   info->buffer[info->length] = 0;
   if (exists)
     eina_hash_modify(startup_info, (void *)info->win, info);
   else
     eina_hash_add(startup_info, (void *)info->win, info);
   if (strlen(info->buffer) != 20)
     {
	/* We have a '\0' in there, the message is done */
	_ecore_x_netwm_startup_info_process(info);
     }
#else
   win = 0;
   data = NULL;
#endif
   return 1;
}

int
_ecore_x_netwm_startup_info(Ecore_X_Window win, char *data)
{
#if 0
   Ecore_X_Startup_Info *info;
   char *p;

   if (!startup_info) return 0;
   info = eina_hash_find(startup_info, (void *)win);
   if (!info) return 0;
   if ((info->length + 20) > info->buffer_size)
     {
	info->buffer_size += 160;
	info->buffer = realloc(info->buffer, info->buffer_size * sizeof(char));
	if (!info->buffer)
	  {
	     eina_hash_del(startup_info, (void *)info->win);
	     _ecore_x_netwm_startup_info_free(info);
	     return 0;
	  }
     }
   memcpy(info->buffer + info->length, data, 20);
   p = info->buffer + info->length;
   info->length += 20;
   info->buffer[info->length] = 0;
   if (strlen(p) != 20)
     {
	/* We have a '\0' in there, the message is done */
	_ecore_x_netwm_startup_info_process(info);
     }
#else
   win = 0;
   data = NULL;
#endif
   return 1;
}

/*
 * Set UTF-8 string property
 */
static void
_ecore_x_window_prop_string_utf8_set(Ecore_X_Window win, Ecore_X_Atom atom,
				     const char *str)
{
   XChangeProperty(_ecore_x_disp, win, atom, ECORE_X_ATOM_UTF8_STRING, 8,
		   PropModeReplace, (unsigned char *)str, strlen(str));
}

/*
 * Get UTF-8 string property
 */
static char *
_ecore_x_window_prop_string_utf8_get(Ecore_X_Window win, Ecore_X_Atom atom)
{
   char               *str;
   unsigned char      *prop_ret;
   Atom                type_ret;
   unsigned long       bytes_after, num_ret;
   int                 format_ret;

   str = NULL;
   prop_ret = NULL;
   XGetWindowProperty(_ecore_x_disp, win, atom, 0, 0x7fffffff, False,
		      ECORE_X_ATOM_UTF8_STRING, &type_ret,
		      &format_ret, &num_ret, &bytes_after, &prop_ret);
   if (prop_ret && num_ret > 0 && format_ret == 8)
     {
	str = malloc(num_ret + 1);
	if (str)
	  {
	     memcpy(str, prop_ret, num_ret);
	     str[num_ret] = '\0';
	  }
     }
   if (prop_ret)
      XFree(prop_ret);

   return str;
}

#if 0 /* Unused */
/*
 * Process startup info
 */
static int
_ecore_x_netwm_startup_info_process(Ecore_X_Startup_Info *info)
{
   Ecore_X_Event_Startup_Sequence *e;
   int                             event;
   char                           *p;

   p = strchr(info->buffer, ':');
   if (!p)
     {
	eina_hash_del(startup_info, (void *)info->win);
	_ecore_x_netwm_startup_info_free(info);
	return 0;
     }
   *p = 0;
   if (!strcmp(info->buffer, "new"))
     {
	if (info->init)
	  event = ECORE_X_EVENT_STARTUP_SEQUENCE_CHANGE;
	else
	  event = ECORE_X_EVENT_STARTUP_SEQUENCE_NEW;
	info->init = 1;
     }
   else if (!strcmp(info->buffer, "change"))
     {
	event = ECORE_X_EVENT_STARTUP_SEQUENCE_CHANGE;
     }
   else if (!strcmp(info->buffer, "remove"))
     event = ECORE_X_EVENT_STARTUP_SEQUENCE_REMOVE;
   else
     {
	eina_hash_del(startup_info, (void *)info->win);
	_ecore_x_netwm_startup_info_free(info);
	return 0;
     }

   p++;

   if (!_ecore_x_netwm_startup_info_parse(info, p))
     {
	eina_hash_del(startup_info, (void *)info->win);
	_ecore_x_netwm_startup_info_free(info);
	return 0;
     }

   if (info->init)
     {
	e = calloc(1, sizeof(Ecore_X_Event_Startup_Sequence));
	if (!e)
	  {
	     eina_hash_del(startup_info, (void *)info->win);
	     _ecore_x_netwm_startup_info_free(info);
	     return 0;
	  }
	e->win = info->win;
	ecore_event_add(event, e, NULL, NULL);
     }

   if (event == ECORE_X_EVENT_STARTUP_SEQUENCE_REMOVE)
     {
	eina_hash_del(startup_info, (void *)info->win);
	_ecore_x_netwm_startup_info_free(info);
     }
   else
     {
	/* Discard buffer */
	info->length = 0;
	info->buffer[0] = 0;
     }
   return 1;
}

/*
 * Parse startup info
 */
static int
_ecore_x_netwm_startup_info_parse(Ecore_X_Startup_Info *info, char *data)
{

   while (*data)
     {
	int in_quot_sing, in_quot_dbl, escaped;
	char *p, *pp;
	char *key;
	char value[1024];

	/* Skip space */
	while (*data == ' ') data++;
	/* Get key */
	key = data;
	data = strchr(key, '=');
	if (!data) return 0;
	*data = 0;
	data++;

	/* Get value */
	p = data;
	pp = value;
	in_quot_dbl = 0;
	in_quot_sing = 0;
	escaped = 0;
	while (*p)
	  {
	     if ((pp - value) >= 1024) return 0;
	     if (escaped)
	       {
		  *pp = *p;
		  pp++;
		  escaped = 0;
	       }
	     else if (in_quot_sing)
	       {
		  if (*p == '\\')
		    escaped = 1;
		  else if (*p == '\'')
		    in_quot_sing = 0;
		  else
		    {
		       *pp = *p;
		       pp++;
		    }
	       }
	     else if (in_quot_dbl)
	       {
		  if (*p == '\\')
		    escaped = 1;
		  else if (*p == '\"')
		    in_quot_dbl = 0;
		  else
		    {
		       *pp = *p;
		       pp++;
		    }
	       }
	     else
	       {
		  if (*p == '\\')
		    escaped = 1;
		  else if (*p == '\'')
		    in_quot_sing = 1;
		  else if (*p == '\"')
		    in_quot_dbl = 1;
		  else if (*p == ' ')
		    {
		       break;
		    }
		  else
		    {
		       *pp = *p;
		       pp++;
		    }
	       }
	     p++;
	  }
	if ((in_quot_dbl) || (in_quot_sing)) return 0;
	data = p;
	*pp = 0;

	/* Parse info */
	if (!strcmp(key, "ID"))
	  {
	     if ((info->id) && (strcmp(info->id, value))) return 0;
	     info->id = strdup(value);
	     p = strstr(value, "_TIME");
	     if (p)
	       {
		  info->timestamp = atoi(p + 5);
	       }
	  }
	else if (!strcmp(key, "NAME"))
	  {
	     if (info->name) free(info->name);
	     info->name = strdup(value);
	  }
	else if (!strcmp(key, "SCREEN"))
	  {
	     info->screen = atoi(value);
	  }
	else if (!strcmp(key, "BIN"))
	  {
	     if (info->bin) free(info->bin);
	     info->bin = strdup(value);
	  }
	else if (!strcmp(key, "ICON"))
	  {
	     if (info->icon) free(info->icon);
	     info->icon = strdup(value);
	  }
	else if (!strcmp(key, "DESKTOP"))
	  {
	     info->desktop = atoi(value);
	  }
	else if (!strcmp(key, "TIMESTAMP"))
	  {
	     if (!info->timestamp)
	       info->timestamp = atoi(value);
	  }
	else if (!strcmp(key, "DESCRIPTION"))
	  {
	     if (info->description) free(info->description);
	     info->description = strdup(value);
	  }
	else if (!strcmp(key, "WMCLASS"))
	  {
	     if (info->wmclass) free(info->wmclass);
	     info->wmclass = strdup(value);
	  }
	else if (!strcmp(key, "SILENT"))
	  {
	     info->silent = atoi(value);
	  }
	else
	  {
	     printf("Ecore X Sequence, Unknown: %s=%s\n", key, value);
	  }
     }
   if (!info->id) return 0;
   return 1;
}
#endif

/*
 * Free startup info struct
 */
static void
_ecore_x_netwm_startup_info_free(void *data)
{
   Ecore_X_Startup_Info *info;

   info = data;
   if (!info) return;
   if (info->buffer) free(info->buffer);
   if (info->id) free(info->id);
   if (info->name) free(info->name);
   if (info->bin) free(info->bin);
   if (info->icon) free(info->icon);
   if (info->description) free(info->description);
   if (info->wmclass) free(info->wmclass);
   free(info);
}

/*
 * Is screen composited?
 */
EAPI int
ecore_x_screen_is_composited(int screen)
{
   Ecore_X_Window      win;
   Ecore_X_Atom        atom;
   char                buf[32];

   snprintf(buf, sizeof(buf), "_NET_WM_CM_S%d", screen);
   atom = XInternAtom(_ecore_x_disp, buf, True);
   if (atom == None) return 0;

   win = XGetSelectionOwner(_ecore_x_disp, atom);

   return win != None;
}
