/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>

#include "ecore_private.h"
#include "Ecore.h"
#include "ecore_x_private.h"
#include "Ecore_X.h"
#include "Ecore_X_Atoms.h"

static int _ecore_x_fd_handler(void *data, Ecore_Fd_Handler *fd_handler);
static int _ecore_x_fd_handler_buf(void *data, Ecore_Fd_Handler *fd_handler);
static int _ecore_x_key_mask_get(KeySym sym);

static Ecore_Fd_Handler *_ecore_x_fd_handler_handle = NULL;
static int _ecore_x_event_shape_id = 0;
static int _ecore_x_event_screensaver_id = 0;
static int _ecore_x_event_sync_id = 0;

#ifdef ECORE_XRANDR
static int _ecore_x_event_randr_id = 0;
#endif
#ifdef ECORE_XFIXES
static int _ecore_x_event_fixes_selection_id = 0;
#endif
#ifdef ECORE_XDAMAGE
static int _ecore_x_event_damage_id = 0;
#endif
static int _ecore_x_event_handlers_num = 0;
static void (**_ecore_x_event_handlers) (XEvent * event) = NULL;

static int _ecore_x_init_count = 0;
static int _ecore_x_grab_count = 0;

Display *_ecore_x_disp = NULL;
double   _ecore_x_double_click_time = 0.25;
Time     _ecore_x_event_last_time = 0;
Window   _ecore_x_event_last_win = 0;
int      _ecore_x_event_last_root_x = 0;
int      _ecore_x_event_last_root_y = 0;
int      _ecore_x_xcursor = 0;
XIC      _ecore_x_ic = NULL; /* Input context for composed characters */

Ecore_X_Window _ecore_x_private_win = 0;

Ecore_X_Atom _ecore_x_atoms_wm_protocols[ECORE_X_WM_PROTOCOL_NUM];

EAPI int ECORE_X_EVENT_KEY_DOWN = 0;
EAPI int ECORE_X_EVENT_KEY_UP = 0;
EAPI int ECORE_X_EVENT_MOUSE_BUTTON_DOWN = 0;
EAPI int ECORE_X_EVENT_MOUSE_BUTTON_UP = 0;
EAPI int ECORE_X_EVENT_MOUSE_MOVE = 0;
EAPI int ECORE_X_EVENT_MOUSE_IN = 0;
EAPI int ECORE_X_EVENT_MOUSE_OUT = 0;
EAPI int ECORE_X_EVENT_MOUSE_WHEEL = 0;
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
/*
EAPI int ECORE_X_EVENT_WINDOW_PROP_TITLE_CHANGE = 0;
EAPI int ECORE_X_EVENT_WINDOW_PROP_VISIBLE_TITLE_CHANGE = 0;
EAPI int ECORE_X_EVENT_WINDOW_PROP_NAME_CLASS_CHANGE = 0;
EAPI int ECORE_X_EVENT_WINDOW_PROP_ICON_NAME_CHANGE = 0;
EAPI int ECORE_X_EVENT_WINDOW_PROP_VISIBLE_ICON_NAME_CHANGE = 0;
EAPI int ECORE_X_EVENT_WINDOW_PROP_CLIENT_MACHINE_CHANGE = 0;
EAPI int ECORE_X_EVENT_WINDOW_PROP_PID_CHANGE = 0;
EAPI int ECORE_X_EVENT_WINDOW_PROP_DESKTOP_CHANGE = 0;
*/

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
   int shape_base = 0;
   int shape_err_base = 0;
#ifdef ECORE_XSS
   int screensaver_base = 0;
   int screensaver_err_base = 0;
#endif
   int sync_base = 0;
   int sync_err_base = 0;
#ifdef ECORE_XRANDR
   int randr_base = 0;
   int randr_err_base = 0;
#endif
#ifdef ECORE_XFIXES
   int fixes_base = 0;
   int fixes_err_base = 0;
#endif
#ifdef ECORE_XDAMAGE
   int damage_base = 0;
   int damage_err_base = 0;
#endif
   
   if (_ecore_x_init_count > 0) 
     {
	_ecore_x_init_count++;
	return _ecore_x_init_count;
     }
   _ecore_x_disp = XOpenDisplay((char *)name);
   if (!_ecore_x_disp) return 0;
   _ecore_x_error_handler_init();
   _ecore_x_event_handlers_num = LASTEvent;
   
   if (XShapeQueryExtension(_ecore_x_disp, &shape_base, &shape_err_base))
     _ecore_x_event_shape_id = shape_base + ShapeNotify;
   if (_ecore_x_event_shape_id >= _ecore_x_event_handlers_num)
     _ecore_x_event_handlers_num = _ecore_x_event_shape_id + 1;
   
#ifdef ECORE_XSS
   if (XScreenSaverQueryExtension(_ecore_x_disp, &screensaver_base, &screensaver_err_base))
     _ecore_x_event_screensaver_id = screensaver_base + ScreenSaverNotify;
#endif   
   if (_ecore_x_event_screensaver_id >= _ecore_x_event_handlers_num)
     _ecore_x_event_handlers_num = _ecore_x_event_screensaver_id + 1;
   
   if (XSyncQueryExtension(_ecore_x_disp, &sync_base, &sync_err_base))
     {
	int major, minor;

	_ecore_x_event_sync_id = sync_base;
	if (!XSyncInitialize(_ecore_x_disp, &major, &minor))
	  _ecore_x_event_sync_id = 0;
     }
   if (_ecore_x_event_sync_id + XSyncAlarmNotify >= _ecore_x_event_handlers_num)
     _ecore_x_event_handlers_num = _ecore_x_event_sync_id + XSyncAlarmNotify + 1;
   
#ifdef ECORE_XRANDR
   if (XRRQueryExtension(_ecore_x_disp, &randr_base, &randr_err_base))
     _ecore_x_event_randr_id = randr_base + RRScreenChangeNotify;
   if (_ecore_x_event_randr_id >= _ecore_x_event_handlers_num)
     _ecore_x_event_handlers_num = _ecore_x_event_randr_id + 1;
#endif

#ifdef ECORE_XFIXES
   if (XFixesQueryExtension(_ecore_x_disp, &fixes_base, &fixes_err_base))
     _ecore_x_event_fixes_selection_id = fixes_base + XFixesSelectionNotify;
   if (_ecore_x_event_fixes_selection_id >= _ecore_x_event_handlers_num)
     _ecore_x_event_handlers_num = _ecore_x_event_fixes_selection_id + 1;
#endif

#ifdef ECORE_XDAMAGE
   if (XDamageQueryExtension(_ecore_x_disp, &damage_base, &damage_err_base))
     _ecore_x_event_damage_id = damage_base + XDamageNotify;
   if (_ecore_x_event_damage_id >= _ecore_x_event_handlers_num)
     _ecore_x_event_handlers_num = _ecore_x_event_damage_id + 1;
