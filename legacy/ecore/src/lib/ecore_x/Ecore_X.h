/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifndef _ECORE_X_H
#define _ECORE_X_H

#ifdef EAPI
# undef EAPI
#endif
#ifdef _MSC_VER
# ifdef BUILDING_DLL
#  define EAPI __declspec(dllexport)
# else
#  define EAPI __declspec(dllimport)
# endif
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EAPI __attribute__ ((visibility("default")))
#  else
#   define EAPI
#  endif
# else
#  define EAPI
# endif
#endif

#include <sys/types.h>

/**
 * @file
 * @brief Ecore functions for dealing with the X Windows System
 *
 * Ecore_X provides a wrapper and convenience functions for using the
 * X Windows System.  Function groups for this part of the library
 * include the following:
 * @li @ref Ecore_X_Init_Group
 * @li @ref Ecore_X_Display_Attr_Group
 * @li @ref Ecore_X_Flush_Group
 */


typedef unsigned int Ecore_X_ID;
#  ifndef _ECORE_X_WINDOW_PREDEF
typedef Ecore_X_ID   Ecore_X_Window;
#  endif
#ifdef HAVE_ECORE_X_XCB
typedef Ecore_X_ID   Ecore_X_Visual;
#else
typedef void       * Ecore_X_Visual;
#endif /* HAVE_ECORE_X_XCB */
typedef Ecore_X_ID   Ecore_X_Pixmap;
typedef Ecore_X_ID   Ecore_X_Drawable;
#ifdef HAVE_ECORE_X_XCB
typedef Ecore_X_ID   Ecore_X_GC;
#else
typedef void       * Ecore_X_GC;
#endif /* HAVE_ECORE_X_XCB */
typedef Ecore_X_ID   Ecore_X_Atom;
typedef Ecore_X_ID   Ecore_X_Colormap;
typedef Ecore_X_ID   Ecore_X_Time;
typedef Ecore_X_ID   Ecore_X_Cursor;
typedef void         Ecore_X_Display;
typedef void         Ecore_X_Connection;
typedef void         Ecore_X_Screen;
typedef Ecore_X_ID   Ecore_X_Sync_Counter;
typedef Ecore_X_ID   Ecore_X_Sync_Alarm;

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _Ecore_X_Rectangle {
   int x, y;
   unsigned int width, height;
} Ecore_X_Rectangle;

typedef struct _Ecore_X_Icon {
   unsigned int  width, height;
   unsigned int *data;
} Ecore_X_Icon;

typedef enum _Ecore_X_Window_State {
    /** The window is iconified. */
    ECORE_X_WINDOW_STATE_ICONIFIED,
    /** The window is a modal dialog box. */
    ECORE_X_WINDOW_STATE_MODAL,
    /** The window manager should keep the window's position fixed
     * even if the virtual desktop scrolls. */
    ECORE_X_WINDOW_STATE_STICKY,
    /** The window has the maximum vertical size. */
    ECORE_X_WINDOW_STATE_MAXIMIZED_VERT,
    /** The window has the maximum horizontal size. */
    ECORE_X_WINDOW_STATE_MAXIMIZED_HORZ,
    /** The window is shaded. */
    ECORE_X_WINDOW_STATE_SHADED,
    /** The window should not be included in the taskbar. */
    ECORE_X_WINDOW_STATE_SKIP_TASKBAR,
    /** The window should not be included in the pager. */
    ECORE_X_WINDOW_STATE_SKIP_PAGER,
    /** The window is invisible (i.e. minimized/iconified) */
    ECORE_X_WINDOW_STATE_HIDDEN,
    /** The window should fill the entire screen and have no
     * window border/decorations */
    ECORE_X_WINDOW_STATE_FULLSCREEN,
    /* The following are not documented because they are not
     * intended for use in applications. */
    ECORE_X_WINDOW_STATE_ABOVE,
    ECORE_X_WINDOW_STATE_BELOW,
    /* FIXME: Documentation */
    ECORE_X_WINDOW_STATE_DEMANDS_ATTENTION,
    /* Unknown state */
    ECORE_X_WINDOW_STATE_UNKNOWN
} Ecore_X_Window_State;

typedef enum _Ecore_X_Window_State_Action {
     ECORE_X_WINDOW_STATE_ACTION_REMOVE,
     ECORE_X_WINDOW_STATE_ACTION_ADD,
     ECORE_X_WINDOW_STATE_ACTION_TOGGLE
} Ecore_X_Window_State_Action;

typedef enum _Ecore_X_Window_Stack_Mode {
   ECORE_X_WINDOW_STACK_ABOVE = 0,
   ECORE_X_WINDOW_STACK_BELOW = 1,
   ECORE_X_WINDOW_STACK_TOP_IF = 2,
   ECORE_X_WINDOW_STACK_BOTTOM_IF = 3,
   ECORE_X_WINDOW_STACK_OPPOSITE = 4
} Ecore_X_Window_Stack_Mode;

typedef enum _Ecore_X_Randr_Rotation {
   ECORE_X_RANDR_ROT_0 = (1 << 0),
   ECORE_X_RANDR_ROT_90 = (1 << 1),
   ECORE_X_RANDR_ROT_180 = (1 << 2),
   ECORE_X_RANDR_ROT_270 = (1 << 3),
   ECORE_X_RANDR_FLIP_X = (1 << 4),
   ECORE_X_RANDR_FLIP_Y = (1 << 5)
} Ecore_X_Randr_Rotation;

#define ECORE_X_SELECTION_TARGET_TARGETS "TARGETS"
#define ECORE_X_SELECTION_TARGET_TEXT "TEXT"
#define ECORE_X_SELECTION_TARGET_COMPOUND_TEXT "COMPOUND_TEXT"
#define ECORE_X_SELECTION_TARGET_STRING "STRING"
#define ECORE_X_SELECTION_TARGET_UTF8_STRING "UTF8_STRING"
#define ECORE_X_SELECTION_TARGET_FILENAME "FILENAME"

#define ECORE_X_DND_VERSION 5

EAPI extern Ecore_X_Atom ECORE_X_DND_ACTION_COPY;
EAPI extern Ecore_X_Atom ECORE_X_DND_ACTION_MOVE;
EAPI extern Ecore_X_Atom ECORE_X_DND_ACTION_LINK;
EAPI extern Ecore_X_Atom ECORE_X_DND_ACTION_ASK;
EAPI extern Ecore_X_Atom ECORE_X_DND_ACTION_PRIVATE;

typedef enum _Ecore_X_Selection {
   ECORE_X_SELECTION_PRIMARY,
   ECORE_X_SELECTION_SECONDARY,
   ECORE_X_SELECTION_XDND,
   ECORE_X_SELECTION_CLIPBOARD
} Ecore_X_Selection;

typedef enum _Ecore_X_Event_Mode
{
   ECORE_X_EVENT_MODE_NORMAL,
   ECORE_X_EVENT_MODE_WHILE_GRABBED,
   ECORE_X_EVENT_MODE_GRAB,
   ECORE_X_EVENT_MODE_UNGRAB
} Ecore_X_Event_Mode;

typedef enum _Ecore_X_Event_Detail
{
   ECORE_X_EVENT_DETAIL_ANCESTOR,
   ECORE_X_EVENT_DETAIL_VIRTUAL,
   ECORE_X_EVENT_DETAIL_INFERIOR,
   ECORE_X_EVENT_DETAIL_NON_LINEAR,
   ECORE_X_EVENT_DETAIL_NON_LINEAR_VIRTUAL,
   ECORE_X_EVENT_DETAIL_POINTER,
   ECORE_X_EVENT_DETAIL_POINTER_ROOT,
   ECORE_X_EVENT_DETAIL_DETAIL_NONE
} Ecore_X_Event_Detail;

typedef enum _Ecore_X_Event_Mask
{
   ECORE_X_EVENT_MASK_NONE                   = 0L,
   ECORE_X_EVENT_MASK_KEY_DOWN               = (1L << 0),
   ECORE_X_EVENT_MASK_KEY_UP                 = (1L << 1),
   ECORE_X_EVENT_MASK_MOUSE_DOWN             = (1L << 2),
   ECORE_X_EVENT_MASK_MOUSE_UP               = (1L << 3),
   ECORE_X_EVENT_MASK_MOUSE_IN               = (1L << 4),
   ECORE_X_EVENT_MASK_MOUSE_OUT              = (1L << 5),
   ECORE_X_EVENT_MASK_MOUSE_MOVE             = (1L << 6),
   ECORE_X_EVENT_MASK_WINDOW_DAMAGE          = (1L << 15),
   ECORE_X_EVENT_MASK_WINDOW_VISIBILITY      = (1L << 16),
   ECORE_X_EVENT_MASK_WINDOW_CONFIGURE       = (1L << 17),
   ECORE_X_EVENT_MASK_WINDOW_RESIZE_MANAGE   = (1L << 18),
   ECORE_X_EVENT_MASK_WINDOW_MANAGE          = (1L << 19),
   ECORE_X_EVENT_MASK_WINDOW_CHILD_CONFIGURE = (1L << 20),
   ECORE_X_EVENT_MASK_WINDOW_FOCUS_CHANGE    = (1L << 21),
   ECORE_X_EVENT_MASK_WINDOW_PROPERTY        = (1L << 22),
   ECORE_X_EVENT_MASK_WINDOW_COLORMAP        = (1L << 23),
   ECORE_X_EVENT_MASK_WINDOW_GRAB            = (1L << 24),
   ECORE_X_EVENT_MASK_MOUSE_WHEEL            = (1L << 29),
   ECORE_X_EVENT_MASK_WINDOW_FOCUS_IN        = (1L << 30),
   ECORE_X_EVENT_MASK_WINDOW_FOCUS_OUT       = (1L << 31)
} Ecore_X_Event_Mask;

typedef enum _Ecore_X_Gravity {
     ECORE_X_GRAVITY_FORGET = 0,
     ECORE_X_GRAVITY_UNMAP = 0,
     ECORE_X_GRAVITY_NW = 1,
     ECORE_X_GRAVITY_N = 2,
     ECORE_X_GRAVITY_NE = 3,
     ECORE_X_GRAVITY_W = 4,
     ECORE_X_GRAVITY_CENTER = 5,
     ECORE_X_GRAVITY_E = 6,
     ECORE_X_GRAVITY_SW = 7,
     ECORE_X_GRAVITY_S = 8,
     ECORE_X_GRAVITY_SE = 9,
     ECORE_X_GRAVITY_STATIC = 10
} Ecore_X_Gravity;

/* Needed for ecore_x_region_window_shape_set */
typedef enum _Ecore_X_Shape_Type
{
   ECORE_X_SHAPE_BOUNDING,
   ECORE_X_SHAPE_CLIP
} Ecore_X_Shape_Type;

