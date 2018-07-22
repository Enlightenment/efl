#ifndef __EVAS_ENGINE_SOFTWARE_GDI_H__
#define __EVAS_ENGINE_SOFTWARE_GDI_H__


#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN


typedef struct _Evas_Engine_Info_Software_Gdi Evas_Engine_Info_Software_Gdi;

struct _Evas_Engine_Info_Software_Gdi
{
   /* PRIVATE - don't mess with this baby or evas will poke its tongue out */
   /* at you and make nasty noises */
   Evas_Engine_Info magic;

   struct {
      HWND         window;
      int          rotation;
      unsigned int borderless : 1;
      unsigned int fullscreen : 1;
      unsigned int region     : 1;
   } info;

   /* non-blocking or blocking mode */
   Evas_Engine_Render_Mode render_mode;
};


#endif /* __EVAS_ENGINE_SOFTWARE_GDI_H__ */