#endif

   _ecore_x_event_handlers = calloc(_ecore_x_event_handlers_num, sizeof(void *));
   if (!_ecore_x_event_handlers)
     {
        XCloseDisplay(_ecore_x_disp);
	_ecore_x_fd_handler_handle = NULL;
	_ecore_x_disp = NULL;
	return 0;	
     }
#ifdef ECORE_XCURSOR   
   _ecore_x_xcursor = XcursorSupportsARGB(_ecore_x_disp);
#endif
   _ecore_x_event_handlers[KeyPress]         = _ecore_x_event_handle_key_press;
   _ecore_x_event_handlers[KeyRelease]       = _ecore_x_event_handle_key_release;
   _ecore_x_event_handlers[ButtonPress]      = _ecore_x_event_handle_button_press;
   _ecore_x_event_handlers[ButtonRelease]    = _ecore_x_event_handle_button_release;
   _ecore_x_event_handlers[MotionNotify]     = _ecore_x_event_handle_motion_notify;
   _ecore_x_event_handlers[EnterNotify]      = _ecore_x_event_handle_enter_notify;
   _ecore_x_event_handlers[LeaveNotify]      = _ecore_x_event_handle_leave_notify;
   _ecore_x_event_handlers[FocusIn]          = _ecore_x_event_handle_focus_in;
   _ecore_x_event_handlers[FocusOut]         = _ecore_x_event_handle_focus_out;
   _ecore_x_event_handlers[KeymapNotify]     = _ecore_x_event_handle_keymap_notify;
   _ecore_x_event_handlers[Expose]           = _ecore_x_event_handle_expose;
   _ecore_x_event_handlers[GraphicsExpose]   = _ecore_x_event_handle_graphics_expose;
   _ecore_x_event_handlers[VisibilityNotify] = _ecore_x_event_handle_visibility_notify;
   _ecore_x_event_handlers[CreateNotify]     = _ecore_x_event_handle_create_notify;
   _ecore_x_event_handlers[DestroyNotify]    = _ecore_x_event_handle_destroy_notify;
   _ecore_x_event_handlers[UnmapNotify]      = _ecore_x_event_handle_unmap_notify;
   _ecore_x_event_handlers[MapNotify]        = _ecore_x_event_handle_map_notify;
   _ecore_x_event_handlers[MapRequest]       = _ecore_x_event_handle_map_request;
   _ecore_x_event_handlers[ReparentNotify]   = _ecore_x_event_handle_reparent_notify;
   _ecore_x_event_handlers[ConfigureNotify]  = _ecore_x_event_handle_configure_notify;
   _ecore_x_event_handlers[ConfigureRequest] = _ecore_x_event_handle_configure_request;
   _ecore_x_event_handlers[GravityNotify]    = _ecore_x_event_handle_gravity_notify;
   _ecore_x_event_handlers[ResizeRequest]    = _ecore_x_event_handle_resize_request;
   _ecore_x_event_handlers[CirculateNotify]  = _ecore_x_event_handle_circulate_notify;
   _ecore_x_event_handlers[CirculateRequest] = _ecore_x_event_handle_circulate_request;
   _ecore_x_event_handlers[PropertyNotify]   = _ecore_x_event_handle_property_notify;
   _ecore_x_event_handlers[SelectionClear]   = _ecore_x_event_handle_selection_clear;
   _ecore_x_event_handlers[SelectionRequest] = _ecore_x_event_handle_selection_request;
   _ecore_x_event_handlers[SelectionNotify]  = _ecore_x_event_handle_selection_notify;
   _ecore_x_event_handlers[ColormapNotify]   = _ecore_x_event_handle_colormap_notify;
   _ecore_x_event_handlers[ClientMessage]    = _ecore_x_event_handle_client_message;
   _ecore_x_event_handlers[MappingNotify]    = _ecore_x_event_handle_mapping_notify;
   if (_ecore_x_event_shape_id)
     _ecore_x_event_handlers[_ecore_x_event_shape_id] = _ecore_x_event_handle_shape_change;
   if (_ecore_x_event_screensaver_id)
     _ecore_x_event_handlers[_ecore_x_event_screensaver_id] = _ecore_x_event_handle_screensaver_notify;
   if (_ecore_x_event_sync_id)
     {
	_ecore_x_event_handlers[_ecore_x_event_sync_id + XSyncCounterNotify] =
	   _ecore_x_event_handle_sync_counter;
	_ecore_x_event_handlers[_ecore_x_event_sync_id + XSyncAlarmNotify] =
	   _ecore_x_event_handle_sync_alarm;
     }
#ifdef ECORE_XRANDR
   if (_ecore_x_event_randr_id)
     _ecore_x_event_handlers[_ecore_x_event_randr_id] = _ecore_x_event_handle_randr_change;
#endif
#ifdef ECORE_XFIXES
   if (_ecore_x_event_fixes_selection_id)
     _ecore_x_event_handlers[_ecore_x_event_fixes_selection_id] = _ecore_x_event_handle_fixes_selection_notify;
#endif
#ifdef ECORE_XDAMAGE
   if (_ecore_x_event_damage_id)
     _ecore_x_event_handlers[_ecore_x_event_damage_id] = _ecore_x_event_handle_damage_notify;
#endif
#ifdef ECORE_XKB
   // set x autorepeat detection to on. that means instead of
   //   press-release-press-release-press-release
   // you get
   //   press-press-press-press-press-release
   do
     {
        Bool works = 0;
        XkbSetDetectableAutoRepeat(_ecore_x_disp, 1, &works);
     }
   while (0);
