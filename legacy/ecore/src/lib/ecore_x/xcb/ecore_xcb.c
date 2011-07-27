#include "ecore_xcb_private.h"
#include <X11/Xlib-xcb.h>
#include <dlfcn.h>

/* local function prototypes */
static int _ecore_xcb_shutdown(Eina_Bool close_display);
static Eina_Bool _ecore_xcb_fd_handle(void *data, Ecore_Fd_Handler *hdlr __UNUSED__);
static Eina_Bool _ecore_xcb_fd_handle_buff(void *data, Ecore_Fd_Handler *hdlr __UNUSED__);

/* local variables */
static int _ecore_xcb_init_count = 0;
static int _ecore_xcb_grab_count = 0;
static Ecore_Fd_Handler *_ecore_xcb_fd_handler = NULL;
static xcb_generic_event_t *_ecore_xcb_event_buffered = NULL;

/* external variables */
int _ecore_xcb_log_dom = -1;
Ecore_X_Display *_ecore_xcb_display = NULL;
Ecore_X_Connection *_ecore_xcb_conn = NULL;
Ecore_X_Screen *_ecore_xcb_screen = NULL;
int _ecore_xcb_event_handlers_num = 0;
Ecore_X_Atom _ecore_xcb_atoms_wm_protocol[ECORE_X_WM_PROTOCOL_NUM];
double _ecore_xcb_double_click_time = 0.25;

/**
 * @defgroup Ecore_X_Init_Group X Library Init and Shutdown Functions
 *
 * Functions that start and shut down the Ecore X Library.
 */

/**
 * Initialize the X display connection to the given display.
 *
 * @param   name Display target name.  If @c NULL, the default display is
 *               assumed.
 * @return  The number of times the library has been initialized without
 *          being shut down.  0 is returned if an error occurs.
 * @ingroup Ecore_X_Init_Group
 */
