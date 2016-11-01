#ifndef _EVAS_ENGINE_WAYLAND_EGL_H
# define _EVAS_ENGINE_WAYLAND_EGL_H

typedef struct _Evas_Engine_Info_Wayland_Egl Evas_Engine_Info_Wayland_Egl;

struct _Evas_Engine_Info_Wayland_Egl
{
   /* PRIVATE - don't mess with this baby or evas will poke its tongue out
    * at you and make nasty noises */
   Evas_Engine_Info magic;

   /* engine specific data & parameters it needs to set up */
   struct 
     {
        struct wl_display *display;
        struct wl_surface *wl_surface;
        struct wl_egl_window *win;
        int depth, rotation, edges;
        unsigned int destination_alpha : 1;
     } info;

   /* non-blocking or blocking mode */
   Evas_Engine_Render_Mode render_mode;

   Eina_Bool vsync : 1;
   Eina_Bool indirect : 1;

   Eina_Bool www_avail : 1;
   Eina_Bool just_mapped : 1;
   Eina_Rectangle window;
   int x_rel, y_rel;
   int x_cursor, y_cursor;
   int resizing;
   uint32_t timestamp;
   Eina_Bool dragging : 1;
   Eina_Bool drag_start : 1;
   Eina_Bool drag_stop : 1;
   Eina_Bool drag_ack : 1;
};

#endif
