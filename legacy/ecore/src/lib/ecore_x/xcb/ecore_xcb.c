#include <string.h>

#include <X11/keysym.h>

#include <Ecore.h>
#include <Ecore_Input.h>

#include "ecore_xcb_private.h"
#include "Ecore_X_Atoms.h"

static Eina_Bool      _ecore_xcb_fd_handler(void *data, Ecore_Fd_Handler *fd_handler);
static Eina_Bool      _ecore_xcb_fd_handler_buf(void *data, Ecore_Fd_Handler *fd_handler);
static int            _ecore_xcb_key_mask_get(xcb_keysym_t sym);
static int            _ecore_xcb_event_modifier(unsigned int state);

static void *         _ecore_xcb_event_filter_start(void *data);
static Eina_Bool      _ecore_xcb_event_filter_filter(void *data, void *loop_data,int type, void *event);
static void           _ecore_xcb_event_filter_end(void *data, void *loop_data);

static Ecore_Fd_Handler *_ecore_xcb_fd_handler_handle = NULL;
static Ecore_Event_Filter *_ecore_xcb_filter_handler = NULL;

static const int XCB_EVENT_ANY = 0; /* 0 can be used as there are no event types
                                     * with index 0 and 1 as they are used for
                                     * errors
                                     */

#ifdef ECORE_XCB_DAMAGE
static int _ecore_xcb_event_damage_id = 0;
#endif /* ECORE_XCB_DAMAGE */
#ifdef ECORE_XCB_RANDR
static int _ecore_xcb_event_randr_id = 0;
#endif /* ECORE_XCB_RANDR */
#ifdef ECORE_XCB_SCREENSAVER
static int _ecore_xcb_event_screensaver_id = 0;
#endif /* ECORE_XCB_SCREENSAVER */
#ifdef ECORE_XCB_SHAPE
static int _ecore_xcb_event_shape_id = 0;
#endif /* ECORE_XCB_SHAPE */
#ifdef ECORE_XCB_SYNC
static int _ecore_xcb_event_sync_id = 0;
#endif /* ECORE_XCB_SYNC */
#ifdef ECORE_XCB_FIXES
static int _ecore_xcb_event_fixes_selection_id = 0;
#endif /* ECORE_XCB_FIXES */

static int _ecore_xcb_event_handlers_num = 0;
static void (**_ecore_xcb_event_handlers) (xcb_generic_event_t * event) = NULL;
static xcb_generic_event_t *_ecore_xcb_event_buffered = NULL;

static int _ecore_xcb_init_count = 0;
static int _ecore_xcb_grab_count = 0;
int _ecore_x11xcb_log_dom = -1;

Ecore_X_Connection *_ecore_xcb_conn = NULL;
Ecore_X_Screen *_ecore_xcb_screen = NULL;
double _ecore_xcb_double_click_time = 0.25;
Ecore_X_Time _ecore_xcb_event_last_time = XCB_NONE;
Ecore_X_Window _ecore_xcb_event_last_window = XCB_NONE;
int16_t _ecore_xcb_event_last_root_x = 0;
int16_t _ecore_xcb_event_last_root_y = 0;
int _ecore_xcb_xcursor = 0;

Ecore_X_Window _ecore_xcb_private_window = 0;

/* FIXME - These are duplicates after making ecore atoms public */

Ecore_X_Atom _ecore_xcb_atoms_wm_protocols[ECORE_X_WM_PROTOCOL_NUM];

EAPI int ECORE_X_EVENT_ANY = 0;
EAPI int ECORE_X_EVENT_MOUSE_IN = 0;
EAPI int ECORE_X_EVENT_MOUSE_OUT = 0;
EAPI int ECORE_X_EVENT_WINDOW_FOCUS_IN = 0;
EAPI int ECORE_X_EVENT_WINDOW_FOCUS_OUT = 0;
EAPI int ECORE_X_EVENT_WINDOW_KEYMAP = 0;
EAPI int ECORE_X_EVENT_WINDOW_DAMAGE = 0;
EAPI int ECORE_X_EVENT_WINDOW_VISIBILITY_CHANGE = 0;
EAPI int ECORE_X_EVENT_WINDOW_CREATE = 0;
EAPI int ECORE_X_EVENT_WINDOW_DESTROY = 0;
EAPI int ECORE_X_EVENT_WINDOW_HIDE = 0;
EAPI int ECORE_X_EVENT_WINDOW_SHOW = 0;
EAPI int ECORE_X_EVENT_WINDOW_SHOW_REQUEST = 0;
EAPI int ECORE_X_EVENT_WINDOW_REPARENT = 0;
EAPI int ECORE_X_EVENT_WINDOW_CONFIGURE = 0;
EAPI int ECORE_X_EVENT_WINDOW_CONFIGURE_REQUEST = 0;
EAPI int ECORE_X_EVENT_WINDOW_GRAVITY = 0;
EAPI int ECORE_X_EVENT_WINDOW_RESIZE_REQUEST = 0;
EAPI int ECORE_X_EVENT_WINDOW_STACK = 0;
EAPI int ECORE_X_EVENT_WINDOW_STACK_REQUEST = 0;
EAPI int ECORE_X_EVENT_WINDOW_PROPERTY = 0;
EAPI int ECORE_X_EVENT_WINDOW_COLORMAP = 0;
EAPI int ECORE_X_EVENT_WINDOW_MAPPING = 0;
EAPI int ECORE_X_EVENT_MAPPING_CHANGE = 0;
EAPI int ECORE_X_EVENT_SELECTION_CLEAR = 0;
EAPI int ECORE_X_EVENT_SELECTION_REQUEST = 0;
EAPI int ECORE_X_EVENT_SELECTION_NOTIFY = 0;
EAPI int ECORE_X_EVENT_CLIENT_MESSAGE = 0;
EAPI int ECORE_X_EVENT_WINDOW_SHAPE = 0;
EAPI int ECORE_X_EVENT_SCREENSAVER_NOTIFY = 0;
EAPI int ECORE_X_EVENT_SYNC_COUNTER = 0;
EAPI int ECORE_X_EVENT_SYNC_ALARM = 0;
EAPI int ECORE_X_EVENT_SCREEN_CHANGE = 0;
EAPI int ECORE_X_EVENT_DAMAGE_NOTIFY = 0;

EAPI int ECORE_X_EVENT_WINDOW_DELETE_REQUEST = 0;

EAPI int ECORE_X_EVENT_WINDOW_MOVE_RESIZE_REQUEST = 0;
EAPI int ECORE_X_EVENT_WINDOW_STATE_REQUEST = 0;
EAPI int ECORE_X_EVENT_FRAME_EXTENTS_REQUEST = 0;
EAPI int ECORE_X_EVENT_PING = 0;
EAPI int ECORE_X_EVENT_DESKTOP_CHANGE = 0;

EAPI int ECORE_X_EVENT_STARTUP_SEQUENCE_NEW = 0;
EAPI int ECORE_X_EVENT_STARTUP_SEQUENCE_CHANGE = 0;
EAPI int ECORE_X_EVENT_STARTUP_SEQUENCE_REMOVE = 0;

EAPI int ECORE_X_MODIFIER_SHIFT = 0;
EAPI int ECORE_X_MODIFIER_CTRL = 0;
EAPI int ECORE_X_MODIFIER_ALT = 0;
EAPI int ECORE_X_MODIFIER_WIN = 0;

EAPI int ECORE_X_LOCK_SCROLL = 0;
EAPI int ECORE_X_LOCK_NUM = 0;
EAPI int ECORE_X_LOCK_CAPS = 0;

/**
 * @defgroup Ecore_Xcb_Init_Group X Library Init and Shutdown Functions
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
 * @ingroup Ecore_Xcb_Init_Group
 */
