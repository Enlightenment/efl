#ifndef _ECORE_FB_H
#define _ECORE_FB_H

/* FIXME:
 * maybe a new module?
 * - code to get battery info
 * - code to get thermal info
 */

#ifdef __cplusplus
extern "C" {
#endif

extern int ECORE_FB_EVENT_KEY_DOWN; /**< FB Key Down event */
extern int ECORE_FB_EVENT_KEY_UP; /**< FB Key Up event */
extern int ECORE_FB_EVENT_MOUSE_BUTTON_DOWN; /**< FB Mouse Down event */
extern int ECORE_FB_EVENT_MOUSE_BUTTON_UP; /**< FB Mouse Up event */
extern int ECORE_FB_EVENT_MOUSE_MOVE; /**< FB Mouse Move event */

typedef struct _Ecore_Fb_Event_Key_Down          Ecore_Fb_Event_Key_Down; /**< FB Key Down event */
typedef struct _Ecore_Fb_Event_Key_Up            Ecore_Fb_Event_Key_Up; /**< FB Key Up event */
typedef struct _Ecore_Fb_Event_Mouse_Button_Down Ecore_Fb_Event_Mouse_Button_Down; /**< FB Mouse Down event */
typedef struct _Ecore_Fb_Event_Mouse_Button_Up   Ecore_Fb_Event_Mouse_Button_Up; /**< FB Mouse Up event */
typedef struct _Ecore_Fb_Event_Mouse_Move        Ecore_Fb_Event_Mouse_Move; /**< FB Mouse Move event */

struct _Ecore_Fb_Event_Key_Down /** FB Key Down event */
{
   char   *keyname; /**< The name of the key that was prerssed */
};
   
struct _Ecore_Fb_Event_Key_Up /** FB Key Up event */
{
   char   *keyname; /**< The name of the key that was released */
};
   
struct _Ecore_Fb_Event_Mouse_Button_Down /** FB Mouse Down event */
{
   int     button; /**< Mouse button that was pressed (1 - 32) */
   int     x; /**< Mouse co-ordinates when mouse button was pressed */
   int     y; /**< Mouse co-ordinates when mouse button was pressed */
   int     double_click : 1; /**< Set if click was a double click */
   int     triple_click : 1; /**< Set if click was a triple click  */
};
   
struct _Ecore_Fb_Event_Mouse_Button_Up /** FB Mouse Up event */
{
   int     button; /**< Mouse button that was released (1 - 32) */
   int     x; /**< Mouse co-ordinates when mouse button was raised */
   int     y; /**< Mouse co-ordinates when mouse button was raised */
};
   
struct _Ecore_Fb_Event_Mouse_Move /** FB Mouse Move event */ 
{
   int     x; /**< Mouse co-ordinates where the mouse cursor moved to */
   int     y; /**< Mouse co-ordinates where the mouse cursor moved to */
};

int    ecore_fb_init(const char *name);
int    ecore_fb_shutdown(void);
   
void   ecore_fb_double_click_time_set(double t);
double ecore_fb_double_click_time_get(void);

void   ecore_fb_size_get(int *w, int *h);   
   
void   ecore_fb_touch_screen_calibrate_set(int xscale, int xtrans, int yscale, int ytrans, int xyswap);
void   ecore_fb_touch_screen_calibrate_get(int *xscale, int *xtrans, int *yscale, int *ytrans, int *xyswap);

void   ecore_fb_backlight_set(int on);
int    ecore_fb_backlight_get(void);

void   ecore_fb_backlight_brightness_set(double br);
double ecore_fb_backlight_brightness_get(void);

void   ecore_fb_led_set(int on);
void   ecore_fb_led_blink_set(double speed);

void   ecore_fb_contrast_set(double cr);
double ecore_fb_contrast_get(void);

double ecore_fb_light_sensor_get(void);

void ecore_fb_callback_gain_set(void (*func) (void *data), void *data);
void ecore_fb_callback_lose_set(void (*func) (void *data), void *data);
       
#ifdef __cplusplus
}
#endif

#endif
