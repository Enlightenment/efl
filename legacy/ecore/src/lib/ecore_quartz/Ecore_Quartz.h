/*
* vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
*/

#ifndef _ECORE_QUARTZ_H
#define _ECORE_QUARTZ_H

#ifdef EAPI
# undef EAPI
#endif

# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EAPI __attribute__ ((visibility("default")))
#  else
#   define EAPI
#  endif
# else
#  define EAPI
# endif

#ifdef __cplusplus
extern "C" {
#endif

EAPI extern int ECORE_QUARTZ_EVENT_GOT_FOCUS;
EAPI extern int ECORE_QUARTZ_EVENT_LOST_FOCUS;
EAPI extern int ECORE_QUARTZ_EVENT_RESIZE;
EAPI extern int ECORE_QUARTZ_EVENT_EXPOSE;

typedef struct _Ecore_Quartz_Event_Key_Down Ecore_Quartz_Event_Key_Down;
struct _Ecore_Quartz_Event_Key_Down /** Quartz Key Down event */
{
   const char      *keyname; /**< The name of the key that was pressed */
   const char      *keycompose; /**< The UTF-8 string conversion if any */
   unsigned int    time;
};

typedef struct _Ecore_Quartz_Event_Key_Up Ecore_Quartz_Event_Key_Up;
struct _Ecore_Quartz_Event_Key_Up /** Quartz Key Up event */
{
   const char      *keyname; /**< The name of the key that was released */
   const char      *keycompose; /**< The UTF-8 string conversion if any */
   unsigned int    time;
};

typedef struct _Ecore_Quartz_Event_Mouse_Button_Down Ecore_Quartz_Event_Mouse_Button_Down;
struct _Ecore_Quartz_Event_Mouse_Button_Down /** Quartz Mouse Down event */
{
   int             button; /**< Mouse button that was pressed (1 - 32) */
   int             x; /**< Mouse co-ordinates when mouse button was pressed */
   int             y; /**< Mouse co-ordinates when mouse button was pressed */
   int             double_click : 1; /**< Set if click was a double click */
   int             triple_click : 1; /**< Set if click was a triple click  */
   unsigned int    time;
};

typedef struct _Ecore_Quartz_Event_Mouse_Button_Up Ecore_Quartz_Event_Mouse_Button_Up;
struct _Ecore_Quartz_Event_Mouse_Button_Up /** Quartz Mouse Up event */
{
   int             button; /**< Mouse button that was released (1 - 32) */
   int             x; /**< Mouse co-ordinates when mouse button was raised */
   int             y; /**< Mouse co-ordinates when mouse button was raised */
   int             double_click : 1; /**< Set if click was a double click */
   int             triple_click : 1; /**< Set if click was a triple click  */
   unsigned int    time;
};

typedef struct _Ecore_Quartz_Event_Mouse_Move Ecore_Quartz_Event_Mouse_Move;
struct _Ecore_Quartz_Event_Mouse_Move /** Quartz Mouse Move event */ 
{
   int             x; /**< Mouse co-ordinates where the mouse cursor moved to */
   int             y; /**< Mouse co-ordinates where the mouse cursor moved to */
   unsigned int    time;
   void            *window; /**< Need the NSWindow in order to convert coords */
};

typedef struct _Ecore_Quartz_Event_Mouse_Wheel Ecore_Quartz_Event_Mouse_Wheel;
struct _Ecore_Quartz_Event_Mouse_Wheel /** Quartz Mouse Wheel event */
{
   int             x,y;
   int             direction; /* 0 = vertical, 1 = horizontal */
   int             wheel; /* value 1 (left/up), -1 (right/down) */
   unsigned int    time;
};

typedef struct _Ecore_Quartz_Event_Video_Resize Ecore_Quartz_Event_Video_Resize;
struct _Ecore_Quartz_Event_Video_Resize
{
   int             w;
   int             h;
};

EAPI int        ecore_quartz_init(const char *name);
EAPI int        ecore_quartz_shutdown(void);
EAPI void       ecore_quartz_feed_events(void);

#ifdef __cplusplus
}
#endif

#endif
