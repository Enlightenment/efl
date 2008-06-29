#ifndef __EVAS_ENGINE_GL_GLEW_H__
#define __EVAS_ENGINE_GL_GLEW_H__


#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN


typedef struct _Evas_Engine_Info_GL_Glew              Evas_Engine_Info_GL_Glew;

struct _Evas_Engine_Info_GL_Glew
{
   /* PRIVATE - don't mess with this baby or evas will poke its tongue out */
   /* at you and make nasty noises */
   Evas_Engine_Info magic;

   /* engine specific data & parameters it needs to set up */
   struct {
      HWND  window;
      int   depth;
   } info;
};


#endif /* __EVAS_ENGINE_GL_GLEW_H__ */
