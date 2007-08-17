#ifndef __EVAS_ENGINE_DIRECT3D_H__
#define __EVAS_ENGINE_DIRECT3D_H__


#include <windows.h>
#include <d3d9.h>
#include <d3dx9.h>

typedef struct _Evas_Engine_Info_Direct3D Evas_Engine_Info_Direct3D;

struct _Evas_Engine_Info_Direct3D
{
   /* PRIVATE - don't mess with this baby or evas will poke its tongue out */
   /* at you and make nasty noises */
   Evas_Engine_Info magic;

   struct {
      HWND                window;
      LPDIRECT3D9         object;    /* Direct3D object */
      LPDIRECT3DDEVICE9   device;    /* Direct3D device */
      LPD3DXSPRITE        sprite;    /* Direct3D sprite */
      LPDIRECT3DTEXTURE9  texture;   /* Direct3D texture */

      int                 depth;
      int                 rotation;
   } info;
};


#endif /* __EVAS_ENGINE_DIRECT3D_H__ */
