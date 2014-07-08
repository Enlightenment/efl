#ifndef __ECORE_WIN32_H__
#define __ECORE_WIN32_H__

/*
 * DO NOT USE THIS HEADER. IT IS WORK IN PROGRESS. IT IS NOT FINAL AND
 * THE API MAY CHANGE.
 */

#ifndef ECORE_WIN32_WIP_POZEFLKSD
# ifdef _MSC_VER
#  pragma message ("You are using a work in progress API. This API is not stable")
#  pragma message ("and is subject to change. You use this at your own risk.")
# else
#  warning "You are using a work in progress API. This API is not stable"
#  warning "and is subject to change. You use this at your own risk."
# endif
#endif

#include <Eina.h>

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

/**
 * @defgroup Ecore_Win32_Group Ecore_Win32 library
 * @ingroup Ecore
 *
 * @{
 */

/**
 * @typedef Ecore_Win32_Window_State
 * State of a window.
 */
typedef enum
{
   ECORE_WIN32_WINDOW_STATE_ICONIFIED, /**< iconified window */
   ECORE_WIN32_WINDOW_STATE_MODAL, /**< modal dialog box */
   ECORE_WIN32_WINDOW_STATE_STICKY, /**< sticky window */
   ECORE_WIN32_WINDOW_STATE_MAXIMIZED_VERT, /**< maximum vertical sized window */
   ECORE_WIN32_WINDOW_STATE_MAXIMIZED_HORZ, /**< maximum horizontal sized window */
   ECORE_WIN32_WINDOW_STATE_MAXIMIZED, /**< maximum sized window */
   ECORE_WIN32_WINDOW_STATE_SHADED, /**< shaded window */
   ECORE_WIN32_WINDOW_STATE_HIDDEN, /**< hidden (minimized or iconified) window */
   ECORE_WIN32_WINDOW_STATE_FULLSCREEN, /**< fullscreen window */
   ECORE_WIN32_WINDOW_STATE_ABOVE, /**< above window */
   ECORE_WIN32_WINDOW_STATE_BELOW, /**< below window */
   ECORE_WIN32_WINDOW_STATE_DEMANDS_ATTENTION, /**< To document */
   ECORE_WIN32_WINDOW_STATE_UNKNOWN /**< Unknown state */
} Ecore_Win32_Window_State;

/**
 * @typedef Ecore_Win32_Window_Type
 * Type of a window.
 */
typedef enum
{
   ECORE_WIN32_WINDOW_TYPE_DESKTOP, /**< Desktop feature */
   ECORE_WIN32_WINDOW_TYPE_DOCK,    /**< Dock window (should be on top of other windows) */
   ECORE_WIN32_WINDOW_TYPE_TOOLBAR, /**< Toolbar window */
   ECORE_WIN32_WINDOW_TYPE_MENU,    /**< Menu window */
   ECORE_WIN32_WINDOW_TYPE_UTILITY, /**< Small persistent utility window, such as a palette or toolbox */
   ECORE_WIN32_WINDOW_TYPE_SPLASH,  /**< Splash screen window displayed as an application is starting up */
   ECORE_WIN32_WINDOW_TYPE_DIALOG,  /**< Dialog window */
   ECORE_WIN32_WINDOW_TYPE_NORMAL,  /**< Normal top-level window */
   ECORE_WIN32_WINDOW_TYPE_UNKNOWN  /**< Unknown type */
} Ecore_Win32_Window_Type;

/**
 * @typedef Ecore_Win32_Cursor_Shape
 * Shape of a cursor.
 */
typedef enum
{
   ECORE_WIN32_CURSOR_SHAPE_APP_STARTING, /**< Standard arrow and small hourglass */
   ECORE_WIN32_CURSOR_SHAPE_ARROW,        /**< Standard arrow */
   ECORE_WIN32_CURSOR_SHAPE_CROSS,        /**< Crosshair */
   ECORE_WIN32_CURSOR_SHAPE_HAND,         /**< Hand */
   ECORE_WIN32_CURSOR_SHAPE_HELP,         /**< Arrow and question mark */
   ECORE_WIN32_CURSOR_SHAPE_I_BEAM,       /**< I-beam */
   ECORE_WIN32_CURSOR_SHAPE_NO,           /**< Slashed circle */
   ECORE_WIN32_CURSOR_SHAPE_SIZE_ALL,     /**< Four-pointed arrow pointing north, south, east, and west */
   ECORE_WIN32_CURSOR_SHAPE_SIZE_NESW,    /**< Double-pointed arrow pointing northeast and southwest */
   ECORE_WIN32_CURSOR_SHAPE_SIZE_NS,      /**< Double-pointed arrow pointing north and south */
   ECORE_WIN32_CURSOR_SHAPE_SIZE_NWSE,    /**< Double-pointed arrow pointing northwest and southeast */
   ECORE_WIN32_CURSOR_SHAPE_SIZE_WE,      /**< Double-pointed arrow pointing west and east */
   ECORE_WIN32_CURSOR_SHAPE_UP_ARROW,     /**< Vertical arrow */
   ECORE_WIN32_CURSOR_SHAPE_WAIT          /**< Hourglass */
} Ecore_Win32_Cursor_Shape;