typedef struct _Ecore_X_Event_Key_Down                 Ecore_X_Event_Key_Down;
typedef struct _Ecore_X_Event_Key_Up                   Ecore_X_Event_Key_Up;
typedef struct _Ecore_X_Event_Mouse_Button_Down        Ecore_X_Event_Mouse_Button_Down;
typedef struct _Ecore_X_Event_Mouse_Button_Up          Ecore_X_Event_Mouse_Button_Up;
typedef struct _Ecore_X_Event_Mouse_Move               Ecore_X_Event_Mouse_Move;
typedef struct _Ecore_X_Event_Mouse_In                 Ecore_X_Event_Mouse_In;
typedef struct _Ecore_X_Event_Mouse_Out                Ecore_X_Event_Mouse_Out;
typedef struct _Ecore_X_Event_Mouse_Wheel              Ecore_X_Event_Mouse_Wheel;
typedef struct _Ecore_X_Event_Window_Focus_In          Ecore_X_Event_Window_Focus_In;
typedef struct _Ecore_X_Event_Window_Focus_Out         Ecore_X_Event_Window_Focus_Out;
typedef struct _Ecore_X_Event_Window_Keymap            Ecore_X_Event_Window_Keymap;
typedef struct _Ecore_X_Event_Window_Damage            Ecore_X_Event_Window_Damage;
typedef struct _Ecore_X_Event_Window_Visibility_Change Ecore_X_Event_Window_Visibility_Change;
typedef struct _Ecore_X_Event_Window_Create            Ecore_X_Event_Window_Create;
typedef struct _Ecore_X_Event_Window_Destroy           Ecore_X_Event_Window_Destroy;
typedef struct _Ecore_X_Event_Window_Hide              Ecore_X_Event_Window_Hide;
typedef struct _Ecore_X_Event_Window_Show              Ecore_X_Event_Window_Show;
typedef struct _Ecore_X_Event_Window_Show_Request      Ecore_X_Event_Window_Show_Request;
typedef struct _Ecore_X_Event_Window_Reparent          Ecore_X_Event_Window_Reparent;
typedef struct _Ecore_X_Event_Window_Configure         Ecore_X_Event_Window_Configure;
typedef struct _Ecore_X_Event_Window_Configure_Request Ecore_X_Event_Window_Configure_Request;
typedef struct _Ecore_X_Event_Window_Gravity           Ecore_X_Event_Window_Gravity;
typedef struct _Ecore_X_Event_Window_Resize_Request    Ecore_X_Event_Window_Resize_Request;
typedef struct _Ecore_X_Event_Window_Stack             Ecore_X_Event_Window_Stack;
typedef struct _Ecore_X_Event_Window_Stack_Request     Ecore_X_Event_Window_Stack_Request;
typedef struct _Ecore_X_Event_Window_Property          Ecore_X_Event_Window_Property;
typedef struct _Ecore_X_Event_Window_Colormap          Ecore_X_Event_Window_Colormap;
typedef struct _Ecore_X_Event_Window_Mapping           Ecore_X_Event_Window_Mapping;
typedef struct _Ecore_X_Event_Selection_Clear          Ecore_X_Event_Selection_Clear;
typedef struct _Ecore_X_Event_Selection_Request        Ecore_X_Event_Selection_Request;
typedef struct _Ecore_X_Event_Selection_Notify         Ecore_X_Event_Selection_Notify;
typedef struct _Ecore_X_Selection_Data                 Ecore_X_Selection_Data;
typedef struct _Ecore_X_Selection_Data_Files           Ecore_X_Selection_Data_Files;
typedef struct _Ecore_X_Selection_Data_Text            Ecore_X_Selection_Data_Text;
typedef struct _Ecore_X_Selection_Data_Targets         Ecore_X_Selection_Data_Targets;
typedef struct _Ecore_X_Event_Xdnd_Enter               Ecore_X_Event_Xdnd_Enter;
typedef struct _Ecore_X_Event_Xdnd_Position            Ecore_X_Event_Xdnd_Position;
typedef struct _Ecore_X_Event_Xdnd_Status              Ecore_X_Event_Xdnd_Status;
typedef struct _Ecore_X_Event_Xdnd_Leave               Ecore_X_Event_Xdnd_Leave;
typedef struct _Ecore_X_Event_Xdnd_Drop                Ecore_X_Event_Xdnd_Drop;
typedef struct _Ecore_X_Event_Xdnd_Finished            Ecore_X_Event_Xdnd_Finished;
typedef struct _Ecore_X_Event_Client_Message           Ecore_X_Event_Client_Message;
typedef struct _Ecore_X_Event_Window_Shape             Ecore_X_Event_Window_Shape;
typedef struct _Ecore_X_Event_Screensaver_Notify       Ecore_X_Event_Screensaver_Notify;
typedef struct _Ecore_X_Event_Sync_Counter             Ecore_X_Event_Sync_Counter;
typedef struct _Ecore_X_Event_Sync_Alarm               Ecore_X_Event_Sync_Alarm;
typedef struct _Ecore_X_Event_Screen_Change            Ecore_X_Event_Screen_Change;

typedef struct _Ecore_X_Event_Window_Delete_Request                Ecore_X_Event_Window_Delete_Request;
typedef struct _Ecore_X_Event_Window_Prop_Title_Change             Ecore_X_Event_Window_Prop_Title_Change;
typedef struct _Ecore_X_Event_Window_Prop_Visible_Title_Change     Ecore_X_Event_Window_Prop_Visible_Title_Change;
typedef struct _Ecore_X_Event_Window_Prop_Icon_Name_Change         Ecore_X_Event_Window_Prop_Icon_Name_Change;
typedef struct _Ecore_X_Event_Window_Prop_Visible_Icon_Name_Change Ecore_X_Event_Window_Prop_Visible_Icon_Name_Change;
typedef struct _Ecore_X_Event_Window_Prop_Client_Machine_Change    Ecore_X_Event_Window_Prop_Client_Machine_Change;
typedef struct _Ecore_X_Event_Window_Prop_Name_Class_Change        Ecore_X_Event_Window_Prop_Name_Class_Change;
typedef struct _Ecore_X_Event_Window_Prop_Pid_Change               Ecore_X_Event_Window_Prop_Pid_Change;
typedef struct _Ecore_X_Event_Window_Prop_Desktop_Change           Ecore_X_Event_Window_Prop_Desktop_Change;

typedef struct _Ecore_X_Event_Window_Move_Resize_Request Ecore_X_Event_Window_Move_Resize_Request;
typedef struct _Ecore_X_Event_Window_State_Request       Ecore_X_Event_Window_State_Request;
typedef struct _Ecore_X_Event_Frame_Extents_Request      Ecore_X_Event_Frame_Extents_Request;
typedef struct _Ecore_X_Event_Ping                       Ecore_X_Event_Ping;
typedef struct _Ecore_X_Event_Desktop_Change             Ecore_X_Event_Desktop_Change;

typedef struct _Ecore_X_Event_Startup_Sequence           Ecore_X_Event_Startup_Sequence;

struct _Ecore_X_Event_Key_Down
{
   char   *keyname;
   char   *keysymbol;
   char   *key_compose;
   int     modifiers;
   int	   same_screen;
   Ecore_X_Window  win;
   Ecore_X_Window  event_win;
   Ecore_X_Window  root_win;
   Ecore_X_Time    time;
};

struct _Ecore_X_Event_Key_Up
{
   char   *keyname;
   char   *keysymbol;
   char   *key_compose;
   int     modifiers;
   int	   same_screen;
   Ecore_X_Window  win;
   Ecore_X_Window  event_win;
   Ecore_X_Window  root_win;
   Ecore_X_Time    time;
};

struct _Ecore_X_Event_Mouse_Button_Down
{
   int     button;
   int     modifiers;
   int     x, y;
   int	   same_screen;
   struct {
      int  x, y;
   } root;
   Ecore_X_Window  win;
   Ecore_X_Window  event_win;
   Ecore_X_Window  root_win;
   Ecore_X_Time    time;
   unsigned int    double_click : 1;
   unsigned int    triple_click : 1;
};

struct _Ecore_X_Event_Mouse_Button_Up
{
   int     button;
   int     modifiers;
   int     x, y;
   int	   same_screen;
   struct {
      int  x, y;
   } root;
   Ecore_X_Window  win;
   Ecore_X_Window  event_win;
   Ecore_X_Window  root_win;
   Ecore_X_Time    time;
   unsigned int    double_click : 1;
   unsigned int    triple_click : 1;
};

struct _Ecore_X_Event_Mouse_Move
{
   int     modifiers;
   int     x, y;
   int	   same_screen;
   struct {
      int  x, y;
   } root;
   Ecore_X_Window  win;
   Ecore_X_Window  event_win;
   Ecore_X_Window  root_win;
   Ecore_X_Time    time;
};

struct _Ecore_X_Event_Mouse_In
{
   int                  modifiers;
   int                  x, y;
   int			same_screen;
   struct {
      int  x, y;
   } root;
   Ecore_X_Window               win;
   Ecore_X_Window               event_win;
   Ecore_X_Window		root_win;
   Ecore_X_Event_Mode		mode;
   Ecore_X_Event_Detail		detail;
   Ecore_X_Time                 time;
};

struct _Ecore_X_Event_Mouse_Out
{
   int                  modifiers;
   int                  x, y;
   int			same_screen;
   struct {
      int  x, y;
   } root;
   Ecore_X_Window               win;
   Ecore_X_Window               event_win;
   Ecore_X_Window		root_win;
   Ecore_X_Event_Mode		mode;
   Ecore_X_Event_Detail		detail;
   Ecore_X_Time                 time;
};

struct _Ecore_X_Event_Mouse_Wheel
{
   int direction; /* 0 = default up/down wheel FIXME: more wheel types */
   int z; /* ...,-2,-1 = down, 1,2,... = up */
   int modifiers;
   int x, y;
   int same_screen;

   struct {
      int x, y;
   } root;

   Ecore_X_Window win;
   Ecore_X_Window event_win;
   Ecore_X_Window root_win;
   Ecore_X_Time   time;
};

struct _Ecore_X_Event_Window_Focus_In
{
   Ecore_X_Window       win;
   Ecore_X_Event_Mode   mode;
   Ecore_X_Event_Detail detail;
   Ecore_X_Time         time;
};

struct _Ecore_X_Event_Window_Focus_Out
{
   Ecore_X_Window       win;
   Ecore_X_Event_Mode   mode;
   Ecore_X_Event_Detail detail;
   Ecore_X_Time         time;
};

struct _Ecore_X_Event_Window_Keymap
{
   Ecore_X_Window  win;
};

struct _Ecore_X_Event_Window_Damage
{
   Ecore_X_Window  win;
   int             x, y, w, h;
   int             count;
   Ecore_X_Time    time;
};

struct _Ecore_X_Event_Window_Visibility_Change
{
   Ecore_X_Window  win;
   int             fully_obscured;
   Ecore_X_Time    time;
};

struct _Ecore_X_Event_Window_Create
{
   Ecore_X_Window  win;
   int             override;
   Ecore_X_Time    time;
};

struct _Ecore_X_Event_Window_Destroy
{
   Ecore_X_Window  win;
   Ecore_X_Time    time;
};

struct _Ecore_X_Event_Window_Hide
{
   Ecore_X_Window  win;
   Ecore_X_Time    time;
};

struct _Ecore_X_Event_Window_Show
{
   Ecore_X_Window  win;
   Ecore_X_Time    time;
};

struct _Ecore_X_Event_Window_Show_Request
{
   Ecore_X_Window  win;
   Ecore_X_Window  parent;
   Ecore_X_Time    time;
};

struct _Ecore_X_Event_Window_Reparent
{
   Ecore_X_Window  win;
   Ecore_X_Window  parent;
   Ecore_X_Time    time;
};

struct _Ecore_X_Event_Window_Configure
{
   Ecore_X_Window  win;
   Ecore_X_Window  abovewin;
   int             x, y, w, h;
   int             border;
   unsigned int    override : 1;
   unsigned int    from_wm : 1;
   Ecore_X_Time    time;
};

struct _Ecore_X_Event_Window_Configure_Request
{
   Ecore_X_Window  win;
   Ecore_X_Window  abovewin;
   int             x, y, w, h;
   int             border;
   Ecore_X_Window_Stack_Mode detail;
   unsigned long   value_mask;
   Ecore_X_Time    time;
};

struct _Ecore_X_Event_Window_Gravity
{
   Ecore_X_Window  win;
   Ecore_X_Time    time;
};

struct _Ecore_X_Event_Window_Resize_Request
{
   Ecore_X_Window  win;
   int             w, h;
   Ecore_X_Time    time;
};