EAPI int 
ecore_x_init(const char *name) 
{
   char *gl = NULL;
   uint32_t mask, list[1];

   /* check if we have initialized already */
   if (++_ecore_xcb_init_count != 1) 
     return _ecore_xcb_init_count;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   /* try to initialize eina
    if (!eina_init()) return --_ecore_xcb_init_count;
    */

   /* try to initialize ecore */
   if (!ecore_init()) 
     {
        /* unregister log domain */
        /* eina_log_domain_unregister(_ecore_xcb_log_dom); */
        /* _ecore_xcb_log_dom = -1; */
        /* eina_shutdown(); */
        return --_ecore_xcb_init_count;
     }

   /* setup ecore_xcb log domain */
   _ecore_xcb_log_dom = 
     eina_log_domain_register("ecore_x", ECORE_XCB_DEFAULT_LOG_COLOR);
   if (_ecore_xcb_log_dom < 0) 
     {
        EINA_LOG_ERR("Cannot create Ecore Xcb log domain");
        ecore_shutdown();
        return --_ecore_xcb_init_count;
     }

   /* try to initialize ecore_event */
   if (!ecore_event_init()) 
     {
        /* unregister log domain */
        eina_log_domain_unregister(_ecore_xcb_log_dom);
        _ecore_xcb_log_dom = -1;
        ecore_shutdown();
        return --_ecore_xcb_init_count;
     }

   /* check for env var which says we are not going to use GL @ all
    * 
    * NB: This is done because if someone wants a 'pure' xcb implementation 
    * of ecore_x, all they need do is export this variable in the environment 
    * and ecore_x will not use xlib stuff at all. 
    * 
    * The upside is you can get pure xcb-based ecore_x (w/ all the speed), but 
    * there is a down-side here in that you cannot get OpenGL without XLib :( 
    */
   if ((gl = getenv("ECORE_X_NO_XLIB")))
     {
        /* we found the env var that says 'Yes, we are not ever gonna try 
         * OpenGL so it is safe to not use XLib at all' */

        /* try to connect to the display server */
         _ecore_xcb_conn = xcb_connect(name, NULL);
     }
   else 
     {
        /* env var was not specified, so we will assume that the user 
         * may want opengl @ some point. connect this way for opengl to work */

        /* want to dlopen here to avoid actual library linkage */
        void *libxcb, *libxlib;
        Display *(*_real_display)(const char *display);
        xcb_connection_t *(*_real_connection)(Display *dpy);
        void (*_real_queue)(Display *dpy, enum XEventQueueOwner owner);

        libxlib = dlopen("libX11.so", (RTLD_LAZY | RTLD_GLOBAL));
        if (!libxlib) 
          libxlib = dlopen("libX11.so.6", (RTLD_LAZY | RTLD_GLOBAL));
        if (!libxlib) 
          libxlib = dlopen("libX11.so.6.3.0", (RTLD_LAZY | RTLD_GLOBAL));
        if (!libxlib) 
          {
             ERR("Could not dlsym to libX11");
             /* unregister log domain */
             eina_log_domain_unregister(_ecore_xcb_log_dom);
             _ecore_xcb_log_dom = -1;
             ecore_shutdown();
             return --_ecore_xcb_init_count;
          }

        libxcb = dlopen("libX11-xcb.so", (RTLD_LAZY | RTLD_GLOBAL));
        if (!libxcb) 
          libxcb = dlopen("libX11-xcb.so.1", (RTLD_LAZY | RTLD_GLOBAL));
        if (!libxcb) 
          libxcb = dlopen("libX11-xcb.so.1.0.0", (RTLD_LAZY | RTLD_GLOBAL));
        if (!libxcb) 
          {
             ERR("Could not dlsym to libX11-xcb");
             /* unregister log domain */
             eina_log_domain_unregister(_ecore_xcb_log_dom);
             _ecore_xcb_log_dom = -1;
             ecore_shutdown();
             return --_ecore_xcb_init_count;
          }

        _real_display = dlsym(libxlib, "XOpenDisplay");
        _real_connection = dlsym(libxcb, "XGetXCBConnection");
        _real_queue = dlsym(libxcb, "XSetEventQueueOwner");

        if (_real_display) 
          {
             _ecore_xcb_display = _real_display(name);
             if (_real_connection) 
               _ecore_xcb_conn = _real_connection(_ecore_xcb_display);
             if (_real_queue) 
               _real_queue(_ecore_xcb_display, XCBOwnsEventQueue);
          }
     }

   if (xcb_connection_has_error(_ecore_xcb_conn))
     {
        CRIT("XCB Connection has error");
        eina_log_domain_unregister(_ecore_xcb_log_dom);
        _ecore_xcb_log_dom = -1;
        ecore_event_shutdown();
        ecore_shutdown();
        return --_ecore_xcb_init_count;
     }

   /* grab the default screen */
   _ecore_xcb_screen = 
     xcb_setup_roots_iterator(xcb_get_setup(_ecore_xcb_conn)).data;

   /* NB: This method of init/finalize extensions first, then atoms
    * Does end up being 2 round trips to X, BUT if we do extensions init then 
    * atoms init first, and call the 'finalize' functions later, we end up 
    * being slower, so it's a trade-off. This current method clocks in 
    * around 0.003 for fetching atoms VS 0.010 for init both then finalize */

   /* prefetch extension data */
   _ecore_xcb_extensions_init();

   /* finalize extensions */
   _ecore_xcb_extensions_finalize();

   /* setup xcb events */
   _ecore_xcb_events_init();

   /* set keyboard autorepeat */
   mask = XCB_KB_AUTO_REPEAT_MODE;
   list[0] = XCB_AUTO_REPEAT_MODE_ON;
   xcb_change_keyboard_control(_ecore_xcb_conn, mask, list);

   /* setup xcb keymasks */
   _ecore_xcb_keymap_init();

   /* finalize xcb keymasks */
   _ecore_xcb_keymap_finalize();

   /* setup ecore fd handler */
   _ecore_xcb_fd_handler = 
     ecore_main_fd_handler_add(xcb_get_file_descriptor(_ecore_xcb_conn), 
                               ECORE_FD_READ, _ecore_xcb_fd_handle, 
                               _ecore_xcb_conn, _ecore_xcb_fd_handle_buff, 
                               _ecore_xcb_conn);
   if (!_ecore_xcb_fd_handler) 
     return _ecore_xcb_shutdown(EINA_TRUE);

   /* prefetch atoms */
   _ecore_xcb_atoms_init();

   /* finalize atoms */
   _ecore_xcb_atoms_finalize();

   /* icccm_init: dummy function */
   ecore_x_icccm_init();

   /* setup netwm */
   ecore_x_netwm_init();

   /* old e hints init: dummy function */
   ecore_x_e_init();

   _ecore_xcb_atoms_wm_protocol[ECORE_X_WM_PROTOCOL_DELETE_REQUEST] = 
     ECORE_X_ATOM_WM_DELETE_WINDOW;
   _ecore_xcb_atoms_wm_protocol[ECORE_X_WM_PROTOCOL_TAKE_FOCUS] = 
     ECORE_X_ATOM_WM_TAKE_FOCUS;
   _ecore_xcb_atoms_wm_protocol[ECORE_X_NET_WM_PROTOCOL_PING] = 
     ECORE_X_ATOM_NET_WM_PING;
   _ecore_xcb_atoms_wm_protocol[ECORE_X_NET_WM_PROTOCOL_SYNC_REQUEST] = 
     ECORE_X_ATOM_NET_WM_SYNC_REQUEST;

   /* setup selection */
   _ecore_xcb_selection_init();

   /* setup dnd */
   _ecore_xcb_dnd_init();

   return _ecore_xcb_init_count;
}

/**
 * Shuts down the Ecore X library.
 *
 * In shutting down the library, the X display connection is terminated
 * and any event handlers for it are removed.
 *
 * @return  The number of times the library has been initialized without
 *          being shut down.
 * @ingroup Ecore_X_Init_Group
 */
EAPI int 
ecore_x_shutdown(void) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   return _ecore_xcb_shutdown(EINA_TRUE);
}

/**
 * Shuts down the Ecore X library.
 *
 * As ecore_x_shutdown, except do not close Display, only connection.
 *
 * @ingroup Ecore_X_Init_Group
 */
EAPI int 
ecore_x_disconnect(void) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   return _ecore_xcb_shutdown(EINA_FALSE);
}

