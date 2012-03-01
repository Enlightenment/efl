#ifndef _ECORE_WAYLAND_H_
# define _ECORE_WAYLAND_H_

# include <Eina.h>
# include <wayland-client.h>
# include <wayland-egl.h> // NB: already includes wayland-client.h
# include <GL/gl.h>
# include <EGL/egl.h>
# include <EGL/eglext.h>

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

typedef enum _Ecore_Wl_Window_Type Ecore_Wl_Window_Type;
typedef enum _Ecore_Wl_Window_Buffer_Type Ecore_Wl_Window_Buffer_Type;

typedef struct _Ecore_Wl_Display Ecore_Wl_Display;
typedef struct _Ecore_Wl_Output Ecore_Wl_Output;
typedef struct _Ecore_Wl_Input Ecore_Wl_Input;
# ifndef _ECORE_WAYLAND_WINDOW_PREDEF
typedef struct _Ecore_Wl_Window Ecore_Wl_Window;
# endif
typedef struct _Ecore_Wl_Dnd_Source Ecore_Wl_Dnd_Source;
typedef struct _Ecore_Wl_Dnd_Target Ecore_Wl_Dnd_Target;

typedef struct _Ecore_Wl_Event_Mouse_In Ecore_Wl_Event_Mouse_In;
typedef struct _Ecore_Wl_Event_Mouse_Out Ecore_Wl_Event_Mouse_Out;
typedef struct _Ecore_Wl_Event_Focus_In Ecore_Wl_Event_Focus_In;
typedef struct _Ecore_Wl_Event_Focus_Out Ecore_Wl_Event_Focus_Out;
typedef struct _Ecore_Wl_Event_Window_Configure Ecore_Wl_Event_Window_Configure;
typedef struct _Ecore_Wl_Event_Dnd_Enter Ecore_Wl_Event_Dnd_Enter;
typedef struct _Ecore_Wl_Event_Dnd_Position Ecore_Wl_Event_Dnd_Position;
typedef struct _Ecore_Wl_Event_Dnd_Leave Ecore_Wl_Event_Dnd_Leave;
typedef struct _Ecore_Wl_Event_Dnd_Drop Ecore_Wl_Event_Dnd_Drop;

enum _Ecore_Wl_Window_Type
{
   ECORE_WL_WINDOW_TYPE_TOPLEVEL,
   ECORE_WL_WINDOW_TYPE_FULLSCREEN,
   ECORE_WL_WINDOW_TYPE_MAXIMIZED,
   ECORE_WL_WINDOW_TYPE_TRANSIENT,
   ECORE_WL_WINDOW_TYPE_MENU,
   ECORE_WL_WINDOW_TYPE_CUSTOM
};

enum _Ecore_Wl_Window_Buffer_Type
{
   ECORE_WL_WINDOW_BUFFER_TYPE_EGL_WINDOW,
   ECORE_WL_WINDOW_BUFFER_TYPE_EGL_IMAGE,
   ECORE_WL_WINDOW_BUFFER_TYPE_SHM
};

struct _Ecore_Wl_Display
{
   struct 
     {
        struct wl_display *display;
        struct wl_compositor *compositor;
        struct wl_shell *shell;
        struct wl_shm *shm;
        struct wl_data_device_manager *data_device_manager;
     } wl;

   struct 
     {
        EGLDisplay display;
        EGLConfig rgb_config;
        EGLConfig argb_config;
        EGLContext rgb_context;
        EGLContext argb_context;
     } egl;

   int fd;
   unsigned int mask;
   Ecore_Fd_Handler *fd_hdl;

   struct wl_list inputs;
   struct wl_list outputs;

   struct xkb_desc *xkb;

   Ecore_Wl_Output *output;

   PFNGLEGLIMAGETARGETTEXTURE2DOESPROC image_target_texture_2d;
   PFNEGLCREATEIMAGEKHRPROC create_image;
   PFNEGLDESTROYIMAGEKHRPROC destroy_image;

   void (*output_configure)(Ecore_Wl_Output *output, void *data);
   void *data;
};

struct _Ecore_Wl_Output
{
   Ecore_Wl_Display *display;
   struct wl_output *output;
   Eina_Rectangle allocation;
   struct wl_list link;

   void (*destroy) (Ecore_Wl_Output *output, void *data);
   void *data;
};

struct _Ecore_Wl_Input
{
   Ecore_Wl_Display *display;
   struct wl_input_device *input_device;
   struct wl_data_device *data_device;

   Ecore_Wl_Window *pointer_focus;
   Ecore_Wl_Window *keyboard_focus;

   unsigned int button;
   unsigned int timestamp;
   unsigned int modifiers;
   int sx, sy;

   struct wl_list link;

   /* TODO: grab */
   unsigned int grab_button;

   Ecore_Wl_Dnd_Source *drag_source;
   Ecore_Wl_Dnd_Source *selection_source;
};

struct _Ecore_Wl_Window
{
   Ecore_Wl_Display *display;
   Ecore_Wl_Window *parent;

   struct wl_surface *surface;
   struct wl_shell_surface *shell_surface;

   int id;
   int x, y;
   int edges;