struct _Ecore_X_Event_Window_Stack
{
   Ecore_X_Window  win;
   Ecore_X_Window  event_win;
   Ecore_X_Window_Stack_Mode detail;
   Ecore_X_Time    time;
};

struct _Ecore_X_Event_Window_Stack_Request
{
   Ecore_X_Window  win;
   Ecore_X_Window  parent;
   Ecore_X_Window_Stack_Mode detail;
   Ecore_X_Time    time;
};

struct _Ecore_X_Event_Window_Property
{
   Ecore_X_Window  win;
   Ecore_X_Atom    atom;
   Ecore_X_Time    time;
};

struct _Ecore_X_Event_Window_Colormap
{
   Ecore_X_Window   win;
   Ecore_X_Colormap cmap;
   int              installed;
   Ecore_X_Time     time;
};

struct _Ecore_X_Event_Selection_Clear
{
   Ecore_X_Window    win;
   Ecore_X_Selection selection;
   Ecore_X_Time      time;
};

struct _Ecore_X_Event_Selection_Request
{
   Ecore_X_Window    owner;
   Ecore_X_Window    requestor;
   Ecore_X_Time      time;
   Ecore_X_Atom      selection;
   Ecore_X_Atom      target;
   Ecore_X_Atom      property;
};

struct _Ecore_X_Event_Selection_Notify
{
   Ecore_X_Window             win;
   Ecore_X_Time               time;
   Ecore_X_Selection          selection;
   char                      *target;
   void                      *data;
};

struct _Ecore_X_Selection_Data
{
   enum {
	ECORE_X_SELECTION_CONTENT_NONE,
	ECORE_X_SELECTION_CONTENT_TEXT,
	ECORE_X_SELECTION_CONTENT_FILES,
	ECORE_X_SELECTION_CONTENT_TARGETS,
	ECORE_X_SELECTION_CONTENT_CUSTOM
   } content;
   unsigned char    *data;
   int               length;
   int               format;

   int             (*free)(void *data);
};

struct _Ecore_X_Selection_Data_Files
{
   Ecore_X_Selection_Data data;
   char     **files;
   int        num_files;
};

struct _Ecore_X_Selection_Data_Text
{
   Ecore_X_Selection_Data data;
   char     *text;
};

struct _Ecore_X_Selection_Data_Targets
{
   Ecore_X_Selection_Data data;
   char     **targets;
   int        num_targets;
};

struct _Ecore_X_Event_Xdnd_Enter
{
   Ecore_X_Window       win, source;

   char               **types;
   int                  num_types;
};

struct _Ecore_X_Event_Xdnd_Position
{
   Ecore_X_Window       win, source;
   struct {
      int x, y;
   } position;
   Ecore_X_Atom         action;
};

struct _Ecore_X_Event_Xdnd_Status
{
   Ecore_X_Window       win, target;
   int                  will_accept;
   Ecore_X_Rectangle    rectangle;
   Ecore_X_Atom         action;
};

struct _Ecore_X_Event_Xdnd_Leave
{
   Ecore_X_Window       win, source;
};

struct _Ecore_X_Event_Xdnd_Drop
{
   Ecore_X_Window       win, source;
   Ecore_X_Atom         action;
   struct {
      int x, y;
   } position;
};

struct _Ecore_X_Event_Xdnd_Finished
{
   Ecore_X_Window       win, target;
   int                  completed;
   Ecore_X_Atom         action;
};

struct _Ecore_X_Event_Client_Message
{
   Ecore_X_Window       win;
   Ecore_X_Atom         message_type;
   int                  format;
   union {
      char              b[20];
      short             s[10];
      long              l[5];
   }                    data;
   Ecore_X_Time         time;
};

struct _Ecore_X_Event_Window_Shape
{
   Ecore_X_Window  win;
   Ecore_X_Time    time;
};

struct _Ecore_X_Event_Screensaver_Notify
{
   Ecore_X_Window  win;
   int             on;
   Ecore_X_Time    time;
};

struct _Ecore_X_Event_Sync_Counter
{
   Ecore_X_Time    time;
};

struct _Ecore_X_Event_Sync_Alarm
{
   Ecore_X_Time       time;
   Ecore_X_Sync_Alarm alarm;
};

struct _Ecore_X_Event_Screen_Change
{
   Ecore_X_Window win, root;
   int            width, height;
};

struct _Ecore_X_Event_Window_Delete_Request
{
   Ecore_X_Window  win;
   Ecore_X_Time    time;
};

struct _Ecore_X_Event_Window_Prop_Title_Change
{
   Ecore_X_Window  win;
   char   *title;
   Ecore_X_Time    time;
};

struct _Ecore_X_Event_Window_Prop_Visible_Title_Change
{
   Ecore_X_Window  win;
   char   *title;
   Ecore_X_Time    time;
};

struct _Ecore_X_Event_Window_Prop_Icon_Name_Change
{
   Ecore_X_Window  win;
   char   *name;
   Ecore_X_Time    time;
};

struct _Ecore_X_Event_Window_Prop_Visible_Icon_Name_Change
{
   Ecore_X_Window  win;
   char   *name;
   Ecore_X_Time    time;
};

struct _Ecore_X_Event_Window_Prop_Client_Machine_Change
{
   Ecore_X_Window  win;
   char   *name;
   Ecore_X_Time    time;
};

struct _Ecore_X_Event_Window_Prop_Name_Class_Change
{
   Ecore_X_Window  win;
   char   *name;
   char   *clas;
   Ecore_X_Time    time;
};

struct _Ecore_X_Event_Window_Prop_Pid_Change
{
   Ecore_X_Window  win;
   pid_t   pid;
   Ecore_X_Time    time;
};

struct _Ecore_X_Event_Window_Prop_Desktop_Change
{
   Ecore_X_Window  win;
   long    desktop;
   Ecore_X_Time    time;
};

struct _Ecore_X_Event_Startup_Sequence
{
   Ecore_X_Window  win;
};

struct _Ecore_X_Event_Window_Move_Resize_Request
{
   Ecore_X_Window win;
   int            x, y;
   int            direction;
   int            button;
   int            source;
};

struct _Ecore_X_Event_Window_State_Request
{
   Ecore_X_Window              win;
   Ecore_X_Window_State_Action action;
   Ecore_X_Window_State        state[2];
   int                         source;
};

struct _Ecore_X_Event_Frame_Extents_Request
{
   Ecore_X_Window              win;
};

struct _Ecore_X_Event_Ping
{
   Ecore_X_Window  win;
   Ecore_X_Window  event_win;
   Ecore_X_Time    time;
};

struct _Ecore_X_Event_Desktop_Change
{
   Ecore_X_Window              win;
   unsigned int                desk;
   int                         source;
};

EAPI extern int ECORE_X_EVENT_KEY_DOWN;
EAPI extern int ECORE_X_EVENT_KEY_UP;
EAPI extern int ECORE_X_EVENT_MOUSE_BUTTON_DOWN;
EAPI extern int ECORE_X_EVENT_MOUSE_BUTTON_UP;
EAPI extern int ECORE_X_EVENT_MOUSE_MOVE;
EAPI extern int ECORE_X_EVENT_MOUSE_IN;
EAPI extern int ECORE_X_EVENT_MOUSE_OUT;
EAPI extern int ECORE_X_EVENT_MOUSE_WHEEL;
EAPI extern int ECORE_X_EVENT_WINDOW_FOCUS_IN;
EAPI extern int ECORE_X_EVENT_WINDOW_FOCUS_OUT;
EAPI extern int ECORE_X_EVENT_WINDOW_KEYMAP;
EAPI extern int ECORE_X_EVENT_WINDOW_DAMAGE;
EAPI extern int ECORE_X_EVENT_WINDOW_VISIBILITY_CHANGE;
EAPI extern int ECORE_X_EVENT_WINDOW_CREATE;
EAPI extern int ECORE_X_EVENT_WINDOW_DESTROY;
EAPI extern int ECORE_X_EVENT_WINDOW_HIDE;
EAPI extern int ECORE_X_EVENT_WINDOW_SHOW;
EAPI extern int ECORE_X_EVENT_WINDOW_SHOW_REQUEST;
EAPI extern int ECORE_X_EVENT_WINDOW_REPARENT;
EAPI extern int ECORE_X_EVENT_WINDOW_CONFIGURE;
EAPI extern int ECORE_X_EVENT_WINDOW_CONFIGURE_REQUEST;
EAPI extern int ECORE_X_EVENT_WINDOW_GRAVITY;
EAPI extern int ECORE_X_EVENT_WINDOW_RESIZE_REQUEST;
EAPI extern int ECORE_X_EVENT_WINDOW_STACK;
EAPI extern int ECORE_X_EVENT_WINDOW_STACK_REQUEST;
EAPI extern int ECORE_X_EVENT_WINDOW_PROPERTY;
EAPI extern int ECORE_X_EVENT_WINDOW_COLORMAP;
EAPI extern int ECORE_X_EVENT_WINDOW_MAPPING;
EAPI extern int ECORE_X_EVENT_SELECTION_CLEAR;
EAPI extern int ECORE_X_EVENT_SELECTION_REQUEST;
EAPI extern int ECORE_X_EVENT_SELECTION_NOTIFY;
EAPI extern int ECORE_X_EVENT_CLIENT_MESSAGE;
EAPI extern int ECORE_X_EVENT_WINDOW_SHAPE;
EAPI extern int ECORE_X_EVENT_SCREENSAVER_NOTIFY;
EAPI extern int ECORE_X_EVENT_SYNC_COUNTER;
EAPI extern int ECORE_X_EVENT_SYNC_ALARM;
EAPI extern int ECORE_X_EVENT_SCREEN_CHANGE;
EAPI extern int ECORE_X_EVENT_DAMAGE_NOTIFY;

EAPI extern int ECORE_X_EVENT_WINDOW_DELETE_REQUEST;
/*
EAPI extern int ECORE_X_EVENT_WINDOW_PROP_TITLE_CHANGE;
EAPI extern int ECORE_X_EVENT_WINDOW_PROP_VISIBLE_TITLE_CHANGE;
EAPI extern int ECORE_X_EVENT_WINDOW_PROP_ICON_NAME_CHANGE;
EAPI extern int ECORE_X_EVENT_WINDOW_PROP_VISIBLE_ICON_NAME_CHANGE;
EAPI extern int ECORE_X_EVENT_WINDOW_PROP_CLIENT_MACHINE_CHANGE;
EAPI extern int ECORE_X_EVENT_WINDOW_PROP_NAME_CLASS_CHANGE;
EAPI extern int ECORE_X_EVENT_WINDOW_PROP_PID_CHANGE;
EAPI extern int ECORE_X_EVENT_WINDOW_PROP_DESKTOP_CHANGE;
*/

EAPI extern int ECORE_X_EVENT_WINDOW_MOVE_RESIZE_REQUEST;
EAPI extern int ECORE_X_EVENT_WINDOW_STATE_REQUEST;
EAPI extern int ECORE_X_EVENT_FRAME_EXTENTS_REQUEST;
EAPI extern int ECORE_X_EVENT_PING;
EAPI extern int ECORE_X_EVENT_DESKTOP_CHANGE;

EAPI extern int ECORE_X_EVENT_STARTUP_SEQUENCE_NEW;
EAPI extern int ECORE_X_EVENT_STARTUP_SEQUENCE_CHANGE;
EAPI extern int ECORE_X_EVENT_STARTUP_SEQUENCE_REMOVE;

