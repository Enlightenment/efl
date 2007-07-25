/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

/*
 * _NET_WM... aka Extended Window Manager Hint (EWMH) functions.
 */

#include "Ecore_Data.h"
#include "ecore_xcb_private.h"
#include "Ecore_X_Atoms.h"


/**
 * @defgroup Ecore_X_NetWM_Group Extended Window Manager Hint (EWMH) functions
 *
 * Functions related to the Extended Window Manager Hint (EWMH).
 */


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

#if 0
static void  _ecore_x_window_prop_string_utf8_get_prefetch(Ecore_X_Window window, Ecore_X_Atom atom);
static void  _ecore_x_window_prop_string_utf8_get_fetch(void);
#endif
static void  _ecore_x_window_prop_string_utf8_set(Ecore_X_Window window, Ecore_X_Atom atom, const char *str);
static char *_ecore_x_window_prop_string_utf8_get(Ecore_X_Window window, Ecore_X_Atom atom);
#if 0 /* Unused */
static int   _ecore_x_netwm_startup_info_process(Ecore_X_Startup_Info *info);
static int   _ecore_x_netwm_startup_info_parse(Ecore_X_Startup_Info *info, char *data);
#endif
static void  _ecore_x_netwm_startup_info_free(void *data);

/*
 * Local variables
 */

static Ecore_Hash *startup_info = NULL;

/**
 * Initialize the NetWM module
 */
EAPI void
ecore_x_netwm_init(void)
{
   startup_info = ecore_hash_new(ecore_direct_hash, ecore_direct_compare);
   if (startup_info)
     {
	ecore_hash_free_value_cb_set(startup_info, _ecore_x_netwm_startup_info_free);
     }
}

/**
 * Shutdown the NetWM module
 */
EAPI void
ecore_x_netwm_shutdown(void)
{
   if (startup_info)
     ecore_hash_destroy(startup_info);
   startup_info = NULL;
}

/**
 * Set the _NET_SUPPORTING_WM_CHECK property.
 * @param root    The root window.
 * @param check   The child window.
 * @param wm_name The name of the Window Manager.
 *
 * Set the _NET_SUPPORTING_WM_CHECK property on the @p root window to be
 * the ID of the child window @p check created by the Window Manager.
 * @p check also have the _NET_WM_NAME property set to the name
 * @p wm_name of the Window Manager.
 *
 * The Window MUST call that function to indicate that a compliant
 * window manager is active.
 * @ingroup Ecore_X_NetWM_Group
 */
EAPI void
ecore_x_netwm_wm_identify(Ecore_X_Window root,
                          Ecore_X_Window check,
			  const char    *wm_name)
{
   ecore_x_window_prop_window_set(root, ECORE_X_ATOM_NET_SUPPORTING_WM_CHECK, &check, 1);
   ecore_x_window_prop_window_set(check, ECORE_X_ATOM_NET_SUPPORTING_WM_CHECK, &check, 1);
   _ecore_x_window_prop_string_utf8_set(check, ECORE_X_ATOM_NET_WM_NAME, wm_name);
   /* This one isn't mandatory */
   _ecore_x_window_prop_string_utf8_set(root, ECORE_X_ATOM_NET_WM_NAME, wm_name);
}

/**
 * Set the _NET_SUPPORTED property.
 * @param root      The root window.
 * @param supported The supported hints.
 * @param num       The number of hints.
 *
 * Set the _NET_SUPPORTED property on the @p root window. The hints
 * that the Window Manager supports are stored in @p supported.
 *
 * The Window Manager MUST set this property to indicate which hints
 * it supports.
 * @ingroup Ecore_X_NetWM_Group
 */
EAPI void
ecore_x_netwm_supported_set(Ecore_X_Window root,
                            Ecore_X_Atom  *supported,
                            int            num)
{
   ecore_x_window_prop_atom_set(root, ECORE_X_ATOM_NET_SUPPORTED, supported, num);
}

/**
 * Sends the GetProperty request.
 * @param root The root window
 * @ingroup Ecore_X_NetWM_Group
 */
EAPI void
ecore_x_netwm_supported_get_prefetch(Ecore_X_Window root)
{
   xcb_get_property_cookie_t cookie;

   cookie = xcb_get_property_unchecked(_ecore_xcb_conn, 0, root,
                                       ECORE_X_ATOM_NET_SUPPORTED, ECORE_X_ATOM_ATOM,
                                       0, 0x7fffffff);
   _ecore_xcb_cookie_cache(cookie.sequence);
}

/**
 * Gets the reply of the GetProperty request sent by ecore_x_netwm_supported_get_prefetch().
 * @ingroup Ecore_X_NetWM_Group
 */
EAPI void
ecore_x_netwm_supported_get_fetch(void)
{
   xcb_get_property_cookie_t cookie;
   xcb_get_property_reply_t *reply;

   cookie.sequence = _ecore_xcb_cookie_get();
   reply = xcb_get_property_reply(_ecore_xcb_conn, cookie, NULL);
   _ecore_xcb_reply_cache(reply);
}

/**
 * Get the hints supported by the Window Manager.
 * @param root      The root window.
 * @param supported The supported hints.
 * @param num       The number of atoms.
 * @return          1 on success, 0 otherwise.
 *
 * Get the hints supported by the Window Manager. @p root is the root
 * window. The hints are stored in @p supported. The number of hints
 * is stored in @p num.
 *
 * To use this function, you must call before, and in order,
 * ecore_x_netwm_supported_get_prefetch(), which sends the GetProperty request,
 * then ecore_x_netwm_supported_get_fetch(), which gets the reply.
 * @ingroup Ecore_X_NetWM_Group
 */
EAPI int
ecore_x_netwm_supported_get(Ecore_X_Window root,
                            Ecore_X_Atom **supported,
                            int           *num)
{
   int num_ret;

   if (num) *num = 0UL;
   if (supported) *supported = NULL;

   num_ret = ecore_x_window_prop_xid_list_get(root,
                                              ECORE_X_ATOM_NET_SUPPORTED,
                                              ECORE_X_ATOM_ATOM,
                                              supported);
   if (num_ret <= 0)
      return 0;

   if (num) *num = (uint32_t)num_ret;
   return 1;
}

/**
 * Set the _NET_NUMBER_OF_DESKTOPS property.
 * @param root The root window.
 * @param n_desks The number of desktops.
 *
 * Set the number of desktops @p n_desks of the Window Manager by
 * sending the _NET_NUMBER_OF_DESKTOPS to the @p root window.
 *
 * The Window Manager SHOULD set and update this property to indicate
 * the number of virtual desktops. A Pager can request a change in the
 * number of desktops by using that function.
 * @ingroup Ecore_X_NetWM_Group
 */
EAPI void
ecore_x_netwm_desk_count_set(Ecore_X_Window root,
                             unsigned int   n_desks)
{
   ecore_x_window_prop_card32_set(root, ECORE_X_ATOM_NET_NUMBER_OF_DESKTOPS,
				  &n_desks, 1);
}

/**
 * Set the _NET_VIRTUAL_ROOTS property.
 * @param root    The root window.
 * @param vroots  The virtual root windows.
 * @param n_desks The number of desks.
 *
 * Set the number of virtual desktops by sending the
 * _NET_VIRTUAL_ROOTS property to the @p root window. @p vroots is an
 * array of window and @p n_desks is the number of windows.
 *
 * A Window Manager that implements virtual desktops by reparent
 * client windows to a child of the root window MUST use that
 * function.
 * @ingroup Ecore_X_NetWM_Group
 */
EAPI void
ecore_x_netwm_desk_roots_set(Ecore_X_Window  root,
			     Ecore_X_Window *vroots,
                             unsigned int    n_desks)
{
   ecore_x_window_prop_window_set(root, ECORE_X_ATOM_NET_VIRTUAL_ROOTS, vroots, n_desks);
}

/**
 * Set the _NET_DESKTOP_NAMES property.
 * @param root    The root window.
 * @param names   The names of the virtual desktops.
 * @param n_desks The number of virtual desktops.
 *
 * Set the name of each virtual desktop by sending the
 * _NET_DESKTOP_NAMES to the @p root window. @p names are the names of
 * the virtual desktops and @p n_desks is the number of virtual
 * desktops.
 *
 * A Pager MAY use that function. @p n_desks may be different from the
 * one passed to ecore_x_netwm_desk_count_set().  If it less or equal,
 * then the desktops with high numbers are unnamed. If it is larger,
 * then the excess names are considered to be reserved in case the
 * number of desktops is increased.
 * @ingroup Ecore_X_NetWM_Group
 */
EAPI void
ecore_x_netwm_desk_names_set(Ecore_X_Window root,
			     const char   **names,
                             unsigned int   n_desks)
{
   char        ss[32];
   char       *buf;
   const char *s;
   uint32_t    i;
   uint32_t    len;
   uint32_t    l;

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

   xcb_change_property(_ecore_xcb_conn, XCB_PROP_MODE_REPLACE, root,
                       ECORE_X_ATOM_NET_DESKTOP_NAMES,
                       ECORE_X_ATOM_UTF8_STRING,
                       8, len, (const void *)buf);
   free(buf);
}

/**
 * Set the _NET_DESKTOP_GEOMETRY property.
 * @param root   The root window.
 * @param width  The width of the desktop.
 * @param height The height of the desktop.
 *
 * Set the common @p width and @p height of all desktops by sending
 * the _NET_DESKTOP_GEOMETRY to the @p root window.
 *
 * This size is equal to the screen size if the Window Manager doesn't
 * support large desktops, otherwise it's equal to the virtual size of
 * the desktop. The Window Manager SHOULD set this property. A Pager
 * can request a change in the desktop geometry by using this
 * function.
 * @ingroup Ecore_X_NetWM_Group
 */
EAPI void
ecore_x_netwm_desk_size_set(Ecore_X_Window root,
                            unsigned int   width,
                            unsigned int   height)
{
   uint32_t size[2];

   size[0] = width;
   size[1] = height;
   ecore_x_window_prop_card32_set(root,
                                  ECORE_X_ATOM_NET_DESKTOP_GEOMETRY,
                                  size, 2);
}

/**
 * Set the _NET_DESKTOP_VIEWPORT property.
 * @param root    The root window.
 * @param origins An array of paris of coordiantes.
 * @param n_desks The number of virtual desktops.
 *
 * Set the top left corner of each desktop's viewport by sending the
 * _NET_DESKTOP_VIEWPORT property to the @p root window. @p origins
 * contains each pair of X coordinate and Y coordinate of the top left
 * corner of each desktop's viewport.
 *
 * If the Window Manager does not support large desktops, the
 * coordinates MUST be (0,0). A Pager can request to change the
 * viewport for the current desktop by using this function.
 * @ingroup Ecore_X_NetWM_Group
 */
EAPI void
ecore_x_netwm_desk_viewports_set(Ecore_X_Window root,
                                 unsigned int  *origins,
                                 unsigned int   n_desks)
{
   ecore_x_window_prop_card32_set(root,
                                  ECORE_X_ATOM_NET_DESKTOP_VIEWPORT,
                                  origins, 2 * n_desks);
}