/**
 * @defgroup Ecore_X_Flush_Group X Synchronization Functions
 *
 * Functions that ensure that all commands that have been issued by the
 * Ecore X library have been sent to the server.
 */

/**
 * Sends all X commands in the X Display buffer.
 * @ingroup Ecore_X_Flush_Group
 */
EAPI void 
ecore_x_flush(void) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   xcb_flush(_ecore_xcb_conn);
}

/**
 * Retrieves the Ecore_X_Screen handle used for the current X connection.
 * @return  The current default screen.
 * @ingroup Ecore_X_Display_Attr_Group
 */
EAPI Ecore_X_Screen *
ecore_x_default_screen_get(void) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   return (Ecore_X_Screen *)_ecore_xcb_screen;
}

EAPI Ecore_X_Connection *
ecore_x_connection_get(void) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   return (Ecore_X_Connection *)_ecore_xcb_conn;
}

/**
 * Return the last event time
 */
EAPI Ecore_X_Time 
ecore_x_current_time_get(void) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   return _ecore_xcb_events_last_time_get();
}

/**
 * Flushes the command buffer and waits until all requests have been
 * processed by the server.
 * @ingroup Ecore_X_Flush_Group
 */
EAPI void 
ecore_x_sync(void) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   free(xcb_get_input_focus_reply(_ecore_xcb_conn, 
                                  xcb_get_input_focus(_ecore_xcb_conn), NULL));
}

EAPI void 
ecore_x_grab(void) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   _ecore_xcb_grab_count++;
   if (_ecore_xcb_grab_count == 1) 
     xcb_grab_server(_ecore_xcb_conn);
}

EAPI void 
ecore_x_ungrab(void) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   _ecore_xcb_grab_count--;
   if (_ecore_xcb_grab_count < 0) _ecore_xcb_grab_count = 0;
   if (_ecore_xcb_grab_count == 0) 
     xcb_ungrab_server(_ecore_xcb_conn);
}

/**
 * Send client message with given type and format 32.
 *
 * @param win     The window the message is sent to.
 * @param type    The client message type.
 * @param d0      The client message data item 1
 * @param d1      The client message data item 2
 * @param d2      The client message data item 3
 * @param d3      The client message data item 4
 * @param d4      The client message data item 5
 *
 * @return EINA_TRUE on success EINA_FALSE otherwise.
 */