EAPI extern int ECORE_X_EVENT_XDND_ENTER;
EAPI extern int ECORE_X_EVENT_XDND_POSITION;
EAPI extern int ECORE_X_EVENT_XDND_STATUS;
EAPI extern int ECORE_X_EVENT_XDND_LEAVE;
EAPI extern int ECORE_X_EVENT_XDND_DROP;
EAPI extern int ECORE_X_EVENT_XDND_FINISHED;

EAPI extern int ECORE_X_MODIFIER_SHIFT;
EAPI extern int ECORE_X_MODIFIER_CTRL;
EAPI extern int ECORE_X_MODIFIER_ALT;
EAPI extern int ECORE_X_MODIFIER_WIN;

EAPI extern int ECORE_X_LOCK_SCROLL;
EAPI extern int ECORE_X_LOCK_NUM;
EAPI extern int ECORE_X_LOCK_CAPS;

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

	/**
	 * If enabled the window manager can ping the window to check
	 * if it is alive.
	 */
	ECORE_X_NET_WM_PROTOCOL_PING,

	/**
	 * If enabled the window manager can sync updating with the
	 * window (?)
	 */
	ECORE_X_NET_WM_PROTOCOL_SYNC_REQUEST,

	/* Number of defined items */
	ECORE_X_WM_PROTOCOL_NUM
} Ecore_X_WM_Protocol;

typedef enum _Ecore_X_Window_Input_Mode {
	/** The window can never be focused */
	ECORE_X_WINDOW_INPUT_MODE_NONE,

	/** The window can be focused by the WM but doesn't focus itself */
	ECORE_X_WINDOW_INPUT_MODE_PASSIVE,

	/** The window sets the focus itself if one of its sub-windows
	 * already is focused
	 */
	ECORE_X_WINDOW_INPUT_MODE_ACTIVE_LOCAL,

	/** The window sets the focus itself even if another window
	 * is currently focused
	 */
	ECORE_X_WINDOW_INPUT_MODE_ACTIVE_GLOBAL
} Ecore_X_Window_Input_Mode;

typedef enum _Ecore_X_Window_State_Hint {
   /** Do not provide any state hint to the window manager */
   ECORE_X_WINDOW_STATE_HINT_NONE = -1,

   /** The window wants to remain hidden and NOT iconified */
   ECORE_X_WINDOW_STATE_HINT_WITHDRAWN,

   /** The window wants to be mapped normally */
   ECORE_X_WINDOW_STATE_HINT_NORMAL,

   /** The window wants to start in an iconified state */
   ECORE_X_WINDOW_STATE_HINT_ICONIC,
} Ecore_X_Window_State_Hint;

typedef enum _Ecore_X_Window_Type {
    ECORE_X_WINDOW_TYPE_DESKTOP,
    ECORE_X_WINDOW_TYPE_DOCK,
    ECORE_X_WINDOW_TYPE_TOOLBAR,
    ECORE_X_WINDOW_TYPE_MENU,
    ECORE_X_WINDOW_TYPE_UTILITY,
    ECORE_X_WINDOW_TYPE_SPLASH,
    ECORE_X_WINDOW_TYPE_DIALOG,
    ECORE_X_WINDOW_TYPE_NORMAL,
    ECORE_X_WINDOW_TYPE_UNKNOWN
} Ecore_X_Window_Type;

typedef enum _Ecore_X_Action {
    ECORE_X_ACTION_MOVE,
    ECORE_X_ACTION_RESIZE,
    ECORE_X_ACTION_MINIMIZE,
    ECORE_X_ACTION_SHADE,
    ECORE_X_ACTION_STICK,
    ECORE_X_ACTION_MAXIMIZE_HORZ,
    ECORE_X_ACTION_MAXIMIZE_VERT,
    ECORE_X_ACTION_FULLSCREEN,
    ECORE_X_ACTION_CHANGE_DESKTOP,
    ECORE_X_ACTION_CLOSE
} Ecore_X_Action;

typedef enum _Ecore_X_Window_Configure_Mask {
   ECORE_X_WINDOW_CONFIGURE_MASK_X              = (1 << 0),
   ECORE_X_WINDOW_CONFIGURE_MASK_Y              = (1 << 1),
   ECORE_X_WINDOW_CONFIGURE_MASK_W              = (1 << 2),
   ECORE_X_WINDOW_CONFIGURE_MASK_H              = (1 << 3),
   ECORE_X_WINDOW_CONFIGURE_MASK_BORDER_WIDTH   = (1 << 4),
   ECORE_X_WINDOW_CONFIGURE_MASK_SIBLING        = (1 << 5),
   ECORE_X_WINDOW_CONFIGURE_MASK_STACK_MODE     = (1 << 6)
} Ecore_X_Window_Configure_Mask;

typedef enum _Ecore_X_Virtual_Keyboard_State {
    ECORE_X_VIRTUAL_KEYBOARD_STATE_UNKNOWN = 0,
    ECORE_X_VIRTUAL_KEYBOARD_STATE_OFF,
    ECORE_X_VIRTUAL_KEYBOARD_STATE_ON,
    ECORE_X_VIRTUAL_KEYBOARD_STATE_ALPHA,
    ECORE_X_VIRTUAL_KEYBOARD_STATE_NUMERIC,
    ECORE_X_VIRTUAL_KEYBOARD_STATE_PIN,
    ECORE_X_VIRTUAL_KEYBOARD_STATE_PHONE_NUMBER,
    ECORE_X_VIRTUAL_KEYBOARD_STATE_HEX,
    ECORE_X_VIRTUAL_KEYBOARD_STATE_TERMINAL,
    ECORE_X_VIRTUAL_KEYBOARD_STATE_PASSWORD
} Ecore_X_Virtual_Keyboard_State;


/* Window layer constants */
#define ECORE_X_WINDOW_LAYER_BELOW 2
#define ECORE_X_WINDOW_LAYER_NORMAL 4
#define ECORE_X_WINDOW_LAYER_ABOVE 6

/* Property list operations */
#define ECORE_X_PROP_LIST_REMOVE    0
#define ECORE_X_PROP_LIST_ADD       1
#define ECORE_X_PROP_LIST_TOGGLE    2

EAPI int              ecore_x_init(const char *name);
EAPI int              ecore_x_shutdown(void);
EAPI int              ecore_x_disconnect(void);
EAPI Ecore_X_Display *ecore_x_display_get(void);
EAPI Ecore_X_Connection *ecore_x_connection_get(void);
EAPI int              ecore_x_fd_get(void);
EAPI Ecore_X_Screen  *ecore_x_default_screen_get(void);
EAPI void             ecore_x_double_click_time_set(double t);
EAPI double           ecore_x_double_click_time_get(void);
EAPI void             ecore_x_flush(void);
EAPI void             ecore_x_sync(void);
EAPI void             ecore_x_killall(Ecore_X_Window root);
EAPI void             ecore_x_kill(Ecore_X_Window win);

EAPI Ecore_X_Time     ecore_x_current_time_get(void);

EAPI void             ecore_x_error_handler_set(void (*func) (void *data), const void *data);
EAPI void             ecore_x_io_error_handler_set(void (*func) (void *data), const void *data);
EAPI int              ecore_x_error_request_get(void);
EAPI int              ecore_x_error_code_get(void);

EAPI void             ecore_x_event_mask_set(Ecore_X_Window w, Ecore_X_Event_Mask mask);
EAPI void             ecore_x_event_mask_unset(Ecore_X_Window w, Ecore_X_Event_Mask mask);

EAPI int              ecore_x_selection_notify_send(Ecore_X_Window requestor, Ecore_X_Atom selection, Ecore_X_Atom target, Ecore_X_Atom property, Ecore_X_Time time);
EAPI void             ecore_x_selection_primary_prefetch(void);
EAPI void             ecore_x_selection_primary_fetch(void);
EAPI int              ecore_x_selection_primary_set(Ecore_X_Window w, const void *data, int size);
EAPI int              ecore_x_selection_primary_clear(void);
EAPI void             ecore_x_selection_secondary_prefetch(void);
EAPI void             ecore_x_selection_secondary_fetch(void);
EAPI int              ecore_x_selection_secondary_set(Ecore_X_Window w, const void *data, int size);
EAPI int              ecore_x_selection_secondary_clear(void);
EAPI void             ecore_x_selection_xdnd_prefetch(void);
EAPI void             ecore_x_selection_xdnd_fetch(void);
EAPI int              ecore_x_selection_xdnd_set(Ecore_X_Window w, const void *data, int size);
EAPI int              ecore_x_selection_xdnd_clear(void);
EAPI void             ecore_x_selection_clipboard_prefetch(void);
EAPI void             ecore_x_selection_clipboard_fetch(void);
EAPI int              ecore_x_selection_clipboard_set(Ecore_X_Window w, const void *data, int size);
EAPI int              ecore_x_selection_clipboard_clear(void);
EAPI void             ecore_x_selection_primary_request(Ecore_X_Window w, const char *target);
EAPI void             ecore_x_selection_secondary_request(Ecore_X_Window w, const char *target);
EAPI void             ecore_x_selection_xdnd_request(Ecore_X_Window w, const char *target);
EAPI void             ecore_x_selection_clipboard_request(Ecore_X_Window w, const char *target);
EAPI int              ecore_x_selection_convert(Ecore_X_Atom selection, Ecore_X_Atom target, void **data_ret);
EAPI void             ecore_x_selection_converter_add(char *target, int (*func)(char *target, void *data, int size, void **data_ret, int *size_ret));
EAPI void             ecore_x_selection_converter_atom_add(Ecore_X_Atom target, int (*func)(char *target, void *data, int size, void **data_ret, int *size_ret));
EAPI void             ecore_x_selection_converter_del(char *target);
EAPI void             ecore_x_selection_converter_atom_del(Ecore_X_Atom target);
EAPI void             ecore_x_selection_parser_add(const char *target, void *(*func)(const char *target, void *data, int size, int format));
EAPI void             ecore_x_selection_parser_del(const char *target);

EAPI void             ecore_x_dnd_aware_set(Ecore_X_Window win, int on);
EAPI void             ecore_x_dnd_version_get_prefetch(Ecore_X_Window window);
EAPI void             ecore_x_dnd_version_get_fetch(void);
EAPI int              ecore_x_dnd_version_get(Ecore_X_Window win);
EAPI void             ecore_x_dnd_type_get_prefetch(Ecore_X_Window window);
EAPI void             ecore_x_dnd_type_get_fetch(void);
EAPI int              ecore_x_dnd_type_isset(Ecore_X_Window win, const char *type);
EAPI void             ecore_x_dnd_type_set(Ecore_X_Window win, const char *type, int on);
EAPI void             ecore_x_dnd_types_set(Ecore_X_Window win, char **types, unsigned int num_types);
EAPI void             ecore_x_dnd_actions_set(Ecore_X_Window win, Ecore_X_Atom *actions, unsigned int num_actions);
EAPI void             ecore_x_dnd_begin_prefetch(Ecore_X_Window source);
EAPI void             ecore_x_dnd_begin_fetch(void);
EAPI int              ecore_x_dnd_begin(Ecore_X_Window source, unsigned char *data, int size);
EAPI int              ecore_x_dnd_drop(void);
EAPI void             ecore_x_dnd_send_status(int will_accept, int suppress, Ecore_X_Rectangle rectangle, Ecore_X_Atom action);
EAPI void             ecore_x_dnd_send_finished(void);

