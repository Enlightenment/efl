#ifndef _ECORE_X_PRIVATE_H
#define _ECORE_X_PRIVATE_H

#include <sys/param.h>
#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN 256
#endif /* ifndef MAXHOSTNAMELEN */

#include <X11/Xlib.h>
#include <X11/Xlib-xcb.h>
#include <X11/Xproto.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xresource.h>
#include <X11/keysymdef.h>
#include <X11/extensions/XShm.h>
#include <X11/extensions/shape.h>
#include <X11/extensions/sync.h>
#include <X11/extensions/dpms.h>
#ifdef ECORE_XCURSOR
#include <X11/Xcursor/Xcursor.h>
#endif /* ifdef ECORE_XCURSOR */
#ifdef ECORE_XINERAMA
#include <X11/extensions/Xinerama.h>
#endif /* ifdef ECORE_XINERAMA */
#ifdef ECORE_XRANDR
#include <X11/extensions/Xrandr.h>
#endif /* ifdef ECORE_XRANDR */
#ifdef ECORE_XSS
#include <X11/extensions/scrnsaver.h>
#endif /* ifdef ECORE_XSS */
#ifdef ECORE_XRENDER
#include <X11/extensions/Xrender.h>
#endif /* ifdef ECORE_XRENDER */
#ifdef ECORE_XFIXES
#include <X11/extensions/Xfixes.h>
#endif /* ifdef ECORE_XFIXES */
#ifdef ECORE_XCOMPOSITE
#include <X11/extensions/Xcomposite.h>
#endif /* ifdef ECORE_XCOMPOSITE */
#ifdef ECORE_XPRESENT
#include <X11/extensions/Xpresent.h>
#endif /* ifdef ECORE_XPRESENT */
#ifdef ECORE_XDAMAGE
#include <X11/extensions/Xdamage.h>
#endif /* ifdef ECORE_XDAMAGE */
#ifdef ECORE_XGESTURE
#include <X11/extensions/gesture.h>
#include <X11/extensions/gestureproto.h>
#endif /* ifdef ECORE_XGESTURE */
#ifdef ECORE_XDPMS
#include <X11/extensions/dpms.h>
#endif /* ifdef ECORE_XDPMS */
#ifdef ECORE_XKB
#include <X11/XKBlib.h>
#endif /* ifdef ECORE_XKB */
#ifdef ECORE_XI2
#include <X11/extensions/XInput2.h>
#endif /* ifdef ECORE_XI2 */

#ifndef XK_MISCELLANY
# define XK_MISCELLANY 1
#endif

#include "Ecore.h"
#include "ecore_private.h"
#include "Ecore_X.h"
#include "Ecore_Input.h"

extern int _ecore_xlib_log_dom;
extern Eina_Bool _ecore_xlib_sync;
#ifdef ECORE_XLIB_DEFAULT_LOG_COLOR
# undef ECORE_XLIB_DEFAULT_LOG_COLOR
#endif /* ifdef ECORE_XLIB_DEFAULT_LOG_COLOR */
#define ECORE_XLIB_DEFAULT_LOG_COLOR EINA_COLOR_BLUE

#ifdef ERR
# undef ERR
#endif /* ifdef ERR */
#define ERR(...) EINA_LOG_DOM_ERR(_ecore_xlib_log_dom, __VA_ARGS__)

#ifdef DBG
# undef DBG
#endif /* ifdef DBG */
#define DBG(...) EINA_LOG_DOM_DBG(_ecore_xlib_log_dom, __VA_ARGS__)

#ifdef INF
# undef INF
#endif /* ifdef INF */
#define INF(...) EINA_LOG_DOM_INFO(_ecore_xlib_log_dom, __VA_ARGS__)

#ifdef WRN
# undef WRN
#endif /* ifdef WRN */
#define WRN(...) EINA_LOG_DOM_WARN(_ecore_xlib_log_dom, __VA_ARGS__)

#ifdef CRI
# undef CRI
#endif /* ifdef CRI */
#define CRI(...) EINA_LOG_DOM_CRIT(_ecore_xlib_log_dom, __VA_ARGS__)

