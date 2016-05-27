#ifndef _EVAS_ENGINE_DRM_H
# define _EVAS_ENGINE_DRM_H

typedef struct _Evas_Engine_Info_Drm
{
   /* PRIVATE - don't mess with this baby or evas will poke its tongue out */
   /* at you and make nasty noises */
   Evas_Engine_Info magic;

   struct
     {
        int fd;
        int depth, bpp;
        unsigned int format, rotation;

        void *output;
        Eina_Bool alpha : 1;
        Eina_Bool vsync : 1;
     } info;

   /* non-blocking or blocking mode */
   Evas_Engine_Render_Mode render_mode;
} Evas_Engine_Info_Drm;

#endif
