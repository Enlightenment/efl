/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include "Ecore.h"
#include "ecore_xcb_private.h"
#include "Ecore_X_Atoms.h"


static int             ignore_num = 0;
static Ecore_X_Window *ignore_list = NULL;

static Ecore_X_Window _ecore_x_window_at_xy_get(Ecore_X_Window  base,
                                                int16_t         base_x,
                                                int16_t         base_y,
                                                int16_t         x,
                                                int16_t         y,
                                                Ecore_X_Window *skip,
                                                int             skip_num);

#ifdef ECORE_XCB_RENDER
static Ecore_X_Window _ecore_x_window_argb_internal_new(Ecore_X_Window parent,
                                                        int16_t        x,
                                                        int16_t        y,
                                                        uint16_t       w,
                                                        uint16_t       h,
                                                        uint8_t        override_redirect,
                                                        uint8_t        save_under);
#endif /* ECORE_XCB_RENDER */


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
ecore_x_window_new(Ecore_X_Window parent,
                   int            x,
                   int            y,
                   int            width,
                   int            height)
{
   uint32_t       value_list[9];
   Ecore_X_Window window;
   xcb_visualid_t vis = { XCB_WINDOW_CLASS_COPY_FROM_PARENT };
   uint32_t       value_mask;

   if (parent == 0) parent = ((xcb_screen_t *)_ecore_xcb_screen)->root;

   value_mask =
     XCB_CW_BACK_PIXMAP | XCB_CW_BORDER_PIXEL  | XCB_CW_BIT_GRAVITY       |
     XCB_CW_WIN_GRAVITY | XCB_CW_BACKING_STORE | XCB_CW_OVERRIDE_REDIRECT |
     XCB_CW_SAVE_UNDER  | XCB_CW_EVENT_MASK    | XCB_CW_DONT_PROPAGATE;

   value_list[0]  = XCB_NONE;
   value_list[1]  = 0;
   value_list[2]  = XCB_GRAVITY_NORTH_WEST;
   value_list[3]  = XCB_GRAVITY_NORTH_WEST;
   value_list[4]  = XCB_BACKING_STORE_NOT_USEFUL;
   value_list[5]  = 0;
   value_list[6]  = 0;
   value_list[7]  =
     XCB_EVENT_MASK_KEY_PRESS         | XCB_EVENT_MASK_KEY_RELEASE      |
     XCB_EVENT_MASK_BUTTON_PRESS      | XCB_EVENT_MASK_BUTTON_RELEASE   |
     XCB_EVENT_MASK_ENTER_WINDOW      | XCB_EVENT_MASK_LEAVE_WINDOW     |
     XCB_EVENT_MASK_POINTER_MOTION    | XCB_EVENT_MASK_EXPOSURE        |
     XCB_EVENT_MASK_VISIBILITY_CHANGE | XCB_EVENT_MASK_STRUCTURE_NOTIFY |
     XCB_EVENT_MASK_FOCUS_CHANGE      | XCB_EVENT_MASK_PROPERTY_CHANGE  |
     XCB_EVENT_MASK_COLOR_MAP_CHANGE;
   value_list[8]  = XCB_EVENT_MASK_NO_EVENT;

   window = xcb_generate_id(_ecore_xcb_conn);
   xcb_create_window(_ecore_xcb_conn,
                   XCB_WINDOW_CLASS_COPY_FROM_PARENT,
                   window, parent, x, y, width, height, 0,
                   XCB_WINDOW_CLASS_INPUT_OUTPUT,
                   vis,
                   value_mask,
                   value_list);

   if (parent == ((xcb_screen_t *)_ecore_xcb_screen)->root) ecore_x_window_defaults_set(window);
   return window;
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
ecore_x_window_override_new(Ecore_X_Window parent,
                            int            x,
                            int            y,
                            int            width,
                            int            height)
{
   uint32_t         value_list[9];
   Ecore_X_Window window;
   xcb_visualid_t   vis = { XCB_WINDOW_CLASS_COPY_FROM_PARENT };
   uint32_t         value_mask;

   if (parent == 0) parent = ((xcb_screen_t *)_ecore_xcb_screen)->root;

   value_mask =
     XCB_CW_BACK_PIXMAP | XCB_CW_BORDER_PIXEL  | XCB_CW_BIT_GRAVITY       |
     XCB_CW_WIN_GRAVITY | XCB_CW_BACKING_STORE | XCB_CW_OVERRIDE_REDIRECT |
     XCB_CW_SAVE_UNDER  | XCB_CW_EVENT_MASK    | XCB_CW_DONT_PROPAGATE;

   value_list[0]  = XCB_NONE;
   value_list[1]  = 0;
   value_list[2]  = XCB_GRAVITY_NORTH_WEST;
   value_list[3]  = XCB_GRAVITY_NORTH_WEST;
   value_list[4]  = XCB_BACKING_STORE_NOT_USEFUL;
   value_list[5]  = 1;
   value_list[6]  = 0;
   value_list[7]  =
     XCB_EVENT_MASK_KEY_PRESS         | XCB_EVENT_MASK_KEY_RELEASE      |
     XCB_EVENT_MASK_BUTTON_PRESS      | XCB_EVENT_MASK_BUTTON_RELEASE   |
     XCB_EVENT_MASK_ENTER_WINDOW      | XCB_EVENT_MASK_LEAVE_WINDOW     |
     XCB_EVENT_MASK_POINTER_MOTION    | XCB_EVENT_MASK_EXPOSURE        |
     XCB_EVENT_MASK_VISIBILITY_CHANGE | XCB_EVENT_MASK_STRUCTURE_NOTIFY |
     XCB_EVENT_MASK_FOCUS_CHANGE      | XCB_EVENT_MASK_PROPERTY_CHANGE  |
     XCB_EVENT_MASK_COLOR_MAP_CHANGE;
   value_list[8]  = XCB_EVENT_MASK_NO_EVENT;

   window = xcb_generate_id(_ecore_xcb_conn);
   xcb_create_window(_ecore_xcb_conn,
                   XCB_WINDOW_CLASS_COPY_FROM_PARENT,
                   window, parent, x, y, width, height, 0,
                   XCB_WINDOW_CLASS_INPUT_OUTPUT,
                   vis,
                   value_mask,
                   value_list);
   return window;
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
ecore_x_window_input_new(Ecore_X_Window parent,
                         int            x,
                         int            y,
                         int            width,
                         int            height)
{
   uint32_t         value_list[3];
   Ecore_X_Window window;
   xcb_visualid_t   vis = { XCB_WINDOW_CLASS_COPY_FROM_PARENT };
   uint32_t         value_mask;

   if (parent == 0) parent = ((xcb_screen_t *)_ecore_xcb_screen)->root;

   value_mask = XCB_CW_OVERRIDE_REDIRECT | XCB_CW_EVENT_MASK | XCB_CW_DONT_PROPAGATE;

   value_list[0]  = 1;
   value_list[1]  =
     XCB_EVENT_MASK_KEY_PRESS         | XCB_EVENT_MASK_KEY_RELEASE      |
     XCB_EVENT_MASK_BUTTON_PRESS      | XCB_EVENT_MASK_BUTTON_RELEASE   |
     XCB_EVENT_MASK_ENTER_WINDOW      | XCB_EVENT_MASK_LEAVE_WINDOW     |
     XCB_EVENT_MASK_POINTER_MOTION    | XCB_EVENT_MASK_EXPOSURE        |
     XCB_EVENT_MASK_VISIBILITY_CHANGE | XCB_EVENT_MASK_STRUCTURE_NOTIFY |
     XCB_EVENT_MASK_FOCUS_CHANGE      | XCB_EVENT_MASK_PROPERTY_CHANGE  |
     XCB_EVENT_MASK_COLOR_MAP_CHANGE;
   value_list[2]  = XCB_EVENT_MASK_NO_EVENT;

   window = xcb_generate_id(_ecore_xcb_conn);
   xcb_create_window(_ecore_xcb_conn,
                   XCB_WINDOW_CLASS_COPY_FROM_PARENT,
                   window, parent, x, y, width, height, 0,
                   XCB_WINDOW_CLASS_INPUT_OUTPUT,
                   vis,
                   value_mask,
                   value_list);

   if (parent == ((xcb_screen_t *)_ecore_xcb_screen)->root)
     {
     }

   return window;
}

/**
 * Creates a new window.
 * @param   parent The parent window to use.  If @p parent is @c 0, the root
 *                 window of the default display is used.
 * @param   x      X position.
 * @param   y      Y position.
 * @param   width  Width.
 * @param   height Height.
 * @return         The new window handle.
 * @ingroup Ecore_X_Window_Create_Group
 */
EAPI Ecore_X_Window
ecore_x_window_manager_argb_new(Ecore_X_Window parent,
                                int            x,
                                int            y,
                                int            width,
                                int            height)
{
   Ecore_X_Window window = 0;

#ifdef ECORE_XCB_RENDER
   window = _ecore_x_window_argb_internal_new(parent,
                                              x, y, width, height,
                                              1, 0);
#endif /* ECORE_XCB_RENDER */

   return window;
}

/**
 * Creates a new window.
 * @param   parent The parent window to use.  If @p parent is @c 0, the root
 *                 window of the default display is used.
 * @param   x      X position.
 * @param   y      Y position.
 * @param   width  Width.
 * @param   height Height.
 * @return         The new window handle.
 * @ingroup Ecore_X_Window_Create_Group
 */
EAPI Ecore_X_Window
ecore_x_window_argb_new(Ecore_X_Window parent,
                        int            x,
                        int            y,
                        int            width,
                        int            height)
{
   Ecore_X_Window window = 0;

#ifdef ECORE_XCB_RENDER
   window = _ecore_x_window_argb_internal_new(parent,
                                              x, y, width, height,
                                              0, 0);
#endif /* ECORE_XCB_RENDER */

   return window;
}

/**
 * Creates a window with the override redirect attribute set to @c True.
 * @param   parent The parent window to use.  If @p parent is @c 0, the root
 *                 window of the default display is used.
 * @param   x      X position.
 * @param   y      Y position.
 * @param   width  Width.
 * @param   height Height.
 * @return         The new window handle.
 * @ingroup Ecore_X_Window_Create_Group
 */
EAPI Ecore_X_Window
ecore_x_window_override_argb_new(Ecore_X_Window parent,
                                 int            x,
                                 int            y,
                                 int            width,
                                 int            height)
{
   Ecore_X_Window window = 0;

#ifdef ECORE_XCB_RENDER
   window = _ecore_x_window_argb_internal_new(parent,
                                              x, y, width, height,
                                              1, 0);
#endif /* ECORE_XCB_RENDER */

   return window;
}

/**
 * @defgroup Ecore_X_Window_Destroy_Group X Window Destroy Functions
 *
 * Functions to destroy X windows.
 */

/**
 * Deletes the given window.
 * @param   window The given window.
 * @ingroup Ecore_X_Window_Destroy_Group
 */
EAPI void
ecore_x_window_del(Ecore_X_Window window)
{
   /* sorry sir, deleting the root window doesn't sound like
    * a smart idea.
    */
   if (window)
      xcb_destroy_window(_ecore_xcb_conn, window);
}

/**
 * Sends a delete request to the given window.
 * @param   window The given window.
 * @ingroup Ecore_X_Window_Destroy_Group
 */
EAPI void
ecore_x_window_delete_request_send(Ecore_X_Window window)
{
   xcb_client_message_event_t ev;

   /* sorry sir, deleting the root window doesn't sound like
    * a smart idea.
    */
   if (window == 0)
      return;

   ev.response_type = XCB_CLIENT_MESSAGE;
   ev.format = 32;
   ev.sequence = 0;
   ev.window = window;
   ev.type = ECORE_X_ATOM_WM_PROTOCOLS;
   ev.data.data32[0] = ECORE_X_ATOM_WM_DELETE_WINDOW;
   ev.data.data32[1] = XCB_CURRENT_TIME;

   xcb_send_event(_ecore_xcb_conn, 0, window,
                  XCB_EVENT_MASK_NO_EVENT, (const char *)&ev);
}

/**
 * @defgroup Ecore_X_Window_Configure_Group X Window Configure Functions
 *
 * Functions to configure X windows.
 */


/**
 * Configures the given window with the given mask.
 * @param   window       The given window.
 * @param   mask         The given mask.
 * @param   x            The X coordinate of the window.
 * @param   y            The Y coordinate of the window.
 * @param   width        The width of the window.
 * @param   height       The height of the window.
 * @param   border_width The border width of the window.
 * @param   sibling      The sibling window of the window.
 * @param   stack_mode   The stack mode of the window.
 * @ingroup Ecore_X_Window_Configure_Group
 */
EAPI void
ecore_x_window_configure(Ecore_X_Window                window,
                         Ecore_X_Window_Configure_Mask mask,
                         int                           x,
                         int                           y,
                         int                           width,
                         int                           height,
                         int                           border_width,
                         Ecore_X_Window                sibling,
                         int                           stack_mode)
{
   uint32_t *value_list;
   uint32_t  value_mask;
   int       length = 0;

   if (!window)
      return;

   value_mask = mask;
   for ( ; value_mask; value_mask >>= 1)
     if (value_mask & 1)
       length++;
   value_list = (uint32_t *)malloc(sizeof(uint32_t) * length);
   if (!value_list)
      return;

   value_mask = mask;
   for ( ; value_mask; value_mask >>= 1, value_list++)
      if (value_mask & 1)
        {
          switch (value_mask) {
          case XCB_CONFIG_WINDOW_X:
            *value_list = x;
            break;
          case XCB_CONFIG_WINDOW_Y:
            *value_list = y;
            break;
          case XCB_CONFIG_WINDOW_WIDTH:
            *value_list = width;
            break;
          case XCB_CONFIG_WINDOW_HEIGHT:
            *value_list = height;
            break;
          case XCB_CONFIG_WINDOW_BORDER_WIDTH:
            *value_list = border_width;
            break;
          case XCB_CONFIG_WINDOW_SIBLING:
            *value_list = sibling;
            break;
          case XCB_CONFIG_WINDOW_STACK_MODE:
            *value_list = stack_mode;
            break;
          }
        }

   xcb_configure_window(_ecore_xcb_conn, window, mask, value_list);
   free(value_list);
}

/**
 * Moves a window to the position @p x, @p y.
 *
 * The position is relative to the upper left hand corner of the
 * parent window.
 *
 * @param   window The window to move.
 * @param   x      X position.
 * @param   y      Y position.
 * @ingroup Ecore_X_Window_Configure_Group
 */
EAPI void
ecore_x_window_move(Ecore_X_Window window,
                    int            x,
                    int            y)
{
   uint32_t value_list[2];
   uint32_t value_mask;

   if (!window)
      return;

   value_mask = XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y;

   value_list[0] = x;
   value_list[1] = y;

   xcb_configure_window(_ecore_xcb_conn, window, value_mask, value_list);
}

/**
 * Resizes a window.
 * @param   window The window to resize.
 * @param   width  New width of the window.
 * @param   height New height of the window.
 * @ingroup Ecore_X_Window_Configure_Group
 */
EAPI void
ecore_x_window_resize(Ecore_X_Window window,
                      int            width,
                      int            height)
{
   uint32_t value_list[2];
   uint32_t value_mask;

   if (!window)
      return;

   if (width < 1) width = 1;
   if (height < 1) height = 1;

   value_mask = XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT;

   value_list[0] = width;
   value_list[1] = height;

   xcb_configure_window(_ecore_xcb_conn, window, value_mask, value_list);
}

/**
 * Moves and resizes a window.
 * @param   window The window to move and resize.
 * @param   x      New X position of the window.
 * @param   y      New Y position of the window.
 * @param   width  New width of the window.
 * @param   height New height of the window.
 * @ingroup Ecore_X_Window_Configure_Group
 */
EAPI void
ecore_x_window_move_resize(Ecore_X_Window window,
                           int            x,
                           int            y,
                           int            width,
                           int            height)
{
   uint32_t value_list[4];
   uint32_t value_mask;

   if (!window)
      return;

   if (width < 1) width = 1;
   if (height < 1) height = 1;

   value_mask =
     XCB_CONFIG_WINDOW_X     | XCB_CONFIG_WINDOW_Y     |
     XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT;

   value_list[0] = x;
   value_list[1] = y;
   value_list[2] = width;
   value_list[3] = height;

   xcb_configure_window(_ecore_xcb_conn, window, value_mask, value_list);
}

/**
 * Sets the width of the border of the given window.
 * @param   window       The given window.
 * @param   border_width The new border width.
 * @ingroup Ecore_X_Window_Configure_Group
 */
EAPI void
ecore_x_window_border_width_set(Ecore_X_Window window,
                                int            border_width)
{
   uint32_t value_list;

   /* doesn't make sense to call this on a root window */
   if (!window)
      return;

   value_list = border_width;

   xcb_configure_window(_ecore_xcb_conn, window, XCB_CONFIG_WINDOW_BORDER_WIDTH, &value_list);
}

/**
 * Raises the given window.
 * @param   window The window to raise.
 * @ingroup Ecore_X_Window_Configure_Group
 */
EAPI void
ecore_x_window_raise(Ecore_X_Window window)
{
   uint32_t value_list;

   if (!window)
      return;

   value_list = XCB_STACK_MODE_ABOVE;

   xcb_configure_window(_ecore_xcb_conn, window, XCB_CONFIG_WINDOW_STACK_MODE, &value_list);
}

/**
 * Lowers the given window.
 * @param   window The window to lower.
 * @ingroup Ecore_X_Window_Configure_Group
 */
EAPI void
ecore_x_window_lower(Ecore_X_Window window)
{
   uint32_t value_list;

   if (!window)
      return;

   value_list = XCB_STACK_MODE_BELOW;

   xcb_configure_window(_ecore_xcb_conn, window, XCB_CONFIG_WINDOW_STACK_MODE, &value_list);
}

/**
 * @defgroup Evas_X_Window_Change_Properties_Group X Window Change Property Functions
 *
 * Functions that change window properties.
 */

/**
 * Sets the default properties for the given window.
 *
 * The default properties set for the window are @c WM_CLIENT_MACHINE and
 * @c _NET_WM_PID.
 *
 * @param   window The given window.
 * @ingroup Ecore_X_Window_Change_Property_Group
 */
EAPI void
ecore_x_window_defaults_set(Ecore_X_Window window)
{
   char   buf[MAXHOSTNAMELEN];
   pid_t  pid;
   int    argc;
   char **argv;

   /*
    * Set WM_CLIENT_MACHINE.
    */
   gethostname(buf, MAXHOSTNAMELEN);
   buf[MAXHOSTNAMELEN - 1] = '\0';
   /* The ecore function uses UTF8 which Xlib may not like (especially
    * with older clients) */
   /* ecore_xcb_window_prop_string_set(win, ECORE_X_ATOM_WM_CLIENT_MACHINE,
				  (char *)buf); */
   xcb_change_property(_ecore_xcb_conn, XCB_PROP_MODE_REPLACE, window,
                       ECORE_X_ATOM_WM_CLIENT_MACHINE,
                       ECORE_X_ATOM_STRING,
                       8, strlen(buf), buf);

   /*
    * Set _NET_WM_PID
    */
   pid = getpid();
   ecore_x_netwm_pid_set(window, pid);

   ecore_x_netwm_window_type_set(window, ECORE_X_WINDOW_TYPE_NORMAL);

   ecore_app_args_get(&argc, &argv);
   ecore_x_icccm_command_set(window, argc, argv);
}

/**
 * @defgroup Ecore_X_Window_Visibility_Group X Window Visibility Functions
 *
 * Functions to change the visibility of X windows.
 */

/**
 * Shows a window.
 *
 * Synonymous to "mapping" a window in X Window System terminology.
 *
 * @param   window The window to show.
 * @ingroup Ecore_X_Window_Visibility_Group
 */
EAPI void
ecore_x_window_show(Ecore_X_Window window)
{
   xcb_map_window(_ecore_xcb_conn, window);
}

/**
 * Hides a window.
 *
 * Synonymous to "unmapping" a window in X Window System terminology.
 *
 * @param   window The window to hide.
 * @ingroup Ecore_X_Window_Visibility_Group
 */
EAPI void
ecore_x_window_hide(Ecore_X_Window window)
{
   xcb_unmap_notify_event_t ev;
   Ecore_X_Window           root;

   /* ICCCM: SEND unmap event... */
   root = window;
   /* FIXME: is it correct ? */
   if (xcb_setup_roots_iterator(xcb_get_setup(_ecore_xcb_conn)).rem == 1)
     root = ((xcb_screen_t *)_ecore_xcb_screen)->root;
   else
     {
        xcb_get_geometry_cookie_t   cookie;
        xcb_get_geometry_reply_t   *rep;
        Ecore_X_Drawable            draw;

        /* FIXME: can we avoid round trips, here ? */
        draw = window;
        cookie = xcb_get_geometry_unchecked(_ecore_xcb_conn, draw);
        rep = xcb_get_geometry_reply(_ecore_xcb_conn, cookie, NULL);
        if (!rep)
           return;
        root = rep->root;
        free(rep);
     }
   ev.response_type = XCB_UNMAP_NOTIFY;
   ev.pad0 = 0;
   ev.sequence = 0;
   ev.event = root;
   ev.window = window;
   ev.from_configure = 0;

   xcb_send_event(_ecore_xcb_conn, 0, root,
                  XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY | XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT,
                  (const char *)&ev);
   xcb_unmap_window(_ecore_xcb_conn, window);
}

/**
 * @defgroup Ecore_X_Window_Input_Focus_Group X Window Input Focus Functions
 *
 * Functions that manage the focus of an X Window.
 */

/**
 * Sets the focus to the window @p window.
 * @param   window The window to focus.
 * @ingroup Ecore_X_Window_Input_Focus_Group
 */
EAPI void
ecore_x_window_focus(Ecore_X_Window window)
{
   Ecore_X_Time time = XCB_CURRENT_TIME;

   if (window == 0) window = ((xcb_screen_t *)_ecore_xcb_screen)->root;

   /* xcb_set_input_focus(_ecore_xcb_conn, XCB_INPUT_FOCUS_NONE, win, time); */
   xcb_set_input_focus(_ecore_xcb_conn,
                       XCB_INPUT_FOCUS_POINTER_ROOT, window, time);
}

/**
 * Sets the focus to the given window at a specific time.
 * @param   window The window to focus.
 * @param   time   When to set the focus to the window.
 * @ingroup Ecore_X_Window_Input_Focus_Group
 */
EAPI void
ecore_x_window_focus_at_time(Ecore_X_Window window,
                             Ecore_X_Time   time)
{
   if (window == 0) window = ((xcb_screen_t *)_ecore_xcb_screen)->root;

   /* xcb_set_input_focus(_ecore_xcb_conn, XCB_INPUT_FOCUS_NONE, win, time); */
   xcb_set_input_focus(_ecore_xcb_conn,
                       XCB_INPUT_FOCUS_POINTER_ROOT, window, time);
}

/**
 * @defgroup Ecore_X_Window_Reparent_Group X Window Reparent Functions
 *
 * Functions that retrieve or changes the parent window of a window.
 */

/**
 * Moves a window to within another window at a given position.
 * @param   window     The window to reparent.
 * @param   new_parent The new parent window.
 * @param   x          X position within new parent window.
 * @param   y          Y position within new parent window.
 * @ingroup Ecore_X_Window_Reparent_Group
 */
EAPI void
ecore_x_window_reparent(Ecore_X_Window window,
                        Ecore_X_Window new_parent,
                        int            x,
                        int            y)
{
   if (new_parent == 0) new_parent = ((xcb_screen_t *)_ecore_xcb_screen)->root;

   xcb_reparent_window(_ecore_xcb_conn, window, new_parent, x, y);
}


/**
 * @defgroup Ecore_X_Window_Change_Attributes_Group X Window Change Attributes Functions
 *
 * Functions that change the attributes of a window.
 */

/**
 * Sets the background pixmap of the given window.
 * @param   window The given window.
 * @param   pixmap The pixmap to set to.
 * @ingroup Ecore_X_Window_Change_Attributes_Group
 */
EAPI void
ecore_x_window_pixmap_set(Ecore_X_Window window,
                          Ecore_X_Pixmap pixmap)
{
   uint32_t value_list;

   value_list = pixmap;
   xcb_change_window_attributes(_ecore_xcb_conn, window,
                                XCB_CW_BACK_PIXMAP, &value_list);
}

/**
 * Sets the background color of the given window.
 * @param   window The given window.
 * @param   red    The red component of the color to set to.
 * @param   green  The green component of the color to set to.
 * @param   blue   The blue component of the color to set to.
 * @ingroup Ecore_X_Window_Change_Attributes_Group
 */
EAPI void
ecore_x_window_background_color_set(Ecore_X_Window window,
                                    unsigned short red,
                                    unsigned short green,
                                    unsigned short blue)
{
   xcb_alloc_color_cookie_t cookie;
   xcb_alloc_color_reply_t *rep;
   uint32_t                 value_list;

   /* FIXME: should I provide a reply, and not the color components, here ? */
   /*        (because of roundtrips) */
   cookie = xcb_alloc_color_unchecked(_ecore_xcb_conn,
                                      ((xcb_screen_t *)_ecore_xcb_screen)->default_colormap,
                                      red, green, blue);
   rep = xcb_alloc_color_reply(_ecore_xcb_conn, cookie, NULL);
   if (!rep)
     return;

   value_list = rep->pixel;
   xcb_change_window_attributes(_ecore_xcb_conn, window,
                                XCB_CW_BACK_PIXEL, &value_list);
   free(rep);
}

/**
 * Sets the bit gravity of the given window.
 * @param   window  The given window.
 * @param   gravity The gravity.
 * @ingroup Ecore_X_Window_Change_Attributes_Group
 */
EAPI void
ecore_x_window_pixel_gravity_set(Ecore_X_Window  window,
                                 Ecore_X_Gravity gravity)
{
   uint32_t value_list;

   value_list = gravity;
   xcb_change_window_attributes(_ecore_xcb_conn, window,
                                XCB_CW_BIT_GRAVITY, &value_list);
}

/**
 * Sets the gravity of the given window.
 * @param   window  The given window.
 * @param   gravity The gravity.
 * @ingroup Ecore_X_Window_Change_Attributes_Group
 */
EAPI void
ecore_x_window_gravity_set(Ecore_X_Window  window,
                           Ecore_X_Gravity gravity)
{
   uint32_t value_list;

   value_list = gravity;
   xcb_change_window_attributes(_ecore_xcb_conn, window,
                                XCB_CW_WIN_GRAVITY, &value_list);
}

/**
 * Sets the override attribute of the given window.
 * @param   window            The given window.
 * @param   override_redirect The override_redirect boolean.
 * @ingroup Ecore_X_Window_Change_Attributes_Group
 */
EAPI void
ecore_x_window_override_set(Ecore_X_Window window,
                            int            override_redirect)
{
   uint32_t value_list;

   value_list = override_redirect;
   xcb_change_window_attributes(_ecore_xcb_conn, window,
                                XCB_CW_OVERRIDE_REDIRECT, &value_list);
}

/**
 * Shows the cursor of the given window.
 * @param   window The given window.
 * @param   show   If set to @c 0, hide the cursor. Show it otherwise.
 * @ingroup Ecore_X_Window_Change_Attributes_Group
 */
EAPI void
ecore_x_window_cursor_show(Ecore_X_Window window,
                           int            show)
{
   if (window == 0) window = ((xcb_screen_t *)_ecore_xcb_screen)->root;

   if (!show)
     {
        Ecore_X_Cursor   cursor;
        Ecore_X_Drawable draw;
        Ecore_X_Pixmap   pixmap;
        Ecore_X_Pixmap   mask;
        Ecore_X_GC       gc;
        xcb_point_t      point;
        uint32_t         value_list;

        draw = window;
        pixmap = xcb_generate_id(_ecore_xcb_conn);
        xcb_create_pixmap(_ecore_xcb_conn,
                          1, pixmap, draw,
                          1, 1);
        mask = xcb_generate_id(_ecore_xcb_conn);
        xcb_create_pixmap(_ecore_xcb_conn,
                          1, mask, draw,
                          1, 1);

        gc = xcb_generate_id(_ecore_xcb_conn);
        xcb_create_gc (_ecore_xcb_conn, gc, draw, 0, NULL);
        value_list = 0;
        xcb_change_gc(_ecore_xcb_conn, gc, XCB_GC_FOREGROUND, &value_list);

        draw = mask;
        point.x = 0;
        point.y = 0;
        xcb_poly_point(_ecore_xcb_conn, XCB_COORD_MODE_ORIGIN, draw,
                       gc, 1, &point);

        xcb_free_gc(_ecore_xcb_conn, gc);

        cursor = xcb_generate_id(_ecore_xcb_conn);
        xcb_create_cursor(_ecore_xcb_conn, cursor,
                          pixmap, mask,
                          0, 0, 0,
                          0, 0, 0,
                          0, 0);
        value_list = cursor;
        xcb_change_window_attributes(_ecore_xcb_conn, window,
                                     XCB_CW_CURSOR, &value_list);

        xcb_free_cursor(_ecore_xcb_conn, cursor);
        xcb_free_pixmap(_ecore_xcb_conn, mask);
        xcb_free_pixmap(_ecore_xcb_conn, pixmap);
     }
   else
     {
        uint32_t value_list;

        value_list = 0;
        xcb_change_window_attributes(_ecore_xcb_conn, window,
                                     XCB_CW_CURSOR, &value_list);
     }
}

/**
 * Sets the cursor of the given window.
 * @param   window The given window.
 * @param   cursor The given cursor.
 * @ingroup Ecore_X_Window_Change_Attributes_Group
 */
EAPI void
ecore_x_window_cursor_set(Ecore_X_Window window,
                          Ecore_X_Cursor cursor)
{
   uint32_t value_list;

   value_list = cursor;
   xcb_change_window_attributes(_ecore_xcb_conn, window,
                                XCB_CW_CURSOR, &value_list);
}

/**
 * Todo
 * @param   window The given window.
 * @ingroup Ecore_X_Window_Change_Attributes_Group
 */
EAPI void
ecore_x_window_container_manage(Ecore_X_Window window)
{
   uint32_t value_list;

   value_list =
     XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY |
     XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT;
   xcb_change_window_attributes(_ecore_xcb_conn, window,
                                XCB_CW_EVENT_MASK, &value_list);

}

/**
 * Todo
 * @param   window The given window.
 * @ingroup Ecore_X_Window_Change_Attributes_Group
 */
EAPI void
ecore_x_window_client_manage(Ecore_X_Window window)
{
   uint32_t value_list;

   value_list =
     XCB_EVENT_MASK_VISIBILITY_CHANGE |
/*      XCB_EVENT_MASK_RESIZE_REDIRECT | */
     XCB_EVENT_MASK_STRUCTURE_NOTIFY |
     XCB_EVENT_MASK_FOCUS_CHANGE |
     XCB_EVENT_MASK_PROPERTY_CHANGE |
     XCB_EVENT_MASK_COLOR_MAP_CHANGE;
   xcb_change_window_attributes(_ecore_xcb_conn, window,
                                XCB_CW_EVENT_MASK, &value_list);
#ifdef ECORE_XCB_SHAPE
   xcb_shape_select_input(_ecore_xcb_conn, window, 1);
#endif /* ECORE_XCB_SHAPE */
}

/**
 * Todo
 * @param   window The given window.
 * @ingroup Ecore_X_Window_Change_Attributes_Group
 */
EAPI void
ecore_x_window_sniff(Ecore_X_Window window)
{
   uint32_t value_list;

   value_list =
     XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY |
     XCB_EVENT_MASK_PROPERTY_CHANGE;
   xcb_change_window_attributes(_ecore_xcb_conn, window,
                                XCB_CW_EVENT_MASK, &value_list);
}

/**
 * Todo
 * @param   window The given window.
 * @ingroup Ecore_X_Window_Change_Attributes_Group
 */
EAPI void
ecore_x_window_client_sniff(Ecore_X_Window window)
{
   uint32_t value_list;

   value_list =
     XCB_EVENT_MASK_VISIBILITY_CHANGE |
     XCB_EVENT_MASK_STRUCTURE_NOTIFY |
     XCB_EVENT_MASK_FOCUS_CHANGE |
     XCB_EVENT_MASK_PROPERTY_CHANGE |
     XCB_EVENT_MASK_COLOR_MAP_CHANGE;
   xcb_change_window_attributes(_ecore_xcb_conn, window,
                                XCB_CW_EVENT_MASK, &value_list);
#ifdef ECORE_XCB_SHAPE
   xcb_shape_select_input(_ecore_xcb_conn, window, 1);
#endif /* ECORE_XCB_SHAPE */
}

/**
 * Clears an area of the given window.
 * @param   window  The given window.
 * @param   x            The X coordinate of the area.
 * @param   y            The Y coordinate of the area.
 * @param   width        The width of the area.
 * @param   height       The height of the area.
 * @ingroup Ecore_X_Window_Clear_Area_Group
 */
EAPI void
ecore_x_window_area_clear(Ecore_X_Window window,
                          int            x,
                          int            y,
                          int            width,
                          int            height)
{
   xcb_clear_area(_ecore_xcb_conn, 0, window, x, y, width, height);
}

/**
 * Exposes an area of the given window.
 * @param   window  The given window.
 * @param   x            The X coordinate of the area.
 * @param   y            The Y coordinate of the area.
 * @param   width        The width of the area.
 * @param   height       The height of the area.
 * @ingroup Ecore_X_Window_Clear_Area_Group
 */
EAPI void
ecore_x_window_area_expose(Ecore_X_Window window,
                           int            x,
                           int            y,
                           int            width,
                           int            height)
{
   xcb_clear_area(_ecore_xcb_conn, 1, window, x, y, width, height);
}


/**
 * @defgroup Ecore_X_Window_Save_Set_Group X Window Change Save Set Functions
 *
 * Functions that either inserts or deletes the specified window from
 * the client's save-set.
 */

/**
 * Inserts the window in the client's save-set.
 * @param   window The window to insert in the client's save-set.
 * @ingroup Ecore_X_Window_Save_Set_Group
 */
EAPI void
ecore_x_window_save_set_add(Ecore_X_Window window)
{
   xcb_change_save_set(_ecore_xcb_conn, XCB_SET_MODE_INSERT, window);
}

/**
 * Deletes the window from the client's save-set.
 * @param   window The window to delete from the client's save-set.
 * @ingroup Ecore_X_Window_Save_Set_Group
 */
EAPI void
ecore_x_window_save_set_del(Ecore_X_Window window)
{
   xcb_change_save_set(_ecore_xcb_conn, XCB_SET_MODE_DELETE, window);
}

/******************************
 *
 * Request that have a reply
 *
 ******************************/


/**
 * Sends the GetInputFocus request.
 * @ingroup Ecore_X_Window_Input_Focus_Group
 */
EAPI void
ecore_x_get_input_focus_prefetch(void)
{
   xcb_get_input_focus_cookie_t cookie;

   cookie = xcb_get_input_focus_unchecked(_ecore_xcb_conn);
   _ecore_xcb_cookie_cache(cookie.sequence);
}

/**
 * Gets the reply of the GetInputFocus request sent by ecore_x_get_input_focus_prefetch().
 * @ingroup Ecore_X_Window_Input_Focus_Group
 */
EAPI void
ecore_x_get_input_focus_fetch(void)
{
   xcb_get_input_focus_cookie_t cookie;
   xcb_get_input_focus_reply_t *reply;

   cookie.sequence = _ecore_xcb_cookie_get();
   reply = xcb_get_input_focus_reply(_ecore_xcb_conn, cookie, NULL);
   _ecore_xcb_reply_cache(reply);
}

/**
 * Gets the window that has focus.
 * @return  The window that has focus.
 *
 * Returns the window that has the focus. If an error aoocured, @c 0
 * is returned, otherwise the function returns the window that has focus.
 *
 * To use this function, you must call before, and in order,
 * ecore_x_get_input_focus_prefetch(), which sends the GetInputFocus request,
 * then ecore_x_get_input_focus_fetch(), which gets the reply.
 * @ingroup Ecore_X_Window_Input_Focus_Group
 */
EAPI Ecore_X_Window
ecore_x_window_focus_get(void)
{
   xcb_get_input_focus_reply_t *reply;
   Ecore_X_Window               window = 0;

   reply = _ecore_xcb_reply_get();
   if (!reply) return window;

   return reply->focus;
}


/**
 * @defgroup Ecore_X_Window_Get_Attributes_Group X Window Get Attributes Functions
 *
 * Functions that get the attributes of a window.
 */


/**
 * Sends the GetWindowAttributes request.
 * @ingroup Ecore_X_Window_Get_Attributes_Group
 */
EAPI void
ecore_x_get_window_attributes_prefetch(Ecore_X_Window window)
{
   xcb_get_window_attributes_cookie_t cookie;

   cookie = xcb_get_window_attributes_unchecked(_ecore_xcb_conn, window);
   _ecore_xcb_cookie_cache(cookie.sequence);
}

/**
 * Gets the reply of the GetWindowAttributes request sent by ecore_x_get_window_attributes_prefetch().
 * @ingroup Ecore_X_Window_Get_Attributes_Group
 */
EAPI void
ecore_x_get_window_attributes_fetch(void)
{
   xcb_get_window_attributes_cookie_t cookie;
   xcb_get_window_attributes_reply_t *reply;

   cookie.sequence = _ecore_xcb_cookie_get();
   reply = xcb_get_window_attributes_reply(_ecore_xcb_conn, cookie, NULL);
   _ecore_xcb_reply_cache(reply);
}

/**
 * Retrieves the attributes of a window.
 * @param windows Unused.
 * @param att_ret Pointer to an Ecore_X_Window_Attributes
 *                  structure in which the attributes of a window
 *                  are to be stored.
 *
 * Retrieves the attributes of a window. If
 * @p att_ret is @c NULL, the function does nothing. If an error
 * occurred, @p att_ret is set to 0. Otherwise, the @p att_ret structure
 * is filled with the attributes os the requested window.
 *
 * To use this function, you must call before, and in order,
 * ecore_x_get_window_attributes_prefetch(), which sends the GetWindowAttributes request,
 * then ecore_x_get_window_attributes_fetch(), which gets the reply.
 * @ingroup Ecore_X_Window_Get_Attributes_Group
 */
EAPI int
ecore_x_window_attributes_get(Ecore_X_Window             window __UNUSED__,
                              Ecore_X_Window_Attributes *att_ret)
{
   xcb_get_window_attributes_reply_t *reply;

   if (!att_ret) return 0;

   reply = _ecore_xcb_reply_get();
   if (!reply) return 0;

   memset(att_ret, 0, sizeof(Ecore_X_Window_Attributes));

   if (reply->map_state != XCB_MAP_STATE_UNMAPPED) att_ret->visible = 1;
   if (reply->map_state == XCB_MAP_STATE_VIEWABLE) att_ret->viewable = 1;
   if (reply->override_redirect) att_ret->override = 1;
   if (reply->_class == XCB_WINDOW_CLASS_INPUT_ONLY) att_ret->input_only = 1;
   if (reply->save_under) att_ret->save_under = 1;

   att_ret->event_mask.mine         = reply->your_event_mask;
   att_ret->event_mask.all          = reply->all_event_masks;
   att_ret->event_mask.no_propagate = reply->do_not_propagate_mask;
   att_ret->window_gravity          = reply->win_gravity;
   att_ret->pixel_gravity           = reply->bit_gravity;
   att_ret->colormap                = reply->colormap;
   att_ret->visual                  = reply->visual;

   return 1;
}

/**
 * Finds out whether the given window is currently visible.
 * @param  window Unused.
 * @return        1 if the window is visible, otherwise 0.
 *
 * Finds out whether the given window is currently visible.
 * If an error occurred, or if the window is not visible, 0 is
 * returned. Otherwise 1 is returned.
 *
 * To use this function, you must call before, and in order,
 * ecore_x_get_window_attributes_prefetch(), which sends the GetWindowAttributes request,
 * then ecore_x_get_window_attributes_fetch(), which gets the reply.
 * @ingroup Ecore_X_Window_Get_Attributes_Group
 */
EAPI int
ecore_x_window_visible_get(Ecore_X_Window window __UNUSED__)
{
   xcb_get_window_attributes_reply_t *reply;

   reply = _ecore_xcb_reply_get();
   if (!reply) return 0;

   return (reply->map_state == XCB_MAP_STATE_VIEWABLE) ? 1 : 0;
}


/**
 * Sends the QueryPointer request.
 * @ingroup Ecore_X_Window_Parent_Group
 */
EAPI void
ecore_x_pointer_xy_get_prefetch(Ecore_X_Window window)
{
   xcb_query_pointer_cookie_t cookie;

   cookie = xcb_query_pointer_unchecked(_ecore_xcb_conn, window);
   _ecore_xcb_cookie_cache(cookie.sequence);
}

/**
 * Gets the reply of the QueryPointer request sent by ecore_x_query_pointer_prefetch().
 * @ingroup Ecore_X_Window_Parent_Group
 */
EAPI void
ecore_x_pointer_xy_get_fetch(void)
{
   xcb_query_pointer_cookie_t cookie;
   xcb_query_pointer_reply_t *reply;

   cookie.sequence = _ecore_xcb_cookie_get();
   reply = xcb_query_pointer_reply(_ecore_xcb_conn, cookie, NULL);
   _ecore_xcb_reply_cache(reply);
}

/**
 * Retrieves the coordinates of the pointer.
 * @param window Unused.
 * @param x      The X coordinate of the pointer.
 * @param y      The Y coordinate of the pointer.
 *
 * Retrieves the coordinates of the pointer.
 * If the window used in
 * ecore_x_query_pointer_prefetch() is not on the same screen than
 * the root window or if an error occured, @p x and @p y  are set
 * to 0. Otherwise, they are respectively set to the X and Y
 * coordinates of the pointer.
 *
 * To use this function, you must call before, and in order,
 * ecore_x_query_pointer_prefetch(), which sends the QueryPointer request,
 * then ecore_x_query_pointer_fetch(), which gets the reply.
 * @ingroup Ecore_X_Window_Parent_Group
 */
EAPI void
ecore_x_pointer_xy_get(Ecore_X_Window window __UNUSED__,
                       int           *x,
                       int           *y)
{
   xcb_query_pointer_reply_t *reply;

   reply = _ecore_xcb_reply_get();
   if (!reply)
     {
       if (x) *x = 0;
       if (y) *y = 0;

       return;
     }

   if (x) *x = reply->win_x;
   if (y) *y = reply->win_y;
}


/**
 * Sends the QueryTree request.
 * @ingroup Ecore_X_Window_Parent_Group
 */
EAPI void
ecore_x_query_tree_prefetch(Ecore_X_Window window)
{
   xcb_query_tree_cookie_t cookie;

   cookie = xcb_query_tree_unchecked(_ecore_xcb_conn, window);
   _ecore_xcb_cookie_cache(cookie.sequence);
}

/**
 * Gets the reply of the QueryTree request sent by ecore_x_query_tree_prefetch().
 * @ingroup Ecore_X_Window_Parent_Group
 */
EAPI void
ecore_x_query_tree_fetch(void)
{
   xcb_query_tree_cookie_t cookie;
   xcb_query_tree_reply_t *reply;

   cookie.sequence = _ecore_xcb_cookie_get();
   reply = xcb_query_tree_reply(_ecore_xcb_conn, cookie, NULL);
   _ecore_xcb_reply_cache(reply);
}

/**
 * Retrieves the parent window of the given window.
 * @param   window Unused.
 * @return         The parent window of @p window.
 *
 * Retrieves the parent window of the given window. If
 * an error occured, @c 0 is returned.
 *
 * To use this function, you must call before, and in order,
 * ecore_x_query_tree_prefetch(), which sends the QueryTree request,
 * then ecore_x_query_tree_fetch(), which gets the reply.
 * @ingroup Ecore_X_Window_Parent_Group
 */
EAPI Ecore_X_Window
ecore_x_window_parent_get(Ecore_X_Window window __UNUSED__)
{
   xcb_query_tree_reply_t *reply;

   reply = _ecore_xcb_reply_get();
   if (!reply) return 0;

   return reply->parent;
}


/**
 * Retrieves the children windows of the given window.
 * @param   window Unused.
 * @param   num    children windows count.
 * @return         The children windows.
 *
 * Retrieves the children windows of the given window. If
 * an error occured, @c 0 is returned.
 *
 * To use this function, you must call before, and in order,
 * ecore_x_query_tree_prefetch(), which sends the QueryTree request,
 * then ecore_x_query_tree_fetch(), which gets the reply.
 * @ingroup Ecore_X_Window_Parent_Group
 */
EAPI Ecore_X_Window *
ecore_x_window_children_get(Ecore_X_Window window __UNUSED__,
                            int           *num)
{
   xcb_query_tree_reply_t *reply;
   Ecore_X_Window         *windows = NULL;

   if (num) *num = 0;
   reply = _ecore_xcb_reply_get();
   if (!reply) return NULL;

   windows = malloc(reply->children_len);
   if (!windows)
     return NULL;

   if (num) *num = reply->children_len;
   memcpy(windows,
          xcb_query_tree_children(reply),
          sizeof(Ecore_X_Window) * reply->children_len);

   return windows;
}

/* FIXME: I've tried to remove the round trips. 3 cookies are */
/*        created at the beginning of the function. Because of */
/*        the recursivity of the algo, I can't find better trick */
static Ecore_X_Window
_ecore_x_window_at_xy_get(Ecore_X_Window  base,
                          int16_t         base_x,
                          int16_t         base_y,
                          int16_t         x,
                          int16_t         y,
                          Ecore_X_Window *skip,
                          int             skip_num)
{
   xcb_window_iterator_t              iter_children;
   xcb_get_window_attributes_cookie_t cookie_get_window_attributes;
   xcb_get_geometry_cookie_t          cookie_get_geometry;
   xcb_query_tree_cookie_t            cookie_query_tree;
   xcb_get_window_attributes_reply_t *reply_get_window_attributes;
   xcb_get_geometry_reply_t          *reply_get_geometry;
   xcb_query_tree_reply_t            *reply_query_tree;
   Ecore_X_Window                     window = 0;
   Ecore_X_Window                     child = 0;
   int16_t                            win_x;
   int16_t                            win_y;
   uint16_t                           win_width;
   uint16_t                           win_height;

   cookie_get_window_attributes = xcb_get_window_attributes_unchecked(_ecore_xcb_conn, base);
   cookie_get_geometry = xcb_get_geometry_unchecked(_ecore_xcb_conn, base);
   cookie_query_tree = xcb_query_tree_unchecked(_ecore_xcb_conn, base);

   reply_get_window_attributes = xcb_get_window_attributes_reply(_ecore_xcb_conn, cookie_get_window_attributes, NULL);
   if (!reply_get_window_attributes)
     {
       reply_get_geometry = xcb_get_geometry_reply(_ecore_xcb_conn, cookie_get_geometry, NULL);
       if (reply_get_geometry) free(reply_get_geometry);
       reply_query_tree = xcb_query_tree_reply(_ecore_xcb_conn, cookie_query_tree, NULL);
       if (reply_query_tree) free(reply_query_tree);
       return window;
     }

   if (reply_get_window_attributes->map_state != XCB_MAP_STATE_VIEWABLE)
     {
       free(reply_get_window_attributes);
       reply_get_geometry = xcb_get_geometry_reply(_ecore_xcb_conn, cookie_get_geometry, NULL);
       if (reply_get_geometry) free(reply_get_geometry);
       reply_query_tree = xcb_query_tree_reply(_ecore_xcb_conn, cookie_query_tree, NULL);
       if (reply_query_tree) free(reply_query_tree);
       return window;
     }

   free(reply_get_window_attributes);

   reply_get_geometry = xcb_get_geometry_reply(_ecore_xcb_conn, cookie_get_geometry, NULL);
   if (!reply_get_geometry)
     {
       reply_query_tree = xcb_query_tree_reply(_ecore_xcb_conn, cookie_query_tree, NULL);
       if (reply_query_tree) free(reply_query_tree);
       return window;
     }

   win_x = reply_get_geometry->x;
   win_y = reply_get_geometry->y;
   win_width = reply_get_geometry->width;
   win_height = reply_get_geometry->height;

   free(reply_get_geometry);

   win_x += base_x;
   win_y += base_y;

   if (!((x >= win_x) &&
         (y >= win_y) &&
         (x < (int16_t)(win_x + win_width)) &&
         (y < (int16_t)(win_y + win_height))))
     {
       reply_query_tree = xcb_query_tree_reply(_ecore_xcb_conn, cookie_query_tree, NULL);
       if (reply_query_tree) free(reply_query_tree);
       return window;
     }

   reply_query_tree = xcb_query_tree_reply(_ecore_xcb_conn, cookie_query_tree, NULL);
   if (!reply_query_tree)
     {
        if (skip)
	  {
             int i;

	     for (i = 0; i < skip_num; i++)
	       if (base == skip[i])
		 return window;
	  }
	return base;
     }

   iter_children = xcb_query_tree_children_iterator(reply_query_tree);
   for (; iter_children.rem; xcb_window_next(&iter_children))
     {
        if (skip)
          {
             int j;

             for (j = 0; j < skip_num; j++)
               if (*iter_children.data == skip[j])
                 continue;
          }
        child = _ecore_x_window_at_xy_get(*iter_children.data, win_x, win_y, x, y, skip, skip_num);
        if (child)
          {
             free(reply_query_tree);

             return child;
          }
     }

   if (skip)
     {
        int i;

	for (i = 0; i < skip_num; i++)
	  if (base == skip[i])
            {
               /* We return 0. child has an xid equal to 0 */
               free(reply_query_tree);
               return child;
            }
     }

   free(reply_query_tree);

   return base;
}

/**
 * @defgroup Ecore_X_Window_Geometry_Group X Window Geometry Functions
 *
 * Functions that change or retrieve the geometry of X windows.
 */

/**
 * Retrieves the top, visible window at the given location.
 * @param   x The given X position.
 * @param   y The given Y position.
 * @return    The window at that position.
 * @ingroup Ecore_X_Window_Geometry_Group
 */
EAPI Ecore_X_Window
ecore_x_window_at_xy_get(int x,
                           int y)
{
   Ecore_X_Window window;
   Ecore_X_Window root;

   /* FIXME: Proper function to determine current root/virtual root
    * window missing here */
   root = ((xcb_screen_t *)_ecore_xcb_screen)->root;

   ecore_x_grab();
   window = _ecore_x_window_at_xy_get(root, 0, 0, x, y, NULL, 0);
   ecore_x_ungrab();

   return window ? window : root;
}

/**
 * Retrieves the top, visible window at the given location,
 * but skips the windows in the list.
 * @param   x The given X position.
 * @param   y The given Y position.
 * @return    The window at that position.
 * @ingroup Ecore_X_Window_Geometry_Group
 */
EAPI Ecore_X_Window
ecore_x_window_at_xy_with_skip_get(int             x,
                                     int             y,
                                     Ecore_X_Window *skip,
                                     int             skip_num)
{
   Ecore_X_Window window;
   Ecore_X_Window root;

   /* FIXME: Proper function to determine current root/virtual root
    * window missing here */
   root = ((xcb_screen_t *)_ecore_xcb_screen)->root;

   ecore_x_grab();
   window = _ecore_x_window_at_xy_get(root, 0, 0, x, y, skip, skip_num);
   ecore_x_ungrab();

   return window ? window : root;
}

/**
 * Retrieves the top, visible window at the given location,
 * but begins at the @p begin window instead of the root one.
 * @param   begin The window from which we begin.
 * @param   x     The given X position.
 * @param   y     The given Y position.
 * @return        The window at that position.
 * @ingroup Ecore_X_Window_Geometry_Group
 */
EAPI Ecore_X_Window
ecore_x_window_at_xy_begin_get(Ecore_X_Window begin,
                               int            x,
                               int            y)
{
   Ecore_X_Window window;

   ecore_x_grab();
   window = _ecore_x_window_at_xy_get(begin, 0, 0, x, y, NULL, 0);
   ecore_x_ungrab();

   return window ? window : begin;
}



/* FIXME: Should I provide the replies (or the cookies), instead of
          creating them in the function ? */
#ifdef ECORE_XCB_RENDER
static Ecore_X_Window
_ecore_x_window_argb_internal_new(Ecore_X_Window parent,
                                  int16_t        x,
                                  int16_t        y,
                                  uint16_t       w,
                                  uint16_t       h,
                                  uint8_t        override_redirect,
                                  uint8_t        save_under)
{
   uint32_t                               value_list[10];
   xcb_depth_iterator_t                   iter_depth;
   xcb_visualtype_iterator_t              iter_visualtype;
   xcb_render_query_pict_formats_cookie_t cookie_pict_format;
   xcb_render_query_pict_formats_reply_t *rep_pict_format;
   Ecore_X_Screen                        *screen = NULL;
   Ecore_X_Window                         win = { 0 };
   xcb_visualid_t                         vis = { 0 };
   Ecore_X_Colormap                       colormap;
   uint32_t                               value_mask;

   cookie_pict_format = xcb_render_query_pict_formats_unchecked(_ecore_xcb_conn);

   if (parent == 0)
     {
        parent = ((xcb_screen_t *)_ecore_xcb_screen)->root;
	screen = ((xcb_screen_t *)_ecore_xcb_screen);
     }
   else
     {
        xcb_screen_iterator_t     iter_screen;
        xcb_get_geometry_reply_t *rep;
        Ecore_X_Drawable        draw;
        Ecore_X_Window          root;

        draw = parent;
        rep = xcb_get_geometry_reply(_ecore_xcb_conn,
                                     xcb_get_geometry_unchecked(_ecore_xcb_conn,
                                                                draw),
                                     NULL);
        if (!rep)
          return win;

        root = rep->root;

        free(rep);

        for (; iter_screen.rem; xcb_screen_next(&iter_screen))
          {
             if (iter_screen.data->root == root)
               {
                 screen = iter_screen.data;
               }
          }
     }
   if (!screen)
     return win;

   /* we get the X visual types */
   iter_depth = xcb_screen_allowed_depths_iterator(screen);
   for (; iter_depth.rem; xcb_depth_next(&iter_depth)) {
	if (iter_depth.data->depth == 32) {
	     iter_visualtype = xcb_depth_visuals_iterator(iter_depth.data);
	     break;
	}
   }

   /* we get the X render visual id */
   rep_pict_format = xcb_render_query_pict_formats_reply(_ecore_xcb_conn,
                                                         cookie_pict_format,
                                                         NULL);
   if (!rep_pict_format)
     return win;

   for (; iter_visualtype.rem; xcb_visualtype_next(&iter_visualtype)) {
      if (iter_visualtype.data->_class == XCB_VISUAL_CLASS_TRUE_COLOR) {
         xcb_render_pictforminfo_iterator_t iter_forminfo;
         xcb_render_pictscreen_iterator_t   iter_pictscreen;
         xcb_render_pictformat_t            pict_format = { 0 };

         iter_forminfo = xcb_render_query_pict_formats_formats_iterator(rep_pict_format);
         for (; iter_forminfo.rem; xcb_render_pictforminfo_next(&iter_forminfo)) {
            if (iter_forminfo.data->type == XCB_RENDER_PICT_TYPE_DIRECT &&
                iter_forminfo.data->direct.alpha_mask && iter_forminfo.data->depth == 32) {
               pict_format = iter_forminfo.data->id;
               break;
            }
         }
         if (pict_format == 0) {
            free(rep_pict_format);
            return win;
         }
         iter_pictscreen = xcb_render_query_pict_formats_screens_iterator(rep_pict_format);
         for (; iter_pictscreen.rem; xcb_render_pictscreen_next(&iter_pictscreen)) {
            xcb_render_pictdepth_iterator_t iter_depth;

            iter_depth = xcb_render_pictscreen_depths_iterator(iter_pictscreen.data);
            for (; iter_depth.rem; xcb_render_pictdepth_next(&iter_depth)) {
               xcb_render_pictvisual_iterator_t iter_visual;

               iter_visual = xcb_render_pictdepth_visuals_iterator(iter_depth.data);
               for (; iter_visual.rem; xcb_render_pictvisual_next(&iter_visual)) {
                  if ((iter_visual.data->visual == iter_visualtype.data->visual_id) &&
                      (pict_format == iter_visual.data->format)) {
                    vis = iter_visual.data->visual;
                    break;
                  }
               }
            }
         }
      }
   }

   free(rep_pict_format);

   if (vis == 0)
     return win;

   colormap = xcb_generate_id(_ecore_xcb_conn);
   xcb_create_colormap(_ecore_xcb_conn, XCB_COLORMAP_ALLOC_NONE, colormap, parent, vis);

   value_mask =
     XCB_CW_BACK_PIXMAP | XCB_CW_BORDER_PIXEL  | XCB_CW_BIT_GRAVITY       |
     XCB_CW_WIN_GRAVITY | XCB_CW_BACKING_STORE | XCB_CW_OVERRIDE_REDIRECT |
     XCB_CW_SAVE_UNDER  | XCB_CW_EVENT_MASK    | XCB_CW_DONT_PROPAGATE    |
     XCB_CW_COLORMAP;

   value_list[0]  = XCB_NONE;
   value_list[1]  = 0;
   value_list[2]  = XCB_GRAVITY_NORTH_WEST;
   value_list[3]  = XCB_GRAVITY_NORTH_WEST;
   value_list[4]  = XCB_BACKING_STORE_NOT_USEFUL;
   value_list[5]  = override_redirect;
   value_list[6]  = save_under;
   value_list[7]  =
     XCB_EVENT_MASK_KEY_PRESS         | XCB_EVENT_MASK_KEY_RELEASE      |
     XCB_EVENT_MASK_BUTTON_PRESS      | XCB_EVENT_MASK_BUTTON_RELEASE   |
     XCB_EVENT_MASK_ENTER_WINDOW      | XCB_EVENT_MASK_LEAVE_WINDOW     |
     XCB_EVENT_MASK_POINTER_MOTION    | XCB_EVENT_MASK_EXPOSURE        |
     XCB_EVENT_MASK_VISIBILITY_CHANGE | XCB_EVENT_MASK_STRUCTURE_NOTIFY |
     XCB_EVENT_MASK_FOCUS_CHANGE      | XCB_EVENT_MASK_PROPERTY_CHANGE  |
     XCB_EVENT_MASK_COLOR_MAP_CHANGE;
   value_list[8]  = XCB_EVENT_MASK_NO_EVENT;
   value_list[9]  = colormap;

   win = xcb_generate_id(_ecore_xcb_conn);
   xcb_create_window(_ecore_xcb_conn,
                     32, /* depth */
                     win, parent,
                     x, y, w, h, 0,
                     XCB_WINDOW_CLASS_INPUT_OUTPUT,
                     vis,
                     value_mask,
                     value_list);

   xcb_free_colormap(_ecore_xcb_conn, colormap);

   if (parent == ((xcb_screen_t *)_ecore_xcb_screen)->root)
     ecore_x_window_defaults_set(win);

   return win;
}
#endif /* ECORE_XCB_RENDER */



/* FIXME: round trip */
EAPI int
ecore_x_window_argb_get(Ecore_X_Window win)
{
   uint8_t                                ret = 0;
#ifdef ECORE_XCB_RENDER
   xcb_render_pictforminfo_iterator_t     iter_forminfo;
   xcb_render_pictscreen_iterator_t       iter_pictscreen;
   xcb_render_pictformat_t                pict_format = { 0 };
   xcb_render_query_pict_formats_reply_t *rep_pictformat;
   xcb_get_window_attributes_reply_t     *rep;
   xcb_visualid_t                         visual;

   rep = xcb_get_window_attributes_reply(_ecore_xcb_conn,
                                         xcb_get_window_attributes_unchecked(_ecore_xcb_conn,
                                                                             win),
                                         NULL);
   if (!rep)
     return ret;

   visual = rep->visual;

   free(rep);

   rep_pictformat = xcb_render_query_pict_formats_reply(_ecore_xcb_conn,
                                                        xcb_render_query_pict_formats_unchecked(_ecore_xcb_conn),
                                                        NULL);
   if (!rep_pictformat)
     return ret;

   iter_forminfo = xcb_render_query_pict_formats_formats_iterator(rep_pictformat);
   for (; iter_forminfo.rem; xcb_render_pictforminfo_next(&iter_forminfo))
     {
        if ((iter_forminfo.data->type == XCB_RENDER_PICT_TYPE_DIRECT) &&
            (iter_forminfo.data->direct.alpha_mask))
          {
             pict_format = iter_forminfo.data->id;
             break;
          }
     }
   if (pict_format == 0)
     {
        free(rep_pictformat);

        return ret;
     }

   iter_pictscreen = xcb_render_query_pict_formats_screens_iterator(rep_pictformat);
   for (; iter_pictscreen.rem; xcb_render_pictscreen_next(&iter_pictscreen))
     {
        xcb_render_pictdepth_iterator_t iter_depth;

        iter_depth = xcb_render_pictscreen_depths_iterator(iter_pictscreen.data);
        for (; iter_depth.rem; xcb_render_pictdepth_next(&iter_depth))
          {
             xcb_render_pictvisual_iterator_t iter_visual;

             iter_visual = xcb_render_pictdepth_visuals_iterator(iter_depth.data);
             for (; iter_visual.rem; xcb_render_pictvisual_next(&iter_visual))
               {
                  if ((iter_visual.data->visual == visual) &&
                      (pict_format == iter_visual.data->format))
                    {
                       ret = 1;
                       break;
                    }
               }
          }
     }

   free(rep_pictformat);
#endif /* ECORE_XCB_RENDER */

   return ret;
}




/**
 * Set if a window should be ignored.
 * @param   window The given window.
 * @param   ignore if to ignore
 */
EAPI void
ecore_x_window_ignore_set(Ecore_X_Window window,
                          int            ignore)
{
   int i, j;

   if (ignore)
     {
	if (ignore_list)
	  {
	     for (i = 0; i < ignore_num; i++)
	       {
		  if (window == ignore_list[i])
		    return;
	       }
	     ignore_list = realloc(ignore_list, (ignore_num + 1) * sizeof(Ecore_X_Window));
	     if (!ignore_list) return;
	     ignore_list[ignore_num++] = window;
	  }
	else
	  {
	     ignore_num = 0;
	     ignore_list = malloc(sizeof(Ecore_X_Window));
	     ignore_list[ignore_num++] = window;
	  }
     }
   else
     {
	if (!ignore_list) return;
	for (i = 0, j = 0; i < ignore_num; i++)
	  {
	     if (window != ignore_list[i])
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
 * Retrieves the size of the given window.
 * @param   win The given window.
 * @param   w   Pointer to an integer into which the width is to be stored.
 * @param   h   Pointer to an integer into which the height is to be stored.
 *
 * To use this function, you must call before, and in order,
 * ecore_x_drawable_geometry_get_prefetch(), which sends the GetGeometry request,
 * then ecore_x_drawable_geometry_get_fetch(), which gets the reply.
 * @ingroup Ecore_X_Window_Geometry_Group
 */
EAPI void
ecore_x_window_size_get(Ecore_X_Window window,
                        int           *width,
                        int           *height)
{
   if (window == 0)
      window = ((xcb_screen_t *)_ecore_xcb_screen)->root;

   ecore_x_drawable_geometry_get(window, NULL, NULL, width, height);
}

/**
 * Retrieves the geometry of the given window.
 * @param   win The given window.
 * @param   x   Pointer to an integer in which the X position is to be stored.
 * @param   y   Pointer to an integer in which the Y position is to be stored.
 * @param   w   Pointer to an integer in which the width is to be stored.
 * @param   h   Pointer to an integer in which the height is to be stored.
 *
 * To use this function, you must call before, and in order,
 * ecore_x_drawable_geometry_get_prefetch(), which sends the GetGeometry request,
 * then ecore_x_drawable_geometry_get_fetch(), which gets the reply.
 * @ingroup Ecore_X_Window_Geometry_Group
 */
EAPI void
ecore_x_window_geometry_get(Ecore_X_Window window,
                            int           *x,
                            int           *y,
                            int           *width,
                            int           *height)
{
   if (!window)
      window = ((xcb_screen_t *)_ecore_xcb_screen)->root;

   ecore_x_drawable_geometry_get(window, x, y, width, height);
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
 * Retrieves the depth of the given window.
 * @param  win The given window.
 * @return Depth of the window.
 */
EAPI int
ecore_x_window_depth_get(Ecore_X_Window win)
{
   return ecore_x_drawable_depth_get(win);
}