EAPI int
ecore_x_init(const char *name)
{
   xcb_screen_iterator_t iter;
   int screen;
   uint32_t max_request_length;
   const xcb_query_extension_reply_t *reply_big_requests;
#ifdef ECORE_XCB_DAMAGE
   const xcb_query_extension_reply_t *reply_damage;
#endif /* ECORE_XCB_DAMAGE */
#ifdef ECORE_XCB_COMPOSITE
   const xcb_query_extension_reply_t *reply_composite;
#endif /* ECORE_XCB_COMPOSITE */
#ifdef ECORE_XCB_DPMS
   const xcb_query_extension_reply_t *reply_dpms;
#endif /* ECORE_XCB_DPMS */
#ifdef ECORE_XCB_RANDR
   const xcb_query_extension_reply_t *reply_randr;
#endif /* ECORE_XCB_RANDR */
#ifdef ECORE_XCB_SCREENSAVER
   const xcb_query_extension_reply_t *reply_screensaver;
#endif /* ECORE_XCB_SCREENSAVER */
#ifdef ECORE_XCB_SHAPE
   const xcb_query_extension_reply_t *reply_shape;
#endif /* ECORE_XCB_SHAPE */
#ifdef ECORE_XCB_SYNC
   xcb_sync_initialize_cookie_t cookie_sync_init;
   xcb_sync_initialize_reply_t *reply_sync_init;
   const xcb_query_extension_reply_t *reply_sync;
#endif /* ECORE_XCB_SYNC */
#ifdef ECORE_XCB_FIXES
   const xcb_query_extension_reply_t *reply_xfixes;
#endif /* ECORE_XCB_FIXES */
#ifdef ECORE_XCB_XINERAMA
   const xcb_query_extension_reply_t *reply_xinerama;
#endif /* ECORE_XCB_XINERAMA */
#ifdef ECORE_XCB_XPRINT
   const xcb_query_extension_reply_t *reply_xprint;
#endif /* ECORE_XCB_XPRINT */

   xcb_intern_atom_cookie_t atom_cookies[ECORE_X_ATOMS_COUNT];

   if (++_ecore_xcb_init_count != 1)
      return _ecore_xcb_init_count;

   /* We init some components (not related to XCB) */
   if (!eina_init())
      return --_ecore_xcb_init_count;

   _ecore_x11xcb_log_dom = eina_log_domain_register
     ("ecore_x", ECORE_XLIB_XCB_DEFAULT_LOG_COLOR);
   if (_ecore_x11xcb_log_dom < 0)
     {
        EINA_LOG_ERR("Impossible to create a log domain the Ecore XCB module.");
        goto shutdown_eina;
     }

   if (!ecore_init())
      goto shutdown_eina;
   if (!ecore_event_init())
      goto shutdown_ecore;

   _ecore_xcb_conn = xcb_connect(name, &screen);
   if (xcb_connection_has_error(_ecore_xcb_conn))
      goto shutdown_ecore_event;

   /* FIXME: no error code right now */
   /* _ecore_xcb_error_handler_init(); */

   /********************/
   /* First round trip */
   /********************/

   /*
    * Non blocking stuff:
    *
    * 1. We request the atoms
    * 2. We Prefetch the extension data
    *
    */

   /* We request the atoms (non blocking) */
   _ecore_x_atom_init(atom_cookies);

   /* We prefetch all the extension data (non blocking) */

   xcb_prefetch_extension_data(_ecore_xcb_conn, &xcb_big_requests_id);

#ifdef ECORE_XCB_DAMAGE
   xcb_prefetch_extension_data(_ecore_xcb_conn, &xcb_damage_id);
#endif /* ECORE_XCB_DAMAGE */

#ifdef ECORE_XCB_COMPOSITE
   xcb_prefetch_extension_data(_ecore_xcb_conn, &xcb_composite_id);
#endif /* ECORE_XCB_COMPOSITE */

#ifdef ECORE_XCB_DPMS
   xcb_prefetch_extension_data(_ecore_xcb_conn, &xcb_dpms_id);
#endif /* ECORE_XCB_DPMS */

#ifdef ECORE_XCB_RANDR
   xcb_prefetch_extension_data(_ecore_xcb_conn, &xcb_randr_id);
#endif /* ECORE_XCB_RANDR */

#ifdef ECORE_XCB_SCREENSAVER
   xcb_prefetch_extension_data(_ecore_xcb_conn, &xcb_screensaver_id);
#endif /* ECORE_XCB_SCREENSAVER */

#ifdef ECORE_XCB_SHAPE
   xcb_prefetch_extension_data(_ecore_xcb_conn, &xcb_shape_id);
#endif /* ECORE_XCB_SHAPE */

#ifdef ECORE_XCB_SYNC
   xcb_prefetch_extension_data(_ecore_xcb_conn, &xcb_sync_id);
   cookie_sync_init = xcb_sync_initialize_unchecked(_ecore_xcb_conn,
                                                    XCB_SYNC_MAJOR_VERSION,
                                                    XCB_SYNC_MINOR_VERSION);
#endif /* ECORE_XCB_SYNC */

#ifdef ECORE_XCB_FIXES
   xcb_prefetch_extension_data(_ecore_xcb_conn, &xcb_xfixes_id);
#endif /* ECORE_XCB_FIXES */

#ifdef ECORE_XCB_XINERAMA
   xcb_prefetch_extension_data(_ecore_xcb_conn, &xcb_xinerama_id);
#endif /* ECORE_XCB_XINERAMA */

#ifdef ECORE_XCB_XPRINT
   xcb_prefetch_extension_data(_ecore_xcb_conn, &xcb_x_print_id);
#endif /* ECORE_XCB_XPRINT */

   _ecore_x_reply_init();
   _ecore_x_dnd_init();
   ecore_x_netwm_init();
   _ecore_x_selection_init();

   /* There is no LASTEvent constant in XCB */
   /* LASTevent is equal to 35 */
   _ecore_xcb_event_handlers_num = 35;

   /* We get the default screen */
   iter = xcb_setup_roots_iterator(xcb_get_setup(_ecore_xcb_conn));
   for (; iter.rem; --screen, xcb_screen_next (&iter))
      if (screen == 0)
        {
           _ecore_xcb_screen = iter.data;
           break;
        }

   /*
    * Blocking stuff:
    *
    * 1. We get the atoms
    * 2. We ask for the extension data
    *
    */

   /* We get the atoms (blocking) */
   _ecore_x_atom_init_finalize(atom_cookies);

   /* We then ask for the extension data (blocking) */
   reply_big_requests = xcb_get_extension_data(_ecore_xcb_conn, &xcb_big_requests_id);

#ifdef ECORE_XCB_DAMAGE
   reply_damage = xcb_get_extension_data(_ecore_xcb_conn, &xcb_damage_id);
   if (reply_damage)
      _ecore_xcb_event_damage_id = reply_damage->first_event + XCB_DAMAGE_NOTIFY;

   if (_ecore_xcb_event_damage_id >= _ecore_xcb_event_handlers_num)
      _ecore_xcb_event_handlers_num = _ecore_xcb_event_damage_id + 1;

#endif /* ECORE_XCB_DAMAGE */

#ifdef ECORE_XCB_COMPOSITE
   reply_composite = xcb_get_extension_data(_ecore_xcb_conn, &xcb_composite_id);
#endif /* ECORE_XCB_COMPOSITE */

#ifdef ECORE_XCB_DPMS
   reply_dpms = xcb_get_extension_data(_ecore_xcb_conn, &xcb_dpms_id);
#endif /* ECORE_XCB_DPMS */

#ifdef ECORE_XCB_RANDR
   reply_randr = xcb_get_extension_data(_ecore_xcb_conn, &xcb_randr_id);
   if (reply_randr)
      _ecore_xcb_event_randr_id = reply_randr->first_event + XCB_RANDR_SCREEN_CHANGE_NOTIFY;

   if (_ecore_xcb_event_randr_id >= _ecore_xcb_event_handlers_num)
      _ecore_xcb_event_handlers_num = _ecore_xcb_event_randr_id + 1;

#endif /* ECORE_XCB_RANDR */

#ifdef ECORE_XCB_SCREENSAVER
   reply_screensaver = xcb_get_extension_data(_ecore_xcb_conn, &xcb_screensaver_id);
   if (reply_screensaver)
      _ecore_xcb_event_screensaver_id = reply_screensaver->first_event + XCB_SCREENSAVER_NOTIFY;

   if (_ecore_xcb_event_screensaver_id >= _ecore_xcb_event_handlers_num)
      _ecore_xcb_event_handlers_num = _ecore_xcb_event_screensaver_id + 1;

#endif /* ECORE_XCB_SCREENSAVER */

#ifdef ECORE_XCB_SHAPE
   reply_shape = xcb_get_extension_data(_ecore_xcb_conn, &xcb_shape_id);
   if (reply_shape)
      _ecore_xcb_event_shape_id = reply_shape->first_event + XCB_SHAPE_NOTIFY;

   if (_ecore_xcb_event_shape_id >= _ecore_xcb_event_handlers_num)
      _ecore_xcb_event_handlers_num = _ecore_xcb_event_shape_id + 1;

#endif /* ECORE_XCB_SHAPE */

#ifdef ECORE_XCB_SYNC
   reply_sync = xcb_get_extension_data(_ecore_xcb_conn, &xcb_sync_id);
   if (reply_sync)
     {
        _ecore_xcb_event_sync_id = reply_sync->first_event;
        reply_sync_init = xcb_sync_initialize_reply(_ecore_xcb_conn,
                                                    cookie_sync_init, NULL);
        if (!reply_sync_init)
           _ecore_xcb_event_sync_id = 0;
        else
           free(reply_sync_init);
     }

   if (_ecore_xcb_event_sync_id + XCB_SYNC_ALARM_NOTIFY >= _ecore_xcb_event_handlers_num)
      _ecore_xcb_event_handlers_num = _ecore_xcb_event_sync_id + XCB_SYNC_ALARM_NOTIFY + 1;

#endif /* ECORE_XCB_SYNC */

#ifdef ECORE_XCB_FIXES
   reply_xfixes = xcb_get_extension_data(_ecore_xcb_conn, &xcb_xfixes_id);
   if (reply_xfixes)
      _ecore_xcb_event_fixes_selection_id = reply_xfixes->first_event + XCB_XFIXES_SELECTION_NOTIFY;

   if (_ecore_xcb_event_fixes_selection_id >= _ecore_xcb_event_handlers_num)
      _ecore_xcb_event_handlers_num = _ecore_xcb_event_fixes_selection_id + 1;

#endif /* ECORE_XCB_FIXES */

#ifdef ECORE_XCB_XINERAMA
   reply_xinerama = xcb_get_extension_data(_ecore_xcb_conn, &xcb_xinerama_id);
#endif /* ECORE_XCB_XINERAMA */

#ifdef ECORE_XCB_XPRINT
   reply_xprint = xcb_get_extension_data(_ecore_xcb_conn, &xcb_x_print_id);
#endif /* ECORE_XCB_XPRINT */

   /*********************/
   /* Second round trip */
   /*********************/

   /* We ask for the QueryVersion request of the extensions */
#ifdef ECORE_XCB_DAMAGE
   _ecore_x_damage_init(reply_damage);
#endif /* ECORE_XCB_DAMAGE */
#ifdef ECORE_XCB_COMPOSITE
   _ecore_x_composite_init(reply_composite);
#endif /* ECORE_XCB_COMPOSITE */
#ifdef ECORE_XCB_DPMS
   _ecore_x_dpms_init(reply_dpms);
#endif /* ECORE_XCB_DPMS */
#ifdef ECORE_XCB_RANDR
   _ecore_x_randr_init(reply_randr);
#endif /* ECORE_XCB_RANDR */
#ifdef ECORE_XCB_SCREENSAVER
   _ecore_x_screensaver_init(reply_screensaver);
#endif /* ECORE_XCB_SCREENSAVER */
#ifdef ECORE_XCB_SHAPE
   _ecore_x_shape_init(reply_shape);
#endif /* ECORE_XCB_SHAPE */
#ifdef ECORE_XCB_SYNC
   _ecore_x_sync_init(reply_sync);
#endif /* ECORE_XCB_SYNC */
#ifdef ECORE_XCB_FIXES
   _ecore_x_xfixes_init(reply_xfixes);
#endif /* ECORE_XCB_FIXES */
#ifdef ECORE_XCB_XINERAMA
   _ecore_x_xinerama_init(reply_xinerama);
#endif /* ECORE_XCB_XINERAMA */

   /* we enable the Big Request extension if present */
   max_request_length = xcb_get_maximum_request_length(_ecore_xcb_conn);

   _ecore_xcb_event_handlers = calloc(_ecore_xcb_event_handlers_num, sizeof(void *));
   if (!_ecore_xcb_event_handlers)
      goto finalize_extensions;

#ifdef ECORE_XCB_CURSOR
   _ecore_xcb_xcursor = XcursorSupportsARGB(_ecore_xcb_conn);
#endif /* ECORE_XCB_CURSOR */

   _ecore_xcb_event_handlers[XCB_EVENT_ANY] = _ecore_x_event_handle_any_event;
   _ecore_xcb_event_handlers[XCB_KEY_PRESS] = _ecore_x_event_handle_key_press;
   _ecore_xcb_event_handlers[XCB_KEY_RELEASE] = _ecore_x_event_handle_key_release;
   _ecore_xcb_event_handlers[XCB_BUTTON_PRESS] = _ecore_x_event_handle_button_press;
   _ecore_xcb_event_handlers[XCB_BUTTON_RELEASE] = _ecore_x_event_handle_button_release;
   _ecore_xcb_event_handlers[XCB_MOTION_NOTIFY] = _ecore_x_event_handle_motion_notify;
   _ecore_xcb_event_handlers[XCB_ENTER_NOTIFY] = _ecore_x_event_handle_enter_notify;
   _ecore_xcb_event_handlers[XCB_LEAVE_NOTIFY] = _ecore_x_event_handle_leave_notify;
   _ecore_xcb_event_handlers[XCB_FOCUS_IN] = _ecore_x_event_handle_focus_in;
   _ecore_xcb_event_handlers[XCB_FOCUS_OUT] = _ecore_x_event_handle_focus_out;
   _ecore_xcb_event_handlers[XCB_KEYMAP_NOTIFY] = _ecore_x_event_handle_keymap_notify;
   _ecore_xcb_event_handlers[XCB_EXPOSE] = _ecore_x_event_handle_expose;
   _ecore_xcb_event_handlers[XCB_GRAPHICS_EXPOSURE] = _ecore_x_event_handle_graphics_expose;
   _ecore_xcb_event_handlers[XCB_VISIBILITY_NOTIFY] = _ecore_x_event_handle_visibility_notify;
   _ecore_xcb_event_handlers[XCB_CREATE_NOTIFY] = _ecore_x_event_handle_create_notify;
   _ecore_xcb_event_handlers[XCB_DESTROY_NOTIFY] = _ecore_x_event_handle_destroy_notify;
   _ecore_xcb_event_handlers[XCB_UNMAP_NOTIFY] = _ecore_x_event_handle_unmap_notify;
   _ecore_xcb_event_handlers[XCB_MAP_NOTIFY] = _ecore_x_event_handle_map_notify;
   _ecore_xcb_event_handlers[XCB_MAP_REQUEST] = _ecore_x_event_handle_map_request;
   _ecore_xcb_event_handlers[XCB_REPARENT_NOTIFY] = _ecore_x_event_handle_reparent_notify;
   _ecore_xcb_event_handlers[XCB_CONFIGURE_NOTIFY] = _ecore_x_event_handle_configure_notify;
   _ecore_xcb_event_handlers[XCB_CONFIGURE_REQUEST] = _ecore_x_event_handle_configure_request;
   _ecore_xcb_event_handlers[XCB_GRAVITY_NOTIFY] = _ecore_x_event_handle_gravity_notify;
   _ecore_xcb_event_handlers[XCB_RESIZE_REQUEST] = _ecore_x_event_handle_resize_request;
   _ecore_xcb_event_handlers[XCB_CIRCULATE_NOTIFY] = _ecore_x_event_handle_circulate_notify;
   _ecore_xcb_event_handlers[XCB_CIRCULATE_REQUEST] = _ecore_x_event_handle_circulate_request;
   _ecore_xcb_event_handlers[XCB_PROPERTY_NOTIFY] = _ecore_x_event_handle_property_notify;
   _ecore_xcb_event_handlers[XCB_SELECTION_CLEAR] = _ecore_x_event_handle_selection_clear;
   _ecore_xcb_event_handlers[XCB_SELECTION_REQUEST] = _ecore_x_event_handle_selection_request;
   _ecore_xcb_event_handlers[XCB_SELECTION_NOTIFY] = _ecore_x_event_handle_selection_notify;
   _ecore_xcb_event_handlers[XCB_COLORMAP_NOTIFY] = _ecore_x_event_handle_colormap_notify;
   _ecore_xcb_event_handlers[XCB_CLIENT_MESSAGE] = _ecore_x_event_handle_client_message;
   _ecore_xcb_event_handlers[XCB_MAPPING_NOTIFY] = _ecore_x_event_handle_mapping_notify;
#ifdef ECORE_XCB_DAMAGE
   if (_ecore_xcb_event_damage_id)
      _ecore_xcb_event_handlers[_ecore_xcb_event_damage_id] = _ecore_x_event_handle_damage_notify;

#endif /* ECORE_XCB_DAMAGE */
#ifdef ECORE_XCB_RANDR
   if (_ecore_xcb_event_randr_id)
      _ecore_xcb_event_handlers[_ecore_xcb_event_randr_id] = _ecore_x_event_handle_randr_change;

#endif /* ECORE_XCB_RANDR */
#ifdef ECORE_XCB_SCREENSAVER
   if (_ecore_xcb_event_screensaver_id)
      _ecore_xcb_event_handlers[_ecore_xcb_event_screensaver_id] = _ecore_x_event_handle_screensaver_notify;

#endif /* ECORE_XCB_SCREENSAVER */
#ifdef ECORE_XCB_SHAPE
   if (_ecore_xcb_event_shape_id)
      _ecore_xcb_event_handlers[_ecore_xcb_event_shape_id] = _ecore_x_event_handle_shape_change;

#endif /* ECORE_XCB_SHAPE */
#ifdef ECORE_XCB_SYNC
   if (_ecore_xcb_event_sync_id)
     {
        _ecore_xcb_event_handlers[_ecore_xcb_event_sync_id + XCB_SYNC_COUNTER_NOTIFY] =
           _ecore_x_event_handle_sync_counter;
        _ecore_xcb_event_handlers[_ecore_xcb_event_sync_id + XCB_SYNC_ALARM_NOTIFY] =
           _ecore_x_event_handle_sync_alarm;
     }

#endif /* ECORE_XCB_SYNC */
#ifdef ECORE_XCB_FIXES
   if (_ecore_xcb_event_fixes_selection_id)
      _ecore_xcb_event_handlers[_ecore_xcb_event_fixes_selection_id] = _ecore_x_event_handle_fixes_selection_notify;

#endif /* ECORE_XCB_FIXES */

   if (!ECORE_X_EVENT_ANY)
     {
        ECORE_X_EVENT_ANY = ecore_event_type_new();
        ECORE_X_EVENT_MOUSE_IN = ecore_event_type_new();
        ECORE_X_EVENT_MOUSE_OUT = ecore_event_type_new();
        ECORE_X_EVENT_WINDOW_FOCUS_IN = ecore_event_type_new();
        ECORE_X_EVENT_WINDOW_FOCUS_OUT = ecore_event_type_new();
        ECORE_X_EVENT_WINDOW_KEYMAP = ecore_event_type_new();
        ECORE_X_EVENT_WINDOW_DAMAGE = ecore_event_type_new();
        ECORE_X_EVENT_WINDOW_VISIBILITY_CHANGE = ecore_event_type_new();
        ECORE_X_EVENT_WINDOW_CREATE = ecore_event_type_new();
        ECORE_X_EVENT_WINDOW_DESTROY = ecore_event_type_new();
        ECORE_X_EVENT_WINDOW_HIDE = ecore_event_type_new();
        ECORE_X_EVENT_WINDOW_SHOW = ecore_event_type_new();
        ECORE_X_EVENT_WINDOW_SHOW_REQUEST = ecore_event_type_new();
        ECORE_X_EVENT_WINDOW_REPARENT = ecore_event_type_new();
        ECORE_X_EVENT_WINDOW_CONFIGURE = ecore_event_type_new();
        ECORE_X_EVENT_WINDOW_CONFIGURE_REQUEST = ecore_event_type_new();
        ECORE_X_EVENT_WINDOW_GRAVITY = ecore_event_type_new();
        ECORE_X_EVENT_WINDOW_RESIZE_REQUEST = ecore_event_type_new();
        ECORE_X_EVENT_WINDOW_STACK = ecore_event_type_new();
        ECORE_X_EVENT_WINDOW_STACK_REQUEST = ecore_event_type_new();
        ECORE_X_EVENT_WINDOW_PROPERTY = ecore_event_type_new();
        ECORE_X_EVENT_WINDOW_COLORMAP = ecore_event_type_new();
        ECORE_X_EVENT_WINDOW_MAPPING = ecore_event_type_new();
        ECORE_X_EVENT_MAPPING_CHANGE = ecore_event_type_new();
        ECORE_X_EVENT_SELECTION_CLEAR = ecore_event_type_new();
        ECORE_X_EVENT_SELECTION_REQUEST = ecore_event_type_new();
        ECORE_X_EVENT_SELECTION_NOTIFY = ecore_event_type_new();
        ECORE_X_EVENT_CLIENT_MESSAGE = ecore_event_type_new();
        ECORE_X_EVENT_WINDOW_SHAPE = ecore_event_type_new();
        ECORE_X_EVENT_SCREENSAVER_NOTIFY = ecore_event_type_new();
        ECORE_X_EVENT_SYNC_COUNTER = ecore_event_type_new();
        ECORE_X_EVENT_SYNC_ALARM = ecore_event_type_new();
        ECORE_X_EVENT_SCREEN_CHANGE = ecore_event_type_new();
        ECORE_X_EVENT_DAMAGE_NOTIFY = ecore_event_type_new();

        ECORE_X_EVENT_WINDOW_DELETE_REQUEST = ecore_event_type_new();
        ECORE_X_EVENT_DESKTOP_CHANGE = ecore_event_type_new();
        ECORE_X_EVENT_WINDOW_MOVE_RESIZE_REQUEST = ecore_event_type_new();
        ECORE_X_EVENT_WINDOW_STATE_REQUEST = ecore_event_type_new();
        ECORE_X_EVENT_FRAME_EXTENTS_REQUEST = ecore_event_type_new();
        ECORE_X_EVENT_PING = ecore_event_type_new();

        ECORE_X_EVENT_STARTUP_SEQUENCE_NEW = ecore_event_type_new();
        ECORE_X_EVENT_STARTUP_SEQUENCE_CHANGE = ecore_event_type_new();
        ECORE_X_EVENT_STARTUP_SEQUENCE_REMOVE = ecore_event_type_new();
     }

   /* everything has these... unless its like a pda... :) */
   ECORE_X_MODIFIER_SHIFT = _ecore_xcb_key_mask_get(XK_Shift_L);
   ECORE_X_MODIFIER_CTRL = _ecore_xcb_key_mask_get(XK_Control_L);

   /* apple's xdarwin has no alt!!!! */
   ECORE_X_MODIFIER_ALT = _ecore_xcb_key_mask_get(XK_Alt_L);
   if (!ECORE_X_MODIFIER_ALT)
      ECORE_X_MODIFIER_ALT = _ecore_xcb_key_mask_get(XK_Meta_L);

   if (!ECORE_X_MODIFIER_ALT)
      ECORE_X_MODIFIER_ALT = _ecore_xcb_key_mask_get(XK_Super_L);

   /* the windows key... a valid modifier :) */
   ECORE_X_MODIFIER_WIN = _ecore_xcb_key_mask_get(XK_Super_L);
   if (!ECORE_X_MODIFIER_WIN)
      ECORE_X_MODIFIER_WIN = _ecore_xcb_key_mask_get(XK_Mode_switch);

   if (!ECORE_X_MODIFIER_WIN)
      ECORE_X_MODIFIER_WIN = _ecore_xcb_key_mask_get(XK_Meta_L);

   if (ECORE_X_MODIFIER_WIN == ECORE_X_MODIFIER_ALT)
      ECORE_X_MODIFIER_WIN = 0;

   if (ECORE_X_MODIFIER_ALT == ECORE_X_MODIFIER_CTRL)
      ECORE_X_MODIFIER_ALT = 0;

   ECORE_X_LOCK_SCROLL = _ecore_xcb_key_mask_get(XK_Scroll_Lock);
   ECORE_X_LOCK_NUM = _ecore_xcb_key_mask_get(XK_Num_Lock);
   ECORE_X_LOCK_CAPS = _ecore_xcb_key_mask_get(XK_Caps_Lock);

   _ecore_xcb_fd_handler_handle =
      ecore_main_fd_handler_add(xcb_get_file_descriptor(_ecore_xcb_conn),
                                ECORE_FD_READ,
                                _ecore_xcb_fd_handler, _ecore_xcb_conn,
                                _ecore_xcb_fd_handler_buf, _ecore_xcb_conn);
   if (!_ecore_xcb_fd_handler_handle)
      goto free_event_handlers;

   _ecore_xcb_filter_handler = ecore_event_filter_add(_ecore_xcb_event_filter_start, _ecore_xcb_event_filter_filter, _ecore_xcb_event_filter_end, NULL);

   /* This is just to be anal about naming conventions */

   _ecore_xcb_atoms_wm_protocols[ECORE_X_WM_PROTOCOL_DELETE_REQUEST] = ECORE_X_ATOM_WM_DELETE_WINDOW;
   _ecore_xcb_atoms_wm_protocols[ECORE_X_WM_PROTOCOL_TAKE_FOCUS] = ECORE_X_ATOM_WM_TAKE_FOCUS;
   _ecore_xcb_atoms_wm_protocols[ECORE_X_NET_WM_PROTOCOL_PING] = ECORE_X_ATOM_NET_WM_PING;
   _ecore_xcb_atoms_wm_protocols[ECORE_X_NET_WM_PROTOCOL_SYNC_REQUEST] = ECORE_X_ATOM_NET_WM_SYNC_REQUEST;

   _ecore_xcb_init_count++;

   _ecore_xcb_private_window = ecore_x_window_override_new(0, -77, -777, 123, 456);

   /* We finally get the replies of the QueryVersion request */
#ifdef ECORE_XCB_DAMAGE
   _ecore_x_damage_init_finalize();
#endif /* ECORE_XCB_DAMAGE */
#ifdef ECORE_XCB_COMPOSITE
   _ecore_x_composite_init_finalize();
#endif /* ECORE_XCB_COMPOSITE */
#ifdef ECORE_XCB_DPMS
   _ecore_x_dpms_init_finalize();
#endif /* ECORE_XCB_DPMS */
#ifdef ECORE_XCB_RANDR
   _ecore_x_randr_init_finalize();
#endif /* ECORE_XCB_RANDR */
#ifdef ECORE_XCB_SCREENSAVER
   _ecore_x_screensaver_init_finalize();
#endif /* ECORE_XCB_SCREENSAVER */
#ifdef ECORE_XCB_SHAPE
   _ecore_x_shape_init_finalize();
#endif /* ECORE_XCB_SHAPE */
#ifdef ECORE_XCB_SYNC
   _ecore_x_sync_init_finalize();
#endif /* ECORE_XCB_SYNC */
#ifdef ECORE_XCB_FIXES
   _ecore_x_xfixes_init_finalize();
#endif /* ECORE_XCB_FIXES */
#ifdef ECORE_XCB_XINERAMA
   _ecore_x_xinerama_init_finalize();
#endif /* ECORE_XCB_XINERAMA */

   return _ecore_xcb_init_count;

free_event_handlers:
   free(_ecore_xcb_event_handlers);
   _ecore_xcb_event_handlers = NULL;
finalize_extensions:
   /* We get the replies of the QueryVersion request because we leave */
#ifdef ECORE_XCB_DAMAGE
   _ecore_x_damage_init_finalize();
#endif /* ECORE_XCB_DAMAGE */
#ifdef ECORE_XCB_COMPOSITE
   _ecore_x_composite_init_finalize();
#endif /* ECORE_XCB_COMPOSITE */
#ifdef ECORE_XCB_DPMS
   _ecore_x_dpms_init_finalize();
#endif /* ECORE_XCB_DPMS */
#ifdef ECORE_XCB_RANDR
   _ecore_x_randr_init_finalize();
#endif /* ECORE_XCB_RANDR */
#ifdef ECORE_XCB_SCREENSAVER
   _ecore_x_screensaver_init_finalize();
#endif /* ECORE_XCB_SCREENSAVER */
#ifdef ECORE_XCB_SHAPE
   _ecore_x_shape_init_finalize();
#endif /* ECORE_XCB_SHAPE */
#ifdef ECORE_XCB_SYNC
   _ecore_x_sync_init_finalize();
#endif /* ECORE_XCB_SYNC */
#ifdef ECORE_XCB_FIXES
   _ecore_x_xfixes_init_finalize();
#endif /* ECORE_XCB_FIXES */
#ifdef ECORE_XCB_XINERAMA
   _ecore_x_xinerama_init_finalize();
#endif /* ECORE_XCB_XINERAMA */
shutdown_ecore_event:
   ecore_event_shutdown();
shutdown_ecore:
   ecore_shutdown();
shutdown_eina:
   eina_log_domain_unregister(_ecore_x11xcb_log_dom);
   _ecore_x11xcb_log_dom = -1;
   eina_shutdown();

   return --_ecore_xcb_init_count;
} /* ecore_x_init */

