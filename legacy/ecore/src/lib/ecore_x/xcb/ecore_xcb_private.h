#ifndef __ECORE_XCB_PRIVATE_H__
#define __ECORE_XCB_PRIVATE_H__

#include "config.h"

#include <sys/param.h>

#ifndef MAXHOSTNAMELEN
# define MAXHOSTNAMELEN 256
#endif

#ifndef XK_MISCELLANY
# define XK_MISCELLANY 1
#endif /* XK_MISCELLANY */

#include <xcb/xcb.h>
#include <xcb/xcb_icccm.h>
#include <xcb/xcb_keysyms.h>
#include <xcb/bigreq.h>
#ifdef ECORE_XCB_CURSOR
# include <xcb/cursor.h>
#endif /* ECORE_XCB_CURSOR */
#ifdef ECORE_XCB_DAMAGE
# include <xcb/damage.h>
#endif /* ECORE_XCB_DAMAGE */
#ifdef ECORE_XCB_COMPOSITE
# include <xcb/composite.h>
#endif /* ECORE_XCB_COMPOSITE */
#ifdef ECORE_XCB_DPMS
# include <xcb/dpms.h>
#endif /* ECORE_XCB_DPMS */
#ifdef ECORE_XCB_RANDR
# include <xcb/randr.h>
#endif /* ECORE_XCB_RANDR */
#ifdef ECORE_XCB_RENDER
# include <xcb/render.h>
#endif /* ECORE_XCB_RENDER */
#ifdef ECORE_XCB_SCREENSAVER
# include <xcb/screensaver.h>
#endif /* ECORE_XCB_SCREENSAVER */
#ifdef ECORE_XCB_SHAPE
# include <xcb/shape.h>
#endif /* ECORE_XCB_SHAPE */
#ifdef ECORE_XCB_SYNC
# include <xcb/sync.h>
#endif /* ECORE_XCB_SYNC */
#ifdef ECORE_XCB_XFIXES
# include <xcb/xfixes.h>
#endif /* ECORE_XCB_XFIXES */
#ifdef ECORE_XCB_XINERAMA
# include <xcb/xinerama.h>
#endif /* ECORE_XCB_XINERAMA */
#ifdef ECORE_XCB_XPRINT
# include <xcb/xprint.h>
#endif /* ECORE_XCB_XPRINT */

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
   Ecore_X_Time      time;
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
   
   Ecore_X_Time time;

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

   Ecore_X_Time time;

   Ecore_X_Atom action, accepted_action;

   int will_accept;
} Ecore_X_DND_Target;


extern Ecore_X_Connection *_ecore_xcb_conn;
extern Ecore_X_Screen     *_ecore_xcb_screen;
extern double              _ecore_xcb_double_click_time;
extern Ecore_X_Time        _ecore_xcb_event_last_time;
extern Ecore_X_Window      _ecore_xcb_event_last_window;
extern int16_t             _ecore_xcb_event_last_root_x;
extern int16_t             _ecore_xcb_event_last_root_y;
extern int                 _ecore_xcb_xcursor;

extern Ecore_X_Atom        _ecore_xcb_atoms_wm_protocols[ECORE_X_WM_PROTOCOL_NUM];

extern int                 _ecore_window_grabs_num;
extern Ecore_X_Window     *_ecore_window_grabs;
extern int               (*_ecore_window_grab_replay_func) (void *data, int event_type, void *event);
extern void               *_ecore_window_grab_replay_data;

extern Ecore_X_Window      _ecore_xcb_private_window;


void _ecore_x_error_handler_init(void);

