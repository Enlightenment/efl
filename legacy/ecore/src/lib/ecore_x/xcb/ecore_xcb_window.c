#include "ecore_xcb_private.h"
#ifdef ECORE_XCB_RENDER
# include <xcb/render.h>
#endif
#ifdef ECORE_XCB_SHAPE
# include <xcb/shape.h>
#endif
#ifdef ECORE_XCB_XPRINT
#include <xcb/xprint.h>
#endif

/* local function prototypes */
static Ecore_X_Window _ecore_xcb_window_argb_internal_new(Ecore_X_Window parent, int x, int y, int w, int h, uint8_t override_redirect, uint8_t save_under);
static Ecore_X_Window _ecore_xcb_window_at_xy_get(Ecore_X_Window base, int bx, int by, int x, int y, Ecore_X_Window *skip, int skip_num);
static int _ecore_xcb_window_modifiers_get(unsigned int state);
static xcb_visualtype_t *_ecore_xcb_window_find_visual_by_id(xcb_visualid_t id);
static xcb_screen_t *_ecore_xcb_window_screen_of_display(int screen);

/* local variables */
static int ignore_num = 0;
static Ecore_X_Window *ignore_list = NULL;

/* external variables */
int _ecore_xcb_button_grabs_num = 0;
int _ecore_xcb_key_grabs_num = 0;
Ecore_X_Window *_ecore_xcb_button_grabs = NULL;
Ecore_X_Window *_ecore_xcb_key_grabs = NULL;
Eina_Bool (*_ecore_xcb_window_grab_replay_func)(void *data, int type, void *event);
void *_ecore_xcb_window_grab_replay_data;

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
   Ecore_X_Window win;
   uint32_t mask, mask_list[9];

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (parent == 0)
     parent = ((xcb_screen_t *)_ecore_xcb_screen)->root;

   /* NB: Order here is very important due to xcb_cw_t enum */
   mask = (XCB_CW_BACK_PIXMAP | XCB_CW_BORDER_PIXEL | XCB_CW_BIT_GRAVITY | 
           XCB_CW_WIN_GRAVITY | XCB_CW_BACKING_STORE | 
           XCB_CW_OVERRIDE_REDIRECT | XCB_CW_SAVE_UNDER | XCB_CW_EVENT_MASK | 
           XCB_CW_DONT_PROPAGATE);

   mask_list[0] = XCB_BACK_PIXMAP_NONE;
   mask_list[1] = 0;
   mask_list[2] = XCB_GRAVITY_NORTH_WEST;
   mask_list[3] = XCB_GRAVITY_NORTH_WEST;
   mask_list[4] = XCB_BACKING_STORE_NOT_USEFUL;
   mask_list[5] = 0;
   mask_list[6] = 0;
   mask_list[7] = (XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE | 
                   XCB_EVENT_MASK_BUTTON_PRESS | 
                   XCB_EVENT_MASK_BUTTON_RELEASE | 
                   XCB_EVENT_MASK_ENTER_WINDOW | XCB_EVENT_MASK_LEAVE_WINDOW | 
                   XCB_EVENT_MASK_POINTER_MOTION | XCB_EVENT_MASK_EXPOSURE | 
                   XCB_EVENT_MASK_VISIBILITY_CHANGE | 
                   XCB_EVENT_MASK_STRUCTURE_NOTIFY | 
                   XCB_EVENT_MASK_FOCUS_CHANGE | 
                   XCB_EVENT_MASK_PROPERTY_CHANGE | 
                   XCB_EVENT_MASK_COLOR_MAP_CHANGE);
   mask_list[8] = XCB_EVENT_MASK_NO_EVENT;

   win = xcb_generate_id(_ecore_xcb_conn);
   xcb_create_window(_ecore_xcb_conn, XCB_COPY_FROM_PARENT, 
                     win, parent, x, y, w, h, 0, 
                     XCB_WINDOW_CLASS_INPUT_OUTPUT, 
                     XCB_COPY_FROM_PARENT, mask, mask_list);

   if (parent == ((xcb_screen_t *)_ecore_xcb_screen)->root) 
     ecore_x_window_defaults_set(win);

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
   Ecore_X_Window win;
   uint32_t mask, mask_list[9];

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (parent == 0)
     parent = ((xcb_screen_t *)_ecore_xcb_screen)->root;

   /* NB: Order here is very important due to xcb_cw_t enum */
   mask = (XCB_CW_BACK_PIXMAP | XCB_CW_BORDER_PIXEL | XCB_CW_BIT_GRAVITY | 
           XCB_CW_WIN_GRAVITY | XCB_CW_BACKING_STORE | 
           XCB_CW_OVERRIDE_REDIRECT | XCB_CW_SAVE_UNDER | XCB_CW_EVENT_MASK | 
           XCB_CW_DONT_PROPAGATE);

   mask_list[0] = XCB_BACK_PIXMAP_NONE;
   mask_list[1] = 0;
   mask_list[2] = XCB_GRAVITY_NORTH_WEST;
   mask_list[3] = XCB_GRAVITY_NORTH_WEST;
   mask_list[4] = XCB_BACKING_STORE_NOT_USEFUL;
   mask_list[5] = 1;
   mask_list[6] = 0;
   mask_list[7] = (XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE | 
                   XCB_EVENT_MASK_BUTTON_PRESS | 
                   XCB_EVENT_MASK_BUTTON_RELEASE | 
                   XCB_EVENT_MASK_ENTER_WINDOW | XCB_EVENT_MASK_LEAVE_WINDOW | 
                   XCB_EVENT_MASK_POINTER_MOTION | XCB_EVENT_MASK_EXPOSURE | 
                   XCB_EVENT_MASK_VISIBILITY_CHANGE | 
                   XCB_EVENT_MASK_STRUCTURE_NOTIFY | 
                   XCB_EVENT_MASK_FOCUS_CHANGE | 
                   XCB_EVENT_MASK_PROPERTY_CHANGE | 
                   XCB_EVENT_MASK_COLOR_MAP_CHANGE);
   mask_list[8] = XCB_EVENT_MASK_NO_EVENT;

   win = xcb_generate_id(_ecore_xcb_conn);
   xcb_create_window(_ecore_xcb_conn, XCB_COPY_FROM_PARENT, 
                     win, parent, x, y, w, h, 0, 
                     XCB_WINDOW_CLASS_INPUT_OUTPUT, 
                     XCB_COPY_FROM_PARENT, mask, mask_list);

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
   Ecore_X_Window win;
   uint32_t mask, mask_list[3];

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (parent == 0)
     parent = ((xcb_screen_t *)_ecore_xcb_screen)->root;

   /* NB: Order here is very important due to xcb_cw_t enum */
   mask = (XCB_CW_OVERRIDE_REDIRECT | XCB_CW_EVENT_MASK | 
           XCB_CW_DONT_PROPAGATE);

   mask_list[0] = 1;
   mask_list[1] = (XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE | 
                   XCB_EVENT_MASK_BUTTON_PRESS | 
                   XCB_EVENT_MASK_BUTTON_RELEASE | 
                   XCB_EVENT_MASK_ENTER_WINDOW | XCB_EVENT_MASK_LEAVE_WINDOW | 
                   XCB_EVENT_MASK_POINTER_MOTION | XCB_EVENT_MASK_EXPOSURE | 
                   XCB_EVENT_MASK_VISIBILITY_CHANGE | 
                   XCB_EVENT_MASK_STRUCTURE_NOTIFY | 
                   XCB_EVENT_MASK_FOCUS_CHANGE | 
                   XCB_EVENT_MASK_PROPERTY_CHANGE | 
                   XCB_EVENT_MASK_COLOR_MAP_CHANGE);
   mask_list[2] = XCB_EVENT_MASK_NO_EVENT;

   win = xcb_generate_id(_ecore_xcb_conn);
   xcb_create_window(_ecore_xcb_conn, XCB_COPY_FROM_PARENT, 
                     win, parent, x, y, w, h, 0, 
                     XCB_WINDOW_CLASS_INPUT_ONLY, 
                     XCB_COPY_FROM_PARENT, mask, mask_list);

   return win;
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
   Ecore_X_Window win = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   win = _ecore_xcb_window_argb_internal_new(parent, x, y, w, h, 1, 0);

   return win;
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
   Ecore_X_Window win = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   win = _ecore_xcb_window_argb_internal_new(parent, x, y, w, h, 0, 0);

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
ecore_x_window_override_argb_new(Ecore_X_Window parent, int x, int y, int w, int h) 
{
   Ecore_X_Window win = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   win = _ecore_xcb_window_argb_internal_new(parent, x, y, w, h, 1, 0);

   return win;
}

