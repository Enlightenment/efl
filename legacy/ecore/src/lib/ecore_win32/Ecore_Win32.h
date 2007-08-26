/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifndef __ECORE_WIN32_H__
#define __ECORE_WIN32_H__


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


typedef void Ecore_Win32_Window;
typedef void Ecore_Win32_DDraw_Object;
typedef void Ecore_Win32_DDraw_Surface;
typedef void Ecore_Win32_Direct3D_Object;
typedef void Ecore_Win32_Direct3D_Device;
typedef void Ecore_Win32_Direct3D_Sprite;
typedef void Ecore_Win32_Direct3D_Texture;

/* Events */

typedef struct _Ecore_Win32_Event_Key_Down          Ecore_Win32_Event_Key_Down;
typedef struct _Ecore_Win32_Event_Key_Up            Ecore_Win32_Event_Key_Up;
typedef struct _Ecore_Win32_Event_Mouse_Button_Down Ecore_Win32_Event_Mouse_Button_Down;
typedef struct _Ecore_Win32_Event_Mouse_Button_Up   Ecore_Win32_Event_Mouse_Button_Up;
typedef struct _Ecore_Win32_Event_Mouse_Move        Ecore_Win32_Event_Mouse_Move;
typedef struct _Ecore_Win32_Event_Mouse_In          Ecore_Win32_Event_Mouse_In;
typedef struct _Ecore_Win32_Event_Mouse_Out         Ecore_Win32_Event_Mouse_Out;
typedef struct _Ecore_Win32_Event_Window_Focus_In   Ecore_Win32_Event_Window_Focus_In;
typedef struct _Ecore_Win32_Event_Window_Focus_Out  Ecore_Win32_Event_Window_Focus_Out;
typedef struct _Ecore_Win32_Event_Window_Damage     Ecore_Win32_Event_Window_Damage;
typedef struct _Ecore_Win32_Event_Window_Destroy    Ecore_Win32_Event_Window_Destroy;
typedef struct _Ecore_Win32_Event_Window_Hide       Ecore_Win32_Event_Window_Hide;
typedef struct _Ecore_Win32_Event_Window_Show       Ecore_Win32_Event_Window_Show;
typedef struct _Ecore_Win32_Event_Window_Delete     Ecore_Win32_Event_Window_Delete;

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

struct _Ecore_Win32_Event_Window_Delete
{
   Ecore_Win32_Window *window;
   double              time;
};

EAPI extern int ECORE_WIN32_EVENT_KEY_DOWN;
EAPI extern int ECORE_WIN32_EVENT_KEY_UP;
EAPI extern int ECORE_WIN32_EVENT_MOUSE_BUTTON_DOWN;
EAPI extern int ECORE_WIN32_EVENT_MOUSE_BUTTON_UP;
EAPI extern int ECORE_WIN32_EVENT_MOUSE_MOVE;
EAPI extern int ECORE_WIN32_EVENT_MOUSE_IN;
EAPI extern int ECORE_WIN32_EVENT_MOUSE_OUT;
EAPI extern int ECORE_WIN32_EVENT_WINDOW_FOCUS_IN;
EAPI extern int ECORE_WIN32_EVENT_WINDOW_FOCUS_OUT;
EAPI extern int ECORE_WIN32_EVENT_WINDOW_DAMAGE;
EAPI extern int ECORE_WIN32_EVENT_WINDOW_DESTROY;
EAPI extern int ECORE_WIN32_EVENT_WINDOW_HIDE;
EAPI extern int ECORE_WIN32_EVENT_WINDOW_SHOW;
EAPI extern int ECORE_WIN32_EVENT_WINDOW_DELETE;


/* Core */
EAPI int    ecore_win32_init();
EAPI int    ecore_win32_shutdown();
EAPI double ecore_win32_current_time_get(void);
EAPI void   ecore_win32_message_loop_begin (void);

/* Window */

EAPI Ecore_Win32_Window *ecore_win32_window_new(Ecore_Win32_Window *parent,
                                                int                 x,
                                                int                 y,
                                                int                 width,
                                                int                 height);

EAPI void ecore_win32_window_del(Ecore_Win32_Window *window);

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

EAPI void ecore_win32_window_size_get(Ecore_Win32_Window *window,
                                      int                *width,
                                      int                *height);

EAPI void ecore_win32_window_size_min_set(Ecore_Win32_Window *window,
                                          int                 min_width,
                                          int                 min_height);

EAPI void ecore_win32_window_size_max_set(Ecore_Win32_Window *window,
                                          int                 max_width,
                                          int                 max_height);

EAPI void ecore_win32_window_size_base_set(Ecore_Win32_Window *window,
                                           int                 base_width,
                                           int                 base_height);

EAPI void ecore_win32_window_size_step_set(Ecore_Win32_Window *window,
                                           int                 step_width,
                                           int                 step_height);

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

/* DirectDraw */

EAPI int   ecore_win32_ddraw_init(Ecore_Win32_Window *window);

EAPI void  ecore_win32_ddraw_shutdown(Ecore_Win32_Window *window);

EAPI Ecore_Win32_DDraw_Object *ecore_win32_ddraw_object_get(Ecore_Win32_Window *window);

EAPI Ecore_Win32_DDraw_Surface *ecore_win32_ddraw_surface_primary_get(Ecore_Win32_Window *window);

EAPI Ecore_Win32_DDraw_Surface *ecore_win32_ddraw_surface_back_get(Ecore_Win32_Window *window);

EAPI int ecore_win32_ddraw_depth_get(Ecore_Win32_Window *window);

/* Direct3D */

#ifdef __cplusplus
extern "C" {
#endif

EAPI int   ecore_win32_direct3d_init(Ecore_Win32_Window *window);

EAPI void  ecore_win32_direct3d_shutdown(Ecore_Win32_Window *window);

EAPI Ecore_Win32_Direct3D_Object *ecore_win32_direct3d_object_get(Ecore_Win32_Window *window);

EAPI Ecore_Win32_Direct3D_Device *ecore_win32_direct3d_device_get(Ecore_Win32_Window *window);

EAPI Ecore_Win32_Direct3D_Sprite *ecore_win32_direct3d_sprite_get(Ecore_Win32_Window *window);

EAPI Ecore_Win32_Direct3D_Texture *ecore_win32_direct3d_texture_get(Ecore_Win32_Window *window);

EAPI int ecore_win32_direct3d_depth_get(Ecore_Win32_Window *window);

#ifdef __cplusplus
}
#endif

#endif /* __ECORE_WIN32_H__ */