#endif
   
   if (!ECORE_X_EVENT_KEY_DOWN)
     {
	ECORE_X_EVENT_KEY_DOWN                 = ecore_event_type_new();
	ECORE_X_EVENT_KEY_UP                   = ecore_event_type_new();
	ECORE_X_EVENT_MOUSE_BUTTON_DOWN        = ecore_event_type_new();
	ECORE_X_EVENT_MOUSE_BUTTON_UP          = ecore_event_type_new();
	ECORE_X_EVENT_MOUSE_MOVE               = ecore_event_type_new();
	ECORE_X_EVENT_MOUSE_IN                 = ecore_event_type_new();
	ECORE_X_EVENT_MOUSE_OUT                = ecore_event_type_new();
	ECORE_X_EVENT_MOUSE_WHEEL              = ecore_event_type_new();
	ECORE_X_EVENT_WINDOW_FOCUS_IN          = ecore_event_type_new();
	ECORE_X_EVENT_WINDOW_FOCUS_OUT         = ecore_event_type_new();
	ECORE_X_EVENT_WINDOW_KEYMAP            = ecore_event_type_new();
	ECORE_X_EVENT_WINDOW_DAMAGE            = ecore_event_type_new();
	ECORE_X_EVENT_WINDOW_VISIBILITY_CHANGE = ecore_event_type_new();
	ECORE_X_EVENT_WINDOW_CREATE            = ecore_event_type_new();
	ECORE_X_EVENT_WINDOW_DESTROY           = ecore_event_type_new();
	ECORE_X_EVENT_WINDOW_HIDE              = ecore_event_type_new();
	ECORE_X_EVENT_WINDOW_SHOW              = ecore_event_type_new();
	ECORE_X_EVENT_WINDOW_SHOW_REQUEST      = ecore_event_type_new();
	ECORE_X_EVENT_WINDOW_REPARENT          = ecore_event_type_new();
	ECORE_X_EVENT_WINDOW_CONFIGURE         = ecore_event_type_new();
	ECORE_X_EVENT_WINDOW_CONFIGURE_REQUEST = ecore_event_type_new();
	ECORE_X_EVENT_WINDOW_GRAVITY           = ecore_event_type_new();
	ECORE_X_EVENT_WINDOW_RESIZE_REQUEST    = ecore_event_type_new();
	ECORE_X_EVENT_WINDOW_STACK             = ecore_event_type_new();
	ECORE_X_EVENT_WINDOW_STACK_REQUEST     = ecore_event_type_new();
	ECORE_X_EVENT_WINDOW_PROPERTY          = ecore_event_type_new();
	ECORE_X_EVENT_WINDOW_COLORMAP          = ecore_event_type_new();
	ECORE_X_EVENT_WINDOW_MAPPING           = ecore_event_type_new();
	ECORE_X_EVENT_SELECTION_CLEAR          = ecore_event_type_new();
	ECORE_X_EVENT_SELECTION_REQUEST        = ecore_event_type_new();
	ECORE_X_EVENT_SELECTION_NOTIFY         = ecore_event_type_new();
	ECORE_X_EVENT_CLIENT_MESSAGE           = ecore_event_type_new();
	ECORE_X_EVENT_WINDOW_SHAPE             = ecore_event_type_new();
	ECORE_X_EVENT_SCREENSAVER_NOTIFY       = ecore_event_type_new();
	ECORE_X_EVENT_SYNC_COUNTER             = ecore_event_type_new();
	ECORE_X_EVENT_SYNC_ALARM               = ecore_event_type_new();
	ECORE_X_EVENT_SCREEN_CHANGE            = ecore_event_type_new();
	ECORE_X_EVENT_DAMAGE_NOTIFY            = ecore_event_type_new();
	
	ECORE_X_EVENT_WINDOW_DELETE_REQUEST                = ecore_event_type_new();
	/*
	ECORE_X_EVENT_WINDOW_PROP_TITLE_CHANGE             = ecore_event_type_new();
	ECORE_X_EVENT_WINDOW_PROP_VISIBLE_TITLE_CHANGE     = ecore_event_type_new();
        ECORE_X_EVENT_WINDOW_PROP_NAME_CLASS_CHANGE        = ecore_event_type_new();
	ECORE_X_EVENT_WINDOW_PROP_ICON_NAME_CHANGE         = ecore_event_type_new();
	ECORE_X_EVENT_WINDOW_PROP_VISIBLE_ICON_NAME_CHANGE = ecore_event_type_new();
	ECORE_X_EVENT_WINDOW_PROP_CLIENT_MACHINE_CHANGE    = ecore_event_type_new();
	ECORE_X_EVENT_WINDOW_PROP_PID_CHANGE               = ecore_event_type_new();
	ECORE_X_EVENT_WINDOW_PROP_DESKTOP_CHANGE           = ecore_event_type_new();
	*/

	ECORE_X_EVENT_DESKTOP_CHANGE           = ecore_event_type_new();
	ECORE_X_EVENT_WINDOW_MOVE_RESIZE_REQUEST = ecore_event_type_new();
	ECORE_X_EVENT_WINDOW_STATE_REQUEST     = ecore_event_type_new();
	ECORE_X_EVENT_FRAME_EXTENTS_REQUEST    = ecore_event_type_new();
	ECORE_X_EVENT_PING                     = ecore_event_type_new();

	ECORE_X_EVENT_STARTUP_SEQUENCE_NEW     = ecore_event_type_new();
	ECORE_X_EVENT_STARTUP_SEQUENCE_CHANGE  = ecore_event_type_new();
	ECORE_X_EVENT_STARTUP_SEQUENCE_REMOVE  = ecore_event_type_new();
     }
   
   /* everything has these... unless its like a pda... :) */
   ECORE_X_MODIFIER_SHIFT = _ecore_x_key_mask_get(XK_Shift_L);
   ECORE_X_MODIFIER_CTRL  = _ecore_x_key_mask_get(XK_Control_L);
   
   /* apple's xdarwin has no alt!!!! */
   ECORE_X_MODIFIER_ALT   = _ecore_x_key_mask_get(XK_Alt_L);
   if (!ECORE_X_MODIFIER_ALT) 
     ECORE_X_MODIFIER_ALT = _ecore_x_key_mask_get(XK_Meta_L);
   if (!ECORE_X_MODIFIER_ALT) 
     ECORE_X_MODIFIER_ALT = _ecore_x_key_mask_get(XK_Super_L);   
   
   /* the windows key... a valid modifier :) */
   ECORE_X_MODIFIER_WIN   = _ecore_x_key_mask_get(XK_Super_L);
   if (!ECORE_X_MODIFIER_WIN) 
     ECORE_X_MODIFIER_WIN = _ecore_x_key_mask_get(XK_Mode_switch);
   if (!ECORE_X_MODIFIER_WIN) 
     ECORE_X_MODIFIER_WIN = _ecore_x_key_mask_get(XK_Meta_L);
   
   if (ECORE_X_MODIFIER_WIN == ECORE_X_MODIFIER_ALT)
     ECORE_X_MODIFIER_WIN = 0;
   if (ECORE_X_MODIFIER_ALT == ECORE_X_MODIFIER_CTRL)
     ECORE_X_MODIFIER_ALT = 0;
   
   ECORE_X_LOCK_SCROLL    = _ecore_x_key_mask_get(XK_Scroll_Lock);
   ECORE_X_LOCK_NUM       = _ecore_x_key_mask_get(XK_Num_Lock);
   ECORE_X_LOCK_CAPS      = _ecore_x_key_mask_get(XK_Caps_Lock);
   
   _ecore_x_fd_handler_handle = 
     ecore_main_fd_handler_add(ConnectionNumber(_ecore_x_disp),
			       ECORE_FD_READ,
			       _ecore_x_fd_handler, _ecore_x_disp,
			       _ecore_x_fd_handler_buf, _ecore_x_disp);
   if (!_ecore_x_fd_handler_handle)
     {
	XCloseDisplay(_ecore_x_disp);
	free(_ecore_x_event_handlers);
	_ecore_x_fd_handler_handle = NULL;
	_ecore_x_disp = NULL;
	_ecore_x_event_handlers = NULL;
	return 0;
     }
   

   _ecore_x_atoms_init();

   /* Set up the ICCCM hints */
   ecore_x_icccm_init();

   /* Set up the _NET_... hints */
   ecore_x_netwm_init();

   /* old e hints init */
   ecore_x_e_init();
   
   /* This is just to be anal about naming conventions */

   _ecore_x_atoms_wm_protocols[ECORE_X_WM_PROTOCOL_DELETE_REQUEST] = ECORE_X_ATOM_WM_DELETE_WINDOW;
   _ecore_x_atoms_wm_protocols[ECORE_X_WM_PROTOCOL_TAKE_FOCUS] = ECORE_X_ATOM_WM_TAKE_FOCUS;
   _ecore_x_atoms_wm_protocols[ECORE_X_NET_WM_PROTOCOL_PING] = ECORE_X_ATOM_NET_WM_PING;
   _ecore_x_atoms_wm_protocols[ECORE_X_NET_WM_PROTOCOL_SYNC_REQUEST] = ECORE_X_ATOM_NET_WM_SYNC_REQUEST;

   _ecore_x_selection_data_init();
   _ecore_x_dnd_init();
   _ecore_x_fixes_init();
   _ecore_x_damage_init();
   _ecore_x_composite_init();
   _ecore_x_dpms_init();
   
   _ecore_x_init_count++;
   
   _ecore_x_private_win = ecore_x_window_override_new(0, -77, -777, 123, 456);
   
   /* Setup XIM */
   if (!_ecore_x_ic && XSupportsLocale())
     {
	XIM im;
	XIC ic;
	XIMStyles *supported_styles;
	XIMStyle chosen_style = 0;
	Ecore_X_Window client_window = ecore_x_window_root_get(_ecore_x_private_win);
	char *ret;
	int i;

	XSetLocaleModifiers("@im=none");
	if ((im = XOpenIM(_ecore_x_disp, NULL, NULL, NULL)) == NULL)
	  goto _im_create_end;
	ret = XGetIMValues(im, XNQueryInputStyle, &supported_styles, NULL);
	if (ret || !supported_styles)
	  goto _im_create_error;
	for (i = 0; i < supported_styles->count_styles; i++)
	  if (supported_styles->supported_styles[i] == (XIMPreeditNothing | XIMStatusNothing))
	    chosen_style = supported_styles->supported_styles[i];
	XFree(supported_styles);
	if (!chosen_style)
	  goto _im_create_error;
	ic = XCreateIC(im, XNInputStyle, chosen_style, XNClientWindow, client_window, NULL);
	if (ic)
	  {
	     _ecore_x_ic = ic;
	     goto _im_create_end;
	  }
_im_create_error:
	XCloseIM(im);
     }
