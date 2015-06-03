#ifndef _EVAS_ENGINE_GL_DRM_H
# define _EVAS_ENGINE_GL_DRM_H

# include <Ecore_Drm.h>
# include <gbm.h>

typedef struct _Evas_Engine_Info_GL_Drm Evas_Engine_Info_GL_Drm;

struct _Evas_Engine_Info_GL_Drm
{
   /* PRIVATE - don't mess with this baby or evas will poke its tongue out */
   /* at you and make nasty noises */
   Evas_Engine_Info magic;

   struct 
     {
        struct gbm_device *gbm;
        struct gbm_surface *surface;

        unsigned int rotation, depth;
        unsigned int crtc_id, conn_id, buffer_id;
        unsigned int format, flags;

        Ecore_Drm_Device *dev;

        Eina_Bool destination_alpha : 1;
        Eina_Bool vsync : 1;
        Eina_Bool indirect : 1;
        unsigned char swap_mode : 4;
     } info;

   /* non-blocking or blocking mode */
   Evas_Engine_Render_Mode render_mode;
};

#endif