/**
 * Set the _NET_DESKTOP_LAYOUT property.
 * @param root The root window.
 * @param orientation
 * @param columns
 * @param rows
 * @param starting_corner
 *
 * Set the layout of virtual desktops relative to each other by
 * sending the _NET_DESKTOP_LAYOUT to the @p root window.
 * @p orientation defines the orientation of the virtual desktop. 0
 * means horizontal layout, 1 means vertical layout. @p columns is
 * the number of desktops in the X direction and @p rows is the number
 * in the Y direction. @p starting_corner is the corner containing the
 * first desktop. The values for @p starting_corner are 0 (top-left),
 * 1 (top-right), 2 (bottom-right) and 3 (bottom-left).
 *
 * When the orientation is horizontal the desktops are laid out in
 * rows, with the first desktop in the specified starting corner. So a
 * layout with four columns and three rows starting in
 * the top-left corner looks like this:
 *
 * +--+--+--+--+
 * | 0| 1| 2| 3|
 * +--+--+--+--+
 * | 4| 5| 6| 7|
 * +--+--+--+--+
 * | 8| 9|10|11|
 * +--+--+--+--+
 *
 * With @p starting_corner being bottom-right, it looks like this:
 *
 * +--+--+--+--+
 * |11|10| 9| 8|
 * +--+--+--+--+
 * | 7| 6| 5| 4|
 * +--+--+--+--+
 * | 3| 2| 1| 0|
 * +--+--+--+--+
 *
 * When the orientation is vertical the layout with four columns and
 * three rows starting in the top-left corner looks like:
 *
 * +--+--+--+--+
 * | 0| 3| 6| 9|
 * +--+--+--+--+
 * | 1| 4| 7|10|
 * +--+--+--+--+
 * | 2| 5| 8|11|
 * +--+--+--+--+
 *
 * With @p starting_corner  being top-right, it looks like:
 *
 * +--+--+--+--+
 * | 9| 6| 3| 0|
 * +--+--+--+--+
 * |10| 7| 4| 1|
 * +--+--+--+--+
 * |11| 8| 5| 2|
 * +--+--+--+--+
 *
 * This function MUST be used by a Pager and NOT by the Window
 * Manager. When using this function, the Pager must own a manager
 * selection.
 * @ingroup Ecore_X_NetWM_Group
 */
EAPI void
ecore_x_netwm_desk_layout_set(Ecore_X_Window root,
                              int            orientation,
                              int            columns,
                              int            rows,
                              int            starting_corner)
{
   uint32_t layout[4];

   layout[0] = orientation;
   layout[1] = columns;
   layout[2] = rows;
   layout[3] = starting_corner;
   ecore_x_window_prop_card32_set(root, ECORE_X_ATOM_NET_DESKTOP_LAYOUT, layout, 4);
}

/**
 * Set the _NET_WORKAREA property.
 * @param root    The root window.
 * @param areas   An array of areas.
 * @param n_desks The number of desks.
 *
 * Set the work area for each desktop by sending the _NET_WORKAREA
 * property to the @p root window. An area contains the geometry (X
 * and Y coordinates, width and height). These geometries are
 * specified relative to the viewport on each desktop and specify an
 * area that is completely contained within the viewport. @p areas
 * stores these geometries. @p n_desks is the number of geometry to
 * set.
 *
 * This function MUST be set by the Window Manager. It is used by
 * desktop applications to place desktop icons appropriately.
 * @ingroup Ecore_X_NetWM_Group
 */
EAPI void
ecore_x_netwm_desk_workareas_set(Ecore_X_Window root,
                                 unsigned int  *areas,
                                 unsigned int   n_desks)
{
   ecore_x_window_prop_card32_set(root,
                                  ECORE_X_ATOM_NET_WORKAREA,
                                  areas, 4 * n_desks);
}

/**
 * Set the _NET_CURRENT_DESKTOP property.
 * @param root The root window.
 * @param desk The index of the current desktop.
 *
 * Set the current desktop by sending the _NET_CURRENT_DESKTOP to the
 * @p root window. @p deskmust be an integer number between 0 and the
 * number of desks (set by ecore_x_netwm_desk_count_set()) -1.
 *
 * This function MUST be called by the Window Manager. If a Pagerwants
 * to switch to naother desktop, it MUST call that function.
 * @ingroup Ecore_X_NetWM_Group
 */
EAPI void
ecore_x_netwm_desk_current_set(Ecore_X_Window root,
                               unsigned int   desk)
{
   ecore_x_window_prop_card32_set(root,
                                  ECORE_X_ATOM_NET_CURRENT_DESKTOP,
                                  &desk, 1);
}

/**
 * Set the _NET_SHOWING_DESKTOP property.
 * @param root The root window
 * @param on   0 to hide the desktop, non 0 to show it.
 *
 * Set or unset the desktop in a "showing mode" by sending the
 * _NET_SHOWING_DESKTOP property to the @p root window. If @p on is 0,
 * the windows are hidden and the desktop background is displayed and
 * focused.
 *
 * If a Pager wants to enter or leave the mode, it MUST use this
 * function.
 * @ingroup Ecore_X_NetWM_Group
 */
EAPI void
ecore_x_netwm_showing_desktop_set(Ecore_X_Window root,
                                  int            on)
{
   uint32_t val;

   val = (on) ? 1 : 0;
   ecore_x_window_prop_card32_set(root,
                                  ECORE_X_ATOM_NET_SHOWING_DESKTOP,
                                  &val, 1);
}

/*
 * Client status
 */

/**
 * Set the _NET_CLIENT_LIST property.
 * @param root The root window.
 * @param p_clients An array of windows.
 * @param n_clients The number of windows.
 *
 * Map all the X windows managed by the window manager from the oldest
 * to the newest by sending the _NET_CLIENT_LIST property to the
 * @p root window. The X windows are stored in @p p_clients and their
 * number in @p n_clients.
 *
 * This function SHOULD be called by the Window Manager.
 * @ingroup Ecore_X_NetWM_Group
 */
EAPI void
ecore_x_netwm_client_list_set(Ecore_X_Window  root,
                              Ecore_X_Window *p_clients,
                              unsigned int    n_clients)
{
   ecore_x_window_prop_window_set(root,
                                  ECORE_X_ATOM_NET_CLIENT_LIST,
                                  p_clients, n_clients);
}

/**
 * Set the _NET_CLIENT_LIST_STACKING property.
 * @param root The root window.
 * @param p_clients An array of windows.
 * @param n_clients The number of windows.
 *
 * Stack all the X windows managed by the window manager from bottom
 * to top order by sending the _NET_CLIENT_LIST_STACKING property to the
 * @p root window. The X windows are stored in @p p_clients and their
 * number in @p n_clients.
 *
 * This function SHOULD be called by the Window Manager.
 * @ingroup Ecore_X_NetWM_Group
 */
EAPI void
ecore_x_netwm_client_list_stacking_set(Ecore_X_Window  root,
                                       Ecore_X_Window *p_clients,
                                       unsigned int    n_clients)
{
   ecore_x_window_prop_window_set(root,
                                  ECORE_X_ATOM_NET_CLIENT_LIST_STACKING,
                                  p_clients, n_clients);
}

/**
 * Set the _NET_ACTIVE_WINDOW property.
 * @param root   The root window.
 * @param window The widow to activate.
 *
 * Activate @p window by sending the _NET_ACTIVE_WINDOW property to
 * the @p root window.
 *
 * If a Client wants to activate another window, it MUST call this
 * function.
 * @ingroup Ecore_X_NetWM_Group
 */
EAPI void
ecore_x_netwm_client_active_set(Ecore_X_Window root,
                                Ecore_X_Window window)
{
   ecore_x_window_prop_window_set(root,
                                  ECORE_X_ATOM_NET_ACTIVE_WINDOW,
                                  &window, 1);
}

/**
 * Set the _NET_WM_NAME property.
 * @param window The widow to activate.
 * @param name   The title name of the window.
 *
 * Set the title name of @p window to @p name by sending the
 * _NET_WM_NAME property to @p window.
 *
 * The Client SHOULD set the title of @p window in UTF-8 encoding. If
 * set, the Window Manager should use this in preference to WM_NAME.
 * @ingroup Ecore_X_NetWM_Group
 */
EAPI void
ecore_x_netwm_name_set(Ecore_X_Window window,
                       const char    *name)
{
   _ecore_x_window_prop_string_utf8_set(window, ECORE_X_ATOM_NET_WM_NAME, name);
}

/**
 * Sends the GetProperty request.
 * @param window The window.
 * @ingroup Ecore_X_NetWM_Group
 */
EAPI void
ecore_x_netwm_name_get_prefetch(Ecore_X_Window window)
{
   xcb_get_property_cookie_t cookie;

   cookie = xcb_get_property_unchecked(_ecore_xcb_conn, 0, window,
                                       ECORE_X_ATOM_NET_WM_NAME, ECORE_X_ATOM_UTF8_STRING,
                                       0, 0x7fffffff);
   _ecore_xcb_cookie_cache(cookie.sequence);
}

/**
 * Gets the reply of the GetProperty request sent by ecore_x_netwm_name_get_prefetch().
 * @ingroup Ecore_X_NetWM_Group
 */
EAPI void
ecore_x_netwm_name_get_fetch(void)
{
   xcb_get_property_cookie_t cookie;
   xcb_get_property_reply_t *reply;

   cookie.sequence = _ecore_xcb_cookie_get();
   reply = xcb_get_property_reply(_ecore_xcb_conn, cookie, NULL);
   _ecore_xcb_reply_cache(reply);
}

/**
 * Get the title of a window.
 * @param window The window.
 * @param name   The title name.
 * @return       Returns always 1.
 *
 * Retrieve the title name of @p window and store it in @p name. The
 * function returns always 1.
 *
 * To use this function, you must call before, and in order,
 * ecore_x_netwm_name_get_prefetch(), which sends the GetProperty request,
 * then ecore_x_netwm_name_get_fetch(), which gets the reply.
 * @ingroup Ecore_X_NetWM_Group
 */
EAPI int
ecore_x_netwm_name_get(Ecore_X_Window window,
                       char         **name)
{
   if (name)
     *name = _ecore_x_window_prop_string_utf8_get(window, ECORE_X_ATOM_NET_WM_NAME);
   return 1;
}

/**
 * Set the _NET_STARTUP_ID property.
 * @param window The window.
 * @param id     The ID name.
 *
 * Set the ID @p id used for the startup sequence by sending the
 * property _NET_STARTUP_ID to @p window. The ID name should be
 * encoded in UTF-8.
 *
 * If a new value for the property is set, the Window Manager
 * should update the window's status accordingly (update its virtual
 * desktop, etc.).
 * @ingroup Ecore_X_NetWM_Group
 */
EAPI void
ecore_x_netwm_startup_id_set(Ecore_X_Window window,
                             const char    *id)
{
   _ecore_x_window_prop_string_utf8_set(window, ECORE_X_ATOM_NET_STARTUP_ID, id);
}

/**
 * Sends the GetProperty request.
 * @param window The window.
 * @ingroup Ecore_X_NetWM_Group
 */
EAPI void
ecore_x_netwm_startup_id_get_prefetch(Ecore_X_Window window)
{
   xcb_get_property_cookie_t cookie;

   cookie = xcb_get_property_unchecked(_ecore_xcb_conn, 0, window,
                                       ECORE_X_ATOM_NET_STARTUP_ID, ECORE_X_ATOM_UTF8_STRING,
                                       0, 0x7fffffff);
   _ecore_xcb_cookie_cache(cookie.sequence);
}

/**
 * Gets the reply of the GetProperty request sent by ecore_x_netwm_startup_id_get_prefetch().
 * @ingroup Ecore_X_NetWM_Group
 */
