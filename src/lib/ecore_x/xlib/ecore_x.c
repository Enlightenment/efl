#ifdef HAVE_CONFIG_H
# include <config.h>
#endif /* ifdef HAVE_CONFIG_H */

#ifdef STDC_HEADERS
# include <stdlib.h>
# include <stddef.h>
#else
# ifdef HAVE_STDLIB_H
#  include <stdlib.h>
# endif
#endif

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

//#define LOGRT 1

#ifdef LOGRT
#include <dlfcn.h>
#endif /* ifdef LOGRT */

#include "Ecore.h"
#include "ecore_private.h"
#include "ecore_x_private.h"
#include "Ecore_X.h"
#include "Ecore_X_Atoms.h"
#include "Ecore_Input.h"

static Ecore_X_Version _version = { VMAJ, VMIN, VMIC, VREV };
EAPI Ecore_X_Version *ecore_x_version = &_version;

static Eina_Bool _ecore_x_fd_handler(void *data,
                                     Ecore_Fd_Handler *fd_handler);
static Eina_Bool _ecore_x_fd_handler_buf(void *data,
                                         Ecore_Fd_Handler *fd_handler);
static int       _ecore_x_key_mask_get(KeySym sym);
static int       _ecore_x_event_modifier(unsigned int state);

static Ecore_Fd_Handler *_ecore_x_fd_handler_handle = NULL;

static const int AnyXEvent = 0; /* 0 can be used as there are no event types
                                 * with index 0 and 1 as they are used for
                                 * errors
                                 */

static int _ecore_x_event_shape_id = 0;
static int _ecore_x_event_screensaver_id = 0;
static int _ecore_x_event_sync_id = 0;
int _ecore_xlib_log_dom = -1;

Eina_Bool _ecore_xlib_sync = EINA_FALSE;

#ifdef ECORE_XRANDR
static int _ecore_x_event_randr_id = 0;
#endif /* ifdef ECORE_XRANDR */
#ifdef ECORE_XFIXES
static int _ecore_x_event_fixes_selection_id = 0;
#endif /* ifdef ECORE_XFIXES */
#ifdef ECORE_XDAMAGE
static int _ecore_x_event_damage_id = 0;
#endif /* ifdef ECORE_XDAMAGE */
#ifdef ECORE_XGESTURE
static int _ecore_x_event_gesture_id = 0;
#endif /* ifdef ECORE_XGESTURE */
#ifdef ECORE_XKB
static int _ecore_x_event_xkb_id = 0;
#endif /* ifdef ECORE_XKB */
static int _ecore_x_event_handlers_num = 0;
typedef void (*Ecore_X_Event_Handler) (XEvent *event);
static Ecore_X_Event_Handler *_ecore_x_event_handlers = NULL;

static int _ecore_x_init_count = 0;
static int _ecore_x_grab_count = 0;

Display *_ecore_x_disp = NULL;
double _ecore_x_double_click_time = 0.25;
Time _ecore_x_event_last_time = 0;
Window _ecore_x_event_last_win = 0;
int _ecore_x_event_last_root_x = 0;
int _ecore_x_event_last_root_y = 0;
Eina_Bool _ecore_x_xcursor = EINA_FALSE;

Ecore_X_Window _ecore_x_private_win = 0;

Ecore_X_Atom _ecore_x_atoms_wm_protocols[ECORE_X_WM_PROTOCOL_NUM];

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
EAPI int ECORE_X_EVENT_FIXES_SELECTION_NOTIFY = 0;
EAPI int ECORE_X_EVENT_CLIENT_MESSAGE = 0;
EAPI int ECORE_X_EVENT_WINDOW_SHAPE = 0;
EAPI int ECORE_X_EVENT_SCREENSAVER_NOTIFY = 0;
EAPI int ECORE_X_EVENT_GESTURE_NOTIFY_FLICK;
EAPI int ECORE_X_EVENT_GESTURE_NOTIFY_PAN;
EAPI int ECORE_X_EVENT_GESTURE_NOTIFY_PINCHROTATION;
EAPI int ECORE_X_EVENT_GESTURE_NOTIFY_TAP;
EAPI int ECORE_X_EVENT_GESTURE_NOTIFY_TAPNHOLD;
EAPI int ECORE_X_EVENT_GESTURE_NOTIFY_HOLD;
EAPI int ECORE_X_EVENT_GESTURE_NOTIFY_GROUP;
EAPI int ECORE_X_EVENT_SYNC_COUNTER = 0;
EAPI int ECORE_X_EVENT_SYNC_ALARM = 0;
EAPI int ECORE_X_EVENT_SCREEN_CHANGE = 0;
EAPI int ECORE_X_EVENT_DAMAGE_NOTIFY = 0;
EAPI int ECORE_X_EVENT_RANDR_CRTC_CHANGE = 0;
EAPI int ECORE_X_EVENT_RANDR_OUTPUT_CHANGE = 0;
EAPI int ECORE_X_EVENT_RANDR_OUTPUT_PROPERTY_NOTIFY = 0;
EAPI int ECORE_X_EVENT_WINDOW_DELETE_REQUEST = 0;
EAPI int ECORE_X_EVENT_WINDOW_MOVE_RESIZE_REQUEST = 0;
EAPI int ECORE_X_EVENT_WINDOW_STATE_REQUEST = 0;
EAPI int ECORE_X_EVENT_FRAME_EXTENTS_REQUEST = 0;
EAPI int ECORE_X_EVENT_PING = 0;
EAPI int ECORE_X_EVENT_DESKTOP_CHANGE = 0;

EAPI int ECORE_X_EVENT_STARTUP_SEQUENCE_NEW = 0;
EAPI int ECORE_X_EVENT_STARTUP_SEQUENCE_CHANGE = 0;
EAPI int ECORE_X_EVENT_STARTUP_SEQUENCE_REMOVE = 0;

EAPI int ECORE_X_EVENT_XKB_STATE_NOTIFY = 0;
EAPI int ECORE_X_EVENT_XKB_NEWKBD_NOTIFY = 0;


EAPI int ECORE_X_EVENT_GENERIC = 0;

EAPI int ECORE_X_EVENT_PRESENT_CONFIGURE = 0;
EAPI int ECORE_X_EVENT_PRESENT_COMPLETE = 0;
EAPI int ECORE_X_EVENT_PRESENT_IDLE = 0;

EAPI int ECORE_X_MODIFIER_SHIFT = 0;
EAPI int ECORE_X_MODIFIER_CTRL = 0;
EAPI int ECORE_X_MODIFIER_ALT = 0;
EAPI int ECORE_X_MODIFIER_WIN = 0;
EAPI int ECORE_X_MODIFIER_ALTGR = 0;

EAPI int ECORE_X_LOCK_SCROLL = 0;
EAPI int ECORE_X_LOCK_NUM = 0;
EAPI int ECORE_X_LOCK_CAPS = 0;
EAPI int ECORE_X_LOCK_SHIFT = 0;

EAPI int ECORE_X_RAW_BUTTON_PRESS = 0;
EAPI int ECORE_X_RAW_BUTTON_RELEASE = 0;
EAPI int ECORE_X_RAW_MOTION = 0;

#ifdef LOGRT
static double t0 = 0.0;
static Status (*_logrt_real_reply)(Display *disp,
                                   void *rep,
                                   int extra,
                                   Bool discard) = NULL;
static void
_logrt_init(void)
{
   void *lib;

   lib = dlopen("libX11.so", RTLD_GLOBAL | RTLD_LAZY);
   if (!lib)
     lib = dlopen("libX11.so.6", RTLD_GLOBAL | RTLD_LAZY);

   if (!lib)
     lib = dlopen("libX11.so.6.3", RTLD_GLOBAL | RTLD_LAZY);

   if (!lib)
     lib = dlopen("libX11.so.6.3.0", RTLD_GLOBAL | RTLD_LAZY);

   _logrt_real_reply = dlsym(lib, "_XReply");
   t0 = ecore_time_get();
}

Status
_XReply(Display *disp,
        void *rep,
        int extra,
        Bool discard)
{
   void *bt[128];
   int i, n;
   char **sym;

   n = backtrace(bt, 128);
   if (n > 0)
     {
        sym = backtrace_symbols(bt, n);
        printf("ROUNDTRIP: %4.4f :", ecore_time_get() - t0);
        if (sym)
          {
             for (i = n - 1; i > 0; i--)
               {
                  char *fname = strchr(sym[i], '(');
                  if (fname)
                    {
                       char *tsym = alloca(strlen(fname) + 1);
                       char *end;
                       strcpy(tsym, fname + 1);
                       end = strchr(tsym, '+');
                       if (end)
                         {
                            *end = 0;
                            printf("%s", tsym);
                         }
                       else
                         printf("???");
                    }
                  else
                    printf("???");

                  if (i > 1)
                    printf(" > ");
               }
             printf("\n");
          }
     }

   // fixme: logme
   return _logrt_real_reply(disp, rep, extra, discard);
}