typedef struct _Ecore_X_Selection_Intern Ecore_X_Selection_Intern;

struct _Ecore_X_Selection_Intern
{
   Ecore_X_Window win;
   Ecore_X_Atom   selection;
   unsigned char *data;
   int            length;
   Time           time;
};

typedef struct _Ecore_X_Selection_Converter Ecore_X_Selection_Converter;

struct _Ecore_X_Selection_Converter
{
   EINA_INLIST;
   Ecore_X_Atom                 target;
   Eina_Bool                    (*convert)(char *target, void *data, int size, void **data_ret, int *size_ret, Ecore_X_Atom *type, int *typeseize);
};

typedef struct _Ecore_X_Selection_Parser Ecore_X_Selection_Parser;

struct _Ecore_X_Selection_Parser
{
   EINA_INLIST;
   char                     *target;
   void                     *(*parse)(const char *target, void *data, int size, int format);
};

typedef struct _Ecore_X_DND_Source
{
   int            version;
   Ecore_X_Window win, dest;

   enum {
      ECORE_X_DND_SOURCE_IDLE,
      ECORE_X_DND_SOURCE_DRAGGING,
      ECORE_X_DND_SOURCE_DROPPED,
      ECORE_X_DND_SOURCE_CONVERTING
   } state;

   struct
   {
      short          x, y;
      unsigned short width, height;
   } rectangle;

   struct
   {
      Ecore_X_Window window;
      int            x, y;
   } prev;

   Time         time;

   Ecore_X_Atom action, accepted_action;

   int          will_accept;
   int          suppress;

   int          await_status;
} Ecore_X_DND_Source;

typedef struct _Ecore_X_DND_Target
{
   int            version;
   Ecore_X_Window win, source;

   enum {
      ECORE_X_DND_TARGET_IDLE,
      ECORE_X_DND_TARGET_ENTERED
   } state;

   struct
   {
      int x, y;
   } pos;

   Time         time;

   Ecore_X_Atom action, accepted_action;

   int          will_accept;
} Ecore_X_DND_Target;

extern Display *_ecore_x_disp;
extern double _ecore_x_double_click_time;
extern Time _ecore_x_event_last_time;
extern Window _ecore_x_event_last_win;
extern int _ecore_x_event_last_root_x;
extern int _ecore_x_event_last_root_y;
extern Eina_Bool _ecore_x_xcursor;

extern Ecore_X_Atom _ecore_x_atoms_wm_protocols[ECORE_X_WM_PROTOCOL_NUM];

extern int _ecore_window_grabs_num;
typedef struct _Wingrab Wingrab;
struct _Wingrab
{
   Window win;
   Ecore_X_Event_Mask event_mask;
   int mod, any_mod;
   int button;
};
extern Wingrab *_ecore_window_grabs;

extern Eina_Bool (*_ecore_window_grab_replay_func)(void *data,
                                                   int event_type,
                                                   void *event);
extern void *_ecore_window_grab_replay_data;

extern Ecore_X_Window _ecore_x_private_win;

