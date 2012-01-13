#ifndef _ECORE_WAYLAND_H_
# define _ECORE_WAYLAND_H_

# include <Eina.h>
# include <wayland-client.h>

# ifdef EAPI
#  undef EAPI
# endif

# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EAPI __attribute__ ((visibility("default")))
#  else
#   define EAPI
#  endif
# else
#  define EAPI
# endif

typedef struct _Ecore_Wl_Event_Mouse_In Ecore_Wl_Event_Mouse_In;
typedef struct _Ecore_Wl_Event_Mouse_Out Ecore_Wl_Event_Mouse_Out;
typedef struct _Ecore_Wl_Event_Focus_In Ecore_Wl_Event_Focus_In;
typedef struct _Ecore_Wl_Event_Focus_Out Ecore_Wl_Event_Focus_Out;

typedef struct _Ecore_Wl_Event_Drag_Start Ecore_Wl_Event_Drag_Start;
typedef struct _Ecore_Wl_Event_Drag_Stop Ecore_Wl_Event_Drag_Stop;

struct _Ecore_Wl_Event_Mouse_In 
{
   int modifiers;
   int x, y;

   struct 
     {
        int x, y;
     } root;

   unsigned int window;

   unsigned int time;
};

struct _Ecore_Wl_Event_Mouse_Out 
{
   int modifiers;
   int x, y;

   struct 
     {
        int x, y;
     } root;

   unsigned int window;

   unsigned int time;
};

struct _Ecore_Wl_Event_Focus_In 
{
   unsigned int window;
   /* TODO: mode & detail */
   unsigned int time;
};

struct _Ecore_Wl_Event_Focus_Out 
{
   unsigned int window;
   /* TODO: mode & detail */
   unsigned int time;
};

struct _Ecore_Wl_Event_Drag_Start
{
   struct wl_data_device *device;
   struct wl_surface *surface;
   const char *mime_type;
   uint32_t timestamp;
};

struct _Ecore_Wl_Event_Drag_Stop
{

};

/**
 * @file
 * @brief Ecore functions for dealing with the Wayland window system
 * 
 * Ecore_Wl provides a wrapper and convenience functions for using the 
 * Wayland window system. Function groups for this part of the library 
 * include the following:
 * @li @ref Ecore_Wl_Init_Group
 */

EAPI int ecore_wl_init(const char *name);
EAPI int ecore_wl_shutdown(void);

EAPI struct wl_display *ecore_wl_display_get(void);
EAPI struct wl_shm *ecore_wl_shm_get(void);
EAPI struct wl_compositor *ecore_wl_compositor_get(void);
EAPI struct wl_shell *ecore_wl_shell_get(void);
EAPI struct wl_input_device *ecore_wl_input_device_get(void);
EAPI void ecore_wl_screen_size_get(int *w, int *h);
EAPI unsigned int ecore_wl_format_get(void);
EAPI void ecore_wl_flush(void);
EAPI void ecore_wl_sync(void);
EAPI void ecore_wl_pointer_xy_get(int *x, int *y);
EAPI void ecore_wl_drag_start();
EAPI void ecore_wl_drag_stop();

EAPI extern int ECORE_WL_EVENT_MOUSE_IN;
EAPI extern int ECORE_WL_EVENT_MOUSE_OUT;
EAPI extern int ECORE_WL_EVENT_FOCUS_IN;
EAPI extern int ECORE_WL_EVENT_FOCUS_OUT;
EAPI extern int ECORE_WL_EVENT_DRAG_START;
EAPI extern int ECORE_WL_EVENT_DRAG_STOP;

#endif