#endif /* ifdef LOGRT */

/* wrapper to use XkbKeycodeToKeysym when possible */
KeySym
_ecore_x_XKeycodeToKeysym(Display *display, KeyCode keycode, int idx)
{
#ifdef ECORE_XKB
   return XkbKeycodeToKeysym(display, keycode, 0, idx);
#else
   return XKeycodeToKeysym(display, keycode, idx);
#endif
}

void
_ecore_x_modifiers_get(void)
{
   ECORE_X_MODIFIER_SHIFT = 0;
   ECORE_X_MODIFIER_CTRL = 0;
   ECORE_X_MODIFIER_ALT = 0;
   ECORE_X_MODIFIER_WIN = 0;
   ECORE_X_MODIFIER_ALTGR = 0;
   ECORE_X_LOCK_SCROLL = 0;
   ECORE_X_LOCK_NUM = 0;
   ECORE_X_LOCK_CAPS = 0;
   ECORE_X_LOCK_SHIFT = 0;

   /* everything has these... unless its like a pda... :) */
   ECORE_X_MODIFIER_SHIFT = _ecore_x_key_mask_get(XK_Shift_L);
   ECORE_X_MODIFIER_CTRL = _ecore_x_key_mask_get(XK_Control_L);

   /* apple's xdarwin has no alt!!!! */
   ECORE_X_MODIFIER_ALT = _ecore_x_key_mask_get(XK_Alt_L);
   if (!ECORE_X_MODIFIER_ALT)
     ECORE_X_MODIFIER_ALT = _ecore_x_key_mask_get(XK_Meta_L);

   if (!ECORE_X_MODIFIER_ALT)
     ECORE_X_MODIFIER_ALT = _ecore_x_key_mask_get(XK_Super_L);

   /* the windows key... a valid modifier :) */
   ECORE_X_MODIFIER_WIN = _ecore_x_key_mask_get(XK_Super_L);
   if (!ECORE_X_MODIFIER_WIN)
     ECORE_X_MODIFIER_WIN = _ecore_x_key_mask_get(XK_Meta_L);

   ECORE_X_MODIFIER_ALTGR = _ecore_x_key_mask_get(XK_Mode_switch);

   if (ECORE_X_MODIFIER_WIN == ECORE_X_MODIFIER_ALT)
     ECORE_X_MODIFIER_WIN = 0;

   if (ECORE_X_MODIFIER_ALT == ECORE_X_MODIFIER_CTRL)
     ECORE_X_MODIFIER_ALT = 0;

   if (ECORE_X_MODIFIER_ALTGR)
     {
        if ((ECORE_X_MODIFIER_ALTGR == ECORE_X_MODIFIER_SHIFT) ||
            (ECORE_X_MODIFIER_ALTGR == ECORE_X_MODIFIER_CTRL) ||
            (ECORE_X_MODIFIER_ALTGR == ECORE_X_MODIFIER_ALT) ||
            (ECORE_X_MODIFIER_ALTGR == ECORE_X_MODIFIER_WIN))
          {
             ERR("ALTGR conflicts with other modifiers. IGNORE ALTGR");
             ECORE_X_MODIFIER_ALTGR = 0;
          }
     }

   if (ECORE_X_MODIFIER_ALT)
     {
        if ((ECORE_X_MODIFIER_ALT == ECORE_X_MODIFIER_SHIFT) ||
            (ECORE_X_MODIFIER_ALT == ECORE_X_MODIFIER_CTRL) ||
            (ECORE_X_MODIFIER_ALT == ECORE_X_MODIFIER_WIN))
          {
             ERR("ALT conflicts with other modifiers. IGNORE ALT");
             ECORE_X_MODIFIER_ALT = 0;
          }
     }

   if (ECORE_X_MODIFIER_WIN)
     {
        if ((ECORE_X_MODIFIER_WIN == ECORE_X_MODIFIER_SHIFT) ||
            (ECORE_X_MODIFIER_WIN == ECORE_X_MODIFIER_CTRL))
          {
             ERR("WIN conflicts with other modifiers. IGNORE WIN");
             ECORE_X_MODIFIER_WIN = 0;
          }
     }

   if (ECORE_X_MODIFIER_SHIFT)
     {
        if ((ECORE_X_MODIFIER_SHIFT == ECORE_X_MODIFIER_CTRL))
          {
             ERR("CTRL conflicts with other modifiers. IGNORE CTRL");
             ECORE_X_MODIFIER_CTRL = 0;
          }
     }

   ECORE_X_LOCK_SCROLL = _ecore_x_key_mask_get(XK_Scroll_Lock);
   ECORE_X_LOCK_NUM = _ecore_x_key_mask_get(XK_Num_Lock);
   ECORE_X_LOCK_CAPS = _ecore_x_key_mask_get(XK_Caps_Lock);
   ECORE_X_LOCK_SHIFT = _ecore_x_key_mask_get(XK_Shift_Lock);
}

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
#endif /* ifdef ECORE_XSS */
   int sync_base = 0;
   int sync_err_base = 0;
#ifdef ECORE_XRANDR
   int randr_base = 0;
   int randr_err_base = 0;
#endif /* ifdef ECORE_XRANDR */
#ifdef ECORE_XFIXES
   int fixes_base = 0;
   int fixes_err_base = 0;
#endif /* ifdef ECORE_XFIXES */
#ifdef ECORE_XDAMAGE
   int damage_base = 0;
   int damage_err_base = 0;
#endif /* ifdef ECORE_XDAMAGE */
#ifdef ECORE_XGESTURE
   int gesture_base = 0;
   int gesture_err_base = 0;
#endif /* ifdef ECORE_XGESTURE */
#ifdef ECORE_XKB
   int xkb_base = 0;
#endif /* ifdef ECORE_XKB */
   if (++_ecore_x_init_count != 1)
     return _ecore_x_init_count;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
#ifdef LOGRT
   _logrt_init();
#endif /* ifdef LOGRT */

   eina_init();
   _ecore_xlib_log_dom = eina_log_domain_register
       ("ecore_x", ECORE_XLIB_DEFAULT_LOG_COLOR);
   if (_ecore_xlib_log_dom < 0)
     {
        EINA_LOG_ERR(
          "Impossible to create a log domain for the Ecore Xlib module.");
        return --_ecore_x_init_count;
     }

   if (!ecore_init())
     goto shutdown_eina;
   if (!ecore_event_init())
     goto shutdown_ecore;

#ifdef EVAS_FRAME_QUEUING
   XInitThreads();
#endif /* ifdef EVAS_FRAME_QUEUING */
   _ecore_x_disp = XOpenDisplay((char *)name);
   if (!_ecore_x_disp)
     goto shutdown_ecore_event;

   _ecore_x_error_handler_init();
   _ecore_x_event_handlers_num = LASTEvent;

#define ECORE_X_EVENT_HANDLERS_GROW(ext_base, ext_num_events)            \
  do {                                                                   \
       if (_ecore_x_event_handlers_num < (ext_base + ext_num_events)) {  \
            _ecore_x_event_handlers_num = (ext_base + ext_num_events); } \
    } while (0)

   if (XShapeQueryExtension(_ecore_x_disp, &shape_base, &shape_err_base))
     _ecore_x_event_shape_id = shape_base;

   ECORE_X_EVENT_HANDLERS_GROW(shape_base, ShapeNumberEvents);

#ifdef ECORE_XSS
   if (XScreenSaverQueryExtension(_ecore_x_disp, &screensaver_base,
                                  &screensaver_err_base))
     _ecore_x_event_screensaver_id = screensaver_base;

   ECORE_X_EVENT_HANDLERS_GROW(screensaver_base, ScreenSaverNumberEvents);
#endif /* ifdef ECORE_XSS */

   if (XSyncQueryExtension(_ecore_x_disp, &sync_base, &sync_err_base))
     {
        int major, minor;

        _ecore_x_event_sync_id = sync_base;
        if (!XSyncInitialize(_ecore_x_disp, &major, &minor))
          _ecore_x_event_sync_id = 0;
     }

   ECORE_X_EVENT_HANDLERS_GROW(sync_base, XSyncNumberEvents);

#ifdef ECORE_XRANDR
   if (XRRQueryExtension(_ecore_x_disp, &randr_base, &randr_err_base))
     _ecore_x_event_randr_id = randr_base;

   ECORE_X_EVENT_HANDLERS_GROW(randr_base, RRNumberEvents);
