#ifndef __ECORE_WINCE_H__
#define __ECORE_WINCE_H__

/*
 * DO NOT USE THIS HEADER. IT IS WORK IN PROGRESS. IT IS NOT FINAL AND
 * THE API MAY CHANGE.
 */

#ifndef ECORE_WINCE_WIP_OSXCKQSD
# warning "You are using a work in progress API. This API is not stable"
# warning "and is subject to change. You use this at your own risk."
#endif

#include <Eina.h>

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef EFL_ECORE_WINCE_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif /* ! DLL_EXPORT */
# else
#  define EAPI __declspec(dllimport)
# endif /* ! EFL_ECORE_WINCE_BUILD */
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
#endif /* ! _WINCE */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup Ecore_WinCE_Group Ecore_WinCE library
 *
 * @{
 */


/**
 * @typedef Ecore_WinCE_Window
 * Abstract type for a window.
 */
typedef struct _Ecore_WinCE_Window Ecore_WinCE_Window;


/**
 * @typedef Ecore_WinCE_Event_Mouse_In
 * Event sent when the mouse enters the window.
 */
typedef struct _Ecore_WinCE_Event_Mouse_In              Ecore_WinCE_Event_Mouse_In;

/**
 * @typedef Ecore_WinCE_Event_Mouse_Out
 * Event sent when the mouse leaves the window.
 */
typedef struct _Ecore_WinCE_Event_Mouse_Out             Ecore_WinCE_Event_Mouse_Out;

/**
 * @typedef Ecore_WinCE_Event_Window_Focus_In
 * Event sent when the window gets the focus.
 */
typedef struct _Ecore_WinCE_Event_Window_Focus_In       Ecore_WinCE_Event_Window_Focus_In;

/**
 * @typedef Ecore_WinCE_Event_Window_Focus_Out
 * Event sent when the window looses the focus.
 */
typedef struct _Ecore_WinCE_Event_Window_Focus_Out      Ecore_WinCE_Event_Window_Focus_Out;

/**
 * @typedef Ecore_WinCE_Event_Window_Damage
 * Event sent when the window is damaged.
 */
typedef struct _Ecore_WinCE_Event_Window_Damage         Ecore_WinCE_Event_Window_Damage;

/**
 * @typedef Ecore_WinCE_Event_Window_Create
 * Event sent when the window is created.
 */
typedef struct _Ecore_WinCE_Event_Window_Create         Ecore_WinCE_Event_Window_Create;

/**
 * @typedef Ecore_WinCE_Event_Window_Destroy
 * Event sent when the window is destroyed.
 */
typedef struct _Ecore_WinCE_Event_Window_Destroy        Ecore_WinCE_Event_Window_Destroy;

/**
 * @typedef Ecore_WinCE_Event_Window_Hide
 * Event sent when the window is hidden.
 */
typedef struct _Ecore_WinCE_Event_Window_Hide           Ecore_WinCE_Event_Window_Hide;

/**
 * @typedef Ecore_WinCE_Event_Window_Show
 * Event sent when the window is shown.
 */
typedef struct _Ecore_WinCE_Event_Window_Show           Ecore_WinCE_Event_Window_Show;

/**
 * @typedef Ecore_WinCE_Event_Window_Delete_Request
 * Event sent when the window is deleted.
 */
typedef struct _Ecore_WinCE_Event_Window_Delete_Request Ecore_WinCE_Event_Window_Delete_Request;


/**
 * @struct _Ecore_WinCE_Event_Mouse_In
 * Event sent when the mouse enters the window.
 */
struct _Ecore_WinCE_Event_Mouse_In
{
   Ecore_WinCE_Window *window; /**< The window that received the event */
   int                 x; /**< The x coordinate where the mouse entered */
   int                 y; /**< The y coordinate where the mouse entered */
   long                time; /**< The time the event occurred */
};

/**
 * @struct _Ecore_WinCE_Event_Mouse_Out
 * Event sent when the mouse leaves the window.
 */
struct _Ecore_WinCE_Event_Mouse_Out
{
   Ecore_WinCE_Window *window; /**< The window that received the event */
   int                 x; /**< The x coordinate where the mouse leaved */
   int                 y; /**< The y coordinate where the mouse leaved */
   long                time; /**< The time the event occurred */
};

/**
 * @struct _Ecore_WinCE_Event_Window_Focus_In
 * Event sent when the window gets the focus.
 */
struct _Ecore_WinCE_Event_Window_Focus_In
{
   Ecore_WinCE_Window *window; /**< The window that received the event */
   long                time; /**< The time the event occurred */
};

/**
 * @struct _Ecore_WinCE_Event_Window_Focus_Out
 * Event sent when the window looses the focus.
 */
struct _Ecore_WinCE_Event_Window_Focus_Out
{
   Ecore_WinCE_Window *window; /**< The window that received the event */
   long                time; /**< The time the event occurred */
};

/**
 * @struct _Ecore_WinCE_Event_Window_Damage
 * Event sent when the window is damaged.
 */
struct _Ecore_WinCE_Event_Window_Damage
{
   Ecore_WinCE_Window *window; /**< The window that received the event */
   int                 x; /**< The x coordinate of the top left corner of the damaged region */
   int                 y; /**< The y coordinate of the top left corner of the damaged region */
   int                 width; /**< The width of the damaged region */
   int                 height; /**< The height of the damaged region */
   long                time; /**< The time the event occurred */
};

/**
 * @struct _Ecore_WinCE_Event_Window_Create
 * Event sent when the window is created.
 */
struct _Ecore_WinCE_Event_Window_Create
{
   Ecore_WinCE_Window *window; /**< The window that received the event */
   long                time; /**< The time the event occurred */
};

/**
 * @struct _Ecore_WinCE_Event_Window_Destroy
 * Event sent when the window is destroyed.
 */
struct _Ecore_WinCE_Event_Window_Destroy
{
   Ecore_WinCE_Window *window; /**< The window that received the event */
   long                time; /**< The time the event occurred */
};

/**
 * @struct _Ecore_WinCE_Event_Window_Hide
 * Event sent when the window is hidden.
 */
struct _Ecore_WinCE_Event_Window_Hide
{
   Ecore_WinCE_Window *window; /**< The window that received the event */
   long                time; /**< The time the event occurred */
};

/**
 * @struct _Ecore_WinCE_Event_Window_Show
 * Event sent when the window is shown.
 */
struct _Ecore_WinCE_Event_Window_Show
{
   Ecore_WinCE_Window *window; /**< The window that received the event */
   long                time; /**< The time the event occurred */
};

/**
 * @struct _Ecore_WinCE_Event_Window_Delete_Request
 * Event sent when the window is deleted.
 */
struct _Ecore_WinCE_Event_Window_Delete_Request
{
   Ecore_WinCE_Window *window; /**< The window that received the event */
   long                time; /**< The time the event occurred */
};


EAPI extern int ECORE_WINCE_EVENT_MOUSE_IN; /**< Ecore_Event for the #Ecore_WinCE_Event_Mouse_In event */
EAPI extern int ECORE_WINCE_EVENT_MOUSE_OUT; /**< Ecore_Event for the #Ecore_WinCE_Event_Mouse_Out event */
EAPI extern int ECORE_WINCE_EVENT_WINDOW_FOCUS_IN; /**< Ecore_Event for the #Ecore_WinCE_Event_Window_Focus_In event */
EAPI extern int ECORE_WINCE_EVENT_WINDOW_FOCUS_OUT; /**< Ecore_Event for the #Ecore_WinCE_Event_Window_Focus_Out event */
EAPI extern int ECORE_WINCE_EVENT_WINDOW_DAMAGE; /**< Ecore_Event for the #Ecore_WinCE_Event_Damage event */
EAPI extern int ECORE_WINCE_EVENT_WINDOW_CREATE; /**< Ecore_Event for the #Ecore_WinCE_Event_Create event */
EAPI extern int ECORE_WINCE_EVENT_WINDOW_DESTROY; /**< Ecore_Event for the #Ecore_WinCE_Event_Destroy event */
EAPI extern int ECORE_WINCE_EVENT_WINDOW_HIDE; /**< Ecore_Event for the #Ecore_WinCE_Event_Hide event */
EAPI extern int ECORE_WINCE_EVENT_WINDOW_SHOW; /**< Ecore_Event for the #Ecore_WinCE_Event_Show event */
EAPI extern int ECORE_WINCE_EVENT_WINDOW_DELETE_REQUEST; /**< Ecore_Event for the #Ecore_WinCE_Event_Window_Delete_Request event */


/* Core */

EAPI int    ecore_wince_init();
EAPI int    ecore_wince_shutdown();
EAPI void   ecore_wince_double_click_time_set(double t);
EAPI double ecore_wince_double_click_time_get(void);
EAPI long   ecore_wince_current_time_get(void);

/* Window */

EAPI Ecore_WinCE_Window *ecore_wince_window_new(Ecore_WinCE_Window *parent,
                                                int                 x,
                                                int                 y,
                                                int                 width,
                                                int                 height);

EAPI void ecore_wince_window_free(Ecore_WinCE_Window *window);

EAPI void *ecore_wince_window_hwnd_get(Ecore_WinCE_Window *window);

EAPI void ecore_wince_window_move(Ecore_WinCE_Window *window,
                                  int                 x,
                                  int                 y);

EAPI void ecore_wince_window_resize(Ecore_WinCE_Window *window,
                                    int                 width,
                                    int                 height);

EAPI void ecore_wince_window_move_resize(Ecore_WinCE_Window *window,
                                         int                 x,
                                         int                 y,
                                         int                 width,
                                         int                 height);

EAPI void ecore_wince_window_show(Ecore_WinCE_Window *window);

EAPI void ecore_wince_window_hide(Ecore_WinCE_Window *window);

EAPI void ecore_wince_window_title_set(Ecore_WinCE_Window *window,
                                       const char         *title);

EAPI void ecore_wince_window_backend_set(Ecore_WinCE_Window *window, int backend);

EAPI void ecore_wince_window_suspend_cb_set(Ecore_WinCE_Window *window, int (*suspend_cb)(int));

EAPI void ecore_wince_window_resume_cb_set(Ecore_WinCE_Window *window, int (*resume_cb)(int));

EAPI void ecore_wince_window_geometry_get(Ecore_WinCE_Window *window,
                                          int                *x,
                                          int                *y,
                                          int                *width,
                                          int                *height);

EAPI void ecore_wince_window_size_get(Ecore_WinCE_Window *window,
                                      int                *width,
                                      int                *height);

EAPI void ecore_wince_window_fullscreen_set(Ecore_WinCE_Window *window,
                                            Eina_Bool           on);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* __ECORE_WINCE_H__ */
