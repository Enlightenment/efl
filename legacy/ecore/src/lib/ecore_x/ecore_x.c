#include "Ecore.h"
#include "ecore_x_private.h"
#include "Ecore_X.h"

static int _ecore_x_fd_handler(void *data, Ecore_Fd_Handler *fd_handler);
static int _ecore_x_fd_handler_buf(void *data, Ecore_Fd_Handler *fd_handler);
static int _ecore_x_key_mask_get(KeySym sym);
static void *_ecore_x_event_filter_start(void *data);
static int   _ecore_x_event_filter_filter(void *data, void *loop_data,int type, void *event);
static void  _ecore_x_event_filter_end(void *data, void *loop_data);

static Ecore_Fd_Handler *_ecore_x_fd_handler_handle = NULL;
static Ecore_Event_Filter *_ecore_x_filter_handler = NULL;
static int _ecore_x_event_shape_id = 0;
static int _ecore_x_event_handlers_num = 0;
static void (**_ecore_x_event_handlers) (XEvent * event) = NULL;

static int _ecore_x_init_count = 0;

Display *_ecore_x_disp = NULL;
double   _ecore_x_double_click_time = 0.25;
Time     _ecore_x_event_last_time = 0;
Window   _ecore_x_event_last_win = 0;
int      _ecore_x_event_last_root_x = 0;
int      _ecore_x_event_last_root_y = 0;

/*
 * ICCCM and Motif hints.
 */
Atom     _ecore_x_atom_wm_delete_window = 0;
Atom     _ecore_x_atom_wm_take_focus = 0;
Atom     _ecore_x_atom_wm_protocols = 0;
Atom     _ecore_x_atom_wm_class = 0;
Atom     _ecore_x_atom_wm_name = 0;
Atom     _ecore_x_atom_wm_command = 0;
Atom     _ecore_x_atom_wm_icon_name = 0;
Atom     _ecore_x_atom_wm_client_machine = 0;
Atom     _ecore_x_atom_motif_wm_hints = 0;
Atom     _ecore_x_atom_win_layer = 0;
Atom     _ecore_x_atom_selection_primary = 0;
Atom     _ecore_x_atom_selection_secondary = 0;
Atom     _ecore_x_atom_selection_clipboard = 0;
Atom     _ecore_x_atom_selection_prop_primary = 0;
Atom     _ecore_x_atom_selection_prop_secondary = 0;
Atom     _ecore_x_atom_selection_prop_clipboard = 0;

/*
 * Root window NetWM hints.
 */
Atom     _ecore_x_atom_net_supported = 0;
Atom     _ecore_x_atom_net_client_list = 0;
Atom     _ecore_x_atom_net_number_of_desktops = 0;
Atom     _ecore_x_atom_net_desktop_geometry = 0;
Atom     _ecore_x_atom_net_desktop_viewport = 0;
Atom     _ecore_x_atom_net_current_desktop = 0;
Atom     _ecore_x_atom_net_desktop_names = 0;
Atom     _ecore_x_atom_net_active_window = 0;
Atom     _ecore_x_atom_net_workarea = 0;
Atom     _ecore_x_atom_net_supporting_wm_check = 0;
Atom     _ecore_x_atom_net_virtual_roots = 0;
Atom     _ecore_x_atom_net_desktop_layout = 0;
Atom     _ecore_x_atom_net_showing_desktop = 0;

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

Atom     _ecore_x_atom_file_name = 0;
Atom     _ecore_x_atom_string = 0;
Atom     _ecore_x_atom_text = 0;
Atom     _ecore_x_atom_utf8_string = 0;
Atom     _ecore_x_atom_compound_text = 0;

Atom     _ecore_x_atoms_wm_protocols[ECORE_X_WM_PROTOCOL_NUM] = {0};

