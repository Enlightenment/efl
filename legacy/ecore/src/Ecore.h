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


#define ECORE_ATOM(atom, name) \
        if (!atom) (atom) = ecore_atom_get(name);
#define MEMCPY(src, dst, type, num) \
        memcpy(dst, src, sizeof(type) * (num))
#define NEW(type, num) \
        malloc(sizeof(type) * (num))
#define ZERO(ptr, type, num) \
        memset((ptr), 0, sizeof(type) * (num))
#define NEW_PTR(num) \
        malloc(sizeof(void *) * (num))
#define FREE(ptr) \
        { \
                free(ptr); \
                (ptr) = NULL; \
        }
#define IF_FREE(ptr) \
        {if (ptr) FREE(ptr);}
#define REALLOC(ptr, type, num) \
        { \
	        if ((ptr) && (num == 0)) {free(ptr); ptr = NULL;} \
                else if (ptr) ptr = realloc((ptr), sizeof(type) * (num)); \
                else ptr = malloc(sizeof(type) * (num)); \
        }
#define REALLOC_PTR(ptr, num) \
        { \
	        if ((ptr) && (num == 0)) {free(ptr); ptr = NULL;} \
                else if (ptr) ptr = realloc(ptr, sizeof(void *) * (num)); \
                else ptr = malloc(sizeof(void *) * (num)); \
        }

#define START_LIST_DEL(type, base, cmp) \
        type *_p, *_pp; _pp = NULL; \
        _p = (base); \
        while(_p) \
        { \
                if (cmp) \
                { \
                        if (_pp) _pp->next = _p->next; \
                        else (base) = _p->next;
#define END_LIST_DEL \
                        return; \
               } \
               _pp = _p; \
               _p = _p->next; \
        }

#define DND_TYPE_URI_LIST     0
#define DND_TYPE_PLAIN_TEXT   1
#define DND_TYPE_MOZ_URL      2
#define DND_TYPE_NETSCAPE_URL 3

