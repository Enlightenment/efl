#ifndef _EVAS_ENGINE_WAYLAND_EGL_H
# define _EVAS_ENGINE_WAYLAND_EGL_H

# define EVAS_ENGINE_WAYLAND_EGL_SWAP_MODE_EXISTS 1

typedef struct _Evas_Engine_Info_Wayland_Egl Evas_Engine_Info_Wayland_Egl;

typedef enum _Evas_Engine_Info_Wayland_Egl_Swap_Mode
{
   EVAS_ENGINE_WAYLAND_EGL_SWAP_MODE_AUTO = 0,
   EVAS_ENGINE_WAYLAND_EGL_SWAP_MODE_FULL = 1,
   EVAS_ENGINE_WAYLAND_EGL_SWAP_MODE_COPY = 2,
   EVAS_ENGINE_WAYLAND_EGL_SWAP_MODE_DOUBLE = 3,
   EVAS_ENGINE_WAYLAND_EGL_SWAP_MODE_TRIPLE = 4,
   EVAS_ENGINE_WAYLAND_EGL_SWAP_MODE_QUADRUPLE = 5
} Evas_Engine_Info_Wayland_Egl_Swap_Mode;

struct _Evas_Engine_Info_Wayland_Egl
{
   /* PRIVATE - don't mess with this baby or evas will poke its tongue out
    * at you and make nasty noises */
   Evas_Engine_Info magic;

   /* engine specific data & parameters it needs to set up */
   struct 
     {
        struct wl_display *display;
        struct wl_surface *surface;
        struct wl_egl_window *win;
        int depth, screen, rotation, edges;
        unsigned int destination_alpha : 1;
     } info;

   struct 
     {
        void (*pre_swap) (void *data, Evas *evas);
        void (*post_swap) (void *data, Evas *evas);
        void *data;
     } callback;

   /* non-blocking or blocking mode */
   Evas_Engine_Render_Mode render_mode;

   Eina_Bool vsync : 1;
   Eina_Bool indirect : 1;
   unsigned char swap_mode : 4;
};

#endif