/**
 * @defgroup Ecore_X_Window_Destroy_Group X Window Destroy Functions
 *
 * Functions to destroy X windows.
 */

/**
 * Deletes the given window.
 * @param   win The given window.
 * @ingroup Ecore_X_Window_Destroy_Group
 */
EAPI void 
ecore_x_window_free(Ecore_X_Window win) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (win) 
     {
        /* xcb_destroy_notify_event_t ev; */
        /* Ecore_X_Window root; */

        /* if (xcb_setup_roots_iterator(xcb_get_setup(_ecore_xcb_conn)).rem == 1) */
        /*   root = ((xcb_screen_t *)_ecore_xcb_screen)->root; */
        /* else  */
        /*   { */
        /*      xcb_get_geometry_cookie_t cookie; */
        /*      xcb_get_geometry_reply_t *reply; */

        /*      cookie = xcb_get_geometry_unchecked(_ecore_xcb_conn, win); */
        /*      reply = xcb_get_geometry_reply(_ecore_xcb_conn, cookie, NULL); */
        /*      if (!reply) return; */
        /*      root = reply->root; */
        /*      free(reply); */
        /*   } */

        /* memset(&ev, 0, sizeof(xcb_destroy_notify_event_t)); */

        /* ev.response_type = XCB_DESTROY_NOTIFY; */
        /* ev.window = win; */
        /* ev.event = root; */

        /* xcb_send_event(_ecore_xcb_conn, 0, root,  */
        /*                XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY |  */
        /*                XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT,  */
        /*                (const char *)&ev); */

        xcb_destroy_window(_ecore_xcb_conn, win);
        ecore_x_flush();
     }
}

/**
 * Sends a delete request to the given window.
 * @param   win The given window.
 * @ingroup Ecore_X_Window_Destroy_Group
 */
EAPI void 
ecore_x_window_delete_request_send(Ecore_X_Window win) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!win) return;
   ecore_x_client_message32_send(win, ECORE_X_ATOM_WM_PROTOCOLS, 
                                 XCB_EVENT_MASK_NO_EVENT, 
                                 ECORE_X_ATOM_WM_DELETE_WINDOW, 
                                 XCB_CURRENT_TIME, 0, 0, 0);
}

EAPI void 
ecore_x_window_configure(Ecore_X_Window win, Ecore_X_Window_Configure_Mask mask, int x, int y, int w, int h, int border_width, Ecore_X_Window sibling, int stack_mode) 
{
   uint16_t vmask = 0;
   uint32_t vlist[7];
   unsigned int i = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!win) return;

   if (mask & XCB_CONFIG_WINDOW_X) 
     {
        vmask |= XCB_CONFIG_WINDOW_X;
        vlist[i++] = x;
     }
   if (mask & XCB_CONFIG_WINDOW_Y) 
     {
        vmask |= XCB_CONFIG_WINDOW_Y;
        vlist[i++] = y;
     }
   if (mask & XCB_CONFIG_WINDOW_WIDTH) 
     {
        vmask |= XCB_CONFIG_WINDOW_WIDTH;
        vlist[i++] = w;
     }
   if (mask & XCB_CONFIG_WINDOW_HEIGHT) 
     {
        vmask |= XCB_CONFIG_WINDOW_HEIGHT;
        vlist[i++] = h;
     }
   if (mask & XCB_CONFIG_WINDOW_BORDER_WIDTH) 
     {
        vmask |= XCB_CONFIG_WINDOW_BORDER_WIDTH;
        vlist[i++] = border_width;
     }
   if (mask & XCB_CONFIG_WINDOW_SIBLING) 
     {
        vmask |= XCB_CONFIG_WINDOW_SIBLING;
        vlist[i++] = sibling;
     }
   if (mask & XCB_CONFIG_WINDOW_STACK_MODE) 
     {
        vmask |= XCB_CONFIG_WINDOW_STACK_MODE;
        vlist[i++] = stack_mode;
     }

   xcb_configure_window(_ecore_xcb_conn, win, vmask, 
                        (const uint32_t *)&vlist);
   ecore_x_flush();
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
   uint32_t list[2], mask;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!win) return;

   mask = (XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y);
   list[0] = x;
   list[1] = y;

   xcb_configure_window(_ecore_xcb_conn, win, mask, 
                        (const uint32_t *)&list);
   ecore_x_flush();
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
   uint32_t list[2], mask;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!win) return;
   if (w < 1) w = 1;
   if (h < 1) h = 1;

   mask = (XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT);
   list[0] = w;
   list[1] = h;

   xcb_configure_window(_ecore_xcb_conn, win, mask, 
                        (const uint32_t *)&list);
   ecore_x_flush();
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
   uint32_t list[4], mask;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!win) return;
   if (w < 1) w = 1;
   if (h < 1) h = 1;

   mask = (XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y | 
           XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT);
   list[0] = x;
   list[1] = y;
   list[2] = w;
   list[3] = h;

   xcb_configure_window(_ecore_xcb_conn, win, mask, 
                        (const uint32_t *)&list);
   ecore_x_flush();
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
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!win) return 0;
   return ecore_x_drawable_border_width_get(win);
}

/**
 * Sets the width of the border of the given window.
 * @param   win The given window.
 * @param   width The new border width.
 * @ingroup Ecore_X_Window_Geometry_Group
 */
EAPI void 
ecore_x_window_border_width_set(Ecore_X_Window win, int border_width) 
{
   uint32_t list;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!win) return;

   list = border_width;

   xcb_configure_window(_ecore_xcb_conn, win, 
                        XCB_CONFIG_WINDOW_BORDER_WIDTH, &list);
   ecore_x_flush();
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
   uint32_t list[] = { XCB_STACK_MODE_ABOVE };

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   xcb_configure_window(_ecore_xcb_conn, win, 
                        XCB_CONFIG_WINDOW_STACK_MODE, list);
   ecore_x_flush();
}

/**
 * Lowers the given window.
 * @param   win The window to lower.
 * @ingroup Ecore_X_Window_Z_Order_Group
 */
EAPI void 
ecore_x_window_lower(Ecore_X_Window win) 
{
   uint32_t list[] = { XCB_STACK_MODE_BELOW };

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   xcb_configure_window(_ecore_xcb_conn, win, 
                        XCB_CONFIG_WINDOW_STACK_MODE, list);
   ecore_x_flush();
}