EAPI Eina_Bool 
ecore_x_client_message32_send(Ecore_X_Window win, Ecore_X_Atom type, Ecore_X_Event_Mask mask, long d0, long d1, long d2, long d3, long d4) 
{
   xcb_client_message_event_t ev;
   xcb_void_cookie_t cookie;
   xcb_generic_error_t *err;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   memset(&ev, 0, sizeof(xcb_client_message_event_t));

   ev.response_type = XCB_CLIENT_MESSAGE;
   ev.format = 32;
   ev.window = win;
   ev.type = type;
   ev.data.data32[0] = (uint32_t)d0;
   ev.data.data32[1] = (uint32_t)d1;
   ev.data.data32[2] = (uint32_t)d2;
   ev.data.data32[3] = (uint32_t)d3;
   ev.data.data32[4] = (uint32_t)d4;

   // FIXME: Use unchecked version after development is ironed out
   cookie = 
     xcb_send_event_checked(_ecore_xcb_conn, 0, win, mask, (const char *)&ev);
   err = xcb_request_check(_ecore_xcb_conn, cookie);
   if (err) 
     {
        DBG("Problem Sending Event");
        DBG("\tType: %d", type);
        DBG("\tWin: %d", win);
        _ecore_xcb_error_handle(err);
        free(err);
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

/**
 * Send client message with given type and format 8.
 *
 * @param win     The window the message is sent to.
 * @param type    The client message type.
 * @param data    Data to be sent.
 * @param len     Number of data bytes, max 20.
 *
 * @return EINA_TRUE on success EINA_FALSE otherwise.
 */
EAPI Eina_Bool 
ecore_x_client_message8_send(Ecore_X_Window win, Ecore_X_Atom type, const void *data, int len) 
{
   xcb_client_message_event_t ev;
   xcb_void_cookie_t cookie;
   xcb_generic_error_t *err;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   memset(&ev, 0, sizeof(xcb_client_message_event_t));

   ev.response_type = XCB_CLIENT_MESSAGE;
   ev.format = 8;
   ev.window = win;
   ev.type = type;
   if (len > 20) len = 20;
   memcpy(ev.data.data8, data, len);
   memset(ev.data.data8 + len, 0, 20 - len);

   // FIXME: Use unchecked version after development is ironed out
   cookie = 
     xcb_send_event_checked(_ecore_xcb_conn, 0, win, 
                            XCB_EVENT_MASK_NO_EVENT, (const char *)&ev);
   err = xcb_request_check(_ecore_xcb_conn, cookie);
   if (err) 
     {
        DBG("Problem Sending Event");
        DBG("\tType: %d", type);
        DBG("\tWin: %d", win);
        _ecore_xcb_error_handle(err);
        free(err);
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

EAPI Eina_Bool 
ecore_x_mouse_down_send(Ecore_X_Window win, int x, int y, int b) 
{
   xcb_translate_coordinates_cookie_t cookie;
   xcb_translate_coordinates_reply_t *reply;
   xcb_button_press_event_t ev;
   xcb_void_cookie_t vcookie;
   xcb_generic_error_t *err;
   Ecore_X_Window root = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   root = ecore_x_window_root_get(win);
   cookie = xcb_translate_coordinates(_ecore_xcb_conn, win, root, x, y);
   reply = xcb_translate_coordinates_reply(_ecore_xcb_conn, cookie, NULL);
   if (!reply) return EINA_FALSE;

   memset(&ev, 0, sizeof(xcb_button_press_event_t));

   ev.response_type = XCB_BUTTON_PRESS; 
   ev.event = win;
   ev.child = win;
   ev.root = root;
   ev.event_x = x;
   ev.event_y = y;
   ev.same_screen = 1;
   ev.state = 1 << b;
   ev.detail = b; // xcb uses detail for button
   ev.root_x = reply->dst_x;
   ev.root_y = reply->dst_y;
   ev.time = ecore_x_current_time_get();
   free(reply);

   // FIXME: Use unchecked version after development is ironed out
   vcookie = 
     xcb_send_event_checked(_ecore_xcb_conn, 1, win, 
                            XCB_EVENT_MASK_BUTTON_PRESS, (const char *)&ev);
   err = xcb_request_check(_ecore_xcb_conn, vcookie);
   if (err) 
     {
        _ecore_xcb_error_handle(err);
        free(err);
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

EAPI Eina_Bool 
ecore_x_mouse_up_send(Ecore_X_Window win, int x, int y, int b) 
{
   xcb_translate_coordinates_cookie_t cookie;
   xcb_translate_coordinates_reply_t *reply;
   xcb_button_release_event_t ev;
   xcb_void_cookie_t vcookie;
   xcb_generic_error_t *err;
   Ecore_X_Window root = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   root = ecore_x_window_root_get(win);
   cookie = xcb_translate_coordinates(_ecore_xcb_conn, win, root, x, y);
   reply = xcb_translate_coordinates_reply(_ecore_xcb_conn, cookie, NULL);
   if (!reply) return EINA_FALSE;

   memset(&ev, 0, sizeof(xcb_button_release_event_t));

   ev.response_type = XCB_BUTTON_RELEASE;
   ev.event = win;
   ev.child = win;
   ev.root = root;
   ev.event_x = x;
   ev.event_y = y;
   ev.same_screen = 1;
   ev.state = 0;
   ev.root_x = reply->dst_x;
   ev.root_y = reply->dst_y;
   ev.detail = b; // xcb uses detail for button
   ev.time = ecore_x_current_time_get();
   free(reply);

   // FIXME: Use unchecked version after development is ironed out
   vcookie = 
     xcb_send_event_checked(_ecore_xcb_conn, 1, win, 
                            XCB_EVENT_MASK_BUTTON_RELEASE, (const char *)&ev);
   err = xcb_request_check(_ecore_xcb_conn, vcookie);
   if (err) 
     {
        _ecore_xcb_error_handle(err);
        free(err);
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

EAPI Eina_Bool 
ecore_x_mouse_move_send(Ecore_X_Window win, int x, int y) 
{
   xcb_translate_coordinates_cookie_t cookie;
   xcb_translate_coordinates_reply_t *reply;
   xcb_motion_notify_event_t ev;
   xcb_void_cookie_t vcookie;
   xcb_generic_error_t *err;
   Ecore_X_Window root = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   root = ecore_x_window_root_get(win);
   cookie = xcb_translate_coordinates(_ecore_xcb_conn, win, root, x, y);
   reply = xcb_translate_coordinates_reply(_ecore_xcb_conn, cookie, NULL);
   if (!reply) return EINA_FALSE;

   memset(&ev, 0, sizeof(xcb_motion_notify_event_t));

   ev.response_type = XCB_MOTION_NOTIFY;
   ev.event = win;
   ev.child = win;
   ev.root = root;
   ev.event_x = x;
   ev.event_y = y;
   ev.same_screen = 1;
   ev.state = 0;
   ev.detail = 0; // xcb uses 'detail' for is_hint
   ev.root_x = reply->dst_x;
   ev.root_y = reply->dst_y;
   ev.time = ecore_x_current_time_get();
   free(reply);

   // FIXME: Use unchecked version after development is ironed out
   vcookie = 
     xcb_send_event_checked(_ecore_xcb_conn, 1, win, 
                            XCB_EVENT_MASK_POINTER_MOTION, (const char *)&ev);
   err = xcb_request_check(_ecore_xcb_conn, vcookie);
   if (err) 
     {
        _ecore_xcb_error_handle(err);
        free(err);
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

EAPI Eina_Bool 
ecore_x_keyboard_grab(Ecore_X_Window win) 
{
   xcb_grab_keyboard_cookie_t cookie;
   xcb_grab_keyboard_reply_t *reply;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   cookie = 
     xcb_grab_keyboard_unchecked(_ecore_xcb_conn, 0, win, XCB_CURRENT_TIME, 
                                 XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);
   reply = xcb_grab_keyboard_reply(_ecore_xcb_conn, cookie, NULL);
   if (!reply) return EINA_FALSE;
   free(reply);
   return EINA_TRUE;
}

EAPI void 
ecore_x_keyboard_ungrab(void) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   xcb_ungrab_keyboard(_ecore_xcb_conn, XCB_CURRENT_TIME);
}

EAPI void 
ecore_x_pointer_xy_get(Ecore_X_Window win, int *x, int *y) 
{
   xcb_query_pointer_cookie_t cookie;
   xcb_query_pointer_reply_t *reply;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

//   if (!win) win = ((xcb_screen_t *)_ecore_xcb_screen)->root;

   if (x) *x = -1;
   if (y) *y = -1;

   cookie = xcb_query_pointer(_ecore_xcb_conn, win);
   reply = xcb_query_pointer_reply(_ecore_xcb_conn, cookie, NULL);
   if (!reply) return;
   if (x) *x = reply->win_x;
   if (y) *y = reply->win_y;
   free(reply);
}

EAPI Eina_Bool 
ecore_x_pointer_control_set(int accel_num, int accel_denom, int threshold) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   xcb_change_pointer_control(_ecore_xcb_conn, 
                              accel_num, accel_denom, threshold, 1, 1);
   return EINA_TRUE;
}

EAPI Eina_Bool 
ecore_x_pointer_control_get(int *accel_num, int *accel_denom, int *threshold) 
{
   xcb_get_pointer_control_cookie_t cookie;
   xcb_get_pointer_control_reply_t *reply;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (accel_num) *accel_num = 0;
   if (accel_denom) *accel_denom = 0;
   if (threshold) *threshold = 0;

   cookie = xcb_get_pointer_control_unchecked(_ecore_xcb_conn);
   reply = xcb_get_pointer_control_reply(_ecore_xcb_conn, cookie, NULL);
   if (!reply) return EINA_FALSE;

   if (accel_num) *accel_num = reply->acceleration_numerator;
   if (accel_denom) *accel_denom = reply->acceleration_denominator;
   if (threshold) *threshold = reply->threshold;
   free(reply);

   return EINA_TRUE;
}

EAPI Eina_Bool 
ecore_x_pointer_mapping_set(unsigned char *map, int nmap) 
{
   xcb_set_pointer_mapping_cookie_t cookie;
   xcb_set_pointer_mapping_reply_t *reply;
   Eina_Bool ret = EINA_FALSE;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   cookie = xcb_set_pointer_mapping_unchecked(_ecore_xcb_conn, nmap, map);
   reply = xcb_set_pointer_mapping_reply(_ecore_xcb_conn, cookie, NULL);
   if (!reply) return EINA_FALSE;
   ret = 
     (reply->status == XCB_MAPPING_STATUS_SUCCESS) ? EINA_TRUE : EINA_FALSE;

   free(reply);
   return ret;
}

EAPI Eina_Bool 
ecore_x_pointer_mapping_get(unsigned char *map, int nmap) 
{
   xcb_get_pointer_mapping_cookie_t cookie;
   xcb_get_pointer_mapping_reply_t *reply;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (map) *map = 0;
   nmap = 0;

   cookie = xcb_get_pointer_mapping_unchecked(_ecore_xcb_conn);
   reply = xcb_get_pointer_mapping_reply(_ecore_xcb_conn, cookie, NULL);
   if (!reply) return EINA_FALSE;

   nmap = xcb_get_pointer_mapping_map_length(reply);
   if (nmap <= 0)
     {
        free(reply);
        return EINA_FALSE;
     }

   if (map) 
     {
        uint8_t *tmp;
        int i = 0;

        tmp = xcb_get_pointer_mapping_map(reply);
        for (i = 0; i < nmap; i++)
          map[i] = tmp[i];
     }

   free(reply);
   return EINA_TRUE;
}

EAPI Eina_Bool 
ecore_x_pointer_grab(Ecore_X_Window win) 
{
   xcb_grab_pointer_cookie_t cookie;
   xcb_grab_pointer_reply_t *reply;
   uint16_t mask;
   Eina_Bool ret = EINA_FALSE;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   mask = (XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE | 
           XCB_EVENT_MASK_ENTER_WINDOW | XCB_EVENT_MASK_LEAVE_WINDOW | 
           XCB_EVENT_MASK_POINTER_MOTION);

   cookie = xcb_grab_pointer_unchecked(_ecore_xcb_conn, 0, win, mask, 
                                       XCB_GRAB_MODE_ASYNC, 
                                       XCB_GRAB_MODE_ASYNC, 
                                       XCB_NONE, XCB_NONE, XCB_CURRENT_TIME);
   reply = xcb_grab_pointer_reply(_ecore_xcb_conn, cookie, NULL);
   if (!reply) return EINA_FALSE;

   ret = (reply->status == XCB_GRAB_STATUS_SUCCESS) ? EINA_TRUE : EINA_FALSE;

   free(reply);
   return ret;
}

EAPI Eina_Bool 
ecore_x_pointer_confine_grab(Ecore_X_Window win) 
{
   xcb_grab_pointer_cookie_t cookie;
   xcb_grab_pointer_reply_t *reply;
   uint16_t mask;
   Eina_Bool ret = EINA_FALSE;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   mask = (XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE | 
           XCB_EVENT_MASK_ENTER_WINDOW | XCB_EVENT_MASK_LEAVE_WINDOW | 
           XCB_EVENT_MASK_POINTER_MOTION);

   cookie = xcb_grab_pointer_unchecked(_ecore_xcb_conn, 0, win, mask, 
                                       XCB_GRAB_MODE_ASYNC, 
                                       XCB_GRAB_MODE_ASYNC, 
                                       win, XCB_NONE, XCB_CURRENT_TIME);
   reply = xcb_grab_pointer_reply(_ecore_xcb_conn, cookie, NULL);
   if (!reply) return EINA_FALSE;

   ret = (reply->status == XCB_GRAB_STATUS_SUCCESS) ? EINA_TRUE : EINA_FALSE;

   free(reply);
   return ret;
}

EAPI void 
ecore_x_pointer_ungrab(void) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   xcb_ungrab_pointer(_ecore_xcb_conn, XCB_CURRENT_TIME);
}

EAPI Eina_Bool 
ecore_x_pointer_warp(Ecore_X_Window win, int x, int y) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   xcb_warp_pointer(_ecore_xcb_conn, XCB_NONE, win, 0, 0, 0, 0, x, y);
   return EINA_TRUE;
}

/**
 * Invoke the standard system beep to alert users
 *
 * @param percent The volume at which the bell rings. Must be in the range
 * [-100,+100]. If percent >= 0, the final volume will be:
 *       base - [(base * percent) / 100] + percent
 * Otherwise, it's calculated as:
 *       base + [(base * percent) / 100]
 * where @c base is the bell's base volume as set by XChangeKeyboardControl(3).
 *
 * @returns EINA_TRUE on success, EINA_FALSE otherwise.
 */
EAPI Eina_Bool 
ecore_x_bell(int percent) 
{
   xcb_void_cookie_t cookie;
   xcb_generic_error_t *err;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   // FIXME: Use unchecked version after development is ironed out
   cookie = xcb_bell_checked(_ecore_xcb_conn, percent);
   err = xcb_request_check(_ecore_xcb_conn, cookie);
   if (err) 
     {
        _ecore_xcb_error_handle(err);
        free(err);
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

EAPI void 
ecore_x_display_size_get(Ecore_X_Display *dsp, int *w, int *h) 
{
   xcb_screen_t *screen;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   /* grab the default screen */
   screen = xcb_setup_roots_iterator(xcb_get_setup(dsp)).data;
   if (w) *w = screen->width_in_pixels;
   if (h) *h = screen->height_in_pixels;
}

EAPI unsigned long 
ecore_x_display_black_pixel_get(Ecore_X_Display *dsp) 
{
   xcb_screen_t *screen;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   /* grab the default screen */
   screen = xcb_setup_roots_iterator(xcb_get_setup(dsp)).data;
   return screen->black_pixel;
}

EAPI unsigned long 
ecore_x_display_white_pixel_get(Ecore_X_Display *dsp) 
{
   xcb_screen_t *screen;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   /* grab the default screen */
   screen = xcb_setup_roots_iterator(xcb_get_setup(dsp)).data;
   return screen->white_pixel;
}

EAPI void 
ecore_x_pointer_last_xy_get(int *x, int *y) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (x) *x = _ecore_xcb_event_last_root_x;
   if (y) *y = _ecore_xcb_event_last_root_y;
}

EAPI void 
ecore_x_focus_reset(void) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   xcb_set_input_focus(_ecore_xcb_conn, XCB_INPUT_FOCUS_POINTER_ROOT, 
                       ((xcb_screen_t *)_ecore_xcb_screen)->root, 
                       XCB_CURRENT_TIME);
}

EAPI void 
ecore_x_events_allow_all(void) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   xcb_allow_events(_ecore_xcb_conn, XCB_ALLOW_ASYNC_BOTH, XCB_CURRENT_TIME);
}

/**
 * Kill a specific client
 *
 * You can kill a specific client owning window @p win
 *
 * @param win Window of the client to be killed
 */
EAPI void 
ecore_x_kill(Ecore_X_Window win) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   xcb_kill_client(_ecore_xcb_conn, win);
}

/**
 * Kill all clients with subwindows under a given window.
 *
 * You can kill all clients connected to the X server by using
 * @ref ecore_x_window_root_list to get a list of root windows, and
 * then passing each root window to this function.
 *
 * @param root The window whose children will be killed.
 */
EAPI void 
ecore_x_killall(Ecore_X_Window root) 
{
   int screens = 0, i = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   ecore_x_grab();

   screens = xcb_setup_roots_iterator(xcb_get_setup(_ecore_xcb_conn)).rem;

   /* Traverse window tree starting from root, and drag each
    * before the firing squad */
   for (i = 0; i < screens; ++i)
     {
        xcb_query_tree_cookie_t cookie;
        xcb_query_tree_reply_t *reply;

        cookie = xcb_query_tree_unchecked(_ecore_xcb_conn, root);
        reply = xcb_query_tree_reply(_ecore_xcb_conn, cookie, NULL);
        if (reply)
          {
             xcb_window_t *wins = NULL;
             int tree_c_len, j = 0;

             wins = xcb_query_tree_children(reply);
             tree_c_len = xcb_query_tree_children_length(reply);
             for (j = 0; j < tree_c_len; j++)
               xcb_kill_client(_ecore_xcb_conn, wins[j]);
             free(reply);
          }
     }

   ecore_x_ungrab();
   ecore_x_sync(); // needed
}

/**
 * Return the screen DPI
 *
 * This is a simplistic call to get DPI. It does not account for differing
 * DPI in the x amd y axes nor does it accoutn for multihead or xinerama and
 * xrander where different parts of the screen may have differen DPI etc.
 *
 * @return the general screen DPI (dots/pixels per inch).
 */
EAPI int 
ecore_x_dpi_get(void) 
{
   uint16_t mw = 0, w = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   mw = ((xcb_screen_t *)_ecore_xcb_screen)->width_in_millimeters;
   if (mw <= 0) return 75;
   w = ((xcb_screen_t *)_ecore_xcb_screen)->width_in_pixels;
   return ((((w * 254) / mw) + 5) / 10);
}

/**
 * @defgroup Ecore_X_Display_Attr_Group X Display Attributes
 *
 * Functions that set and retrieve X display attributes.
 */

/**
 * Retrieves the Ecore_X_Display handle used for the current X connection.
 * @return  The current X display.
 * @ingroup Ecore_X_Display_Attr_Group
 */
EAPI Ecore_X_Display *
ecore_x_display_get(void) 
{
   char *gl = NULL;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   /* if we have the 'dont use xlib' env var, then we are not using 
    * XLib and thus cannot return a real XDisplay.
    * 
    * NB: This may break EFL in some places and needs lots of testing !!! */
   if ((gl = getenv("ECORE_X_NO_XLIB")))
     return (Ecore_X_Display *)_ecore_xcb_conn;
   else /* we can safely return an XDisplay var */
     return (Ecore_X_Display *)_ecore_xcb_display;
}

/**
 * Retrieves the X display file descriptor.
 * @return  The current X display file descriptor.
 * @ingroup Ecore_X_Display_Attr_Group
 */
EAPI int 
ecore_x_fd_get(void) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   return xcb_get_file_descriptor(_ecore_xcb_conn);
}

EAPI void 
ecore_x_passive_grab_replay_func_set(Eina_Bool (*func) (void *data, int type, void *event), void *data) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   _ecore_xcb_window_grab_replay_func = func;
   _ecore_xcb_window_grab_replay_data = data;
}

/**
 * Retrieves the size of an Ecore_X_Screen.
 * @param screen the handle to the screen to query.
 * @param w where to return the width. May be NULL. Returns 0 on errors.
 * @param h where to return the height. May be NULL. Returns 0 on errors.
 * @ingroup Ecore_X_Display_Attr_Group
 * @see ecore_x_default_screen_get()
 *
 * @since 1.1
 */
EAPI void 
ecore_x_screen_size_get(const Ecore_X_Screen *screen, int *w, int *h) 
{
   xcb_screen_t *s;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (w) *w = 0;
   if (h) *h = 0;
   if (!(s = (xcb_screen_t *)screen)) return;
   if (w) *w = s->width_in_pixels;
   if (h) *h = s->height_in_pixels;
}

/**
 * Retrieves the count of screens.
 * 
 * @return  The count of screens.
 * @ingroup Ecore_X_Display_Attr_Group
 *
 * @since 1.1
 */
EAPI int 
ecore_x_screen_count_get(void) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   return xcb_setup_roots_length(xcb_get_setup(_ecore_xcb_conn));
}

/**
 * Retrieves the index number of the given screen.
 * 
 * @return  The index number of the screen.
 * @ingroup Ecore_X_Display_Attr_Group
 *
 * @since 1.1
 */
EAPI int 
ecore_x_screen_index_get(const Ecore_X_Screen *screen) 
{
   xcb_screen_iterator_t iter;

   iter = 
     xcb_setup_roots_iterator(xcb_get_setup((xcb_connection_t *)_ecore_xcb_conn));
   for (; iter.rem; xcb_screen_next(&iter)) 
     {
        if (iter.data == (xcb_screen_t *)screen) 
          return iter.index;
     }

   return 0;
}

/**
 * Retrieves the screen based on index number.
 * 
 * @return  The Ecore_X_Screen at this index.
 * @ingroup Ecore_X_Display_Attr_Group
 *
 * @since 1.1
 */
EAPI Ecore_X_Screen *
ecore_x_screen_get(int index) 
{
   xcb_screen_iterator_t iter;

   iter = 
     xcb_setup_roots_iterator(xcb_get_setup((xcb_connection_t *)_ecore_xcb_conn));
   for (; iter.rem; xcb_screen_next(&iter)) 
     {
        if (iter.index == index) 
          return iter.data;
     }

   return NULL;
}

EAPI unsigned int 
ecore_x_visual_id_get(Ecore_X_Visual visual)
{
   return ((xcb_visualtype_t *)visual)->visual_id;
}

/**
 * Retrieve the default Visual.
 *
 * @param disp  The Display to get the Default Visual from
 * @param screen The Screen.
 *
 * @return The default visual.
 * @since 1.1.0
 */
EAPI Ecore_X_Visual 
ecore_x_default_visual_get(Ecore_X_Display *disp __UNUSED__, Ecore_X_Screen *screen)
{
   xcb_screen_t *s;
   xcb_depth_iterator_t diter;
   xcb_visualtype_iterator_t viter;

   s = (xcb_screen_t *)screen;
   diter = xcb_screen_allowed_depths_iterator(s);
   for (; diter.rem; xcb_depth_next(&diter)) 
     {
        viter = xcb_depth_visuals_iterator(diter.data);
        for (; viter.rem; xcb_visualtype_next(&viter)) 
          {
             if (viter.data->visual_id == s->root_visual)
               return viter.data;
          }
     }
   return 0;
}

/**
 * Retrieve the default Colormap.
 *
 * @param disp  The Display to get the Default Colormap from
 * @param screen The Screen.
 *
 * @return The default colormap.
 * @since 1.1.0
 */
EAPI Ecore_X_Colormap 
ecore_x_default_colormap_get(Ecore_X_Display *disp __UNUSED__, Ecore_X_Screen *screen)
{
   xcb_screen_t *s;

   s = (xcb_screen_t *)screen;
   return s->default_colormap;
}

/**
 * Retrieve the default depth.
 *
 * @param disp  The Display to get the Default Depth from
 * @param screen The Screen.
 *
 * @return The default depth.
 * @since 1.1.0
 */
EAPI int 
ecore_x_default_depth_get(Ecore_X_Display *disp __UNUSED__, Ecore_X_Screen *screen)
{
   xcb_screen_t *s;

   s = (xcb_screen_t *)screen;
   return s->root_depth;
}

/**
 * Sets the timeout for a double and triple clicks to be flagged.
 *
 * This sets the time between clicks before the double_click flag is
 * set in a button down event. If 3 clicks occur within double this
 * time, the triple_click flag is also set.
 *
 * @param   t The time in seconds
 * @ingroup Ecore_X_Display_Attr_Group
 */
EAPI void 
ecore_x_double_click_time_set(double t) 
{
   if (t < 0.0) t = 0.0;
   _ecore_xcb_double_click_time = t;
}

/**
 * Retrieves the double and triple click flag timeout.
 *
 * See @ref ecore_x_double_click_time_set for more information.
 *
 * @return  The timeout for double clicks in seconds.
 * @ingroup Ecore_X_Display_Attr_Group
 */
EAPI double 
ecore_x_double_click_time_get(void) 
{
   return _ecore_xcb_double_click_time;
}

/* local function prototypes */
static int 
_ecore_xcb_shutdown(Eina_Bool close_display) 
{
   if (--_ecore_xcb_init_count != 0) return _ecore_xcb_init_count;
   if (!_ecore_xcb_conn) return _ecore_xcb_init_count;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   /* shutdown dnd */
   _ecore_xcb_dnd_shutdown();

   /* shutdown selection */
   _ecore_xcb_selection_shutdown();

   /* shutdown netwm */
   ecore_x_netwm_shutdown();

   if (_ecore_xcb_fd_handler)
     ecore_main_fd_handler_del(_ecore_xcb_fd_handler);

   /* shutdown keymap */
   _ecore_xcb_keymap_shutdown();

   /* shutdown events */
   _ecore_xcb_events_shutdown();

   /* disconnect from display server */
   if (close_display)
     xcb_disconnect(_ecore_xcb_conn);
   else 
     close(xcb_get_file_descriptor(_ecore_xcb_conn));

   /* shutdown ecore_event */
   ecore_event_shutdown();

   /* unregister log domain */
   eina_log_domain_unregister(_ecore_xcb_log_dom);
   _ecore_xcb_log_dom = -1;

   /* shutdown ecore */
   ecore_shutdown();

   /* shutdown eina */
//   eina_shutdown();

   return _ecore_xcb_init_count;
}

static Eina_Bool 
_ecore_xcb_fd_handle(void *data, Ecore_Fd_Handler *hdlr __UNUSED__) 
{
   xcb_connection_t *conn;
   xcb_generic_event_t *ev = NULL;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   conn = (xcb_connection_t *)data;

   if (_ecore_xcb_event_buffered) 
     {
        _ecore_xcb_events_handle(_ecore_xcb_event_buffered);
        _ecore_xcb_event_buffered = NULL;
     }

   while ((ev = xcb_poll_for_event(conn))) 
     {
        // Event type 0 is an error if we don't use the xcb_*_checked funcs
        if (xcb_connection_has_error(_ecore_xcb_conn)) 
          {
             xcb_generic_error_t *err;

             err = (xcb_generic_error_t *)ev;
             _ecore_xcb_io_error_handle(err);
          }
        else
          _ecore_xcb_events_handle(ev);

        free(ev);
     }

   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool 
_ecore_xcb_fd_handle_buff(void *data, Ecore_Fd_Handler *hdlr __UNUSED__) 
{
   xcb_connection_t *conn;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   conn = (xcb_connection_t *)data;
   _ecore_xcb_event_buffered = xcb_poll_for_event(conn);
   if (_ecore_xcb_event_buffered) return ECORE_CALLBACK_RENEW;

   return ECORE_CALLBACK_CANCEL;
}