_im_create_end:

   return _ecore_x_init_count;
}

static int
_ecore_x_shutdown(int close_display)
{
   _ecore_x_init_count--;
   if (_ecore_x_init_count > 0) return _ecore_x_init_count;
   if (!_ecore_x_disp) return _ecore_x_init_count;
   if (_ecore_x_ic)
     {
	XIM xim;
	xim = XIMOfIC(_ecore_x_ic);
	XDestroyIC(_ecore_x_ic);
	XCloseIM(xim);
	_ecore_x_ic = NULL;
     }
   if (close_display)
      XCloseDisplay(_ecore_x_disp);
   else
      close(ConnectionNumber(_ecore_x_disp));
   free(_ecore_x_event_handlers);
   ecore_main_fd_handler_del(_ecore_x_fd_handler_handle);
   _ecore_x_fd_handler_handle = NULL;
   _ecore_x_disp = NULL;
   _ecore_x_event_handlers = NULL;
   _ecore_x_selection_shutdown();
   _ecore_x_dnd_shutdown();
   ecore_x_netwm_shutdown();
   if (_ecore_x_init_count < 0) _ecore_x_init_count = 0;
   return _ecore_x_init_count;
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
   return _ecore_x_shutdown(1);
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
   return _ecore_x_shutdown(0);
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
   return (Ecore_X_Display *)_ecore_x_disp;
}

/**
 * Retrieves the X display file descriptor.
 * @return  The current X display file descriptor.
 * @ingroup Ecore_X_Display_Attr_Group
 */
EAPI int
ecore_x_fd_get(void)
{
   return ConnectionNumber(_ecore_x_disp);
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
   _ecore_x_double_click_time = t;
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
   return _ecore_x_double_click_time;
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
   XFlush(_ecore_x_disp);
}

/**
 * Flushes the command buffer and waits until all requests have been
 * processed by the server.
 * @ingroup Ecore_X_Flush_Group
 */
EAPI void
ecore_x_sync(void)
{
   XSync(_ecore_x_disp, False);
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
   unsigned int j;
   Window root_r;
   Window parent_r;
   Window *children_r = NULL;
   unsigned int num_children = 0;
   
   XGrabServer(_ecore_x_disp);
   /* Tranverse window tree starting from root, and drag each
    * before the firing squad */
   while (XQueryTree(_ecore_x_disp, root, &root_r, &parent_r,
		     &children_r, &num_children) && (num_children > 0))
     {
	for (j = 0; j < num_children; ++j)
	  {
	     XKillClient(_ecore_x_disp, children_r[j]);
	  }
	
	XFree(children_r);
     }
   XUngrabServer(_ecore_x_disp);
   XSync(_ecore_x_disp, False);
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
   XKillClient(_ecore_x_disp, win);
}

/**
 * Return the last event time
 */
EAPI Ecore_X_Time
ecore_x_current_time_get(void)
{
   return _ecore_x_event_last_time;
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
   Screen *s;
   
   s = DefaultScreenOfDisplay(_ecore_x_disp);
   if (s->mwidth <= 0) return 75;
   return (((s->width * 254) / s->mwidth) + 5) / 10;
}

static int
_ecore_x_fd_handler(void *data, Ecore_Fd_Handler *fd_handler __UNUSED__)
{
   Display *d;
   
   d = data;
   while (XPending(d))
     {
	XEvent ev;
	
	XNextEvent(d, &ev);

	/* Filter event for XIM */
	if (XFilterEvent(&ev, ev.xkey.window)) continue;

	if ((ev.type >= 0) && (ev.type < _ecore_x_event_handlers_num))
	  {
	     if (_ecore_x_event_handlers[ev.type])
	       _ecore_x_event_handlers[ev.type] (&ev);
	  }
     }
   return 1;
}