/**
 * Retrieves the depth of the given window.
 * @param  win The given window.
 * @return Depth of the window.
 */
EAPI int 
ecore_x_window_depth_get(Ecore_X_Window win) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   return ecore_x_drawable_depth_get(win);
}

/**
 * @defgroup Ecore_X_Window_Properties_Group X Window Property Functions
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
 * @ingroup Ecore_X_Window_Properties_Groups
 */
EAPI void 
ecore_x_window_defaults_set(Ecore_X_Window win) 
{
   char buff[MAXHOSTNAMELEN], **argv;
   int argc;
   pid_t pid;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   gethostname(buff, MAXHOSTNAMELEN);
   buff[MAXHOSTNAMELEN - 1] = '\0';

   xcb_change_property(_ecore_xcb_conn, XCB_PROP_MODE_REPLACE, win, 
                       ECORE_X_ATOM_WM_CLIENT_MACHINE, ECORE_X_ATOM_STRING, 
                       8, strlen(buff), buff);

   pid = getpid();
   ecore_x_netwm_pid_set(win, pid);
   ecore_x_netwm_window_type_set(win, ECORE_X_WINDOW_TYPE_NORMAL);
   ecore_app_args_get(&argc, &argv);
   ecore_x_icccm_command_set(win, argc, argv);
}

/**
 * @defgroup Ecore_X_Window_Visibility_Group X Window Visibility Functions
 *
 * Functions to access and change the visibility of X windows.
 */

/**
 * Shows a window.
 *
 * Synonymous to "mapping" a window in X Window System terminology.
 *
 * @param   win The window to show.
 * @ingroup Ecore_X_Window_Visibility
 */
EAPI void 
ecore_x_window_show(Ecore_X_Window win) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (win) 
     {
        xcb_map_window(_ecore_xcb_conn, win);
        ecore_x_flush();
     }
}

/**
 * Hides a window.
 *
 * Synonymous to "unmapping" a window in X Window System terminology.
 *
 * @param   win The window to hide.
 * @ingroup Ecore_X_Window_Visibility
 */
EAPI void 
ecore_x_window_hide(Ecore_X_Window win) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (win) 
     {
        xcb_unmap_notify_event_t ev;
        Ecore_X_Window root;

        if (xcb_setup_roots_iterator(xcb_get_setup(_ecore_xcb_conn)).rem == 1)
          root = ((xcb_screen_t *)_ecore_xcb_screen)->root;
        else 
          {
             xcb_get_geometry_cookie_t cookie;
             xcb_get_geometry_reply_t *reply;

             cookie = xcb_get_geometry_unchecked(_ecore_xcb_conn, win);
             reply = xcb_get_geometry_reply(_ecore_xcb_conn, cookie, NULL);
             if (!reply) return;
             root = reply->root;
             free(reply);
          }

        memset(&ev, 0, sizeof(xcb_unmap_notify_event_t));

        ev.response_type = XCB_UNMAP_NOTIFY;
        ev.window = win;
        ev.event = root;
        ev.from_configure = 0;

        xcb_send_event(_ecore_xcb_conn, 0, root, 
                       (XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY | 
                           XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT), 
                       (const char *)&ev);

        xcb_unmap_window(_ecore_xcb_conn, win);
        ecore_x_flush();
     }
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
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!win) win = ((xcb_screen_t *)_ecore_xcb_screen)->root;

   xcb_set_input_focus(_ecore_xcb_conn, 
                       XCB_INPUT_FOCUS_PARENT, win, XCB_CURRENT_TIME);
   ecore_x_flush();
}

/**
 * Sets the focus to the given window at a specific time.
 * @param   win The window to focus.
 * @param   t   When to set the focus to the window.
 * @ingroup Ecore_X_Window_Focus_Functions
 */
EAPI void 
ecore_x_window_focus_at_time(Ecore_X_Window win, Ecore_X_Time time) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!win) win = ((xcb_screen_t *)_ecore_xcb_screen)->root;
   xcb_set_input_focus(_ecore_xcb_conn, XCB_INPUT_FOCUS_PARENT, win, time);
   ecore_x_flush();
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
ecore_x_window_reparent(Ecore_X_Window win, Ecore_X_Window parent, int x, int y) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (parent == 0) 
     parent = ((xcb_screen_t *)_ecore_xcb_screen)->root;

   xcb_reparent_window(_ecore_xcb_conn, win, parent, x, y);
   ecore_x_flush();
}

EAPI void 
ecore_x_window_pixmap_set(Ecore_X_Window win, Ecore_X_Pixmap pixmap) 
{
   uint32_t list;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   list = pixmap;

   xcb_change_window_attributes(_ecore_xcb_conn, win, 
                                XCB_CW_BACK_PIXMAP, &list);
   ecore_x_flush();
}

/**
 * Sets the background color of the given window.
 * @param win   The given window
 * @param r     red value (0...65536, 16 bits)
 * @param g     green value (0...65536, 16 bits)
 * @param b     blue value (0...65536, 16 bits)
 */
EAPI void 
ecore_x_window_background_color_set(Ecore_X_Window win, unsigned short red, unsigned short green, unsigned short blue) 
{
   xcb_alloc_color_cookie_t cookie;
   xcb_alloc_color_reply_t *reply;
   uint32_t list;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   cookie = 
     xcb_alloc_color_unchecked(_ecore_xcb_conn, 
                               ((xcb_screen_t *)_ecore_xcb_screen)->default_colormap, 
                               red, green, blue);
   reply = xcb_alloc_color_reply(_ecore_xcb_conn, cookie, NULL);
   if (!reply) return;
   list = reply->pixel;
   free(reply);

   xcb_change_window_attributes(_ecore_xcb_conn, win, 
                                XCB_CW_BACK_PIXEL, &list);
   ecore_x_flush();
}

EAPI void 
ecore_x_window_pixel_gravity_set(Ecore_X_Window win, Ecore_X_Gravity gravity) 
{
   uint32_t list;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   list = gravity;

   xcb_change_window_attributes(_ecore_xcb_conn, win, 
                                XCB_CW_BIT_GRAVITY, &list);
   ecore_x_flush();
}

EAPI void 
ecore_x_window_gravity_set(Ecore_X_Window win, Ecore_X_Gravity gravity) 
{
   uint32_t list;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   list = gravity;

   xcb_change_window_attributes(_ecore_xcb_conn, win, 
                                XCB_CW_WIN_GRAVITY, &list);
   ecore_x_flush();
}

