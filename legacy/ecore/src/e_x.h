#ifndef E_X_H
#define E_X_H 1

#define XK_MISCELLANY 1

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xresource.h>
#include <X11/keysymdef.h>
#include <X11/extensions/XShm.h>
#include <X11/extensions/shape.h>

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
};

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
void                e_window_show(Window win);
void                e_window_hide(Window win);
Pixmap              e_pixmap_new(Window win, int w, int h, int dep);
void                e_pixmap_free(Pixmap pmap);
void                e_window_set_background_pixmap(Window win, Pixmap pmap);
void                e_window_set_shape_mask(Window win, Pixmap mask);
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
void                e_display_init(char *display);
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

					    char *class);
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
void                e_window_gravity_reset(Window win);
void                e_pointer_warp_by(int dx, int dy);
void                e_pointer_warp_to(int x, int y);
void                e_gc_set_include_inferiors(GC gc);
void                e_area_copy(Drawable src, Drawable dest, GC gc,
				int sx, int sy, int sw, int sh, int dx, int dy);
Window              e_window_root(void);
void                e_window_get_virtual_area(Window win, int *area_x,

					      int *area_y);
void                e_get_virtual_area(int *area_x, int *area_y);

#define E_ATOM(atom, name) \
if (!atom) atom = e_atom_get(name);

#endif