int ECORE_X_EVENT_KEY_DOWN = 0;
int ECORE_X_EVENT_KEY_UP = 0;
int ECORE_X_EVENT_MOUSE_BUTTON_DOWN = 0;
int ECORE_X_EVENT_MOUSE_BUTTON_UP = 0;
int ECORE_X_EVENT_MOUSE_MOVE = 0;
int ECORE_X_EVENT_MOUSE_IN = 0;
int ECORE_X_EVENT_MOUSE_OUT = 0;
int ECORE_X_EVENT_MOUSE_WHEEL = 0;
int ECORE_X_EVENT_WINDOW_FOCUS_IN = 0;
int ECORE_X_EVENT_WINDOW_FOCUS_OUT = 0;
int ECORE_X_EVENT_WINDOW_KEYMAP = 0;
int ECORE_X_EVENT_WINDOW_DAMAGE = 0;
int ECORE_X_EVENT_WINDOW_VISIBILITY_CHANGE = 0;
int ECORE_X_EVENT_WINDOW_CREATE = 0;
int ECORE_X_EVENT_WINDOW_DESTROY = 0;
int ECORE_X_EVENT_WINDOW_HIDE = 0;
int ECORE_X_EVENT_WINDOW_SHOW = 0;
int ECORE_X_EVENT_WINDOW_SHOW_REQUEST = 0;
int ECORE_X_EVENT_WINDOW_REPARENT = 0;
int ECORE_X_EVENT_WINDOW_CONFIGURE = 0;
int ECORE_X_EVENT_WINDOW_CONFIGURE_REQUEST = 0;
int ECORE_X_EVENT_WINDOW_GRAVITY = 0;
int ECORE_X_EVENT_WINDOW_RESIZE_REQUEST = 0;
int ECORE_X_EVENT_WINDOW_STACK = 0;
int ECORE_X_EVENT_WINDOW_STACK_REQUEST = 0;
int ECORE_X_EVENT_WINDOW_PROPERTY = 0;
int ECORE_X_EVENT_WINDOW_COLORMAP = 0;
int ECORE_X_EVENT_WINDOW_MAPPING = 0;
int ECORE_X_EVENT_SELECTION_CLEAR = 0;
int ECORE_X_EVENT_SELECTION_REQUEST = 0;
int ECORE_X_EVENT_SELECTION_NOTIFY = 0;
int ECORE_X_EVENT_CLIENT_MESSAGE = 0;
int ECORE_X_EVENT_WINDOW_SHAPE = 0;

int ECORE_X_EVENT_WINDOW_DELETE_REQUEST = 0;
int ECORE_X_EVENT_WINDOW_PROP_TITLE_CHANGE = 0;
int ECORE_X_EVENT_WINDOW_PROP_VISIBLE_TITLE_CHANGE = 0;
int ECORE_X_EVENT_WINDOW_PROP_NAME_CLASS_CHANGE = 0;
int ECORE_X_EVENT_WINDOW_PROP_ICON_NAME_CHANGE = 0;
int ECORE_X_EVENT_WINDOW_PROP_VISIBLE_ICON_NAME_CHANGE = 0;
int ECORE_X_EVENT_WINDOW_PROP_CLIENT_MACHINE_CHANGE = 0;
int ECORE_X_EVENT_WINDOW_PROP_PID_CHANGE = 0;
int ECORE_X_EVENT_WINDOW_PROP_DESKTOP_CHANGE = 0;

int ECORE_X_MODIFIER_SHIFT = 0;
int ECORE_X_MODIFIER_CTRL = 0;
int ECORE_X_MODIFIER_ALT = 0;
int ECORE_X_MODIFIER_WIN = 0;

int ECORE_X_LOCK_SCROLL = 0;
int ECORE_X_LOCK_NUM = 0;
int ECORE_X_LOCK_CAPS = 0;

/**
 * Init the X display connection.
 * @param name Display target name
 * @return greater than 1 on success, 0 on failure
 * 
 * Set up an X display connection and event handlers for event processing
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
 */
