#ifndef _EVAS_ENGINE_WAYLAND_SHM_H
# define _EVAS_ENGINE_WAYLAND_SHM_H

typedef struct _Evas_Engine_Info_Wayland_Shm Evas_Engine_Info_Wayland_Shm;

struct _Evas_Engine_Info_Wayland_Shm
{
   /* PRIVATE - don't mess with this baby or evas will poke its tongue out */
   /* at you and make nasty noises */
   Evas_Engine_Info magic;

   /* engine specific data and parameters for setup */
   struct 
     {
        /* the wayland shm object used to create new shm pool */
        struct wl_shm *wl_shm;
        struct wl_surface *wl_surface;

        unsigned int rotation, depth;
        Eina_Bool destination_alpha : 1;
        int edges;
     } info;

   /* non-blocking or blocking mode */
   Evas_Engine_Render_Mode render_mode;
};

#endif