static int
_ecore_x_shutdown(int close_display)
{
   if (--_ecore_xcb_init_count != 0)
      return _ecore_xcb_init_count;

   if (!_ecore_xcb_conn)
      return _ecore_xcb_init_count;

   ecore_main_fd_handler_del(_ecore_xcb_fd_handler_handle);
   if (close_display)
      xcb_disconnect(_ecore_xcb_conn);
   else
      close(xcb_get_file_descriptor(_ecore_xcb_conn));

   free(_ecore_xcb_event_handlers);
   ecore_event_filter_del(_ecore_xcb_filter_handler);
   _ecore_xcb_fd_handler_handle = NULL;
   _ecore_xcb_filter_handler = NULL;
   _ecore_xcb_conn = NULL;
   _ecore_xcb_event_handlers = NULL;
   _ecore_x_selection_shutdown();
   _ecore_x_dnd_shutdown();
   ecore_x_netwm_shutdown();
   _ecore_x_reply_shutdown();

   ecore_event_shutdown();
   ecore_shutdown();

   eina_log_domain_unregister(_ecore_x11xcb_log_dom);
   _ecore_x11xcb_log_dom = -1;
   eina_shutdown();

   return _ecore_xcb_init_count;
} /* _ecore_x_shutdown */

/**
 * Shuts down the Ecore X library.
 *
 * In shutting down the library, the X display connection is terminated
 * and any event handlers for it are removed.
 *
 * @return  The number of times the library has been initialized without
 *          being shut down.
 * @ingroup Ecore_Xcb_Init_Group
 */
