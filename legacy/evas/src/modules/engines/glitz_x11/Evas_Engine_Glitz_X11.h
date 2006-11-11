#ifndef _EVAS_ENGINE_GLITZ_X11_H
#define _EVAS_ENGINE_GLITZ_X11_H

#include <X11/Xlib.h>

typedef struct _Evas_Engine_Info_Glitz_X11              Evas_Engine_Info_Glitz_X11;

struct _Evas_Engine_Info_Glitz_X11
{
   /* PRIVATE - don't mess with this baby or evas will poke its tongue out */
   /* at you and make nasty noises */
   Evas_Engine_Info magic;

   /* engine specific data & parameters it needs to set up */
   struct {
      Display       *display;
      Drawable       drawable;
      Pixmap         mask;
      Visual        *visual;
      unsigned char  destination_alpha : 1;
   } info;
};
#endif


