#ifndef _ECORE_X_PRIVATE_H
#define _ECORE_X_PRIVATE_H

#include <sys/param.h>
#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN 256
#endif

#define XK_MISCELLANY 1
#include <X11/Xlib.h>
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
#endif
#ifdef ECORE_XPRINT
#include <X11/extensions/Print.h>
#endif
#ifdef ECORE_XINERAMA
#include <X11/extensions/Xinerama.h>
#endif
#ifdef ECORE_XRANDR
#include <X11/extensions/Xrandr.h>
#endif
#ifdef ECORE_XSS
#include <X11/extensions/scrnsaver.h>
#endif
#ifdef ECORE_XRENDER
#include <X11/extensions/Xrender.h>
#endif
#ifdef ECORE_XFIXES
#include <X11/extensions/Xfixes.h>
#endif
#ifdef ECORE_XCOMPOSITE
#include <X11/extensions/Xcomposite.h>
#endif
#ifdef ECORE_XDAMAGE
#include <X11/extensions/Xdamage.h>
#endif
#ifdef ECORE_XDPMS
#include <X11/extensions/dpms.h>
#endif
#ifdef ECORE_XKB
#include <X11/XKBlib.h>
#endif

#include "ecore_private.h"
#include "Ecore_X.h"

/* FIXME: this is for simulation only */
#include "Ecore_Txt.h"

typedef struct _Ecore_X_Selection_Intern Ecore_X_Selection_Intern;

struct _Ecore_X_Selection_Intern
{
   Ecore_X_Window    win;
   Ecore_X_Atom      selection;
   unsigned char     *data;
   int               length;
   Time              time;
};

typedef struct _Ecore_X_Selection_Converter Ecore_X_Selection_Converter;

struct _Ecore_X_Selection_Converter
{
   Ecore_X_Atom target;
   int (*convert)(char *target, void *data, int size, 
                  void **data_ret, int *size_ret);
   Ecore_X_Selection_Converter *next;
};

typedef struct _Ecore_X_Selection_Parser Ecore_X_Selection_Parser;

struct _Ecore_X_Selection_Parser
{
   char     *target;
   void *(*parse)(const char *target, void *data, int size, int format);
   Ecore_X_Selection_Parser *next;
};

typedef struct _Ecore_X_DND_Source 
{
   int version;
   Ecore_X_Window win, dest;
   
   enum {
      ECORE_X_DND_SOURCE_IDLE,
      ECORE_X_DND_SOURCE_DRAGGING,
      ECORE_X_DND_SOURCE_DROPPED,
      ECORE_X_DND_SOURCE_CONVERTING
   } state;

   struct {
      short x, y;
      unsigned short width, height;
   } rectangle;

   struct {
      Ecore_X_Window window;
      int x, y;
   } prev;

   Time time;

   Ecore_X_Atom action, accepted_action;
   
   int will_accept;
   int suppress;

   int await_status;
} Ecore_X_DND_Source;

typedef struct _Ecore_X_DND_Target 
{
   int version;
   Ecore_X_Window win, source;
   
   enum {
      ECORE_X_DND_TARGET_IDLE,
      ECORE_X_DND_TARGET_ENTERED
   } state;

   struct {
      int x, y;
   } pos;
   
   Time time;

   Ecore_X_Atom action, accepted_action;
   
   int will_accept;
} Ecore_X_DND_Target;


extern int ECORE_X_MODIFIER_SHIFT;
extern int ECORE_X_MODIFIER_CTRL;
extern int ECORE_X_MODIFIER_ALT;
extern int ECORE_X_MODIFIER_WIN;

extern int ECORE_X_LOCK_SCROLL;
extern int ECORE_X_LOCK_NUM;
extern int ECORE_X_LOCK_CAPS;

extern Display *_ecore_x_disp;
extern double   _ecore_x_double_click_time;
extern Time     _ecore_x_event_last_time;
extern Window   _ecore_x_event_last_win;
extern int      _ecore_x_event_last_root_x;
extern int      _ecore_x_event_last_root_y;
extern int      _ecore_x_xcursor;
extern XIC      _ecore_x_ic;

extern Ecore_X_Atom     _ecore_x_atoms_wm_protocols[ECORE_X_WM_PROTOCOL_NUM];

extern int      _ecore_window_grabs_num;
extern Window  *_ecore_window_grabs;
extern int    (*_ecore_window_grab_replay_func) (void *data, int event_type, void *event);
extern void    *_ecore_window_grab_replay_data;

extern Ecore_X_Window _ecore_x_private_win;

void _ecore_x_error_handler_init(void);
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
void _ecore_x_event_handle_sync_counter(XEvent *xevent);
void _ecore_x_event_handle_sync_alarm(XEvent *xevent);
#ifdef ECORE_XRANDR
void _ecore_x_event_handle_randr_change(XEvent *xevent);
#endif
#ifdef ECORE_XFIXES
void _ecore_x_event_handle_fixes_selection_notify(XEvent *xevent);
#endif
#ifdef ECORE_XDAMAGE
void _ecore_x_event_handle_damage_notify(XEvent *xevent);
#endif

void  _ecore_x_selection_data_init(void);
void  _ecore_x_selection_shutdown(void);
Ecore_X_Atom
      _ecore_x_selection_target_atom_get(const char *target);
char *_ecore_x_selection_target_get(Ecore_X_Atom target);
Ecore_X_Selection_Intern * 
      _ecore_x_selection_get(Ecore_X_Atom selection);
int   _ecore_x_selection_set(Window w, const void *data, int len, Ecore_X_Atom selection);
int   _ecore_x_selection_convert(Ecore_X_Atom selection, Ecore_X_Atom target, void **data_ret);
void *_ecore_x_selection_parse(const char *target, void *data, int size, int format);

void _ecore_x_sync_magic_send(int val, Ecore_X_Window swin);
void _ecore_x_window_grab_remove(Ecore_X_Window win);
void _ecore_x_key_grab_remove(Ecore_X_Window win);

/* from dnd */
void _ecore_x_dnd_init(void);
Ecore_X_DND_Source *_ecore_x_dnd_source_get(void);
Ecore_X_DND_Target *_ecore_x_dnd_target_get(void);
void _ecore_x_dnd_drag(Ecore_X_Window root, int x, int y);
void _ecore_x_dnd_shutdown(void);

/* from netwm */
Ecore_X_Window_State _ecore_x_netwm_state_get(Ecore_X_Atom a);
int                  _ecore_x_netwm_startup_info_begin(Ecore_X_Window win, char *data);
int                  _ecore_x_netwm_startup_info(Ecore_X_Window win, char *data);

/* Fixes * Damage * Composite * DPMS */
void _ecore_x_fixes_init(void);
void _ecore_x_damage_init(void);
void _ecore_x_composite_init(void);
void _ecore_x_dpms_init(void);

void _ecore_x_atoms_init(void);
    
/* from sync */

#endif
