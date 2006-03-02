#ifndef _EVAS_ENGINE_SOFTWARE_WIN32_GDI_H
#define _EVAS_ENGINE_SOFTWARE_WIN32_GDI_H

typedef struct _Evas_Engine_Info_Software_Win32_GDI Evas_Engine_Info_Software_Win32_GDI;

struct _Evas_Engine_Info_Software_Win32_GDI
{
   /* PRIVATE - don't mess with this baby or evas will poke its tongue out */
   /* at you and make nasty noises */
   Evas_Engine_Info magic;

   /* engine specific data & parameters it needs to set up */
   struct {
		HWND		 hwnd;
/*
      Colormap  colormap;
*/

      int       depth;
      int       rotation;

   } info;
   /* engine specific function calls to query stuff about the destination */
   /* engine (what visual & colormap & depth to use, performance info etc. */
};
#endif


