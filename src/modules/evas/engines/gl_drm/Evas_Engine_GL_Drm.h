#ifndef _EVAS_ENGINE_GL_DRM_H
# define _EVAS_ENGINE_GL_DRM_H

#include "../gl_generic/Evas_Engine_GL_Shared.h"

# include <Ecore_Drm2.h>
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

        Ecore_Drm2_Device *dev;
        int bpp;
        unsigned int rotation, depth;
        unsigned int format, flags;

        Ecore_Drm2_Output *output;

        Eina_Bool destination_alpha : 1;
        Eina_Bool vsync : 1;
        Eina_Bool indirect : 1;
        Evas_Engine_Info_Gl_Swap_Mode swap_mode : 4;
     } info;

   struct
     {
        void (*pre_swap)(void *data, Evas *evas);
        void (*post_swap)(void *data, Evas *evas);
        void *data;
     } callback;

   /* non-blocking or blocking mode */
   Evas_Engine_Render_Mode render_mode;
};

#endif