EAPI void 
ecore_x_window_override_set(Ecore_X_Window win, Eina_Bool override) 
{
   uint32_t list;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   list = override;

   xcb_change_window_attributes(_ecore_xcb_conn, win, 
                                XCB_CW_OVERRIDE_REDIRECT, &list);
   ecore_x_flush();
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 */
EAPI void 
ecore_x_window_cursor_show(Ecore_X_Window win, Eina_Bool show) 
{
   uint32_t list = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!win) win = ((xcb_screen_t *)_ecore_xcb_screen)->root;

   if (!show) 
     {
        Ecore_X_Cursor cursor;
        Ecore_X_Pixmap p, m;
        Ecore_X_GC gc;
        xcb_point_t point;

        p = xcb_generate_id(_ecore_xcb_conn);
        xcb_create_pixmap(_ecore_xcb_conn, 1, p, win, 1, 1);
        m = xcb_generate_id(_ecore_xcb_conn);
        xcb_create_pixmap(_ecore_xcb_conn, 1, m, win, 1, 1);
        gc = xcb_generate_id(_ecore_xcb_conn);
        xcb_create_gc(_ecore_xcb_conn, gc, win, 0, NULL);
        xcb_change_gc(_ecore_xcb_conn, gc, XCB_GC_FOREGROUND, &list);
        point.x = 0;
        point.y = 0;
        xcb_poly_point(_ecore_xcb_conn, XCB_COORD_MODE_ORIGIN, 
                       win, gc, 1, &point);
        xcb_free_gc(_ecore_xcb_conn, gc);

        cursor = xcb_generate_id(_ecore_xcb_conn);
        xcb_create_cursor(_ecore_xcb_conn, cursor, 
                          p, m, 0, 0, 0, 0, 0, 0, 0, 0);
        list = cursor;

        xcb_change_window_attributes(_ecore_xcb_conn, win, 
                                     XCB_CW_CURSOR, &list);

        xcb_free_cursor(_ecore_xcb_conn, cursor);
        xcb_free_pixmap(_ecore_xcb_conn, m);
        xcb_free_pixmap(_ecore_xcb_conn, p);
     }
   else 
     {
        xcb_change_window_attributes(_ecore_xcb_conn, win, 
                                     XCB_CW_CURSOR, &list);
     }
   ecore_x_flush();
}

EAPI void 
ecore_x_window_cursor_set(Ecore_X_Window win, Ecore_X_Cursor cursor) 
{
   uint32_t list;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   list = cursor;

   xcb_change_window_attributes(_ecore_xcb_conn, win, XCB_CW_CURSOR, &list);
   ecore_x_flush();
}

EAPI void 
ecore_x_window_container_manage(Ecore_X_Window win) 
{
   uint32_t list;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   list = (XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT | 
           XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY);

   xcb_change_window_attributes(_ecore_xcb_conn, win, 
                                XCB_CW_EVENT_MASK, &list);
   ecore_x_flush();
}

EAPI void 
ecore_x_window_client_manage(Ecore_X_Window win) 
{
   uint32_t list;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   list = (XCB_EVENT_MASK_VISIBILITY_CHANGE | 
           XCB_EVENT_MASK_FOCUS_CHANGE | 
           XCB_EVENT_MASK_PROPERTY_CHANGE | 
           XCB_EVENT_MASK_COLOR_MAP_CHANGE | 
           XCB_EVENT_MASK_STRUCTURE_NOTIFY | 
           XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY);

   xcb_change_window_attributes(_ecore_xcb_conn, win, 
                                XCB_CW_EVENT_MASK, &list);

#ifdef ECORE_XCB_SHAPE
   xcb_shape_select_input(_ecore_xcb_conn, win, 1);
#endif
   ecore_x_flush();
}

EAPI void 
ecore_x_window_sniff(Ecore_X_Window win) 
{
   uint32_t list;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   list = (XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY | 
           XCB_EVENT_MASK_PROPERTY_CHANGE);

   xcb_change_window_attributes(_ecore_xcb_conn, win, 
                                XCB_CW_EVENT_MASK, &list);
   ecore_x_flush();
}

EAPI void 
ecore_x_window_client_sniff(Ecore_X_Window win) 
{
   uint32_t list;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   list = (XCB_EVENT_MASK_VISIBILITY_CHANGE | 
           XCB_EVENT_MASK_STRUCTURE_NOTIFY | 
           XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY | 
           XCB_EVENT_MASK_FOCUS_CHANGE | 
           XCB_EVENT_MASK_PROPERTY_CHANGE | 
           XCB_EVENT_MASK_COLOR_MAP_CHANGE);

   xcb_change_window_attributes(_ecore_xcb_conn, win, 
                                XCB_CW_EVENT_MASK, &list);
#ifdef ECORE_XCB_SHAPE
   xcb_shape_select_input(_ecore_xcb_conn, win, 1);
#endif
   ecore_x_flush();
}

EAPI void 
ecore_x_window_area_clear(Ecore_X_Window win, int x, int y, int w, int h) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   xcb_clear_area(_ecore_xcb_conn, 0, win, x, y, w, h);
   ecore_x_flush();
}

EAPI void 
ecore_x_window_area_expose(Ecore_X_Window win, int x, int y, int w, int h) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   xcb_clear_area(_ecore_xcb_conn, 1, win, x, y, w, h);
   ecore_x_flush();
}

EAPI void 
ecore_x_window_save_set_add(Ecore_X_Window win) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   xcb_change_save_set(_ecore_xcb_conn, XCB_SET_MODE_INSERT, win);
}

EAPI void 
ecore_x_window_save_set_del(Ecore_X_Window win) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   xcb_change_save_set(_ecore_xcb_conn, XCB_SET_MODE_DELETE, win);
}

/**
 * gets the focus to the window @p win.
 * @return  The window that has focus.
 * @ingroup Ecore_X_Window_Focus_Functions
 */
EAPI Ecore_X_Window 
ecore_x_window_focus_get(void) 
{
   xcb_get_input_focus_cookie_t cookie;
   xcb_get_input_focus_reply_t *reply;
   Ecore_X_Window focus = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   cookie = xcb_get_input_focus_unchecked(_ecore_xcb_conn);
   reply = xcb_get_input_focus_reply(_ecore_xcb_conn, cookie, NULL);
   if (!reply) return 0;
   focus = reply->focus;
   free(reply);
   return focus;
}

EAPI int 
ecore_x_window_argb_get(Ecore_X_Window win) 
{
   uint8_t ret = 0;
#ifdef ECORE_XCB_RENDER
   Ecore_X_Visual visual;
#endif

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

//   if (!win) return ret;

#ifdef ECORE_XCB_RENDER
   /* grab the window's visual */
   visual = _ecore_xcb_window_visual_get(win);

   /* check if this visual supports alpha */
   ret = _ecore_xcb_render_visual_supports_alpha(visual);
#endif

   return ret;
}

EAPI Eina_Bool 
ecore_x_window_manage(Ecore_X_Window win) 
{
   xcb_get_window_attributes_cookie_t cookie;
   xcb_get_window_attributes_reply_t *reply;
   xcb_void_cookie_t change_cookie;
   xcb_generic_error_t *err;
   uint32_t list;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   cookie = xcb_get_window_attributes(_ecore_xcb_conn, win);
   reply = xcb_get_window_attributes_reply(_ecore_xcb_conn, cookie, NULL);
   if (!reply) return EINA_FALSE;

   ecore_x_flush(); // needed
//   ecore_x_sync(); // needed

   list = (XCB_EVENT_MASK_ENTER_WINDOW | XCB_EVENT_MASK_LEAVE_WINDOW | 
           XCB_EVENT_MASK_PROPERTY_CHANGE | XCB_EVENT_MASK_RESIZE_REDIRECT | 
           XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT | 
           XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY | 
           XCB_EVENT_MASK_STRUCTURE_NOTIFY | 
           XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE | 
           reply->your_event_mask);
   free(reply);

   change_cookie = xcb_change_window_attributes(_ecore_xcb_conn, win, 
                                                XCB_CW_EVENT_MASK, &list);

   ecore_x_flush(); // needed
//   ecore_x_sync(); // needed

   err = xcb_request_check(_ecore_xcb_conn, change_cookie);
   if (err) return EINA_FALSE;

   return EINA_TRUE;
}