int
ecore_x_init(const char *name)
{
   int shape_base = 0;
   int shape_err_base = 0;
   
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
   if (_ecore_x_event_shape_id >= LASTEvent)
     _ecore_x_event_handlers_num = _ecore_x_event_shape_id + 1;
   _ecore_x_event_handlers = calloc(_ecore_x_event_handlers_num, sizeof(void *));
   if (!_ecore_x_event_handlers)
     {
        XCloseDisplay(_ecore_x_disp);
	_ecore_x_fd_handler_handle = NULL;
	_ecore_x_disp = NULL;
	return 0;	
     }
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
   _ecore_x_event_handlers[MappingNotify]    = _ecore_x_event_handle_mapping_notify;
   _ecore_x_event_handlers[ClientMessage]    = _ecore_x_event_handle_client_message;
   if (_ecore_x_event_shape_id)
     _ecore_x_event_handlers[_ecore_x_event_shape_id] = _ecore_x_event_handle_shape_change;
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
	
	ECORE_X_EVENT_WINDOW_DELETE_REQUEST                = ecore_event_type_new();
	ECORE_X_EVENT_WINDOW_PROP_TITLE_CHANGE             = ecore_event_type_new();
	ECORE_X_EVENT_WINDOW_PROP_VISIBLE_TITLE_CHANGE     = ecore_event_type_new();
        ECORE_X_EVENT_WINDOW_PROP_NAME_CLASS_CHANGE        = ecore_event_type_new();
	ECORE_X_EVENT_WINDOW_PROP_ICON_NAME_CHANGE         = ecore_event_type_new();
	ECORE_X_EVENT_WINDOW_PROP_VISIBLE_ICON_NAME_CHANGE = ecore_event_type_new();
	ECORE_X_EVENT_WINDOW_PROP_CLIENT_MACHINE_CHANGE    = ecore_event_type_new();
	ECORE_X_EVENT_WINDOW_PROP_PID_CHANGE               = ecore_event_type_new();
	ECORE_X_EVENT_WINDOW_PROP_DESKTOP_CHANGE               = ecore_event_type_new();
     }
   
   ECORE_X_MODIFIER_SHIFT = _ecore_x_key_mask_get(XK_Shift_L);
   ECORE_X_MODIFIER_CTRL  = _ecore_x_key_mask_get(XK_Control_L);
   ECORE_X_MODIFIER_ALT   = _ecore_x_key_mask_get(XK_Alt_L);
   ECORE_X_MODIFIER_WIN   = _ecore_x_key_mask_get(XK_Super_L);
   if (!ECORE_X_MODIFIER_WIN) 
     ECORE_X_MODIFIER_WIN = _ecore_x_key_mask_get(XK_Meta_L);   
   
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
   _ecore_x_filter_handler = ecore_event_filter_add(_ecore_x_event_filter_start, _ecore_x_event_filter_filter, _ecore_x_event_filter_end, NULL);
   _ecore_x_atom_wm_delete_window         = XInternAtom(_ecore_x_disp, "WM_DELETE_WINDOW", False);
   _ecore_x_atom_wm_take_focus            = XInternAtom(_ecore_x_disp, "WM_TAKE_FOCUS", False);
   _ecore_x_atom_wm_protocols             = XInternAtom(_ecore_x_disp, "WM_PROTOCOLS", False);
   _ecore_x_atom_wm_class                 = XInternAtom(_ecore_x_disp, "WM_CLASS", False);
   _ecore_x_atom_wm_name                  = XInternAtom(_ecore_x_disp, "WM_NAME", False);
   _ecore_x_atom_wm_command               = XInternAtom(_ecore_x_disp, "WM_COMMAND", False);
   _ecore_x_atom_wm_icon_name             = XInternAtom(_ecore_x_disp, "WM_ICON_NAME", False);
   _ecore_x_atom_wm_client_machine        = XInternAtom(_ecore_x_disp, "WM_CLIENT_MACHINE", False);
   _ecore_x_atom_motif_wm_hints           = XInternAtom(_ecore_x_disp, "_MOTIF_WM_HINTS", False);
   _ecore_x_atom_win_layer                = XInternAtom(_ecore_x_disp, "_WIN_LAYER", False);
   /* This is just to be anal about naming conventions */
   _ecore_x_atom_selection_primary        = XA_PRIMARY;
   _ecore_x_atom_selection_secondary      = XA_SECONDARY;
   _ecore_x_atom_selection_clipboard      = XInternAtom(_ecore_x_disp, "CLIPBOARD", False);
   _ecore_x_atom_selection_prop_primary   = XInternAtom(_ecore_x_disp, "_ECORE_SELECTION_PRIMARY", False);
   _ecore_x_atom_selection_prop_secondary = XInternAtom(_ecore_x_disp, "_ECORE_SELECTION_SECONDARY", False);
   _ecore_x_atom_selection_prop_clipboard = XInternAtom(_ecore_x_disp, "_ECORE_SELECTION_CLIPBOARD", False);
   
   _ecore_x_atom_net_current_desktop      = XInternAtom(_ecore_x_disp, "_NET_CURRENT_DESKTOP", False);
   _ecore_x_atom_net_wm_name              = XInternAtom(_ecore_x_disp, "_NET_WM_NAME", False);
   _ecore_x_atom_net_wm_visible_name      = XInternAtom(_ecore_x_disp, "_NET_WM_VISIBLE_NAME", False);
   _ecore_x_atom_net_wm_icon_name         = XInternAtom(_ecore_x_disp, "_NET_WM_ICON_NAME", False);
   _ecore_x_atom_net_wm_visible_icon_name = XInternAtom(_ecore_x_disp, "_NET_WM_VISIBLE_ICON_NAME", False);
   _ecore_x_atom_net_wm_desktop           = XInternAtom(_ecore_x_disp, "_NET_WM_DESKTOP", False);
   _ecore_x_atom_net_wm_window_type       = XInternAtom(_ecore_x_disp, "_NET_WM_WINDOW_TYPE", False);
   _ecore_x_atom_net_wm_state             = XInternAtom(_ecore_x_disp, "_NET_WM_STATE", False);
   _ecore_x_atom_net_wm_allowed_actions   = XInternAtom(_ecore_x_disp, "_NET_WM_ALLOWED_ACTIONS", False);
   _ecore_x_atom_net_wm_strut             = XInternAtom(_ecore_x_disp, "_NET_WM_STRUT", False);
   _ecore_x_atom_net_wm_strut_partial     = XInternAtom(_ecore_x_disp, "_NET_WM_STRUT_PARTIAL", False);
   _ecore_x_atom_net_wm_icon_geometry     = XInternAtom(_ecore_x_disp, "_NET_WM_ICON_GEOMETRY", False);
   _ecore_x_atom_net_wm_icon              = XInternAtom(_ecore_x_disp, "_NET_WM_ICON", False);
   _ecore_x_atom_net_wm_pid               = XInternAtom(_ecore_x_disp, "_NET_WM_PID", False);
   _ecore_x_atom_net_wm_user_time         = XInternAtom(_ecore_x_disp, "_NET_WM_USER_TIME", False);

   _ecore_x_atom_net_wm_window_type_desktop = XInternAtom(_ecore_x_disp, "_NET_WM_WINDOW_TYPE_DESKTOP", False);
   _ecore_x_atom_net_wm_window_type_dock    = XInternAtom(_ecore_x_disp, "_NET_WM_WINDOW_TYPE_DOCK", False);
   _ecore_x_atom_net_wm_window_type_toolbar = XInternAtom(_ecore_x_disp, "_NET_WM_WINDOW_TYPE_TOOLBAR", False);
   _ecore_x_atom_net_wm_window_type_menu    = XInternAtom(_ecore_x_disp, "_NET_WM_WINDOW_TYPE_MENU", False);
   _ecore_x_atom_net_wm_window_type_utility = XInternAtom(_ecore_x_disp, "_NET_WM_WINDOW_TYPE_UTILITY", False);
   _ecore_x_atom_net_wm_window_type_splash  = XInternAtom(_ecore_x_disp, "_NET_WM_WINDOW_TYPE_SPLASH", False);
   _ecore_x_atom_net_wm_window_type_dialog  = XInternAtom(_ecore_x_disp, "_NET_WM_WINDOW_TYPE_DIALOG", False);
   _ecore_x_atom_net_wm_window_type_normal  = XInternAtom(_ecore_x_disp, "_NET_WM_WINDOW_TYPE_NORMAL", False);

   _ecore_x_atom_net_wm_state_modal          = XInternAtom(_ecore_x_disp, "_NET_WM_STATE_MODAL", False);
   _ecore_x_atom_net_wm_state_sticky         = XInternAtom(_ecore_x_disp, "_NET_WM_STATE_STICKY", False);
   _ecore_x_atom_net_wm_state_maximized_vert = XInternAtom(_ecore_x_disp, "_NET_WM_STATE_MAXIMIZED_VERT", False);
   _ecore_x_atom_net_wm_state_maximized_horz = XInternAtom(_ecore_x_disp, "_NET_WM_STATE_MAXIMIZED_HORZ", False);
   _ecore_x_atom_net_wm_state_shaded         = XInternAtom(_ecore_x_disp, "_NET_WM_STATE_SHADED", False);
   _ecore_x_atom_net_wm_state_skip_taskbar   = XInternAtom(_ecore_x_disp, "_NET_WM_STATE_SKIP_TASKBAR", False);
   _ecore_x_atom_net_wm_state_skip_pager     = XInternAtom(_ecore_x_disp, "_NET_WM_STATE_SKIP_PAGER", False);
   _ecore_x_atom_net_wm_state_hidden         = XInternAtom(_ecore_x_disp, "_NET_WM_STATE_HIDDEN", False);
   _ecore_x_atom_net_wm_state_fullscreen     = XInternAtom(_ecore_x_disp, "_NET_WM_STATE_FULLSCREEN", False);
   _ecore_x_atom_net_wm_state_above          = XInternAtom(_ecore_x_disp, "_NET_WM_STATE_ABOVE", False);
   _ecore_x_atom_net_wm_state_below          = XInternAtom(_ecore_x_disp, "_NET_WM_STATE_BELOW", False);
   _ecore_x_atom_net_wm_window_opacity       = XInternAtom(_ecore_x_disp, "_NET_WM_WINDOW_OPACITY", False);

   _ecore_x_atom_compound_text      = XInternAtom(_ecore_x_disp, "COMPOUND_TEXT", False);
   _ecore_x_atom_utf8_string        = XInternAtom(_ecore_x_disp, "UTF8_STRING", False);
   _ecore_x_atom_file_name          = XInternAtom(_ecore_x_disp, "FILE_NAME", False);
   _ecore_x_atom_string             = XInternAtom(_ecore_x_disp, "STRING", False);
   _ecore_x_atom_text               = XInternAtom(_ecore_x_disp, "TEXT", False);

   _ecore_x_atoms_wm_protocols[ECORE_X_WM_PROTOCOL_DELETE_REQUEST] = _ecore_x_atom_wm_delete_window;
   _ecore_x_atoms_wm_protocols[ECORE_X_WM_PROTOCOL_TAKE_FOCUS] = _ecore_x_atom_wm_take_focus;

   _ecore_x_selection_data_initialize();
   
   _ecore_x_init_count++;
   return _ecore_x_init_count;
}

