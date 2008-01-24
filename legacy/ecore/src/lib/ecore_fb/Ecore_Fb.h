#ifndef _ECORE_FB_H
#define _ECORE_FB_H

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

/**
 * @file
 * @brief Ecore frame buffer system functions.
 */

/* FIXME:
 * maybe a new module?
 * - code to get battery info
 * - code to get thermal info
 * ecore evas fb isnt good enough for weird things, like multiple fb's, same happens here.
 * backlight support using new kernel interface
 * absolute axis
 * joystick
 * ecore_fb_li_device_close_all ? or a shutdown of the subsystem?
 *
 */

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _Ecore_Fb_Input_Device Ecore_Fb_Input_Device; /* an input device handler */

/* device capabilities */
enum _Ecore_Fb_Input_Device_Cap
{
   ECORE_FB_INPUT_DEVICE_CAP_NONE            = 0x00000000,
     ECORE_FB_INPUT_DEVICE_CAP_RELATIVE        = 0x00000001,
     ECORE_FB_INPUT_DEVICE_CAP_ABSOLUTE        = 0x00000002,
     ECORE_FB_INPUT_DEVICE_CAP_KEYS_OR_BUTTONS = 0x00000004
};
typedef enum   _Ecore_Fb_Input_Device_Cap Ecore_Fb_Input_Device_Cap;

EAPI extern int ECORE_FB_EVENT_KEY_DOWN; /**< FB Key Down event */
EAPI extern int ECORE_FB_EVENT_KEY_UP; /**< FB Key Up event */
EAPI extern int ECORE_FB_EVENT_MOUSE_BUTTON_DOWN; /**< FB Mouse Down event */
EAPI extern int ECORE_FB_EVENT_MOUSE_BUTTON_UP; /**< FB Mouse Up event */
EAPI extern int ECORE_FB_EVENT_MOUSE_MOVE; /**< FB Mouse Move event */
EAPI extern int ECORE_FB_EVENT_MOUSE_WHEEL; /**< FB Mouse Wheel event */

typedef struct _Ecore_Fb_Event_Key_Down          Ecore_Fb_Event_Key_Down; /**< FB Key Down event */
typedef struct _Ecore_Fb_Event_Key_Up            Ecore_Fb_Event_Key_Up; /**< FB Key Up event */
typedef struct _Ecore_Fb_Event_Mouse_Button_Down Ecore_Fb_Event_Mouse_Button_Down; /**< FB Mouse Down event */
typedef struct _Ecore_Fb_Event_Mouse_Button_Up   Ecore_Fb_Event_Mouse_Button_Up; /**< FB Mouse Up event */
typedef struct _Ecore_Fb_Event_Mouse_Move        Ecore_Fb_Event_Mouse_Move; /**< FB Mouse Move event */
typedef struct _Ecore_Fb_Event_Mouse_Wheel       Ecore_Fb_Event_Mouse_Wheel; /**< FB Mouse Wheel event */

struct _Ecore_Fb_Event_Key_Down /** FB Key Down event */
{
   Ecore_Fb_Input_Device *dev; /**< The device associated with the event */
   char   *keyname; /**< The name of the key that was pressed */
   char   *keysymbol; /**< The logical symbol of the key that was pressed */
   char   *key_compose; /**< The UTF-8 string conversion if any */
};
   
struct _Ecore_Fb_Event_Key_Up /** FB Key Up event */
{
   Ecore_Fb_Input_Device *dev; /**< The device associated with the event */
   char   *keyname; /**< The name of the key that was released */
   char   *keysymbol; /**< The logical symbol of the key that was pressed */
   char   *key_compose; /**< The UTF-8 string conversion if any */
};
   
struct _Ecore_Fb_Event_Mouse_Button_Down /** FB Mouse Down event */
{
   Ecore_Fb_Input_Device *dev; /**< The device associated with the event */
   int     button; /**< Mouse button that was pressed (1 - 32) */
   int     x; /**< Mouse co-ordinates when mouse button was pressed */
   int     y; /**< Mouse co-ordinates when mouse button was pressed */
   int     double_click : 1; /**< Set if click was a double click */
   int     triple_click : 1; /**< Set if click was a triple click  */
};
   
struct _Ecore_Fb_Event_Mouse_Button_Up /** FB Mouse Up event */
{
   Ecore_Fb_Input_Device *dev; /**< The device associated with the event */
   int     button; /**< Mouse button that was released (1 - 32) */
   int     x; /**< Mouse co-ordinates when mouse button was raised */
   int     y; /**< Mouse co-ordinates when mouse button was raised */
};
   
struct _Ecore_Fb_Event_Mouse_Move /** FB Mouse Move event */ 
{
   Ecore_Fb_Input_Device *dev; /**< The device associated with the event */
   int     x; /**< Mouse co-ordinates where the mouse cursor moved to */
   int     y; /**< Mouse co-ordinates where the mouse cursor moved to */
};
   
struct _Ecore_Fb_Event_Mouse_Wheel /** FB Mouse Wheel event */
{
   Ecore_Fb_Input_Device *dev;
   int x,y;
   int direction; /* 0 = vertical, 1 = horizontal */
   int wheel; /* value 1 (left/up), -1 (right/down) */
};

/* ecore_fb_vt.c */
EAPI void ecore_fb_callback_gain_set(void (*func) (void *data), void *data);
EAPI void ecore_fb_callback_lose_set(void (*func) (void *data), void *data);
/* ecore_fb_li.c */
EAPI Ecore_Fb_Input_Device           *ecore_fb_input_device_open(const char *dev);
EAPI void                             ecore_fb_input_device_close(Ecore_Fb_Input_Device *dev);
EAPI void                             ecore_fb_input_device_listen(Ecore_Fb_Input_Device *dev, int listen);
EAPI const char                      *ecore_fb_input_device_name_get(Ecore_Fb_Input_Device *dev);
EAPI Ecore_Fb_Input_Device_Cap        ecore_fb_input_device_cap_get(Ecore_Fb_Input_Device *dev);
EAPI void                             ecore_fb_input_device_axis_size_set(Ecore_Fb_Input_Device *dev, int w, int h);
EAPI void                             ecore_fb_input_threshold_click_set(Ecore_Fb_Input_Device *dev, double threshold);
EAPI double                           ecore_fb_input_threshold_click_get(Ecore_Fb_Input_Device *dev);
/* ecore_fb.c */
EAPI int    ecore_fb_init(const char *name);
EAPI int    ecore_fb_shutdown(void);
EAPI void   ecore_fb_size_get(int *w, int *h);   

EAPI void   ecore_fb_touch_screen_calibrate_set(int xscale, int xtrans, int yscale, int ytrans, int xyswap);
EAPI void   ecore_fb_touch_screen_calibrate_get(int *xscale, int *xtrans, int *yscale, int *ytrans, int *xyswap);
   
#ifdef __cplusplus
}
#endif

#endif
