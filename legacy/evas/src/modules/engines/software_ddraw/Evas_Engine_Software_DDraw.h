#ifndef __EVAS_ENGINE_SOFTWARE_DDRAW_H__
#define __EVAS_ENGINE_SOFTWARE_DDRAW_H__


#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN


typedef struct _Evas_Engine_Info_Software_DDraw Evas_Engine_Info_Software_DDraw;

struct _Evas_Engine_Info_Software_DDraw
{
   /* PRIVATE - don't mess with this baby or evas will poke its tongue out */
   /* at you and make nasty noises */
   Evas_Engine_Info magic;

   struct {
      HWND         window;
      int          depth;
      int          rotation;
      unsigned int fullscreen : 1;
   } info;
};


#endif /* __EVAS_ENGINE_SOFTWARE_DDRAW_H__ */
