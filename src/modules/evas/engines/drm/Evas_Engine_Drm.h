#ifndef _EVAS_ENGINE_DRM_H
# define _EVAS_ENGINE_DRM_H

# include <Ecore_Drm2.h>

typedef struct _Evas_Engine_Info_Drm
{
   Evas_Engine_Info magic;

   struct 
     {
        int depth, bpp;
        unsigned int format, rotation;
        Ecore_Drm2_Device *dev;
        Ecore_Drm2_Output *output;
        Eina_Bool alpha : 1;
        Eina_Bool vsync : 1;
     } info;

   Evas_Engine_Render_Mode render_mode;
} Evas_Engine_Info_Drm;

#endif