EAPI Ecore_X_Window   ecore_x_window_new(Ecore_X_Window parent, int x, int y, int w, int h);
EAPI Ecore_X_Window   ecore_x_window_override_new(Ecore_X_Window parent, int x, int y, int w, int h);
EAPI int              ecore_x_window_argb_get(Ecore_X_Window win);
EAPI Ecore_X_Window   ecore_x_window_manager_argb_new(Ecore_X_Window parent, int x, int y, int w, int h);
EAPI Ecore_X_Window   ecore_x_window_argb_new(Ecore_X_Window parent, int x, int y, int w, int h);
EAPI Ecore_X_Window   ecore_x_window_override_argb_new(Ecore_X_Window parent, int x, int y, int w, int h);
EAPI Ecore_X_Window   ecore_x_window_input_new(Ecore_X_Window parent, int x, int y, int w, int h);
EAPI void             ecore_x_window_configure(Ecore_X_Window win,
					       Ecore_X_Window_Configure_Mask mask,
					       int x, int y, int w, int h,
					       int border_width,
					       Ecore_X_Window sibling,
					       int stack_mode);
EAPI void             ecore_x_window_cursor_set(Ecore_X_Window win, Ecore_X_Cursor c);
EAPI void             ecore_x_window_del(Ecore_X_Window win);
EAPI void             ecore_x_window_ignore_set(Ecore_X_Window win, int ignore);
EAPI Ecore_X_Window  *ecore_x_window_ignore_list(int *num);

EAPI void             ecore_x_window_delete_request_send(Ecore_X_Window win);
EAPI void             ecore_x_window_show(Ecore_X_Window win);
EAPI void             ecore_x_window_hide(Ecore_X_Window win);
EAPI void             ecore_x_window_move(Ecore_X_Window win, int x, int y);
EAPI void             ecore_x_window_resize(Ecore_X_Window win, int w, int h);
EAPI void             ecore_x_window_move_resize(Ecore_X_Window win, int x, int y, int w, int h);
EAPI void             ecore_x_window_focus(Ecore_X_Window win);
EAPI void             ecore_x_window_focus_at_time(Ecore_X_Window win, Ecore_X_Time t);
EAPI void             ecore_x_get_input_focus_prefetch(void);
EAPI void             ecore_x_get_input_focus_fetch(void);
EAPI Ecore_X_Window   ecore_x_window_focus_get(void);
EAPI void             ecore_x_window_raise(Ecore_X_Window win);
EAPI void             ecore_x_window_lower(Ecore_X_Window win);
EAPI void             ecore_x_window_reparent(Ecore_X_Window win, Ecore_X_Window new_parent, int x, int y);
EAPI void             ecore_x_window_size_get(Ecore_X_Window win, int *w, int *h);
EAPI void             ecore_x_window_geometry_get(Ecore_X_Window win, int *x, int *y, int *w, int *h);
EAPI int              ecore_x_window_border_width_get(Ecore_X_Window win);
EAPI void             ecore_x_window_border_width_set(Ecore_X_Window win, int width);
EAPI int              ecore_x_window_depth_get(Ecore_X_Window win);
EAPI void             ecore_x_window_cursor_show(Ecore_X_Window win, int show);
EAPI void             ecore_x_window_defaults_set(Ecore_X_Window win);
EAPI int              ecore_x_window_visible_get(Ecore_X_Window win);
EAPI Ecore_X_Window   ecore_x_window_shadow_tree_at_xy_with_skip_get(Ecore_X_Window base, int x, int y, Ecore_X_Window *skip, int skip_num);
EAPI Ecore_X_Window   ecore_x_window_shadow_parent_get(Ecore_X_Window root, Ecore_X_Window win);
EAPI void             ecore_x_window_shadow_tree_flush(void);
EAPI Ecore_X_Window   ecore_x_window_root_get(Ecore_X_Window win);
EAPI Ecore_X_Window   ecore_x_window_at_xy_get(int x, int y);
EAPI Ecore_X_Window   ecore_x_window_at_xy_with_skip_get(int x, int y, Ecore_X_Window *skip, int skip_num);
EAPI Ecore_X_Window   ecore_x_window_at_xy_begin_get(Ecore_X_Window begin, int x, int y);
EAPI void             ecore_x_query_tree_prefetch(Ecore_X_Window window);
EAPI void             ecore_x_query_tree_fetch(void);
EAPI Ecore_X_Window   ecore_x_window_parent_get(Ecore_X_Window win);

EAPI void             ecore_x_window_background_color_set(Ecore_X_Window win,
							  unsigned short r,
							  unsigned short g,
							  unsigned short b);
EAPI void             ecore_x_window_gravity_set(Ecore_X_Window win,
						 Ecore_X_Gravity grav);
EAPI void             ecore_x_window_pixel_gravity_set(Ecore_X_Window win,
						       Ecore_X_Gravity grav);
EAPI void             ecore_x_window_pixmap_set(Ecore_X_Window win,
						Ecore_X_Pixmap pmap);
EAPI void             ecore_x_window_area_clear(Ecore_X_Window win,
						int x, int y, int w, int h);
EAPI void             ecore_x_window_area_expose(Ecore_X_Window win,
						 int x, int y, int w, int h);
EAPI void             ecore_x_window_override_set(Ecore_X_Window win, int override);

EAPI void             ecore_x_window_prop_card32_set(Ecore_X_Window win, Ecore_X_Atom atom,
						     unsigned int *val, unsigned int num);
EAPI void             ecore_x_window_prop_card32_get_prefetch(Ecore_X_Window window,
                                                              Ecore_X_Atom atom);
EAPI void             ecore_x_window_prop_card32_get_fetch(void);
EAPI int              ecore_x_window_prop_card32_get(Ecore_X_Window win, Ecore_X_Atom atom,
						     unsigned int *val, unsigned int len);
EAPI int              ecore_x_window_prop_card32_list_get(Ecore_X_Window win,
                                                          Ecore_X_Atom atom,
                                                          unsigned int **plst);

EAPI void             ecore_x_window_prop_xid_set(Ecore_X_Window win,
                                                  Ecore_X_Atom atom,
                                                  Ecore_X_Atom type,
                                                  Ecore_X_ID * lst,
                                                  unsigned int num);
EAPI void             ecore_x_window_prop_xid_get_prefetch(Ecore_X_Window window,
                                                           Ecore_X_Atom   atom,
                                                           Ecore_X_Atom   type);
EAPI void             ecore_x_window_prop_xid_get_fetch(void);
EAPI int              ecore_x_window_prop_xid_get(Ecore_X_Window win,
                                                  Ecore_X_Atom atom,
                                                  Ecore_X_Atom type,
                                                  Ecore_X_ID * lst,
                                                  unsigned int len);
EAPI int              ecore_x_window_prop_xid_list_get(Ecore_X_Window win,
                                                       Ecore_X_Atom atom,
                                                       Ecore_X_Atom type,
                                                       Ecore_X_ID ** plst);
EAPI void             ecore_x_window_prop_xid_list_change(Ecore_X_Window win,
                                                          Ecore_X_Atom atom,
                                                          Ecore_X_Atom type,
                                                          Ecore_X_ID item,
                                                          int op);
EAPI void             ecore_x_window_prop_atom_set(Ecore_X_Window win,
                                                   Ecore_X_Atom atom,
                                                   Ecore_X_Atom * val,
                                                   unsigned int num);
EAPI void             ecore_x_window_prop_atom_get_prefetch(Ecore_X_Window window,
                                                            Ecore_X_Atom   atom);
EAPI void             ecore_x_window_prop_atom_get_fetch(void);
EAPI int              ecore_x_window_prop_atom_get(Ecore_X_Window win,
                                                   Ecore_X_Atom atom,
                                                   Ecore_X_Atom * val,
                                                   unsigned int len);
EAPI int              ecore_x_window_prop_atom_list_get(Ecore_X_Window win,
                                                        Ecore_X_Atom atom,
                                                        Ecore_X_Atom ** plst);
EAPI void             ecore_x_window_prop_atom_list_change(Ecore_X_Window win,
                                                           Ecore_X_Atom atom,
                                                           Ecore_X_Atom item,
                                                           int op);
EAPI void             ecore_x_window_prop_window_set(Ecore_X_Window win,
                                                     Ecore_X_Atom atom,
                                                     Ecore_X_Window * val,
                                                     unsigned int num);
EAPI void             ecore_x_window_prop_window_get_prefetch(Ecore_X_Window window,
                                                              Ecore_X_Atom   atom);
EAPI void             ecore_x_window_prop_window_get_fetch(void);
EAPI int              ecore_x_window_prop_window_get(Ecore_X_Window win,
                                                     Ecore_X_Atom atom,
                                                     Ecore_X_Window * val,
                                                     unsigned int len);
EAPI int              ecore_x_window_prop_window_list_get(Ecore_X_Window win,
                                                          Ecore_X_Atom atom,
                                                          Ecore_X_Window ** plst);

EAPI Ecore_X_Atom     ecore_x_window_prop_any_type(void);
EAPI void             ecore_x_window_prop_property_set(Ecore_X_Window win, Ecore_X_Atom type, Ecore_X_Atom format, int size, void *data, int number);
EAPI void             ecore_x_window_prop_property_get_prefetch(Ecore_X_Window window,
                                                                Ecore_X_Atom   property,
                                                                Ecore_X_Atom   type);
EAPI void             ecore_x_window_prop_property_get_fetch(void);
EAPI int              ecore_x_window_prop_property_get(Ecore_X_Window win, Ecore_X_Atom property, Ecore_X_Atom type, int size, unsigned char **data, int *num);
EAPI void             ecore_x_window_prop_property_del(Ecore_X_Window win, Ecore_X_Atom property);
EAPI void             ecore_x_window_prop_list_prefetch(Ecore_X_Window window);
EAPI void             ecore_x_window_prop_list_fetch(void);
EAPI Ecore_X_Atom    *ecore_x_window_prop_list(Ecore_X_Window win, int *num_ret);
EAPI void             ecore_x_window_prop_string_set(Ecore_X_Window win, Ecore_X_Atom type, const char *str);
EAPI void             ecore_x_window_prop_string_get_prefetch(Ecore_X_Window window,
                                                              Ecore_X_Atom   type);
EAPI void             ecore_x_window_prop_string_get_fetch(void);
EAPI char            *ecore_x_window_prop_string_get(Ecore_X_Window win, Ecore_X_Atom type);
EAPI int              ecore_x_window_prop_protocol_isset(Ecore_X_Window win, Ecore_X_WM_Protocol protocol);
EAPI Ecore_X_WM_Protocol *ecore_x_window_prop_protocol_list_get(Ecore_X_Window win, int *num_ret);

EAPI void               ecore_x_window_shape_mask_set(Ecore_X_Window win, Ecore_X_Pixmap mask);
EAPI void               ecore_x_window_shape_window_set(Ecore_X_Window win, Ecore_X_Window shape_win);
EAPI void               ecore_x_window_shape_window_set_xy(Ecore_X_Window win, Ecore_X_Window shape_win, int x, int y);
EAPI void               ecore_x_window_shape_rectangle_set(Ecore_X_Window win, int x, int y, int w, int h);
EAPI void               ecore_x_window_shape_rectangles_set(Ecore_X_Window win, Ecore_X_Rectangle *rects, int num);
EAPI void               ecore_x_window_shape_window_add(Ecore_X_Window win, Ecore_X_Window shape_win);
EAPI void               ecore_x_window_shape_window_add_xy(Ecore_X_Window win, Ecore_X_Window shape_win, int x, int y);
EAPI void               ecore_x_window_shape_rectangle_add(Ecore_X_Window win, int x, int y, int w, int h);
EAPI void               ecore_x_window_shape_rectangle_clip(Ecore_X_Window win, int x, int y, int w, int h);
EAPI void               ecore_x_window_shape_rectangles_add(Ecore_X_Window win, Ecore_X_Rectangle *rects, int num);
EAPI void               ecore_x_window_shape_rectangles_get_prefetch(Ecore_X_Window window);
EAPI void               ecore_x_window_shape_rectangles_get_fetch(void);
EAPI Ecore_X_Rectangle *ecore_x_window_shape_rectangles_get(Ecore_X_Window win, int *num_ret);
EAPI void               ecore_x_window_shape_events_select(Ecore_X_Window win, int on);
EAPI void               ecore_x_window_shape_input_mask_set(Ecore_X_Window win, Ecore_X_Pixmap mask);

