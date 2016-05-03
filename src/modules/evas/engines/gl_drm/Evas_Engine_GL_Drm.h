#ifndef _EVAS_ENGINE_GL_DRM_H
# define _EVAS_ENGINE_GL_DRM_H

# include <Ecore_Drm2.h>
# include <gbm.h>

typedef enum _Evas_Engine_Info_GL_Drm_Swap_Mode
{
   EVAS_ENGINE_GL_DRM_SWAP_MODE_AUTO = 0,
   EVAS_ENGINE_GL_DRM_SWAP_MODE_FULL = 1,
   EVAS_ENGINE_GL_DRM_SWAP_MODE_COPY = 2,
   EVAS_ENGINE_GL_DRM_SWAP_MODE_DOUBLE = 3,
   EVAS_ENGINE_GL_DRM_SWAP_MODE_TRIPLE = 4,
   EVAS_ENGINE_GL_DRM_SWAP_MODE_QUADRUPLE = 5
} Evas_Engine_Info_GL_Drm_Swap_Mode;

typedef struct _Evas_Engine_Info_GL_Drm Evas_Engine_Info_GL_Drm;

struct _Evas_Engine_Info_GL_Drm
{
   /* PRIVATE - don't mess with this baby or evas will poke its tongue out */
   /* at you and make nasty noises */
   Evas_Engine_Info magic;

   struct 
     {
        struct gbm_device *gbm;

        int fd, bpp;
        unsigned int rotation, depth;
        unsigned int format, flags;

        void *output;

        Eina_Bool destination_alpha : 1;
        Eina_Bool vsync : 1;
        Eina_Bool indirect : 1;
        unsigned char swap_mode : 4;
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
