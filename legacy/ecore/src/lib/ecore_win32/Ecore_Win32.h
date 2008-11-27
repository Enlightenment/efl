/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifndef __ECORE_WIN32_H__
#define __ECORE_WIN32_H__


#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef EFL_ECORE_WIN32_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif /* ! DLL_EXPORT */
# else
#  define EAPI __declspec(dllimport)
# endif /* ! EFL_ECORE_WIN32_BUILD */
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
#endif /* ! _WIN32 */


#ifdef __cplusplus
extern "C" {
#endif


typedef void Ecore_Win32_Window;
typedef void Ecore_Win32_Cursor;

/* Window state */

typedef enum
{
   /* The window is iconified. */
   ECORE_WIN32_WINDOW_STATE_ICONIFIED,
   /* The window is a modal dialog box. */
   ECORE_WIN32_WINDOW_STATE_MODAL,
   /* The window manager should keep the window's position fixed
    * even if the virtual desktop scrolls. */
   ECORE_WIN32_WINDOW_STATE_STICKY,
   /* The window has the maximum vertical size. */
   ECORE_WIN32_WINDOW_STATE_MAXIMIZED_VERT,
   /* The window has the maximum horizontal size. */
   ECORE_WIN32_WINDOW_STATE_MAXIMIZED_HORZ,
   /* The window has the maximum horizontal and vertical size. */
   ECORE_WIN32_WINDOW_STATE_MAXIMIZED,
   /* The window is shaded. */
   ECORE_WIN32_WINDOW_STATE_SHADED,
   /* The window is invisible (i.e. minimized/iconified) */
   ECORE_WIN32_WINDOW_STATE_HIDDEN,
   /* The window should fill the entire screen and have no
    * window border/decorations */
   ECORE_WIN32_WINDOW_STATE_FULLSCREEN,
   /* The following are not documented because they are not
    * intended for use in applications. */
   ECORE_WIN32_WINDOW_STATE_ABOVE,
   ECORE_WIN32_WINDOW_STATE_BELOW,
   /* FIXME: Documentation */
   ECORE_WIN32_WINDOW_STATE_DEMANDS_ATTENTION,
   /* Unknown state */
   ECORE_WIN32_WINDOW_STATE_UNKNOWN
} Ecore_Win32_Window_State;

/* Window type */

typedef enum
{
   /* Desktop feature*/
   ECORE_WIN32_WINDOW_TYPE_DESKTOP,
   /* Dock window (should be on top of other windows */
   ECORE_WIN32_WINDOW_TYPE_DOCK,
   /* Toolbar window */
   ECORE_WIN32_WINDOW_TYPE_TOOLBAR,
   /* Menu window */
   ECORE_WIN32_WINDOW_TYPE_MENU,
   /* Small persistent utility window, such as a palette or toolbox */
   ECORE_WIN32_WINDOW_TYPE_UTILITY,
   /* Splash screen window displayed as an application is starting up */
   ECORE_WIN32_WINDOW_TYPE_SPLASH,
   /* Dialog window */
   ECORE_WIN32_WINDOW_TYPE_DIALOG,
   /* Normal top-level window */
   ECORE_WIN32_WINDOW_TYPE_NORMAL,
   /* Unknown type */
   ECORE_WIN32_WINDOW_TYPE_UNKNOWN
} Ecore_Win32_Window_Type;

/*cursor shapes */

typedef enum
{
   ECORE_WIN32_CURSOR_SHAPE_APP_STARTING, /* Standard arrow and small hourglass */
   ECORE_WIN32_CURSOR_SHAPE_ARROW,        /* Standard arrow */
   ECORE_WIN32_CURSOR_SHAPE_CROSS,        /* Crosshair */
   ECORE_WIN32_CURSOR_SHAPE_HAND,         /* Hand */
   ECORE_WIN32_CURSOR_SHAPE_HELP,         /* Arrow and question mark */
   ECORE_WIN32_CURSOR_SHAPE_I_BEAM,       /* I-beam */
   ECORE_WIN32_CURSOR_SHAPE_NO,           /* Slashed circle */
   ECORE_WIN32_CURSOR_SHAPE_SIZE_ALL,     /* Four-pointed arrow pointing north, south, east, and west */
   ECORE_WIN32_CURSOR_SHAPE_SIZE_NESW,    /* Double-pointed arrow pointing northeast and southwest */
   ECORE_WIN32_CURSOR_SHAPE_SIZE_NS,      /* Double-pointed arrow pointing north and south */
   ECORE_WIN32_CURSOR_SHAPE_SIZE_NWSE,    /* Double-pointed arrow pointing northwest and southeast */
   ECORE_WIN32_CURSOR_SHAPE_SIZE_WE,      /* Double-pointed arrow pointing west and east */
   ECORE_WIN32_CURSOR_SHAPE_UP_ARROW,     /* Vertical arrow */
   ECORE_WIN32_CURSOR_SHAPE_WAIT          /* Hourglass */
} Ecore_Win32_Cursor_Shape;

/* Events */

typedef struct _Ecore_Win32_Event_Key_Down              Ecore_Win32_Event_Key_Down;
typedef struct _Ecore_Win32_Event_Key_Up                Ecore_Win32_Event_Key_Up;
typedef struct _Ecore_Win32_Event_Mouse_Button_Down     Ecore_Win32_Event_Mouse_Button_Down;
typedef struct _Ecore_Win32_Event_Mouse_Button_Up       Ecore_Win32_Event_Mouse_Button_Up;
typedef struct _Ecore_Win32_Event_Mouse_Move            Ecore_Win32_Event_Mouse_Move;
typedef struct _Ecore_Win32_Event_Mouse_In              Ecore_Win32_Event_Mouse_In;
typedef struct _Ecore_Win32_Event_Mouse_Out             Ecore_Win32_Event_Mouse_Out;
typedef struct _Ecore_Win32_Event_Mouse_Wheel           Ecore_Win32_Event_Mouse_Wheel;
typedef struct _Ecore_Win32_Event_Window_Focus_In       Ecore_Win32_Event_Window_Focus_In;
typedef struct _Ecore_Win32_Event_Window_Focus_Out      Ecore_Win32_Event_Window_Focus_Out;
typedef struct _Ecore_Win32_Event_Window_Damage         Ecore_Win32_Event_Window_Damage;
typedef struct _Ecore_Win32_Event_Window_Create         Ecore_Win32_Event_Window_Create;
typedef struct _Ecore_Win32_Event_Window_Destroy        Ecore_Win32_Event_Window_Destroy;
typedef struct _Ecore_Win32_Event_Window_Hide           Ecore_Win32_Event_Window_Hide;
typedef struct _Ecore_Win32_Event_Window_Show           Ecore_Win32_Event_Window_Show;
typedef struct _Ecore_Win32_Event_Window_Configure      Ecore_Win32_Event_Window_Configure;
typedef struct _Ecore_Win32_Event_Window_Resize         Ecore_Win32_Event_Window_Resize;
typedef struct _Ecore_Win32_Event_Window_Delete_Request Ecore_Win32_Event_Window_Delete_Request;

struct _Ecore_Win32_Event_Key_Down
{
   Ecore_Win32_Window *window;
   char               *keyname;
   char               *keysymbol;
   char               *keycompose;
   double              time;
};

struct _Ecore_Win32_Event_Key_Up
{
   Ecore_Win32_Window *window;
   char               *keyname;
   char               *keysymbol;
   char               *keycompose;
   double              time;
};

struct _Ecore_Win32_Event_Mouse_Button_Down
{
   Ecore_Win32_Window *window;
   int                 button;
   int                 x;
   int                 y;
   double              time;
   unsigned int        double_click : 1;
   unsigned int        triple_click : 1;
};

struct _Ecore_Win32_Event_Mouse_Button_Up
{
   Ecore_Win32_Window *window;
   int                 button;
   int                 x;
   int                 y;
   double              time;
   unsigned int        double_click : 1;
   unsigned int        triple_click : 1;
};

struct _Ecore_Win32_Event_Mouse_Move
{
   Ecore_Win32_Window *window;
   int                 x;
   int                 y;
   double              time;
};

struct _Ecore_Win32_Event_Mouse_In
{
   Ecore_Win32_Window *window;
   int                 x;
   int                 y;
   double              time;
};

struct _Ecore_Win32_Event_Mouse_Out
{
   Ecore_Win32_Window *window;
   int                 x;
   int                 y;
   double              time;
};

struct _Ecore_Win32_Event_Mouse_Wheel
{
   Ecore_Win32_Window *window;
   int                 direction; /* 0 = default up/down wheel FIXME: more wheel types */
   int                 z; /* ...,-2,-1 = down, 1,2,... = up */
   int                 modifiers;
   int                 x;
   int                 y;
   double              time;
};

struct _Ecore_Win32_Event_Window_Focus_In
{
   Ecore_Win32_Window *window;
   double              time;
};

struct _Ecore_Win32_Event_Window_Focus_Out
{
   Ecore_Win32_Window *window;
   double              time;
};

struct _Ecore_Win32_Event_Window_Damage
{
   Ecore_Win32_Window *window;
   int                 x;
   int                 y;
   int                 width;
   int                 height;
   double              time;
};

struct _Ecore_Win32_Event_Window_Create
{
   Ecore_Win32_Window *window;
   double              time;
};

struct _Ecore_Win32_Event_Window_Destroy
{
   Ecore_Win32_Window *window;
   double              time;
};

struct _Ecore_Win32_Event_Window_Hide
{
   Ecore_Win32_Window *window;
   double              time;
};

struct _Ecore_Win32_Event_Window_Show
{
   Ecore_Win32_Window *window;
   double              time;
};

struct _Ecore_Win32_Event_Window_Configure
{
   Ecore_Win32_Window *window;
   Ecore_Win32_Window *abovewin;
   int                 x;
   int                 y;
   int                 width;
   int                 height;
   double              time;
};

struct _Ecore_Win32_Event_Window_Resize
{
   Ecore_Win32_Window *window;
   int                 width;
   int                 height;
   double              time;
};

struct _Ecore_Win32_Event_Window_Delete_Request
{
   Ecore_Win32_Window *window;
   double              time;
};

#define ECORE_WIN32_DND_EVENT_DRAG_ENTER     1
#define ECORE_WIN32_DND_EVENT_DRAG_OVER      2
#define ECORE_WIN32_DND_EVENT_DRAG_LEAVE     3
#define ECORE_WIN32_DND_EVENT_DROP           4


typedef int (*Ecore_Win32_Dnd_DropTarget_Callback)(void *window, int event, int pt_x, int pt_y, void *data, int size);

EAPI extern int ECORE_WIN32_EVENT_KEY_DOWN;
EAPI extern int ECORE_WIN32_EVENT_KEY_UP;
EAPI extern int ECORE_WIN32_EVENT_MOUSE_BUTTON_DOWN;
EAPI extern int ECORE_WIN32_EVENT_MOUSE_BUTTON_UP;
EAPI extern int ECORE_WIN32_EVENT_MOUSE_MOVE;
EAPI extern int ECORE_WIN32_EVENT_MOUSE_IN;
EAPI extern int ECORE_WIN32_EVENT_MOUSE_OUT;
EAPI extern int ECORE_WIN32_EVENT_MOUSE_WHEEL;
EAPI extern int ECORE_WIN32_EVENT_WINDOW_FOCUS_IN;
EAPI extern int ECORE_WIN32_EVENT_WINDOW_FOCUS_OUT;
EAPI extern int ECORE_WIN32_EVENT_WINDOW_DAMAGE;
EAPI extern int ECORE_WIN32_EVENT_WINDOW_CREATE;
EAPI extern int ECORE_WIN32_EVENT_WINDOW_DESTROY;
EAPI extern int ECORE_WIN32_EVENT_WINDOW_HIDE;
EAPI extern int ECORE_WIN32_EVENT_WINDOW_SHOW;
EAPI extern int ECORE_WIN32_EVENT_WINDOW_CONFIGURE;
EAPI extern int ECORE_WIN32_EVENT_WINDOW_RESIZE;
EAPI extern int ECORE_WIN32_EVENT_WINDOW_DELETE_REQUEST;


/* Core */

EAPI int    ecore_win32_init();
EAPI int    ecore_win32_shutdown();
EAPI int    ecore_win32_screen_depth_get();
EAPI double ecore_win32_current_time_get(void);
EAPI void   ecore_win32_message_loop_begin (void);

/* Window */

EAPI Ecore_Win32_Window *ecore_win32_window_new(Ecore_Win32_Window *parent,
                                                int                 x,
                                                int                 y,
                                                int                 width,
                                                int                 height);
EAPI Ecore_Win32_Window *ecore_win32_window_override_new(Ecore_Win32_Window *parent,
                                                         int                 x,
                                                         int                 y,
                                                         int                 width,
                                                         int                 height);

EAPI void ecore_win32_window_del(Ecore_Win32_Window *window);

EAPI void *ecore_win32_window_hwnd_get(Ecore_Win32_Window *window);

EAPI void ecore_win32_window_move(Ecore_Win32_Window *window,
                                  int                 x,
                                  int                 y);

EAPI void ecore_win32_window_resize(Ecore_Win32_Window *window,
                                    int                 width,
                                    int                 height);

EAPI void ecore_win32_window_move_resize(Ecore_Win32_Window *window,
                                         int                 x,
                                         int                 y,
                                         int                 width,
                                         int                 height);

EAPI void ecore_win32_window_geometry_get(Ecore_Win32_Window *window,
                                          int                *x,
                                          int                *y,
                                          int                *width,
                                          int                *height);

EAPI void ecore_win32_window_size_get(Ecore_Win32_Window *window,
                                      int                *width,
                                      int                *height);

EAPI void ecore_win32_window_size_min_set(Ecore_Win32_Window *window,
                                          unsigned int        min_width,
                                          unsigned int        min_height);

EAPI void ecore_win32_window_size_min_get(Ecore_Win32_Window *window,
                                          unsigned int       *min_width,
                                          unsigned int       *min_height);

EAPI void ecore_win32_window_size_max_set(Ecore_Win32_Window *window,
                                          unsigned int        max_width,
                                          unsigned int        max_height);

EAPI void ecore_win32_window_size_max_get(Ecore_Win32_Window *window,
                                          unsigned int       *max_width,
                                          unsigned int       *max_height);

EAPI void ecore_win32_window_size_base_set(Ecore_Win32_Window *window,
                                           unsigned int        base_width,
                                           unsigned int        base_height);

EAPI void ecore_win32_window_size_base_get(Ecore_Win32_Window *window,
                                           unsigned int       *base_width,
                                           unsigned int       *base_height);

EAPI void ecore_win32_window_size_step_set(Ecore_Win32_Window *window,
                                           unsigned int        step_width,
                                           unsigned int        step_height);

EAPI void ecore_win32_window_size_step_get(Ecore_Win32_Window *window,
                                           unsigned int       *step_width,
                                           unsigned int       *step_height);

EAPI void ecore_win32_window_show(Ecore_Win32_Window *window);

EAPI void ecore_win32_window_hide(Ecore_Win32_Window *window);

EAPI void ecore_win32_window_raise(Ecore_Win32_Window *window);

EAPI void ecore_win32_window_lower(Ecore_Win32_Window *window);

EAPI void ecore_win32_window_title_set(Ecore_Win32_Window *window,
                                       const char         *title);

EAPI void ecore_win32_window_focus_set(Ecore_Win32_Window *window);

EAPI void ecore_win32_window_iconified_set(Ecore_Win32_Window *window,
                                           int                 on);

EAPI void ecore_win32_window_borderless_set(Ecore_Win32_Window *window,
                                            int                 on);

EAPI void ecore_win32_window_fullscreen_set(Ecore_Win32_Window *window,
                                            int                 on);

EAPI void ecore_win32_window_shape_set(Ecore_Win32_Window *window,
                                       unsigned short      width,
                                       unsigned short      height,
                                       unsigned char      *mask);

EAPI void ecore_win32_window_cursor_set(Ecore_Win32_Window *window,
                                        Ecore_Win32_Cursor *cursor);

EAPI void ecore_win32_window_state_set(Ecore_Win32_Window       *window,
                                       Ecore_Win32_Window_State *state,
                                       unsigned int              num);

EAPI void ecore_win32_window_state_request_send(Ecore_Win32_Window      *window,
                                                Ecore_Win32_Window_State state,
                                                unsigned int             set);

EAPI void ecore_win32_window_type_set(Ecore_Win32_Window      *window,
                                      Ecore_Win32_Window_Type  type);

/* Cursor */

EAPI Ecore_Win32_Cursor *ecore_win32_cursor_new(const void *pixels_and,
                                                const void *pixels_xor,
                                                int         width,
                                                int         height,
                                                int         hot_x,
                                                int         hot_y);

EAPI void                ecore_win32_cursor_free(Ecore_Win32_Cursor *cursor);

EAPI Ecore_Win32_Cursor *ecore_win32_cursor_shape_get(Ecore_Win32_Cursor_Shape shape);

EAPI int                 ecore_win32_cursor_size_get(void);



/* Drag and drop */
EAPI int ecore_win32_dnd_init();
EAPI int ecore_win32_dnd_shutdown();
EAPI int ecore_win32_dnd_begin(const char *data,
                               int         size);
EAPI int ecore_win32_dnd_register_drop_target(Ecore_Win32_Window                 *window,
                                              Ecore_Win32_Dnd_DropTarget_Callback callback);
EAPI void ecore_win32_dnd_unregister_drop_target(Ecore_Win32_Window *window);


#ifdef __cplusplus
}
#endif


#endif /* __ECORE_WIN32_H__ */