EAPI Ecore_X_Pixmap   ecore_x_pixmap_new(Ecore_X_Window win, int w, int h, int dep);
EAPI void             ecore_x_pixmap_del(Ecore_X_Pixmap pmap);
EAPI void             ecore_x_pixmap_paste(Ecore_X_Pixmap pmap, Ecore_X_Drawable dest, Ecore_X_GC gc, int sx, int sy, int w, int h, int dx, int dy);
EAPI void             ecore_x_pixmap_geometry_get(Ecore_X_Pixmap pmap, int *x, int *y, int *w, int *h);
EAPI int              ecore_x_pixmap_depth_get(Ecore_X_Pixmap pmap);

EAPI Ecore_X_GC       ecore_x_gc_new(Ecore_X_Drawable draw);
EAPI void             ecore_x_gc_del(Ecore_X_GC gc);

EAPI int              ecore_x_client_message32_send(Ecore_X_Window win, Ecore_X_Atom type, Ecore_X_Event_Mask mask, long d0, long d1, long d2, long d3, long d4);
EAPI int              ecore_x_client_message8_send(Ecore_X_Window win, Ecore_X_Atom type, const void *data, int len);
EAPI int              ecore_x_mouse_move_send(Ecore_X_Window win, int x, int y);
EAPI int              ecore_x_mouse_down_send(Ecore_X_Window win, int x, int y, int b);
EAPI int              ecore_x_mouse_up_send(Ecore_X_Window win, int x, int y, int b);

EAPI void            ecore_x_drawable_geometry_get_prefetch(Ecore_X_Drawable drawable);
EAPI void            ecore_x_drawable_geometry_get_fetch(void);
EAPI void            ecore_x_drawable_geometry_get(Ecore_X_Drawable d, int *x, int *y, int *w, int *h);
EAPI int             ecore_x_drawable_border_width_get(Ecore_X_Drawable d);
EAPI int             ecore_x_drawable_depth_get(Ecore_X_Drawable d);

EAPI int  ecore_x_cursor_color_supported_get(void);
EAPI Ecore_X_Cursor ecore_x_cursor_new(Ecore_X_Window win, int *pixels, int w, int h, int hot_x, int hot_y);
EAPI void ecore_x_cursor_free(Ecore_X_Cursor c);
EAPI Ecore_X_Cursor ecore_x_cursor_shape_get(int shape);
EAPI void ecore_x_cursor_size_set(int size);
EAPI int  ecore_x_cursor_size_get(void);


/* FIXME: these funcs need categorising */
EAPI Ecore_X_Window *ecore_x_window_root_list(int *num_ret);
EAPI Ecore_X_Window  ecore_x_window_root_first_get(void);
EAPI int             ecore_x_window_manage(Ecore_X_Window win);
EAPI void            ecore_x_window_container_manage(Ecore_X_Window win);
EAPI void            ecore_x_window_client_manage(Ecore_X_Window win);
EAPI void            ecore_x_window_sniff(Ecore_X_Window win);
EAPI void            ecore_x_window_client_sniff(Ecore_X_Window win);
EAPI void            ecore_x_atom_get_prefetch(const char *name);
EAPI void            ecore_x_atom_get_fetch(void);
EAPI Ecore_X_Atom    ecore_x_atom_get(const char *name);
EAPI void            ecore_x_atoms_get(const char **names, int num, Ecore_X_Atom *atoms);
       


EAPI void            ecore_x_icccm_init(void);
EAPI void            ecore_x_icccm_state_set(Ecore_X_Window win, Ecore_X_Window_State_Hint state);
EAPI Ecore_X_Window_State_Hint ecore_x_icccm_state_get(Ecore_X_Window win);
EAPI void            ecore_x_icccm_delete_window_send(Ecore_X_Window win, Ecore_X_Time t);
EAPI void            ecore_x_icccm_take_focus_send(Ecore_X_Window win, Ecore_X_Time t);
EAPI void            ecore_x_icccm_save_yourself_send(Ecore_X_Window win, Ecore_X_Time t);
EAPI void            ecore_x_icccm_move_resize_send(Ecore_X_Window win, int x, int y, int w, int h);
EAPI void  ecore_x_icccm_hints_set(Ecore_X_Window win,
				   int accepts_focus,
				   Ecore_X_Window_State_Hint initial_state,
				   Ecore_X_Pixmap icon_pixmap,
				   Ecore_X_Pixmap icon_mask,
				   Ecore_X_Window icon_window,
				   Ecore_X_Window window_group,
				   int is_urgent);
EAPI int  ecore_x_icccm_hints_get(Ecore_X_Window win,
				  int *accepts_focus,
				  Ecore_X_Window_State_Hint *initial_state,
				  Ecore_X_Pixmap *icon_pixmap,
				  Ecore_X_Pixmap *icon_mask,
				  Ecore_X_Window *icon_window,
				  Ecore_X_Window *window_group,
				  int *is_urgent);
EAPI void  ecore_x_icccm_size_pos_hints_set(Ecore_X_Window win,
					    int request_pos,
					    Ecore_X_Gravity gravity,
					    int min_w, int min_h,
					    int max_w, int max_h,
					    int base_w, int base_h,
					    int step_x, int step_y,
					    double min_aspect,
					    double max_aspect);
EAPI int   ecore_x_icccm_size_pos_hints_get(Ecore_X_Window win,
					    int *request_pos,
					    Ecore_X_Gravity *gravity,
					    int *min_w, int *min_h,
					    int *max_w, int *max_h,
					    int *base_w, int *base_h,
					    int *step_x, int *step_y,
					    double *min_aspect,
					    double *max_aspect);
EAPI void  ecore_x_icccm_title_set(Ecore_X_Window win, const char *t);
EAPI char *ecore_x_icccm_title_get(Ecore_X_Window win);
EAPI void  ecore_x_icccm_protocol_set(Ecore_X_Window win,
				      Ecore_X_WM_Protocol protocol,
				      int on);
EAPI int   ecore_x_icccm_protocol_isset(Ecore_X_Window win,
					Ecore_X_WM_Protocol protocol);
EAPI void  ecore_x_icccm_name_class_set(Ecore_X_Window win,
					const char *n,
					const char *c);
EAPI void  ecore_x_icccm_name_class_get(Ecore_X_Window win,
					char **n,
					char **c);
EAPI char *ecore_x_icccm_client_machine_get(Ecore_X_Window win);
EAPI void  ecore_x_icccm_command_set(Ecore_X_Window win, int argc, char **argv);
EAPI void  ecore_x_icccm_command_get(Ecore_X_Window win, int *argc, char ***argv);
EAPI char *ecore_x_icccm_icon_name_get(Ecore_X_Window win);
EAPI void  ecore_x_icccm_icon_name_set(Ecore_X_Window win, const char *t);
EAPI void  ecore_x_icccm_colormap_window_set(Ecore_X_Window win, Ecore_X_Window subwin);
EAPI void  ecore_x_icccm_colormap_window_unset(Ecore_X_Window win, Ecore_X_Window subwin);
EAPI void  ecore_x_icccm_transient_for_set(Ecore_X_Window win, Ecore_X_Window forwin);
EAPI void  ecore_x_icccm_transient_for_unset(Ecore_X_Window win);
EAPI Ecore_X_Window ecore_x_icccm_transient_for_get(Ecore_X_Window win);
EAPI void  ecore_x_icccm_window_role_set(Ecore_X_Window win, const char *role);
EAPI char *ecore_x_icccm_window_role_get(Ecore_X_Window win);
EAPI void  ecore_x_icccm_client_leader_set(Ecore_X_Window win, Ecore_X_Window l);
EAPI Ecore_X_Window ecore_x_icccm_client_leader_get(Ecore_X_Window win);
EAPI void ecore_x_icccm_iconic_request_send(Ecore_X_Window win, Ecore_X_Window root);


typedef enum _Ecore_X_MWM_Hint_Func
{
   ECORE_X_MWM_HINT_FUNC_ALL = (1 << 0),
   ECORE_X_MWM_HINT_FUNC_RESIZE = (1 << 1),
   ECORE_X_MWM_HINT_FUNC_MOVE = (1 << 2),
   ECORE_X_MWM_HINT_FUNC_MINIMIZE = (1 << 3),
   ECORE_X_MWM_HINT_FUNC_MAXIMIZE = (1 << 4),
   ECORE_X_MWM_HINT_FUNC_CLOSE = (1 << 5)
} Ecore_X_MWM_Hint_Func;

typedef enum _Ecore_X_MWM_Hint_Decor
{
   ECORE_X_MWM_HINT_DECOR_ALL = (1 << 0),
   ECORE_X_MWM_HINT_DECOR_BORDER = (1 << 1),
   ECORE_X_MWM_HINT_DECOR_RESIZEH = (1 << 2),
   ECORE_X_MWM_HINT_DECOR_TITLE = (1 << 3),
   ECORE_X_MWM_HINT_DECOR_MENU = (1 << 4),
   ECORE_X_MWM_HINT_DECOR_MINIMIZE = (1 << 5),
   ECORE_X_MWM_HINT_DECOR_MAXIMIZE = (1 << 6)
} Ecore_X_MWM_Hint_Decor;

typedef enum _Ecore_X_MWM_Hint_Input
{
   ECORE_X_MWM_HINT_INPUT_MODELESS = 0,
   ECORE_X_MWM_HINT_INPUT_PRIMARY_APPLICATION_MODAL = 1,
   ECORE_X_MWM_HINT_INPUT_SYSTEM_MODAL = 2,
   ECORE_X_MWM_HINT_INPUT_FULL_APPLICATION_MODAL = 3,
} Ecore_X_MWM_Hint_Input;

EAPI void ecore_x_mwm_hints_get_prefetch(Ecore_X_Window window);
EAPI void ecore_x_mwm_hints_get_fetch(void);
EAPI int  ecore_x_mwm_hints_get(Ecore_X_Window win,
                                Ecore_X_MWM_Hint_Func *fhint,
                                Ecore_X_MWM_Hint_Decor *dhint,
                                Ecore_X_MWM_Hint_Input *ihint);
EAPI void ecore_x_mwm_borderless_set(Ecore_X_Window win, int borderless);