void _ecore_x_error_handler_init(void);
void _ecore_x_event_handle_any_event(XEvent *xevent);
void _ecore_x_event_handle_key_press(XEvent *xevent);
void _ecore_x_event_handle_key_release(XEvent *xevent);
void _ecore_x_event_handle_button_press(XEvent *xevent);
void _ecore_x_event_handle_button_release(XEvent *xevent);
void _ecore_x_event_handle_motion_notify(XEvent *xevent);
void _ecore_x_event_handle_enter_notify(XEvent *xevent);
void _ecore_x_event_handle_leave_notify(XEvent *xevent);
void _ecore_x_event_handle_focus_in(XEvent *xevent);
void _ecore_x_event_handle_focus_out(XEvent *xevent);
void _ecore_x_event_handle_keymap_notify(XEvent *xevent);
void _ecore_x_event_handle_expose(XEvent *xevent);
void _ecore_x_event_handle_graphics_expose(XEvent *xevent);
void _ecore_x_event_handle_visibility_notify(XEvent *xevent);
void _ecore_x_event_handle_create_notify(XEvent *xevent);
void _ecore_x_event_handle_destroy_notify(XEvent *xevent);
void _ecore_x_event_handle_unmap_notify(XEvent *xevent);
void _ecore_x_event_handle_map_notify(XEvent *xevent);
void _ecore_x_event_handle_map_request(XEvent *xevent);
void _ecore_x_event_handle_reparent_notify(XEvent *xevent);
void _ecore_x_event_handle_configure_notify(XEvent *xevent);
void _ecore_x_event_handle_configure_request(XEvent *xevent);
void _ecore_x_event_handle_gravity_notify(XEvent *xevent);
void _ecore_x_event_handle_resize_request(XEvent *xevent);
void _ecore_x_event_handle_circulate_notify(XEvent *xevent);
void _ecore_x_event_handle_circulate_request(XEvent *xevent);
void _ecore_x_event_handle_property_notify(XEvent *xevent);
void _ecore_x_event_handle_selection_clear(XEvent *xevent);
void _ecore_x_event_handle_selection_request(XEvent *xevent);
void _ecore_x_event_handle_selection_notify(XEvent *xevent);
void _ecore_x_event_handle_colormap_notify(XEvent *xevent);
void _ecore_x_event_handle_client_message(XEvent *xevent);
void _ecore_x_event_handle_mapping_notify(XEvent *xevent);
void _ecore_x_event_handle_shape_change(XEvent *xevent);
void _ecore_x_event_handle_screensaver_notify(XEvent *xevent);
#ifdef ECORE_XGESTURE
void _ecore_x_event_handle_gesture_notify_flick(XEvent *xevent);
void _ecore_x_event_handle_gesture_notify_pan(XEvent *xevent);
void _ecore_x_event_handle_gesture_notify_pinchrotation(XEvent *xevent);
void _ecore_x_event_handle_gesture_notify_tap(XEvent *xevent);
void _ecore_x_event_handle_gesture_notify_tapnhold(XEvent *xevent);
void _ecore_x_event_handle_gesture_notify_hold(XEvent *xevent);
void _ecore_x_event_handle_gesture_notify_group(XEvent *xevent);
#endif /* ifdef ECORE_XGESTURE */
void _ecore_x_event_handle_sync_counter(XEvent *xevent);
void _ecore_x_event_handle_sync_alarm(XEvent *xevent);
#ifdef ECORE_XRANDR
void _ecore_x_event_handle_randr_change(XEvent *xevent);
void _ecore_x_event_handle_randr_notify(XEvent *xevent);
#endif /* ifdef ECORE_XRANDR */
#ifdef ECORE_XFIXES
void _ecore_x_event_handle_fixes_selection_notify(XEvent *xevent);
#endif /* ifdef ECORE_XFIXES */
#ifdef ECORE_XDAMAGE
void      _ecore_x_event_handle_damage_notify(XEvent *xevent);
#endif /* ifdef ECORE_XDAMAGE */
#ifdef ECORE_XKB
void   _ecore_x_event_handle_xkb(XEvent *xevent);
#endif  /* ifdef ECORE_XKB */
void      _ecore_x_event_handle_generic_event(XEvent *xevent);

#ifdef ECORE_XPRESENT
void      _ecore_x_present_handler(XGenericEvent *ge);
#endif

void      _ecore_x_selection_data_init(void);
void      _ecore_x_selection_shutdown(void);
Ecore_X_Atom _ecore_x_selection_target_atom_get(const char *target);
char     *_ecore_x_selection_target_get(Ecore_X_Atom target);
Ecore_X_Selection_Intern *_ecore_x_selection_get(Ecore_X_Atom selection);
Eina_Bool _ecore_x_selection_set(Window w,
                                  const void *data,
                                  int len,
                                  Ecore_X_Atom selection);
int _ecore_x_selection_convert(Ecore_X_Atom selection,
                               Ecore_X_Atom target,
                               void **data_ret,
                               Ecore_X_Atom *targettype,
                               int *targetsize);
void *_ecore_x_selection_parse(const char *target,
                               void *data,
                               int size,
                               int format);

