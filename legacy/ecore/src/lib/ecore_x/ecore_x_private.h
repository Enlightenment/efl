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


/* FIXME: this is for simulation only */
#include "Ecore_Job.h"

typedef struct _Ecore_X_Reply Ecore_X_Reply;

struct _Ecore_X_Reply
{
/* FIXME: this is for simulation only */
   Ecore_Job *job;
   
   void *reply_data;
   void (*reply_data_free) (void *reply_data);
   
   void (*func) (void *data, Ecore_X_Reply *reply, void *reply_data);   
   void *data;
};

typedef struct _Ecore_X_Selection_Data Ecore_X_Selection_Data;

struct _Ecore_X_Selection_Data
{
   Window            win;
   Atom              selection;
   unsigned char     *data;
   int               length;
   Time              time;
};

typedef struct _Ecore_X_Selection_Converter Ecore_X_Selection_Converter;

struct _Ecore_X_Selection_Converter
{
   Atom     target;
   int      (*convert)(char *target, void *data, int size, 
                    void **data_ret, int *size_ret);
   struct _Ecore_X_Selection_Converter *next;
};

typedef struct _Ecore_X_DND_Protocol 
{
   int version;
   Window source, dest;
   
   enum {
      ECORE_X_DND_IDLE,
      ECORE_X_DND_DRAGGING,
      ECORE_X_DND_DROPPED,
      ECORE_X_DND_ENTERED,
      ECORE_X_DND_TARGET_ENTERED,
      ECORE_X_DND_TARGET_CONVERTING,
      ECORE_X_DND_FINISHED
   } state;

   struct {
      short x, y;
      unsigned short width, height;
   } rectangle;

   struct {
      int x, y;
   } pos;
   
   Time time;
   
   Atom *types;
   Atom action, accepted_action;
   int num_types;
   
   int will_accept;
   int suppress;

   int await_status;
   
   struct {
      Ecore_Event_Handler *mouse_move;
      Ecore_Event_Handler *mouse_up;
      Ecore_Event_Handler *mouse_in;
      Ecore_Event_Handler *mouse_out;
   } handlers;
} Ecore_X_DND_Protocol;

typedef enum _Ecore_X_WM_Protocol {
	/**
	 * If enabled the window manager will be asked to send a
	 * delete message instead of just closing (destroying) the window.
	 */
	ECORE_X_WM_PROTOCOL_DELETE_REQUEST,

	/**
	 * If enabled the window manager will be told that the window
	 * explicitly sets input focus.
	 */
	ECORE_X_WM_PROTOCOL_TAKE_FOCUS,
	ECORE_X_WM_PROTOCOL_NUM
} Ecore_X_WM_Protocol;

extern Display *_ecore_x_disp;
extern double   _ecore_x_double_click_time;
extern Time     _ecore_x_event_last_time;
extern Window   _ecore_x_event_last_win;
extern int      _ecore_x_event_last_root_x;
extern int      _ecore_x_event_last_root_y;

extern Atom     _ecore_x_atom_wm_delete_window;
extern Atom     _ecore_x_atom_wm_take_focus;
extern Atom     _ecore_x_atom_wm_protocols;
extern Atom     _ecore_x_atom_wm_class;
extern Atom     _ecore_x_atom_wm_name;
extern Atom     _ecore_x_atom_wm_command;
extern Atom     _ecore_x_atom_wm_icon_name;
extern Atom     _ecore_x_atom_wm_client_machine;
extern Atom     _ecore_x_atom_wm_change_state;

extern Atom     _ecore_x_atom_motif_wm_hints;

extern Atom     _ecore_x_atom_win_layer;

extern Atom     _ecore_x_atom_net_wm_desktop;
extern Atom     _ecore_x_atom_net_current_desktop;
extern Atom     _ecore_x_atom_net_wm_state;
extern Atom     _ecore_x_atom_net_wm_state_above;
extern Atom     _ecore_x_atom_net_wm_state_below;