EAPI Eina_Bool 
ecore_x_window_attributes_get(Ecore_X_Window win, Ecore_X_Window_Attributes *att_ret) 
{
   xcb_get_window_attributes_cookie_t cookie;
   xcb_get_window_attributes_reply_t *reply;
   xcb_get_geometry_cookie_t gcookie;
   xcb_get_geometry_reply_t *greply;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   cookie = xcb_get_window_attributes_unchecked(_ecore_xcb_conn, win);
   reply = xcb_get_window_attributes_reply(_ecore_xcb_conn, cookie, NULL);
   if (!reply) return EINA_FALSE;

   memset(att_ret, 0, sizeof(Ecore_X_Window_Attributes));

   if (reply->map_state != XCB_MAP_STATE_UNMAPPED)
     att_ret->visible = EINA_TRUE;

   if (reply->map_state == XCB_MAP_STATE_VIEWABLE)
     att_ret->viewable = EINA_TRUE;

   if (reply->override_redirect) 
     att_ret->override = EINA_TRUE;

   if (reply->_class == XCB_WINDOW_CLASS_INPUT_ONLY)
     att_ret->input_only = EINA_TRUE;

   if (reply->save_under) 
     att_ret->save_under = EINA_TRUE;

   att_ret->event_mask.mine = reply->your_event_mask;
   att_ret->event_mask.all = reply->all_event_masks;
   att_ret->event_mask.no_propagate = reply->do_not_propagate_mask;
   att_ret->window_gravity = reply->win_gravity;
   att_ret->pixel_gravity = reply->bit_gravity;
   att_ret->colormap = reply->colormap;
   att_ret->visual = _ecore_xcb_window_find_visual_by_id(reply->visual);

   free(reply);

   gcookie = xcb_get_geometry_unchecked(_ecore_xcb_conn, win);
   greply = xcb_get_geometry_reply(_ecore_xcb_conn, gcookie, NULL);
   if (!greply) return EINA_TRUE;

   /* xcb_translate_coordinates_reply_t *trans; */
   /* xcb_query_tree_cookie_t tcookie; */
   /* xcb_query_tree_reply_t *treply; */

   /* tcookie = xcb_query_tree(_ecore_xcb_conn, win); */
   /* treply = xcb_query_tree_reply(_ecore_xcb_conn, tcookie, NULL); */

   /* trans =  */
   /*   xcb_translate_coordinates_reply(_ecore_xcb_conn,  */
   /*                                   xcb_translate_coordinates(_ecore_xcb_conn,  */
   /*                                                             win, treply->parent, greply->x, greply->y), NULL); */
   /* free(treply); */

   att_ret->root = greply->root;
   att_ret->depth = greply->depth;
//   att_ret->x = trans->dst_x;
//   att_ret->y = trans->dst_y;
   att_ret->x = greply->x;
   att_ret->y = greply->y;
   att_ret->w = greply->width;
   att_ret->h = greply->height;
   att_ret->border = greply->border_width;

//   free(trans);

   free(greply);
   return EINA_TRUE;
}

/**
 * Retrieves the size of the given window.
 * @param   win The given window.
 * @param   w   Pointer to an integer into which the width is to be stored.
 * @param   h   Pointer to an integer into which the height is to be stored.
 * @ingroup Ecore_X_Window_Geometry_Group
 */
EAPI void 
ecore_x_window_size_get(Ecore_X_Window win, int *width, int *height) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!win) win = ((xcb_screen_t *)_ecore_xcb_screen)->root;
   ecore_x_drawable_geometry_get(win, NULL, NULL, width, height);
}

/**
 * Set if a window should be ignored.
 * @param   win The given window.
 * @param   ignore if to ignore
 */
EAPI void 
ecore_x_window_ignore_set(Ecore_X_Window win, int ignore) 
{
   int i = 0, j = 0, count = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (ignore) 
     {
        if (ignore_list) 
          {
             for (i = 0; i < ignore_num; i++) 
               if (win == ignore_list[i]) return;

             ignore_list = 
               realloc(ignore_list, (ignore_num + 1) * sizeof(Ecore_X_Window));
             if (!ignore_list) return;

             ignore_list[ignore_num++] = win;
          }
        else 
          {
             ignore_num = 0;
             ignore_list = malloc(sizeof(Ecore_X_Window));
             if (!ignore_list) return;
             ignore_list[ignore_num++] = win;
          }
     }
   else 
     {
        if (!ignore_list) return;
        for (count = ignore_num, i = 0, j = 0; i < count; i++) 
          {
             if (win != ignore_list[i])
               ignore_list[j++] = ignore_list[i];
             else
               ignore_num--;
          }
        if (ignore_num <= 0) 
          {
             free(ignore_list);
             ignore_list = NULL;
             return;
          }

        ignore_list = 
          realloc(ignore_list, ignore_num * sizeof(Ecore_X_Window));
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
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (num) *num = ignore_num;
   return ignore_list;
}

/**
 * Get a list of all the root windows on the server.
 *
 * @note   The returned array will need to be freed after use.
 * @param  num_ret Pointer to integer to put number of windows returned in.
 * @return An array of all the root windows.  @c NULL is returned if memory
 *         could not be allocated for the list, or if @p num_ret is @c NULL.
 */
EAPI Ecore_X_Window *
ecore_x_window_root_list(int *num_ret) 
{
   xcb_screen_iterator_t iter;
   const xcb_setup_t *setup;
   uint8_t i, num;
   Ecore_X_Window *roots = NULL;
#ifdef ECORE_XCB_XPRINT
   const xcb_query_extension_reply_t *ext_reply;
#endif

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!num_ret) return NULL;
   *num_ret = 0;

   setup = xcb_get_setup(_ecore_xcb_conn);
   num = setup->roots_len;

#ifdef ECORE_XCB_XPRINT
   ext_reply = xcb_get_extension_data(_ecore_xcb_conn, &xcb_x_print_id);
   if ((ext_reply) && (ext_reply->present)) 
     {
        xcb_x_print_print_query_screens_cookie_t cookie;
        xcb_x_print_print_query_screens_reply_t *reply;

        cookie = xcb_x_print_print_query_screens_unchecked(_ecore_xcb_conn);
        reply = 
          xcb_x_print_print_query_screens_reply(_ecore_xcb_conn, cookie, NULL);
        if (reply) 
          {
             xcb_window_t *screens;
             int psnum = 0, overlap = 0, j = 0, k = 0;

             psnum = xcb_x_print_print_query_screens_roots_length(reply);
             screens = xcb_x_print_print_query_screens_roots(reply);
             for (i = 0; i < num; i++) 
               {
                  for (j = 0; j < psnum; j++) 
                    {
                       if ((_ecore_xcb_window_screen_of_display(i))->root == 
                           screens[j]) 
                         {
                            overlap++;
                         }
                    }
               }
             if (!(roots = malloc((num - overlap) 
                                  * sizeof(Ecore_X_Window)))) return NULL;
             for (i = 0; i < num; i++) 
               {
                  Eina_Bool is_print = EINA_FALSE;

                  for (j = 0; j < psnum; j++) 
                    {
                       if ((_ecore_xcb_window_screen_of_display(i))->root == 
                           screens[j]) 
                         {
                            is_print = EINA_TRUE;
                            break;
                         }
                    }
                  if (!is_print) 
                    {
                       xcb_screen_t *s;

                       s = _ecore_xcb_window_screen_of_display(i);
                       if (s) 
                         {
                            roots[k] = s->root;
                            k++;
                         }
                    }
               }
             *num_ret = k;
          }
        else 
          {
             /* Fallback to default method */
             iter = xcb_setup_roots_iterator(setup);
             if (!(roots = malloc(num * sizeof(Ecore_X_Window)))) return NULL;
             *num_ret = num;
             for (i = 0; iter.rem; xcb_screen_next(&iter), i++)
               roots[i] = iter.data->root;
          }
     }
   else 
     {
        /* Fallback to default method */
        iter = xcb_setup_roots_iterator(setup);
        if (!(roots = malloc(num * sizeof(Ecore_X_Window)))) return NULL;
        *num_ret = num;
        for (i = 0; iter.rem; xcb_screen_next(&iter), i++)
          roots[i] = iter.data->root;
     }
#else
   iter = xcb_setup_roots_iterator(setup);
   if (!(roots = malloc(num * sizeof(Ecore_X_Window)))) return NULL;
   *num_ret = num;
   for (i = 0; iter.rem; xcb_screen_next(&iter), i++)
     roots[i] = iter.data->root;
#endif

   return roots;
}

