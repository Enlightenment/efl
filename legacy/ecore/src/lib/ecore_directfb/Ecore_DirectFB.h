#ifndef _ECORE_DIRECTFB_H
#define _ECORE_DIRECTFB_H

#include <directfb.h>
#include <Ecore_Data.h>

#ifdef EAPI
#undef EAPI
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


#ifdef __cplusplus
extern "C" {
#endif

EAPI extern int ECORE_DIRECTFB_EVENT_POSITION;
EAPI extern int ECORE_DIRECTFB_EVENT_SIZE;
EAPI extern int ECORE_DIRECTFB_EVENT_CLOSE;
EAPI extern int ECORE_DIRECTFB_EVENT_DESTROYED;
EAPI extern int ECORE_DIRECTFB_EVENT_GOT_FOCUS;
EAPI extern int ECORE_DIRECTFB_EVENT_LOST_FOCUS;
EAPI extern int ECORE_DIRECTFB_EVENT_KEY_DOWN;
EAPI extern int ECORE_DIRECTFB_EVENT_KEY_UP;
EAPI extern int ECORE_DIRECTFB_EVENT_BUTTON_DOWN;
EAPI extern int ECORE_DIRECTFB_EVENT_BUTTON_UP;
EAPI extern int ECORE_DIRECTFB_EVENT_MOTION;
EAPI extern int ECORE_DIRECTFB_EVENT_ENTER;
EAPI extern int ECORE_DIRECTFB_EVENT_LEAVE;
EAPI extern int ECORE_DIRECTFB_EVENT_WHEEL;

	

#ifndef _ECORE_DIRECTFB_WINDOW_PREDEF
typedef struct _Ecore_DirectFB_Window			Ecore_DirectFB_Window;
#endif
typedef struct _Ecore_DirectFB_Cursor			Ecore_DirectFB_Cursor;

typedef struct _Ecore_DirectFB_Event_Key_Down		Ecore_DirectFB_Event_Key_Down; 
typedef struct _Ecore_DirectFB_Event_Key_Up		Ecore_DirectFB_Event_Key_Up;
typedef struct _Ecore_DirectFB_Event_Button_Down	Ecore_DirectFB_Event_Button_Down;
typedef struct _Ecore_DirectFB_Event_Button_Up		Ecore_DirectFB_Event_Button_Up;
typedef struct _Ecore_DirectFB_Event_Motion		Ecore_DirectFB_Event_Motion;
typedef struct _Ecore_DirectFB_Event_Enter		Ecore_DirectFB_Event_Enter;
typedef struct _Ecore_DirectFB_Event_Leave		Ecore_DirectFB_Event_Leave;
typedef struct _Ecore_DirectFB_Event_Wheel		Ecore_DirectFB_Event_Wheel;
typedef struct _Ecore_DirectFB_Event_Got_Focus		Ecore_DirectFB_Event_Got_Focus;
typedef struct _Ecore_DirectFB_Event_Lost_Focus		Ecore_DirectFB_Event_Lost_Focus;


/* this struct is to keep windows data (id, window itself and surface) in memory as every call
 * to DirectFB for this values (e.g window->GetSurface(window,&surface)) will increment the
 * reference count, then we will have to release N times the data, so better we just ask for 
 them once */
struct _Ecore_DirectFB_Window
{
	DFBWindowID 		id;
	IDirectFBWindow		*window;
	IDirectFBSurface	*surface;
	Ecore_DirectFB_Cursor	*cursor;
	
};

struct _Ecore_DirectFB_Cursor
{
	IDirectFBSurface	*surface;
	int			hot_x;
	int			hot_y;

};

struct _Ecore_DirectFB_Event_Key_Down /** DirectFB Key Down event */
{
	char   *name; /**< The name of the key that was released */
   	char   *string; /**< The logical symbol of the key that was pressed */
	char   *key_compose; /**< The UTF-8 string conversion if any */
   	unsigned int time;
	DFBWindowID win;
};
   
struct _Ecore_DirectFB_Event_Key_Up /** DirectFB Key Up event */
{
	char   *name; /**< The name of the key that was released */
   	char   *string; /**< The logical symbol of the key that was pressed */
	char   *key_compose; /**< The UTF-8 string conversion if any */
   	unsigned int time;
	DFBWindowID win;
};

struct _Ecore_DirectFB_Event_Button_Down
{
	int button;
	int modifiers;
	int x, y;
	unsigned int time;
	int double_click : 1;
	int triple_click : 1;
	DFBWindowID win;
};
struct _Ecore_DirectFB_Event_Button_Up
{
	int button;
	int modifiers;
	int x, y;
	unsigned int time;
	DFBWindowID win;
};
struct _Ecore_DirectFB_Event_Motion
{
	int modifiers;
	int x, y;
	unsigned int time;
	DFBWindowID win;
};

struct _Ecore_DirectFB_Event_Enter
{
	int modifiers;
	int x, y;
	unsigned int time;
	DFBWindowID win;
};

struct _Ecore_DirectFB_Event_Leave
{
	int modifiers;
	int x, y;
	unsigned int time;
	DFBWindowID win;
};

struct _Ecore_DirectFB_Event_Wheel
{
	int direction;
	int z;
	int modifiers;
	unsigned int time;
	DFBWindowID win;
};

struct _Ecore_DirectFB_Event_Got_Focus
{
	unsigned int time;
	DFBWindowID win;
};

struct _Ecore_DirectFB_Event_Lost_Focus
{
	unsigned int time;
	DFBWindowID win;
};

/* main functions */
EAPI int ecore_directfb_init(const char *name);
EAPI int ecore_directfb_shutdown(void);
EAPI IDirectFB * ecore_directfb_interface_get(void);
/* window operations */
EAPI Ecore_DirectFB_Window * ecore_directfb_window_new(int x, int y, int w, int h);
EAPI void ecore_directfb_window_del(Ecore_DirectFB_Window *window);
EAPI void ecore_directfb_window_move(Ecore_DirectFB_Window *window, int x, int y);
EAPI void ecore_directfb_window_resize(Ecore_DirectFB_Window *window, int w, int h);
EAPI void ecore_directfb_window_focus(Ecore_DirectFB_Window *window);
EAPI void ecore_directfb_window_show(Ecore_DirectFB_Window *window);
EAPI void ecore_directfb_window_hide(Ecore_DirectFB_Window *window);
EAPI void ecore_directfb_window_shaped_set(Ecore_DirectFB_Window *window, int set);
EAPI void ecore_directfb_window_fullscreen_set(Ecore_DirectFB_Window *window, int set);
EAPI void ecore_directfb_window_size_get(Ecore_DirectFB_Window *window, int *w, int *h);
EAPI void ecore_directfb_window_cursor_show(Ecore_DirectFB_Window *window, int show);



#ifdef __cplusplus
}
#endif

#endif
