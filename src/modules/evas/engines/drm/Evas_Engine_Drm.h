#ifndef _EVAS_ENGINE_DRM_H
# define _EVAS_ENGINE_DRM_H

# ifdef HAVE_DRM_HW_ACCEL
#  include <EGL/egl.h>
# endif

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

        int fd;
        Eina_Bool own_fd : 1;

        int tty;
        Eina_Bool own_tty : 1;

        int output;
        int plane;

# ifdef HAVE_DRM_HW_ACCEL
        struct 
          {
             EGLDisplay disp;
             EGLContext ctxt;
             EGLConfig cfg;
          } egl;
# endif

        Eina_Bool use_hw_accel : 1;
     } info;

   /* non-blocking or blocking mode */
   Evas_Engine_Render_Mode render_mode;
};

#endif
