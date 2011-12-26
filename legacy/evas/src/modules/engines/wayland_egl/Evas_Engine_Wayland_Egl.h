#ifndef _EVAS_ENGINE_WAYLAND_EGL_H
# define _EVAS_ENGINE_WAYLAND_EGL_H

# include <wayland-client.h>
# include <wayland-egl.h>

typedef struct _Evas_Engine_Info_Wayland_Egl Evas_Engine_Info_Wayland_Egl;
struct _Evas_Engine_Info_Wayland_Egl 
{
   Evas_Engine_Info magic;

   struct 
     {
        struct wl_display *disp;
        struct wl_compositor *comp;
        struct wl_shell *shell;

        int rotation;

        Eina_Bool debug : 1;
     } info;

   Evas_Engine_Render_Mode render_mode;
};

#endif