/**
 * Shut down the X syystem.
 * @return The number of times ecore X has left to be shut down
 * 
 * This closes the X display connection and removes the handlers for it.
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
 */
int
ecore_x_shutdown(void)
{
   _ecore_x_init_count--;
   if (_ecore_x_init_count > 0) return _ecore_x_init_count;
   if (!_ecore_x_disp) return _ecore_x_init_count;
   XCloseDisplay(_ecore_x_disp);
   free(_ecore_x_event_handlers);
   ecore_main_fd_handler_del(_ecore_x_fd_handler_handle);
   ecore_event_filter_del(_ecore_x_filter_handler);
   _ecore_x_fd_handler_handle = NULL;
   _ecore_x_filter_handler = NULL;
   _ecore_x_disp = NULL;
   _ecore_x_event_handlers = NULL;
   if (_ecore_x_init_count < 0) _ecore_x_init_count = 0;
   return _ecore_x_init_count;
}

/**
 * Get the X display pointer handle.
 * @return The X Display handle
 * 
 * This returns the X Display handle used for the X connection.
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
 */
Ecore_X_Display *
ecore_x_display_get(void)
{
   return (Ecore_X_Display *)_ecore_x_disp;
}

/**
 * Get the X display fd.
 * @return The X Display fd
 * 
 * This returns the X Display file descriptor.
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
 */