EAPI void
ecore_x_netwm_startup_id_get_fetch(void)
{
   xcb_get_property_cookie_t cookie;
   xcb_get_property_reply_t *reply;

   cookie.sequence = _ecore_xcb_cookie_get();
   reply = xcb_get_property_reply(_ecore_xcb_conn, cookie, NULL);
   _ecore_xcb_reply_cache(reply);
}

/**
 * Get the startup ID name of a window.
 * @param window The window
 * @param id     The ID name
 * @return       Return always 1.
 *
 * To use this function, you must call before, and in order,
 * ecore_x_netwm_startup_id_get_prefetch(), which sends the GetProperty request,
 * then ecore_x_netwm_startup_id_get_fetch(), which gets the reply.
 * @ingroup Ecore_X_NetWM_Group
 */
EAPI int
ecore_x_netwm_startup_id_get(Ecore_X_Window window,
                             char         **id)
{
   if (id)
     *id = _ecore_x_window_prop_string_utf8_get(window, ECORE_X_ATOM_NET_STARTUP_ID);
   return 1;
}

/**
 * Set the _NET_WM_VISIBLE_NAME property.
 * @param window The widow to activate.
 * @param name   The title name of the window.
 *
 * Set the title name of @p window to @p name by sending the
 * _NET_WM_VISIBLE_NAME property to @p window, when the Window Manager
 * displays a window name other than by calling
 * ecore_x_netwm_name_set().
 *
 * The Client SHOULD set the title of @p window in UTF-8
 * encoding. This function is used for displaying title windows like
 * [xterm1], [xterm2], ... thereby allowing Pagers to display the same
 * title as the Window Manager.
 * @ingroup Ecore_X_NetWM_Group
 */
EAPI void
ecore_x_netwm_visible_name_set(Ecore_X_Window window,
                               const char    *name)
{
   _ecore_x_window_prop_string_utf8_set(window, ECORE_X_ATOM_NET_WM_VISIBLE_NAME,
                                        name);
}

/**
 * Sends the GetProperty request.
 * @param window The window.
 * @ingroup Ecore_X_NetWM_Group
 */
EAPI void
ecore_x_netwm_visible_name_get_prefetch(Ecore_X_Window window)
{
   xcb_get_property_cookie_t cookie;

   cookie = xcb_get_property_unchecked(_ecore_xcb_conn, 0, window,
                                       ECORE_X_ATOM_NET_WM_VISIBLE_NAME, ECORE_X_ATOM_UTF8_STRING,
                                       0, 0x7fffffff);
   _ecore_xcb_cookie_cache(cookie.sequence);
}

/**
 * Gets the reply of the GetProperty request sent by ecore_x_netwm_visible_name_get_prefetch().
 * @ingroup Ecore_X_NetWM_Group
 */
EAPI void
ecore_x_netwm_visible_name_get_fetch(void)
{
   xcb_get_property_cookie_t cookie;
   xcb_get_property_reply_t *reply;

   cookie.sequence = _ecore_xcb_cookie_get();
   reply = xcb_get_property_reply(_ecore_xcb_conn, cookie, NULL);
   _ecore_xcb_reply_cache(reply);
}

/**
 * Get the visible title of a window.
 * @param window The window.
 * @param name   The title name.
 * @return       Returns always 1.
 *
 * Retrieve the visible title name of @p window and store it in @p name. The
 * function returns always 1.
 * @param window The window
 *
 * To use this function, you must call before, and in order,
 * ecore_x_netwm_visible_name_get_prefetch(), which sends the GetProperty request,
 * then ecore_x_netwm_visible_name_get_fetch(), which gets the reply.
 * @ingroup Ecore_X_NetWM_Group
 */
EAPI int
ecore_x_netwm_visible_name_get(Ecore_X_Window window,
                               char         **name)
{
   if (name)
     *name = _ecore_x_window_prop_string_utf8_get(window,
                                                  ECORE_X_ATOM_NET_WM_VISIBLE_NAME);
   return 1;
}

/**
 * Set the _NET_WM_ICON_NAME property.
 * @param window The widow to activate.
 * @param name   The icon name of the window.
 *
 * Set the icon name of @p window to @p name by sending the
 * _NET_WM_ICON_NAME property to @p window.
 *
 * The Client SHOULD set the title of @p window in UTF-8 encoding. If
 * set, the Window Manager should use this in preference to WM_ICON_NAME.
 * @ingroup Ecore_X_NetWM_Group
 */
EAPI void
ecore_x_netwm_icon_name_set(Ecore_X_Window window,
                            const char    *name)
{
   _ecore_x_window_prop_string_utf8_set(window, ECORE_X_ATOM_NET_WM_ICON_NAME,
                                        name);
}

/**
 * Sends the GetProperty request.
 * @param window The window.
 * @ingroup Ecore_X_NetWM_Group
 */
EAPI void
ecore_x_netwm_icon_name_get_prefetch(Ecore_X_Window window)
{
   xcb_get_property_cookie_t cookie;

   cookie = xcb_get_property_unchecked(_ecore_xcb_conn, 0, window,
                                       ECORE_X_ATOM_NET_WM_ICON_NAME, ECORE_X_ATOM_UTF8_STRING,
                                       0, 0x7fffffff);
   _ecore_xcb_cookie_cache(cookie.sequence);
}

/**
 * Gets the reply of the GetProperty request sent by ecore_x_netwm_icon_name_get_prefetch().
 * @ingroup Ecore_X_NetWM_Group
 */
EAPI void
ecore_x_netwm_icon_name_get_fetch(void)
{
   xcb_get_property_cookie_t cookie;
   xcb_get_property_reply_t *reply;

   cookie.sequence = _ecore_xcb_cookie_get();
   reply = xcb_get_property_reply(_ecore_xcb_conn, cookie, NULL);
   _ecore_xcb_reply_cache(reply);
}

/**
 * Get the icon name of a window.
 * @param window The window.
 * @param name   The icon name.
 * @return       Returns always 1.
 *
 * Retrieve the icon name of @p window and store it in @p name. The
 * function returns always 1.
 *
 * To use this function, you must call before, and in order,
 * ecore_x_netwm_icon_name_get_prefetch(), which sends the GetProperty request,
 * then ecore_x_netwm_icon_name_get_fetch(), which gets the reply.
 * @ingroup Ecore_X_NetWM_Group
 */
EAPI int
ecore_x_netwm_icon_name_get(Ecore_X_Window window,
                            char         **name)
{
   if (name)
     *name = _ecore_x_window_prop_string_utf8_get(window,
						  ECORE_X_ATOM_NET_WM_ICON_NAME);
   return 1;
}

/**
 * Set the _NET_WM_VISIBLE_ICON_NAME property.
 * @param window The widow to activate.
 * @param name   The title name of the window.
 *
 * Set the icon name of @p window to @p name by sending the
 * _NET_WM_VISIBLE_ICON_NAME property to @p window, when the Window Manager
 * displays a icon name other than by calling
 * ecore_x_netwm_icon_name_set().
 *
 * The Client SHOULD set the icon name in UTF-8
 * encoding. The Window Manager MUST use this function is it display
 * an icon name other than with ecore_x_netwm_icon_name_set().
 * @ingroup Ecore_X_NetWM_Group
 */
EAPI void
ecore_x_netwm_visible_icon_name_set(Ecore_X_Window window,
                                    const char    *name)
{
   _ecore_x_window_prop_string_utf8_set(window,
                                        ECORE_X_ATOM_NET_WM_VISIBLE_ICON_NAME,
                                        name);
}

/**
 * Sends the GetProperty request.
 * @param window The window.
 * @ingroup Ecore_X_NetWM_Group
 */
EAPI void
ecore_x_netwm_visible_icon_name_get_prefetch(Ecore_X_Window window)
{
   xcb_get_property_cookie_t cookie;

   cookie = xcb_get_property_unchecked(_ecore_xcb_conn, 0, window,
                                       ECORE_X_ATOM_NET_WM_VISIBLE_ICON_NAME, ECORE_X_ATOM_UTF8_STRING,
                                       0, 0x7fffffff);
   _ecore_xcb_cookie_cache(cookie.sequence);
}

/**
 * Gets the reply of the GetProperty request sent by ecore_x_netwm_visible_icon_name_get_prefetch().
 * @ingroup Ecore_X_NetWM_Group
 */
EAPI void
ecore_x_netwm_visible_icon_name_get_fetch(void)
{
   xcb_get_property_cookie_t cookie;
   xcb_get_property_reply_t *reply;

   cookie.sequence = _ecore_xcb_cookie_get();
   reply = xcb_get_property_reply(_ecore_xcb_conn, cookie, NULL);
   _ecore_xcb_reply_cache(reply);
}

/**
 * Get the visible icon name of a window.
 * @param window The window.
 * @param name   The icon name.
 * @return       Returns always 1.
 *
 * Retrieve the visible icon name of @p window and store it in
 * @p name. The function returns always 1.
 *
 * To use this function, you must call before, and in order,
 * ecore_x_netwm_visible_icon_name_get_prefetch(), which sends the GetProperty request,
 * then ecore_x_netwm_visible_icon_name_get_fetch(), which gets the reply.
 * @ingroup Ecore_X_NetWM_Group
 */
EAPI int
ecore_x_netwm_visible_icon_name_get(Ecore_X_Window window,
                                    char         **name)
{
   if (name)
     *name = _ecore_x_window_prop_string_utf8_get(window,
                                                  ECORE_X_ATOM_NET_WM_VISIBLE_ICON_NAME);
   return 1;
}

/**
 * Set the _NET_WM_DESKTOP property.
 * @param window The window.
 * @param desk   The desktop index.
 *
 * Set on which desktop the @p window is in by sending the
 * _NET_WM_DESKTOP property to @p window. @p desk is the index of
 * the desktop, starting from 0. To indicate that the window should
 * appear on all desktops, @p desk must be equal to 0xFFFFFFFF.
 *
 * A Client MAY choose not to set this property, in which case the
 * Window Manager SHOULD place it as it wishes.
 *
 * The Window Manager should honor _NET_WM_DESKTOP whenever a
 * withdrawn window requests to be mapped.
 *
 * A Client can request a change of desktop for a non-withdrawn window
 * by sending a _NET_WM_DESKTOP client message to the root window.
 * @ingroup Ecore_X_NetWM_Group
 */
EAPI void
ecore_x_netwm_desktop_set(Ecore_X_Window window,
                          unsigned int   desk)
{
   ecore_x_window_prop_card32_set(window, ECORE_X_ATOM_NET_WM_DESKTOP, &desk, 1);
}

/**
 * Sends the GetProperty request.
 * @param window The window.
 * @ingroup Ecore_X_NetWM_Group
 */
EAPI void
ecore_x_netwm_desktop_get_prefetch(Ecore_X_Window window)
{
   xcb_get_property_cookie_t cookie;

   cookie = xcb_get_property_unchecked(_ecore_xcb_conn, 0, window,
                                       ECORE_X_ATOM_NET_WM_DESKTOP, ECORE_X_ATOM_CARDINAL,
                                       0, 0x7fffffff);
   _ecore_xcb_cookie_cache(cookie.sequence);
}

/**
 * Gets the reply of the GetProperty request sent by ecore_x_netwm_desktop_get_prefetch().
 * @ingroup Ecore_X_NetWM_Group
 */
EAPI void
ecore_x_netwm_desktop_get_fetch(void)
{
   xcb_get_property_cookie_t cookie;
   xcb_get_property_reply_t *reply;

   cookie.sequence = _ecore_xcb_cookie_get();
   reply = xcb_get_property_reply(_ecore_xcb_conn, cookie, NULL);
   _ecore_xcb_reply_cache(reply);
}