   Eina_Rectangle allocation, pending_allocation;
   Eina_Rectangle saved_allocation, server_allocation;

   /* Eina_Bool redraw_scheduled : 1; */
   /* Eina_Bool resize_scheduled : 1; */
   Eina_Bool transparent : 1;

   Ecore_Wl_Window_Type type;
   Ecore_Wl_Window_Buffer_Type buffer_type;

   Ecore_Wl_Input *pointer_device;
   Ecore_Wl_Input *keyboard_device;

   void *data;
};

struct _Ecore_Wl_Event_Mouse_In
{
   int modifiers;
   int x, y;
   struct 
     {
        int x, y;
     } root;
   unsigned int win;
   unsigned int event_win;
   unsigned int root_win;
   unsigned int timestamp;
};

struct _Ecore_Wl_Event_Mouse_Out
{
   int modifiers;
   int x, y;
   struct 
     {
        int x, y;
     } root;
   unsigned int win;
   unsigned int event_win;
   unsigned int root_win;
   unsigned int timestamp;
};

struct _Ecore_Wl_Event_Focus_In
{
   unsigned int win;
   unsigned int timestamp;
};

struct _Ecore_Wl_Event_Focus_Out
{
   unsigned int win;
   unsigned int timestamp;
};

struct _Ecore_Wl_Event_Window_Configure
{
   unsigned int win;
   unsigned int event_win;
   int x, y, w, h;
   unsigned int timestamp;
};

struct _Ecore_Wl_Event_Dnd_Enter
{
   unsigned int win, source;
   char **types;
   int num_types;
   struct 
     {
        int x, y;
     } position;
};

struct _Ecore_Wl_Event_Dnd_Position
{
   unsigned int win, source;
   struct 
     {
        int x, y;
     } position;
};

struct _Ecore_Wl_Event_Dnd_Leave
{
   unsigned int win, source;
};

struct _Ecore_Wl_Event_Dnd_Drop
{
   unsigned int win, source;
   struct 
     {
        int x, y;
     } position;
};

/**
 * @file
 * @brief Ecore functions for dealing with the Wayland window system
 * 
 * Ecore_Wl provides a wrapper and convenience functions for using the 
 * Wayland window system. Function groups for this part of the library 
 * include the following:
 * 
 * @li @ref Ecore_Wl_Init_Group
 * @li @ref Ecore_Wl_Display_Group
 * @li @ref Ecore_Wl_Flush_Group
 * @li @ref Ecore_Wl_Window_Group
 */

EAPI extern int ECORE_WL_EVENT_MOUSE_IN;
EAPI extern int ECORE_WL_EVENT_MOUSE_OUT;
EAPI extern int ECORE_WL_EVENT_FOCUS_IN;
EAPI extern int ECORE_WL_EVENT_FOCUS_OUT;
EAPI extern int ECORE_WL_EVENT_WINDOW_CONFIGURE;
EAPI extern int ECORE_WL_EVENT_DND_ENTER;
EAPI extern int ECORE_WL_EVENT_DND_POSITION;
EAPI extern int ECORE_WL_EVENT_DND_LEAVE;
EAPI extern int ECORE_WL_EVENT_DND_DROP;

EAPI int ecore_wl_init(const char *name);
EAPI int ecore_wl_shutdown(void);
EAPI void ecore_wl_flush(void);
EAPI void ecore_wl_sync(void);
EAPI struct wl_shm *ecore_wl_shm_get(void);
EAPI struct wl_display *ecore_wl_display_get(void);
EAPI void ecore_wl_screen_size_get(int *w, int *h);
EAPI void ecore_wl_pointer_xy_get(Ecore_Wl_Window *win, int *x, int *y);

EAPI Ecore_Wl_Window *ecore_wl_window_new(Ecore_Wl_Window *parent, int x, int y, int w, int h, int buffer_type);
EAPI void ecore_wl_window_free(Ecore_Wl_Window *win);
EAPI void ecore_wl_window_move(Ecore_Wl_Window *win, int x, int y);
EAPI void ecore_wl_window_resize(Ecore_Wl_Window *win, int w, int h, int location);
EAPI void ecore_wl_window_damage(Ecore_Wl_Window *win, int x, int y, int w, int h);
EAPI void ecore_wl_window_buffer_attach(Ecore_Wl_Window *win, struct wl_buffer *buffer, int x, int y);
EAPI void ecore_wl_window_show(Ecore_Wl_Window *win);
EAPI void ecore_wl_window_hide(Ecore_Wl_Window *win);
EAPI void ecore_wl_window_raise(Ecore_Wl_Window *win);
EAPI void ecore_wl_window_maximized_set(Ecore_Wl_Window *win, Eina_Bool maximized);
EAPI void ecore_wl_window_fullscreen_set(Ecore_Wl_Window *win, Eina_Bool fullscreen);
EAPI void ecore_wl_window_update_size(Ecore_Wl_Window *win, int w, int h);
EAPI struct wl_surface *ecore_wl_window_surface_get(Ecore_Wl_Window *win);
EAPI Ecore_Wl_Window *ecore_wl_window_find(unsigned int id);

#endif