static int
_ecore_x_fd_handler_buf(void *data, Ecore_Fd_Handler *fd_handler __UNUSED__)
{
   Display *d;

   d = data;
   if (XPending(d)) return 1;
   return 0;
}

static int
_ecore_x_key_mask_get(KeySym sym)
{
   XModifierKeymap    *mod;
   KeySym              sym2;
   int                 i, j;
   const int           masks[8] = 
     {
	ShiftMask, LockMask, ControlMask, 
	  Mod1Mask, Mod2Mask, Mod3Mask, Mod4Mask, Mod5Mask
     };
   
   mod = XGetModifierMapping(_ecore_x_disp);
   if ((mod) && (mod->max_keypermod > 0))
     {
	for (i = 0; i < (8 * mod->max_keypermod); i++)
	  {
	     for (j = 0; j < 8; j++)
	       {
		  sym2 = XKeycodeToKeysym(_ecore_x_disp, mod->modifiermap[i], j);
		  if (sym2 != 0) break;
	       }
	     if (sym2 == sym)
	       {
		  int mask;
		  
		  mask = masks[i / mod->max_keypermod];
		  if (mod->modifiermap) XFree(mod->modifiermap);
		  XFree(mod);
		  return mask;
	       }
	  }
     }
   if (mod)
     {
	if (mod->modifiermap) XFree(mod->modifiermap);
	XFree(mod);
     }
  return 0;
}






















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
   int num, i;
   Ecore_X_Window *roots;
#ifdef ECORE_XPRINT
   int xp_base, xp_err_base;
#endif
   
   if (!num_ret) return NULL;
   *num_ret = 0;

#ifdef ECORE_XPRINT
   num = ScreenCount(_ecore_x_disp);
   if (XpQueryExtension(_ecore_x_disp, &xp_base, &xp_err_base))
     {
	Screen **ps = NULL;
	int psnum = 0;
	
	ps = XpQueryScreens(_ecore_x_disp, &psnum);
	if (ps)
	  {
	     int overlap, j;
	     
	     overlap = 0;
	     for (i = 0; i < num; i++)
	       {
		  for (j = 0; j < psnum; j++)
		    {
		       if (ScreenOfDisplay(_ecore_x_disp, i) == ps[j])
			 overlap++;
		    }
	       }
	     roots = malloc((num - overlap) * sizeof(Window));
	     if (roots)
	       {
		  int k;
		  
		  k = 0;
		  for (i = 0; i < num; i++)
		    {
		       int is_print;
		       
		       is_print = 0;
		       for (j = 0; j < psnum; j++)
			 {
			    if (ScreenOfDisplay(_ecore_x_disp, i) == ps[j])
			      {
				 is_print = 1;
				 break;
			      }
			 }
		       if (!is_print)
			 {
			    roots[k] = RootWindow(_ecore_x_disp, i);
			    k++;
			 }
		    }
		  *num_ret = k;
	       }
	     XFree(ps);
	  }
	else
	  {
	     roots = malloc(num * sizeof(Window));
	     if (!roots) return NULL;
	     *num_ret = num;
	     for (i = 0; i < num; i++)
	       roots[i] = RootWindow(_ecore_x_disp, i);
	  }
     }
   else
     {
	roots = malloc(num * sizeof(Window));
	if (!roots) return NULL;
	*num_ret = num;
	for (i = 0; i < num; i++)
	  roots[i] = RootWindow(_ecore_x_disp, i);
     }
#else   
   num = ScreenCount(_ecore_x_disp);
   roots = malloc(num * sizeof(Window));
   if (!roots) return NULL;
   *num_ret = num;
   for (i = 0; i < num; i++)
     roots[i] = RootWindow(_ecore_x_disp, i);
#endif   
   return roots;
}

EAPI Ecore_X_Window
ecore_x_window_root_first_get(void)
{
   int num;
   Ecore_X_Window root, *roots = NULL;

   roots = ecore_x_window_root_list(&num);
   if(!(roots)) return 0;
   
   if (num > 0)
      root = roots[0];
   else
      root = 0;

   free(roots);
   return root;
}


static void _ecore_x_window_manage_error(void *data);

static int _ecore_x_window_manage_failed = 0;
static void
_ecore_x_window_manage_error(void *data __UNUSED__)
{
   if ((ecore_x_error_request_get() == X_ChangeWindowAttributes) &&
       (ecore_x_error_code_get() == BadAccess))
     _ecore_x_window_manage_failed = 1;
}

EAPI int
ecore_x_window_manage(Ecore_X_Window win)
{
   XWindowAttributes   att;
   
   if (XGetWindowAttributes(_ecore_x_disp, win, &att) != True) return 0;
   ecore_x_sync();
   _ecore_x_window_manage_failed = 0;
   ecore_x_error_handler_set(_ecore_x_window_manage_error, NULL);
   XSelectInput(_ecore_x_disp, win, 
		EnterWindowMask | 
		LeaveWindowMask | 
		PropertyChangeMask | 
		ResizeRedirectMask |
		SubstructureRedirectMask | 
		SubstructureNotifyMask |
		StructureNotifyMask |
		KeyPressMask | 
		KeyReleaseMask |
		att.your_event_mask);
   ecore_x_sync();
   ecore_x_error_handler_set(NULL, NULL);
   if (_ecore_x_window_manage_failed)
     {
	_ecore_x_window_manage_failed = 0;
	return 0;
     }
   return 1;
}

EAPI void
ecore_x_window_container_manage(Ecore_X_Window win)
{
   XSelectInput(_ecore_x_disp, win, 
		SubstructureRedirectMask | 
		SubstructureNotifyMask);
}

EAPI void
ecore_x_window_client_manage(Ecore_X_Window win)
{
   XSelectInput(_ecore_x_disp, win, 
		PropertyChangeMask | 
//		ResizeRedirectMask |
		FocusChangeMask |
		ColormapChangeMask |
		VisibilityChangeMask |
		StructureNotifyMask
		);
   XShapeSelectInput(_ecore_x_disp, win, ShapeNotifyMask);
}

EAPI void
ecore_x_window_sniff(Ecore_X_Window win)
{
   XSelectInput(_ecore_x_disp, win,
		PropertyChangeMask |
		SubstructureNotifyMask);
}

EAPI void
ecore_x_window_client_sniff(Ecore_X_Window win)
{
   XSelectInput(_ecore_x_disp, win,
		PropertyChangeMask |
		FocusChangeMask |
		ColormapChangeMask |
		VisibilityChangeMask |
		StructureNotifyMask);
   XShapeSelectInput(_ecore_x_disp, win, ShapeNotifyMask);
}

/**
 * Retrieves the atom value associated with the given name.
 * @param  name The given name.
 * @return Associated atom value.
 */