/**
 * Get the visible icon name of a window.
 * @param window The window.
 * @param desk   The desktop index.
 * @return       1 on success, 0 otherwise.
 *
 * Retrieve the desktop index in which @p window is displayed and
 * store it in @p desk. If @p desk value is 0xFFFFFFFF, the window
 * appears on all desktops. The function returns 1 on success, 0
 * otherwise.
 *
 * To use this function, you must call before, and in order,
 * ecore_x_netwm_desktop_get_prefetch(), which sends the GetProperty request,
 * then ecore_x_netwm_desktop_get_fetch(), which gets the reply.
 * @ingroup Ecore_X_NetWM_Group
 */
EAPI int
ecore_x_netwm_desktop_get(Ecore_X_Window window,
                          unsigned int *desk)
{
   int      ret;
   uint32_t tmp;

   ret = ecore_x_window_prop_card32_get(window, ECORE_X_ATOM_NET_WM_DESKTOP,
					&tmp, 1);

   if (desk) *desk = tmp;
   return (ret == 1) ? 1 : 0;
}

/**
 * Set the _NET_WM_STRUT property.
 * @param window The window
 * @param left   The number of pixels at the left of the screen.
 * @param right  The number of pixels at the right of the screen.
 * @param top    The number of pixels at the top of the screen.
 * @param bottom The number of pixels at the bottom of the screen.
 *
 * Set space at the edje of the screen by sending the _NET_WM_STRUT
 * property to @p window if @p window is to reserve that space.
 * @p left, @p right, @p top and @p bottom are respectively the number
 * of pixels at the left, right, top and bottom of the screen.
 *
 * This property is deprecated and ecore_x_netwm_strut_partial_set()
 * should be used instead. However, Clients MAY set this property in
 * addition to _NET_WM_STRUT_PARTIAL to ensure backward compatibility
 * with Window Managers supporting older versions of the
 * Specification.
 * @ingroup Ecore_X_NetWM_Group
 */
EAPI void
ecore_x_netwm_strut_set(Ecore_X_Window window,
                        int            left,
                        int            right,
                        int            top,
                        int            bottom)
{
   uint32_t strut[4];

   strut[0] = left;
   strut[1] = right;
   strut[2] = top;
   strut[3] = bottom;
   ecore_x_window_prop_card32_set(window, ECORE_X_ATOM_NET_WM_STRUT, strut, 4);
}

/**
 * Sends the GetProperty request.
 * @param window The window.
 * @ingroup Ecore_X_NetWM_Group
 */
EAPI void
ecore_x_netwm_strut_get_prefetch(Ecore_X_Window window)
{
   xcb_get_property_cookie_t cookie;

   cookie = xcb_get_property_unchecked(_ecore_xcb_conn, 0, window,
                                       ECORE_X_ATOM_NET_WM_STRUT, ECORE_X_ATOM_CARDINAL,
                                       0, 0x7fffffff);
   _ecore_xcb_cookie_cache(cookie.sequence);
}

/**
 * Gets the reply of the GetProperty request sent by ecore_x_strut_get_prefetch().
 * @ingroup Ecore_X_NetWM_Group
 */
EAPI void
ecore_x_netwm_strut_get_fetch(void)
{
   xcb_get_property_cookie_t cookie;
   xcb_get_property_reply_t *reply;

   cookie.sequence = _ecore_xcb_cookie_get();
   reply = xcb_get_property_reply(_ecore_xcb_conn, cookie, NULL);
   _ecore_xcb_reply_cache(reply);
}

/*
 * _NET_WM_STRUT is deprecated
 */

/**
 * Get the space at the edje of the screen.
 * @param window The window
 * @param left   The number of pixels at the left of the screen.
 * @param right  The number of pixels at the right of the screen.
 * @param top    The number of pixels at the top of the screen.
 * @param bottom The number of pixels at the bottom of the screen.
 * @return       1 on success, 0 otherwise.
 *
 * Retrieve the space at the edje of the screen if @p window is to
 * reserve such space. The number of pixels at the left, right, top
 * and bottom of the screen are respectively stored in @p left,
 * @p right, @p top and @p bottom. This function returns 1 on success,
 * 0 otherwise.
 *
 * This property is deprecated. See ecore_x_netwm_strut_set() for more
 * informations.
 *
 * To use this function, you must call before, and in order,
 * ecore_x_netwm_strut_get_prefetch(), which sends the GetProperty request,
 * then ecore_x_netwm_strut_get_fetch(), which gets the reply.
 * @ingroup Ecore_X_NetWM_Group
 */
EAPI int
ecore_x_netwm_strut_get(Ecore_X_Window window,
                        int           *left,
                        int           *right,
                        int           *top,
                        int           *bottom)
{
   uint32_t strut[4];
   int      ret = 0;

   ret = ecore_x_window_prop_card32_get(window, ECORE_X_ATOM_NET_WM_STRUT, strut, 4);
   if (ret != 4)
     return 0;

   if (left)   *left   = strut[0];
   if (right)  *right  = strut[1];
   if (top)    *top    = strut[2];
   if (bottom) *bottom = strut[3];

   return 1;
}

/**
 * Set the _NET_WM_STRUT_PARTIAL property.
 * @param window         The window
 * @param left           The number of pixels at the left of the screen.
 * @param right          The number of pixels at the right of the screen.
 * @param top            The number of pixels at the top of the screen.
 * @param bottom         The number of pixels at the bottom of the screen.
 * @param left_start_y   The number of pixels.
 * @param left_end_y     The number of pixels.
 * @param right_start_y  The number of pixels.
 * @param right_end_y    The number of pixels.
 * @param top_start_x    The number of pixels.
 * @param top_end_x      The number of pixels.
 * @param bottom_start_x The number of pixels.
 * @param bottom_end_x   The number of pixels.
 *
 * Set space at the edje of the screen by sending the
 * _NET_WM_STRUT_PARTIAL property to @p window if @p window is to
 * reserve that space. @p left, @p right, @p top and @p bottom are
 * respectively the number of pixels at the left, right, top and
 * bottom of the screen.
 *
 * TODO: more description for that function.
 * @ingroup Ecore_X_NetWM_Group
 */
EAPI void
ecore_x_netwm_strut_partial_set(Ecore_X_Window window,
                                int            left,
                                int            right,
                                int            top,
                                int            bottom,
                                int            left_start_y,
                                int            left_end_y,
                                int            right_start_y,
                                int            right_end_y,
                                int            top_start_x,
                                int            top_end_x,
                                int            bottom_start_x,
                                int            bottom_end_x)
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
   ecore_x_window_prop_card32_set(window, ECORE_X_ATOM_NET_WM_STRUT_PARTIAL, strut, 12);
}

/**
 * Sends the GetProperty request.
 * @param window The window.
 * @ingroup Ecore_X_NetWM_Group
 */
EAPI void
ecore_x_netwm_strut_partial_get_prefetch(Ecore_X_Window window)
{
   xcb_get_property_cookie_t cookie;

   cookie = xcb_get_property_unchecked(_ecore_xcb_conn, 0, window,
                                       ECORE_X_ATOM_NET_WM_STRUT_PARTIAL, ECORE_X_ATOM_CARDINAL,
                                       0, 0x7fffffff);
   _ecore_xcb_cookie_cache(cookie.sequence);
}

/**
 * Gets the reply of the GetProperty request sent by ecore_x_strut_partial_get_prefetch().
 * @ingroup Ecore_X_NetWM_Group
 */
EAPI void
ecore_x_netwm_strut_partial_get_fetch(void)
{
   xcb_get_property_cookie_t cookie;
   xcb_get_property_reply_t *reply;

   cookie.sequence = _ecore_xcb_cookie_get();
   reply = xcb_get_property_reply(_ecore_xcb_conn, cookie, NULL);
   _ecore_xcb_reply_cache(reply);
}

/**
 * Get the space at the edje of the screen of a window.
 * @param window         The window
 * @param left           The number of pixels at the left of the screen.
 * @param right          The number of pixels at the right of the screen.
 * @param top            The number of pixels at the top of the screen.
 * @param bottom         The number of pixels at the bottom of the screen.
 * @param left_start_y   The number of pixels.
 * @param left_end_y     The number of pixels.
 * @param right_start_y  The number of pixels.
 * @param right_end_y    The number of pixels.
 * @param top_start_x    The number of pixels.
 * @param top_end_x      The number of pixels.
 * @param bottom_start_x The number of pixels.
 * @param bottom_end_x   The number of pixels.
 *
 * Retrieve the space at the edje of the screen if @p window is to
 * reserve such space. The number of pixels at the left, right, top
 * and bottom of the screen are respectively stored in @p left,
 * @p right, @p top and @p bottom. This function returns 1 on success,
 * 0 otherwise.
 *
 * TODO: more description for that function.
 *
 * To use this function, you must call before, and in order,
 * ecore_x_netwm_strut_partial_get_prefetch(), which sends the GetProperty request,
 * then ecore_x_netwm_strut_partial_get_fetch(), which gets the reply.
 * @ingroup Ecore_X_NetWM_Group
 */
EAPI int
ecore_x_netwm_strut_partial_get(Ecore_X_Window window,
                                int           *left,
                                int           *right,
                                int           *top,
                                int           *bottom,
                                int           *left_start_y,
                                int           *left_end_y,
                                int           *right_start_y,
                                int           *right_end_y,
                                int           *top_start_x,
                                int           *top_end_x,
                                int           *bottom_start_x,
                                int           *bottom_end_x)
{
   uint32_t strut[12];
   int      ret = 0;

   ret = ecore_x_window_prop_card32_get(window, ECORE_X_ATOM_NET_WM_STRUT_PARTIAL, strut, 12);
   if (ret != 12)
     return 0;

   if (left)           *left           = strut[0];
   if (right)          *right          = strut[1];
   if (top)            *top            = strut[2];
   if (bottom)         *bottom         = strut[3];
   if (left_start_y)   *left_start_y   = strut[4];
   if (left_end_y)     *left_end_y     = strut[5];
   if (right_start_y)  *right_start_y  = strut[6];
   if (right_end_y)    *right_end_y    = strut[7];
   if (top_start_x)    *top_start_x    = strut[8];
   if (top_end_x)      *top_end_x      = strut[9];
   if (bottom_start_x) *bottom_start_x = strut[10];
   if (bottom_end_x)   *bottom_end_x   = strut[11];
   return 1;
}

/**
 * Sends the GetProperty request.
 * @param window The window.
 * @ingroup Ecore_X_NetWM_Group
 */
EAPI void
ecore_x_netwm_icons_get_prefetch(Ecore_X_Window window)
{
   xcb_get_property_cookie_t cookie;

   cookie = xcb_get_property_unchecked(_ecore_xcb_conn, 0, window,
                                       ECORE_X_ATOM_NET_WM_ICON, ECORE_X_ATOM_CARDINAL,
                                       0, 0x7fffffff);
   _ecore_xcb_cookie_cache(cookie.sequence);
}

/**
 * Gets the reply of the GetProperty request sent by ecore_x_icons_get_prefetch().
 * @ingroup Ecore_X_NetWM_Group
 */
EAPI void
ecore_x_netwm_icons_get_fetch(void)
{
   xcb_get_property_cookie_t cookie;
   xcb_get_property_reply_t *reply;

   cookie.sequence = _ecore_xcb_cookie_get();
   reply = xcb_get_property_reply(_ecore_xcb_conn, cookie, NULL);
   _ecore_xcb_reply_cache(reply);
}