/**
 * @typedef Ecore_Win32_DnD_State
 * State of a DnD operation.
 */
typedef enum
{
   ECORE_WIN32_DND_EVENT_DRAG_ENTER = 1, /**< Drag enter */
   ECORE_WIN32_DND_EVENT_DRAG_OVER  = 2, /**< Drag over */
   ECORE_WIN32_DND_EVENT_DRAG_LEAVE = 3, /**< Drag leave */
   ECORE_WIN32_DND_EVENT_DROP       = 4  /**< Drop */
} Ecore_Win32_DnD_State;

/**
 * @typedef Ecore_Win32_Window
 * Abstract type for a window.
 */
typedef struct _Ecore_Win32_Window Ecore_Win32_Window;

/**
 * @typedef Ecore_Win32_Cursor
 * Abstract type for a cursor.
 */
typedef void Ecore_Win32_Cursor;


/**
 * @typedef Ecore_Win32_Event_Mouse_In
 * Event sent when the mouse enters the window.
 */
typedef struct _Ecore_Win32_Event_Mouse_In              Ecore_Win32_Event_Mouse_In;

/**
 * @typedef Ecore_Win32_Event_Mouse_Out
 * Event sent when the mouse leaves the window.
 */
typedef struct _Ecore_Win32_Event_Mouse_Out             Ecore_Win32_Event_Mouse_Out;

/**
 * @typedef Ecore_Win32_Event_Window_Focus_In
 * Event sent when the window gets the focus.
 */
typedef struct _Ecore_Win32_Event_Window_Focus_In       Ecore_Win32_Event_Window_Focus_In;

/**
 * @typedef Ecore_Win32_Event_Window_Focus_Out
 * Event sent when the window looses the focus.
 */
typedef struct _Ecore_Win32_Event_Window_Focus_Out      Ecore_Win32_Event_Window_Focus_Out;

/**
 * @typedef Ecore_Win32_Event_Window_Damage
 * Event sent when the window is damaged.
 */
typedef struct _Ecore_Win32_Event_Window_Damage         Ecore_Win32_Event_Window_Damage;

/**
 * @typedef Ecore_Win32_Event_Window_Create
 * Event sent when the window is created.
 */
typedef struct _Ecore_Win32_Event_Window_Create         Ecore_Win32_Event_Window_Create;

/**
 * @typedef Ecore_Win32_Event_Window_Destroy
 * Event sent when the window is destroyed.
 */
typedef struct _Ecore_Win32_Event_Window_Destroy        Ecore_Win32_Event_Window_Destroy;

/**
 * @typedef Ecore_Win32_Event_Window_Hide
 * Event sent when the window is hidden.
 */
typedef struct _Ecore_Win32_Event_Window_Hide           Ecore_Win32_Event_Window_Hide;

/**
 * @typedef Ecore_Win32_Event_Window_Show
 * Event sent when the window is shown.
 */
typedef struct _Ecore_Win32_Event_Window_Show           Ecore_Win32_Event_Window_Show;

/**
 * @typedef Ecore_Win32_Event_Window_Configure
 * Event sent when the window is configured.
 */
typedef struct _Ecore_Win32_Event_Window_Configure      Ecore_Win32_Event_Window_Configure;

/**
 * @typedef Ecore_Win32_Event_Window_Resize
 * Event sent when the window is resized.
 */
typedef struct _Ecore_Win32_Event_Window_Resize         Ecore_Win32_Event_Window_Resize;

/**
 * @typedef Ecore_Win32_Event_Window_Delete_Request
 * Event sent when the window is deleted.
 */
typedef struct _Ecore_Win32_Event_Window_Delete_Request Ecore_Win32_Event_Window_Delete_Request;

/**
 * @struct _Ecore_Win32_Event_Mouse_In
 * Event sent when the mouse enters the window.
 */
struct _Ecore_Win32_Event_Mouse_In
{
   Ecore_Win32_Window *window; /**< The window that received the event */
   int                 x; /**< The x coordinate where the mouse leaved */
   int                 y; /**< The y coordinate where the mouse entered */
   unsigned long       timestamp; /**< The time the event occurred */
   unsigned int        modifiers; /**< The keyboard modifiers */
};

/**
 * @struct _Ecore_Win32_Event_Mouse_Out
 * Event sent when the mouse leaves the window.
 */