EAPI int
ecore_x_shutdown(void)
{
   return _ecore_x_shutdown(1);
} /* ecore_x_shutdown */

/**
 * Shuts down the Ecore X library.
 *
 * As ecore_xcb_shutdown, except do not close Display, only connection.
 *
 * @ingroup Ecore_Xcb_Init_Group
 */
EAPI int
ecore_x_disconnect(void)
{
   return _ecore_x_shutdown(0);
} /* ecore_x_disconnect */

/**
 * @defgroup Ecore_Xcb_Display_Attr_Group X Display Attributes
 *
 * Functions that set and retrieve X display attributes.
 */

EAPI Ecore_X_Display *
ecore_x_display_get(void)
{
   return NULL;
} /* ecore_x_display_get */

/**
 * Retrieves the Ecore_X_Connection handle used for the current X connection.
 * @return  The current X connection.
 * @ingroup Ecore_Xcb_Display_Attr_Group
 */
EAPI Ecore_X_Connection *
ecore_x_connection_get(void)
{
   return (Ecore_X_Connection *)_ecore_xcb_conn;
} /* ecore_x_connection_get */

/**
 * Retrieves the X display file descriptor.
 * @return  The current X display file descriptor.
 * @ingroup Ecore_Xcb_Display_Attr_Group
 */
EAPI int
ecore_x_fd_get(void)
{
   return xcb_get_file_descriptor(_ecore_xcb_conn);
} /* ecore_x_fd_get */

/**
 * Retrieves the Ecore_X_Screen handle used for the current X connection.
 * @return  The current default screen.
 * @ingroup Ecore_Xcb_Display_Attr_Group
 */