void _ecore_x_event_handle_key_press          (xcb_generic_event_t *event);
void _ecore_x_event_handle_key_release        (xcb_generic_event_t *event);
void _ecore_x_event_handle_button_press       (xcb_generic_event_t *event);
void _ecore_x_event_handle_button_release     (xcb_generic_event_t *event);
void _ecore_x_event_handle_motion_notify      (xcb_generic_event_t *event);
void _ecore_x_event_handle_enter_notify       (xcb_generic_event_t *event);
void _ecore_x_event_handle_leave_notify       (xcb_generic_event_t *event);
void _ecore_x_event_handle_focus_in           (xcb_generic_event_t *event);
void _ecore_x_event_handle_focus_out          (xcb_generic_event_t *event);
void _ecore_x_event_handle_keymap_notify      (xcb_generic_event_t *event);
void _ecore_x_event_handle_expose             (xcb_generic_event_t *event);
void _ecore_x_event_handle_graphics_expose    (xcb_generic_event_t *event);
void _ecore_x_event_handle_visibility_notify  (xcb_generic_event_t *event);
void _ecore_x_event_handle_create_notify      (xcb_generic_event_t *event);
void _ecore_x_event_handle_destroy_notify     (xcb_generic_event_t *event);
void _ecore_x_event_handle_unmap_notify       (xcb_generic_event_t *event);
void _ecore_x_event_handle_map_notify         (xcb_generic_event_t *event);
void _ecore_x_event_handle_map_request        (xcb_generic_event_t *event);
void _ecore_x_event_handle_reparent_notify    (xcb_generic_event_t *event);
void _ecore_x_event_handle_configure_notify   (xcb_generic_event_t *event);
void _ecore_x_event_handle_configure_request  (xcb_generic_event_t *event);
void _ecore_x_event_handle_gravity_notify     (xcb_generic_event_t *event);
void _ecore_x_event_handle_resize_request     (xcb_generic_event_t *event);
void _ecore_x_event_handle_circulate_notify   (xcb_generic_event_t *event);
void _ecore_x_event_handle_circulate_request  (xcb_generic_event_t *event);
void _ecore_x_event_handle_property_notify    (xcb_generic_event_t *event);
void _ecore_x_event_handle_selection_clear    (xcb_generic_event_t *event);
void _ecore_x_event_handle_selection_request  (xcb_generic_event_t *event);
void _ecore_x_event_handle_selection_notify   (xcb_generic_event_t *event);
void _ecore_x_event_handle_colormap_notify    (xcb_generic_event_t *event);
void _ecore_x_event_handle_client_message     (xcb_generic_event_t *event);
void _ecore_x_event_handle_mapping_notify     (xcb_generic_event_t *event);
#ifdef ECORE_XCB_DAMAGE
void _ecore_x_event_handle_damage_notify      (xcb_generic_event_t *event);
#endif /* ECORE_XCB_DAMAGE */
#ifdef ECORE_XCB_RANDR
void _ecore_x_event_handle_randr_change       (xcb_generic_event_t *event);
#endif /* ECORE_XCB_RANDR */
#ifdef ECORE_XCB_SCREENSAVER
void _ecore_x_event_handle_screensaver_notify (xcb_generic_event_t *event);
#endif /* ECORE_XCB_SCREENSAVER */
#ifdef ECORE_XCB_SHAPE
void _ecore_x_event_handle_shape_change       (xcb_generic_event_t *event);
#endif /* ECORE_XCB_SHAPE */
#ifdef ECORE_XCB_SYNC
void _ecore_x_event_handle_sync_counter       (xcb_generic_event_t *event);
void _ecore_x_event_handle_sync_alarm         (xcb_generic_event_t *event);
#endif /* ECORE_XCB_SYNC */
#ifdef ECORE_XCB_FIXES
void _ecore_x_event_handle_fixes_selection_notify(xcb_generic_event_t *event);
#endif /* ECORE_XCB_FIXES */


/* requests / replies */
int          _ecore_x_reply_init ();
void         _ecore_x_reply_shutdown ();
void         _ecore_xcb_cookie_cache (unsigned int cookie);
unsigned int _ecore_xcb_cookie_get (void);
void         _ecore_xcb_reply_cache (void *reply);
void        *_ecore_xcb_reply_get (void);