extern Atom     _ecore_x_atom_net_wm_name;
extern Atom     _ecore_x_atom_net_wm_visible_name;
extern Atom     _ecore_x_atom_net_wm_icon_name;
extern Atom     _ecore_x_atom_net_wm_visible_icon_name;
extern Atom     _ecore_x_atom_net_wm_desktop;
extern Atom     _ecore_x_atom_net_wm_window_type;
extern Atom     _ecore_x_atom_net_wm_state;
extern Atom     _ecore_x_atom_net_wm_allowed_actions;
extern Atom     _ecore_x_atom_net_wm_strut;
extern Atom     _ecore_x_atom_net_wm_strut_partial;
extern Atom     _ecore_x_atom_net_wm_icon_geometry;
extern Atom     _ecore_x_atom_net_wm_icon;
extern Atom     _ecore_x_atom_net_wm_pid;
extern Atom     _ecore_x_atom_net_wm_handle_icons;
extern Atom     _ecore_x_atom_net_wm_user_time;

extern Atom     _ecore_x_atom_net_wm_window_type_desktop;
extern Atom     _ecore_x_atom_net_wm_window_type_dock;
extern Atom     _ecore_x_atom_net_wm_window_type_toolbar;
extern Atom     _ecore_x_atom_net_wm_window_type_menu;
extern Atom     _ecore_x_atom_net_wm_window_type_utility;
extern Atom     _ecore_x_atom_net_wm_window_type_splash;
extern Atom     _ecore_x_atom_net_wm_window_type_dialog;
extern Atom     _ecore_x_atom_net_wm_window_type_normal;

extern Atom     _ecore_x_atom_net_wm_window_opacity;

extern Atom     _ecore_x_atom_net_wm_state_modal;
extern Atom     _ecore_x_atom_net_wm_state_sticky;
extern Atom     _ecore_x_atom_net_wm_state_maximized_vert;
extern Atom     _ecore_x_atom_net_wm_state_maximized_horz;
extern Atom     _ecore_x_atom_net_wm_state_shaded;
extern Atom     _ecore_x_atom_net_wm_state_skip_taskbar;
extern Atom     _ecore_x_atom_net_wm_state_skip_pager;
extern Atom     _ecore_x_atom_net_wm_state_hidden;
extern Atom     _ecore_x_atom_net_wm_state_fullscreen;
extern Atom     _ecore_x_atom_net_wm_state_above;
extern Atom     _ecore_x_atom_net_wm_state_below;

extern Atom     _ecore_x_atoms_wm_protocols[ECORE_X_WM_PROTOCOL_NUM];

extern Atom     _ecore_x_atom_utf8_string;
extern Atom     _ecore_x_atom_file_name;
extern Atom     _ecore_x_atom_string;
extern Atom     _ecore_x_atom_text;
extern Atom     _ecore_x_atom_compound_text;

extern Atom     _ecore_x_atom_selection_primary;
extern Atom     _ecore_x_atom_selection_secondary;
extern Atom     _ecore_x_atom_selection_clipboard;
extern Atom     _ecore_x_atom_selection_prop_primary;
extern Atom     _ecore_x_atom_selection_prop_secondary;
extern Atom     _ecore_x_atom_selection_prop_clipboard;

extern Atom     _ecore_x_atom_selection_xdnd;
extern Atom     _ecore_x_atom_selection_prop_xdnd;
extern Atom     _ecore_x_atom_xdnd_aware;
extern Atom     _ecore_x_atom_xdnd_enter;
extern Atom     _ecore_x_atom_xdnd_type_list;
extern Atom     _ecore_x_atom_xdnd_position;
extern Atom     _ecore_x_atom_xdnd_action_copy;
extern Atom     _ecore_x_atom_xdnd_action_private;
extern Atom     _ecore_x_atom_xdnd_action_ask;
extern Atom     _ecore_x_atom_xdnd_action_list;
extern Atom     _ecore_x_atom_xdnd_action_description;
extern Atom     _ecore_x_atom_xdnd_status;
extern Atom     _ecore_x_atom_xdnd_leave;
extern Atom     _ecore_x_atom_xdnd_drop;
extern Atom     _ecore_x_atom_xdnd_finished;

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

void _ecore_x_selection_data_init(void);
void _ecore_x_selection_shutdown(void);
Atom _ecore_x_selection_target_atom_get(char *target);
char * 
     _ecore_x_selection_target_get(Atom target);
void _ecore_x_selection_request_data_set(Ecore_X_Selection_Data data);
Ecore_X_Selection_Data * 
     _ecore_x_selection_get(Atom selection);
int  _ecore_x_selection_set(Window w, unsigned char *data, int len, Atom selection);
int  _ecore_x_selection_convert(Atom selection, Atom target, void **data_ret);

void _ecore_x_dnd_init(void);
Ecore_X_DND_Protocol * _ecore_x_dnd_protocol_get(void);
#endif