EAPI Ecore_X_Window *
ecore_x_window_children_get(Ecore_X_Window win, int *num) 
{
   xcb_query_tree_cookie_t cookie;
   xcb_query_tree_reply_t *reply;
   Ecore_X_Window *windows = NULL;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (num) *num = 0;
   cookie = xcb_query_tree_unchecked(_ecore_xcb_conn, win);
   reply = xcb_query_tree_reply(_ecore_xcb_conn, cookie, NULL);
   if (!reply) return NULL;

   if (num) *num = reply->children_len;
   if (reply->children_len > 0) 
     {
        windows = malloc(sizeof(Ecore_X_Window) * reply->children_len);
        if (windows) 
          {
             unsigned int i = 0;
             xcb_window_t *w;

             w = xcb_query_tree_children(reply);
             for (i = 0; i < reply->children_len; i++) 
               windows[i] = w[i];
          }
     }

   free(reply);
   return windows;
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
   xcb_get_geometry_cookie_t gcookie;
   xcb_get_geometry_reply_t *greply;
   Ecore_X_Window window = 0;

   /* LOGFN(__FILE__, __LINE__, __FUNCTION__); */

   gcookie = xcb_get_geometry_unchecked(_ecore_xcb_conn, win);
   greply = xcb_get_geometry_reply(_ecore_xcb_conn, gcookie, NULL);
   if (!greply) return 0;
   window = greply->root;
   free(greply);

   return window;
}

EAPI Ecore_X_Window 
ecore_x_window_root_first_get(void) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   return ((xcb_screen_t *)_ecore_xcb_screen)->root;
}

/**
 * Retrieves the geometry of the given window.
 *
 * Note that the x & y coordingates are relative to your parent.  In
 * particular for reparenting window managers - relative to you window border.
 * If you want screen coordinates either walk the window tree to the root,
 * else for ecore_evas applications see ecore_evas_geometry_get().  Elementary
 * applications can use elm_win_screen_position_get().
 *
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
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!win) win = ((xcb_screen_t *)_ecore_xcb_screen)->root;
   ecore_x_drawable_geometry_get(win, x, y, w, h);
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
   Ecore_X_Window root, win = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   root = ((xcb_screen_t *)_ecore_xcb_screen)->root;

   ecore_x_grab();
   win = _ecore_xcb_window_at_xy_get(root, 0, 0, x, y, NULL, 0);
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
   Ecore_X_Window root, win = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   root = ((xcb_screen_t *)_ecore_xcb_screen)->root;

   ecore_x_grab();
   win = _ecore_xcb_window_at_xy_get(root, 0, 0, x, y, skip, skip_num);
   ecore_x_ungrab();

   return win ? win : root;
}

EAPI Ecore_X_Window 
ecore_x_window_at_xy_begin_get(Ecore_X_Window begin, int x, int y) 
{
   Ecore_X_Window win = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   ecore_x_grab();
   win = _ecore_xcb_window_at_xy_get(begin, 0, 0, x, y, NULL, 0);
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
   xcb_query_tree_cookie_t cookie;
   xcb_query_tree_reply_t *reply;
   Ecore_X_Window window = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

//   if (!win) return 0;
   cookie = xcb_query_tree(_ecore_xcb_conn, win);
   reply = xcb_query_tree_reply(_ecore_xcb_conn, cookie, NULL);
   if (!reply) return 0;
   window = reply->parent;
   free(reply);

   return window;
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
   xcb_get_window_attributes_cookie_t cookie;
   xcb_get_window_attributes_reply_t *reply;
   int ret = EINA_FALSE;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   cookie = xcb_get_window_attributes_unchecked(_ecore_xcb_conn, win);
   reply = xcb_get_window_attributes_reply(_ecore_xcb_conn, cookie, NULL);
   if (!reply) return EINA_FALSE;

   if (reply->map_state == XCB_MAP_STATE_VIEWABLE) 
     ret = EINA_TRUE;

   free(reply);
   return ret;
}

EAPI void 
ecore_x_window_button_grab(Ecore_X_Window win, int button, Ecore_X_Event_Mask mask, int mod, int any_mod) 
{
   int i = 0;
   uint16_t m, locks[8], ev;
   uint8_t b;
   Ecore_X_Window *t;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   b = button;
   if (b == 0)
     b = XCB_BUTTON_INDEX_ANY;

   m = _ecore_xcb_window_modifiers_get(mod);
   if (any_mod) m = XCB_MOD_MASK_ANY;

   locks[0] = 0;
   locks[1] = ECORE_X_LOCK_CAPS;
   locks[2] = ECORE_X_LOCK_NUM;
   locks[3] = ECORE_X_LOCK_SCROLL;
   locks[4] = ECORE_X_LOCK_CAPS | ECORE_X_LOCK_NUM;
   locks[5] = ECORE_X_LOCK_CAPS | ECORE_X_LOCK_SCROLL;
   locks[6] = ECORE_X_LOCK_NUM | ECORE_X_LOCK_SCROLL;
   locks[7] = ECORE_X_LOCK_CAPS | ECORE_X_LOCK_NUM | ECORE_X_LOCK_SCROLL;

   ev = mask;
   for (i = 0; i < 8; i++) 
     xcb_grab_button(_ecore_xcb_conn, 0, win, ev, 
                     XCB_GRAB_MODE_SYNC, XCB_GRAB_MODE_ASYNC, 
                     XCB_NONE, XCB_NONE, b, m | locks[i]);

   _ecore_xcb_button_grabs_num++;
   t = realloc(_ecore_xcb_button_grabs, 
               _ecore_xcb_button_grabs_num * sizeof(Ecore_X_Window));
   if (!t) return;

   _ecore_xcb_button_grabs = t;
   _ecore_xcb_button_grabs[_ecore_xcb_button_grabs_num - 1] = win;
}

EAPI void 
ecore_x_window_button_ungrab(Ecore_X_Window win, int button, int mod, int any_mod) 
{
   int i = 0;
   uint16_t m = 0, locks[8];
   uint8_t b;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   b = button;
   if (b == 0) b = XCB_BUTTON_INDEX_ANY;

   m = _ecore_xcb_window_modifiers_get(mod);
   if (any_mod) m = XCB_MOD_MASK_ANY;

   locks[0] = 0;
   locks[1] = ECORE_X_LOCK_CAPS;
   locks[2] = ECORE_X_LOCK_NUM;
   locks[3] = ECORE_X_LOCK_SCROLL;
   locks[4] = ECORE_X_LOCK_CAPS | ECORE_X_LOCK_NUM;
   locks[5] = ECORE_X_LOCK_CAPS | ECORE_X_LOCK_SCROLL;
   locks[6] = ECORE_X_LOCK_NUM | ECORE_X_LOCK_SCROLL;
   locks[7] = ECORE_X_LOCK_CAPS | ECORE_X_LOCK_NUM | ECORE_X_LOCK_SCROLL;

   for (i = 0; i < 8; i++)
     xcb_ungrab_button(_ecore_xcb_conn, b, win, m | locks[i]);

   _ecore_xcb_sync_magic_send(1, win);
}

EAPI void 
ecore_x_window_key_grab(Ecore_X_Window win, const char *key, int mod, int any_mod) 
{
   xcb_keycode_t keycode = XCB_NO_SYMBOL;
   uint16_t m = 0, locks[8];
   int i = 0;
   Ecore_X_Window *t;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   keycode = _ecore_xcb_keymap_string_to_keycode(key);
   if (keycode == XCB_NO_SYMBOL) return;

   m = _ecore_xcb_window_modifiers_get(mod);
   if (any_mod) m = XCB_MOD_MASK_ANY;

   locks[0] = 0;
   locks[1] = ECORE_X_LOCK_CAPS;
   locks[2] = ECORE_X_LOCK_NUM;
   locks[3] = ECORE_X_LOCK_SCROLL;
   locks[4] = ECORE_X_LOCK_CAPS | ECORE_X_LOCK_NUM;
   locks[5] = ECORE_X_LOCK_CAPS | ECORE_X_LOCK_SCROLL;
   locks[6] = ECORE_X_LOCK_NUM | ECORE_X_LOCK_SCROLL;
   locks[7] = ECORE_X_LOCK_CAPS | ECORE_X_LOCK_NUM | ECORE_X_LOCK_SCROLL;

   for (i = 0; i < 8; i++)
     xcb_grab_key(_ecore_xcb_conn, 0, win, m | locks[i], 
                 keycode, XCB_GRAB_MODE_SYNC, XCB_GRAB_MODE_ASYNC);
   _ecore_xcb_key_grabs_num++;
   t = realloc(_ecore_xcb_key_grabs, 
               _ecore_xcb_key_grabs_num * sizeof(Ecore_X_Window));
   if (!t) return;
   _ecore_xcb_key_grabs = t;
   _ecore_xcb_key_grabs[_ecore_xcb_key_grabs_num - 1] = win;
}

EAPI void 
ecore_x_window_key_ungrab(Ecore_X_Window win, const char *key, int mod, int any_mod) 
{
   xcb_keycode_t keycode = XCB_NO_SYMBOL;
   uint16_t m = 0, locks[8];
   int i = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   keycode = _ecore_xcb_keymap_string_to_keycode(key);
   if (keycode == XCB_NO_SYMBOL) return;

   m = _ecore_xcb_window_modifiers_get(mod);
   if (any_mod) m = XCB_MOD_MASK_ANY;

   locks[0] = 0;
   locks[1] = ECORE_X_LOCK_CAPS;
   locks[2] = ECORE_X_LOCK_NUM;
   locks[3] = ECORE_X_LOCK_SCROLL;
   locks[4] = ECORE_X_LOCK_CAPS | ECORE_X_LOCK_NUM;
   locks[5] = ECORE_X_LOCK_CAPS | ECORE_X_LOCK_SCROLL;
   locks[6] = ECORE_X_LOCK_NUM | ECORE_X_LOCK_SCROLL;
   locks[7] = ECORE_X_LOCK_CAPS | ECORE_X_LOCK_NUM | ECORE_X_LOCK_SCROLL;

   for (i = 0; i < 8; i++)
     xcb_ungrab_key(_ecore_xcb_conn, keycode, win, m | locks[i]);

   _ecore_xcb_sync_magic_send(2, win);
}

/* local functions */
Ecore_X_Window 
_ecore_xcb_window_root_of_screen_get(int screen) 
{
   xcb_screen_iterator_t iter;

   iter = xcb_setup_roots_iterator(xcb_get_setup(_ecore_xcb_conn));
   for (; iter.rem; --screen, xcb_screen_next(&iter))
     if (screen == 0) 
       {
          xcb_screen_t *s;

          if ((s = iter.data))
            return s->root;
       }
   return 0;
}

