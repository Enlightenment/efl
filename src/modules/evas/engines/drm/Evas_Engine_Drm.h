#ifndef _EVAS_ENGINE_DRM_H
# define _EVAS_ENGINE_DRM_H

#include <Ecore_Drm.h>

typedef struct _Evas_Engine_Info_Drm Evas_Engine_Info_Drm;

struct _Evas_Engine_Info_Drm
{
   /* PRIVATE - don't mess with this baby or evas will poke its tongue out */
   /* at you and make nasty noises */
   Evas_Engine_Info magic;

   struct 
     {
        Evas *evas;

        unsigned int rotation, depth;
        Eina_Bool destination_alpha : 1;
        Eina_Bool vsync : 1;

        int fd;
        Eina_Bool own_fd : 1;

        int tty;
        Eina_Bool own_tty : 1;

        int output;
        int plane;

        Eina_Bool use_hw_accel : 1;
        Ecore_Drm_Device *dev;
     } info;

   /* non-blocking or blocking mode */
   Evas_Engine_Render_Mode render_mode;
};

#endif