#endif /* ifdef ECORE_XRANDR */

#ifdef ECORE_XFIXES
   if (XFixesQueryExtension(_ecore_x_disp, &fixes_base, &fixes_err_base))
     _ecore_x_event_fixes_selection_id = fixes_base;

   ECORE_X_EVENT_HANDLERS_GROW(fixes_base, XFixesNumberEvents);
#endif /* ifdef ECORE_XFIXES */

#ifdef ECORE_XDAMAGE
   if (XDamageQueryExtension(_ecore_x_disp, &damage_base, &damage_err_base))
     _ecore_x_event_damage_id = damage_base;

   ECORE_X_EVENT_HANDLERS_GROW(damage_base, XDamageNumberEvents);
#endif /* ifdef ECORE_XDAMAGE */

#ifdef ECORE_XGESTURE
   if (XGestureQueryExtension(_ecore_x_disp, &gesture_base, &gesture_err_base))
     _ecore_x_event_gesture_id = gesture_base;

   ECORE_X_EVENT_HANDLERS_GROW(gesture_base, GestureNumberEvents);
#endif /* ifdef ECORE_XGESTURE */
#ifdef ECORE_XKB
     {
        int dummy;
        
        if (XkbQueryExtension(_ecore_x_disp, &dummy, &xkb_base,
                              &dummy, &dummy, &dummy))
          _ecore_x_event_xkb_id = xkb_base;
        XkbSelectEventDetails(_ecore_x_disp, XkbUseCoreKbd, XkbStateNotify,
                              XkbAllStateComponentsMask, XkbGroupStateMask);
     }
   ECORE_X_EVENT_HANDLERS_GROW(xkb_base, XkbNumberEvents);
#endif

   _ecore_x_event_handlers = calloc(_ecore_x_event_handlers_num, sizeof(Ecore_X_Event_Handler));
   if (!_ecore_x_event_handlers)
     goto close_display;

#ifdef ECORE_XCURSOR
   _ecore_x_xcursor = XcursorSupportsARGB(_ecore_x_disp) ? EINA_TRUE : EINA_FALSE;
#endif /* ifdef ECORE_XCURSOR */
   _ecore_x_event_handlers[AnyXEvent] = _ecore_x_event_handle_any_event;
   _ecore_x_event_handlers[KeyPress] = _ecore_x_event_handle_key_press;
   _ecore_x_event_handlers[KeyRelease] = _ecore_x_event_handle_key_release;
   _ecore_x_event_handlers[ButtonPress] = _ecore_x_event_handle_button_press;
   _ecore_x_event_handlers[ButtonRelease] = _ecore_x_event_handle_button_release;
   _ecore_x_event_handlers[MotionNotify] = _ecore_x_event_handle_motion_notify;
   _ecore_x_event_handlers[EnterNotify] = _ecore_x_event_handle_enter_notify;
   _ecore_x_event_handlers[LeaveNotify] = _ecore_x_event_handle_leave_notify;
   _ecore_x_event_handlers[FocusIn] = _ecore_x_event_handle_focus_in;
   _ecore_x_event_handlers[FocusOut] = _ecore_x_event_handle_focus_out;
   _ecore_x_event_handlers[KeymapNotify] = _ecore_x_event_handle_keymap_notify;
   _ecore_x_event_handlers[Expose] = _ecore_x_event_handle_expose;
   _ecore_x_event_handlers[GraphicsExpose] = _ecore_x_event_handle_graphics_expose;
   _ecore_x_event_handlers[VisibilityNotify] = _ecore_x_event_handle_visibility_notify;
   _ecore_x_event_handlers[CreateNotify] = _ecore_x_event_handle_create_notify;
   _ecore_x_event_handlers[DestroyNotify] = _ecore_x_event_handle_destroy_notify;
   _ecore_x_event_handlers[UnmapNotify] = _ecore_x_event_handle_unmap_notify;
   _ecore_x_event_handlers[MapNotify] = _ecore_x_event_handle_map_notify;
   _ecore_x_event_handlers[MapRequest] = _ecore_x_event_handle_map_request;
   _ecore_x_event_handlers[ReparentNotify] = _ecore_x_event_handle_reparent_notify;
   _ecore_x_event_handlers[ConfigureNotify] = _ecore_x_event_handle_configure_notify;
   _ecore_x_event_handlers[ConfigureRequest] = _ecore_x_event_handle_configure_request;
   _ecore_x_event_handlers[GravityNotify] = _ecore_x_event_handle_gravity_notify;
   _ecore_x_event_handlers[ResizeRequest] = _ecore_x_event_handle_resize_request;
   _ecore_x_event_handlers[CirculateNotify] = _ecore_x_event_handle_circulate_notify;
   _ecore_x_event_handlers[CirculateRequest] = _ecore_x_event_handle_circulate_request;
   _ecore_x_event_handlers[PropertyNotify] = _ecore_x_event_handle_property_notify;
   _ecore_x_event_handlers[SelectionClear] = _ecore_x_event_handle_selection_clear;
   _ecore_x_event_handlers[SelectionRequest] = _ecore_x_event_handle_selection_request;
   _ecore_x_event_handlers[SelectionNotify] = _ecore_x_event_handle_selection_notify;
   _ecore_x_event_handlers[ColormapNotify] = _ecore_x_event_handle_colormap_notify;
   _ecore_x_event_handlers[ClientMessage] = _ecore_x_event_handle_client_message;
   _ecore_x_event_handlers[MappingNotify] = _ecore_x_event_handle_mapping_notify;
#ifdef GenericEvent
   _ecore_x_event_handlers[GenericEvent] = _ecore_x_event_handle_generic_event;
#endif /* ifdef GenericEvent */

   if (_ecore_x_event_shape_id)
     _ecore_x_event_handlers[_ecore_x_event_shape_id] = _ecore_x_event_handle_shape_change;
   if (_ecore_x_event_screensaver_id)
     _ecore_x_event_handlers[_ecore_x_event_screensaver_id] = _ecore_x_event_handle_screensaver_notify;
   if (_ecore_x_event_sync_id)
     {
        _ecore_x_event_handlers[_ecore_x_event_sync_id + XSyncCounterNotify] = _ecore_x_event_handle_sync_counter;
        _ecore_x_event_handlers[_ecore_x_event_sync_id + XSyncAlarmNotify] = _ecore_x_event_handle_sync_alarm;
     }

#ifdef ECORE_XRANDR
   if (_ecore_x_event_randr_id)
     {
        _ecore_x_event_handlers[_ecore_x_event_randr_id + RRScreenChangeNotify] = _ecore_x_event_handle_randr_change;
        _ecore_x_event_handlers[_ecore_x_event_randr_id + RRNotify] = _ecore_x_event_handle_randr_notify;
     }
#endif /* ifdef ECORE_XRANDR */
#ifdef ECORE_XFIXES
   if (_ecore_x_event_fixes_selection_id)
     _ecore_x_event_handlers[_ecore_x_event_fixes_selection_id] = _ecore_x_event_handle_fixes_selection_notify;

#endif /* ifdef ECORE_XFIXES */
#ifdef ECORE_XDAMAGE
   if (_ecore_x_event_damage_id)
     _ecore_x_event_handlers[_ecore_x_event_damage_id] = _ecore_x_event_handle_damage_notify;

#endif /* ifdef ECORE_XDAMAGE */
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
   if (_ecore_x_event_xkb_id)
   _ecore_x_event_handlers[_ecore_x_event_xkb_id] = _ecore_x_event_handle_xkb;
#endif /* ifdef ECORE_XKB */

#ifdef ECORE_XGESTURE
   if (_ecore_x_event_gesture_id)
     {
        _ecore_x_event_handlers[_ecore_x_event_gesture_id + GestureNotifyFlick] = _ecore_x_event_handle_gesture_notify_flick;
        _ecore_x_event_handlers[_ecore_x_event_gesture_id + GestureNotifyPan] = _ecore_x_event_handle_gesture_notify_pan;
        _ecore_x_event_handlers[_ecore_x_event_gesture_id + GestureNotifyPinchRotation] = _ecore_x_event_handle_gesture_notify_pinchrotation;
        _ecore_x_event_handlers[_ecore_x_event_gesture_id + GestureNotifyTap] = _ecore_x_event_handle_gesture_notify_tap;
        _ecore_x_event_handlers[_ecore_x_event_gesture_id + GestureNotifyTapNHold] = _ecore_x_event_handle_gesture_notify_tapnhold;
        _ecore_x_event_handlers[_ecore_x_event_gesture_id + GestureNotifyHold] = _ecore_x_event_handle_gesture_notify_hold;
        _ecore_x_event_handlers[_ecore_x_event_gesture_id + GestureNotifyGroup] = _ecore_x_event_handle_gesture_notify_group;
     }