static Ecore_X_Window 
_ecore_xcb_window_argb_internal_new(Ecore_X_Window parent, int x, int y, int w, int h, uint8_t override_redirect, uint8_t save_under) 
{
   Ecore_X_Window win = 0;
#ifdef ECORE_XCB_RENDER
   uint32_t value_list[10];
   uint32_t value_mask;
   uint32_t vis;
   Ecore_X_Colormap colormap;
#endif

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

#ifdef ECORE_XCB_RENDER
   if (parent == 0)
     parent = ((xcb_screen_t *)_ecore_xcb_screen)->root;

   vis = 
     _ecore_xcb_render_find_visual_id(XCB_RENDER_PICT_TYPE_DIRECT, EINA_TRUE);

   colormap = xcb_generate_id(_ecore_xcb_conn);
   xcb_create_colormap(_ecore_xcb_conn, XCB_COLORMAP_ALLOC_NONE, 
                       colormap, parent, vis);

   value_mask = (XCB_CW_BACK_PIXMAP | XCB_CW_BORDER_PIXEL | XCB_CW_BIT_GRAVITY |
                 XCB_CW_WIN_GRAVITY | XCB_CW_BACKING_STORE | 
                 XCB_CW_OVERRIDE_REDIRECT | XCB_CW_SAVE_UNDER | 
                 XCB_CW_EVENT_MASK | XCB_CW_DONT_PROPAGATE | XCB_CW_COLORMAP);

   value_list[0] = XCB_BACK_PIXMAP_NONE;
   value_list[1] = 0;
   value_list[2] = XCB_GRAVITY_NORTH_WEST;
   value_list[3] = XCB_GRAVITY_NORTH_WEST;
   value_list[4] = XCB_BACKING_STORE_NOT_USEFUL;
   value_list[5] = override_redirect;
   value_list[6] = save_under;
   value_list[7] = (XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE |
                    XCB_EVENT_MASK_BUTTON_PRESS | 
                    XCB_EVENT_MASK_BUTTON_RELEASE |
                    XCB_EVENT_MASK_ENTER_WINDOW | XCB_EVENT_MASK_LEAVE_WINDOW |
                    XCB_EVENT_MASK_POINTER_MOTION | XCB_EVENT_MASK_EXPOSURE |
                    XCB_EVENT_MASK_VISIBILITY_CHANGE | 
                    XCB_EVENT_MASK_STRUCTURE_NOTIFY |
                    XCB_EVENT_MASK_FOCUS_CHANGE | 
                    XCB_EVENT_MASK_PROPERTY_CHANGE |
                    XCB_EVENT_MASK_COLOR_MAP_CHANGE);
   value_list[8] = XCB_EVENT_MASK_NO_EVENT;
   value_list[9] = colormap;

   win = xcb_generate_id(_ecore_xcb_conn);
   xcb_create_window(_ecore_xcb_conn, 32, win, parent, x, y, w, h, 0,
                     XCB_WINDOW_CLASS_INPUT_OUTPUT, vis, value_mask, 
                     value_list);

   xcb_free_colormap(_ecore_xcb_conn, colormap);

   if (parent == ((xcb_screen_t *)_ecore_xcb_screen)->root)
     ecore_x_window_defaults_set(win);
#endif

   return win;
}