EAPI Ecore_X_Screen *
ecore_x_default_screen_get(void)
{
   return (Ecore_X_Screen *)_ecore_xcb_screen;
} /* ecore_x_default_screen_get */

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
   xcb_screen_t *s = (xcb_screen_t *)screen;
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   if (w) *w = 0;
   if (h) *h = 0;
   if (!s) return;
   if (w) *w = s->width_in_pixels;
   if (h) *h = s->height_in_pixels;
}

/**
 * Sets the timeout for a double and triple clicks to be flagged.
 *
 * This sets the time between clicks before the double_click flag is
 * set in a button down event. If 3 clicks occur within double this
 * time, the triple_click flag is also set.
 *
 * @param   t The time in seconds
 * @ingroup Ecore_Xcb_Display_Attr_Group
 */
EAPI void
ecore_x_double_click_time_set(double t)
{
   if (t < 0.0)
      t = 0.0;

   _ecore_xcb_double_click_time = t;
} /* ecore_x_double_click_time_set */

/**
 * Retrieves the double and triple click flag timeout.
 *
 * See @ref ecore_xcb_double_click_time_set for more information.
 *
 * @return  The timeout for double clicks in seconds.
 * @ingroup Ecore_Xcb_Display_Attr_Group
 */
EAPI double
ecore_x_double_click_time_get(void)
{
   return _ecore_xcb_double_click_time;
} /* ecore_x_double_click_time_get */

/**
 * @defgroup Ecore_Xcb_Flush_Group X Synchronization Functions
 *
 * Functions that ensure that all commands that have been issued by the
 * Ecore X library have been sent to the server.
 */

/**
 * Sends all X commands in the X Display buffer.
 * @ingroup Ecore_Xcb_Flush_Group
 */
EAPI void
ecore_x_flush(void)
{
   xcb_flush(_ecore_xcb_conn);
} /* ecore_x_flush */

/**
 * Flushes the command buffer and waits until all requests have been
 * processed by the server.
 * @ingroup Ecore_Xcb_Flush_Group
 */
EAPI void
ecore_x_sync(void)
{
   free(xcb_get_input_focus_reply(_ecore_xcb_conn, xcb_get_input_focus(_ecore_xcb_conn), NULL));
} /* ecore_x_sync */

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
   int screens;
   int i;

   xcb_grab_server(_ecore_xcb_conn);
   screens = xcb_setup_roots_iterator (xcb_get_setup (_ecore_xcb_conn)).rem;

   /* Tranverse window tree starting from root, and drag each
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
             int tree_c_len;
             int i;

             wins = xcb_query_tree_children(reply);
             tree_c_len = xcb_query_tree_children_length(reply);
             for (i = 0; i < tree_c_len; i++)
                xcb_kill_client(_ecore_xcb_conn, wins[i]);
             free(reply);
          }
     }

   xcb_ungrab_server(_ecore_xcb_conn);
   free(xcb_get_input_focus_reply(_ecore_xcb_conn, xcb_get_input_focus(_ecore_xcb_conn), NULL));
} /* ecore_x_killall */

/**
 * Kill a specific client
 *
 * You can kill a specific client woning window @p window
 *
 * @param window Window of the client to be killed
 */
EAPI void
ecore_x_kill(Ecore_X_Window window)
{
   xcb_kill_client(_ecore_xcb_conn, window);
} /* ecore_x_kill */

/**
 * TODO: Invoke the standard system beep to alert users
 */
EAPI Eina_Bool
ecore_x_bell(int percent)
{
   return 0;
} /* ecore_x_bell */

/**
 * Return the last event time
 */
EAPI Ecore_X_Time
ecore_x_current_time_get(void)
{
   return _ecore_xcb_event_last_time;
} /* ecore_x_current_time_get */

static void
handle_event(xcb_generic_event_t *ev)
{
   uint8_t response_type = ev->response_type & ~0x80;

   if (response_type < _ecore_xcb_event_handlers_num)
     {
        if (_ecore_xcb_event_handlers[XCB_EVENT_ANY])
           _ecore_xcb_event_handlers[XCB_EVENT_ANY] (ev);

        if (_ecore_xcb_event_handlers[response_type])
           _ecore_xcb_event_handlers[response_type] (ev);
     }
} /* handle_event */

static Eina_Bool
_ecore_xcb_fd_handler(void *data, Ecore_Fd_Handler *fd_handler __UNUSED__)
{
   xcb_connection_t *c;
   xcb_generic_event_t *ev;

   c = (xcb_connection_t *)data;

/*    INF ("nbr events: %d", _ecore_xcb_event_handlers_num); */

   /* We check if _ecore_xcb_event_buffered is NULL or not */
   if (_ecore_xcb_event_buffered)
     {
        handle_event(_ecore_xcb_event_buffered);
        _ecore_xcb_event_buffered = NULL;
     }

   while ((ev = xcb_poll_for_event(c)))
      handle_event(ev);

   return ECORE_CALLBACK_RENEW;
} /* _ecore_xcb_fd_handler */

static Eina_Bool
_ecore_xcb_fd_handler_buf(void *data, Ecore_Fd_Handler *fd_handler __UNUSED__)
{
   xcb_connection_t *c;

   c = (xcb_connection_t *)data;

   _ecore_xcb_event_buffered = xcb_poll_for_event(c);
   if (!_ecore_xcb_event_buffered)
      return ECORE_CALLBACK_CANCEL;

   return ECORE_CALLBACK_RENEW;
} /* _ecore_xcb_fd_handler_buf */

/* FIXME: possible roundtrip */
/* FIXME: fix xcb_keysyms. It's ugly !! (reply in xcb_key_symbols_get_keysym) */
static int
_ecore_xcb_key_mask_get(xcb_keysym_t sym)
{
   xcb_get_modifier_mapping_cookie_t cookie;
   xcb_get_modifier_mapping_reply_t *reply;
   xcb_key_symbols_t *symbols;
   xcb_keysym_t sym2;
   int i, j;
   xcb_keycode_t *keycodes = NULL;
   int mod_keys_len;
   const int masks[8] =
   {
      XCB_MOD_MASK_SHIFT,
      XCB_MOD_MASK_LOCK,
      XCB_MOD_MASK_CONTROL,
      XCB_MOD_MASK_1,
      XCB_MOD_MASK_2,
      XCB_MOD_MASK_3,
      XCB_MOD_MASK_4,
      XCB_MOD_MASK_5
   };

   cookie = xcb_get_modifier_mapping_unchecked(_ecore_xcb_conn);
   symbols = xcb_key_symbols_alloc(_ecore_xcb_conn);

   reply = xcb_get_modifier_mapping_reply(_ecore_xcb_conn, cookie, NULL);
   if (!reply)
     {
        xcb_key_symbols_free(symbols);

        return 0;
     }

   keycodes = xcb_get_modifier_mapping_keycodes(reply);
   mod_keys_len = xcb_get_modifier_mapping_keycodes_length(reply);
   for (i = 0; i < mod_keys_len; i++)
     {
        for (j = 0; j < 8; j++)
          {
             sym2 = xcb_key_symbols_get_keysym(symbols, keycodes[i], j);
             if (sym2 != 0)
                break;
          }
        if (sym2 == sym)
          {
             int mask;

             mask = masks[j];
             free(reply);
             xcb_key_symbols_free(symbols);
             return mask;
          }
     }

   free(reply);
   xcb_key_symbols_free(symbols);

   return 0;
} /* _ecore_xcb_key_mask_get */

typedef struct _Ecore_X_Filter_Data   Ecore_X_Filter_Data;

struct _Ecore_X_Filter_Data
{
   int last_event_type;
};

static void *
_ecore_xcb_event_filter_start(void *data __UNUSED__)
{
   Ecore_X_Filter_Data *filter_data;

   filter_data = calloc(1, sizeof(Ecore_X_Filter_Data));
   return filter_data;
} /* _ecore_xcb_event_filter_start */

static Eina_Bool
_ecore_xcb_event_filter_filter(void *data __UNUSED__, void *loop_data,int type, void *event __UNUSED__)
{
   Ecore_X_Filter_Data *filter_data;

   filter_data = loop_data;
   if (!filter_data)
      return EINA_TRUE;

   if (type == ECORE_EVENT_MOUSE_MOVE)
     {
        if ((filter_data->last_event_type) == ECORE_EVENT_MOUSE_MOVE)
          {
             filter_data->last_event_type = type;
             return EINA_FALSE;
          }
     }

   filter_data->last_event_type = type;
   return EINA_TRUE;
} /* _ecore_xcb_event_filter_filter */

static void
_ecore_xcb_event_filter_end(void *data __UNUSED__, void *loop_data)
{
   Ecore_X_Filter_Data *filter_data;

   filter_data = loop_data;
   if (filter_data)
      free(filter_data);
} /* _ecore_xcb_event_filter_end */

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/* FIXME: these funcs need categorising */
/*****************************************************************************/

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
   uint8_t i;
   uint8_t num;
   Ecore_X_Window *roots;
/* #ifdef ECORE_XCBXPRINT */
/*    int xp_base, xp_err_base; */
/* #endif /\* ECORE_XCBXPRINT *\/ */

   if (!num_ret)
      return NULL;

   *num_ret = 0;

   /* FIXME: todo... */
/* #ifdef ECORE_XCBXPRINT */
/*    num = ScreenCount(_ecore_xcb_conn); */
/*    if (ecore_xcb_xprint_query()) */
/*      { */
/*	Screen **ps = NULL; */
/*	int psnum = 0; */

/*	ps = XpQueryScreens(_ecore_xcb_conn, &psnum); */
/*	if (ps) */
/*	  { */
/*	     int overlap, j; */

/*	     overlap = 0; */
/*	     for (i = 0; i < num; i++) */
/*	       { */
/*		  for (j = 0; j < psnum; j++) */
/*		    { */
/*		       if (ScreenOfDisplay(_ecore_xcb_conn, i) == ps[j]) */
/*			 overlap++; */
/*		    } */
/*	       } */
/*	     roots = malloc((num - overlap) * sizeof(Ecore_X_Window)); */
/*	     if (roots) */
/*	       { */
/*		  int k; */

/*		  k = 0; */
/*		  for (i = 0; i < num; i++) */
/*		    { */
/*		       int is_print; */