EAPI Ecore_X_Atom    
ecore_x_atom_get(const char *name)
{
   if (!_ecore_x_disp) return 0;
   return XInternAtom(_ecore_x_disp, name, False);
}

EAPI void
ecore_x_atoms_get(const char **names, int num, Ecore_X_Atom *atoms)
{
   Atom *atoms_int;
   int i;
   
   if (!_ecore_x_disp) return;
   atoms_int = alloca(num * sizeof(Atom));
   XInternAtoms(_ecore_x_disp, (char **)names, num, False, atoms_int);
   for (i = 0; i < num; i++)
     atoms[i] = atoms_int[i];
}






EAPI int
ecore_x_window_attributes_get(Ecore_X_Window win, Ecore_X_Window_Attributes *att_ret)
{
   XWindowAttributes att;
   
   if (!XGetWindowAttributes(_ecore_x_disp, win, &att)) return 0;
   memset(att_ret, 0, sizeof(Ecore_X_Window_Attributes));
   att_ret->root = att.root;
   att_ret->x = att.x;
   att_ret->y = att.y;
   att_ret->w = att.width;
   att_ret->h = att.height;
   att_ret->border = att.border_width;
   att_ret->depth = att.depth;
   if (att.map_state != IsUnmapped) att_ret->visible = 1;
   if (att.map_state == IsViewable) att_ret->viewable = 1;
   if (att.override_redirect) att_ret->override = 1;
   if (att.class == InputOnly) att_ret->input_only = 1;
   if (att.save_under) att_ret->save_under = 1;
   att_ret->event_mask.mine = att.your_event_mask;
   att_ret->event_mask.all = att.all_event_masks;
   att_ret->event_mask.no_propagate = att.do_not_propagate_mask;
   att_ret->window_gravity = att.win_gravity;
   att_ret->pixel_gravity = att.bit_gravity;
   att_ret->colormap = att.colormap;
   att_ret->visual = att.visual;
   return 1;
}

EAPI void
ecore_x_window_save_set_add(Ecore_X_Window win)
{
   XAddToSaveSet(_ecore_x_disp, win);
}

EAPI void
ecore_x_window_save_set_del(Ecore_X_Window win)
{
   XRemoveFromSaveSet(_ecore_x_disp, win);
}

EAPI Ecore_X_Window *
ecore_x_window_children_get(Ecore_X_Window win, int *num)
{
   Ecore_X_Window     *windows = NULL;
   Window              root_ret = 0, parent_ret = 0, *children_ret = NULL;
   unsigned int        children_ret_num = 0;
   
   if (!XQueryTree(_ecore_x_disp, win, &root_ret, &parent_ret, &children_ret,
		   &children_ret_num))
     {
	return NULL;
     }
   if (children_ret)
     {
	windows = malloc(children_ret_num * sizeof(Ecore_X_Window));
	if (windows)
	  {
	     unsigned int i;
	     
	     for (i = 0; i < children_ret_num; i++)
	       windows[i] = children_ret[i];
	     *num = children_ret_num;
	  }
	XFree(children_ret);
     }
   return windows;
}

EAPI int
ecore_x_pointer_control_set(int accel_num, int accel_denom, int threshold)
{
   return XChangePointerControl(_ecore_x_disp, 1, 1, 
				accel_num, accel_denom, threshold);
}

EAPI int
ecore_x_pointer_control_get(int *accel_num, int *accel_denom, int *threshold)
{
   return XGetPointerControl(_ecore_x_disp, 
				accel_num, accel_denom, threshold);
}

EAPI int
ecore_x_pointer_mapping_set(unsigned char *map, int nmap)
{
   return XSetPointerMapping(_ecore_x_disp, map, nmap);
}

EAPI int
ecore_x_pointer_mapping_get(unsigned char *map, int nmap)
{
   return XGetPointerMapping(_ecore_x_disp, map, nmap);
}

EAPI int
ecore_x_pointer_grab(Ecore_X_Window win)
{
   if (XGrabPointer(_ecore_x_disp, win, False,
		    ButtonPressMask | ButtonReleaseMask | 
		    EnterWindowMask | LeaveWindowMask | PointerMotionMask,
		    GrabModeAsync, GrabModeAsync,
		    None, None, CurrentTime) == GrabSuccess) return 1;
   return 0;
}

EAPI int
ecore_x_pointer_confine_grab(Ecore_X_Window win)
{
   if (XGrabPointer(_ecore_x_disp, win, False,
		    ButtonPressMask | ButtonReleaseMask | 
		    EnterWindowMask | LeaveWindowMask | PointerMotionMask,
		    GrabModeAsync, GrabModeAsync,
		    win, None, CurrentTime) == GrabSuccess) return 1;
   return 0;
}

EAPI void
ecore_x_pointer_ungrab(void)
{
   XUngrabPointer(_ecore_x_disp, CurrentTime);
}

EAPI int
ecore_x_pointer_warp(Ecore_X_Window win, int x, int y)
{
   return XWarpPointer(_ecore_x_disp, None, win, 0, 0, 0, 0, x, y);
}

EAPI int
ecore_x_keyboard_grab(Ecore_X_Window win)
{
   if (XGrabKeyboard(_ecore_x_disp, win, False,
		     GrabModeAsync, GrabModeAsync,
		     CurrentTime) == GrabSuccess) return 1;
   return 0;
}

EAPI void
ecore_x_keyboard_ungrab(void)
{
   XUngrabKeyboard(_ecore_x_disp, CurrentTime);   
}

EAPI void
ecore_x_grab(void)
{
   _ecore_x_grab_count++;
   
   if (_ecore_x_grab_count == 1)
      XGrabServer(_ecore_x_disp);
}

EAPI void
ecore_x_ungrab(void)
{
   _ecore_x_grab_count--;
   if (_ecore_x_grab_count < 0)
      _ecore_x_grab_count = 0;

   if (_ecore_x_grab_count == 0)
   {
      XUngrabServer(_ecore_x_disp);
      XSync(_ecore_x_disp, False);
   }
}

int      _ecore_window_grabs_num = 0;
Window  *_ecore_window_grabs = NULL;
int    (*_ecore_window_grab_replay_func) (void *data, int event_type, void *event);
void    *_ecore_window_grab_replay_data;

EAPI void
ecore_x_passive_grab_replay_func_set(int (*func) (void *data, int event_type, void *event), void *data)
{
   _ecore_window_grab_replay_func = func;
   _ecore_window_grab_replay_data = data;
}