struct _Ecore_Win32_Event_Mouse_Out
{
   Ecore_Win32_Window *window; /**< The window that received the event */
   int                 x; /**< The x coordinate where the mouse leaved */
   int                 y; /**< The y coordinate where the mouse leaved */
   unsigned long       timestamp; /**< The time the event occurred */
   unsigned int        modifiers; /**< The keyboard modifiers */
};

/**
 * @struct _Ecore_Win32_Event_Window_Focus_In
 * Event sent when the window gets the focus.
 */
struct _Ecore_Win32_Event_Window_Focus_In
{
   Ecore_Win32_Window *window; /**< The window that received the event */
   unsigned long       timestamp; /**< The time the event occurred */
};

/**
 * @struct _Ecore_Win32_Event_Window_Focus_Out
 * Event sent when the window looses the focus.
 */
struct _Ecore_Win32_Event_Window_Focus_Out
{
   Ecore_Win32_Window *window; /**< The window that received the event */
   unsigned long       timestamp; /**< The time the event occurred */
};

/**
 * @struct _Ecore_Win32_Event_Window_Damage
 * Event sent when the window is damaged.
 */
struct _Ecore_Win32_Event_Window_Damage
{
   Ecore_Win32_Window *window; /**< The window that received the event */
   int                 x; /**< The x coordinate of the top left corner of the damaged region */
   int                 y; /**< The y coordinate of the top left corner of the damaged region */
   int                 width; /**< The width of the damaged region */
   int                 height; /**< The time the event occurred */
   unsigned long       timestamp; /**< The time the event occurred */
};

/**
 * @struct _Ecore_Win32_Event_Window_Create
 * Event sent when the window is created.
 */
struct _Ecore_Win32_Event_Window_Create
{
   Ecore_Win32_Window *window; /**< The window that received the event */
   unsigned long       timestamp; /**< The time the event occurred */
};

/**
 * @struct _Ecore_Win32_Event_Window_Destroy
 * Event sent when the window is destroyed.
 */
struct _Ecore_Win32_Event_Window_Destroy
{
   Ecore_Win32_Window *window; /**< The window that received the event */
   unsigned long       timestamp; /**< The time the event occurred */
};

/**
 * @struct _Ecore_Win32_Event_Window_Hide
 * Event sent when the window is hidden.
 */
struct _Ecore_Win32_Event_Window_Hide
{
   Ecore_Win32_Window *window; /**< The window that received the event */
   unsigned long       timestamp; /**< The time the event occurred */
};

/**
 * @struct _Ecore_Win32_Event_Window_Show
 * Event sent when the window is shown.
 */
struct _Ecore_Win32_Event_Window_Show
{
   Ecore_Win32_Window *window; /**< The window that received the event */
   unsigned long       timestamp; /**< The time the event occurred */
};

/**
 * @struct _Ecore_Win32_Event_Window_Configure
 * Event sent when the window is configured.
 */
struct _Ecore_Win32_Event_Window_Configure
{
   Ecore_Win32_Window *window; /**< The window that received the event */
   Ecore_Win32_Window *abovewin;
   int                 x; /**< The new x coordinate of the top left corner */
   int                 y; /**< The new y coordinate of the top left corner */
   int                 width; /**< The new width */
   int                 height; /**< The new height */
   unsigned long       timestamp; /**< The time the event occurred */
};

/**
 * @struct _Ecore_Win32_Event_Window_Resize
 * Event sent when the window is resized.
 */
struct _Ecore_Win32_Event_Window_Resize
{
   Ecore_Win32_Window *window; /**< The window that received the event */
   int                 width; /**< The new width */
   int                 height; /**< The new height */
   unsigned long       timestamp; /**< The time the event occurred */
};

/**
 * @struct _Ecore_Win32_Event_Window_Delete_Request
 * Event sent when the window is deleted.
 */
struct _Ecore_Win32_Event_Window_Delete_Request
{
   Ecore_Win32_Window *window; /**< The window that received the event */
   unsigned long       timestamp; /**< The time the event occurred */
};

/**
 * @typedef Ecore_Win32_Dnd_DropTarget_Callback
 * Callback type for Drop operations. See ecore_win32_dnd_register_drop_target().
 */
typedef int (*Ecore_Win32_Dnd_DropTarget_Callback)(void *window, int event, int pt_x, int pt_y, void *data, int size);