#endif /* ifdef ECORE_XGESTURE */

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
        ECORE_X_EVENT_GESTURE_NOTIFY_FLICK = ecore_event_type_new();
        ECORE_X_EVENT_GESTURE_NOTIFY_PAN = ecore_event_type_new();
        ECORE_X_EVENT_GESTURE_NOTIFY_PINCHROTATION = ecore_event_type_new();
        ECORE_X_EVENT_GESTURE_NOTIFY_TAP = ecore_event_type_new();
        ECORE_X_EVENT_GESTURE_NOTIFY_TAPNHOLD = ecore_event_type_new();
        ECORE_X_EVENT_GESTURE_NOTIFY_HOLD = ecore_event_type_new();
        ECORE_X_EVENT_GESTURE_NOTIFY_GROUP = ecore_event_type_new();
        ECORE_X_EVENT_SYNC_COUNTER = ecore_event_type_new();
        ECORE_X_EVENT_SYNC_ALARM = ecore_event_type_new();
        ECORE_X_EVENT_SCREEN_CHANGE = ecore_event_type_new();
        ECORE_X_EVENT_RANDR_CRTC_CHANGE = ecore_event_type_new();
        ECORE_X_EVENT_RANDR_OUTPUT_CHANGE = ecore_event_type_new();
        ECORE_X_EVENT_RANDR_OUTPUT_PROPERTY_NOTIFY = ecore_event_type_new();
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

        ECORE_X_EVENT_XKB_STATE_NOTIFY = ecore_event_type_new();
        ECORE_X_EVENT_XKB_NEWKBD_NOTIFY = ecore_event_type_new();
	
        ECORE_X_EVENT_GENERIC = ecore_event_type_new();

	ECORE_X_RAW_BUTTON_PRESS = ecore_event_type_new();
	ECORE_X_RAW_BUTTON_RELEASE = ecore_event_type_new();
	ECORE_X_RAW_MOTION = ecore_event_type_new();
     }

   _ecore_x_modifiers_get();

   _ecore_x_atoms_init();

   /* Set up the ICCCM hints */
   ecore_x_icccm_init();

   /* Set up the _NET_... hints */
   ecore_x_netwm_init();

   /* old e hints init */
   ecore_x_e_init();

   /* This is just to be anal about naming conventions */

   _ecore_x_atoms_wm_protocols[ECORE_X_WM_PROTOCOL_DELETE_REQUEST] =
     ECORE_X_ATOM_WM_DELETE_WINDOW;
   _ecore_x_atoms_wm_protocols[ECORE_X_WM_PROTOCOL_TAKE_FOCUS] =
     ECORE_X_ATOM_WM_TAKE_FOCUS;
   _ecore_x_atoms_wm_protocols[ECORE_X_NET_WM_PROTOCOL_PING] =
     ECORE_X_ATOM_NET_WM_PING;
   _ecore_x_atoms_wm_protocols[ECORE_X_NET_WM_PROTOCOL_SYNC_REQUEST] =
     ECORE_X_ATOM_NET_WM_SYNC_REQUEST;

   _ecore_x_selection_data_init();
   _ecore_x_dnd_init();
   _ecore_x_fixes_init();
   _ecore_x_damage_init();
   _ecore_x_composite_init();
   _ecore_x_present_init();
   _ecore_x_dpms_init();
   _ecore_x_randr_init();
   _ecore_x_gesture_init();
   _ecore_x_input_init();
   _ecore_x_events_init();

   _ecore_x_fd_handler_handle =
     ecore_main_fd_handler_add(ConnectionNumber(_ecore_x_disp),
                               ECORE_FD_READ,
                               _ecore_x_fd_handler, _ecore_x_disp,
                               _ecore_x_fd_handler_buf, _ecore_x_disp);
   if (!_ecore_x_fd_handler_handle)
     goto free_event_handlers;

   _ecore_x_private_win = ecore_x_window_override_new(0, -77, -777, 123, 456);
   _ecore_xlib_sync = !!getenv("ECORE_X_SYNC");

   return _ecore_x_init_count;

free_event_handlers:
   free(_ecore_x_event_handlers);
   _ecore_x_event_handlers = NULL;
close_display:
   XCloseDisplay(_ecore_x_disp);
   _ecore_x_fd_handler_handle = NULL;
   _ecore_x_disp = NULL;
shutdown_ecore_event:
   ecore_event_shutdown();
shutdown_ecore:
   ecore_shutdown();
shutdown_eina:
   eina_log_domain_unregister(_ecore_xlib_log_dom);
   _ecore_xlib_log_dom = -1;
   eina_shutdown();

   return --_ecore_x_init_count;
}

static int
_ecore_x_shutdown(int close_display)
{
   if (--_ecore_x_init_count != 0)
     return _ecore_x_init_count;

   if (!_ecore_x_disp)
     return _ecore_x_init_count;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   ecore_main_fd_handler_del(_ecore_x_fd_handler_handle);
   if (close_display)
     XCloseDisplay(_ecore_x_disp);
   else
     {
        close(ConnectionNumber(_ecore_x_disp));
        // FIXME: may have to clean up x display internal here
// getting segv here? hmmm. odd. disable
//        XFree(_ecore_x_disp);
     }

   free(_ecore_x_event_handlers);
   _ecore_x_fd_handler_handle = NULL;
   _ecore_x_disp = NULL;
   _ecore_x_event_handlers = NULL;
   _ecore_x_events_shutdown();
   _ecore_x_input_shutdown();
   _ecore_x_selection_shutdown();
   _ecore_x_dnd_shutdown();
   ecore_x_netwm_shutdown();

   ecore_event_shutdown();
   ecore_shutdown();

   eina_log_domain_unregister(_ecore_xlib_log_dom);
   _ecore_xlib_log_dom = -1;
   eina_shutdown();
   _ecore_xlib_sync = EINA_FALSE;

   return _ecore_x_init_count;
}

/**
 * Shuts down the Ecore X library.
 *
 * In shutting down the library, the X display connection is terminated
 * and any event handlers for it are removed.
 *
 * @return  The number of times the library has been initialized without
 *          being shut down. 0 is returned if an error occurs.
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
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   return ConnectionNumber(_ecore_x_disp);
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
   return (Ecore_X_Screen *)DefaultScreenOfDisplay(_ecore_x_disp);
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
ecore_x_screen_size_get(const Ecore_X_Screen *screen,
                        int *w,
                        int *h)
{
   Screen *s = (Screen *)screen;
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   if (w) *w = 0;
   if (h) *h = 0;
   if (!s) return;
   if (w) *w = s->width;
   if (h) *h = s->height;
}

/**
 * Retrieves the number of screens.
 *
 * @return  The count of the number of screens.
 * @ingroup Ecore_X_Display_Attr_Group
 *
 * @since 1.1
 */
EAPI int
ecore_x_screen_count_get(void)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   return ScreenCount(_ecore_x_disp);
}

/**
 * Retrieves the index number of the given screen.
 *
 * @param screen The screen for which the index will be retrieved.
 * @return  The index number of the screen.
 * @ingroup Ecore_X_Display_Attr_Group
 *
 * @since 1.1
 */
EAPI int
ecore_x_screen_index_get(const Ecore_X_Screen *screen)
{
   return XScreenNumberOfScreen((Screen *)screen);
}

/**
 * Retrieves the screen based on index number.
 *
 * @param idx The index that will be used to retrieve the screen.
 * @return  The Ecore_X_Screen at this index.
 * @ingroup Ecore_X_Display_Attr_Group
 *
 * @since 1.1
 */
