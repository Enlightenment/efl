/*
 * _NET_WM... aka Extended Window Manager Hint (EWMH) functions.
 */
#include "Ecore.h"
#include "ecore_x_private.h"
#include "Ecore_X.h"

/*
 * Convenience macros
 */
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

#if 0
/*
 * All netwm atoms could be moved to here. Otherwise remove this.
 */
Atom                _ecore_x_atom_net_number_of_desktops;
Atom                _ecore_x_atom_net_virtual_roots;
Atom                _ecore_x_atom_net_desktop_names;
Atom                _ecore_x_atom_net_desktop_geometry;
Atom                _ecore_x_atom_net_workarea;
Atom                _ecore_x_atom_net_current_desktop;
Atom                _ecore_x_atom_net_desktop_viewport;
Atom                _ecore_x_atom_net_showing_desktop;

Atom                _ecore_x_atom_net_client_list;
Atom                _ecore_x_atom_net_client_list_stacking;
Atom                _ecore_x_atom_net_active_window;

void
ecore_x_netwm_init(void)
{
   _ecore_x_atom_net_number_of_desktops =
      XInternAtom(_ecore_x_disp, "_NET_NUMBER_OF_DESKTOPS", False);
   _ecore_x_atom_net_virtual_roots =
      XInternAtom(_ecore_x_disp, "_NET_VIRTUAL_ROOTS", False);
   _ecore_x_atom_net_desktop_names =
      XInternAtom(_ecore_x_disp, "_NET_DESKTOP_NAMES", False);
   _ecore_x_atom_net_desktop_geometry =
      XInternAtom(_ecore_x_disp, "_NET_DESKTOP_GEOMETRY", False);
   _ecore_x_atom_net_workarea =
      XInternAtom(_ecore_x_disp, "_NET_WORKAREA", False);
   _ecore_x_atom_net_current_desktop =
      XInternAtom(_ecore_x_disp, "_NET_CURRENT_DESKTOP", False);
   _ecore_x_atom_net_desktop_viewport =
      XInternAtom(_ecore_x_disp, "_NET_DESKTOP_VIEWPORT", False);
   _ecore_x_atom_net_showing_desktop =
      XInternAtom(_ecore_x_disp, "_NET_SHOWING_DESKTOP", False);

   _ecore_x_atom_net_client_list =
      XInternAtom(_ecore_x_disp, "_NET_CLIENT_LIST", False);
   _ecore_x_atom_net_client_list_stacking =
      XInternAtom(_ecore_x_disp, "_NET_CLIENT_LIST_STACKING", False);
   _ecore_x_atom_net_active_window =
      XInternAtom(_ecore_x_disp, "_NET_ACTIVE_WINDOW", False);
}
#endif

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