EAPI void
ecore_x_window_button_grab(Ecore_X_Window win, int button, 
			   Ecore_X_Event_Mask event_mask,
			   int mod, int any_mod)
{
   unsigned int        b;
   unsigned int        m;
   unsigned int        locks[8];
   int                 i, ev;
   
   b = button;
   if (b == 0) b = AnyButton;
   m = mod;
   if (any_mod) m = AnyModifier;
   locks[0] = 0;
   locks[1] = ECORE_X_LOCK_CAPS;
   locks[2] = ECORE_X_LOCK_NUM;
   locks[3] = ECORE_X_LOCK_SCROLL;
   locks[4] = ECORE_X_LOCK_CAPS   | ECORE_X_LOCK_NUM;
   locks[5] = ECORE_X_LOCK_CAPS   | ECORE_X_LOCK_SCROLL;
   locks[6] = ECORE_X_LOCK_NUM    | ECORE_X_LOCK_SCROLL;
   locks[7] = ECORE_X_LOCK_CAPS   | ECORE_X_LOCK_NUM    | ECORE_X_LOCK_SCROLL;
   ev = event_mask;
   for (i = 0; i < 8; i++)
     XGrabButton(_ecore_x_disp, b, m | locks[i],
		 win, False, ev, GrabModeSync, GrabModeAsync, None, None);
   _ecore_window_grabs_num++;
   _ecore_window_grabs = realloc(_ecore_window_grabs, 
				 _ecore_window_grabs_num * sizeof(Window));
   _ecore_window_grabs[_ecore_window_grabs_num - 1] = win;
}

void
_ecore_x_sync_magic_send(int val, Ecore_X_Window swin)
{
   XEvent xev;
   
   xev.xclient.type = ClientMessage;
   xev.xclient.serial = 0;
   xev.xclient.send_event = True;
   xev.xclient.display = _ecore_x_disp;
   xev.xclient.window = _ecore_x_private_win;
   xev.xclient.format = 32;
   xev.xclient.message_type = 27777;
   xev.xclient.data.l[0] = 0x7162534;
   xev.xclient.data.l[1] = 0x10000000 + val;
   xev.xclient.data.l[2] = swin;
   XSendEvent(_ecore_x_disp, _ecore_x_private_win, False, NoEventMask, &xev);
}

void
_ecore_x_window_grab_remove(Ecore_X_Window win)
{
   int i, shuffle = 0;
   
   if (_ecore_window_grabs_num > 0)
     {
	for (i = 0; i < _ecore_window_grabs_num; i++)
	  {
	     if (shuffle) _ecore_window_grabs[i - 1] = _ecore_window_grabs[i];
	     if ((!shuffle) && (_ecore_window_grabs[i] == win))
	       shuffle = 1;
	  }
	if (shuffle)
	  {
	     _ecore_window_grabs_num--;
	     _ecore_window_grabs = realloc(_ecore_window_grabs, 
					   _ecore_window_grabs_num * sizeof(Window));
	  }
     }
}

EAPI void
ecore_x_window_button_ungrab(Ecore_X_Window win, int button, 
			     int mod, int any_mod)
{
   unsigned int        b;
   unsigned int        m;
   unsigned int        locks[8];
   int                 i;
   
   b = button;
   if (b == 0) b = AnyButton;
   m = mod;
   if (any_mod) m = AnyModifier;
   locks[0] = 0;
   locks[1] = ECORE_X_LOCK_CAPS;
   locks[2] = ECORE_X_LOCK_NUM;
   locks[3] = ECORE_X_LOCK_SCROLL;
   locks[4] = ECORE_X_LOCK_CAPS   | ECORE_X_LOCK_NUM;
   locks[5] = ECORE_X_LOCK_CAPS   | ECORE_X_LOCK_SCROLL;
   locks[6] = ECORE_X_LOCK_NUM    | ECORE_X_LOCK_SCROLL;
   locks[7] = ECORE_X_LOCK_CAPS   | ECORE_X_LOCK_NUM    | ECORE_X_LOCK_SCROLL;
   for (i = 0; i < 8; i++)
     XUngrabButton(_ecore_x_disp, b, m | locks[i], win);
   _ecore_x_sync_magic_send(1, win);
}

int      _ecore_key_grabs_num = 0;
Window  *_ecore_key_grabs = NULL;

EAPI void
ecore_x_window_key_grab(Ecore_X_Window win, const char *key, 
			int mod, int any_mod)
{
   KeyCode             keycode = 0;
   KeySym              keysym;
   unsigned int        m;
   unsigned int        locks[8];
   int                 i;
   
   if (!strncmp(key, "Keycode-", 8))
     keycode = atoi(key + 8);
   else
     {
	keysym = XStringToKeysym(key);
	if (keysym == NoSymbol) return;
	keycode  = XKeysymToKeycode(_ecore_x_disp, XStringToKeysym(key));
     }
   if (keycode == 0) return;
   
   m = mod;
   if (any_mod) m = AnyModifier;
   locks[0] = 0;
   locks[1] = ECORE_X_LOCK_CAPS;
   locks[2] = ECORE_X_LOCK_NUM;
   locks[3] = ECORE_X_LOCK_SCROLL;
   locks[4] = ECORE_X_LOCK_CAPS   | ECORE_X_LOCK_NUM;
   locks[5] = ECORE_X_LOCK_CAPS   | ECORE_X_LOCK_SCROLL;
   locks[6] = ECORE_X_LOCK_NUM    | ECORE_X_LOCK_SCROLL;
   locks[7] = ECORE_X_LOCK_CAPS   | ECORE_X_LOCK_NUM    | ECORE_X_LOCK_SCROLL;
   for (i = 0; i < 8; i++)
     XGrabKey(_ecore_x_disp, keycode, m | locks[i],
	      win, False, GrabModeSync, GrabModeAsync);
   _ecore_key_grabs_num++;
   _ecore_key_grabs = realloc(_ecore_key_grabs,
			      _ecore_key_grabs_num * sizeof(Window));
   _ecore_key_grabs[_ecore_key_grabs_num - 1] = win;
}

void
_ecore_x_key_grab_remove(Ecore_X_Window win)
{
   int i, shuffle = 0;
   
   if (_ecore_key_grabs_num > 0)
     {
	for (i = 0; i < _ecore_key_grabs_num; i++)
	  {
	     if (shuffle) _ecore_key_grabs[i - 1] = _ecore_key_grabs[i];
	     if ((!shuffle) && (_ecore_key_grabs[i] == win))
	       shuffle = 1;
	  }
	if (shuffle)
	  {
	     _ecore_key_grabs_num--;
	     _ecore_key_grabs = realloc(_ecore_key_grabs, 
					_ecore_key_grabs_num * sizeof(Window));
	  }
     }
}

