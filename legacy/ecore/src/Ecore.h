#ifndef E_CORE_H
#define E_CORE_H 1

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <signal.h>

#define XK_MISCELLANY 1

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xresource.h>
#include <X11/keysymdef.h>
#include <X11/extensions/XShm.h>
#include <X11/extensions/shape.h>
#ifdef WITH_DMALLOC
#include <dmalloc.h>
#endif

#define XEV_NONE              NoEventMask
#define XEV_KEY               KeyPressMask | KeyReleaseMask
#define XEV_BUTTON            ButtonPressMask | ButtonReleaseMask
#define XEV_KEY_PRESS         KeyPressMask
#define XEV_KEY_RELEASE       KeyReleaseMask
#define XEV_BUTTON_PRESS      ButtonPressMask
#define XEV_BUTTON_RELEASE    ButtonReleaseMask
#define XEV_IN_OUT            EnterWindowMask | LeaveWindowMask
#define XEV_MOUSE_MOVE        PointerMotionMask | ButtonMotionMask
#define XEV_EXPOSE            ExposureMask
#define XEV_VISIBILITY        VisibilityChangeMask
#define XEV_CONFIGURE         StructureNotifyMask
#define XEV_CHILD_CHANGE      SubstructureNotifyMask
#define XEV_CHILD_REDIRECT    SubstructureRedirectMask | ResizeRedirectMask
#define XEV_FOCUS             FocusChangeMask
#define XEV_PROPERTY          PropertyChangeMask
#define XEV_COLORMAP          ColormapChangeMask

extern XContext            xid_context;

typedef struct _e_xid E_XID;
typedef struct _e_keygrab E_KeyGrab;

enum _ev_modifiers
{
   EV_KEY_MODIFIER_NONE = 0,
   EV_KEY_MODIFIER_SHIFT = (1 << 0),
   EV_KEY_MODIFIER_CTRL = (1 << 1),
   EV_KEY_MODIFIER_ALT = (1 << 2),
   EV_KEY_MODIFIER_WIN = (1 << 3)
};

typedef enum _ev_modifiers Ev_Key_Modifiers;

struct _e_xid
{
   Window              win;
   Window              parent;
   Window              root;
   int                 children_num;
   Window             *children;
   int                 x, y, w, h;
   int                 mapped;
   int                 mouse_in;
   int                 depth;
   int                 gravity;
   int                 coords_invalid;
   int                 bw;
   int                 grab_button_auto_replay;
   int                 grab_button_button;
   Ev_Key_Modifiers    grab_button_mods;
   int                 grab_button_any_mod;
};