EAPI Ecore_X_Screen *
ecore_x_screen_get(int idx)
{
   return XScreenOfDisplay(_ecore_x_disp, idx);
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
   if (t < 0.0)
     t = 0.0;

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
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
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
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
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

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
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
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   XKillClient(_ecore_x_disp, win);
   if (_ecore_xlib_sync) ecore_x_sync();
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
 * DPI in the x amd y axes nor does it account for multihead or xinerama and
 * xrander where different parts of the screen may have different DPI etc.
 *
 * @return the general screen DPI (dots/pixels per inch).
 */
EAPI int
ecore_x_dpi_get(void)
{
   Screen *s;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   s = DefaultScreenOfDisplay(_ecore_x_disp);
   if (s->mwidth <= 0)
     return 75;

   return (((s->width * 254) / s->mwidth) + 5) / 10;
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
 * @returns @c EINA_TRUE on success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool
ecore_x_bell(int percent)
{
   int ret;

   ret = XBell(_ecore_x_disp, percent);
   if (ret == BadValue)
     return EINA_FALSE;

   return EINA_TRUE;
}

static Eina_Bool
_ecore_x_fd_handler(void *data,
                    Ecore_Fd_Handler *fd_handler EINA_UNUSED)
{
   Display *d;

   d = data;
   while (XPending(d))
     {
        XEvent ev;

        XNextEvent(d, &ev);
#ifdef ENABLE_XIM
        /* Filter event for XIM */
        if (XFilterEvent(&ev, ev.xkey.window))
          continue;

#endif /* ifdef ENABLE_XIM */
        if ((ev.type >= 0) && (ev.type < _ecore_x_event_handlers_num))
          {
             if (_ecore_x_event_handlers[AnyXEvent])
               _ecore_x_event_handlers[AnyXEvent] (&ev);

             if (_ecore_x_event_handlers[ev.type])
               _ecore_x_event_handlers[ev.type] (&ev);
          }
     }
   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_ecore_x_fd_handler_buf(void *data,
                        Ecore_Fd_Handler *fd_handler EINA_UNUSED)
{
   Display *d;

   d = data;
   if (XPending(d))
     return ECORE_CALLBACK_RENEW;

   return ECORE_CALLBACK_CANCEL;
}

static int
_ecore_x_key_mask_get(KeySym sym)
{
   XModifierKeymap *mod;
   KeySym sym2;
   int i, j, mask = 0;
   const int masks[8] =
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
                  sym2 = _ecore_x_XKeycodeToKeysym(_ecore_x_disp,
                                                   mod->modifiermap[i], j);
                  if (sym2 != 0)
                  break;
               }
             if (sym2 == sym) mask = masks[i / mod->max_keypermod];
          }
     }
   if (mod)
     {
        if (mod->modifiermap) XFree(mod->modifiermap);
        XFree(mod);
     }
   return mask;
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
#endif /* ifdef ECORE_XPRINT */

   if (!num_ret)
     return NULL;

   *num_ret = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
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
             roots = malloc(MAX((num - overlap) * sizeof(Ecore_X_Window), 1));
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
             roots = malloc(num * sizeof(Ecore_X_Window));
             if (!roots)
               return NULL;

             *num_ret = num;
             for (i = 0; i < num; i++)
               roots[i] = RootWindow(_ecore_x_disp, i);
          }
     }
   else
     {
        roots = malloc(num * sizeof(Ecore_X_Window));
        if (!roots)
          return NULL;

        *num_ret = num;
        for (i = 0; i < num; i++)
          roots[i] = RootWindow(_ecore_x_disp, i);
     }

#else /* ifdef ECORE_XPRINT */
   num = ScreenCount(_ecore_x_disp);
   roots = malloc(num * sizeof(Ecore_X_Window));
   if (!roots)
     return NULL;

   *num_ret = num;
   for (i = 0; i < num; i++)
     roots[i] = RootWindow(_ecore_x_disp, i);
#endif /* ifdef ECORE_XPRINT */
   return roots;
}

EAPI Ecore_X_Window
ecore_x_window_root_first_get(void)
{
   return RootWindow(_ecore_x_disp, 0);
/*
   int num;
   Ecore_X_Window root, *roots = NULL;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   roots = ecore_x_window_root_list(&num);
   if (!(roots)) return 0;

   if (num > 0)
     root = roots[0];
   else
     root = 0;

   free(roots);
   return root;
 */
}

static void _ecore_x_window_manage_error(void *data);

static int _ecore_x_window_manage_failed = 0;
static void
_ecore_x_window_manage_error(void *data EINA_UNUSED)
{
   if ((ecore_x_error_request_get() == X_ChangeWindowAttributes) &&
       (ecore_x_error_code_get() == BadAccess))
     _ecore_x_window_manage_failed = 1;
}

EAPI Eina_Bool
ecore_x_window_manage(Ecore_X_Window win)
{
   XWindowAttributes att;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   if (XGetWindowAttributes(_ecore_x_disp, win, &att) != True)
     return EINA_FALSE;

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
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

EAPI void
ecore_x_window_container_manage(Ecore_X_Window win)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   XSelectInput(_ecore_x_disp, win,
                SubstructureRedirectMask |
                SubstructureNotifyMask);
   if (_ecore_xlib_sync) ecore_x_sync();
}

EAPI void
ecore_x_window_client_manage(Ecore_X_Window win)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   XSelectInput(_ecore_x_disp, win,
                PropertyChangeMask |
//		ResizeRedirectMask |
                FocusChangeMask |
                ColormapChangeMask |
                VisibilityChangeMask |
                StructureNotifyMask |
                SubstructureNotifyMask
                );
   if (_ecore_xlib_sync) ecore_x_sync();
   XShapeSelectInput(_ecore_x_disp, win, ShapeNotifyMask);
   if (_ecore_xlib_sync) ecore_x_sync();
}

EAPI void
ecore_x_window_sniff(Ecore_X_Window win)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   XSelectInput(_ecore_x_disp, win,
                PropertyChangeMask |
                SubstructureNotifyMask);
   if (_ecore_xlib_sync) ecore_x_sync();
}

EAPI void
ecore_x_window_client_sniff(Ecore_X_Window win)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   XSelectInput(_ecore_x_disp, win,
                PropertyChangeMask |
                FocusChangeMask |
                ColormapChangeMask |
                VisibilityChangeMask |
                StructureNotifyMask |
                SubstructureNotifyMask);
   if (_ecore_xlib_sync) ecore_x_sync();
   XShapeSelectInput(_ecore_x_disp, win, ShapeNotifyMask);
   if (_ecore_xlib_sync) ecore_x_sync();
}

EAPI Eina_Bool
ecore_x_window_attributes_get(Ecore_X_Window win,
                              Ecore_X_Window_Attributes *att_ret)
{
   XWindowAttributes att;
   Eina_Bool ret;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   ret = XGetWindowAttributes(_ecore_x_disp, win, &att);
   if (_ecore_xlib_sync) ecore_x_sync();
   if (!ret) return EINA_FALSE;

   memset(att_ret, 0, sizeof(Ecore_X_Window_Attributes));
   att_ret->root = att.root;
   att_ret->x = att.x;
   att_ret->y = att.y;
   att_ret->w = att.width;
   att_ret->h = att.height;
   att_ret->border = att.border_width;
   att_ret->depth = att.depth;
   if (att.map_state != IsUnmapped)
     att_ret->visible = 1;

   if (att.map_state == IsViewable)
     att_ret->viewable = 1;

   if (att.override_redirect)
     att_ret->override = 1;

   if (att.class == InputOnly)
     att_ret->input_only = 1;

   if (att.save_under)
     att_ret->save_under = 1;

   att_ret->event_mask.mine = att.your_event_mask;
   att_ret->event_mask.all = att.all_event_masks;
   att_ret->event_mask.no_propagate = att.do_not_propagate_mask;
   att_ret->window_gravity = att.win_gravity;
   att_ret->pixel_gravity = att.bit_gravity;
   att_ret->colormap = att.colormap;
   att_ret->visual = att.visual;
   return EINA_TRUE;
}

EAPI void
ecore_x_window_save_set_add(Ecore_X_Window win)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   XAddToSaveSet(_ecore_x_disp, win);
   if (_ecore_xlib_sync) ecore_x_sync();
}

EAPI void
ecore_x_window_save_set_del(Ecore_X_Window win)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   XRemoveFromSaveSet(_ecore_x_disp, win);
   if (_ecore_xlib_sync) ecore_x_sync();
}