int
ecore_x_fd_get(void)
{
   return ConnectionNumber(_ecore_x_disp);
}

/**
 * Set the timeout for double/triple click to be flagged.
 * @param t The time in seconds
 * 
 * This sets the time between clicks before the double_click flag is set in a 
 * button down event. If 3 clicks occur within double this time then the 
 * triple_click flag is also set.
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
 */
void
ecore_x_double_click_time_set(double t)
{
   if (t < 0.0) t = 0.0;
   _ecore_x_double_click_time = t;
}

/**
 * Get the double/triple click timeout.
 * @return The timeout for double clicks in seconds
 * 
 * This returns the tiemout that can be set by ecore_x_double_click_time_set()
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
 */
double
ecore_x_double_click_time_get(void)
{
   return _ecore_x_double_click_time;
}

/**
 * Flush the command buffer.
 * 
 * This flushes the x command buffer
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
 */
void
ecore_x_flush(void)
{
   XFlush(_ecore_x_disp);
}

/**
 * Sync with the server.
 * 
 * This flushes the command buffer and waits for a round trip from the server
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
 */
void
ecore_x_sync(void)
{
   XSync(_ecore_x_disp, False);
}

/**
 * Kill all clients under a specified root window.
 * @param root The root window whose children will be killed.
 *
 * This function will start from the given root window and kill the owner of
 * every child window under it. To kill all clients connected to an X server,
 * use the ecore_x_window_root_list() function to obtain an array of root
 * windows, and then pass each window to this function.
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
 */
