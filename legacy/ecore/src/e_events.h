#ifndef E_EVENTS_H
#define E_EVENTS_H 1

#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include "e_x.h"

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
   Time                time;
};

struct _ev_window_focus_out
{
   Window              win, root;
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

void                e_add_event_timer(char *name, double in,
				      void (*func) (int val, void *data),
				      int val, void *data);
void               *e_del_event_timer(char *name);

#endif
