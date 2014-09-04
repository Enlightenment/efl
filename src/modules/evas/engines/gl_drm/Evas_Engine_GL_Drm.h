#ifndef _EVAS_ENGINE_GL_DRM_H
# define _EVAS_ENGINE_GL_DRM_H

# include <gbm.h>
# include <Ecore_Drm.h>

typedef struct _Evas_Engine_Info_GL_Drm Evas_Engine_Info_GL_Drm;

struct _Evas_Engine_Info_GL_Drm
{
   /* PRIVATE - don't mess with this baby or evas will poke its tongue out
    * at you and make nasty noises */
   Evas_Engine_Info magic;

   /* engine specific data & parameters it needs to set up */
   struct
     {
        struct gbm_device *gbm;
        struct gbm_surface *surface;
        uint32_t format;
        uint32_t flags;
        int depth, screen, rotation;
        unsigned char destination_alpha : 1;
        int fd, tty;
        Eina_Bool own_fd : 1;
        Eina_Bool own_tty : 1;
        int output, plane;
        Ecore_Drm_Device *dev;
     } info;

   struct
     {
        void (*pre_swap) (void *data, Evas *evas);
        void (*post_swap) (void *data, Evas *evas);
        void *data;
     } callback;

   /* non-blocking or blocking mode */
   Evas_Engine_Render_Mode render_mode;

   unsigned char vsync : 1;
   unsigned char indirect : 1;
   unsigned char swap_mode : 4;
};

#endif