void
ecore_x_killall(Ecore_X_Window root)
{
   int screens;
   int i, j;
   
   XGrabServer(_ecore_x_disp);
   screens = ScreenCount(_ecore_x_disp);

   /* Tranverse window tree starting from root, and drag each
    * before the firing squad */
   for (i = 0; i < screens; ++i)
   {
      Window root_r;
      Window parent_r;
      Window *children_r = NULL;
      int num_children = 0;

      while (XQueryTree(_ecore_x_disp, root, &root_r, &parent_r,
               &children_r, &num_children) && num_children > 0)
      {
         for (j = 0; j < num_children; ++j)
         {
            XKillClient(_ecore_x_disp, children_r[j]);
         }

         XFree(children_r);
      }
   }

   XUngrabServer(_ecore_x_disp);
   XSync(_ecore_x_disp, False);
}

static int
_ecore_x_fd_handler(void *data, Ecore_Fd_Handler *fd_handler)
{
   Display *d;
   
   d = data;
   while (XPending(d))
     {
	XEvent ev;
	
	XNextEvent(d, &ev);
	if ((ev.type >= 0) && (ev.type < _ecore_x_event_handlers_num))
	  {
	     if (_ecore_x_event_handlers[ev.type])
	       _ecore_x_event_handlers[ev.type] (&ev);
	  }
     }
   return 1;
}

