#ifndef __EVAS_ENGINE_SOFTWARE_16_DDRAW_H__
#define __EVAS_ENGINE_SOFTWARE_16_DDRAW_H__


#include <windows.h>
#include <ddraw.h>

typedef struct _Evas_Engine_Info_Software_16_DDraw Evas_Engine_Info_Software_16_DDraw;

struct _Evas_Engine_Info_Software_16_DDraw
{
   /* PRIVATE - don't mess with this baby or evas will poke its tongue out */
   /* at you and make nasty noises */
   Evas_Engine_Info magic;

   struct {
      HWND                window;
      LPDIRECTDRAW        object;          /* DirectDraw object */
      LPDIRECTDRAWSURFACE surface_primary; /* DirectDraw primary surface */
      LPDIRECTDRAWSURFACE surface_back;    /* DirectDraw back surface */
      LPDIRECTDRAWSURFACE surface_source;  /* DirectDraw source surface */
      int                 depth;

      int                 rotation;
   } info;
};


#endif /* __EVAS_ENGINE_SOFTWARE_16_DDRAW_H__ */
