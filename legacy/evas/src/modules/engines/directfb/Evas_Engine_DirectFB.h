#ifndef _EVAS_ENGINE_DIRECTFB_H
#define _EVAS_ENGINE_DIRECTFB_H

#include <Evas.h>
#include <directfb/directfb.h>

typedef struct _Evas_Engine_Info_DirectFB Evas_Engine_Info_DirectFB;

struct _Evas_Engine_Info_DirectFB
{
   /* PRIVATE - don't mess with this baby or evas will poke its tongue out */
   /* at you and make nasty noises */
   Evas_Engine_Info magic;

   struct Evas_Engine_DirectFB_Spec {
      IDirectFB                 *dfb;
      IDirectFBSurface          *surface;
   } info;
};
#endif