EAPI Ecore_X_Window *
ecore_x_window_children_get(Ecore_X_Window win,
                            int *num)
{
   Ecore_X_Window *windows = NULL;
   Eina_Bool success;
   Window root_ret = 0, parent_ret = 0, *children_ret = NULL;
   unsigned int children_ret_num = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   success = XQueryTree(_ecore_x_disp, win, &root_ret, &parent_ret, &children_ret,
                   &children_ret_num);
   if (_ecore_xlib_sync) ecore_x_sync();
   if (!success) return NULL;

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

EAPI Eina_Bool
ecore_x_pointer_control_set(int accel_num,
                            int accel_denom,
                            int threshold)
{
   Eina_Bool ret;
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   ret = !!XChangePointerControl(_ecore_x_disp, 1, 1,
                                accel_num, accel_denom, threshold);
   if (_ecore_xlib_sync) ecore_x_sync();
   return ret;
}

EAPI Eina_Bool
ecore_x_pointer_control_get(int *accel_num,
                            int *accel_denom,
                            int *threshold)
{
   Eina_Bool ret;
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   ret = !!XGetPointerControl(_ecore_x_disp,
                             accel_num, accel_denom, threshold);
   if (_ecore_xlib_sync) ecore_x_sync();
   return ret;
}

EAPI Eina_Bool
ecore_x_pointer_mapping_set(unsigned char *map,
                            int nmap)
{
   Eina_Bool ret;
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   ret = (XSetPointerMapping(_ecore_x_disp, map, nmap) == MappingSuccess);
   if (_ecore_xlib_sync) ecore_x_sync();
   return ret;
}

EAPI Eina_Bool
ecore_x_pointer_mapping_get(unsigned char *map,
                            int nmap)
{
   Eina_Bool ret;
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   ret = !!XGetPointerMapping(_ecore_x_disp, map, nmap);
   if (_ecore_xlib_sync) ecore_x_sync();
   return ret;
}

EAPI Eina_Bool
ecore_x_pointer_grab(Ecore_X_Window win)
{
   Eina_Bool ret;
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   ret = (XGrabPointer(_ecore_x_disp, win, False,
                    ButtonPressMask | ButtonReleaseMask |
                    EnterWindowMask | LeaveWindowMask | PointerMotionMask,
                    GrabModeAsync, GrabModeAsync,
                    None, None, CurrentTime) == GrabSuccess);
   if (_ecore_xlib_sync) ecore_x_sync();
   return ret;
}

EAPI Eina_Bool
ecore_x_pointer_confine_grab(Ecore_X_Window win)
{
   Eina_Bool ret;
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   ret = (XGrabPointer(_ecore_x_disp, win, False,
                    ButtonPressMask | ButtonReleaseMask |
                    EnterWindowMask | LeaveWindowMask | PointerMotionMask,
                    GrabModeAsync, GrabModeAsync,
                    win, None, CurrentTime) == GrabSuccess);
   if (_ecore_xlib_sync) ecore_x_sync();
   return ret;
}

EAPI void
ecore_x_pointer_ungrab(void)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   XUngrabPointer(_ecore_x_disp, CurrentTime);
   if (_ecore_xlib_sync) ecore_x_sync();
}

EAPI Eina_Bool
ecore_x_pointer_warp(Ecore_X_Window win,
                     int x,
                     int y)
{
   Eina_Bool ret;
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   ret = !!XWarpPointer(_ecore_x_disp, None, win, 0, 0, 0, 0, x, y);
   if (_ecore_xlib_sync) ecore_x_sync();
   return ret;
}

EAPI Eina_Bool
ecore_x_keyboard_grab(Ecore_X_Window win)
{
   Eina_Bool ret;
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   ret = (XGrabKeyboard(_ecore_x_disp, win, False,
                     GrabModeAsync, GrabModeAsync,
                     CurrentTime) == GrabSuccess);
   if (_ecore_xlib_sync) ecore_x_sync();
   return ret;
}

EAPI void
ecore_x_keyboard_ungrab(void)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   XUngrabKeyboard(_ecore_x_disp, CurrentTime);
}

EAPI void
ecore_x_grab(void)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   _ecore_x_grab_count++;
   if (_ecore_x_grab_count == 1)
     XGrabServer(_ecore_x_disp);
}

EAPI void
ecore_x_ungrab(void)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   _ecore_x_grab_count--;
   if (_ecore_x_grab_count < 0)
     _ecore_x_grab_count = 0;

   if (_ecore_x_grab_count == 0)
     XUngrabServer(_ecore_x_disp);
}

Eina_Bool (*_ecore_window_grab_replay_func)(void *data,
                                            int event_type,
                                            void *event);
void *_ecore_window_grab_replay_data;

EAPI void
ecore_x_passive_grab_replay_func_set(Eina_Bool (*func)(void *data,
                                                       int event_type,
                                                       void *event),
                                     void *data)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   _ecore_window_grab_replay_func = func;
   _ecore_window_grab_replay_data = data;
}






//////////////////////////////////////////////////////////////////////////////
int _ecore_window_grabs_num = 0;
Wingrab *_ecore_window_grabs = NULL;

static void
_ecore_x_window_button_grab_internal(Ecore_X_Window win,
                                     int button,
                                     Ecore_X_Event_Mask event_mask,
                                     int mod,
                                     int any_mod)
{
   unsigned int b;
   unsigned int m;
   unsigned int locks[8];
   int i, ev;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   b = button;
   if (b == 0)
     b = AnyButton;

   m = _ecore_x_event_modifier(mod);
   if (any_mod)
     m = AnyModifier;

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
     XGrabButton(_ecore_x_disp, b, m | locks[i],
                 win, False, ev, GrabModeSync, GrabModeAsync, None, None);
}

EAPI void
ecore_x_window_button_grab(Ecore_X_Window win,
                           int button,
                           Ecore_X_Event_Mask event_mask,
                           int mod,
                           int any_mod)
{
   Wingrab *t;

   _ecore_x_window_button_grab_internal(win, button, event_mask, mod, any_mod);
   _ecore_window_grabs_num++;
   t = realloc(_ecore_window_grabs,
               _ecore_window_grabs_num * sizeof(Wingrab));
   if (!t) return;
   _ecore_window_grabs = t;
   _ecore_window_grabs[_ecore_window_grabs_num - 1].win = win;
   _ecore_window_grabs[_ecore_window_grabs_num - 1].button = button;
   _ecore_window_grabs[_ecore_window_grabs_num - 1].event_mask = event_mask;
   _ecore_window_grabs[_ecore_window_grabs_num - 1].mod = mod;
   _ecore_window_grabs[_ecore_window_grabs_num - 1].any_mod = any_mod;
}

static void
_ecore_x_sync_magic_send(int val, Ecore_X_Window swin, int b, int mod, int anymod)
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
   xev.xclient.data.l[1] = val | (anymod << 8);
   xev.xclient.data.l[2] = swin;
   xev.xclient.data.l[3] = b;
   xev.xclient.data.l[4] = mod;
   XSendEvent(_ecore_x_disp, _ecore_x_private_win, False, NoEventMask, &xev);
}

int
_ecore_x_window_grab_remove(Ecore_X_Window win, int button, int mod, int any_mod)
{
   int i, shuffle = 0;
   Wingrab *t;

   if (_ecore_window_grabs_num > 0)
     {
        for (i = 0; i < _ecore_window_grabs_num; i++)
          {
             if (shuffle)
               _ecore_window_grabs[i - 1] = _ecore_window_grabs[i];

             if ((!shuffle) && (_ecore_window_grabs[i].win == win) &&
                 (((button >= 0) && (_ecore_window_grabs[i].mod == mod) &&
                   (_ecore_window_grabs[i].any_mod == any_mod)) ||
                  (button < 0)))
               shuffle = 1;
          }
        if (shuffle)
          {
             _ecore_window_grabs_num--;
             if (_ecore_window_grabs_num <= 0)
               {
                  free(_ecore_window_grabs);
                  _ecore_window_grabs = NULL;
                  return shuffle;
               }
             t = realloc(_ecore_window_grabs,
                         _ecore_window_grabs_num *
                         sizeof(Wingrab));
             if (!t) return shuffle;
             _ecore_window_grabs = t;
          }
     }
   return shuffle;
}

static void
_ecore_x_window_button_ungrab_internal(Ecore_X_Window win,
                                       int button,
                                       int mod,
                                       int any_mod)
{
   unsigned int b;
   unsigned int m;
   unsigned int locks[8];
   int i;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   b = button;
   if (b == 0)
     b = AnyButton;

   m = _ecore_x_event_modifier(mod);
   if (any_mod)
     m = AnyModifier;

   locks[0] = 0;
   locks[1] = ECORE_X_LOCK_CAPS;
   locks[2] = ECORE_X_LOCK_NUM;
   locks[3] = ECORE_X_LOCK_SCROLL;
   locks[4] = ECORE_X_LOCK_CAPS | ECORE_X_LOCK_NUM;
   locks[5] = ECORE_X_LOCK_CAPS | ECORE_X_LOCK_SCROLL;
   locks[6] = ECORE_X_LOCK_NUM | ECORE_X_LOCK_SCROLL;
   locks[7] = ECORE_X_LOCK_CAPS | ECORE_X_LOCK_NUM | ECORE_X_LOCK_SCROLL;
   for (i = 0; i < 8; i++)
     {
        XUngrabButton(_ecore_x_disp, b, m | locks[i], win);
        if (_ecore_xlib_sync) ecore_x_sync();
     }
}

EAPI void
ecore_x_window_button_ungrab(Ecore_X_Window win,
                             int button,
                             int mod,
                             int any_mod)
{
   _ecore_x_window_button_ungrab_internal(win, button, mod, any_mod);
   _ecore_x_sync_magic_send(1, win, button, mod, any_mod);
//   _ecore_x_window_grab_remove(win, button, mod, any_mod);
}