#ifdef __cplusplus
extern              "C"
{
#endif
  
  extern XContext     xid_context;
  
  typedef void        (*Ecore_Error_Function) (Display * d, XErrorEvent * ev);
  
  typedef struct _ecore_keygrab Ecore_KeyGrab;

  typedef enum _ecore_ev_modifiers
  {
    ECORE_EVENT_KEY_MODIFIER_NONE  = 0,
    ECORE_EVENT_KEY_MODIFIER_SHIFT = (1 << 0),
    ECORE_EVENT_KEY_MODIFIER_CTRL  = (1 << 1),
    ECORE_EVENT_KEY_MODIFIER_ALT   = (1 << 2),
    ECORE_EVENT_KEY_MODIFIER_WIN   = (1 << 3)
  } Ecore_Event_Key_Modifiers;
  
  typedef enum _ecore_event_stack_detail
  {
    ECORE_EVENT_STACK_ABOVE     = Above,
    ECORE_EVENT_STACK_BELOW     = Below,
    ECORE_EVENT_STACK_TOP_IF    = TopIf,
    ECORE_EVENT_STACK_BOTTOM_IF = BottomIf,
    ECORE_EVENT_STACK_OPPOSITE  = Opposite
  } Ecore_Event_Stack_Detail;

  typedef enum _ecore_event_value_mask
  {
    ECORE_EVENT_VALUE_X        = CWX,
    ECORE_EVENT_VALUE_Y        = CWY,
    ECORE_EVENT_VALUE_W        = CWWidth,
    ECORE_EVENT_VALUE_H        = CWHeight,
    ECORE_EVENT_VALUE_BORDER   = CWBorderWidth,
    ECORE_EVENT_VALUE_SIBLING  = CWSibling,
    ECORE_EVENT_VALUE_STACKING = CWStackMode
  } Ecore_Event_Value_Mask;

  typedef enum _ecore_event_type
  {
    ECORE_EVENT_MOUSE_MOVE,
    ECORE_EVENT_MOUSE_DOWN,
    ECORE_EVENT_MOUSE_UP,
    ECORE_EVENT_MOUSE_IN,
    ECORE_EVENT_MOUSE_OUT,
    ECORE_EVENT_MOUSE_WHEEL,
    ECORE_EVENT_KEY_DOWN,
    ECORE_EVENT_KEY_UP,
    ECORE_EVENT_WINDOW_MAP,
    ECORE_EVENT_WINDOW_UNMAP,
    ECORE_EVENT_WINDOW_CREATE,
    ECORE_EVENT_WINDOW_DESTROY,
    ECORE_EVENT_WINDOW_CONFIGURE,
    ECORE_EVENT_WINDOW_CONFIGURE_REQUEST,
    ECORE_EVENT_WINDOW_MAP_REQUEST,
    ECORE_EVENT_WINDOW_PROPERTY,
    ECORE_EVENT_WINDOW_CIRCULATE,
    ECORE_EVENT_WINDOW_CIRCULATE_REQUEST,
    ECORE_EVENT_WINDOW_REPARENT,
    ECORE_EVENT_WINDOW_EXPOSE,
    ECORE_EVENT_WINDOW_VISIBILITY,
    ECORE_EVENT_WINDOW_SHAPE,
    ECORE_EVENT_WINDOW_FOCUS_IN,
    ECORE_EVENT_WINDOW_FOCUS_OUT,
    ECORE_EVENT_MESSAGE,
    ECORE_EVENT_WINDOW_DELETE,
    ECORE_EVENT_COLORMAP,

    ECORE_EVENT_DND_DROP_REQUEST,
    ECORE_EVENT_DND_DROP_END,
    ECORE_EVENT_DND_DROP_POSITION,
    ECORE_EVENT_DND_DROP,
    ECORE_EVENT_DND_DROP_STATUS,
    ECORE_EVENT_DND_DATA_REQUEST,
    ECORE_EVENT_PASTE_REQUEST,
    ECORE_EVENT_CLEAR_SELECTION,

    ECORE_EVENT_CHILD,
    ECORE_EVENT_USER,

    ECORE_EVENT_MAX
  } Ecore_Event_Type;

  typedef struct _ecore_event
  {
    Ecore_Event_Type    type;
    char                ignore;
    void               *event;
    void                (*ev_free) (void *evnt);
    struct _ecore_event *next;
  } Ecore_Event;

  typedef struct _ecore_event_fd_handler
  {
    int                 fd;
    void                (*func) (int fd);
    struct _ecore_event_fd_handler *next;
  } Ecore_Event_Fd_Handler;

  typedef struct _ecore_event_pid_handler
  {
    pid_t               pid;
    void                (*func) (pid_t pid);
    struct _ecore_event_pid_handler *next;
  } Ecore_Event_Pid_Handler;

  typedef struct _ecore_event_ipc_handler
  {
    int                 ipc;
    void                (*func) (int ipc);
    struct _ecore_event_ipc_handler *next;
  } Ecore_Event_Ipc_Handler;

  typedef struct _ecore_event_timer
  {
    char               *name;
    void                (*func) (int val, void *data);
    int                 val;
    void               *data;
    double              in;
    char                just_added;
    struct _ecore_event_timer  *next;
  } Ecore_Event_Timer;

  typedef struct _ecore_event_key_down
  {
    Window              win, root;
    Ecore_Event_Key_Modifiers mods;
    char               *key;
    char               *compose;
    Time                time;
  } Ecore_Event_Key_Down;

  typedef struct _ecore_event_key_up
  {
    Window              win, root;
    Ecore_Event_Key_Modifiers mods;
    char               *key;
    char               *compose;
    Time                time;
  } Ecore_Event_Key_Up;

  typedef struct _ecore_event_mouse_down
  {
    Window              win, root;
    Ecore_Event_Key_Modifiers mods;
    int                 button;
    int                 x, y;
    int                 rx, ry;
    int                 double_click, triple_click;
    Time                time;
  } Ecore_Event_Mouse_Down;

  typedef struct _ecore_event_mouse_up
  {
    Window              win, root;
    Ecore_Event_Key_Modifiers mods;
    int                 button;
    int                 x, y;
    int                 rx, ry;
    Time                time;
  } Ecore_Event_Mouse_Up;

  typedef struct _ecore_event_wheel
  {
    Window              win, root;
    Ecore_Event_Key_Modifiers mods;
    int                 x, y, z;
    int                 rx, ry;
    Time                time;
  } Ecore_Event_Wheel;

  typedef struct _ecore_event_mouse_move
  {
    Window              win, root;
    Ecore_Event_Key_Modifiers mods;
    int                 x, y;
    int                 rx, ry;
    Time                time;
  } Ecore_Event_Mouse_Move;

  typedef struct _ecore_event_window_enter
  {
    Window              win, root;
    int                 x, y;
    int                 rx, ry;
    Ecore_Event_Key_Modifiers mods;
    Time                time;
  } Ecore_Event_Window_Enter;

  typedef struct _ecore_event_window_leave
  {
    Window              win, root;
    int                 x, y;
    int                 rx, ry;
    Ecore_Event_Key_Modifiers mods;
    Time                time;
  } Ecore_Event_Window_Leave;

  typedef struct _ecore_event_window_focus_in
  {
    Window              win, root;
    int                 key_grab;
    Time                time;
  } Ecore_Event_Window_Focus_In;

  typedef struct _ecore_event_window_focus_out
  {
    Window              win, root;
    int                 key_grab;
    Time                time;
  } Ecore_Event_Window_Focus_Out;

  typedef struct _ecore_event_window_expose
  {
    Window              win, root;
    int                 x, y, w, h;
  } Ecore_Event_Window_Expose;

  typedef struct _ecore_event_window_visibility
  {
    Window              win, root;
    int                 fully_obscured;
  } Ecore_Event_Window_Visibility;

  typedef struct _ecore_event_window_create
  {
    Window              win, root;
    int                 override;
  } Ecore_Event_Window_Create;

  typedef struct _ecore_event_window_destroy
  {
    Window              win, root;
  } Ecore_Event_Window_Destroy;

  typedef struct _ecore_event_window_map
  {
    Window              win, root;
  } Ecore_Event_Window_Map;

  typedef struct _ecore_event_window_unmap
  {
    Window              win, root;
  } Ecore_Event_Window_Unmap;

  typedef struct _ecore_event_window_map_request
  {
    Window              win, root;
  } Ecore_Event_Window_Map_Request;

  typedef struct _ecore_event_window_reparent
  {
    Window              win, root;
    Window              parent_from, parent;
  } Ecore_Event_Window_Reparent;

  typedef struct _ecore_event_window_configure
  {
    Window              win, root;
    int                 x, y, w, h;
    int                 wm_generated;
  } Ecore_Event_Window_Configure;

  typedef struct _ecore_event_window_configure_request
  {
    Window                   win, root;
    int                      x, y, w, h;
    int                      border;
    Window                   stack_win;
    Ecore_Event_Stack_Detail detail;
    Ecore_Event_Value_Mask   mask;
  } Ecore_Event_Window_Configure_Request;

  typedef struct _ecore_event_window_circulate
  {
    Window              win, root;
    int                 lower;
  } Ecore_Event_Window_Circulate;

  typedef struct _ecore_event_window_circulate_request
  {
    Window              win, root;
    int                 lower;
  } Ecore_Event_Window_Circulate_Request;

  typedef struct _ecore_event_window_property
  {
    Window              win, root;
    Atom                atom;
    Time                time;
  } Ecore_Event_Window_Property;

  typedef struct _ecore_event_window_shape
  {
    Window              win, root;
    Time                time;
  } Ecore_Event_Window_Shape;

  typedef struct _ecore_event_message
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
  } Ecore_Event_Message;

  typedef struct _ecore_event_colormap
  {
    Window              win, root;
    Colormap            cmap;
    int                 installed;
  } Ecore_Event_Colormap;

  typedef struct _ecore_event_window_delete
  {
    Window              win, root;
  } Ecore_Event_Window_Delete;

  typedef struct _ecore_event_paste_request
  {
    Window              win, root, source_win;
    char               *string;
  } Ecore_Event_Paste_Request;

  typedef struct _ecore_event_clear_selection
  {
    Window              win, root;
    Atom                selection;
  } Ecore_Event_Clear_Selection;

  typedef struct _ecore_event_dnd_drop_request
  {
    Window              win, root, source_win;
    int                 num_files;
    char              **files;
    int                 copy, link, move;
  } Ecore_Event_Dnd_Drop_Request;

  typedef struct _ecore_event_dnd_drop_end
  {
    Window              win, root, source_win;
  } Ecore_Event_Dnd_Drop_End;

  typedef struct _ecore_event_dnd_drop_position
  {
    Window              win, root, source_win;
    int                 x, y;
  } Ecore_Event_Dnd_Drop_Position;

  typedef struct _ecore_event_dnd_drop
  {
    Window              win, root, source_win;
  } Ecore_Event_Dnd_Drop;

  typedef struct _ecore_event_dnd_drop_status
  {
    Window              win, root, source_win;
    int                 x, y, w, h;
    int                 copy, link, move, e_private;
    int                 all_position_msgs;
    int                 ok;
  } Ecore_Event_Dnd_Drop_Status;

  typedef struct _ecore_event_dnd_data_request
  {
    Window              win, root, source_win;
    Atom                destination_atom;
    int                 plain_text;
    int                 uri_list;
    int                 moz_url;
    int                 netscape_url;
  } Ecore_Event_Dnd_Data_Request;

  typedef struct _ecore_event_child
  {
    pid_t               pid;
    int                 exit_code;
  } Ecore_Event_Child;

  typedef struct _ecore_event_user
  {
    int                 num;
    int                 hup;
  } Ecore_Event_User;

  typedef struct _ecore_event_ipc_service
  {
    int                 service;
    void                (*func) (int fd);
    struct _ecore_event_ipc_service *next;
  } Ecore_Event_Ipc_Service;


  typedef struct _ecore_xid
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
    int                 (*grab_button_auto_replay) (Ecore_Event_Mouse_Down *ev);
  } Ecore_XID;


  /* ---------------- API CALLS BELOW --------------------*/

  /* ---------------- X WRAPPER CALLS */

  int                 ecore_x_get_fd(void);
  void                ecore_set_error_handler(Ecore_Error_Function func);
  void                ecore_reset_error_handler(void);

  /**
   * ecore_display_init - Establishes a connection to the X server
   * @display: The name of the display to connect to
   *
   * This function creates a connection to the X server. If
   * @display is NULL, the name is taken from the $DISPLAY
   * environment variable.
   */
  int                 ecore_display_init(char *display);

  int                 ecore_events_pending(void);
  void                ecore_get_next_event(XEvent * event);
  int                 ecore_event_shape_get_id(void);
  void                ecore_sync(void);
  void                ecore_flush(void);

  /* ---------------- WINDOW CHILD HANDLING */
  
  void                ecore_del_child(Window win, Window child);
  void                ecore_add_child(Window win, Window child);
  void                ecore_raise_child(Window win, Window child);
  void                ecore_lower_child(Window win, Window child);
  Ecore_XID          *ecore_add_xid(Window win, int x, int y, int w, int h,
				    int depth, Window parent);
  Ecore_XID          *ecore_validate_xid(Window win);
  void                ecore_unvalidate_xid(Window win);

  /* ---------------- WINDOWS-RELATED CALLS */

  Window              ecore_window_new(Window parent, int x, int y, int w,
				       int h);
  Window              ecore_window_override_new(Window parent, int x, int y,
						int w, int h);
  Window              ecore_window_input_new(Window parent, int x, int y, int w,
					     int h);
  void                ecore_window_set_events_propagate(Window win,
							int propagate);
  void                ecore_window_show(Window win);
  void                ecore_window_hide(Window win);
  void                ecore_window_set_background_pixmap(Window win,
							 Pixmap pmap);
  void                ecore_window_set_shape_mask(Window win, Pixmap mask);
  void                ecore_window_add_shape_mask(Window win, Pixmap mask);
  void                ecore_window_set_shape_window(Window win, Window src,
						    int x, int y);
  void                ecore_window_add_shape_window(Window win, Window src,
						    int x, int y);
  void                ecore_window_set_shape_rectangle(Window win, int x, int y,
						       int w, int h);
  void                ecore_window_add_shape_rectangle(Window win, int x, int y,
						       int w, int h);
  void                ecore_window_set_shape_rectangles(Window win,
							XRectangle * rect,
							int num);
  void                ecore_window_add_shape_rectangles(Window win,
							XRectangle * rect,
							int num);
  void                ecore_window_clip_shape_by_rectangle(Window win, int x,
							   int y, int w, int h);
  XRectangle         *ecore_window_get_shape_rectangles(Window win, int *num);
  void                ecore_window_select_shape_events(Window win);
  void                ecore_window_unselect_shape_events(Window win);
  void                ecore_window_clear(Window win);
  void                ecore_window_clear_area(Window win, int x, int y, int w,
					      int h);
  void                ecore_window_set_events(Window win, long mask);
  void                ecore_window_remove_events(Window win, long mask);
  void                ecore_window_add_events(Window win, long mask);
  void                ecore_window_move(Window win, int x, int y);
  void                ecore_window_resize(Window win, int w, int h);
  void                ecore_window_move_resize(Window win, int x, int y, int w,
					       int h);
  void                ecore_window_destroy(Window win);
  void                ecore_window_reparent(Window win, Window parent, int x,
					    int y);
  void                ecore_window_raise(Window win);
  void                ecore_window_lower(Window win);
  void                ecore_window_get_geometry(Window win, int *x, int *y,
						int *w, int *h);
  int                 ecore_window_get_depth(Window win);
  int                 ecore_window_exists(Window win);
  Window              ecore_window_get_parent(Window win);
  Window             *ecore_window_get_children(Window win, int *num);
  int                 ecore_window_mouse_in(Window win);
  void                ecore_window_mouse_set_in(Window win, int in);
  Window              ecore_window_get_root(Window win);
  void                ecore_window_set_delete_inform(Window win);
  void                ecore_window_property_set(Window win, Atom type,
						Atom format, int size,
						void *data, int number);
  void               *ecore_window_property_get(Window win, Atom type,
						Atom format, int *size);
  void                ecore_window_dnd_advertise(Window win);
  void                ecore_window_ignore(Window win);
  void                ecore_window_no_ignore(Window win);
  int                 ecore_window_is_ignored(Window win);
  Window              ecore_window_get_at_xy(int x, int y);

  int                 ecore_window_dnd_capable(Window win);
  void                ecore_window_dnd_handle_motion(Window source_win, int x,
						     int y, int dragging);
  void                ecore_window_dnd_ok(int ok);
  void                ecore_window_dnd_finished(void);
  void                ecore_window_dnd_send_status_ok(Window source_win,
						      Window win, int x, int y,
						      int w, int h);
  void                ecore_window_dnd_send_finished(Window source_win,
						     Window win);
  void                ecore_window_set_title(Window win, char *title);
  void                ecore_window_set_name_class(Window win, char *name,
						  char *);
  void                ecore_window_get_name_class(Window win, char **name,
						  char **name_class);
  char               *ecore_window_get_machine(Window win);
  char               *ecore_window_get_command(Window win);
  char               *ecore_window_get_icon_name(Window win);
  void                ecore_window_get_hints(Window win, int *accepts_focus,
					     int *initial_state,
					     Pixmap * icon_pixmap,
					     Pixmap * icon_mask,
					     Window * icon_window,
					     Window * window_group);
  void                ecore_window_set_min_size(Window win, int w, int h);
  void                ecore_window_set_max_size(Window win, int w, int h);
  void                ecore_window_set_xy_hints(Window win, int x, int y);
  void                ecore_window_get_frame_size(Window win, int *l, int *r,
						  int *t, int *b);
  int                 ecore_window_save_under(Window win);
  void                ecore_window_hint_set_layer(Window win, int layer);
  void                ecore_window_hint_set_sticky(Window win, int sticky);
  void                ecore_window_hint_set_borderless(Window win);
  int                 ecore_window_get_gravity(Window win);
  void                ecore_window_gravity_reset(Window win);
  void                ecore_window_gravity_set(Window win, int gravity);
  void                ecore_window_bit_gravity_set(Window win, int gravity);
  void                ecore_window_get_root_relative_location(Window win,
							      int *x, int *y);
  void                ecore_window_send_event_move_resize(Window win, int x,
							  int y, int w, int h);
  void                ecore_window_send_client_message(Window win, Atom type,
						       int format, void *data);
  void                ecore_window_add_to_save_set(Window win);
  void                ecore_window_del_from_save_set(Window win);
  void                ecore_window_kill_client(Window win);
  void                ecore_window_set_border_width(Window win, int bw);
  int                 ecore_window_get_border_width(Window win);
  int                 ecore_window_get_wm_size_hints(Window win,
						     XSizeHints * hints,
						     int *mask);
  int                 ecore_window_is_visible(Window win);
  int                 ecore_window_is_normal(Window win);

  /**
   * ecore_window_is_manageable - Returns 1 if a window can be managed by the wm.
   * @win: the window for which to query.
   *
   * This function returns 0 if the window is unmapped, should be ignored
   * by the window manager or is of the InputOnly class, 1 otherwise.
   */
  int                 ecore_window_is_manageable(Window win);

  void                ecore_windows_restack(Window * wins, int num);
  void                ecore_window_stack_above(Window win, Window above);
  void                ecore_window_stack_below(Window win, Window below);
  char               *ecore_window_get_title(Window win);
  void                ecore_window_button_grab_auto_replay_set(Window win, 
							       int (*func) (Ecore_Event_Mouse_Down *ev));
  void               *ecore_window_button_grab_auto_replay_get(Window win);
  Window              ecore_window_root(void);
  void                ecore_window_get_virtual_area(Window win, int *area_x,
						    int *area_y);
   
  /* ---------------- PIXMAP CALLS */

  Pixmap              ecore_pixmap_new(Window win, int w, int h, int dep);
  void                ecore_pixmap_free(Pixmap pmap);

  /* ---------------- POINTER CALLS */

  void                ecore_pointer_xy(Window win, int *x, int *y);
  void                ecore_pointer_xy_set(int x, int y);
  void                ecore_pointer_xy_get(int *x, int *y);
  void                ecore_pointer_warp_by(int dx, int dy);
  void                ecore_pointer_warp_to(int x, int y);
  void                ecore_pointer_replay(Time t);
  void                ecore_pointer_grab(Window win, Time t);
  void                ecore_pointer_ungrab(Time t);

  /* ---------------- KEYBOARD-RELATED CALLS */

  KeySym              ecore_key_get_keysym_from_keycode(KeyCode keycode);
  char               *ecore_key_get_string_from_keycode(KeyCode keycode);
  void                ecore_key_grab(char *key, Ecore_Event_Key_Modifiers mods,
				     int anymod, int sync);
  void                ecore_key_ungrab(char *key,
				       Ecore_Event_Key_Modifiers mods,
				       int anymod);
  KeyCode             ecore_key_get_keycode(char *key);
  char               *ecore_keypress_translate_into_typeable(Ecore_Event_Key_Down * e);

  void                ecore_event_allow(int mode, Time t);

  /* ---------------- LOCKS AND MODIFIER CALLS */

  int                 ecore_lock_mask_scroll_get(void);
  int                 ecore_lock_mask_num_get(void);
  int                 ecore_lock_mask_caps_get(void);
  void                ecore_lock_scroll_set(int onoff);
  int                 ecore_lock_scroll_get(void);
  void                ecore_lock_num_set(int onoff);
  int                 ecore_lock_num_get(void);
  void                ecore_lock_caps_set(int onoff);
  int                 ecore_lock_caps_get(void);

  int                 ecore_mod_mask_shift_get(void);
  int                 ecore_mod_mask_ctrl_get(void);
  int                 ecore_mod_mask_alt_get(void);
  int                 ecore_mod_mask_win_get(void);
  void                ecore_mod_shift_set(int onoff);
  int                 ecore_mod_shift_get(void);
  void                ecore_mod_ctrl_set(int onoff);
  int                 ecore_mod_ctrl_get(void);
  void                ecore_mod_alt_set(int onoff);
  int                 ecore_mod_alt_get(void);
  void                ecore_mod_win_set(int onoff);
  int                 ecore_mod_win_get(void);

  int                 ecore_lock_mask_get(void);
  int                 ecore_modifier_mask_get(void);
  Window              ecore_get_key_grab_win(void);
  Display            *ecore_display_get(void);
  void                ecore_focus_window_set(Window win);
  Window              ecore_focus_window_get(void);
  void                ecore_focus_to_window(Window win);
  void                ecore_focus_mode_reset(void);
  Atom                ecore_atom_get(char *name);


  /* ---------------- GRABBING/UNGRABBING CALLS */

  /**
   * ecore_grab - Grabs the X server connection
   *
   * This function wraps XGrabServer, which prevents all other
   * clients from receiving events. Calls are reference-counted.
   */
  void                ecore_grab(void);

  /**
   * ecore_ungrab - Ungrabs the X server connection
   * 
   * This function releases the server, provided that ecore_grab()
   * has been called the same number of times as ecore_ungrab().
   */
  void                ecore_ungrab(void);

  void                ecore_grab_mouse(Window win, int confine, Cursor cursor);
  void                ecore_ungrab_mouse(void);
  Window              ecore_grab_window_get(void);

  void                ecore_button_grab(Window win, int button, int events,
					Ecore_Event_Key_Modifiers mod,
					int any_mod);
  void                ecore_button_ungrab(Window win, int button,
					  Ecore_Event_Key_Modifiers mod,
					  int any_mod);
  void                ecore_keyboard_grab(Window win);
  void                ecore_keyboard_ungrab(void);
  Window              ecore_keyboard_grab_window_get(void);
       

  /* ----------------- GRAPHICS CONTEXT AND DRAWING CALLS */

  GC                  ecore_gc_new(Drawable d);
  void                ecore_gc_free(GC gc);
  void                ecore_gc_set_fg(GC gc, int val);
  void                ecore_gc_set_include_inferiors(GC gc);
  void                ecore_fill_rectangle(Drawable d, GC gc, int x, int y,
					   int w, int h);
  void                ecore_draw_rectangle(Drawable d, GC gc, int x, int y,
					   int w, int h);
  void                ecore_draw_line(Drawable d, GC gc, int x1, int y1, int x2,
				      int y2);
  void                ecore_draw_point(Drawable d, GC gc, int x, int y);
  void                ecore_area_copy(Drawable src, Drawable dest, GC gc,
				      int sx, int sy, int sw, int sh, int dx,
				      int dy);


  /* ---------------- DRAG AND DROP CALLS */


  int                 ecore_dnd_selection_convert(Window win, Window req,
						  Atom type);
  void               *ecore_dnd_selection_get(Window win, Window req, Atom type,
					      int *size);
  void                ecore_dnd_set_data(Window win);
  void                ecore_dnd_set_action(Window win);
       
  void                ecore_dnd_send_data(Window win, Window source_win,
					  void *data, int size, Atom dest_atom,
					  int type);
  void                ecore_dnd_set_mode_copy(void);
  void                ecore_dnd_set_mode_link(void);
  void                ecore_dnd_set_mode_move(void);
  void                ecore_dnd_set_mode_ask(void);
  void                ecore_dnd_own_selection(Window win);
  void                ecore_dnd_send_drop(Window win, Window source_win);
  void                ecore_get_virtual_area(int *area_x, int *area_y);
  void                ecore_clear_target_status(void);


  /* ---------------- IPC CALLS */

  void                ecore_event_ipc_init(char *path);
  void                ecore_event_ipc_cleanup(void);
  void                ecore_add_ipc_service(int service, void (*func) (int fd));
  void                ecore_del_ipc_service(int service);

  /* ---------------- SELECTION CALLS */

  char               *ecore_selection_get_data(Window win, Atom prop);
  Window              ecore_selection_request(void);
  Window              ecore_selection_set(char *string);

  /* ---------------- CURSOR LOOK CALLS */

  void                ecore_set_blank_pointer(Window w);
  Cursor              ecore_cursor_new(Pixmap pmap, Pixmap mask, int x, int y,
				       int fr, int fg, int fb, int br, int bg,
				       int bb);
  void                ecore_cursor_free(Cursor c);
  void                ecore_cursor_set(Window win, Cursor c);

  /* ---------------- EVENT-HANDLING CALLS */

  void                ecore_add_event(Ecore_Event_Type type, void *event,
				      void (*ev_free) (void *event));
  void                ecore_del_event(void *event);
  void                ecore_del_all_events(void);
  Ecore_Event        *ecore_get_last_event(void);
  void                ecore_add_event_fd(int fd, void (*func) (int fd));
  void                ecore_del_event_fd(int fd);
  void                ecore_add_event_pid(pid_t pid, void (*func) (pid_t pid));
  void                ecore_del_event_pid(pid_t pid);
  void                ecore_add_event_ipc(int ipc, void (*func) (int ipc));
  void                ecore_del_event_ipc(int ipc);
  void                ecore_event_loop(void);
  void                ecore_event_loop_quit(void);
  void                ecore_add_event_timer(char *name, double in,
					    void (*func) (int val, void *data),
					    int val, void *data);
  void               *ecore_del_event_timer(char *name);
  void                ecore_event_filter(Ecore_Event * ev);
  void                ecore_event_filter_events_handle(Ecore_Event * ev);
  void                ecore_event_filter_idle_handle(void);

  /**
   * ecore_event_filter_init - Setup event handlers
   *
   * This function initializes all ECORE_EVENT_MAX
   * event handler to be empty.
   */
  void                ecore_event_filter_init(void);

  void                ecore_event_filter_handler_add(Ecore_Event_Type type,
						     void (*func) (Ecore_Event *ev));
  void                ecore_event_filter_idle_handler_add(void (*func) (void *data), void *data);

  /**
   * ecore_event_signal_init - Setup for default signal handlers
   *
   * This function initializes signal handlers for various
   * signals via sigaction().
   */
  void                ecore_event_signal_init(void);

  int                 ecore_event_signal_events_pending(void);

  /**
   * ecore_event_x_init - Ecore X initialization
   *
   * This function initializes Ecore's X event handling
   * callbacks.
   */
  void                ecore_event_x_init(void);

  /* ---------------- MISCELLANEOUS CALLS */

  double              ecore_get_time(void);

#ifdef __cplusplus
}
#endif

#endif