static Ecore_X_Window 
_ecore_xcb_window_at_xy_get(Ecore_X_Window base, int bx, int by, int x, int y, Ecore_X_Window *skip, int skip_num) 
{
   xcb_query_tree_cookie_t cookie;
   xcb_query_tree_reply_t *reply;
   Ecore_X_Window *windows = NULL;
   int wx, wy, ww, wh, num, i = 0;
   Eina_Bool skipit = EINA_FALSE;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ecore_x_window_visible_get(base)) return 0;

   ecore_x_window_geometry_get(base, &wx, &wy, &ww, &wh);
   wx += bx;
   wy += by;

   if (!((x >= wx) && (y >= wy) && (x < (wx + ww)) && (y < (wy + wh))))
     return 0;

   cookie = xcb_query_tree_unchecked(_ecore_xcb_conn, base);
   reply = xcb_query_tree_reply(_ecore_xcb_conn, cookie, NULL);
   if (!reply) return 0;

   num = reply->children_len;
   windows = xcb_query_tree_children(reply);

   for (i = (num - 1); i >= 0; --i) 
     {
        skipit = EINA_FALSE;

        if (skip) 
          {
             int j = 0;

             for (j = 0; j < skip_num; j++) 
               {
                  if (windows[i] == skip[j]) 
                    {
                       skipit = EINA_TRUE;
                       goto onward;
                    }
               }
          }
onward:
        if (!skipit) 
          {
             Ecore_X_Window child = 0;

             child = 
               _ecore_xcb_window_at_xy_get(windows[i], 
                                           wx, wy, x, y, skip, skip_num);
             if (child) 
               {
                  if (reply) free(reply);
                  return child;
               }
          }
     }

   if (reply) free(reply);
   return base;
}

Ecore_X_Visual 
_ecore_xcb_window_visual_get(Ecore_X_Window win) 
{
   xcb_get_window_attributes_cookie_t cookie;
   xcb_get_window_attributes_reply_t *reply;
   Ecore_X_Visual visual = 0;

   cookie = xcb_get_window_attributes(_ecore_xcb_conn, win);
   reply = xcb_get_window_attributes_reply(_ecore_xcb_conn, cookie, NULL);
   if (!reply) return 0;
   visual = _ecore_xcb_window_find_visual_by_id(reply->visual);
   free(reply);

   return visual;
}

void 
_ecore_xcb_window_button_grab_remove(Ecore_X_Window win) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (_ecore_xcb_button_grabs_num > 0) 
     {
        int i = 0, shuffle = 0;

        for (i = 0; i < _ecore_xcb_button_grabs_num; i++) 
          {
             if (shuffle)
               _ecore_xcb_button_grabs[i - 1] = _ecore_xcb_button_grabs[i];

             if ((!shuffle) && (_ecore_xcb_button_grabs[i] == win))
               shuffle = 1;
          }

        if (shuffle) 
          {
             Ecore_X_Window *t;

             _ecore_xcb_button_grabs_num--;
             if (_ecore_xcb_button_grabs_num <= 0) 
               {
                  free(_ecore_xcb_button_grabs);
                  _ecore_xcb_button_grabs = NULL;
                  return;
               }

             t = realloc(_ecore_xcb_button_grabs, 
                         _ecore_xcb_button_grabs_num * sizeof(Ecore_X_Window));
             if (!t) return;
             _ecore_xcb_button_grabs = t;
          }
     }
}

void 
_ecore_xcb_window_key_grab_remove(Ecore_X_Window win) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (_ecore_xcb_key_grabs_num > 0) 
     {
        int i = 0, shuffle = 0;

        for (i = 0; i < _ecore_xcb_key_grabs_num; i++) 
          {
             if (shuffle)
               _ecore_xcb_key_grabs[i - 1] = _ecore_xcb_key_grabs[i];

             if ((!shuffle) && (_ecore_xcb_key_grabs[i] == win))
               shuffle = 1;
          }

        if (shuffle) 
          {
             Ecore_X_Window *t;

             _ecore_xcb_key_grabs_num--;
             if (_ecore_xcb_key_grabs_num <= 0) 
               {
                  free(_ecore_xcb_key_grabs);
                  _ecore_xcb_key_grabs = NULL;
                  return;
               }

             t = realloc(_ecore_xcb_key_grabs, 
                         _ecore_xcb_key_grabs_num * sizeof(Ecore_X_Window));
             if (!t) return;
             _ecore_xcb_key_grabs = t;
          }
     }
}

void 
_ecore_xcb_window_grab_allow_events(Ecore_X_Window event_win, Ecore_X_Window child_win, int type, void *event, Ecore_X_Time timestamp) 
{
   int i = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   for (i = 0; i < _ecore_xcb_button_grabs_num; i++) 
     {
        if ((_ecore_xcb_button_grabs[i] == event_win) || 
            (_ecore_xcb_button_grabs[i] == child_win)) 
          {
             Eina_Bool replay = EINA_FALSE;

             if (_ecore_xcb_window_grab_replay_func) 
               {
                  replay = 
                    _ecore_xcb_window_grab_replay_func(_ecore_xcb_window_grab_replay_data, 
                                                       type, event);
               }
             if (replay) 
               {
                  xcb_allow_events(_ecore_xcb_conn, 
                                   XCB_ALLOW_REPLAY_POINTER, timestamp);
               }
             else 
               {
                  xcb_allow_events(_ecore_xcb_conn, 
                                   XCB_ALLOW_ASYNC_POINTER, timestamp);
               }
             break;
          }
     }
}

static int 
_ecore_xcb_window_modifiers_get(unsigned int state) 
{
   int xmodifiers = 0;

   if (state & ECORE_EVENT_MODIFIER_SHIFT)
      xmodifiers |= ECORE_X_MODIFIER_SHIFT;
   if (state & ECORE_EVENT_MODIFIER_CTRL)
      xmodifiers |= ECORE_X_MODIFIER_CTRL;
   if (state & ECORE_EVENT_MODIFIER_ALT)
      xmodifiers |= ECORE_X_MODIFIER_ALT;
   if (state & ECORE_EVENT_MODIFIER_WIN)
      xmodifiers |= ECORE_X_MODIFIER_WIN;
   if (state & ECORE_EVENT_LOCK_SCROLL)
      xmodifiers |= ECORE_X_LOCK_SCROLL;
   if (state & ECORE_EVENT_LOCK_NUM)
      xmodifiers |= ECORE_X_LOCK_NUM;
   if (state & ECORE_EVENT_LOCK_CAPS)
      xmodifiers |= ECORE_X_LOCK_CAPS;
   if (state & ECORE_EVENT_LOCK_SHIFT)
      xmodifiers |= ECORE_X_LOCK_SHIFT;

   return xmodifiers;
}

static xcb_visualtype_t *
_ecore_xcb_window_find_visual_by_id(xcb_visualid_t id) 
{
   xcb_depth_iterator_t diter;
   xcb_visualtype_iterator_t viter;

   diter = xcb_screen_allowed_depths_iterator(_ecore_xcb_screen);
   for (; diter.rem; xcb_depth_next(&diter)) 
     {
        viter = xcb_depth_visuals_iterator(diter.data);
        for (; viter.rem; xcb_visualtype_next(&viter)) 
          {
             if (viter.data->visual_id == id)
               return viter.data;
          }
     }
   return 0;
}

static xcb_screen_t *
_ecore_xcb_window_screen_of_display(int screen) 
{
   xcb_screen_iterator_t iter;

   iter = xcb_setup_roots_iterator(xcb_get_setup(_ecore_xcb_conn));
   for (; iter.rem; --screen, xcb_screen_next(&iter))
     if (screen == 0)
       return iter.data;

   return NULL;
}