void _ecore_x_window_grab_suspend(void)
{
   int i;

   for (i = 0; i < _ecore_window_grabs_num; i++)
     {
        _ecore_x_window_button_ungrab_internal
        (_ecore_window_grabs[i].win, _ecore_window_grabs[i].button,
         _ecore_window_grabs[i].mod, _ecore_window_grabs[i].any_mod);
     }
}

void _ecore_x_window_grab_resume(void)
{
   int i;

   for (i = 0; i < _ecore_window_grabs_num; i++)
     {
        _ecore_x_window_button_grab_internal
        (_ecore_window_grabs[i].win, _ecore_window_grabs[i].button,
         _ecore_window_grabs[i].event_mask,
         _ecore_window_grabs[i].mod, _ecore_window_grabs[i].any_mod);
     }
}








//////////////////////////////////////////////////////////////////////////////

int _ecore_key_grabs_num = 0;
typedef struct _Keygrab Keygrab;
struct _Keygrab
{
   Window win;
   char *key;
   int mod, any_mod;
};
Keygrab *_ecore_key_grabs = NULL;

static void
_ecore_x_window_key_grab_internal(Ecore_X_Window win,
                                  const char *key,
                                  int mod,
                                  int any_mod)
{
   KeyCode keycode = 0;
   KeySym keysym;
   unsigned int m;
   unsigned int locks[8];
   int i;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   if (!strncmp(key, "Keycode-", 8))
     keycode = atoi(key + 8);
   else
     {
        keysym = XStringToKeysym(key);
        if (keysym == NoSymbol)
          return;

        keycode = XKeysymToKeycode(_ecore_x_disp, keysym);
     }

   if (keycode == 0)
     return;

   m = _ecore_x_event_modifier(mod);
   if (any_mod)
     m = AnyModifier;

   locks[0] = 0;
   locks[1] = ECORE_X_LOCK_CAPS;
   locks[2] = ECORE_X_LOCK_NUM;
   locks[3] = ECORE_X_LOCK_SCROLL;
   locks[4] = ECORE_X_LOCK_CAPS | ECORE_X_LOCK_NUM;
   locks[5] = ECORE_X_LOCK_CAPS | ECORE_X_LOCK_SCROLL;
   locks[6] = ECORE_X_LOCK_NUM | ECORE_X_LOCK_SCROLL;
   locks[7] = ECORE_X_LOCK_CAPS | ECORE_X_LOCK_NUM | ECORE_X_LOCK_SCROLL;
   for (i = 0; i < 8; i++)
     {
        XGrabKey(_ecore_x_disp, keycode, m | locks[i],
                 win, False, GrabModeSync, GrabModeAsync);
        if (_ecore_xlib_sync) ecore_x_sync();
     }
}

EAPI void
ecore_x_window_key_grab(Ecore_X_Window win,
                        const char *key,
                        int mod,
                        int any_mod)
{
   Keygrab *t;

   _ecore_x_window_key_grab_internal(win, key, mod, any_mod);
   _ecore_key_grabs_num++;
   t = realloc(_ecore_key_grabs,
               _ecore_key_grabs_num * sizeof(Keygrab));
   if (!t) return;
   _ecore_key_grabs = t;
   _ecore_key_grabs[_ecore_key_grabs_num - 1].win = win;
   _ecore_key_grabs[_ecore_key_grabs_num - 1].key = strdup(key);
   _ecore_key_grabs[_ecore_key_grabs_num - 1].mod = mod;
   _ecore_key_grabs[_ecore_key_grabs_num - 1].any_mod = any_mod;
}

int
_ecore_x_key_grab_remove(Ecore_X_Window win,
                         const char *key,
                         int mod,
                         int any_mod)
{
   int i, shuffle = 0;
   Keygrab *t;

   if (_ecore_key_grabs_num > 0)
     {
        for (i = 0; i < _ecore_key_grabs_num; i++)
          {
             if (shuffle)
               _ecore_key_grabs[i - 1] = _ecore_key_grabs[i];

             if ((!shuffle) && (_ecore_key_grabs[i].win == win) &&
                 ((key && ((!strcmp(_ecore_key_grabs[i].key, key)) &&
                           (_ecore_key_grabs[i].mod == mod) &&
                           (_ecore_key_grabs[i].any_mod == any_mod))) ||
                  (!key)))
               {
                  free(_ecore_key_grabs[i].key);
                  _ecore_key_grabs[i].key = NULL;
                  shuffle = 1;
               }
          }
        if (shuffle)
          {
             _ecore_key_grabs_num--;
             if (_ecore_key_grabs_num <= 0)
               {
                  free(_ecore_key_grabs);
                  _ecore_key_grabs = NULL;
                  return shuffle;
               }
             t = realloc(_ecore_key_grabs,
                         _ecore_key_grabs_num * sizeof(Keygrab));
             if (!t) return shuffle;
             _ecore_key_grabs = t;
          }
     }
   return shuffle;
}

static void
_ecore_x_window_key_ungrab_internal(Ecore_X_Window win,
                                    const char *key,
                                    int mod,
                                    int any_mod)
{
   KeyCode keycode = 0;
   KeySym keysym;
   unsigned int m;
   unsigned int locks[8];
   int i;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   if (!strncmp(key, "Keycode-", 8))
     keycode = atoi(key + 8);
   else
     {
        keysym = XStringToKeysym(key);
        if (keysym == NoSymbol)
          return;

        keycode = XKeysymToKeycode(_ecore_x_disp, keysym);
     }

   if (keycode == 0)
     return;

   m = _ecore_x_event_modifier(mod);
   if (any_mod)
     m = AnyModifier;

   locks[0] = 0;
   locks[1] = ECORE_X_LOCK_CAPS;
   locks[2] = ECORE_X_LOCK_NUM;
   locks[3] = ECORE_X_LOCK_SCROLL;
   locks[4] = ECORE_X_LOCK_CAPS | ECORE_X_LOCK_NUM;
   locks[5] = ECORE_X_LOCK_CAPS | ECORE_X_LOCK_SCROLL;
   locks[6] = ECORE_X_LOCK_NUM | ECORE_X_LOCK_SCROLL;
   locks[7] = ECORE_X_LOCK_CAPS | ECORE_X_LOCK_NUM | ECORE_X_LOCK_SCROLL;
   for (i = 0; i < 8; i++)
     XUngrabKey(_ecore_x_disp, keycode, m | locks[i], win);
}

EAPI void
ecore_x_window_key_ungrab(Ecore_X_Window win,
                          const char *key,
                          int mod,
                          int any_mod)
{
   _ecore_x_window_key_ungrab_internal(win, key, mod, any_mod);
   _ecore_x_sync_magic_send(2, win, XStringToKeysym(key), mod, any_mod);
//   _ecore_x_key_grab_remove(win, key, mod, any_mod);
}

void
_ecore_x_key_grab_suspend(void)
{
   int i;

   for (i = 0; i < _ecore_key_grabs_num; i++)
     {
        _ecore_x_window_key_ungrab_internal
        (_ecore_key_grabs[i].win, _ecore_key_grabs[i].key,
         _ecore_key_grabs[i].mod, _ecore_key_grabs[i].any_mod);
     }
}

void
_ecore_x_key_grab_resume(void)
{
   int i;

   for (i = 0; i < _ecore_key_grabs_num; i++)
     {
        _ecore_x_window_key_grab_internal
        (_ecore_key_grabs[i].win, _ecore_key_grabs[i].key,
         _ecore_key_grabs[i].mod, _ecore_key_grabs[i].any_mod);
     }
}