/*		       is_print = 0; */
/*		       for (j = 0; j < psnum; j++) */
/*			 { */
/*			    if (ScreenOfDisplay(_ecore_xcb_conn, i) == ps[j]) */
/*			      { */
/*				 is_print = 1; */
/*				 break; */
/*			      } */
/*			 } */
/*		       if (!is_print) */
/*			 { */
/*			    roots[k] = RootWindow(_ecore_xcb_conn, i); */
/*			    k++; */
/*			 } */
/*		    } */
/*		  *num_ret = k; */
/*	       } */
/*	     XFree(ps); */
/*	  } */
/*	else */
/*	  { */
/*	     roots = malloc(num * sizeof(Ecore_X_Window)); */
/*	     if (!roots) return NULL; */
/*	     *num_ret = num; */
/*	     for (i = 0; i < num; i++) */
/*	       roots[i] = RootWindow(_ecore_xcb_conn, i); */
/*	  } */
/*      } */
/*    else */
/*      { */
/*	roots = malloc(num * sizeof(Ecore_X_Window)); */
/*	if (!roots) return NULL; */
/*	*num_ret = num; */
/*	for (i = 0; i < num; i++) */
/*	  roots[i] = RootWindow(_ecore_xcb_conn, i); */
/*      } */
/* #else */
   setup = xcb_get_setup (_ecore_xcb_conn);
   iter = xcb_setup_roots_iterator (setup);
   num = setup->roots_len;
   roots = malloc(num * sizeof(Ecore_X_Window));
   if (!roots)
      return NULL;

   *num_ret = num;
   for (i = 0; iter.rem; xcb_screen_next(&iter), i++)
      roots[i] = iter.data->root;
/* #endif /\* ECORE_XCBXPRINT *\/ */

   return roots;
} /* ecore_x_window_root_list */

EAPI Ecore_X_Window
ecore_x_window_root_first_get(void)
{
   Ecore_X_Window *roots = NULL;
   Ecore_X_Window root;
   int num;

   roots = ecore_x_window_root_list(&num);
   if(!(roots))
      return 0;

   if (num > 0)
      root = roots[0];
   else
      root = 0;

   free(roots);
   return root;
} /* ecore_x_window_root_first_get */

/* FIXME: todo */

static void      _ecore_x_window_manage_error(void *data);

static int _ecore_xcb_window_manage_failed = 0;
static void
_ecore_x_window_manage_error(void *data __UNUSED__)
{
/*    if ((ecore_xcb_error_request_get() == X_ChangeWindowAttributes) && */
/*        (ecore_xcb_error_code_get() == BadAccess)) */
/*      _ecore_xcb_window_manage_failed = 1; */
} /* _ecore_x_window_manage_error */

/* FIXME: round trip */
EAPI Eina_Bool
ecore_x_window_manage(Ecore_X_Window window)
{
   xcb_get_window_attributes_cookie_t cookie_attr;
   xcb_get_input_focus_cookie_t cookie_sync;
   xcb_get_window_attributes_reply_t *reply_attr;
   xcb_get_input_focus_reply_t *reply_sync;
   uint32_t value_list;

   cookie_attr = xcb_get_window_attributes_unchecked(_ecore_xcb_conn, window);
   cookie_sync = xcb_get_input_focus_unchecked(_ecore_xcb_conn);

   reply_attr = xcb_get_window_attributes_reply(_ecore_xcb_conn, cookie_attr, NULL);
   if (!reply_attr)
     {
        reply_sync = xcb_get_input_focus_reply(_ecore_xcb_conn, cookie_sync, NULL);
        if (reply_sync)
           free(reply_sync);

        return 0;
     }

   reply_sync = xcb_get_input_focus_reply(_ecore_xcb_conn, cookie_sync, NULL);
   if (reply_sync)
      free(reply_sync);

   _ecore_xcb_window_manage_failed = 0;
   /* FIXME: no error code yet */
   /* ecore_xcb_error_handler_set(_ecore_xcb_window_manage_error, NULL); */

   value_list =
      XCB_EVENT_MASK_KEY_PRESS |
      XCB_EVENT_MASK_KEY_RELEASE |
      XCB_EVENT_MASK_ENTER_WINDOW |
      XCB_EVENT_MASK_LEAVE_WINDOW |
      XCB_EVENT_MASK_STRUCTURE_NOTIFY |
      XCB_EVENT_MASK_RESIZE_REDIRECT |
      XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY |
      XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT |
      XCB_EVENT_MASK_PROPERTY_CHANGE |
      reply_attr->your_event_mask;
   xcb_change_window_attributes(_ecore_xcb_conn, window,
                                XCB_CW_EVENT_MASK,
                                &value_list);
   free(reply_attr);

   cookie_sync = xcb_get_input_focus_unchecked(_ecore_xcb_conn);
   if (reply_sync)
      free(reply_sync);

   /* FIXME: no error code yet */
   /* ecore_xcb_error_handler_set(NULL, NULL); */
   if (_ecore_xcb_window_manage_failed)
     {
        _ecore_xcb_window_manage_failed = 0;
        return 0;
     }

   return 1;
} /* ecore_x_window_manage */

EAPI Eina_Bool
ecore_x_pointer_control_set(int accel_num,
                            int accel_denom,
                            int threshold)
{
   xcb_change_pointer_control(_ecore_xcb_conn,
                              accel_num, accel_denom, threshold,
                              1, 1);
   return 1;
} /* ecore_x_pointer_control_set */

EAPI void
ecore_x_pointer_control_get_prefetch(void)
{
   xcb_get_pointer_control_cookie_t cookie;

   cookie = xcb_get_pointer_control_unchecked(_ecore_xcb_conn);
   _ecore_xcb_cookie_cache(cookie.sequence);
} /* ecore_x_pointer_control_get_prefetch */

EAPI void
ecore_x_pointer_control_get_fetch(void)
{
   xcb_get_pointer_control_cookie_t cookie;
   xcb_get_pointer_control_reply_t *reply;

   cookie.sequence = _ecore_xcb_cookie_get();
   reply = xcb_get_pointer_control_reply(_ecore_xcb_conn, cookie, NULL);
   _ecore_xcb_reply_cache(reply);
} /* ecore_x_pointer_control_get_fetch */

EAPI Eina_Bool
ecore_x_pointer_control_get(int *accel_num,
                            int *accel_denom,
                            int *threshold)
{
   xcb_get_pointer_control_reply_t *reply;

   if (accel_num)
      *accel_num = 0;

   if (accel_denom)
      *accel_denom = 1;

   if (threshold)
      *threshold = 0;

   reply = _ecore_xcb_reply_get();
   if (!reply)
      return 0;

   if (accel_num)
      *accel_num = reply->acceleration_numerator;

   if (accel_denom)
      *accel_denom = reply->acceleration_denominator;

   if (threshold)
      *threshold = reply->threshold;

   return 1;
} /* ecore_x_pointer_control_get */

EAPI Eina_Bool
ecore_x_pointer_mapping_set(unsigned char *map,
                            int            nmap)
{
   xcb_set_pointer_mapping(_ecore_xcb_conn, nmap, map);
   return 1;
} /* ecore_x_pointer_mapping_set */

EAPI void
ecore_x_pointer_mapping_get_prefetch(void)
{
   xcb_get_pointer_mapping_cookie_t cookie;

   cookie = xcb_get_pointer_mapping_unchecked(_ecore_xcb_conn);
   _ecore_xcb_cookie_cache(cookie.sequence);
} /* ecore_x_pointer_mapping_get_prefetch */

EAPI void
ecore_x_pointer_mapping_get_fetch(void)
{
   xcb_get_pointer_mapping_cookie_t cookie;
   xcb_get_pointer_mapping_reply_t *reply;

   cookie.sequence = _ecore_xcb_cookie_get();
   reply = xcb_get_pointer_mapping_reply(_ecore_xcb_conn, cookie, NULL);
   _ecore_xcb_reply_cache(reply);
} /* ecore_x_pointer_mapping_get_fetch */

EAPI Eina_Bool
ecore_x_pointer_mapping_get(unsigned char *map,
                            int            nmap)
{
   xcb_get_pointer_mapping_cookie_t cookie;
   xcb_get_pointer_mapping_reply_t *reply;
   uint8_t *tmp;
   int i;

   cookie = xcb_get_pointer_mapping_unchecked(_ecore_xcb_conn);
   reply = xcb_get_pointer_mapping_reply(_ecore_xcb_conn, cookie, NULL);
   if (!reply)
      return 0;

   if (nmap > xcb_get_pointer_mapping_map_length(reply))
      return 0;

   tmp = xcb_get_pointer_mapping_map(reply);

   for (i = 0; i < nmap; i++)
      map[i] = tmp[i];

   return 1;
} /* ecore_x_pointer_mapping_get */

EAPI Eina_Bool
ecore_x_pointer_grab(Ecore_X_Window window)
{
   xcb_grab_pointer_cookie_t cookie;
   xcb_grab_pointer_reply_t *reply;

   cookie = xcb_grab_pointer_unchecked(_ecore_xcb_conn, 0, window,
                                       XCB_EVENT_MASK_BUTTON_PRESS |
                                       XCB_EVENT_MASK_BUTTON_RELEASE |
                                       XCB_EVENT_MASK_ENTER_WINDOW |
                                       XCB_EVENT_MASK_LEAVE_WINDOW |
                                       XCB_EVENT_MASK_POINTER_MOTION,
                                       XCB_GRAB_MODE_ASYNC,
                                       XCB_GRAB_MODE_ASYNC,
                                       XCB_NONE, XCB_NONE,
                                       XCB_CURRENT_TIME);
   reply = xcb_grab_pointer_reply(_ecore_xcb_conn, cookie, NULL);
   if (!reply)
      return 0;

   free(reply);

   return 1;
} /* ecore_x_pointer_grab */

EAPI Eina_Bool
ecore_x_pointer_confine_grab(Ecore_X_Window window)
{
   xcb_grab_pointer_cookie_t cookie;
   xcb_grab_pointer_reply_t *reply;

   cookie = xcb_grab_pointer_unchecked(_ecore_xcb_conn, 0, window,
                                       XCB_EVENT_MASK_BUTTON_PRESS |
                                       XCB_EVENT_MASK_BUTTON_RELEASE |
                                       XCB_EVENT_MASK_ENTER_WINDOW |
                                       XCB_EVENT_MASK_LEAVE_WINDOW |
                                       XCB_EVENT_MASK_POINTER_MOTION,
                                       XCB_GRAB_MODE_ASYNC,
                                       XCB_GRAB_MODE_ASYNC,
                                       window, XCB_NONE,
                                       XCB_CURRENT_TIME);
   reply = xcb_grab_pointer_reply(_ecore_xcb_conn, cookie, NULL);
   if (!reply)
      return 0;

   free(reply);

   return 1;
} /* ecore_x_pointer_confine_grab */