/* atoms */
extern Ecore_X_Atom ECORE_X_ATOM_ATOM;
extern Ecore_X_Atom ECORE_X_ATOM_CARDINAL;
extern Ecore_X_Atom ECORE_X_ATOM_STRING;
extern Ecore_X_Atom ECORE_X_ATOM_WINDOW;
extern Ecore_X_Atom ECORE_X_ATOM_E_FRAME_SIZE;
extern Ecore_X_Atom ECORE_X_ATOM_WM_SIZE_HINTS;

#define ECORE_X_ATOMS_COUNT 115

void _ecore_x_atom_init          (xcb_intern_atom_cookie_t *);
void _ecore_x_atom_init_finalize (xcb_intern_atom_cookie_t *);


/* damage */
void _ecore_x_damage_init          (const xcb_query_extension_reply_t *reply);
void _ecore_x_damage_init_finalize (void);

/* composite */
void _ecore_x_composite_init       (const xcb_query_extension_reply_t *reply);
void _ecore_x_composite_init_finalize (void);

/* from dnd */
void                _ecore_x_dnd_init       (void);
void                _ecore_x_dnd_shutdown   (void);
Ecore_X_DND_Source *_ecore_x_dnd_source_get (void);
Ecore_X_DND_Target *_ecore_x_dnd_target_get (void);
void                _ecore_x_dnd_drag       (Ecore_X_Window root,
                                             int x,
                                             int y);


/* dpms */
void _ecore_x_dpms_init          (const xcb_query_extension_reply_t *reply);
void _ecore_x_dpms_init_finalize (void);


/* netwm */
Ecore_X_Window_State _ecore_x_netwm_state_get(Ecore_X_Atom a);
int                  _ecore_x_netwm_startup_info_begin(Ecore_X_Window win, char *data);
int                  _ecore_x_netwm_startup_info(Ecore_X_Window win, char *data);


/* randr */
void _ecore_x_randr_init          (const xcb_query_extension_reply_t *reply);
void _ecore_x_randr_init_finalize (void);


/* selection */
void                      _ecore_x_selection_init(void);
void                      _ecore_x_selection_shutdown(void);
Ecore_X_Atom              _ecore_x_selection_target_atom_get(const char *target);
char                     *_ecore_x_selection_target_get(Ecore_X_Atom target);
Ecore_X_Selection_Intern *_ecore_x_selection_get(Ecore_X_Atom selection);
int                       _ecore_x_selection_set(Ecore_X_Window w, const void *data, int len, Ecore_X_Atom selection);
int                       _ecore_x_selection_convert(Ecore_X_Atom selection, Ecore_X_Atom target, void **data_ret);
void                     *_ecore_x_selection_parse(const char *target, void *data, int size, int format);


/* screensaver */
void _ecore_x_screensaver_init          (const xcb_query_extension_reply_t *reply);
void _ecore_x_screensaver_init_finalize (void);


/* shape */
void _ecore_x_shape_init          (const xcb_query_extension_reply_t *reply);
void _ecore_x_shape_init_finalize (void);


/* sync */
void _ecore_x_sync_init          (const xcb_query_extension_reply_t *reply);
void _ecore_x_sync_init_finalize (void);


/* xfixes */
void _ecore_x_xfixes_init          (const xcb_query_extension_reply_t *reply);
void _ecore_x_xfixes_init_finalize (void);


/* xinerama */
void _ecore_x_xinerama_init          (const xcb_query_extension_reply_t *reply);
void _ecore_x_xinerama_init_finalize (void);


/* xprint */
void _ecore_x_xprint_init          (const xcb_query_extension_reply_t *reply);
void _ecore_x_xprint_init_finalize (void);

/* to categorize */
void _ecore_x_sync_magic_send(int val, Ecore_X_Window swin);
void _ecore_x_window_grab_remove(Ecore_X_Window win);
void _ecore_x_key_grab_remove(Ecore_X_Window win);


#endif /* __ECORE_XCB_PRIVATE_H__*/
