#ifndef _EVAS_ENGINE_WAYLAND_EGL_H
# define _EVAS_ENGINE_WAYLAND_EGL_H

# include <wayland-client.h>

# define EVAS_ENGINE_WAYLAND_EGL_SWAP_MODE_EXISTS 1

typedef enum _Evas_Engine_Info_Wayland_Egl_Swap_Mode
{
   EVAS_ENGINE_WAYLAND_EGL_SWAP_MODE_AUTO = 0,
   EVAS_ENGINE_WAYLAND_EGL_SWAP_MODE_FULL = 1,
   EVAS_ENGINE_WAYLAND_EGL_SWAP_MODE_COPY = 2,
   EVAS_ENGINE_WAYLAND_EGL_SWAP_MODE_DOUBLE = 3,
   EVAS_ENGINE_WAYLAND_EGL_SWAP_MODE_TRIPLE = 4
} Evas_Engine_Info_Wayland_Egl_Swap_Mode;

typedef struct _Evas_Engine_Info_Wayland_Egl Evas_Engine_Info_Wayland_Egl;

struct _Evas_Engine_Info_Wayland_Egl
{
   Evas_Engine_Info magic;

   struct 
     {
        struct wl_display *display;
        struct wl_surface *surface;
        int rotation;
        int edges;
        Eina_Bool destination_alpha : 1;
     } info;

   struct 
     {
        void (*pre_swap) (void *data, Evas *evas);
        void (*post_swap) (void *data, Evas *evas);
        void *data;
     } callback;

   Evas_Engine_Render_Mode render_mode;

   Eina_Bool vsync : 1;
   Eina_Bool indirect : 1;

   unsigned char swap_mode : 4;
};

#endif