/* netwm */
EAPI void                ecore_x_netwm_init(void);
EAPI void                ecore_x_netwm_shutdown(void);
EAPI void                ecore_x_netwm_wm_identify(Ecore_X_Window root, Ecore_X_Window check, const char *wm_name);
EAPI void                ecore_x_netwm_supported_set(Ecore_X_Window root, Ecore_X_Atom *supported, int num);
EAPI void                ecore_x_netwm_supported_get_prefetch(Ecore_X_Window root);
EAPI void                ecore_x_netwm_supported_get_fetch(void);
EAPI int                 ecore_x_netwm_supported_get(Ecore_X_Window root, Ecore_X_Atom **supported, int *num);
EAPI void                ecore_x_netwm_desk_count_set(Ecore_X_Window root, unsigned int n_desks);
EAPI void                ecore_x_netwm_desk_roots_set(Ecore_X_Window root, Ecore_X_Window *vroots, unsigned int n_desks);
EAPI void                ecore_x_netwm_desk_names_set(Ecore_X_Window root, const char **names, unsigned int n_desks);
EAPI void                ecore_x_netwm_desk_size_set(Ecore_X_Window root, unsigned int width, unsigned int height);
EAPI void                ecore_x_netwm_desk_workareas_set(Ecore_X_Window root, unsigned int *areas, unsigned int n_desks);
EAPI void                ecore_x_netwm_desk_current_set(Ecore_X_Window root, unsigned int desk);
EAPI void                ecore_x_netwm_desk_viewports_set(Ecore_X_Window root, unsigned int *origins, unsigned int n_desks);
EAPI void                ecore_x_netwm_desk_layout_set(Ecore_X_Window root, int orientation, int columns, int rows, int starting_corner);
EAPI void                ecore_x_netwm_showing_desktop_set(Ecore_X_Window root, int on);
EAPI void                ecore_x_netwm_client_list_set(Ecore_X_Window root, Ecore_X_Window *p_clients, unsigned int n_clients);
EAPI void                ecore_x_netwm_client_list_stacking_set(Ecore_X_Window root, Ecore_X_Window *p_clients, unsigned int n_clients);
EAPI void                ecore_x_netwm_client_active_set(Ecore_X_Window root, Ecore_X_Window win);
EAPI void                ecore_x_netwm_client_active_request(Ecore_X_Window root, Ecore_X_Window win, int type, Ecore_X_Window current_win);
EAPI void                ecore_x_netwm_name_set(Ecore_X_Window win, const char *name);
EAPI void                ecore_x_netwm_name_get_prefetch(Ecore_X_Window window);
EAPI void                ecore_x_netwm_name_get_fetch(void);
EAPI int                 ecore_x_netwm_name_get(Ecore_X_Window win, char **name);
EAPI void                ecore_x_netwm_startup_id_set(Ecore_X_Window win, const char *id);
EAPI void                ecore_x_netwm_startup_id_get_prefetch(Ecore_X_Window window);
EAPI void                ecore_x_netwm_startup_id_get_fetch(void);
EAPI int                 ecore_x_netwm_startup_id_get(Ecore_X_Window win, char **id);
EAPI void                ecore_x_netwm_visible_name_set(Ecore_X_Window win, const char *name);
EAPI void                ecore_x_netwm_visible_name_get_prefetch(Ecore_X_Window window);
EAPI void                ecore_x_netwm_visible_name_get_fetch(void);
EAPI int                 ecore_x_netwm_visible_name_get(Ecore_X_Window win, char **name);
EAPI void                ecore_x_netwm_icon_name_set(Ecore_X_Window win, const char *name);
EAPI void                ecore_x_netwm_icon_name_get_prefetch(Ecore_X_Window window);
EAPI void                ecore_x_netwm_icon_name_get_fetch(void);
EAPI int                 ecore_x_netwm_icon_name_get(Ecore_X_Window win, char **name);
EAPI void                ecore_x_netwm_visible_icon_name_set(Ecore_X_Window win, const char *name);
EAPI void                ecore_x_netwm_visible_icon_name_get_prefetch(Ecore_X_Window window);
EAPI void                ecore_x_netwm_visible_icon_name_get_fetch(void);
EAPI int                 ecore_x_netwm_visible_icon_name_get(Ecore_X_Window win, char **name);
EAPI void                ecore_x_netwm_desktop_set(Ecore_X_Window win, unsigned int desk);
EAPI void                ecore_x_netwm_desktop_get_prefetch(Ecore_X_Window window);
EAPI void                ecore_x_netwm_desktop_get_fetch(void);
EAPI int                 ecore_x_netwm_desktop_get(Ecore_X_Window win, unsigned int *desk);
EAPI void                ecore_x_netwm_strut_set(Ecore_X_Window win, int left, int right, int top, int bottom);
EAPI void                ecore_x_netwm_strut_get_prefetch(Ecore_X_Window window);
EAPI void                ecore_x_netwm_strut_get_fetch(void);
EAPI int                 ecore_x_netwm_strut_get(Ecore_X_Window win, int *left, int *right, int *top, int *bottom);
EAPI void                ecore_x_netwm_strut_partial_set(Ecore_X_Window win, int left, int right, int top, int bottom, int left_start_y, int left_end_y, int right_start_y, int right_end_y, int top_start_x, int top_end_x, int bottom_start_x, int bottom_end_x);
EAPI void                ecore_x_netwm_strut_partial_get_prefetch(Ecore_X_Window window);
EAPI void                ecore_x_netwm_strut_partial_get_fetch(void);
EAPI int                 ecore_x_netwm_strut_partial_get(Ecore_X_Window win, int *left, int *right, int *top, int *bottom, int *left_start_y, int *left_end_y, int *right_start_y, int *right_end_y, int *top_start_x, int *top_end_x, int *bottom_start_x, int *bottom_end_x);
EAPI void                ecore_x_netwm_icons_get_prefetch(Ecore_X_Window window);
EAPI void                ecore_x_netwm_icons_get_fetch(void);
EAPI int                 ecore_x_netwm_icons_get(Ecore_X_Window win, Ecore_X_Icon **icon, int *num);
EAPI void                ecore_x_netwm_icon_geometry_set(Ecore_X_Window win, int x, int y, int width, int height);
EAPI void                ecore_x_netwm_icon_geometry_get_prefetch(Ecore_X_Window window);
EAPI void                ecore_x_netwm_icon_geometry_get_fetch(void);
EAPI int                 ecore_x_netwm_icon_geometry_get(Ecore_X_Window win, int *x, int *y, int *width, int *height);
EAPI void                ecore_x_netwm_pid_set(Ecore_X_Window win, int pid);
EAPI void                ecore_x_netwm_pid_get_prefetch(Ecore_X_Window window);
EAPI void                ecore_x_netwm_pid_get_fetch(void);
EAPI int                 ecore_x_netwm_pid_get(Ecore_X_Window win, int *pid);
EAPI void                ecore_x_netwm_handled_icons_set(Ecore_X_Window win);
EAPI void                ecore_x_netwm_handled_icons_get_prefetch(Ecore_X_Window window);
EAPI void                ecore_x_netwm_handled_icons_get_fetch(void);
EAPI int                 ecore_x_netwm_handled_icons_get(Ecore_X_Window win);
EAPI void                ecore_x_netwm_user_time_set(Ecore_X_Window win, unsigned int time);
EAPI void                ecore_x_netwm_user_time_get_prefetch(Ecore_X_Window window);
EAPI void                ecore_x_netwm_user_time_get_fetch(void);
EAPI int                 ecore_x_netwm_user_time_get(Ecore_X_Window win, unsigned int *time);
EAPI void                ecore_x_netwm_window_state_set(Ecore_X_Window win, Ecore_X_Window_State *state, unsigned int num);
EAPI void                ecore_x_netwm_window_state_get_prefetch(Ecore_X_Window window);
EAPI void                ecore_x_netwm_window_state_get_fetch(void);
EAPI int                 ecore_x_netwm_window_state_get(Ecore_X_Window win, Ecore_X_Window_State **state, unsigned int *num);
EAPI void                ecore_x_netwm_window_type_set(Ecore_X_Window win, Ecore_X_Window_Type type);
EAPI void                ecore_x_netwm_window_type_get_prefetch(Ecore_X_Window window);
EAPI void                ecore_x_netwm_window_type_get_fetch(void);
EAPI int                 ecore_x_netwm_window_type_get(Ecore_X_Window win, Ecore_X_Window_Type *type);
EAPI int                 ecore_x_netwm_window_types_get(Ecore_X_Window win, Ecore_X_Window_Type **types);
EAPI int                 ecore_x_netwm_allowed_action_isset(Ecore_X_Window win, Ecore_X_Action action);
EAPI void                ecore_x_netwm_allowed_action_set(Ecore_X_Window win, Ecore_X_Action *action, unsigned int num);
EAPI void                ecore_x_netwm_allowed_action_get_prefetch(Ecore_X_Window window);
EAPI void                ecore_x_netwm_allowed_action_get_fetch(void);
EAPI int                 ecore_x_netwm_allowed_action_get(Ecore_X_Window win, Ecore_X_Action **action, unsigned int *num);
EAPI void                ecore_x_netwm_opacity_set(Ecore_X_Window win, unsigned int opacity);
EAPI void                ecore_x_netwm_opacity_get_prefetch(Ecore_X_Window window);
EAPI void                ecore_x_netwm_opacity_get_fetch(void);
EAPI int                 ecore_x_netwm_opacity_get(Ecore_X_Window win, unsigned int *opacity);
EAPI void                ecore_x_netwm_frame_size_set(Ecore_X_Window win, int fl, int fr, int ft, int fb);
EAPI void                ecore_x_netwm_frame_size_get_prefetch(Ecore_X_Window window);
EAPI void                ecore_x_netwm_frame_size_get_fetch(void);
EAPI int                 ecore_x_netwm_frame_size_get(Ecore_X_Window win, int *fl, int *fr, int *ft, int *fb);
EAPI void                ecore_x_netwm_sync_counter_get_prefetch(Ecore_X_Window window);
EAPI void                ecore_x_netwm_sync_counter_get_fetch(void);
EAPI int                 ecore_x_netwm_sync_counter_get(Ecore_X_Window win, Ecore_X_Sync_Counter *counter);
EAPI void                ecore_x_netwm_ping_send(Ecore_X_Window win);
EAPI void                ecore_x_netwm_sync_request_send(Ecore_X_Window win, unsigned int serial);
EAPI void                ecore_x_netwm_state_request_send(Ecore_X_Window win, Ecore_X_Window root, Ecore_X_Window_State s1, Ecore_X_Window_State s2, int set);
EAPI void                ecore_x_netwm_desktop_request_send(Ecore_X_Window win, Ecore_X_Window root, unsigned int desktop);




EAPI void                ecore_x_e_init(void);
EAPI void                ecore_x_e_frame_size_set(Ecore_X_Window win, int fl, int fr, int ft, int fb);
EAPI void                ecore_x_e_virtual_keyboard_set(Ecore_X_Window win, int is_keyboard);
EAPI int                 ecore_x_e_virtual_keyboard_get(Ecore_X_Window win);
EAPI void                ecore_x_e_virtual_keyboard_state_set(Ecore_X_Window win, Ecore_X_Virtual_Keyboard_State state);
EAPI Ecore_X_Virtual_Keyboard_State ecore_x_e_virtual_keyboard_state_get(Ecore_X_Window win);
EAPI void                ecore_x_e_virtual_keyboard_state_send(Ecore_X_Window win, Ecore_X_Virtual_Keyboard_State state);
       

EAPI void                ecore_x_xinerama_query_screens_prefetch(void);
EAPI void                ecore_x_xinerama_query_screens_fetch(void);
EAPI int                 ecore_x_xinerama_screen_count_get(void);
EAPI int                 ecore_x_xinerama_screen_geometry_get(int screen, int *x, int *y, int *w, int *h);

EAPI int                 ecore_x_screensaver_event_available_get(void);
EAPI void                ecore_x_screensaver_idle_time_prefetch(void);
EAPI void                ecore_x_screensaver_idle_time_fetch(void);
EAPI int                 ecore_x_screensaver_idle_time_get(void);
EAPI void                ecore_x_get_screensaver_prefetch(void);
EAPI void                ecore_x_get_screensaver_fetch(void);
EAPI void                ecore_x_screensaver_set(int timeout, int interval, int prefer_blanking, int allow_exposures);
EAPI void                ecore_x_screensaver_timeout_set(int timeout);
EAPI int                 ecore_x_screensaver_timeout_get(void);
EAPI void                ecore_x_screensaver_blank_set(int timeout);
EAPI int                 ecore_x_screensaver_blank_get(void);
EAPI void                ecore_x_screensaver_expose_set(int timeout);
EAPI int                 ecore_x_screensaver_expose_get(void);
EAPI void                ecore_x_screensaver_interval_set(int timeout);
EAPI int                 ecore_x_screensaver_interval_get(void);
EAPI void                ecore_x_screensaver_event_listen_set(int on);