#ifdef __cplusplus
extern "C" {
#endif

void                e_del_child(Window win, Window child);
void                e_add_child(Window win, Window child);
void                e_raise_child(Window win, Window child);
void                e_lower_child(Window win, Window child);
E_XID              *e_add_xid(Window win, int x, int y, int w, int h, int depth,
			      Window parent);
E_XID              *e_validate_xid(Window win);
void                e_unvalidate_xid(Window win);
void                e_sync(void);
void                e_flush(void);
Window              e_window_new(Window parent, int x, int y, int w, int h);
Window              e_window_override_new(Window parent, int x, int y, int w,
					  int h);
Window              e_window_input_new(Window parent, int x, int y, int w,
				       int h);
void                e_window_set_events_propagate(Window win, int propagate);
void                e_window_show(Window win);
void                e_window_hide(Window win);
Pixmap              e_pixmap_new(Window win, int w, int h, int dep);
void                e_pixmap_free(Pixmap pmap);
void                e_window_set_background_pixmap(Window win, Pixmap pmap);
void                e_window_set_shape_mask(Window win, Pixmap mask);
void                e_window_add_shape_mask(Window win, Pixmap mask);
void                e_window_set_shape_window(Window win, Window src, int x, int y);
void                e_window_add_shape_window(Window win, Window src, int x, int y);
void                e_window_set_shape_rectangle(Window win, int x, int y, int w, int h);
void                e_window_add_shape_rectangle(Window win, int x, int y, int w, int h);
void                e_window_set_shape_rectangles(Window win, XRectangle *rect, int num);
void                e_window_add_shape_rectangles(Window win, XRectangle *rect, int num);
void                e_window_clip_shape_by_rectangle(Window win, int x, int y, int w, int h);
XRectangle         *e_window_get_shape_rectangles(Window win, int *num);
void                e_window_select_shape_events(Window win);
void                e_window_unselect_shape_events(Window win);
void                e_window_clear(Window win);
void                e_window_clear_area(Window win, int x, int y, int w, int h);
void                e_pointer_xy(Window win, int *x, int *y);
void                e_pointer_xy_set(int x, int y);
void                e_pointer_xy_get(int *x, int *y);
void                e_window_set_events(Window win, long mask);
void                e_window_remove_events(Window win, long mask);
void                e_window_add_events(Window win, long mask);
void                e_window_move(Window win, int x, int y);
void                e_window_resize(Window win, int w, int h);
void                e_window_move_resize(Window win, int x, int y, int w,
					 int h);
int                 e_x_get_fd(void);
void                e_set_error_handler(void (*func) (Display * d, XErrorEvent * ev));
void                e_reset_error_handler(void);
int                 e_display_init(char *display);
int                 e_events_pending(void);
void                e_get_next_event(XEvent * event);
int                 e_event_shape_get_id(void);
KeySym              e_key_get_keysym_from_keycode(KeyCode keycode);
char               *e_key_get_string_from_keycode(KeyCode keycode);
void                e_event_allow(int mode, Time t);
int                 e_lock_mask_scroll_get(void);
int                 e_lock_mask_num_get(void);
int                 e_lock_mask_caps_get(void);
int                 e_mod_mask_shift_get(void);
int                 e_mod_mask_ctrl_get(void);
int                 e_mod_mask_alt_get(void);
int                 e_mod_mask_win_get(void);
int                 e_lock_mask_get(void);
int                 e_modifier_mask_get(void);
Window              e_get_key_grab_win(void);
void                e_key_grab(char *key, Ev_Key_Modifiers mods, int anymod,

			       int sync);
void                e_key_ungrab(char *key, Ev_Key_Modifiers mods, int anymod);
KeyCode             e_key_get_keycode(char *key);
void                e_window_destroy(Window win);
void                e_window_reparent(Window win, Window parent, int x, int y);
void                e_window_raise(Window win);
void                e_window_lower(Window win);
void                e_window_get_geometry(Window win, int *x, int *y, int *w,

					  int *h);
int                 e_window_get_depth(Window win);
int                 e_window_exists(Window win);
Window              e_window_get_parent(Window win);
Window             *e_window_get_children(Window win, int *num);
int                 e_window_mouse_in(Window win);
void                e_window_mouse_set_in(Window win, int in);
Display            *e_display_get(void);
Window              e_window_get_root(Window win);
void                e_lock_scroll_set(int onoff);
int                 e_lock_scroll_get(void);
void                e_lock_num_set(int onoff);
int                 e_lock_num_get(void);
void                e_lock_caps_set(int onoff);
int                 e_lock_caps_get(void);
void                e_mod_shift_set(int onoff);
int                 e_mod_shift_get(void);
void                e_mod_ctrl_set(int onoff);
int                 e_mod_ctrl_get(void);
void                e_mod_alt_set(int onoff);
int                 e_mod_alt_get(void);
void                e_mod_win_set(int onoff);
int                 e_mod_win_get(void);
void                e_focus_window_set(Window win);
Window              e_focus_window_get(void);
void                e_focus_to_window(Window win);
Atom                e_atom_get(char *name);
void                e_window_set_delete_inform(Window win);
void                e_window_property_set(Window win, Atom type, Atom format,
					  int size, void *data, int number);
void               *e_window_property_get(Window win, Atom type, Atom format,

					  int *size);
void                e_window_dnd_advertise(Window win);
void                e_grab(void);
void                e_ungrab(void);
void                e_window_ignore(Window win);
void                e_window_no_ignore(Window win);
int                 e_window_is_ignored(Window win);
Window              e_window_get_at_xy(int x, int y);

int                 e_window_dnd_capable(Window win);
void                e_window_dnd_handle_motion(Window source_win, int x, int y,

					       int dragging);
int                 e_dnd_selection_convert(Window win, Window req, Atom type);
void               *e_dnd_selection_get(Window win, Window req, Atom type,

					int *size);
void                e_window_dnd_ok(int ok);
void                e_window_dnd_finished(void);
void                e_window_dnd_send_status_ok(Window source_win, Window win,
						int x, int y, int w, int h);
void                e_window_dnd_send_finished(Window source_win, Window win);
void                e_dnd_set_data(Window win);
void                e_dnd_send_data(Window win, Window source_win, void *data,
				    int size, Atom dest_atom, int plain_text);
void                e_window_set_title(Window win, char *title);
void                e_window_set_name_class(Window win, char *name,

					    char *);
void                e_window_get_name_class(Window win, char **name, char **class);       
char               *e_window_get_machine(Window win);
char               *e_window_get_command(Window win);
char               *e_window_get_icon_name(Window win);       
void                e_window_get_hints(Window win, int *accepts_focus, int *initial_state,
				       Pixmap *icon_pixmap, Pixmap *icon_mask,
				       Window *icon_window, Window *window_group);
void                e_window_set_min_size(Window win, int w, int h);
void                e_window_set_max_size(Window win, int w, int h);
void                e_window_set_xy_hints(Window win, int x, int y);
void                e_window_get_frame_size(Window win, int *l, int *r, int *t,

					    int *b);
int                 e_window_save_under(Window win);
GC                  e_gc_new(Drawable d);
void                e_gc_free(GC gc);
void                e_gc_set_fg(GC gc, int val);
void                e_fill_rectangle(Drawable d, GC gc, int x, int y, int w,

				     int h);
void                e_draw_rectangle(Drawable d, GC gc, int x, int y, int w,

				     int h);
void                e_draw_line(Drawable d, GC gc, int x1, int y1, int x2,

				int y2);
void                e_draw_point(Drawable d, GC gc, int x, int y);
void                e_window_hint_set_layer(Window win, int layer);
void                e_window_hint_set_sticky(Window win, int sticky);
void                e_window_hint_set_borderless(Window win);
void                e_grab_mouse(Window win, int confine, Cursor cursor);
void                e_ungrab_mouse(void);
Window              e_grab_window_get(void);
void                e_dnd_set_mode_copy(void);
void                e_dnd_set_mode_link(void);
void                e_dnd_set_mode_move(void);
void                e_dnd_set_mode_ask(void);
void                e_dnd_own_selection(Window win);
void                e_dnd_send_drop(Window win, Window source_win);
int                 e_window_get_gravity(Window win);
void                e_window_gravity_reset(Window win);
void                e_window_gravity_set(Window win, int gravity);
void                e_window_bit_gravity_set(Window win, int gravity);
void                e_pointer_warp_by(int dx, int dy);
void                e_pointer_warp_to(int x, int y);
void                e_gc_set_include_inferiors(GC gc);
void                e_area_copy(Drawable src, Drawable dest, GC gc,
				int sx, int sy, int sw, int sh, int dx, int dy);
Window              e_window_root(void);
void                e_window_get_virtual_area(Window win, int *area_x,

					      int *area_y);
void                e_get_virtual_area(int *area_x, int *area_y);
void                e_window_get_root_relative_location(Window win, int *x, int *y);

void                e_button_grab(Window win, int button, int events, Ev_Key_Modifiers mod, int any_mod);
void                e_button_ungrab(Window win, int button, Ev_Key_Modifiers mod, int any_mod);
void                e_pointer_replay(Time t);
void                e_pointer_grab(Window win, Time t);
void                e_pointer_ungrab(Time t);
void                e_window_send_event_move_resize(Window win, int x, int y, int w, int h);
void                e_window_send_client_message(Window win, Atom type, int format, void *data);
void                e_window_add_to_save_set(Window win);
void                e_window_del_from_save_set(Window win);
void                e_window_kill_client(Window win);
void                e_window_set_border_width(Window win, int bw);
int                 e_window_get_border_width(Window win);
int                 e_window_get_wm_size_hints(Window win, XSizeHints *hints, int *mask);
int                 e_window_is_visible(Window win);
int                 e_window_is_normal(Window win);
int                 e_window_is_manageable(Window win);
void                e_windows_restack(Window *wins, int num);
void                e_window_stack_above(Window win, Window above);
void                e_window_stack_below(Window win, Window below);
char               *e_window_get_title(Window win);
void                e_keyboard_grab(Window win);
void                e_keyboard_ungrab(void);

void                e_ev_ipc_init(char *path);
void                e_ev_ipc_cleanup(void);
void                e_add_ipc_service(int service, void (*func) (int fd));
void                e_del_ipc_service(int service);


char               *e_selection_get_data(Window win, Atom prop);
Window              e_selection_request(void);
Window              e_selection_set(char *string);

void                e_set_blank_pointer(Window w);
Cursor              e_cursor_new(Pixmap pmap, Pixmap mask, int x, int y, int fr, int fg, int fb, int br, int bg, int bb);
void                e_cursor_free(Cursor c);
void                e_cursor_set(Window win, Cursor c);

void                e_window_button_grab_auto_replay_set(Window win, int on);
int                 e_window_button_grab_auto_replay_get(Window win);
   
typedef struct _eev Eevent;
typedef struct _ev_fd_handler Ev_Fd_Handler;
typedef struct _ev_pid_handler Ev_Pid_Handler;
typedef struct _ev_ipc_handler Ev_Ipc_Handler;
typedef struct _ev_timer Ev_Timer;

typedef struct _ev_key_down Ev_Key_Down;
typedef struct _ev_key_up Ev_Key_Up;
typedef struct _ev_mouse_down Ev_Mouse_Down;
typedef struct _ev_mouse_up Ev_Mouse_Up;
typedef struct _ev_wheel Ev_Wheel;
typedef struct _ev_mouse_move Ev_Mouse_Move;
typedef struct _ev_window_enter Ev_Window_Enter;
typedef struct _ev_window_leave Ev_Window_Leave;
typedef struct _ev_window_focus_in Ev_Window_Focus_In;
typedef struct _ev_window_focus_out Ev_Window_Focus_Out;
typedef struct _ev_window_expose Ev_Window_Expose;
typedef struct _ev_window_visibility Ev_Window_Visibility;
typedef struct _ev_window_create Ev_Window_Create;
typedef struct _ev_window_destroy Ev_Window_Destroy;
typedef struct _ev_window_map Ev_Window_Map;
typedef struct _ev_window_unmap Ev_Window_Unmap;
typedef struct _ev_window_map_request Ev_Window_Map_Request;
typedef struct _ev_window_reparent Ev_Window_Reparent;
typedef struct _ev_window_configure Ev_Window_Configure;
typedef struct _ev_window_configure_request Ev_Window_Configure_Request;
typedef struct _ev_window_circulate Ev_Window_Circulate;
typedef struct _ev_window_circulate_request Ev_Window_Circulate_Request;
typedef struct _ev_window_property Ev_Window_Property;
typedef struct _ev_window_shape Ev_Window_Shape;
typedef struct _ev_client_message Ev_Message;
typedef struct _ev_colormap Ev_Colormap;
typedef struct _ev_window_delete Ev_Window_Delete;
typedef struct _ev_child Ev_Child;
typedef struct _ev_user Ev_User;
typedef struct _ev_file_progress Ev_File_Progress;
typedef struct _ev_file_update Ev_File_Update;
typedef struct _ev_dnd_drop_request Ev_Dnd_Drop_Request;
typedef struct _ev_dnd_drop_end Ev_Dnd_Drop_End;
typedef struct _ev_dnd_drop_position Ev_Dnd_Drop_Position;
typedef struct _ev_dnd_drop Ev_Dnd_Drop;
typedef struct _ev_dnd_drop_status Ev_Dnd_Drop_Status;
typedef struct _ev_dnd_data_request Ev_Dnd_Data_Request;
typedef struct _ev_paste_request Ev_Paste_Request;
typedef struct _ev_clear_selection Ev_Clear_Selection;
typedef struct _ev_ipc_service Ev_Ipc_Service;

enum _eev_stack_detail
{
   EV_STACK_ABOVE = Above,
   EV_STACK_BELOW = Below,
   EV_STACK_TOP_IF = TopIf,
   EV_STACK_BOTTOM_IF = BottomIf,
   EV_STACK_OPPOSITE = Opposite
};

enum _eev_value_mask
{
   EV_VALUE_X = CWX,
   EV_VALUE_Y = CWY,
   EV_VALUE_W = CWWidth,
   EV_VALUE_H = CWHeight,
   EV_VALUE_BORDER = CWBorderWidth,
   EV_VALUE_SIBLING = CWSibling,
   EV_VALUE_STACKING = CWStackMode
};

enum _eev_type
{
   EV_MOUSE_MOVE,
   EV_MOUSE_DOWN,
   EV_MOUSE_UP,
   EV_MOUSE_IN,
   EV_MOUSE_OUT,
   EV_MOUSE_WHEEL,
   EV_KEY_DOWN,
   EV_KEY_UP,
   EV_WINDOW_MAP,
   EV_WINDOW_UNMAP,
   EV_WINDOW_CREATE,
   EV_WINDOW_DESTROY,
   EV_WINDOW_CONFIGURE,
   EV_WINDOW_CONFIGURE_REQUEST,
   EV_WINDOW_MAP_REQUEST,
   EV_WINDOW_PROPERTY,
   EV_WINDOW_CIRCULATE,
   EV_WINDOW_CIRCULATE_REQUEST,
   EV_WINDOW_REPARENT,
   EV_WINDOW_EXPOSE,
   EV_WINDOW_VISIBILITY,
   EV_WINDOW_SHAPE,
   EV_WINDOW_FOCUS_IN,
   EV_WINDOW_FOCUS_OUT,
   EV_MESSAGE,
   EV_WINDOW_DELETE,
   EV_COLORMAP,

   EV_DND_DROP_REQUEST,
   EV_DND_DROP_END,
   EV_DND_DROP_POSITION,
   EV_DND_DROP,
   EV_DND_DROP_STATUS,
   EV_DND_DATA_REQUEST,
   EV_PASTE_REQUEST,
   EV_CLEAR_SELECTION,

   EV_CHILD,
   EV_USER,

   EV_MAX
};

typedef enum _eev_type Eevent_Type;
typedef enum _eev_stack_detail Ev_Stack_Detail;
typedef enum _eev_value_mask Ev_Confgure_Value_Mask;

struct _eev
{
   Eevent_Type         type;
   char                ignore;
   void               *event;
   void                (*ev_free) (void *evnt);
   Eevent             *next;
};

struct _ev_fd_handler
{
   int                 fd;
   void                (*func) (int fd);
   Ev_Fd_Handler      *next;
};

struct _ev_pid_handler
{
   pid_t               pid;
   void                (*func) (pid_t pid);
   Ev_Pid_Handler     *next;
};

struct _ev_ipc_handler
{
   int                 ipc;
   void                (*func) (int ipc);
   Ev_Ipc_Handler     *next;
};

struct _ev_timer
{
   char               *name;
   void                (*func) (int val, void *data);
   int                 val;
   void               *data;
   double              in;
   char                just_added;
   Ev_Timer           *next;
};

struct _ev_key_down
{
   Window              win, root;
   Ev_Key_Modifiers    mods;
   char               *key;
   char               *compose;
   Time                time;
};

struct _ev_key_up
{
   Window              win, root;
   Ev_Key_Modifiers    mods;
   char               *key;
   char               *compose;
   Time                time;
};

struct _ev_mouse_down
{
   Window              win, root;
   Ev_Key_Modifiers    mods;
   int                 button;
   int                 x, y;
   int                 rx, ry;
   int                 double_click, triple_click;
   Time                time;
};

struct _ev_mouse_up
{
   Window              win, root;
   Ev_Key_Modifiers    mods;
   int                 button;
   int                 x, y;
   int                 rx, ry;
   Time                time;
};

struct _ev_wheel
{
   Window              win, root;
   Ev_Key_Modifiers    mods;
   int                 x, y, z;
   int                 rx, ry;
   Time                time;
};

struct _ev_mouse_move
{
   Window              win, root;
   Ev_Key_Modifiers    mods;
   int                 x, y;
   int                 rx, ry;
   Time                time;
};

struct _ev_window_enter
{
   Window              win, root;
   int                 x, y;
   int                 rx, ry;
   Ev_Key_Modifiers    mods;
   Time                time;
};

struct _ev_window_leave
{
   Window              win, root;
   int                 x, y;
   int                 rx, ry;
   Ev_Key_Modifiers    mods;
   Time                time;
};

struct _ev_window_focus_in
{
   Window              win, root;
   int                 key_grab;
   Time                time;
};

struct _ev_window_focus_out
{
   Window              win, root;
   int                 key_grab;
   Time                time;
};

struct _ev_window_expose
{
   Window              win, root;
   int                 x, y, w, h;
};

struct _ev_window_visibility
{
   Window              win, root;
   int                 fully_obscured;
};

struct _ev_window_create
{
   Window              win, root;
   int                 override;
};

struct _ev_window_destroy
{
   Window              win, root;
};

struct _ev_window_map
{
   Window              win, root;
};

struct _ev_window_unmap
{
   Window              win, root;
};

struct _ev_window_map_request
{
   Window              win, root;
};

struct _ev_window_reparent
{
   Window              win, root;
   Window              parent_from, parent;
};

struct _ev_window_configure
{
   Window              win, root;
   int                 x, y, w, h;
   int                 wm_generated;
};

struct _ev_window_configure_request
{
   Window              win, root;
   int                 x, y, w, h;
   int                 border;
   Window              stack_win;
   Ev_Stack_Detail     detail;
   Ev_Confgure_Value_Mask mask;
};

struct _ev_window_circulate
{
   Window              win, root;
   int                 lower;
};

struct _ev_window_circulate_request
{
   Window              win, root;
   int                 lower;
};

struct _ev_window_property
{
   Window              win, root;
   Atom                atom;
   Time                time;
};

struct _ev_window_shape
{
   Window              win, root;
   Time                time;
};

struct _ev_client_message
{
   Window              win;
   int                 format;
   Atom                atom;
   union
   {
      char                b[20];
      short               s[10];
      long                l[5];
   }
   data;
};

struct _ev_colormap
{
   Window              win, root;
   Colormap            cmap;
   int                 installed;
};

struct _ev_window_delete
{
   Window              win, root;
};

struct _ev_paste_request
{
   Window              win, root, source_win;
   char               *string;
};

struct _ev_clear_selection
{
   Window              win, root;
   Atom                selection;
};

struct _ev_dnd_drop_request
{
   Window              win, root, source_win;
   int                 num_files;
   char              **files;
   int                 copy, link, move;
};

struct _ev_dnd_drop_end
{
   Window              win, root, source_win;
};

struct _ev_dnd_drop_position
{
   Window              win, root, source_win;
   int                 x, y;
};

struct _ev_dnd_drop
{
   Window              win, root, source_win;
};

struct _ev_dnd_drop_status
{
   Window              win, root, source_win;
   int                 x, y, w, h;
   int                 ok;
};

struct _ev_dnd_data_request
{
   Window              win, root, source_win;
   int                 plain_text;
   Atom                destination_atom;
};

struct _ev_child
{
   pid_t               pid;
   int                 exit_code;
};

struct _ev_user
{
   int                 num;
   int                 hup;
};

struct _ev_ipc_service
{
   int                service;
   void               (*func) (int fd);
   Ev_Ipc_Service     *next;
};

void                e_add_event(Eevent_Type type, void *event,
				void (*ev_free) (void *event));
void                e_del_event(void *event);
void                e_del_all_events(void);
Eevent             *e_get_last_event(void);

void                e_add_event_fd(int fd, void (*func) (int fd));
void                e_del_event_fd(int fd);
void                e_add_event_pid(pid_t pid, void (*func) (pid_t pid));
void                e_del_event_pid(pid_t pid);
void                e_add_event_ipc(int ipc, void (*func) (int ipc));
void                e_del_event_ipc(int ipc);

void                e_event_loop(void);
void                e_event_loop_quit(void);

void                e_add_event_timer(char *name, double in,
				      void (*func) (int val, void *data),
				      int val, void *data);
void               *e_del_event_timer(char *name);

void                e_event_filter(Eevent * ev);
void                e_event_filter_events_handle(Eevent * ev);
void                e_event_filter_idle_handle(void);
void                e_event_filter_init(void);
void                e_event_filter_handler_add(Eevent_Type type,
					       void (*func) (Eevent * ev));
void                e_event_filter_idle_handler_add(void (*func) (void *data),

						    void *data);

void                e_ev_signal_init(void);
int                 e_ev_signal_events_pending(void);

void                e_ev_x_init(void);
char               *e_key_press_translate_into_typeable(Ev_Key_Down * e);


#define E_ATOM(atom, name) \
if (!atom) atom = e_atom_get(name);

#define MEMCPY(src, dst, type, num) memcpy(dst, src, sizeof(type) * (num))

#define NEW(dat, num) malloc(sizeof(dat) * (num))
#define ZERO(ptr, dat, num) memset(ptr, 0, sizeof(dat) * (num))
#define NEW_PTR(num) malloc(sizeof(void *) * (num))
#define FREE(dat) {free(dat); dat = NULL;}
#define IF_FREE(dat) {if (dat) FREE(dat);}
#define REALLOC(dat, type, num) {if (dat) dat = realloc(dat, sizeof(type) * (num)); else dat = malloc(sizeof(type) * (num));}
#define REALLOC_PTR(dat, num) {if (dat) dat = realloc(dat, sizeof(void *) * (num)); else dat = malloc(sizeof(void *) * (num));}

#define START_LIST_DEL(type, base, cmp) \
type *_p, *_pp; _pp = NULL; _p = (base); while(_p) { if (cmp) { \
if (_pp) _pp->next = _p->next; else (base) = _p->next;
#define END_LIST_DEL \
return; } _pp = _p; _p = _p->next; }

double              e_get_time(void);

#ifdef __cplusplus
}
#endif

#endif
