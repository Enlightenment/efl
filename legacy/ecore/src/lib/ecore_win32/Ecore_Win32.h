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
typedef void Ecore_Win32_Cursor;
typedef void Ecore_Win32_DDraw_Object;
typedef void Ecore_Win32_DDraw_Surface;
typedef void Ecore_Win32_Direct3D_Object;
typedef void Ecore_Win32_Direct3D_Device;
typedef void Ecore_Win32_Direct3D_Sprite;
typedef void Ecore_Win32_Direct3D_Texture;

/*cursor shapes */

typedef enum
{
   ECORE_WIN32_CURSO_SHAPE_APP_STARTING, /* Standard arrow and small hourglass */
   ECORE_WIN32_CURSO_SHAPE_ARROW,        /* Standard arrow */
   ECORE_WIN32_CURSO_SHAPE_CROSS,        /* Crosshair */
   ECORE_WIN32_CURSO_SHAPE_HAND,         /* Hand */
   ECORE_WIN32_CURSO_SHAPE_HELP,         /* Arrow and question mark */
   ECORE_WIN32_CURSO_SHAPE_I_BEAM,       /* I-beam */
   ECORE_WIN32_CURSO_SHAPE_NO,           /* Slashed circle */
   ECORE_WIN32_CURSO_SHAPE_SIZE_ALL,     /* Four-pointed arrow pointing north, south, east, and west */
   ECORE_WIN32_CURSO_SHAPE_SIZE_NESW,    /* Double-pointed arrow pointing northeast and southwest */
   ECORE_WIN32_CURSO_SHAPE_SIZE_NS,      /* Double-pointed arrow pointing north and south */
   ECORE_WIN32_CURSO_SHAPE_SIZE_NWSE,    /* Double-pointed arrow pointing northwest and southeast */
   ECORE_WIN32_CURSO_SHAPE_SIZE_WE,      /* Double-pointed arrow pointing west and east */
   ECORE_WIN32_CURSO_SHAPE_UP_ARROW,     /* Vertical arrow */
   ECORE_WIN32_CURSO_SHAPE_WAIT          /* Hourglass */
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

EAPI void ecore_win32_window_cursor_set(Ecore_Win32_Window *window,
                                        Ecore_Win32_Cursor *cursor);

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

/* DirectDraw */

EAPI int                        ecore_win32_ddraw_init(Ecore_Win32_Window *window);

EAPI void                       ecore_win32_ddraw_shutdown(Ecore_Win32_Window *window);

EAPI Ecore_Win32_DDraw_Object  *ecore_win32_ddraw_object_get(Ecore_Win32_Window *window);

EAPI Ecore_Win32_DDraw_Surface *ecore_win32_ddraw_surface_primary_get(Ecore_Win32_Window *window);

EAPI Ecore_Win32_DDraw_Surface *ecore_win32_ddraw_surface_back_get(Ecore_Win32_Window *window);

EAPI int                        ecore_win32_ddraw_depth_get(Ecore_Win32_Window *window);

/* Direct3D */

#ifdef __cplusplus
extern "C" {
#endif

EAPI int                           ecore_win32_direct3d_init(Ecore_Win32_Window *window);

EAPI void                          ecore_win32_direct3d_shutdown(Ecore_Win32_Window *window);

EAPI Ecore_Win32_Direct3D_Object  *ecore_win32_direct3d_object_get(Ecore_Win32_Window *window);

EAPI Ecore_Win32_Direct3D_Device  *ecore_win32_direct3d_device_get(Ecore_Win32_Window *window);

EAPI Ecore_Win32_Direct3D_Sprite  *ecore_win32_direct3d_sprite_get(Ecore_Win32_Window *window);

EAPI Ecore_Win32_Direct3D_Texture *ecore_win32_direct3d_texture_get(Ecore_Win32_Window *window);

EAPI int                           ecore_win32_direct3d_depth_get(Ecore_Win32_Window *window);

#ifdef __cplusplus
}
#endif

#endif /* __ECORE_WIN32_H__ */