/**
 * Send client message with given type and format 32.
 *
 * @param win     The window the message is sent to.
 * @param type    The client message type.
 * @param mask    The mask of the message to be sent.
 * @param d0      The client message data item 1
 * @param d1      The client message data item 2
 * @param d2      The client message data item 3
 * @param d3      The client message data item 4
 * @param d4      The client message data item 5
 *
 * @return @c EINA_TRUE on success @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool
ecore_x_client_message32_send(Ecore_X_Window win,
                              Ecore_X_Atom type,
                              Ecore_X_Event_Mask mask,
                              long d0,
                              long d1,
                              long d2,
                              long d3,
                              long d4)
{
   XEvent xev;
   Eina_Bool ret;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   xev.xclient.window = win;
   xev.xclient.type = ClientMessage;
   xev.xclient.message_type = type;
   xev.xclient.format = 32;
   xev.xclient.data.l[0] = d0;
   xev.xclient.data.l[1] = d1;
   xev.xclient.data.l[2] = d2;
   xev.xclient.data.l[3] = d3;
   xev.xclient.data.l[4] = d4;

   ret = !!XSendEvent(_ecore_x_disp, win, False, mask, &xev);
   if (_ecore_xlib_sync) ecore_x_sync();
   return ret;
}

/**
 * Send client message with given type and format 8.
 *
 * @param win     The window the message is sent to.
 * @param type    The client message type.
 * @param data    Data to be sent.
 * @param len     Number of data bytes, max @c 20.
 *
 * @return @c EINA_TRUE on success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool
ecore_x_client_message8_send(Ecore_X_Window win,
                             Ecore_X_Atom type,
                             const void *data,
                             int len)
{
   XEvent xev;
   Eina_Bool ret;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   xev.xclient.window = win;
   xev.xclient.type = ClientMessage;
   xev.xclient.message_type = type;
   xev.xclient.format = 8;
   if (len > 20)
     len = 20;

   memcpy(xev.xclient.data.b, data, len);
   if (len < 20)
     memset(xev.xclient.data.b + len, 0, 20 - len);

   ret = !!XSendEvent(_ecore_x_disp, win, False, NoEventMask, &xev);
   if (_ecore_xlib_sync) ecore_x_sync();
   return ret;
}

EAPI Eina_Bool
ecore_x_mouse_move_send(Ecore_X_Window win,
                        int x,
                        int y)
{
   XEvent xev;
   XWindowAttributes att;
   Window tw;
   int rx, ry;
   Eina_Bool ret;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
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
   ret = !!XSendEvent(_ecore_x_disp, win, True, PointerMotionMask, &xev);
   if (_ecore_xlib_sync) ecore_x_sync();
   return ret;
}

EAPI Eina_Bool
ecore_x_mouse_down_send(Ecore_X_Window win,
                        int x,
                        int y,
                        int b)
{
   XEvent xev;
   XWindowAttributes att;
   Window tw;
   int rx, ry;
   Eina_Bool ret;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
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
   ret = !!XSendEvent(_ecore_x_disp, win, True, ButtonPressMask, &xev);
   if (_ecore_xlib_sync) ecore_x_sync();
   return ret;
}

EAPI Eina_Bool
ecore_x_mouse_up_send(Ecore_X_Window win,
                      int x,
                      int y,
                      int b)
{
   XEvent xev;
   XWindowAttributes att;
   Window tw;
   int rx, ry;
   Eina_Bool ret;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
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
   ret = !!XSendEvent(_ecore_x_disp, win, True, ButtonReleaseMask, &xev);
   if (_ecore_xlib_sync) ecore_x_sync();
   return ret;
}

EAPI Eina_Bool
ecore_x_mouse_in_send(Ecore_X_Window win,
                      int x,
                      int y)
{
   XEvent xev;
   XWindowAttributes att;
   Window tw;
   int rx, ry;
   Eina_Bool ret;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   XGetWindowAttributes(_ecore_x_disp, win, &att);
   XTranslateCoordinates(_ecore_x_disp, win, att.root, x, y, &rx, &ry, &tw);
   xev.xcrossing.type = EnterNotify;
   xev.xcrossing.window = win;
   xev.xcrossing.root = att.root;
   xev.xcrossing.subwindow = win;
   xev.xcrossing.time = _ecore_x_event_last_time;
   xev.xcrossing.x = x;
   xev.xcrossing.y = y;
   xev.xcrossing.x_root = rx;
   xev.xcrossing.y_root = ry;
   xev.xcrossing.mode = NotifyNormal;
   xev.xcrossing.detail = NotifyNonlinear;
   xev.xcrossing.same_screen = 1;
   xev.xcrossing.focus = 0;
   xev.xcrossing.state = 0;
   ret = !!XSendEvent(_ecore_x_disp, win, True, EnterWindowMask, &xev);
   if (_ecore_xlib_sync) ecore_x_sync();
   return ret;
}

EAPI Eina_Bool
ecore_x_mouse_out_send(Ecore_X_Window win,
                      int x,
                      int y)
{
   XEvent xev;
   XWindowAttributes att;
   Window tw;
   int rx, ry;
   Eina_Bool ret;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   XGetWindowAttributes(_ecore_x_disp, win, &att);
   XTranslateCoordinates(_ecore_x_disp, win, att.root, x, y, &rx, &ry, &tw);
   xev.xcrossing.type = LeaveNotify;
   xev.xcrossing.window = win;
   xev.xcrossing.root = att.root;
   xev.xcrossing.subwindow = win;
   xev.xcrossing.time = _ecore_x_event_last_time;
   xev.xcrossing.x = x;
   xev.xcrossing.y = y;
   xev.xcrossing.x_root = rx;
   xev.xcrossing.y_root = ry;
   xev.xcrossing.mode = NotifyNormal;
   xev.xcrossing.detail = NotifyNonlinear;
   xev.xcrossing.same_screen = 1;
   xev.xcrossing.focus = 0;
   xev.xcrossing.state = 0;
   ret = !!XSendEvent(_ecore_x_disp, win, True, LeaveWindowMask, &xev);
   if (_ecore_xlib_sync) ecore_x_sync();
   return ret;
}

EAPI void
ecore_x_focus_reset(void)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   XSetInputFocus(_ecore_x_disp, PointerRoot, RevertToPointerRoot, CurrentTime);
   if (_ecore_xlib_sync) ecore_x_sync();
}

EAPI void
ecore_x_events_allow_all(void)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   XAllowEvents(_ecore_x_disp, AsyncBoth, CurrentTime);
   if (_ecore_xlib_sync) ecore_x_sync();
}

EAPI void
ecore_x_pointer_last_xy_get(int *x,
                            int *y)
{
   if (x)
     *x = _ecore_x_event_last_root_x;

   if (y)
     *y = _ecore_x_event_last_root_y;
}

EAPI void
ecore_x_pointer_xy_get(Ecore_X_Window win,
                       int *x,
                       int *y)
{
   Window rwin, cwin;
   int rx, ry, wx, wy, ret;
   unsigned int mask;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   ret = XQueryPointer(_ecore_x_disp, win, &rwin, &cwin,
                       &rx, &ry, &wx, &wy, &mask);
   if (!ret)
     wx = wy = -1;

   if (x) *x = wx;
   if (y) *y = wy;
   if (_ecore_xlib_sync) ecore_x_sync();
}

EAPI void
ecore_x_pointer_root_xy_get(int *x, int *y)
{
   Ecore_X_Window *root;
   Window rwin, cwin;
   int rx, ry, wx, wy, ret = 0;
   int i, num;
   unsigned int mask;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   root = ecore_x_window_root_list(&num);
   for (i = 0; i < num; i++) 
     {
        ret = XQueryPointer(_ecore_x_disp, root[i], &rwin, &cwin,
                            &rx, &ry, &wx, &wy, &mask);
        if (_ecore_xlib_sync) ecore_x_sync();
        if (ret) break;
     }

   if (!ret)
     rx = ry = -1;

   if (x) *x = rx;
   if (y) *y = ry;
   free(root);
}

/**
 * Retrieve the Visual ID from a given Visual.
 *
 * @param visual  The Visual to get the ID for.
 *
 * @return The visual id.
 * @since 1.1.0
 */
EAPI unsigned int
ecore_x_visual_id_get(Ecore_X_Visual visual)
{
   unsigned int vis;
   vis = XVisualIDFromVisual(visual);
   if (_ecore_xlib_sync) ecore_x_sync();
   return vis;
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
ecore_x_default_visual_get(Ecore_X_Display *disp,
                           Ecore_X_Screen *screen)
{
   Ecore_X_Visual vis = DefaultVisual(disp, ecore_x_screen_index_get(screen));
   if (_ecore_xlib_sync) ecore_x_sync();
   return vis;
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
ecore_x_default_colormap_get(Ecore_X_Display *disp,
                             Ecore_X_Screen *screen)
{
   Ecore_X_Colormap col = DefaultColormap(disp, ecore_x_screen_index_get(screen));
   if (_ecore_xlib_sync) ecore_x_sync();
   return col;
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
ecore_x_default_depth_get(Ecore_X_Display *disp,
                          Ecore_X_Screen *screen)
{
   int depth = DefaultDepth(disp, ecore_x_screen_index_get(screen));
   if (_ecore_xlib_sync) ecore_x_sync();
   return depth;
}

EAPI void
ecore_x_xkb_select_group(int group)
{
#ifdef ECORE_XKB
   XkbLockGroup(_ecore_x_disp, XkbUseCoreKbd, group);
   if (_ecore_xlib_sync) ecore_x_sync();
#endif
}

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

static int
_ecore_x_event_modifier(unsigned int state)
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

   if (state & ECORE_EVENT_MODIFIER_ALTGR)
     xmodifiers |= ECORE_X_MODIFIER_ALTGR;

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

