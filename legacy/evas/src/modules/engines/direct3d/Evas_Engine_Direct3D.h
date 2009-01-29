#ifndef __EVAS_ENGINE_DIRECT3D_H__
#define __EVAS_ENGINE_DIRECT3D_H__


#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN

typedef struct _Evas_Engine_Info_Direct3D Evas_Engine_Info_Direct3D;

struct _Evas_Engine_Info_Direct3D
{
   /* PRIVATE - don't mess with this baby or evas will poke its tongue out */
   /* at you and make nasty noises */
   Evas_Engine_Info magic;

   struct {
      HWND window;
      int rotation;
      int depth;
      int fullscreen : 1;
      int layered : 1;
   } info;

   struct {
      unsigned short width;
      unsigned short height;
      unsigned char *mask;
   } *shape;
};


#endif /* __EVAS_ENGINE_DIRECT3D_H__ */