/* FIXME: these funcs need categorising */

typedef struct _Ecore_X_Window_Attributes
{
   Ecore_X_Window     root;
   int                x, y, w, h;
   int                border;
   int                depth;
   unsigned char      visible : 1;
   unsigned char      viewable : 1;
   unsigned char      override : 1;
   unsigned char      input_only : 1;
   unsigned char      save_under : 1;
   struct {
	Ecore_X_Event_Mask mine;
	Ecore_X_Event_Mask all;
	Ecore_X_Event_Mask no_propagate;
   } event_mask;
   Ecore_X_Gravity    window_gravity;
   Ecore_X_Gravity    pixel_gravity;
   Ecore_X_Colormap   colormap;
   Ecore_X_Visual     visual;
   /* FIXME: missing
    * int map_installed;
    * Screen *screen;
    */
} Ecore_X_Window_Attributes;

EAPI void ecore_x_get_window_attributes_prefetch(Ecore_X_Window window);
EAPI void ecore_x_get_window_attributes_fetch(void);
EAPI int  ecore_x_window_attributes_get(Ecore_X_Window win, Ecore_X_Window_Attributes *att_ret);
EAPI void ecore_x_window_save_set_add(Ecore_X_Window win);
EAPI void ecore_x_window_save_set_del(Ecore_X_Window win);
EAPI Ecore_X_Window *ecore_x_window_children_get(Ecore_X_Window win, int *num);

EAPI int  ecore_x_pointer_control_set(int accel_num, int accel_denom, int threshold);
EAPI void ecore_x_pointer_control_get_prefetch(void);
EAPI void ecore_x_pointer_control_get_fetch(void);
EAPI int  ecore_x_pointer_control_get(int *accel_num, int *accel_denom, int *threshold);
EAPI int  ecore_x_pointer_grab(Ecore_X_Window win);
EAPI int  ecore_x_pointer_confine_grab(Ecore_X_Window win);
EAPI void ecore_x_pointer_ungrab(void);
EAPI int  ecore_x_pointer_warp(Ecore_X_Window win, int x, int y);
EAPI int  ecore_x_keyboard_grab(Ecore_X_Window win);
EAPI void ecore_x_keyboard_ungrab(void);
EAPI void ecore_x_grab(void);
EAPI void ecore_x_ungrab(void);
EAPI void ecore_x_passive_grab_replay_func_set(int (*func) (void *data, int event_type, void *event), void *data);
EAPI void ecore_x_window_button_grab(Ecore_X_Window win, int button,
				     Ecore_X_Event_Mask event_mask,
				     int mod, int any_mod);
EAPI void ecore_x_window_button_ungrab(Ecore_X_Window win, int button,
				       int mod, int any_mod);
EAPI void ecore_x_window_key_grab(Ecore_X_Window win, const char *key,
				  int mod, int any_mod);
EAPI void ecore_x_window_key_ungrab(Ecore_X_Window win, const char *key,
				    int mod, int any_mod);

EAPI void ecore_x_focus_reset(void);
EAPI void ecore_x_events_allow_all(void);
EAPI void ecore_x_pointer_last_xy_get(int *x, int *y);
EAPI void ecore_x_pointer_xy_get_prefetch(Ecore_X_Window window);
EAPI void ecore_x_pointer_xy_get_fetch(void);
EAPI void ecore_x_pointer_xy_get(Ecore_X_Window win, int *x, int *y);

/* ecore_x_sync.c */
EAPI Ecore_X_Sync_Alarm ecore_x_sync_alarm_new(Ecore_X_Sync_Counter counter);
EAPI int                ecore_x_sync_alarm_free(Ecore_X_Sync_Alarm alarm);

/* ecore_x_randr.c */
typedef struct _Ecore_X_Screen_Size Ecore_X_Screen_Size;
struct _Ecore_X_Screen_Size
{
   int width, height;
};

typedef struct _Ecore_X_Screen_Refresh_Rate Ecore_X_Screen_Refresh_Rate;
struct _Ecore_X_Screen_Refresh_Rate
{
   int rate;
};

EAPI int                     ecore_x_randr_query();
EAPI int                     ecore_x_randr_events_select(Ecore_X_Window win, int on);
EAPI void                    ecore_x_randr_get_screen_info_prefetch(Ecore_X_Window window);
EAPI void                    ecore_x_randr_get_screen_info_fetch(void);
EAPI Ecore_X_Randr_Rotation  ecore_x_randr_screen_rotations_get(Ecore_X_Window root);
EAPI Ecore_X_Randr_Rotation  ecore_x_randr_screen_rotation_get(Ecore_X_Window root);
EAPI void                    ecore_x_randr_screen_rotation_set(Ecore_X_Window root, Ecore_X_Randr_Rotation rot);
EAPI Ecore_X_Screen_Size    *ecore_x_randr_screen_sizes_get(Ecore_X_Window root, int *num);
EAPI Ecore_X_Screen_Size     ecore_x_randr_current_screen_size_get(Ecore_X_Window root);
EAPI int                     ecore_x_randr_screen_size_set(Ecore_X_Window root, Ecore_X_Screen_Size size);

EAPI Ecore_X_Screen_Refresh_Rate *ecore_x_randr_screen_refresh_rates_get(Ecore_X_Window root, int size_id, int *num);
EAPI Ecore_X_Screen_Refresh_Rate  ecore_x_randr_current_screen_refresh_rate_get(Ecore_X_Window root);

EAPI int ecore_x_randr_screen_refresh_rate_set(Ecore_X_Window root, Ecore_X_Screen_Size size, Ecore_X_Screen_Refresh_Rate rate);

/* XRender Support (horrendously incomplete) */
typedef Ecore_X_ID  Ecore_X_Picture;

/* XFixes Extension Support */
typedef Ecore_X_ID  Ecore_X_Region;

typedef enum _Ecore_X_Region_Type {
     ECORE_X_REGION_BOUNDING,
     ECORE_X_REGION_CLIP
} Ecore_X_Region_Type;

EAPI Ecore_X_Region ecore_x_region_new(Ecore_X_Rectangle *rects, int num);
EAPI Ecore_X_Region ecore_x_region_new_from_bitmap(Ecore_X_Pixmap bitmap);
EAPI Ecore_X_Region ecore_x_region_new_from_window(Ecore_X_Window win, Ecore_X_Region_Type type);
EAPI Ecore_X_Region ecore_x_region_new_from_gc(Ecore_X_GC gc);
EAPI Ecore_X_Region ecore_x_region_new_from_picture(Ecore_X_Picture picture);
EAPI void           ecore_x_region_del(Ecore_X_Region region);
EAPI void           ecore_x_region_set(Ecore_X_Region region, Ecore_X_Rectangle *rects, int num);
EAPI void           ecore_x_region_copy(Ecore_X_Region dest, Ecore_X_Region source);
EAPI void           ecore_x_region_combine(Ecore_X_Region dest, Ecore_X_Region source1, Ecore_X_Region source2);
EAPI void           ecore_x_region_intersect(Ecore_X_Region dest, Ecore_X_Region source1, Ecore_X_Region source2);
EAPI void           ecore_x_region_subtract(Ecore_X_Region dest, Ecore_X_Region source1, Ecore_X_Region source2);
EAPI void           ecore_x_region_invert(Ecore_X_Region dest, Ecore_X_Rectangle *bounds, Ecore_X_Region source);
EAPI void           ecore_x_region_translate(Ecore_X_Region region, int dx, int dy);
EAPI void           ecore_x_region_extents(Ecore_X_Region dest, Ecore_X_Region source);
EAPI void           ecore_x_region_fetch_prefetch(Ecore_X_Region region);
EAPI void           ecore_x_region_fetch_fetch(void);
EAPI Ecore_X_Rectangle * ecore_x_region_fetch(Ecore_X_Region region, int *num, Ecore_X_Rectangle *bounds);
EAPI void           ecore_x_region_expand(Ecore_X_Region dest, Ecore_X_Region source, unsigned int left, unsigned int right, unsigned int top, unsigned int bottom);
EAPI void           ecore_x_region_gc_clip_set(Ecore_X_Region region, Ecore_X_GC gc, int x_origin, int y_origin);
EAPI void           ecore_x_region_window_shape_set(Ecore_X_Region region, Ecore_X_Window win, Ecore_X_Shape_Type type, int x_offset, int y_offset);
EAPI void           ecore_x_region_picture_clip_set(Ecore_X_Region region, Ecore_X_Picture picture, int x_origin, int y_origin);

/* XComposite Extension Support */
EAPI int               ecore_x_composite_query(void);

/* XDamage Extension Support */
typedef Ecore_X_ID  Ecore_X_Damage;

typedef enum _Ecore_X_Damage_Report_Level {
     ECORE_X_DAMAGE_REPORT_RAW_RECTANGLES,
     ECORE_X_DAMAGE_REPORT_DELTA_RECTANGLES,
     ECORE_X_DAMAGE_REPORT_BOUNDING_BOX,
     ECORE_X_DAMAGE_REPORT_NON_EMPTY
} Ecore_X_Damage_Report_Level;

struct _Ecore_X_Event_Damage {
     Ecore_X_Damage_Report_Level level;
     Ecore_X_Drawable drawable;
     Ecore_X_Damage   damage;
     int              more;
     Ecore_X_Time     time;
     Ecore_X_Rectangle	area;
     Ecore_X_Rectangle  geometry;
};

typedef struct _Ecore_X_Event_Damage  Ecore_X_Event_Damage;

EAPI int               ecore_x_damage_query(void);
EAPI Ecore_X_Damage    ecore_x_damage_new(Ecore_X_Drawable d, Ecore_X_Damage_Report_Level level);
EAPI void              ecore_x_damage_del(Ecore_X_Damage damage);
EAPI void              ecore_x_damage_subtract(Ecore_X_Damage damage, Ecore_X_Region repair, Ecore_X_Region parts);

EAPI int               ecore_x_screen_is_composited(int screen);

EAPI int               ecore_x_dpms_query(void);
EAPI void              ecore_x_dpms_capable_get_prefetch(void);
EAPI void              ecore_x_dpms_capable_get_fetch(void);
EAPI int               ecore_x_dpms_capable_get(void);
EAPI void              ecore_x_dpms_enable_get_prefetch(void);
EAPI void              ecore_x_dpms_enable_get_fetch(void);
EAPI int               ecore_x_dpms_enabled_get(void);
EAPI void              ecore_x_dpms_enabled_set(int enabled);
EAPI void              ecore_x_dpms_timeouts_get_prefetch(void);
EAPI void              ecore_x_dpms_timeouts_get_fetch(void);
EAPI void              ecore_x_dpms_timeouts_get(unsigned int *standby, unsigned int *suspend, unsigned int *off);
EAPI int               ecore_x_dpms_timeouts_set(unsigned int standby, unsigned int suspend, unsigned int off);
EAPI unsigned int      ecore_x_dpms_timeout_standby_get(void);
EAPI unsigned int      ecore_x_dpms_timeout_suspend_get(void);
EAPI unsigned int      ecore_x_dpms_timeout_off_get(void);
EAPI void              ecore_x_dpms_timeout_standby_set(unsigned int new_timeout);
EAPI void              ecore_x_dpms_timeout_suspend_set(unsigned int new_timeout);
EAPI void              ecore_x_dpms_timeout_off_set(unsigned int new_timeout);

EAPI int ecore_x_test_fake_key_down(const char *key);
EAPI int ecore_x_test_fake_key_up(const char *key);
EAPI int ecore_x_test_fake_key_press(const char *key);
EAPI const char *ecore_x_keysym_string_get(int keysym);
   
#ifdef __cplusplus
}
#endif

#endif