EAPI void
ecore_x_pointer_ungrab(void)
{
   xcb_ungrab_pointer(_ecore_xcb_conn, XCB_CURRENT_TIME);
} /* ecore_x_pointer_ungrab */

EAPI Eina_Bool
ecore_x_pointer_warp(Ecore_X_Window window,
                     int            x,
                     int            y)
{
   xcb_warp_pointer(_ecore_xcb_conn, XCB_NONE, window, 0, 0, 0, 0, x, y);

   return 1;
} /* ecore_x_pointer_warp */

EAPI Eina_Bool
ecore_x_keyboard_grab(Ecore_X_Window window)
{
   xcb_grab_keyboard_cookie_t cookie;
   xcb_grab_keyboard_reply_t *reply;

   cookie = xcb_grab_keyboard_unchecked(_ecore_xcb_conn, 0, window,
                                        XCB_CURRENT_TIME,
                                        XCB_GRAB_MODE_ASYNC,
                                        XCB_GRAB_MODE_ASYNC);
   reply = xcb_grab_keyboard_reply(_ecore_xcb_conn, cookie, NULL);
   if (!reply)
      return 0;

   free(reply);

   return 1;
} /* ecore_x_keyboard_grab */

EAPI void
ecore_x_keyboard_ungrab(void)
{
   xcb_ungrab_keyboard(_ecore_xcb_conn, XCB_CURRENT_TIME);
} /* ecore_x_keyboard_ungrab */

EAPI void
ecore_x_grab(void)
{
   _ecore_xcb_grab_count++;

   if (_ecore_xcb_grab_count == 1)
      xcb_grab_server(_ecore_xcb_conn);
} /* ecore_x_grab */

EAPI void
ecore_x_ungrab(void)
{
   _ecore_xcb_grab_count--;
   if (_ecore_xcb_grab_count < 0)
      _ecore_xcb_grab_count = 0;

   if (_ecore_xcb_grab_count == 0)
     {
        xcb_ungrab_server(_ecore_xcb_conn);
        free(xcb_get_input_focus_reply(_ecore_xcb_conn, xcb_get_input_focus(_ecore_xcb_conn), NULL));
     }
} /* ecore_x_ungrab */

int _ecore_window_grabs_num = 0;
Ecore_X_Window *_ecore_window_grabs = NULL;
Eina_Bool (*_ecore_window_grab_replay_func)(void *data, int event_type, void *event);
void *_ecore_window_grab_replay_data;

EAPI void
ecore_x_passive_grab_replay_func_set(Eina_Bool                             (*func)(void *data,
                                                                     int   event_type,
                                                                     void *event),
                                     void                                 *data)
{
   _ecore_window_grab_replay_func = func;
   _ecore_window_grab_replay_data = data;
} /* ecore_x_passive_grab_replay_func_set */

EAPI void
ecore_x_window_button_grab(Ecore_X_Window     window,
                           int                button,
                           Ecore_X_Event_Mask event_mask,
                           int                mod,
                           int                any_mod)
{
   int i;
   uint16_t m;
   uint16_t locks[8];
   uint16_t ev;

   m = _ecore_xcb_event_modifier(mod);
   if (any_mod)
      m = XCB_BUTTON_MASK_ANY;

   locks[0] = 0;
   locks[1] = ECORE_X_LOCK_CAPS;
   locks[2] = ECORE_X_LOCK_NUM;
   locks[3] = ECORE_X_LOCK_SCROLL;
   locks[4] = ECORE_X_LOCK_CAPS | ECORE_X_LOCK_NUM;
   locks[5] = ECORE_X_LOCK_CAPS | ECORE_X_LOCK_SCROLL;
   locks[6] = ECORE_X_LOCK_NUM | ECORE_X_LOCK_SCROLL;
   locks[7] = ECORE_X_LOCK_CAPS | ECORE_X_LOCK_NUM | ECORE_X_LOCK_SCROLL;
   ev = event_mask;
   for (i = 0; i < 8; i++)
      xcb_grab_button(_ecore_xcb_conn, 0, window, ev,
                      XCB_GRAB_MODE_SYNC, XCB_GRAB_MODE_ASYNC,
                      XCB_NONE, XCB_NONE, button, m | locks[i]);
   _ecore_window_grabs_num++;
   _ecore_window_grabs = realloc(_ecore_window_grabs,
                                 _ecore_window_grabs_num * sizeof(Ecore_X_Window));
   _ecore_window_grabs[_ecore_window_grabs_num - 1] = window;
} /* ecore_x_window_button_grab */

void
_ecore_x_sync_magic_send(int            val,
                         Ecore_X_Window swindow)
{
   xcb_client_message_event_t ev;

   ev.response_type = XCB_CLIENT_MESSAGE | 0x80;
   ev.format = 32;
   ev.window = _ecore_xcb_private_window;
   ev.type = 27777;
   ev.data.data32[0] = 0x7162534;
   ev.data.data32[1] = 0x10000000 + val;
   ev.data.data32[2] = swindow;

   xcb_send_event(_ecore_xcb_conn, 0, _ecore_xcb_private_window,
                  XCB_EVENT_MASK_NO_EVENT, (const char *)&ev);
} /* _ecore_x_sync_magic_send */

void
_ecore_x_window_grab_remove(Ecore_X_Window window)
{
   int i, shuffle = 0;

   if (_ecore_window_grabs_num > 0)
     {
        for (i = 0; i < _ecore_window_grabs_num; i++)
          {
             if (shuffle)
                _ecore_window_grabs[i - 1] = _ecore_window_grabs[i];

             if ((!shuffle) && (_ecore_window_grabs[i] == window))
                shuffle = 1;
          }
        if (shuffle)
          {
             _ecore_window_grabs_num--;
             _ecore_window_grabs = realloc(_ecore_window_grabs,
                                           _ecore_window_grabs_num * sizeof(Ecore_X_Window));
          }
     }
} /* _ecore_x_window_grab_remove */

EAPI void
ecore_x_window_button_ungrab(Ecore_X_Window window,
                             int            button,
                             int            mod,
                             int            any_mod)
{
   int i;
   uint16_t m;
   uint16_t locks[8];

   m = _ecore_xcb_event_modifier(mod);
   if (any_mod)
      m = XCB_BUTTON_MASK_ANY;

   locks[0] = 0;
   locks[1] = ECORE_X_LOCK_CAPS;
   locks[2] = ECORE_X_LOCK_NUM;
   locks[3] = ECORE_X_LOCK_SCROLL;
   locks[4] = ECORE_X_LOCK_CAPS | ECORE_X_LOCK_NUM;
   locks[5] = ECORE_X_LOCK_CAPS | ECORE_X_LOCK_SCROLL;
   locks[6] = ECORE_X_LOCK_NUM | ECORE_X_LOCK_SCROLL;
   locks[7] = ECORE_X_LOCK_CAPS | ECORE_X_LOCK_NUM | ECORE_X_LOCK_SCROLL;
   for (i = 0; i < 8; i++)
      xcb_ungrab_button(_ecore_xcb_conn, button, window, m | locks[i]);
   _ecore_x_sync_magic_send(1, window);
} /* ecore_x_window_button_ungrab */

int _ecore_key_grabs_num = 0;
Ecore_X_Window *_ecore_key_grabs = NULL;

EAPI void
ecore_x_window_key_grab(Ecore_X_Window window,
                        const char    *key,
                        int            mod,
                        int            any_mod)
{
   xcb_keycode_t keycode = 0;
   uint16_t m;
   uint16_t locks[8];
   int i;

   if (!strncmp(key, "Keycode-", 8))
      keycode = atoi(key + 8);

   /* FIXME: TODO... */

/*    else */
/*      { */
/*         KeySym              keysym; */

/*	keysym = XStringToKeysym(key); */
/*	if (keysym == NoSymbol) return; */
/*	keycode  = XKeysymToKeycode(_ecore_xcb_conn, XStringToKeysym(key)); */
/*      } */
   if (keycode == 0)
      return;

   m = _ecore_xcb_event_modifier(mod);
   if (any_mod)
      m = XCB_BUTTON_MASK_ANY;

   locks[0] = 0;
   locks[1] = ECORE_X_LOCK_CAPS;
   locks[2] = ECORE_X_LOCK_NUM;
   locks[3] = ECORE_X_LOCK_SCROLL;
   locks[4] = ECORE_X_LOCK_CAPS | ECORE_X_LOCK_NUM;
   locks[5] = ECORE_X_LOCK_CAPS | ECORE_X_LOCK_SCROLL;
   locks[6] = ECORE_X_LOCK_NUM | ECORE_X_LOCK_SCROLL;
   locks[7] = ECORE_X_LOCK_CAPS | ECORE_X_LOCK_NUM | ECORE_X_LOCK_SCROLL;
   for (i = 0; i < 8; i++)
      xcb_grab_key(_ecore_xcb_conn, 1, window, m | locks[i], keycode,
                   XCB_GRAB_MODE_SYNC, XCB_GRAB_MODE_ASYNC);
   _ecore_key_grabs_num++;
   _ecore_key_grabs = realloc(_ecore_key_grabs,
                              _ecore_key_grabs_num * sizeof(Ecore_X_Window));
   _ecore_key_grabs[_ecore_key_grabs_num - 1] = window;
} /* ecore_x_window_key_grab */

void
_ecore_x_key_grab_remove(Ecore_X_Window window)
{
   int i, shuffle = 0;

   if (_ecore_key_grabs_num > 0)
     {
        for (i = 0; i < _ecore_key_grabs_num; i++)
          {
             if (shuffle)
                _ecore_key_grabs[i - 1] = _ecore_key_grabs[i];

             if ((!shuffle) && (_ecore_key_grabs[i] == window))
                shuffle = 1;
          }
        if (shuffle)
          {
             _ecore_key_grabs_num--;
             _ecore_key_grabs = realloc(_ecore_key_grabs,
                                        _ecore_key_grabs_num * sizeof(Ecore_X_Window));
          }
     }
} /* _ecore_x_key_grab_remove */

