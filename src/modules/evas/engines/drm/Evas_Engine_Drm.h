#ifndef _EVAS_ENGINE_DRM_H
# define _EVAS_ENGINE_DRM_H

typedef struct _Evas_Engine_Info_Drm Evas_Engine_Info_Drm;

struct _Evas_Engine_Info_Drm
{
   /* PRIVATE - don't mess with this baby or evas will poke its tongue out */
   /* at you and make nasty noises */
   Evas_Engine_Info magic;

   struct 
     {
        unsigned int rotation, depth;
        Eina_Bool destination_alpha : 1;
     } info;

   /* non-blocking or blocking mode */
   Evas_Engine_Render_Mode render_mode;
};

#endif