static int
_ecore_x_fd_handler_buf(void *data, Ecore_Fd_Handler *fd_handler)
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
   KeyCode             nl;
   int                 i;
   const int           masks[8] = 
     {
	ShiftMask, LockMask, ControlMask, 
	  Mod1Mask, Mod2Mask, Mod3Mask, Mod4Mask, Mod5Mask
     };
   
   mod = XGetModifierMapping(_ecore_x_disp);
   nl = XKeysymToKeycode(_ecore_x_disp, sym);
   if ((mod) && (mod->max_keypermod > 0))
     {
	for (i = 0; i < (8 * mod->max_keypermod); i++)
	  {
	     if ((nl) && (mod->modifiermap[i] == nl))
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

typedef struct _Ecore_X_Filter_Data Ecore_X_Filter_Data;

struct _Ecore_X_Filter_Data
{
   int last_event_type;
};

static void *
_ecore_x_event_filter_start(void *data)
{
   Ecore_X_Filter_Data *filter_data;
   
   filter_data = calloc(1, sizeof(Ecore_X_Filter_Data));
   return filter_data;
}

static int
_ecore_x_event_filter_filter(void *data, void *loop_data,int type, void *event)
{
   Ecore_X_Filter_Data *filter_data;
   
   filter_data = loop_data;
   if (!filter_data) return 1;
   if (type == ECORE_X_EVENT_MOUSE_MOVE)
     {
	if ((filter_data->last_event_type) == ECORE_X_EVENT_MOUSE_MOVE) 
	  {
	     filter_data->last_event_type = type;
	     return 0;
	  }
     }
   filter_data->last_event_type = type;
   return 1;
}

static void
_ecore_x_event_filter_end(void *data, void *loop_data)
{
   Ecore_X_Filter_Data *filter_data;
   
   filter_data = loop_data;
   if (filter_data) free(filter_data);
}






















/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/* FIXME: these funcs need categorising */
/*****************************************************************************/

Ecore_X_Window *
ecore_x_window_root_list(int *num_ret)
{
   int num, i;
   Ecore_X_Window *roots;
   
   if (!num_ret) return NULL;
   *num_ret = 0;
   num = ScreenCount(_ecore_x_disp);
   roots = malloc(num * sizeof(Window));
   if (!roots) return NULL;
   *num_ret = num;
   for (i = 0; i < num; i++) roots[i] = RootWindow(_ecore_x_disp, i);
   return roots;
}

static void _ecore_x_window_manage_error(void *data);

static int _ecore_x_window_manage_failed = 0;
static void
_ecore_x_window_manage_error(void *data)
{
   if ((ecore_x_error_request_get() == X_ChangeWindowAttributes) &&
       (ecore_x_error_code_get() == BadAccess))
     _ecore_x_window_manage_failed = 1;
}

int
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

void
ecore_x_window_container_manage(Ecore_X_Window win)
{
   XSelectInput(_ecore_x_disp, win, 
		ResizeRedirectMask |
		SubstructureRedirectMask | 
		SubstructureNotifyMask);
}

void
ecore_x_window_client_manage(Ecore_X_Window win)
{
   XSelectInput(_ecore_x_disp, win, 
		PropertyChangeMask | 
		ResizeRedirectMask |
		FocusChangeMask |
		ColormapChangeMask |
		VisibilityChangeMask |
		StructureNotifyMask
		);
}

void
ecore_x_window_sniff(Ecore_X_Window win)
{
   XSelectInput(_ecore_x_disp, win,
		PropertyChangeMask |
		SubstructureNotifyMask);
}

void
ecore_x_window_client_sniff(Ecore_X_Window win)
{
   XSelectInput(_ecore_x_disp, win,
		PropertyChangeMask |
		StructureNotifyMask |
		FocusChangeMask |
		ColormapChangeMask |
		VisibilityChangeMask |
		StructureNotifyMask);
}

Ecore_X_Atom    
ecore_x_atom_get(char *name)
{
   if (!_ecore_x_disp) return 0;
   return XInternAtom(_ecore_x_disp, name, False);
}

void
ecore_x_reply_del(Ecore_X_Reply *reply)
{
   ecore_job_del(reply->job);
   reply->reply_data_free(reply->reply_data);
   free(reply);
}

static void _ecore_x_reply_call(void *data);
static void
_ecore_x_reply_call(void *data)
{
   Ecore_X_Reply *reply;
   
   reply = data;
   reply->func(reply->data, reply, reply->reply_data);
   reply->reply_data_free(reply->reply_data);
   free(reply);
}

Ecore_X_Reply *
ecore_x_window_attributes_fetch(Ecore_X_Window win,
				void (*func) (void *data, Ecore_X_Reply *reply, void *reply_data),
				void *data)
{
   Ecore_X_Reply *reply;
   Ecore_X_Reply_Window_Attributes *reply_data;
   
   reply = calloc(1, sizeof(Ecore_X_Reply));
   if (!reply) return NULL;
   reply_data = calloc(1, sizeof(Ecore_X_Reply_Window_Attributes));
   if (!reply_data)
     {
	free(reply);
	return NULL;
     }
   reply->reply_data = reply_data;
   reply->reply_data_free = free;
   reply->func = func;
   reply->data = data;
   
     {
	XWindowAttributes att;
	
	if (!XGetWindowAttributes(_ecore_x_disp, win, &att))
	  {
	     reply->reply_data_free(reply->reply_data);
	     free(reply);
	     return NULL;
	  }
	reply_data->root = att.root;
	reply_data->x = att.x;
	reply_data->y = att.y;
	reply_data->w = att.width;
	reply_data->h = att.height;
	reply_data->border = att.border_width;
	reply_data->depth = att.depth;
	if (att.map_state != IsUnmapped) reply_data->visible = 1;
	if (att.map_state == IsViewable) reply_data->viewable = 1;
	if (att.override_redirect) reply_data->override = 1;
	if (att.class == InputOnly) reply_data->input_only = 1;
	if (att.save_under) reply_data->save_under = 1;
	reply_data->event_mask.mine = att.your_event_mask;
	reply_data->event_mask.all = att.your_event_mask;
	reply_data->event_mask.no_propagate = att.do_not_propagate_mask;
     }
   reply->job = ecore_job_add(_ecore_x_reply_call, reply);
   if (!reply->job)
     {
	reply->reply_data_free(reply->reply_data);
	free(reply);
	return NULL;
     }
   return reply;
}

Ecore_X_Cursor
ecore_x_cursor_new(Ecore_X_Window win, int *pixels, int w, int h, int hot_x, int hot_y)
{
   XColor c1, c2;
   Cursor c;
   Pixmap pmap, mask;
   GC gc;
   XGCValues gcv;
   XImage *xim;
   unsigned int *pix;
   int fr, fg, fb, br, bg, bb;
   int brightest = 0;
   int darkest = 255 * 3;
   int x, y;
   const int dither[2][2] =
     { 
	  {0, 2},
	  {3, 1}
     };

   
   pmap = XCreatePixmap(_ecore_x_disp, win, w, h, 1);
   mask = XCreatePixmap(_ecore_x_disp, win, w, h, 1);
   xim = XCreateImage(_ecore_x_disp, 
		      DefaultVisual(_ecore_x_disp, 0), 
		      1, ZPixmap, 0, NULL, w, h, 32, 0);
   xim->data = malloc(xim->bytes_per_line * xim->height);
   
   fr = 0x00; fg = 0x00; fb = 0x00;
   br = 0xff; bg = 0xff; bb = 0xff;
   pix = pixels;
   for (y = 0; y < h; y++)
     {
	for (x = 0; x < w; x++)
	  {
	     int r, g, b, a;
	     
	     a = (pix[0] >> 24) & 0xff;
	     r = (pix[0] >> 16) & 0xff;
	     g = (pix[0] >> 8 ) & 0xff;
	     b = (pix[0]      ) & 0xff;
	     if (a > 0)
	       {
		  if ((r + g + b) > brightest)
		    {
		       brightest = r + g + b; 
		       br = r;
		       bg = g;
		       bb = b;
		    }
		  if ((r + g + b) < darkest)
		    {
		       darkest = r + g + b; 
		       fr = r;
		       fg = g;
		       fb = b;
		    }
	       }
	     pix++;
	  }
     }
   
   pix = pixels;
   for (y = 0; y < h; y++)
     {
	for (x = 0; x < w; x++)
	  {
	     int v;
	     int r, g, b;
	     int d1, d2;
	     
	     r = (pix[0] >> 16) & 0xff;
	     g = (pix[0] >> 8 ) & 0xff;
	     b = (pix[0]      ) & 0xff;
	     d1 = 
	       ((r - fr) * (r - fr)) +
	       ((g - fg) * (g - fg)) +
	       ((b - fb) * (b - fb));
	     d2 = 
	       ((r - br) * (r - br)) +
	       ((g - bg) * (g - bg)) +
	       ((b - bb) * (b - bb));
	     v = (((d2 * 255) / (d1 + d2)) * 5) / 256;
	     if (v > dither[x & 0x1][y & 0x1]) v = 1;
	     else v = 0;
	     XPutPixel(xim, x, y, v);
	     pix++;
	  }
     }
   gc = XCreateGC(_ecore_x_disp, pmap, 0, &gcv);
   XPutImage(_ecore_x_disp, pmap, gc, xim, 0, 0, 0, 0, w, h);   
   XFreeGC(_ecore_x_disp, gc);

   pix = pixels;
   for (y = 0; y < h; y++)
     {
	for (x = 0; x < w; x++)
	  {
	     int v;
	     
	     v = (((pix[0] >> 24) & 0xff) * 5) / 256;
	     if (v > dither[x & 0x1][y & 0x1]) v = 1;
	     else v = 0;
	     XPutPixel(xim, x, y, v);
	     pix++;
	  }
     }
   gc = XCreateGC(_ecore_x_disp, mask, 0, &gcv);
   XPutImage(_ecore_x_disp, mask, gc, xim, 0, 0, 0, 0, w, h);   
   XFreeGC(_ecore_x_disp, gc);

   free(xim->data);
   xim->data = NULL;
   XDestroyImage(xim);
   
   c1.pixel = 0;
   c1.red   = fr << 8 | fr;
   c1.green = fg << 8 | fg;
   c1.blue  = fb << 8 | fb;
   c1.flags = DoRed | DoGreen | DoBlue;
   
   c2.pixel = 0;
   c2.red   = br << 8 | br;
   c2.green = bg << 8 | bg;
   c2.blue  = bb << 8 | bb;
   c2.flags = DoRed | DoGreen | DoBlue;   
   
   c = XCreatePixmapCursor(_ecore_x_disp, 
			   pmap, mask, 
			   &c1, &c2, 
			   hot_x, hot_y);   
   XFreePixmap(_ecore_x_disp, pmap);
   XFreePixmap(_ecore_x_disp, mask);
   return c;
}

void
ecore_x_cursor_free(Ecore_X_Cursor c)
{
   XFreeCursor(_ecore_x_disp, c);
}

void
ecore_x_cursor_set(Ecore_X_Window win, Ecore_X_Cursor c)
{
   if (c == 0)
     XUndefineCursor(_ecore_x_disp, win);
   else
     XDefineCursor(_ecore_x_disp, win, c);
}

void
ecore_x_cursor_shape_set(Ecore_X_Window win, int shape)
{
   /* Shapes are defined in Ecore_X_Cursor.h */
   Cursor c = XCreateFontCursor(_ecore_x_disp, shape);
   if (c)
      ecore_x_cursor_set(win, c);
}

int
ecore_x_pointer_grab(Ecore_X_Window win)
{
   return XGrabPointer(_ecore_x_disp, win, False,
		       ButtonPressMask | ButtonReleaseMask | 
		       EnterWindowMask | LeaveWindowMask | PointerMotionMask,
		       GrabModeAsync, GrabModeAsync,
		       None, None, CurrentTime);
}

void
ecore_x_pointer_ungrab(void)
{
   XUngrabPointer(_ecore_x_disp, CurrentTime);
}

int
ecore_x_keyboard_grab(Ecore_X_Window win)
{
   return XGrabKeyboard(_ecore_x_disp, win, False,
			GrabModeAsync, GrabModeAsync,
			CurrentTime);
}

void
ecore_x_keyboard_ungrab(void)
{
   XUngrabKeyboard(_ecore_x_disp, CurrentTime);   
}

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
