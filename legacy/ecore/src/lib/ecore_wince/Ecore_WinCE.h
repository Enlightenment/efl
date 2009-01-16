/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifndef __ECORE_WINCE_H__
#define __ECORE_WINCE_H__


#ifdef EAPI
# undef EAPI
#endif

#ifdef _WINCE
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


#ifndef _ECORE_EVAS_H
typedef void Ecore_WinCE_Window;
#endif


/* Events */

typedef struct _Ecore_WinCE_Event_Key_Down              Ecore_WinCE_Event_Key_Down;
typedef struct _Ecore_WinCE_Event_Key_Up                Ecore_WinCE_Event_Key_Up;
typedef struct _Ecore_WinCE_Event_Mouse_Button_Down     Ecore_WinCE_Event_Mouse_Button_Down;
typedef struct _Ecore_WinCE_Event_Mouse_Button_Up       Ecore_WinCE_Event_Mouse_Button_Up;
typedef struct _Ecore_WinCE_Event_Mouse_Move            Ecore_WinCE_Event_Mouse_Move;
typedef struct _Ecore_WinCE_Event_Mouse_In              Ecore_WinCE_Event_Mouse_In;
typedef struct _Ecore_WinCE_Event_Mouse_Out             Ecore_WinCE_Event_Mouse_Out;
typedef struct _Ecore_WinCE_Event_Window_Focus_In       Ecore_WinCE_Event_Window_Focus_In;
typedef struct _Ecore_WinCE_Event_Window_Focus_Out      Ecore_WinCE_Event_Window_Focus_Out;
typedef struct _Ecore_WinCE_Event_Window_Damage         Ecore_WinCE_Event_Window_Damage;
typedef struct _Ecore_WinCE_Event_Window_Create         Ecore_WinCE_Event_Window_Create;
typedef struct _Ecore_WinCE_Event_Window_Destroy        Ecore_WinCE_Event_Window_Destroy;
typedef struct _Ecore_WinCE_Event_Window_Hide           Ecore_WinCE_Event_Window_Hide;
typedef struct _Ecore_WinCE_Event_Window_Show           Ecore_WinCE_Event_Window_Show;
typedef struct _Ecore_WinCE_Event_Window_Delete_Request Ecore_WinCE_Event_Window_Delete_Request;

struct _Ecore_WinCE_Event_Key_Down
{
   Ecore_WinCE_Window *window;
   char               *keyname;
   char               *keysymbol;
   char               *keycompose;
   double              time;
};

struct _Ecore_WinCE_Event_Key_Up
{
   Ecore_WinCE_Window *window;
   char               *keyname;
   char               *keysymbol;
   char               *keycompose;
   double              time;
};

struct _Ecore_WinCE_Event_Mouse_Button_Down
{
   Ecore_WinCE_Window *window;
   int                 button;
   int                 x;
   int                 y;
   double              time;
   unsigned int        double_click : 1;
   unsigned int        triple_click : 1;
};

struct _Ecore_WinCE_Event_Mouse_Button_Up
{
   Ecore_WinCE_Window *window;
   int                 button;
   int                 x;
   int                 y;
   double              time;
   unsigned int        double_click : 1;
   unsigned int        triple_click : 1;
};

struct _Ecore_WinCE_Event_Mouse_Move
{
   Ecore_WinCE_Window *window;
   int                 x;
   int                 y;
   double              time;
};

struct _Ecore_WinCE_Event_Mouse_In
{
   Ecore_WinCE_Window *window;
   int                 x;
   int                 y;
   double              time;
};

struct _Ecore_WinCE_Event_Mouse_Out
{
   Ecore_WinCE_Window *window;
   int                 x;
   int                 y;
   double              time;
};

struct _Ecore_WinCE_Event_Window_Focus_In
{
   Ecore_WinCE_Window *window;
   double              time;
};

struct _Ecore_WinCE_Event_Window_Focus_Out
{
   Ecore_WinCE_Window *window;
   double              time;
};

struct _Ecore_WinCE_Event_Window_Damage
{
   Ecore_WinCE_Window *window;
   int                 x;
   int                 y;
   int                 width;
   int                 height;
   double              time;
};

struct _Ecore_WinCE_Event_Window_Create
{
   Ecore_WinCE_Window *window;
   double              time;
};

struct _Ecore_WinCE_Event_Window_Destroy
{
   Ecore_WinCE_Window *window;
   double              time;
};

struct _Ecore_WinCE_Event_Window_Hide
{
   Ecore_WinCE_Window *window;
   double              time;
};

struct _Ecore_WinCE_Event_Window_Show
{
   Ecore_WinCE_Window *window;
   double              time;
};

struct _Ecore_WinCE_Event_Window_Delete_Request
{
   Ecore_WinCE_Window *window;
   double              time;
};


EAPI extern int ECORE_WINCE_EVENT_KEY_DOWN;
EAPI extern int ECORE_WINCE_EVENT_KEY_UP;
EAPI extern int ECORE_WINCE_EVENT_MOUSE_BUTTON_DOWN;
EAPI extern int ECORE_WINCE_EVENT_MOUSE_BUTTON_UP;
EAPI extern int ECORE_WINCE_EVENT_MOUSE_MOVE;
EAPI extern int ECORE_WINCE_EVENT_MOUSE_IN;
EAPI extern int ECORE_WINCE_EVENT_MOUSE_OUT;
EAPI extern int ECORE_WINCE_EVENT_WINDOW_FOCUS_IN;
EAPI extern int ECORE_WINCE_EVENT_WINDOW_FOCUS_OUT;
EAPI extern int ECORE_WINCE_EVENT_WINDOW_DAMAGE;
EAPI extern int ECORE_WINCE_EVENT_WINDOW_CREATE;
EAPI extern int ECORE_WINCE_EVENT_WINDOW_DESTROY;
EAPI extern int ECORE_WINCE_EVENT_WINDOW_HIDE;
EAPI extern int ECORE_WINCE_EVENT_WINDOW_SHOW;
EAPI extern int ECORE_WINCE_EVENT_WINDOW_DELETE_REQUEST;


/* Core */

EAPI int    ecore_wince_init();
EAPI int    ecore_wince_shutdown();
EAPI double ecore_wince_current_time_get(void);
EAPI void   ecore_wince_message_loop_begin (void);

/* Window */

EAPI Ecore_WinCE_Window *ecore_wince_window_new(Ecore_WinCE_Window *parent,
                                                int                 x,
                                                int                 y,
                                                int                 width,
                                                int                 height);

EAPI void ecore_wince_window_del(Ecore_WinCE_Window *window);

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

EAPI void ecore_wince_window_suspend_set(Ecore_WinCE_Window *window, int (*suspend)(int));

EAPI void ecore_wince_window_resume_set(Ecore_WinCE_Window *window, int (*resume)(int));

EAPI void ecore_wince_window_geometry_get(Ecore_WinCE_Window *window,
                                          int                *x,
                                          int                *y,
                                          int                *width,
                                          int                *height);

EAPI void ecore_wince_window_size_get(Ecore_WinCE_Window *window,
                                      int                *width,
                                      int                *height);

EAPI void ecore_wince_window_fullscreen_set(Ecore_WinCE_Window *window,
                                            int                 on);


#ifdef __cplusplus
}
#endif

#endif /* __ECORE_WINCE_H__ */