/**
 * Retrieve hte possible icons of a window.
 * @param window The window
 * @param icon   An array of icons.
 * @param num    The number of icons.
 * @return       1 on success, 0 otherwise.
 *
 * Retrieve an array of possible icons of @p window. The icons are
 * stored in @p icon and their number in @p num.
 *
 * To use this function, you must call before, and in order,
 * ecore_x_netwm_icons_get_prefetch(), which sends the GetProperty request,
 * then ecore_x_netwm_icons_get_fetch(), which gets the reply.
 * @ingroup Ecore_X_NetWM_Group
 */
EAPI int
ecore_x_netwm_icons_get(Ecore_X_Window window,
                        Ecore_X_Icon **icon,
                        int           *num)
{
   uint32_t *data;
   uint32_t *p;
   uint32_t *src;
   uint32_t  icons;
   uint32_t  len;
   uint32_t  i;
   int       num_ret;

   if (num) *num = 0UL;
   if (icon) *icon = NULL;

   num_ret = ecore_x_window_prop_card32_list_get(window,
                                                 ECORE_X_ATOM_NET_WM_ICON,
                                                 &data);
   if ((num_ret <= 0) || !data)
     return 0;

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
	uint32_t *ps, *pd, *pe;

	len = p[0] * p[1];
	((*icon)[i]).width = p[0];
	((*icon)[i]).height = p[1];
	src = &(p[2]);
	((*icon)[i]).data = malloc(len * sizeof(uint32_t));
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
	     uint32_t r, g, b, a;

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

/**
 * Set the _NET_WM_ICON_GEOMETRY property.
 * @param window The window.
 * @param x      The X coordinate of the icon.
 * @param y      The Y coordinate of the icon.
 * @param width  The width of the icon.
 * @param height The height of the icon.
 *
 * Set the geometry of the icon of @p window by sending the
 * _NET_WM_ICON_GEOMETRY property to @p window. @p x, @p y, @p width
 * and @p height specify respectively the X coordinate, the Y
 * coordinate, the width and the height of the icon.
 *
 * Stand alone tools like a taskbar or an iconbox MAY use this
 * function. This functions makes possible for a Window Manager to
 * display a nice animation like morphing the window into its icon.
 * @ingroup Ecore_X_NetWM_Group
 */
EAPI void
ecore_x_netwm_icon_geometry_set(Ecore_X_Window window,
                                int            x,
                                int            y,
                                int            width,
                                int            height)
{
   uint32_t geometry[4];

   geometry[0] = (uint32_t)x;
   geometry[1] = (uint32_t)y;
   geometry[2] = (uint32_t)width;
   geometry[3] = (uint32_t)height;
   ecore_x_window_prop_card32_set(window, ECORE_X_ATOM_NET_WM_ICON_GEOMETRY, geometry, 4);
}

/**
 * Sends the GetProperty request.
 * @param window The window.
 * @ingroup Ecore_X_NetWM_Group
 */
EAPI void
ecore_x_netwm_icon_geometry_get_prefetch(Ecore_X_Window window)
{
   xcb_get_property_cookie_t cookie;

   cookie = xcb_get_property_unchecked(_ecore_xcb_conn, 0, window,
                                       ECORE_X_ATOM_NET_WM_ICON_GEOMETRY, ECORE_X_ATOM_CARDINAL,
                                       0, 0x7fffffff);
   _ecore_xcb_cookie_cache(cookie.sequence);
}

/**
 * Gets the reply of the GetProperty request sent by ecore_x_icon_geometry_get_prefetch().
 * @ingroup Ecore_X_NetWM_Group
 */
EAPI void
ecore_x_netwm_icon_geometry_get_fetch(void)
{
   xcb_get_property_cookie_t cookie;
   xcb_get_property_reply_t *reply;

   cookie.sequence = _ecore_xcb_cookie_get();
   reply = xcb_get_property_reply(_ecore_xcb_conn, cookie, NULL);
   _ecore_xcb_reply_cache(reply);
}

/**
 * Get the geometry of an icon.
 * @param window The window
 * @param x      x
 * @param x      The X coordinate of the icon.
 * @param y      The Y coordinate of the icon.
 * @param width  The width of the icon.
 * @param height The height of the icon.
 * @return       1 on success, 0 othrwise.
 *
 * Retrieve the geometry of the icon of @p window. The geometry is
 * stored in @p x, @p y, @p width and @p height. The function returns
 * 1 on success, 0 otherwise.
 *
 * To use this function, you must call before, and in order,
 * ecore_x_netwm_icon_geometry_get_prefetch(), which sends the GetProperty request,
 * then ecore_x_netwm_icon_geometry_get_fetch(), which gets the reply.
 * @ingroup Ecore_X_NetWM_Group
 */
EAPI int
ecore_x_netwm_icon_geometry_get(Ecore_X_Window window,
                                int           *x,
                                int           *y,
                                int           *width,
                                int           *height)
{
   uint32_t geometry[4];
   int      ret;

   ret = ecore_x_window_prop_card32_get(window, ECORE_X_ATOM_NET_WM_ICON_GEOMETRY, geometry, 4);
   if (ret != 4)
     return 0;

   if (x)      *x      = geometry[0];
   if (y)      *y      = geometry[1];
   if (width)  *width  = geometry[2];
   if (height) *height = geometry[3];

   return 1;
}

/**
 * Set the _NET_WM_PID property.
 * @param window The window.
 * @param pid    The process ID.
 *
 * Set the process ID of the client owning @p window by sending the
 * _NET_WM_PID property to @p window.
 *
 * This function MAY be used by the Window Manager to kill windows
 * which do not respond to the _NET_WM_PING protocol.
 *
 * If _NET_WM_PID is set, the ICCCM-specified property
 * WM_CLIENT_MACHINE MUST also be set.
 * @ingroup Ecore_X_NetWM_Group
 */
EAPI void
ecore_x_netwm_pid_set(Ecore_X_Window window,
                      int            pid)
{
   unsigned int tmp;

   tmp = pid;
   ecore_x_window_prop_card32_set(window, ECORE_X_ATOM_NET_WM_PID,
				  &tmp, 1);
}

/**
 * Sends the GetProperty request.
 * @param window The window.
 * @ingroup Ecore_X_NetWM_Group
 */
EAPI void
ecore_x_netwm_pid_get_prefetch(Ecore_X_Window window)
{
   xcb_get_property_cookie_t cookie;

   cookie = xcb_get_property_unchecked(_ecore_xcb_conn, 0, window,
                                       ECORE_X_ATOM_NET_WM_PID, ECORE_X_ATOM_CARDINAL,
                                       0, 0x7fffffff);
   _ecore_xcb_cookie_cache(cookie.sequence);
}

/**
 * Gets the reply of the GetProperty request sent by ecore_x_pid_get_prefetch().
 * @ingroup Ecore_X_NetWM_Group
 */
EAPI void
ecore_x_netwm_pid_get_fetch(void)
{
   xcb_get_property_cookie_t cookie;
   xcb_get_property_reply_t *reply;

   cookie.sequence = _ecore_xcb_cookie_get();
   reply = xcb_get_property_reply(_ecore_xcb_conn, cookie, NULL);
   _ecore_xcb_reply_cache(reply);
}

/**
 * Get the process ID of a client window.
 * @param window The window.
 * @param pid    The process ID.
 * @return       1 on success, 0 otherwise.
 *
 * Retrieve the process ID of @p window and store it in @p pid. This
 * function returns 1 on success, 0 otherwise.
 *
 * To use this function, you must call before, and in order,
 * ecore_x_netwm_pid_get_prefetch(), which sends the GetProperty request,
 * then ecore_x_netwm_pid_get_fetch(), which gets the reply.
 * @ingroup Ecore_X_NetWM_Group
 */
EAPI int
ecore_x_netwm_pid_get(Ecore_X_Window window,
                      int           *pid)
{
   int ret;
   uint32_t tmp;

   ret = ecore_x_window_prop_card32_get(window, ECORE_X_ATOM_NET_WM_PID,
					&tmp, 1);
   if (pid) *pid = tmp;

   return (ret == 1) ? 1 : 0;
}

/**
 * Set the _NET_WM_HANDLED_ICONS property.
 * @param window The window.
 *
 * Indicate to the Window Manager that it does not need to provide
 * icons for the iconified @p window by sending the
 * _NET_WM_HANDLED_ICONS property to @p window.
 *
 * This function can be used by a Pager on one of its own toplevel
 * windows (for example if the Client is a taskbar and provides
 * buttons for iconified windows).
 * @ingroup Ecore_X_NetWM_Group
 */
EAPI void
ecore_x_netwm_handled_icons_set(Ecore_X_Window window)
{
   ecore_x_window_prop_card32_set(window, ECORE_X_ATOM_NET_WM_HANDLED_ICONS,
				  NULL, 0);
}

/**
 * Sends the GetProperty request.
 * @param window The window.
 * @ingroup Ecore_X_NetWM_Group
 */
EAPI void
ecore_x_netwm_handled_icons_get_prefetch(Ecore_X_Window window)
{
   xcb_get_property_cookie_t cookie;

   cookie = xcb_get_property_unchecked(_ecore_xcb_conn, 0, window,
                                       ECORE_X_ATOM_NET_WM_HANDLED_ICONS, ECORE_X_ATOM_CARDINAL,
                                       0, 0x7fffffff);
   _ecore_xcb_cookie_cache(cookie.sequence);
}

/**
 * Gets the reply of the GetProperty request sent by ecore_x_handled_icons_get_prefetch().
 * @ingroup Ecore_X_NetWM_Group
 */
EAPI void
ecore_x_netwm_handled_icons_get_fetch(void)
{
   xcb_get_property_cookie_t cookie;
   xcb_get_property_reply_t *reply;

   cookie.sequence = _ecore_xcb_cookie_get();
   reply = xcb_get_property_reply(_ecore_xcb_conn, cookie, NULL);
   _ecore_xcb_reply_cache(reply);
}

/**
 * Return wheter the Client handles icons or not.
 * @param window The window.
 * @return       1 if icons are handled, 0 otherwise.
 *
 * Return whether the client handles icons or not if @p window is
 * iconified.
 *
 * To use this function, you must call before, and in order,
 * ecore_x_netwm_handled_icons_get_prefetch(), which sends the GetProperty request,
 * then ecore_x_netwm_handled_icons_get_fetch(), which gets the reply.
 * @ingroup Ecore_X_NetWM_Group
 */
EAPI int
ecore_x_netwm_handled_icons_get(Ecore_X_Window window)
{
   int ret = 0;
   ret = ecore_x_window_prop_card32_get(window, ECORE_X_ATOM_NET_WM_HANDLED_ICONS,
					NULL, 0);
   return (ret == 0) ? 1 : 0;
}

/**
 * Set the _NET_WM_USER_TIME property.
 * @param window The window.
 * @param time   The last user activity time in the window.
 *
 * Set the XServer time at which last user activity in @p window took
 * place by sending the _NET_WM_USER_TIME property to @p window. @p
 * time contains that XServer time in seconds.
 *
 * This function allows a Window Manager to alter the focus, stacking,
 * and/or placement behavior of windows when they are mapped depending
 * on whether the new window was created by a user action or is a
 * "pop-up" window activated by a timer or some other event.
 * @ingroup Ecore_X_NetWM_Group
 */
EAPI void
ecore_x_netwm_user_time_set(Ecore_X_Window window,
                            unsigned int   time)
{
   ecore_x_window_prop_card32_set(window, ECORE_X_ATOM_NET_WM_USER_TIME,
				  &time, 1);
}

/**
 * Sends the GetProperty request.
 * @param window The window.
 * @ingroup Ecore_X_NetWM_Group
 */
EAPI void
ecore_x_netwm_user_time_get_prefetch(Ecore_X_Window window)
{
   xcb_get_property_cookie_t cookie;

   cookie = xcb_get_property_unchecked(_ecore_xcb_conn, 0, window,
                                       ECORE_X_ATOM_NET_WM_USER_TIME, ECORE_X_ATOM_CARDINAL,
                                       0, 0x7fffffff);
   _ecore_xcb_cookie_cache(cookie.sequence);
}

/**
 * Gets the reply of the GetProperty request sent by ecore_x_netwm_user_time_get_prefetch().
 * @ingroup Ecore_X_NetWM_Group
 */
EAPI void
ecore_x_netwm_user_time_get_fetch(void)
{
   xcb_get_property_cookie_t cookie;
   xcb_get_property_reply_t *reply;

   cookie.sequence = _ecore_xcb_cookie_get();
   reply = xcb_get_property_reply(_ecore_xcb_conn, cookie, NULL);
   _ecore_xcb_reply_cache(reply);
}

/**
 * Get the last user activity time in the window.
 * @param window The window.
 * @param time   The returned time.
 * @return       1 on success, 0 otherwise.
 *
 * Return the XServer time at which last user activity in @p window
 * took place. The time is stored in @p time.
 *
 * To use this function, you must call before, and in order,
 * ecore_x_netwm_user_time_get_prefetch(), which sends the GetProperty request,
 * then ecore_x_netwm_user_time_get_fetch(), which gets the reply.
 * @ingroup Ecore_X_NetWM_Group
 */
EAPI int
ecore_x_netwm_user_time_get(Ecore_X_Window window,
                            unsigned int  *time)
{
   int ret;
   uint32_t tmp;

   ret = ecore_x_window_prop_card32_get(window, ECORE_X_ATOM_NET_WM_USER_TIME,
					&tmp, 1);
   if (time) *time = tmp;
   return (ret == 1) ? 1 : 0;
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

/**
 * Set the _NET_WM_STATE property.
 * @param window The window.
 * @param state  An array of window hints.
 * @param num    The number of hints.
 *
 * Set a list of hints describing @p window state by sending the
 * _NET_WM_STATE property to @p window. The hints are stored in the
 * array @p state. @p num must contain the number of hints.
 *
 * The Window Manager SHOULD honor _NET_WM_STATE whenever a withdrawn
 * window requests to be mapped. A Client wishing to change the state
 * of a window MUST send a _NET_WM_STATE client message to the root
 * window. The Window Manager MUST keep this property updated to
 * reflect the current state of the window.
 * @ingroup Ecore_X_NetWM_Group
 */
EAPI void
ecore_x_netwm_window_state_set(Ecore_X_Window        window,
                               Ecore_X_Window_State *state,
                               unsigned int          num)
{
   Ecore_X_Atom  *set;
   uint32_t       i;

   if (!num)
     {
	ecore_x_window_prop_property_del(window, ECORE_X_ATOM_NET_WM_STATE);
	return;
     }

   set = malloc(num * sizeof(Ecore_X_Atom));
   if (!set) return;

   for (i = 0; i < num; i++)
     set[i] = _ecore_x_netwm_state_atom_get(state[i]);

   ecore_x_window_prop_atom_set(window, ECORE_X_ATOM_NET_WM_STATE, set, num);

   free(set);
}

/**
 * Sends the GetProperty request.
 * @param window The window.
 * @ingroup Ecore_X_NetWM_Group
 */
EAPI void
ecore_x_netwm_window_state_get_prefetch(Ecore_X_Window window)
{
   xcb_get_property_cookie_t cookie;

   cookie = xcb_get_property_unchecked(_ecore_xcb_conn, 0, window,
                                       ECORE_X_ATOM_NET_WM_STATE, ECORE_X_ATOM_ATOM,
                                       0, 0x7fffffff);;
   _ecore_xcb_cookie_cache(cookie.sequence);
}

/**
 * Gets the reply of the GetProperty request sent by ecore_x_window_state_get_prefetch().
 * @ingroup Ecore_X_NetWM_Group
 */
EAPI void
ecore_x_netwm_window_state_get_fetch(void)
{
   xcb_get_property_cookie_t cookie;
   xcb_get_property_reply_t *reply;

   cookie.sequence = _ecore_xcb_cookie_get();
   reply = xcb_get_property_reply(_ecore_xcb_conn, cookie, NULL);
   _ecore_xcb_reply_cache(reply);
}

/**
 * Get the hints describing the window state.
 * @param window The window.
 * @param state  The returned hins.
 * @param num    The number of hints.
 * @return       1 on success, 0 otherwise.
 *
 * Retrieve the hints describing @p window state. The state is
 * returned in @p state. The nummber of hints is stored in @p
 * num. This function returns 1 on success, 0 otherwise.
 *
 * To use this function, you must call before, and in order,
 * ecore_x_netwm_window_state_get_prefetch(), which sends the GetProperty request,
 * then ecore_x_netwm_window_state_get_fetch(), which gets the reply.
 * @ingroup Ecore_X_NetWM_Group
 */
EAPI int
ecore_x_netwm_window_state_get(Ecore_X_Window         window,
                               Ecore_X_Window_State **state,
                               unsigned int          *num)
{
   Ecore_X_Atom *atoms;
   int           num_ret;
   int           i;

   if (num) *num = 0;
   if (state) *state = NULL;

   num_ret = ecore_x_window_prop_atom_list_get(window, ECORE_X_ATOM_NET_WM_STATE,
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

/**
 * Set the _NET_WM_WINDOW_TYPE property.
 * @param window The window.
 * @param type   The functional type of the window.
 *
 * Set the functional @p type of @p window by sending _NET_WM_WINDOW_TYPE
 * property to @p window.
 *
 * This property SHOULD be set by the Client before mapping. This
 * property SHOULD be used by the window manager in determining the
 * decoration, stacking position and other behavior of the window. The
 * Client SHOULD specify window types in order of preference (the first
 * being most preferable).
 *
 * This hint is intended to replace the MOTIF hints.
 * @ingroup Ecore_X_NetWM_Group
 */
EAPI void
ecore_x_netwm_window_type_set(Ecore_X_Window      window,
                              Ecore_X_Window_Type type)
{
   Ecore_X_Atom atom;

   atom = _ecore_x_netwm_window_type_atom_get(type);
   ecore_x_window_prop_atom_set(window, ECORE_X_ATOM_NET_WM_WINDOW_TYPE,
				&atom, 1);
}

/**
 * Sends the GetProperty request.
 * @param window The window.
 * @ingroup Ecore_X_NetWM_Group
 */
EAPI void
ecore_x_netwm_window_type_get_prefetch(Ecore_X_Window window)
{
   xcb_get_property_cookie_t cookie;

   cookie = xcb_get_property_unchecked(_ecore_xcb_conn, 0, window,
                                       ECORE_X_ATOM_NET_WM_WINDOW_TYPE, ECORE_X_ATOM_ATOM,
                                       0, 0x7fffffff);
   _ecore_xcb_cookie_cache(cookie.sequence);
}

/**
 * Gets the reply of the GetProperty request sent by ecore_x_window_type_get_prefetch().
 * @ingroup Ecore_X_NetWM_Group
 */
EAPI void
ecore_x_netwm_window_type_get_fetch(void)
{
   xcb_get_property_cookie_t cookie;
   xcb_get_property_reply_t *reply;

   cookie.sequence = _ecore_xcb_cookie_get();
   reply = xcb_get_property_reply(_ecore_xcb_conn, cookie, NULL);
   _ecore_xcb_reply_cache(reply);
}

/* FIXME: Maybe return 0 on some conditions? */

/**
 * Get the functional type of a window.
 * @param window The window.
 * @param type   The function type of the window.
 * @return       1 on success, 0 otherwise.
 *
 * Retrieve the functional type of @p window. The type is stored in
 * @p type. This function returns 1 on success, 0 otherwise.
 *
 * To use this function, you must call before, and in order,
 * ecore_x_netwm_window_type_get_prefetch(), which sends the GetProperty request,
 * then ecore_x_netwm_window_type_get_fetch(), which gets the reply.
 * @ingroup Ecore_X_NetWM_Group
 */
EAPI int
ecore_x_netwm_window_type_get(Ecore_X_Window       window,
                              Ecore_X_Window_Type *type)
{
   Ecore_X_Atom *atoms;
   int           num;
   int           i;

   if (type) *type = ECORE_X_WINDOW_TYPE_NORMAL;

   num = ecore_x_window_prop_atom_list_get(window, ECORE_X_ATOM_NET_WM_WINDOW_TYPE,
					   &atoms);
   if (num < 0)
     {
	/* IMO this is not the place to mix netwm and icccm /kwo */
	/* Check if WM_TRANSIENT_FOR is set */

        /* Disable it for xcb */

/*	if ((type) && (ecore_x_icccm_transient_for_get(window))) */
/*	  *type = ECORE_X_WINDOW_TYPE_DIALOG; */
/*	return 1; */
     }

   if (type)
     {
	for (i = 0; i < num; ++i)
	  {
	     *type = _ecore_x_netwm_window_type_type_get(atoms[i]);
	     if (*type != ECORE_X_WINDOW_TYPE_UNKNOWN)
	       break;
	  }
     }

   free(atoms);

   return 1;
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
      default:
	 return 0;
     }
}

/**
 * Sends the GetProperty request.
 * @param window The window.
 * @ingroup Ecore_X_NetWM_Group
 */
EAPI void
ecore_x_netwm_allowed_action_get_prefetch(Ecore_X_Window window)
{
   xcb_get_property_cookie_t cookie;

   cookie = xcb_get_property_unchecked(_ecore_xcb_conn, 0, window,
                                       ECORE_X_ATOM_NET_WM_ALLOWED_ACTIONS, ECORE_X_ATOM_ATOM,
                                       0, 0x7fffffff);
   _ecore_xcb_cookie_cache(cookie.sequence);
}

/**
 * Gets the reply of the GetProperty request sent by ecore_x_allowed_action_get_prefetch().
 * @ingroup Ecore_X_NetWM_Group
 */
EAPI void
ecore_x_netwm_allowed_action_get_fetch(void)
{
   xcb_get_property_cookie_t cookie;
   xcb_get_property_reply_t *reply;

   cookie.sequence = _ecore_xcb_cookie_get();
   reply = xcb_get_property_reply(_ecore_xcb_conn, cookie, NULL);
   _ecore_xcb_reply_cache(reply);
}

/* FIXME: Get complete list */

/**
 * Check whether an action is supported by a window.
 * @param window The window
 * @param action The action
 * @return       1 if set, 0 otherwise.
 *
 * Return whether the user operation @p action is supported by the
 * Window Manager for @p window.
 *
 * To use this function, you must call before, and in order,
 * ecore_x_netwm_allowed_action_get_prefetch(), which sends the GetProperty request,
 * then ecore_x_netwm_allowed_action_get_fetch(), which gets the reply.
 * @ingroup Ecore_X_NetWM_Group
 */
EAPI int
ecore_x_netwm_allowed_action_isset(Ecore_X_Window window,
                                   Ecore_X_Action action)
{
   Ecore_X_Atom *atoms;
   Ecore_X_Atom  atom;
   int           num;
   int           ret = 0;
   int           i;

   num = ecore_x_window_prop_atom_list_get(window, ECORE_X_ATOM_NET_WM_ALLOWED_ACTIONS,
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
/**
 * Set the _NET_WM_ALLOWED_ACTIONS property.
 * @param window The window.
 * @param action An array of allowed actions.
 * @param num    The number of actions.
 *
 * Set the user operations that the Window Manager supports for
 * @p window by sending the _NET_WM_ALLOWED_ACTIONS property to
 * @p window. @p action stores @p num actions.
 *
 * To use this function, you must call before, and in order,
 * ecore_x_netwm_allowed_action_get_prefetch(), which sends the GetProperty request,
 * then ecore_x_netwm_allowed_action_get_fetch(), which gets the reply.
 * @ingroup Ecore_X_NetWM_Group
 */
EAPI void
ecore_x_netwm_allowed_action_set(Ecore_X_Window  window,
                                 Ecore_X_Action *action,
                                 unsigned int    num)
{
   Ecore_X_Atom  *set;
   unsigned int   i;

   if (!num)
     {
	ecore_x_window_prop_property_del(window, ECORE_X_ATOM_NET_WM_ALLOWED_ACTIONS);
	return;
     }

   set = malloc(num * sizeof(Ecore_X_Atom));
   if (!set) return;

   for (i = 0; i < num; i++)
     set[i] = _ecore_x_netwm_action_atom_get(action[i]);

   ecore_x_window_prop_atom_set(window, ECORE_X_ATOM_NET_WM_ALLOWED_ACTIONS, set, num);

   free(set);
}

/**
 * Get the allowed actions supported by a window.
 * @param window The window.
 * @param action The returned array of the actions.
 * @param num    The number of actions.
 * @return       1 on success, 0 otherwise.
 *
 * Retrieve the user operations that the Window Manager supports for
 * @p window and store them in @p action. The number of actions is
 * stored in @p num. This function returns 1 on success, 0 otherwise.
 *
 * To use this function, you must call before, and in order,
 * ecore_x_netwm_allowed_action_get_prefetch(), which sends the GetProperty request,
 * then ecore_x_netwm_allowed_action_get_fetch(), which gets the reply.
 * @ingroup Ecore_X_NetWM_Group
 */
EAPI int
ecore_x_netwm_allowed_action_get(Ecore_X_Window   window,
                                 Ecore_X_Action **action,
                                 unsigned int    *num)
{
   Ecore_X_Atom *atoms;
   int           num_ret;
   int           i;

   if (num) *num = 0;
   if (action) *action = NULL;

   num_ret = ecore_x_window_prop_atom_list_get(window, ECORE_X_ATOM_NET_WM_ALLOWED_ACTIONS,
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

/**
 * Set the _NET_WM_WINDOW_OPACITY property.
 * @param window  The window.
 * @param opacity The opacity value.
 *
 * Set the desired opacity of @p window by sending the
 * _NET_WM_WINDOW_OPACITY property to @p window. @p opacity is 0 for a
 * transparent window and 0xffffffff for an opaque window. @p opacity
 * must be multiplied with the original alpha value of @p window
 * (which is 1 for visuals not including an alpha component) so that
 * @p window content is modulated by the opacity value.
 *
 * Window Managers acting as compositing managers MAY take this into
 * account when displaying a window. Window Managers MUST forward the
 * value of this property to any enclosing frame window.  This
 * property MAY change while the window is mapped and the Window
 * Manager MUST respect changes while the window is mapped.
 * @ingroup Ecore_X_NetWM_Group
 */
EAPI void
ecore_x_netwm_opacity_set(Ecore_X_Window window,
                          unsigned int   opacity)
{
   uint32_t op = opacity;
   ecore_x_window_prop_card32_set(window, ECORE_X_ATOM_NET_WM_WINDOW_OPACITY,
				  &op, 1);
}

/**
 * Sends the GetProperty request.
 * @param window The window.
 * @ingroup Ecore_X_NetWM_Group
 */
EAPI void
ecore_x_netwm_opacity_get_prefetch(Ecore_X_Window window)
{
   xcb_get_property_cookie_t cookie;

   cookie = xcb_get_property_unchecked(_ecore_xcb_conn, 0, window,
                                       ECORE_X_ATOM_NET_WM_WINDOW_OPACITY, ECORE_X_ATOM_CARDINAL,
                                       0, 0x7fffffff);
   _ecore_xcb_cookie_cache(cookie.sequence);
}

/**
 * Gets the reply of the GetProperty request sent by ecore_x_netwm_opacity_get_prefetch().
 * @ingroup Ecore_X_NetWM_Group
 */
EAPI void
ecore_x_netwm_opacity_get_fetch(void)
{
   xcb_get_property_cookie_t cookie;
   xcb_get_property_reply_t *reply;

   cookie.sequence = _ecore_xcb_cookie_get();
   reply = xcb_get_property_reply(_ecore_xcb_conn, cookie, NULL);
   _ecore_xcb_reply_cache(reply);
}

/**
 * Get the opacity value of a window.
 * @param window  The window.
 * @param opacity The returned opacity.
 * @return        1 on success, 0 otherwise.
 *
 * Retriee the opacity value of @p window and store it in
 * @p opacity. This function returns 1 on sucess, 0 otherwise.
 *
 * To use this function, you must call before, and in order,
 * ecore_x_netwm_opacity_get_prefetch(), which sends the GetProperty request,
 * then ecore_x_netwm_opacity_get_fetch(), which gets the reply.
 * @ingroup Ecore_X_NetWM_Group
 */
EAPI int
ecore_x_netwm_opacity_get(Ecore_X_Window window,
                          unsigned int  *opacity)
{
   int ret;
   unsigned int tmp;

   ret = ecore_x_window_prop_card32_get(window, ECORE_X_ATOM_NET_WM_WINDOW_OPACITY,
					&tmp, 1);
   if (opacity) *opacity = tmp;
   return ret == 1 ? 1 : 0;
}

/**
 * Set the _NET_FRAME_EXTENTS property.
 * @param window The window.
 * @param fl     The number of pixels of the left border of the window.
 * @param fr     The number of pixels of the right border of the window.
 * @param ft     The number of pixels of the top border of the window.
 * @param fb     The number of pixels of the bottom border of the window.
 *
 * Set the border witdh of @p window by sending the _NET_FRAME_EXTENTS
 * property to @p window. @p fl, @p fr, @p ft and @p fb are respectively
 * the number of pixels of the left, right, top and bottom border of
 * @p window.
 *
 * The Window Manager MUST set _NET_FRAME_EXTENTS to the extents of
 * the window's frame.
 * @ingroup Ecore_X_NetWM_Group
 */
EAPI void
ecore_x_netwm_frame_size_set(Ecore_X_Window window,
                             int            fl,
                             int            fr,
                             int            ft,
                             int            fb)
{
   uint32_t frames[4];

   frames[0] = fl;
   frames[1] = fr;
   frames[2] = ft;
   frames[3] = fb;
   ecore_x_window_prop_card32_set(window, ECORE_X_ATOM_NET_FRAME_EXTENTS, frames, 4);
}

/**
 * Sends the GetProperty request.
 * @param window The window.
 * @ingroup Ecore_X_NetWM_Group
 */
EAPI void
ecore_x_netwm_frame_size_get_prefetch(Ecore_X_Window window)
{
   xcb_get_property_cookie_t cookie;

   cookie = xcb_get_property_unchecked(_ecore_xcb_conn, 0, window,
                                       ECORE_X_ATOM_NET_FRAME_EXTENTS, ECORE_X_ATOM_CARDINAL,
                                       0, 0x7fffffff);
   _ecore_xcb_cookie_cache(cookie.sequence);
}

/**
 * Gets the reply of the GetProperty request sent by ecore_x_netwm_frame_size_get_prefetch().
 * @ingroup Ecore_X_NetWM_Group
 */
EAPI void
ecore_x_netwm_frame_size_get_fetch(void)
{
   xcb_get_property_cookie_t cookie;
   xcb_get_property_reply_t *reply;

   cookie.sequence = _ecore_xcb_cookie_get();
   reply = xcb_get_property_reply(_ecore_xcb_conn, cookie, NULL);
   _ecore_xcb_reply_cache(reply);
}

/**
 * Get the frame extent of a window.
 * @param window  The window.
 * @param fl      The number of pixels of the left border of the window.
 * @param fr      The number of pixels of the right border of the window.
 * @param ft      The number of pixels of the top border of the window.
 * @param fb      The number of pixels of the bottom border of the window.
 * @return        1 on success, 0 otherwise.
 *
 * Retrieve the frame extents of @p window. The number of pixels of
 * the left, right, top and bottom border of @p window are
 * respectively stored in @p fl, @p fr, @p ft anfd @p fb. TYhis
 * function retuirns 1 on success, 0 otherwise.
 *
 * To use this function, you must call before, and in order,
 * ecore_x_netwm_frame_size_get_prefetch(), which sends the GetProperty request,
 * then ecore_x_netwm_frame_size_get_fetch(), which gets the reply.
 * @ingroup Ecore_X_NetWM_Group
 */
EAPI int
ecore_x_netwm_frame_size_get(Ecore_X_Window window,
                             int           *fl,
                             int           *fr,
                             int           *ft,
                             int           *fb)
{
   uint32_t frames[4];
   int      ret = 0;

   ret = ecore_x_window_prop_card32_get(window, ECORE_X_ATOM_NET_FRAME_EXTENTS, frames, 4);
   if (ret != 4)
     return 0;

   if (fl) *fl = frames[0];
   if (fr) *fr = frames[1];
   if (ft) *ft = frames[2];
   if (fb) *fb = frames[3];
   return 1;
}

/**
 * Sends the GetProperty request.
 * @param window The window.
 * @ingroup Ecore_X_NetWM_Group
 */
EAPI void
ecore_x_netwm_sync_counter_get_prefetch(Ecore_X_Window window)
{
   xcb_get_property_cookie_t cookie;

   cookie = xcb_get_property_unchecked(_ecore_xcb_conn, 0, window,
                                       ECORE_X_ATOM_NET_WM_SYNC_REQUEST_COUNTER, ECORE_X_ATOM_CARDINAL,
                                       0, 0x7fffffff);
   _ecore_xcb_cookie_cache(cookie.sequence);
}

/**
 * Gets the reply of the GetProperty request sent by ecore_x_netwm_sync_counter_get_prefetch().
 * @ingroup Ecore_X_NetWM_Group
 */
EAPI void
ecore_x_netwm_sync_counter_get_fetch(void)
{
   xcb_get_property_cookie_t cookie;
   xcb_get_property_reply_t *reply;

   cookie.sequence = _ecore_xcb_cookie_get();
   reply = xcb_get_property_reply(_ecore_xcb_conn, cookie, NULL);
   _ecore_xcb_reply_cache(reply);
}

/**
 * Get the X ID of a X Sync counter.
 * @param window  The window.
 * @param counter The X ID of the Sync counter.
 * @return        1 on success, 0 otherwise.
 *
 * Retrieve the X ID of the X Sync counter of @p window and store it
 * in @p counter. This function returns 1 on success, 0 otherwise.
 *
 * To use this function, you must call before, and in order,
 * ecore_x_netwm_frame_size_get_prefetch(), which sends the GetProperty request,
 * then ecore_x_netwm_frame_size_get_fetch(), which gets the reply.
 * @ingroup Ecore_X_NetWM_Group
 */
EAPI int
ecore_x_netwm_sync_counter_get(Ecore_X_Window        window,
                               Ecore_X_Sync_Counter *counter)
{
   int          ret;
   unsigned int tmp;

   ret = ecore_x_window_prop_card32_get(window, ECORE_X_ATOM_NET_WM_SYNC_REQUEST_COUNTER,
					&tmp, 1);

   if (counter) *counter = tmp;
   return (ret == 1) ? 1 : 0;
}

/**
 * Send a _NET_WM_PING property event.
 * @param window The window.
 *
 * Send a ClientMessage event from @p window with the _NET_WM_PING
 * property set.
 * @ingroup Ecore_X_NetWM_Group
 */
EAPI void
ecore_x_netwm_ping_send(Ecore_X_Window window)
{
   xcb_client_message_event_t ev;

   if (!window) return;

   ev.response_type = XCB_CLIENT_MESSAGE | 0x80;
   ev.format = 32;
   ev.sequence = 0;
   ev.window = window;
   ev.type = ECORE_X_ATOM_WM_PROTOCOLS;
   ev.data.data32[0] = ECORE_X_ATOM_NET_WM_PING;
   ev.data.data32[1] = XCB_CURRENT_TIME;
   ev.data.data32[2] = window;
   ev.data.data32[3] = 0;
   ev.data.data32[4] = 0;
   ev.data.data32[5] = 0;

   xcb_send_event(_ecore_xcb_conn, 0, window, XCB_EVENT_MASK_NO_EVENT, (const char *)&ev);
}

/**
 * Send a _NET_WM_SYNC_REQUEST property event.
 * @param window The window.
 * @param serial The update request number.
 *
 * Send a ClientMessage event from @p window with the _NET_WM_SYNC_REQUEST
 * property set.
 * @ingroup Ecore_X_NetWM_Group
 */
EAPI void
ecore_x_netwm_sync_request_send(Ecore_X_Window window,
                                unsigned int   serial)
{
   xcb_client_message_event_t ev;

   if (!window) return;

   ev.response_type = XCB_CLIENT_MESSAGE | 0x80;
   ev.format = 32;
   ev.window = window;
   ev.type = ECORE_X_ATOM_WM_PROTOCOLS;
   ev.data.data32[0] = ECORE_X_ATOM_NET_WM_SYNC_REQUEST;
   ev.data.data32[1] = XCB_CURRENT_TIME;
   ev.data.data32[2] = serial;
   /* FIXME: imho, the following test is useless as serial is non negative */
   /* should we remove it ? */
   ev.data.data32[3] = (serial < 0) ? ~0L : 0L;
   ev.data.data32[4] = 0;

   xcb_send_event(_ecore_xcb_conn, 0, window, 0, (const char *)&ev);
}

/**
 * Send a _NET_WM_STATE property event.
 * @param window The window.
 * @param root   The root window.
 * @param s1     The first state to alter.
 * @param s2     The second state to alter.
 * @param set    0 to unset the property, set it otherwise.
 *
 * Send a ClientMessage event from @p window to the @p root window
 * with the _NET_WM_STATE property set. This change the state of a
 * mapped window. @p s1 is the first state to alter. @p s2 is the
 * second state to alter. If @p set value is 0, the property is
 * removed (or unset), otherwise, the property is set.
 * @ingroup Ecore_X_NetWM_Group
 */
EAPI void
ecore_x_netwm_state_request_send(Ecore_X_Window       window,
                                 Ecore_X_Window       root,
                                 Ecore_X_Window_State s1,
                                 Ecore_X_Window_State s2,
                                 int                  set)
{
   xcb_client_message_event_t ev;

   if (!window) return;
   if (!root) root = ((xcb_screen_t *)_ecore_xcb_screen)->root;

   ev.response_type = XCB_CLIENT_MESSAGE | 0x80;
   ev.format = 32;
   ev.window = window;
   ev.type = ECORE_X_ATOM_NET_WM_STATE;
   ev.data.data32[0] = !!set;
   ev.data.data32[1] = _ecore_x_netwm_state_atom_get(s1);
   ev.data.data32[2] = _ecore_x_netwm_state_atom_get(s2);
   /* 1 == normal client, if someone wants to use this
    * function in a pager, this should be 2 */
   ev.data.data32[3] = 1;
   ev.data.data32[4] = 0;

   xcb_send_event(_ecore_xcb_conn, 0, root,
                  XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY | XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT,
                  (const char *)&ev);
}

/**
 * Send a _NET_WM_DESKTOP property event.
 * @param window  The window.
 * @param root    The root window.
 * @param desktop The new desktop index.
 *
 * Send a ClientMessage event from @p window to the @p root window
 * with the _NET_WM_DESKTOP property set. This change the state of a
 * non-withdrawn window. @p desktop is the new desktop index to set.
 * @ingroup Ecore_X_NetWM_Group
 */
EAPI void
ecore_x_netwm_desktop_request_send(Ecore_X_Window window,
                                   Ecore_X_Window root,
                                   unsigned int   desktop)
{
   xcb_client_message_event_t ev;

   if (!window) return;
   if (!root) root = ((xcb_screen_t *)_ecore_xcb_screen)->root;

   ev.response_type = XCB_CLIENT_MESSAGE | 0x80;
   ev.format = 32;
   ev.window = window;
   ev.type = ECORE_X_ATOM_NET_WM_DESKTOP;
   ev.data.data32[0] = desktop;

   xcb_send_event(_ecore_xcb_conn, 0, root,
                  XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY | XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT,
                  (const char *)&ev);
}

int
_ecore_x_netwm_startup_info_begin(Ecore_X_Window window,
                                  char          *data)
{
#if 0
   Ecore_X_Startup_Info *info;

   if (!startup_info) return 0;
   info = ecore_hash_get(startup_info, (void *)window);
   if (info)
     {
	printf("Already got info for win: 0x%x\n", window);
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
   ecore_hash_set(startup_info, (void *)info->win, info);
   if (strlen(info->buffer) != 20)
     {
	/* We have a '\0' in there, the message is done */
	_ecore_x_netwm_startup_info_process(info);
     }
#else
   window = XCB_NONE;
   data = NULL;
#endif

   return 1;
}

int
_ecore_x_netwm_startup_info(Ecore_X_Window window,
                            char          *data)
{
#if 0
   Ecore_X_Startup_Info *info;
   char *p;

   if (!startup_info) return 0;
   info = ecore_hash_get(startup_info, (void *)window);
   if (!info) return 0;
   if ((info->length + 20) > info->buffer_size)
     {
	info->buffer_size += 160;
	info->buffer = realloc(info->buffer, info->buffer_size * sizeof(char));
	if (!info->buffer)
	  {
	     ecore_hash_remove(startup_info, (void *)info->win);
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
   window = XCB_NONE;
   data = NULL;
#endif

   return 1;
}

/*
 * Set UTF-8 string property
 */
static void
_ecore_x_window_prop_string_utf8_set(Ecore_X_Window window,
                                     Ecore_X_Atom   atom,
                                     const char    *str)
{
   xcb_change_property(_ecore_xcb_conn, XCB_PROP_MODE_REPLACE, window,
                       atom, ECORE_X_ATOM_UTF8_STRING,
                       8, strlen(str), str);
}

#if 0
static void
_ecore_x_window_prop_string_utf8_get_prefetch(Ecore_X_Window window,
                                              Ecore_X_Atom atom)
{
   xcb_get_property_cookie_t cookie;

   cookie = xcb_get_property_unchecked(_ecore_xcb_conn, 0, window,
                                       atom, ECORE_X_ATOM_UTF8_STRING,
                                       0, 0x7fffffff);
   _ecore_xcb_cookie_cache(cookie.sequence);
}

static void
_ecore_x_window_prop_string_utf8_get_fetch(void)
{
   xcb_get_property_cookie_t cookie;
   xcb_get_property_reply_t *reply;

   cookie.sequence = _ecore_xcb_cookie_get();
   reply = xcb_get_property_reply(_ecore_xcb_conn, cookie, NULL);
   _ecore_xcb_reply_cache(reply);
}
#endif

/*
 * Get UTF-8 string property
 */
static char *
_ecore_x_window_prop_string_utf8_get(Ecore_X_Window window __UNUSED__,
                                     Ecore_X_Atom   atom __UNUSED__)
{
   xcb_get_property_reply_t *reply;
   char                     *str;
   int                       length;

   reply = _ecore_xcb_reply_get();
   if (!reply) return NULL;

   if ((reply->format != 8) ||
       (reply->value_len <= 0))
      return NULL;

   length = reply->value_len;
   str = (char *)malloc (sizeof (char) * (length + 1));
   if (!str)
     {
        return NULL;
     }
   memcpy(str, xcb_get_property_value(reply), length);
   str[length] = '\0';

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
	ecore_hash_remove(startup_info, (void *)info->win);
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
	ecore_hash_remove(startup_info, (void *)info->win);
	_ecore_x_netwm_startup_info_free(info);
	return 0;
     }

   p++;

   if (!_ecore_x_netwm_startup_info_parse(info, p))
     {
	ecore_hash_remove(startup_info, (void *)info->win);
	_ecore_x_netwm_startup_info_free(info);
	return 0;
     }

   if (info->init)
     {
	e = calloc(1, sizeof(Ecore_X_Event_Startup_Sequence));
	if (!e)
	  {
	     ecore_hash_remove(startup_info, (void *)info->win);
	     _ecore_x_netwm_startup_info_free(info);
	     return 0;
	  }
	e->win = info->win;
	ecore_event_add(event, e, NULL, NULL);
     }

   if (event == ECORE_X_EVENT_STARTUP_SEQUENCE_REMOVE)
     {
	ecore_hash_remove(startup_info, (void *)info->win);
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
_ecore_x_netwm_startup_info_parse(Ecore_X_Startup_Info *info,
                                  char                 *data)
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

/* FIXME: one round trip can be removed. Can we keep it ? */

/**
 * Check whether a screen is composited or not.
 * @param screen The screen index.
 *
 * Return 1 if @p screen is composited, 0 otherwise.
 * @ingroup Ecore_X_NetWM_Group
 */
EAPI int
ecore_x_screen_is_composited(int screen)
{
   char                             buf[32];
   xcb_intern_atom_cookie_t         cookie_atom;
   xcb_get_selection_owner_cookie_t cookie_owner;
   xcb_intern_atom_reply_t         *reply_atom;
   xcb_get_selection_owner_reply_t *reply_owner;
   Ecore_X_Window                   window;
   Ecore_X_Atom                     atom;

   snprintf(buf, sizeof(buf), "_NET_WM_CM_S%d", screen);
   cookie_atom = xcb_intern_atom_unchecked(_ecore_xcb_conn, 1,
                                           strlen(buf), buf);
   reply_atom = xcb_intern_atom_reply(_ecore_xcb_conn, cookie_atom, NULL);
   if (!reply_atom) return 0;
   atom = reply_atom->atom;
   free(reply_atom);

   cookie_owner = xcb_get_selection_owner_unchecked(_ecore_xcb_conn, atom);
   reply_owner = xcb_get_selection_owner_reply(_ecore_xcb_conn, cookie_owner, NULL);
   if (!reply_owner) return 0;

   window = reply_owner->owner;
   free(reply_owner);

   return window != XCB_NONE;
}