int _ecore_x_window_grab_remove(Ecore_X_Window win, int button, int mod, int any_mod);
int _ecore_x_key_grab_remove(Ecore_X_Window win, const char *key, int mod, int any_mod);
void _ecore_x_window_grab_suspend(void);
void _ecore_x_window_grab_resume(void);
void _ecore_x_key_grab_suspend(void);
void _ecore_x_key_grab_resume(void);

/* from dnd */
void                _ecore_x_dnd_init(void);
Ecore_X_DND_Source *_ecore_x_dnd_source_get(void);
Ecore_X_DND_Target *_ecore_x_dnd_target_get(void);
void                _ecore_x_dnd_drag(Ecore_X_Window root,
                                      int x,
                                      int y);
void _ecore_x_dnd_shutdown(void);

/* from netwm */
Ecore_X_Window_State _ecore_x_netwm_state_get(Ecore_X_Atom a);
int                  _ecore_x_netwm_startup_info_begin(Ecore_X_Window win,
                                                       char *data);
int                  _ecore_x_netwm_startup_info(Ecore_X_Window win,
                                                 char *data);

/* Fixes * Damage * Composite * DPMS */
void _ecore_x_fixes_init(void);
void _ecore_x_damage_init(void);
void _ecore_x_composite_init(void);
void _ecore_x_present_init(void);
void _ecore_x_dpms_init(void);
void _ecore_x_randr_init(void);
void _ecore_x_gesture_init(void);

void _ecore_x_atoms_init(void);

extern int _ecore_x_present_major;
extern int _ecore_x_xi2_opcode;

void _ecore_x_events_init(void);
void _ecore_x_events_shutdown(void);

void _ecore_x_input_init(void);
void _ecore_x_input_shutdown(void);
void _ecore_x_input_handler(XEvent *xevent);
/* from sync */

void
_ecore_x_axis_update(Ecore_Window window,
                     Ecore_Window event_window,
                     Ecore_Window root_window,
                     unsigned int timestamp,
                     int devid,
                     int toolid,
                     int naxis,
                     Ecore_Axis *axis);

void _ecore_mouse_move(unsigned int timestamp,
                       unsigned int xmodifiers,
                       int x,
                       int y,
                       int x_root,
                       int y_root,
                       unsigned int event_window,
                       unsigned int window,
                       unsigned int root_win,
                       int same_screen,
                       int dev,
                       double radx,
                       double rady,
                       double pressure,
                       double angle,
                       double mx,
                       double my,
                       double mrx,
                       double mry);
Ecore_Event_Mouse_Button *_ecore_mouse_button(int event,
                                              unsigned int timestamp,
                                              unsigned int xmodifiers,
                                              unsigned int buttons,
                                              int x,
                                              int y,
                                              int x_root,
                                              int y_root,
                                              unsigned int event_window,
                                              unsigned int window,
                                              unsigned int root_win,
                                              int same_screen,
                                              int dev,
                                              double radx,
                                              double rady,
                                              double pressure,
                                              double angle,
                                              double mx,
                                              double my,
                                              double mrx,
                                              double mry);

void _ecore_x_modifiers_get(void);
KeySym _ecore_x_XKeycodeToKeysym(Display *display, KeyCode keycode, int index);

int _ecore_x_shutdown(void);
//#define LOGFNS 1

#ifdef LOGFNS
#include <stdio.h>
#define LOGFN(fl, ln, fn) printf("-ECORE-X: %25s: %5i - %s\n", fl, ln, fn);
#else /* ifdef LOGFNS */
#define LOGFN(fl, ln, fn)
#endif /* ifdef LOGFNS */



#ifdef EAPI
# undef EAPI
#endif // ifdef EAPI

#ifdef __GNUC__
# if __GNUC__ >= 4
#  define EAPI __attribute__ ((visibility("default")))
# else // if __GNUC__ >= 4
#  define EAPI
# endif // if __GNUC__ >= 4
#else // ifdef __GNUC__
# define EAPI
#endif // ifdef __GNUC__
EAPI void ecore_x_window_root_properties_select(void);
#undef EAPI
#define EAPI


#endif /* ifndef _ECORE_X_PRIVATE_H */