EAPI extern int ECORE_WIN32_EVENT_MOUSE_IN; /**< Ecore_Event for the #Ecore_Win32_Event_Mouse_In event */
EAPI extern int ECORE_WIN32_EVENT_MOUSE_OUT; /**< Ecore_Event for the #Ecore_Win32_Event_Mouse_Out event */
EAPI extern int ECORE_WIN32_EVENT_WINDOW_FOCUS_IN; /**< Ecore_Event for the #Ecore_Win32_Event_Window_Focus_In event */
EAPI extern int ECORE_WIN32_EVENT_WINDOW_FOCUS_OUT; /**< Ecore_Event for the #Ecore_Win32_Event_Window_Focus_Out event */
EAPI extern int ECORE_WIN32_EVENT_WINDOW_DAMAGE; /**< Ecore_Event for the Ecore_Win32_Event_Damage event */
EAPI extern int ECORE_WIN32_EVENT_WINDOW_CREATE; /**< Ecore_Event for the Ecore_Win32_Event_Create event */
EAPI extern int ECORE_WIN32_EVENT_WINDOW_DESTROY; /**< Ecore_Event for the Ecore_Win32_Event_Destroy event */
EAPI extern int ECORE_WIN32_EVENT_WINDOW_HIDE; /**< Ecore_Event for the Ecore_Win32_Event_Hide event */
EAPI extern int ECORE_WIN32_EVENT_WINDOW_SHOW; /**< Ecore_Event for the Ecore_Win32_Event_Show event */
EAPI extern int ECORE_WIN32_EVENT_WINDOW_CONFIGURE; /**< Ecore_Event for the Ecore_Win32_Event_Configure event */
EAPI extern int ECORE_WIN32_EVENT_WINDOW_RESIZE; /**< Ecore_Event for the Ecore_Win32_Event_Resize event */
EAPI extern int ECORE_WIN32_EVENT_WINDOW_DELETE_REQUEST; /**< Ecore_Event for the #Ecore_Win32_Event_Window_Delete_Request event */


/* Core */

EAPI int           ecore_win32_init();
EAPI int           ecore_win32_shutdown();
EAPI int           ecore_win32_screen_depth_get();
EAPI void          ecore_win32_double_click_time_set(double t);
EAPI double        ecore_win32_double_click_time_get(void);
EAPI unsigned long ecore_win32_current_time_get(void);

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

EAPI void ecore_win32_window_free(Ecore_Win32_Window *window);

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
                                          int                 min_width,
                                          int                 min_height);

EAPI void ecore_win32_window_size_min_get(Ecore_Win32_Window *window,
                                          int                *min_width,
                                          int                *min_height);

EAPI void ecore_win32_window_size_max_set(Ecore_Win32_Window *window,
                                          int                 max_width,
                                          int                 max_height);

EAPI void ecore_win32_window_size_max_get(Ecore_Win32_Window *window,
                                          int                *max_width,
                                          int                *max_height);

EAPI void ecore_win32_window_size_base_set(Ecore_Win32_Window *window,
                                           int                 base_width,
                                           int                 base_height);

EAPI void ecore_win32_window_size_base_get(Ecore_Win32_Window *window,
                                           int                *base_width,
                                           int                *base_height);

EAPI void ecore_win32_window_size_step_set(Ecore_Win32_Window *window,
                                           int                 step_width,
                                           int                 step_height);

EAPI void ecore_win32_window_size_step_get(Ecore_Win32_Window *window,
                                           int                *step_width,
                                           int                *step_height);

EAPI void ecore_win32_window_show(Ecore_Win32_Window *window);

EAPI void ecore_win32_window_hide(Ecore_Win32_Window *window);

EAPI void ecore_win32_window_raise(Ecore_Win32_Window *window);

EAPI void ecore_win32_window_lower(Ecore_Win32_Window *window);

EAPI void ecore_win32_window_title_set(Ecore_Win32_Window *window,
                                       const char         *title);

EAPI void ecore_win32_window_focus(Ecore_Win32_Window *window);

EAPI void *ecore_win32_window_focus_get(void);

EAPI void ecore_win32_window_iconified_set(Ecore_Win32_Window *window,
                                           Eina_Bool           on);

EAPI void ecore_win32_window_borderless_set(Ecore_Win32_Window *window,
                                            Eina_Bool           on);

EAPI void ecore_win32_window_fullscreen_set(Ecore_Win32_Window *window,
                                            Eina_Bool           on);

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

EAPI Ecore_Win32_Cursor *ecore_win32_cursor_shaped_new(Ecore_Win32_Cursor_Shape shape);

EAPI void                ecore_win32_cursor_size_get(int *width, int *height);



/* Drag and drop */
EAPI int       ecore_win32_dnd_init();
EAPI int       ecore_win32_dnd_shutdown();
EAPI Eina_Bool ecore_win32_dnd_begin(const char *data,
                                     int         size);
EAPI Eina_Bool ecore_win32_dnd_register_drop_target(Ecore_Win32_Window                 *window,
                                                    Ecore_Win32_Dnd_DropTarget_Callback callback);
EAPI void      ecore_win32_dnd_unregister_drop_target(Ecore_Win32_Window *window);

/**
 * @}
 */


#ifdef __cplusplus
}
#endif


#endif /* __ECORE_WIN32_H__ */