EAPI void
ecore_x_window_key_ungrab(Ecore_X_Window window,
                          const char    *key,
                          int            mod,
                          int            any_mod)
{
   xcb_keycode_t keycode = 0;
   uint16_t m;
   uint16_t locks[8];
   int i;

   if (!strncmp(key, "Keycode-", 8))
      keycode = atoi(key + 8);

   /* FIXME: todo... */

/*    else */
/*      { */
/*         KeySym              keysym; */

/*	keysym = XStringToKeysym(key); */
/*	if (keysym == NoSymbol) return; */
/*	keycode  = XKeysymToKeycode(_ecore_xcb_conn, XStringToKeysym(key)); */
/*      } */
   if (keycode == 0)
      return;

   m = _ecore_xcb_event_modifier(mod);
   if (any_mod)
      m = XCB_BUTTON_MASK_ANY;

   locks[0] = 0;
   locks[1] = ECORE_X_LOCK_CAPS;
   locks[2] = ECORE_X_LOCK_NUM;
   locks[3] = ECORE_X_LOCK_SCROLL;
   locks[4] = ECORE_X_LOCK_CAPS | ECORE_X_LOCK_NUM;
   locks[5] = ECORE_X_LOCK_CAPS | ECORE_X_LOCK_SCROLL;
   locks[6] = ECORE_X_LOCK_NUM | ECORE_X_LOCK_SCROLL;
   locks[7] = ECORE_X_LOCK_CAPS | ECORE_X_LOCK_NUM | ECORE_X_LOCK_SCROLL;
   for (i = 0; i < 8; i++)
      xcb_ungrab_key(_ecore_xcb_conn, keycode, window, m | locks[i]);
   _ecore_x_sync_magic_send(2, window);
} /* ecore_x_window_key_ungrab */

/**
 * Send client message with given type and format 32.
 *
 * @param window  The window the message is sent to.
 * @param type    The client message type.
 * @param mask    The client message mask.
 * @param d0      The client message data item 1
 * @param d1      The client message data item 2
 * @param d2      The client message data item 3
 * @param d3      The client message data item 4
 * @param d4      The client message data item 5
 *
 * @return !0 on success.
 */
EAPI Eina_Bool
ecore_x_client_message32_send(Ecore_X_Window     window,
                              Ecore_X_Atom       type,
                              Ecore_X_Event_Mask mask,
                              long               d0,
                              long               d1,
                              long               d2,
                              long               d3,
                              long               d4)
{
   xcb_client_message_event_t ev;

   ev.response_type = XCB_CLIENT_MESSAGE | 0x80;
   ev.format = 32;
   ev.window = window;
   ev.type = type;
   ev.data.data32[0] = d0;
   ev.data.data32[1] = d1;
   ev.data.data32[2] = d2;
   ev.data.data32[3] = d3;
   ev.data.data32[4] = d4;

   xcb_send_event(_ecore_xcb_conn, 0, window, mask, (const char *)&ev);

   return 1;
} /* ecore_x_client_message32_send */

/**
 * Send client message with given type and format 8.
 *
 * @param window  The window the message is sent to.
 * @param type    The client message type.
 * @param data    Data to be sent.
 * @param len     Number of data bytes, max 20.
 *
 * @return !0 on success.
 */
EAPI Eina_Bool
ecore_x_client_message8_send(Ecore_X_Window window,
                             Ecore_X_Atom   type,
                             const void    *data,
                             int            len)
{
   xcb_client_message_event_t ev;

   ev.response_type = XCB_CLIENT_MESSAGE | 0x80;
   ev.format = 8;
   ev.window = window;
   ev.type = type;
   if (len > 20)
      len = 20;

   memcpy(ev.data.data8, data, len);
   memset(ev.data.data8 + len, 0, 20 - len);

   xcb_send_event(_ecore_xcb_conn, 0, window, XCB_EVENT_MASK_NO_EVENT, (const char *)&ev);

   return 1;
} /* ecore_x_client_message8_send */

/* FIXME: round trip */
EAPI Eina_Bool
ecore_x_mouse_move_send(Ecore_X_Window window,
                        int            x,
                        int            y)
{
   xcb_motion_notify_event_t ev;
   xcb_get_geometry_cookie_t cookie_geom;
   xcb_translate_coordinates_cookie_t cookie_trans;
   xcb_get_geometry_reply_t *reply_geom;
   xcb_translate_coordinates_reply_t *reply_trans;

   cookie_geom = xcb_get_geometry_unchecked(_ecore_xcb_conn, window);
   reply_geom = xcb_get_geometry_reply(_ecore_xcb_conn, cookie_geom, NULL);
   if (!reply_geom)
      return 0;

   cookie_trans = xcb_translate_coordinates_unchecked(_ecore_xcb_conn, window, reply_geom->root, x, y);
   reply_trans = xcb_translate_coordinates_reply(_ecore_xcb_conn, cookie_trans, NULL);
   if (!reply_trans)
     {
        free(reply_geom);
        return 0;
     }

   ev.response_type = XCB_MOTION_NOTIFY;
   ev.detail = 0;
   ev.time = _ecore_xcb_event_last_time;
   ev.root = reply_geom->root;
   ev.event = window;
   ev.child = window;
   ev.root_x = reply_trans->dst_x;
   ev.root_y = reply_trans->dst_y;
   ev.event_x = x;
   ev.event_y = y;
   ev.state = 0;
   ev.same_screen = 1;

   xcb_send_event(_ecore_xcb_conn, 1, window, XCB_EVENT_MASK_POINTER_MOTION, (const char *)&ev);

   free(reply_geom);
   free(reply_trans);

   return 1;
} /* ecore_x_mouse_move_send */

/* FIXME: round trip */
EAPI Eina_Bool
ecore_x_mouse_down_send(Ecore_X_Window window,
                        int            x,
                        int            y,
                        int            button)
{
   xcb_button_press_event_t ev;
   xcb_get_geometry_cookie_t cookie_geom;
   xcb_translate_coordinates_cookie_t cookie_trans;
   xcb_get_geometry_reply_t *reply_geom;
   xcb_translate_coordinates_reply_t *reply_trans;

   cookie_geom = xcb_get_geometry_unchecked(_ecore_xcb_conn, window);
   reply_geom = xcb_get_geometry_reply(_ecore_xcb_conn, cookie_geom, NULL);
   if (!reply_geom)
      return 0;

   cookie_trans = xcb_translate_coordinates_unchecked(_ecore_xcb_conn, window, reply_geom->root, x, y);
   reply_trans = xcb_translate_coordinates_reply(_ecore_xcb_conn, cookie_trans, NULL);
   if (!reply_trans)
     {
        free(reply_geom);
        return 0;
     }

   ev.response_type = XCB_BUTTON_PRESS;
   ev.detail = button;
   ev.time = _ecore_xcb_event_last_time;
   ev.root = reply_geom->root;
   ev.event = window;
   ev.child = window;
   ev.root_x = reply_trans->dst_x;
   ev.root_y = reply_trans->dst_y;
   ev.event_x = x;
   ev.event_y = y;
   ev.state = 1 << button;
   ev.same_screen = 1;

   xcb_send_event(_ecore_xcb_conn, 1, window, XCB_EVENT_MASK_BUTTON_PRESS, (const char *)&ev);

   free(reply_geom);
   free(reply_trans);

   return 1;
} /* ecore_x_mouse_down_send */

/* FIXME: round trip */
EAPI Eina_Bool
ecore_x_mouse_up_send(Ecore_X_Window window,
                      int            x,
                      int            y,
                      int            button)
{
   xcb_button_release_event_t ev;
   xcb_get_geometry_cookie_t cookie_geom;
   xcb_translate_coordinates_cookie_t cookie_trans;
   xcb_get_geometry_reply_t *reply_geom;
   xcb_translate_coordinates_reply_t *reply_trans;

   cookie_geom = xcb_get_geometry_unchecked(_ecore_xcb_conn, window);
   reply_geom = xcb_get_geometry_reply(_ecore_xcb_conn, cookie_geom, NULL);
   if (!reply_geom)
      return 0;

   cookie_trans = xcb_translate_coordinates_unchecked(_ecore_xcb_conn, window, reply_geom->root, x, y);
   reply_trans = xcb_translate_coordinates_reply(_ecore_xcb_conn, cookie_trans, NULL);
   if (!reply_trans)
     {
        free(reply_geom);
        return 0;
     }

   ev.response_type = XCB_BUTTON_RELEASE;
   ev.detail = button;
   ev.time = _ecore_xcb_event_last_time;
   ev.root = reply_geom->root;
   ev.event = window;
   ev.child = window;
   ev.root_x = reply_trans->dst_x;
   ev.root_y = reply_trans->dst_y;
   ev.event_x = x;
   ev.event_y = y;
   ev.state = 0;
   ev.same_screen = 1;

   xcb_send_event(_ecore_xcb_conn, 1, window, XCB_EVENT_MASK_BUTTON_RELEASE, (const char *)&ev);

   free(reply_geom);
   free(reply_trans);

   return 1;
} /* ecore_x_mouse_up_send */

EAPI void
ecore_x_focus_reset(void)
{
   xcb_set_input_focus(_ecore_xcb_conn,
                       (uint8_t)XCB_INPUT_FOCUS_POINTER_ROOT,
                       XCB_INPUT_FOCUS_POINTER_ROOT,
                       XCB_CURRENT_TIME);
} /* ecore_x_focus_reset */

EAPI void
ecore_x_events_allow_all(void)
{
   xcb_allow_events(_ecore_xcb_conn, XCB_ALLOW_ASYNC_BOTH, XCB_CURRENT_TIME);
} /* ecore_x_events_allow_all */

EAPI void
ecore_x_pointer_last_xy_get(int *x,
                            int *y)
{
   if (x)
      *x = _ecore_xcb_event_last_root_x;

   if (y)
      *y = _ecore_xcb_event_last_root_y;
} /* ecore_x_pointer_last_xy_get */

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

static int
_ecore_xcb_event_modifier(unsigned int state)
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

   return xmodifiers;
} /* _ecore_xcb_event_modifier */