EAPI void
ecore_x_window_key_ungrab(Ecore_X_Window win, const char *key,
			  int mod, int any_mod)
{
   KeyCode             keycode = 0;
   KeySym              keysym;
   unsigned int        m;
   unsigned int        locks[8];
   int                 i;

   if (!strncmp(key, "Keycode-", 8))
     keycode = atoi(key + 8);
   else
     {
	keysym = XStringToKeysym(key);
	if (keysym == NoSymbol) return;
	keycode  = XKeysymToKeycode(_ecore_x_disp, XStringToKeysym(key));
     }
   if (keycode == 0) return;
   
   m = mod;
   if (any_mod) m = AnyModifier;
   locks[0] = 0;
   locks[1] = ECORE_X_LOCK_CAPS;
   locks[2] = ECORE_X_LOCK_NUM;
   locks[3] = ECORE_X_LOCK_SCROLL;
   locks[4] = ECORE_X_LOCK_CAPS   | ECORE_X_LOCK_NUM;
   locks[5] = ECORE_X_LOCK_CAPS   | ECORE_X_LOCK_SCROLL;
   locks[6] = ECORE_X_LOCK_NUM    | ECORE_X_LOCK_SCROLL;
   locks[7] = ECORE_X_LOCK_CAPS   | ECORE_X_LOCK_NUM    | ECORE_X_LOCK_SCROLL;
   for (i = 0; i < 8; i++)
     XUngrabKey(_ecore_x_disp, keycode, m | locks[i], win);
   _ecore_x_sync_magic_send(2, win);
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
 * @return !0 on success.
 */
EAPI int
ecore_x_client_message32_send(Ecore_X_Window win, Ecore_X_Atom type,
			      Ecore_X_Event_Mask mask,
			      long d0, long d1, long d2, long d3, long d4)
{
    XEvent xev;

    xev.xclient.window = win;
    xev.xclient.type = ClientMessage;
    xev.xclient.message_type = type;
    xev.xclient.format = 32;
    xev.xclient.data.l[0] = d0;
    xev.xclient.data.l[1] = d1;
    xev.xclient.data.l[2] = d2;
    xev.xclient.data.l[3] = d3;
    xev.xclient.data.l[4] = d4;

    return XSendEvent(_ecore_x_disp, win, False, mask, &xev);
}

/**
 * Send client message with given type and format 8.
 *
 * @param win     The window the message is sent to.
 * @param type    The client message type.
 * @param data    Data to be sent.
 * @param len     Number of data bytes, max 20.
 *
 * @return !0 on success.
 */
EAPI int
ecore_x_client_message8_send(Ecore_X_Window win, Ecore_X_Atom type,
			     const void *data, int len)
{
    XEvent xev;

    xev.xclient.window = win;
    xev.xclient.type = ClientMessage;
    xev.xclient.message_type = type;
    xev.xclient.format = 8;
    if (len > 20)
        len = 20;
    memcpy(xev.xclient.data.b, data, len);
    memset(xev.xclient.data.b + len, 0, 20 - len);

    return XSendEvent(_ecore_x_disp, win, False, NoEventMask, &xev);
}

EAPI int
ecore_x_mouse_move_send(Ecore_X_Window win, int x, int y)
{
   XEvent xev;
   XWindowAttributes att;
   Window tw;
   int rx, ry;

   XGetWindowAttributes(_ecore_x_disp, win, &att);
   XTranslateCoordinates(_ecore_x_disp, win, att.root, x, y, &rx, &ry, &tw);
   xev.xmotion.type = MotionNotify;
   xev.xmotion.window = win;
   xev.xmotion.root = att.root;
   xev.xmotion.subwindow = win;
   xev.xmotion.time = _ecore_x_event_last_time;
   xev.xmotion.x = x;
   xev.xmotion.y = y;
   xev.xmotion.x_root = rx;
   xev.xmotion.y_root = ry;
   xev.xmotion.state = 0;
   xev.xmotion.is_hint = 0;
   xev.xmotion.same_screen = 1;
   return XSendEvent(_ecore_x_disp, win, True, PointerMotionMask, &xev);
}

EAPI int
ecore_x_mouse_down_send(Ecore_X_Window win, int x, int y, int b)
{
   XEvent xev;
   XWindowAttributes att;
   Window tw;
   int rx, ry;

   XGetWindowAttributes(_ecore_x_disp, win, &att);
   XTranslateCoordinates(_ecore_x_disp, win, att.root, x, y, &rx, &ry, &tw);
   xev.xbutton.type = ButtonPress;
   xev.xbutton.window = win;
   xev.xbutton.root = att.root;
   xev.xbutton.subwindow = win;
   xev.xbutton.time = _ecore_x_event_last_time;
   xev.xbutton.x = x;
   xev.xbutton.y = y;
   xev.xbutton.x_root = rx;
   xev.xbutton.y_root = ry;
   xev.xbutton.state = 1 << b;
   xev.xbutton.button = b;
   xev.xbutton.same_screen = 1;
   return XSendEvent(_ecore_x_disp, win, True, ButtonPressMask, &xev);
}

EAPI int
ecore_x_mouse_up_send(Ecore_X_Window win, int x, int y, int b)
{
   XEvent xev;
   XWindowAttributes att;
   Window tw;
   int rx, ry;

   XGetWindowAttributes(_ecore_x_disp, win, &att);
   XTranslateCoordinates(_ecore_x_disp, win, att.root, x, y, &rx, &ry, &tw);
   xev.xbutton.type = ButtonRelease;
   xev.xbutton.window = win;
   xev.xbutton.root = att.root;
   xev.xbutton.subwindow = win;
   xev.xbutton.time = _ecore_x_event_last_time;
   xev.xbutton.x = x;
   xev.xbutton.y = y;
   xev.xbutton.x_root = rx;
   xev.xbutton.y_root = ry;
   xev.xbutton.state = 0;
   xev.xbutton.button = b;
   xev.xbutton.same_screen = 1;
   return XSendEvent(_ecore_x_disp, win, True, ButtonReleaseMask, &xev);
}

EAPI void
ecore_x_focus_reset(void)
{
   XSetInputFocus(_ecore_x_disp, PointerRoot, RevertToPointerRoot, CurrentTime);
}

EAPI void
ecore_x_events_allow_all(void)
{
   XAllowEvents(_ecore_x_disp, AsyncBoth, CurrentTime);
}

EAPI void
ecore_x_pointer_last_xy_get(int *x, int *y)
{
   if (x) *x = _ecore_x_event_last_root_x;
   if (y) *y = _ecore_x_event_last_root_y;
}

EAPI void
ecore_x_pointer_xy_get(Ecore_X_Window win, int *x, int *y)
{
   Window rwin, cwin;
   int rx, ry, wx, wy, ret;
   unsigned int mask;
   
   ret = XQueryPointer(_ecore_x_disp, win, &rwin, &cwin, &rx, &ry, &wx, &wy, &mask);
   if (!ret) wx = wy = -1;
   if (x) *x = wx;
   if (y) *y = wy;
}
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
